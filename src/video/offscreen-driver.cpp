// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// Copyright (C) 2008-2017 The Antares Authors
//
// This file is part of Antares, a tactical space combat game.
//
// Antares is free software: you can redistribute it and/or modify it
// under the terms of the Lesser GNU General Public License as published
// by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Antares is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with Antares.  If not, see http://www.gnu.org/licenses/

#include "video/offscreen-driver.hpp"

#include <fcntl.h>
#include <stdlib.h>
#include <strings.h>

#include <algorithm>
#include <pn/output>
#include <sfz/sfz.hpp>

#include "config/preferences.hpp"
#include "drawing/pix-map.hpp"
#include "game/sys.hpp"
#include "game/time.hpp"
#include "math/geometry.hpp"
#include "ui/card.hpp"
#include "ui/event.hpp"

#ifdef __APPLE__
#include <OpenGL/OpenGL.h>
#include <OpenGL/gl.h>

#include "mac/offscreen.hpp"
#else
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glu.h>

#include "linux/offscreen.hpp"
#endif

using sfz::dec;
using sfz::range;
using std::greater;
using std::max;
using std::pair;
using std::unique_ptr;
using std::vector;

namespace path = sfz::path;

namespace antares {

namespace {

class SnapshotBuffer {
  public:
    SnapshotBuffer() {}

    void copy(Rect bounds, ArrayPixMap& pix) {
        _data.resize(bounds.area() * 4);
        Size size = bounds.size();
        glReadPixels(
                bounds.left, bounds.top, size.width, size.height, GL_BGRA, GL_UNSIGNED_BYTE,
                _data.data());
        uint8_t* p = _data.data();
        for (int32_t y : range(size.height)) {
            for (int32_t x : range(size.width)) {
                uint8_t blue  = *(p++);
                uint8_t green = *(p++);
                uint8_t red   = *(p++);
                ++p;
                pix.set(x, size.height - y - 1, rgb(red, green, blue));
            }
        }
    }

  private:
    vector<uint8_t> _data;
};

void gl_check() {
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        throw std::runtime_error(pn::format("gl: {0}", error).c_str());
    }
}

struct Framebuffer {
    GLuint id;

    Framebuffer() {
        glGenFramebuffers(1, &id);
        gl_check();
    }

    ~Framebuffer() { glDeleteFramebuffers(1, &id); }
};

struct Renderbuffer {
    GLuint id;

    Renderbuffer() {
        glGenRenderbuffers(1, &id);
        gl_check();
    }

    ~Renderbuffer() { glDeleteRenderbuffers(1, &id); }
};

}  // namespace

class OffscreenVideoDriver::MainLoop : public EventScheduler::MainLoop {
  public:
    MainLoop(
            OffscreenVideoDriver& driver, const sfz::optional<pn::string>& output_dir,
            Card* initial)
            : _driver(driver),
              _offscreen(driver.viewport_size(), driver._gl_version),
              _setup(*this),
              _loop(driver, initial) {
        if (output_dir.has_value()) {
            _output_dir.emplace(output_dir->copy());
        }
    }

    bool takes_snapshots() { return _output_dir.has_value(); }

    void snapshot(wall_ticks ticks) {
        snapshot_to(
                _driver._capture_rect,
                pn::format("screens/{0}.png", dec(ticks.time_since_epoch().count(), 6)));
    }

    void snapshot_to(Rect bounds, pn::string_view relpath) {
        if (!takes_snapshots()) {
            return;
        }
        bounds = Rect{
                bounds.left * _driver._scale,
                bounds.top * _driver._scale,
                bounds.right * _driver._scale,
                bounds.bottom * _driver._scale,
        };
        bounds.offset(0, _driver.viewport_size().height - bounds.height() - bounds.top);
        ArrayPixMap pix(bounds.size());
        _buffer.copy(bounds, pix);
        pn::string path = pn::format("{0}/{1}", *_output_dir, relpath);
        sfz::makedirs(path::dirname(path), 0755);
        pn::output out{path, pn::binary};
        pix.encode(out);
    }

    void  draw() { _loop.draw(); }
    bool  done() const { return _loop.done(); }
    Card* top() const { return _loop.top(); }

  private:
    const OffscreenVideoDriver& _driver;
    Offscreen                   _offscreen;
    Framebuffer                 _fb;
    Renderbuffer                _rb;
    SnapshotBuffer              _buffer;
    struct Setup {
        Setup(OffscreenVideoDriver::MainLoop& loop) {
            glBindFramebuffer(GL_FRAMEBUFFER, loop._fb.id);
            glBindRenderbuffer(GL_RENDERBUFFER, loop._rb.id);
            glRenderbufferStorage(
                    GL_RENDERBUFFER, GL_RGBA, loop._driver.viewport_size().width,
                    loop._driver.viewport_size().height);
            glFramebufferRenderbuffer(
                    GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, loop._rb.id);
        }
    };
    Setup                       _setup;
    sfz::optional<pn::string>   _output_dir;
    OpenGlVideoDriver::MainLoop _loop;
};

OffscreenVideoDriver::OffscreenVideoDriver(
        Size screen_size, int scale, std::pair<int, int> gl_version, pn::string_view glsl_version,
        const sfz::optional<pn::string>& output_dir)
        : _screen_size(screen_size),
          _scale(scale),
          _gl_version(gl_version),
          _glsl_version(glsl_version.copy()),
          _capture_rect(screen_size.as_rect()) {
    if (output_dir.has_value()) {
        _output_dir.emplace(output_dir->copy());
    }
}

pn::string_view OffscreenVideoDriver::glsl_version() const { return _glsl_version; }

bool OffscreenVideoDriver::start_editing(TextReceiver* text) { return false; }

void OffscreenVideoDriver::stop_editing(TextReceiver* text) {}

void OffscreenVideoDriver::loop(Card* initial, EventScheduler& scheduler) {
    _scheduler = &scheduler;
    MainLoop loop(*this, _output_dir, initial);
    _scheduler->loop(loop);
    _scheduler = nullptr;
}

namespace {

class DummyCard : public Card {
  public:
    void become_front() {
        if (!_inited) {
            sys_init();
            _inited = true;
        }
    }

  private:
    bool _inited = false;
};

}  // namespace

void OffscreenVideoDriver::capture(vector<pair<unique_ptr<Card>, pn::string>>& pix) {
    MainLoop loop(*this, _output_dir, new DummyCard);
    for (auto& p : pix) {
        loop.top()->stack()->push(p.first.release());
        loop.draw();
        loop.snapshot_to(_capture_rect, p.second);
        loop.top()->stack()->pop(loop.top());
    }
}

}  // namespace antares

// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// Copyright (C) 2008-2012 The Antares Authors
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
#include <OpenGL/OpenGL.h>
#include <OpenGL/gl.h>
#include <sfz/sfz.hpp>

#include "config/preferences.hpp"
#include "drawing/pix-map.hpp"
#include "game/time.hpp"
#include "mac/core-opengl.hpp"
#include "math/geometry.hpp"
#include "ui/card.hpp"
#include "ui/event.hpp"

using sfz::BytesSlice;
using sfz::Exception;
using sfz::Optional;
using sfz::PrintItem;
using sfz::ScopedFd;
using sfz::String;
using sfz::StringSlice;
using sfz::WriteTarget;
using sfz::dec;
using sfz::format;
using sfz::range;
using std::greater;
using std::max;
using std::unique_ptr;

namespace path = sfz::path;
namespace utf8 = sfz::utf8;

namespace antares {

namespace {

class SnapshotBuffer {
  public:
    SnapshotBuffer(Size screen_size, int32_t bytes_per_pixel):
            _screen_size(screen_size),
            _bytes_per_pixel(bytes_per_pixel),
            _data(new uint8_t[width() * height() * bytes_per_pixel]) { }

    int32_t width() const { return _screen_size.width; }
    int32_t height() const { return _screen_size.height; }
    int32_t row_bytes() const { return width() * _bytes_per_pixel; }
    void* mutable_data() { return _data.get(); }

    void write_to(const WriteTarget& out) const {
        ArrayPixMap pix(_screen_size.width, _screen_size.height);
        uint8_t* p = _data.get();
        for (int32_t y: range(_screen_size.height)) {
            for (int32_t x: range(_screen_size.width)) {
                uint8_t blue = *(p++);
                uint8_t green = *(p++);
                uint8_t red = *(p++);
                ++p;
                pix.set(x, _screen_size.height - y - 1, RgbColor(red, green, blue));
            }
        }
        write(out, pix);
    }

  private:
    const Size _screen_size;
    const int32_t _bytes_per_pixel;
    unique_ptr<uint8_t[]> _data;
};

void write_to(const WriteTarget& out, const SnapshotBuffer& buffer) {
    buffer.write_to(out);
}

static const CGLPixelFormatAttribute kAttrs[] = {
    kCGLPFAOpenGLProfile, (CGLPixelFormatAttribute)kCGLOGLPVersion_3_2_Core,
    kCGLPFAColorSize, static_cast<CGLPixelFormatAttribute>(24),
    kCGLPFAAccelerated,
    static_cast<CGLPixelFormatAttribute>(0),
};

void gl_check() {
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        throw Exception(error);
    }
}

struct Framebuffer {
    GLuint id;

    Framebuffer() {
        glGenFramebuffers(1, &id);
        gl_check();
    }

    ~Framebuffer() {
        glDeleteFramebuffers(1, &id);
    }
};

struct Renderbuffer {
    GLuint id;

    Renderbuffer() {
        glGenRenderbuffers(1, &id);
        gl_check();
    }

    ~Renderbuffer() {
        glDeleteRenderbuffers(1, &id);
    }
};

}  // namespace

class OffscreenVideoDriver::MainLoop : public EventScheduler::MainLoop {
  public:
    MainLoop(OffscreenVideoDriver& driver, const Optional<String>& output_dir, Card* initial):
            _pix(kAttrs),
            _context(_pix.c_obj(), NULL),
            _buffer(Preferences::preferences()->screen_size(), 4),
            _set_context(*this),
            _setup(*this),
            _output_dir(output_dir),
            _loop(driver, initial) {
    }

    bool takes_snapshots() {
        return _output_dir.has();
    }

    void snapshot(int64_t ticks) {
        snapshot_to(format("screens/{0}.png", dec(ticks, 6)));
    }

    void snapshot_to(PrintItem relpath) {
        if (!takes_snapshots()) {
            return;
        }
        glReadPixels(
                0, 0, _buffer.width(), _buffer.height(), GL_BGRA, GL_UNSIGNED_BYTE,
                _buffer.mutable_data());
        String path(format("{0}/{1}", *_output_dir, relpath));
        makedirs(path::dirname(path), 0755);
        ScopedFd file(open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644));
        write(file, _buffer);
    }

    void draw() { _loop.draw(); }
    bool done() const { return _loop.done(); }
    Card* top() const { return _loop.top(); }

  private:
    cgl::PixelFormat _pix;
    cgl::Context _context;
    struct SetContext {
        SetContext(OffscreenVideoDriver::MainLoop& loop) {
            cgl::check(CGLSetCurrentContext(loop._context.c_obj()));
        }
    };
    SetContext _set_context;
    Framebuffer _fb;
    Renderbuffer _rb;
    SnapshotBuffer _buffer;
    struct Setup {
        Setup(OffscreenVideoDriver::MainLoop& loop) {
            glBindFramebuffer(GL_FRAMEBUFFER, loop._fb.id);
            glBindRenderbuffer(GL_RENDERBUFFER, loop._rb.id);
            glRenderbufferStorage(
                    GL_RENDERBUFFER, GL_RGBA, loop._buffer.width(), loop._buffer.height());
            glFramebufferRenderbuffer(
                    GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, loop._rb.id);
        }
    };
    Setup _setup;
    Optional<String> _output_dir;
    OpenGlVideoDriver::MainLoop _loop;
};

OffscreenVideoDriver::OffscreenVideoDriver(Size screen_size, const Optional<String>& output_dir):
        _screen_size(screen_size),
        _output_dir(output_dir) { }

void OffscreenVideoDriver::loop(Card* initial, EventScheduler& scheduler) {
    _scheduler = &scheduler;
    MainLoop loop(*this, _output_dir, initial);
    _scheduler->loop(loop);
    _scheduler = nullptr;
}

void OffscreenVideoDriver::capture(Card* card, PrintItem path) {
    MainLoop loop(*this, _output_dir, card);
    loop.draw();
    loop.snapshot_to(path);
}

}  // namespace antares

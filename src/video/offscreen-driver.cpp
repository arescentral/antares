// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// Copyright (C) 2008-2011 Ares Central
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
// License along with this program.  If not, see
// <http://www.gnu.org/licenses/>.

#include "video/offscreen-driver.hpp"

#include <fcntl.h>
#include <stdlib.h>
#include <strings.h>
#include <algorithm>
#include <OpenGL/OpenGL.h>
#include <OpenGL/gl.h>
#include <sfz/sfz.hpp>

#include "cocoa/core-opengl.hpp"
#include "config/preferences.hpp"
#include "drawing/pix-map.hpp"
#include "game/time.hpp"
#include "math/geometry.hpp"
#include "ui/card.hpp"
#include "ui/event.hpp"

using sfz::BytesSlice;
using sfz::Exception;
using sfz::Optional;
using sfz::ScopedFd;
using sfz::String;
using sfz::WriteTarget;
using sfz::dec;
using sfz::format;
using sfz::linked_ptr;
using sfz::make_linked_ptr;
using sfz::range;
using sfz::scoped_array;
using std::greater;
using std::max;
namespace utf8 = sfz::utf8;

namespace antares {

class OffscreenVideoDriver::SnapshotBuffer {
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
        SFZ_FOREACH(int32_t y, range(_screen_size.height), {
            SFZ_FOREACH(int32_t x, range(_screen_size.width), {
                uint8_t blue = *(p++);
                uint8_t green = *(p++);
                uint8_t red = *(p++);
                ++p;
                pix.set(x, y, RgbColor(red, green, blue));
            });
        });
        write(out, pix);
    }

  private:
    const Size _screen_size;
    const int32_t _bytes_per_pixel;
    scoped_array<uint8_t> _data;
};

void write_to(const WriteTarget& out, const OffscreenVideoDriver::SnapshotBuffer& buffer) {
    buffer.write_to(out);
}

OffscreenVideoDriver::OffscreenVideoDriver(
        Size screen_size, const Optional<String>& output_dir):
        OpenGlVideoDriver(screen_size),
        _demo(0),
        _output_dir(output_dir),
        _ticks(0),
        _event_tracker(true) { }

void OffscreenVideoDriver::set_demo_scenario(int demo) {
    _demo = demo;
}

int OffscreenVideoDriver::get_demo_scenario() {
    return _demo;
}

void OffscreenVideoDriver::loop(Card* initial) {
    CGLPixelFormatAttribute attrs[] = {
        kCGLPFAColorSize, static_cast<CGLPixelFormatAttribute>(24),
        kCGLPFAStencilSize, static_cast<CGLPixelFormatAttribute>(8),
        // kCGLPFAAccelerated,
        kCGLPFAOffScreen,
        kCGLPFARemotePBuffer,
        static_cast<CGLPixelFormatAttribute>(0),
    };

    cgl::PixelFormat pix(attrs);
    cgl::Context context(pix.c_obj(), NULL);
    cgl::check(CGLSetCurrentContext(context.c_obj()));

    SnapshotBuffer buffer(Preferences::preferences()->screen_size(), 4);
    cgl::check(CGLSetOffScreen(context.c_obj(), buffer.width(), buffer.height(),
                buffer.row_bytes(), buffer.mutable_data()));

    MainLoop loop(*this, initial);
    while (!loop.done()) {
        int64_t at_usecs;
        const bool has_timer = loop.top()->next_timer(at_usecs);
        const int64_t at_ticks = at_usecs * 60 / 1000000;
        if (!_event_heap.empty() && (!has_timer || (_event_heap.front()->at() <= at_ticks))) {
            linked_ptr<Event> event = _event_heap.front();
            pop_heap(_event_heap.begin(), _event_heap.end(), is_later);
            _event_heap.pop_back();
            advance_tick_count(&loop, buffer, event->at());
            event->send(&_event_tracker);
            event->send(loop.top());
        } else {
            if (!has_timer) {
                throw Exception("Event heap empty and timer not set to fire.");
            }
            advance_tick_count(&loop, buffer, max(_ticks + 1, at_ticks));
            loop.top()->fire_timer();
        }
    }
}

void OffscreenVideoDriver::schedule_snapshot(int64_t at) {
    _snapshot_times.push_back(at);
    push_heap(_snapshot_times.begin(), _snapshot_times.end(), greater<int64_t>());
}

void OffscreenVideoDriver::schedule_event(linked_ptr<Event> event) {
    _event_heap.push_back(event);
    push_heap(_event_heap.begin(), _event_heap.end(), is_later);
}

void OffscreenVideoDriver::schedule_key(int32_t key, int64_t down, int64_t up) {
    schedule_event(make_linked_ptr(new KeyDownEvent(down, key)));
    schedule_event(make_linked_ptr(new KeyUpEvent(up, key)));
}

void OffscreenVideoDriver::schedule_mouse(
        int button, const Point& where, int64_t down, int64_t up) {
    schedule_event(make_linked_ptr(new MouseDownEvent(down, button, where)));
    schedule_event(make_linked_ptr(new MouseUpEvent(up, button, where)));
}

void OffscreenVideoDriver::advance_tick_count(
        MainLoop* loop, const SnapshotBuffer& buffer, int64_t ticks) {
    if (_output_dir.has() && have_snapshots_before(ticks)) {
        loop->draw();
        while (have_snapshots_before(ticks)) {
            _ticks = _snapshot_times.front();
            String dir(format("{0}/screens", *_output_dir));
            makedirs(dir, 0755);
            String path(format("{0}/{1}.png", dir, dec(_ticks, 6)));
            ScopedFd file(open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644));
            write(file, buffer);

            pop_heap(_snapshot_times.begin(), _snapshot_times.end(), greater<int64_t>());
            _snapshot_times.pop_back();
        }
    }
    _ticks = ticks;
}

bool OffscreenVideoDriver::have_snapshots_before(int64_t ticks) const {
    return !_snapshot_times.empty() && (_snapshot_times.front() < ticks);
}

bool OffscreenVideoDriver::is_later(const linked_ptr<Event>& x, const linked_ptr<Event>& y) {
    return x->at() > y->at();
}

}  // namespace antares

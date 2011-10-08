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

#include "cocoa/video-driver.hpp"

#include <stdlib.h>
#include <strings.h>
#include <algorithm>
#include <ApplicationServices/ApplicationServices.h>
#include <OpenGL/OpenGL.h>
#include <OpenGL/gl.h>
#include <sfz/sfz.hpp>

#include "cocoa/c/CocoaVideoDriver.h"
#include "cocoa/core-opengl.hpp"
#include "game/time.hpp"
#include "math/geometry.hpp"
#include "ui/card.hpp"
#include "ui/event.hpp"

using sfz::Exception;
using sfz::scoped_ptr;
using std::min;
using std::queue;

namespace antares {

namespace {

namespace cg {

class DisplayCapturer {
  public:
    DisplayCapturer() {
        CGDisplayErr err = CGCaptureAllDisplays();
        if (err != CGDisplayNoErr) {
            throw Exception("CGCaptureAllDisplays() failed");
            return;
        }
    }

    ~DisplayCapturer() {
        CGReleaseAllDisplays();
    }

  private:
    DISALLOW_COPY_AND_ASSIGN(DisplayCapturer);
};

}  // namespace cg

class MenuBarHider {
  public:
    MenuBarHider() { antares_menu_bar_hide(); }
    ~MenuBarHider() { antares_menu_bar_show(); }

  private:
    DISALLOW_COPY_AND_ASSIGN(MenuBarHider);
};

int64_t usecs() {
    timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000000ll + tv.tv_usec;
}

void enqueue_mouse_down(int button, int32_t x, int32_t y, void* userdata) {
    queue<Event*>* q = reinterpret_cast<queue<Event*>*>(userdata);
    q->push(new MouseDownEvent(now_usecs(), button, Point(x, y)));
}

void enqueue_mouse_up(int button, int32_t x, int32_t y, void* userdata) {
    queue<Event*>* q = reinterpret_cast<queue<Event*>*>(userdata);
    q->push(new MouseUpEvent(now_usecs(), button, Point(x, y)));
}

void enqueue_mouse_move(int32_t x, int32_t y, void* userdata) {
    queue<Event*>* q = reinterpret_cast<queue<Event*>*>(userdata);
    q->push(new MouseMoveEvent(now_usecs(), Point(x, y)));
}

void enqueue_key_down(int32_t key, void* userdata) {
    queue<Event*>* q = reinterpret_cast<queue<Event*>*>(userdata);
    q->push(new KeyDownEvent(now_usecs(), key));
}

void enqueue_key_up(int32_t key, void* userdata) {
    queue<Event*>* q = reinterpret_cast<queue<Event*>*>(userdata);
    q->push(new KeyUpEvent(now_usecs(), key));
}

}  // namespace

CocoaVideoDriver::CocoaVideoDriver(Size screen_size)
        : OpenGlVideoDriver(screen_size),
          _start_time(usecs()),
          _translator(screen_size.width, screen_size.height),
          _event_tracker(false) {
    antares_event_translator_set_mouse_down_callback(
            _translator.c_obj(), enqueue_mouse_down, &_event_queue);
    antares_event_translator_set_mouse_up_callback(
            _translator.c_obj(), enqueue_mouse_up, &_event_queue);
    antares_event_translator_set_mouse_move_callback(
            _translator.c_obj(), enqueue_mouse_move, &_event_queue);
    antares_event_translator_set_key_down_callback(
            _translator.c_obj(), enqueue_key_down, &_event_queue);
    antares_event_translator_set_key_up_callback(
            _translator.c_obj(), enqueue_key_up, &_event_queue);
}

bool CocoaVideoDriver::wait_next_event(int64_t until, scoped_ptr<Event>& event) {
    while (!_event_queue.empty() && _event_queue.front()->at() < until) {
        event.reset(_event_queue.front());
        _event_queue.pop();
        return true;
    }
    until += _start_time;

    antares_event_translator_enqueue(_translator.c_obj(), until);
    while (until > usecs()) {
        if (!_event_queue.empty()) {
            event.reset(_event_queue.front());
            _event_queue.pop();
            return true;
        }
        antares_event_translator_enqueue(_translator.c_obj(), until);
    }

    return false;
}

bool CocoaVideoDriver::button() {
    return _event_tracker.button();
}

Point CocoaVideoDriver::get_mouse() {
    return _event_tracker.mouse();
}

void CocoaVideoDriver::get_keys(KeyMap* keys) {
    keys->copy(_event_tracker.keys());
}

void CocoaVideoDriver::set_game_state(GameState state) {
}

void CocoaVideoDriver::main_loop_iteration_complete(uint32_t) { }

int CocoaVideoDriver::ticks() {
    return (usecs() - _start_time) * 60 / 1000000;
}

int64_t CocoaVideoDriver::double_click_interval_usecs() {
    return antares_double_click_interval_usecs();
}

void CocoaVideoDriver::loop(Card* initial) {
    CGLPixelFormatAttribute attrs[] = {
        kCGLPFADisplayMask, static_cast<CGLPixelFormatAttribute>(
                CGDisplayIDToOpenGLDisplayMask(kCGDirectMainDisplay)),
        kCGLPFAFullScreen,
        kCGLPFAColorSize, static_cast<CGLPixelFormatAttribute>(24),
        kCGLPFAStencilSize, static_cast<CGLPixelFormatAttribute>(8),
        kCGLPFADoubleBuffer,
        kCGLPFAAccelerated,
        static_cast<CGLPixelFormatAttribute>(0),
    };

    cgl::PixelFormat pixel_format(attrs);
    cgl::Context context(pixel_format.c_obj(), NULL);
    cg::DisplayCapturer capturer;

    // TODO(sfiera): control the resolution of the OpenGL context by setting the resolution of
    // the backing store, rather than the screen.  Setting the backing store's resolution
    // independently appears to have only been supported since 10.6, and since we currently
    // target 10.4, we need to control the screen resolution directly for the time being.
    CGDisplaySwitchToMode(kCGDirectMainDisplay, CGDisplayBestModeForParameters(
                kCGDirectMainDisplay, 32, screen_size().width, screen_size().height,
                NULL));

    cgl::check(CGLSetFullScreen(context.c_obj()));
    cgl::check(CGLSetCurrentContext(context.c_obj()));
    MenuBarHider hider;

    GLint swap_interval = 1;
    CGLSetParameter(context.c_obj(), kCGLCPSwapInterval, &swap_interval);

    MainLoop main_loop(*this, initial);
    while (!main_loop.done()) {
        main_loop.draw();
        CGLFlushDrawable(context.c_obj());

        scoped_ptr<Event> event;
        int64_t now = now_usecs();
        while (wait_next_event(now, event)) {
            event->send(&_event_tracker);
            event->send(main_loop.top());
        }

        int64_t at;
        if (main_loop.top()->next_timer(at)) {
            if (wait_next_event(at, event)) {
                event->send(&_event_tracker);
                event->send(main_loop.top());
            } else {
                main_loop.top()->fire_timer();
            }
        } else {
            if (wait_next_event(std::numeric_limits<int64_t>::max(), event)) {
                event->send(&_event_tracker);
                event->send(main_loop.top());
            }
        }
    }
}

}  // namespace antares

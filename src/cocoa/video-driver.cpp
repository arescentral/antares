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

#include "cocoa/video-driver.hpp"

#include <stdlib.h>
#include <strings.h>
#include <algorithm>
#include <ApplicationServices/ApplicationServices.h>
#include <OpenGL/OpenGL.h>
#include <OpenGL/gl.h>
#include <sfz/sfz.hpp>
#include <sys/time.h>

#include "cocoa/c/CocoaVideoDriver.h"
#include "cocoa/core-opengl.hpp"
#include "cocoa/fullscreen.hpp"
#include "cocoa/windowed.hpp"
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

CocoaVideoDriver::CocoaVideoDriver(bool fullscreen, Size screen_size)
        : OpenGlVideoDriver(screen_size),
          _fullscreen(fullscreen),
          _start_time(antares::usecs()),
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
    while (until > antares::usecs()) {
        if (!_event_queue.empty()) {
            event.reset(_event_queue.front());
            _event_queue.pop();
            return true;
        }
        antares_event_translator_enqueue(_translator.c_obj(), until);
    }

    return false;
}

bool CocoaVideoDriver::button(int which) {
    int32_t button;
    antares_get_mouse_button(_translator.c_obj(), &button, which);
    return button;
}

Point CocoaVideoDriver::get_mouse() {
    Point p;
    antares_get_mouse_location(_translator.c_obj(), &p.h, &p.v);
    return p;
}

void CocoaVideoDriver::get_keys(KeyMap* keys) {
    keys->copy(_event_tracker.keys());
}

int CocoaVideoDriver::ticks() const {
    return usecs() * 60 / 1000000;
}

int CocoaVideoDriver::usecs() const {
    return antares::usecs() - _start_time;
}

int64_t CocoaVideoDriver::double_click_interval_usecs() const {
    return antares_double_click_interval_usecs();
}

void CocoaVideoDriver::loop(Card* initial) {
    CGLPixelFormatAttribute attrs[] = {
        kCGLPFADisplayMask, static_cast<CGLPixelFormatAttribute>(
                CGDisplayIDToOpenGLDisplayMask(kCGDirectMainDisplay)),
        kCGLPFAFullScreen,
        kCGLPFAColorSize, static_cast<CGLPixelFormatAttribute>(24),
        kCGLPFADoubleBuffer,
        kCGLPFAAccelerated,
        static_cast<CGLPixelFormatAttribute>(0),
    };

    cgl::PixelFormat pixel_format(attrs);
    cgl::Context context(pixel_format.c_obj(), NULL);
    scoped_ptr<CocoaFullscreen> fullscreen;
    scoped_ptr<CocoaWindowed> windowed;
    if (_fullscreen) {
        fullscreen.reset(new CocoaFullscreen(context, screen_size()));
    } else {
        windowed.reset(new CocoaWindowed(pixel_format, context, screen_size()));
    }
    GLint swap_interval = 1;
    CGLSetParameter(context.c_obj(), kCGLCPSwapInterval, &swap_interval);
    CGLSetCurrentContext(context.c_obj());

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

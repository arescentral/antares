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
using std::min;
using std::unique_ptr;

namespace antares {

namespace {

int64_t usecs() {
    timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000000ll + tv.tv_usec;
}

}  // namespace

CocoaVideoDriver::CocoaVideoDriver(bool fullscreen, Size screen_size)
        : OpenGlVideoDriver(screen_size),
          _fullscreen(fullscreen),
          _start_time(antares::usecs()),
          _translator(screen_size.width, screen_size.height),
          _event_tracker(false) { }

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

struct CocoaVideoDriver::EventBridge {
    EventTracker& event_tracker;
    MainLoop& main_loop;
    cgl::Context& context;

    static void mouse_down(int button, int32_t x, int32_t y, void* userdata) {
        EventBridge* self = reinterpret_cast<EventBridge*>(userdata);
        self->send(new MouseDownEvent(now_usecs(), button, Point(x, y)));
    }

    static void mouse_up(int button, int32_t x, int32_t y, void* userdata) {
        EventBridge* self = reinterpret_cast<EventBridge*>(userdata);
        self->send(new MouseUpEvent(now_usecs(), button, Point(x, y)));
    }

    static void mouse_move(int32_t x, int32_t y, void* userdata) {
        EventBridge* self = reinterpret_cast<EventBridge*>(userdata);
        self->send(new MouseMoveEvent(now_usecs(), Point(x, y)));
    }

    static void key_down(int32_t key, void* userdata) {
        EventBridge* self = reinterpret_cast<EventBridge*>(userdata);
        self->send(new KeyDownEvent(now_usecs(), key));
    }

    static void key_up(int32_t key, void* userdata) {
        EventBridge* self = reinterpret_cast<EventBridge*>(userdata);
        self->send(new KeyUpEvent(now_usecs(), key));
    }

    void send(Event* event) {
        event->send(&event_tracker);
        event->send(main_loop.top());
        main_loop.draw();
        CGLFlushDrawable(context.c_obj());
    }
};

void CocoaVideoDriver::loop(Card* initial) {
    CGLPixelFormatAttribute attrs[] = {
        kCGLPFADisplayMask, static_cast<CGLPixelFormatAttribute>(
                CGDisplayIDToOpenGLDisplayMask(kCGDirectMainDisplay)),
        kCGLPFAColorSize, static_cast<CGLPixelFormatAttribute>(24),
        kCGLPFADoubleBuffer,
        kCGLPFAAccelerated,
        static_cast<CGLPixelFormatAttribute>(0),
    };

    cgl::PixelFormat pixel_format(attrs);
    cgl::Context context(pixel_format.c_obj(), NULL);
    unique_ptr<CocoaFullscreen> fullscreen;
    unique_ptr<CocoaWindowed> windowed;
    if (_fullscreen) {
        fullscreen.reset(new CocoaFullscreen(context, screen_size(), attrs[1]));
    } else {
        windowed.reset(new CocoaWindowed(pixel_format, context, screen_size()));
        antares_event_translator_set_window(_translator.c_obj(), windowed->window());
    }
    GLint swap_interval = 1;
    CGLSetParameter(context.c_obj(), kCGLCPSwapInterval, &swap_interval);
    CGLSetCurrentContext(context.c_obj());

    MainLoop main_loop(*this, initial);
    main_loop.draw();
    CGLFlushDrawable(context.c_obj());
    EventBridge bridge = {_event_tracker, main_loop, context};

    antares_event_translator_set_mouse_down_callback(
            _translator.c_obj(), EventBridge::mouse_down, &bridge);
    antares_event_translator_set_mouse_up_callback(
            _translator.c_obj(), EventBridge::mouse_up, &bridge);
    antares_event_translator_set_mouse_move_callback(
            _translator.c_obj(), EventBridge::mouse_move, &bridge);
    antares_event_translator_set_key_down_callback(
            _translator.c_obj(), EventBridge::key_down, &bridge);
    antares_event_translator_set_key_up_callback(
            _translator.c_obj(), EventBridge::key_up, &bridge);

    while (!main_loop.done()) {
        int64_t now = usecs();
        while (antares_event_translator_next(_translator.c_obj(), now)) {
            continue;
        }

        int64_t at;
        if (main_loop.top()->next_timer(at)) {
            at += _start_time;
            if (!antares_event_translator_next(_translator.c_obj(), at)) {
                main_loop.top()->fire_timer();
                main_loop.draw();
                CGLFlushDrawable(context.c_obj());
            }
        } else {
            at = std::numeric_limits<int64_t>::max();
            antares_event_translator_next(_translator.c_obj(), at);
        }
    }
}

}  // namespace antares

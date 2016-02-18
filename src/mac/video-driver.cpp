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

#include "mac/video-driver.hpp"

#include <stdlib.h>
#include <strings.h>
#include <algorithm>
#include <ApplicationServices/ApplicationServices.h>
#include <IOKit/hid/IOHIDLib.h>
#include <OpenGL/OpenGL.h>
#include <OpenGL/gl.h>
#include <sfz/sfz.hpp>
#include <sys/time.h>

#include "game/time.hpp"
#include "mac/c/CocoaVideoDriver.h"
#include "mac/core-opengl.hpp"
#include "mac/core-foundation.hpp"
#include "mac/fullscreen.hpp"
#include "mac/windowed.hpp"
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
        : _screen_size(screen_size),
          _fullscreen(fullscreen),
          _start_time(antares::usecs()),
          _translator(screen_size.width, screen_size.height),
          _event_tracker(false) { }

Point CocoaVideoDriver::get_mouse() {
    Point p;
    antares_get_mouse_location(_translator.c_obj(), &p.h, &p.v);
    return p;
}

void CocoaVideoDriver::get_keys(KeyMap* keys) {
    keys->copy(_event_tracker.keys());
}

InputMode CocoaVideoDriver::input_mode() const {
    return _event_tracker.input_mode();
}

int CocoaVideoDriver::ticks() const {
    return usecs() * 60 / 1000000;
}

int CocoaVideoDriver::usecs() const {
    return antares::usecs() - _start_time;
}

struct CocoaVideoDriver::EventBridge {
    EventTracker& event_tracker;
    MainLoop& main_loop;
    cgl::Context& context;
    EventTranslator& translator;
    std::queue<Event*> event_queue;

    double gamepad[6];

    static void mouse_down(int button, int32_t x, int32_t y, int count, void* userdata) {
        EventBridge* self = reinterpret_cast<EventBridge*>(userdata);
        self->enqueue(new MouseDownEvent(now_usecs(), button, count, Point(x, y)));
    }

    static void mouse_up(int button, int32_t x, int32_t y, void* userdata) {
        EventBridge* self = reinterpret_cast<EventBridge*>(userdata);
        self->enqueue(new MouseUpEvent(now_usecs(), button, Point(x, y)));
    }

    static void mouse_move(int32_t x, int32_t y, void* userdata) {
        EventBridge* self = reinterpret_cast<EventBridge*>(userdata);
        self->enqueue(new MouseMoveEvent(now_usecs(), Point(x, y)));
    }

    static void caps_lock(void* userdata) {
        EventBridge* self = reinterpret_cast<EventBridge*>(userdata);
        self->enqueue(new KeyDownEvent(now_usecs(), Keys::CAPS_LOCK));
    }

    static void caps_unlock(void* userdata) {
        EventBridge* self = reinterpret_cast<EventBridge*>(userdata);
        self->enqueue(new KeyUpEvent(now_usecs(), Keys::CAPS_LOCK));
    }

    static void hid_event(void* userdata, IOReturn result, void* sender, IOHIDValueRef value) {
        EventBridge* self = reinterpret_cast<EventBridge*>(userdata);
        IOHIDElementRef element = IOHIDValueGetElement(value);
        uint32_t usage_page = IOHIDElementGetUsagePage(element);
        switch (usage_page) {
          case kHIDPage_KeyboardOrKeypad:
            self->key_event(result, element, value);
            break;
          case kHIDPage_GenericDesktop:
            self->analog_event(result, element, value);
            break;
          case kHIDPage_Button:
            self->button_event(result, element, value);
            break;
          default:
            sfz::print(sfz::io::err, sfz::format("{0}\n", usage_page));
            break;
        }
    }

    void key_event(IOReturn result, IOHIDElementRef element, IOHIDValueRef value) {
        if (!antares_is_active()) {
            return;
        }
        bool down = IOHIDValueGetIntegerValue(value);
        uint16_t scan_code = IOHIDElementGetUsage(element);
        if ((scan_code < 4) || (231 < scan_code)) {
            return;
        } else if (scan_code == Keys::CAPS_LOCK) {
            return;
        }

        if (down) {
            enqueue(new KeyDownEvent(now_usecs(), scan_code));
        } else {
            enqueue(new KeyUpEvent(now_usecs(), scan_code));
        }
    }

    void button_event(IOReturn result, IOHIDElementRef element, IOHIDValueRef value) {
        if (!antares_is_active()) {
            return;
        }
        bool down = IOHIDValueGetIntegerValue(value);
        uint16_t usage = IOHIDElementGetUsage(element);
        if (down) {
            enqueue(new GamepadButtonDownEvent(now_usecs(), usage));
        } else {
            enqueue(new GamepadButtonUpEvent(now_usecs(), usage));
        }
    }

    void analog_event(IOReturn result, IOHIDElementRef element, IOHIDValueRef value) {
        int int_value = IOHIDValueGetIntegerValue(value);
        uint16_t usage = IOHIDElementGetUsage(element);
        switch (usage) {
          case kHIDUsage_GD_X:
          case kHIDUsage_GD_Y:
          case kHIDUsage_GD_Rx:
          case kHIDUsage_GD_Ry:
            {
                int min = IOHIDElementGetLogicalMin(element);
                int max = IOHIDElementGetLogicalMax(element);
                double double_value = int_value;
                if (int_value < 0) {
                    double_value = -(double_value / min);
                } else {
                    double_value = (double_value / max);
                }

                usage -= kHIDUsage_GD_X;
                gamepad[usage] = double_value;
                static const int x_component[] = {0, 0, -1, 3, 3, -1};
                double x = gamepad[x_component[usage]];
                double y = gamepad[x_component[usage] + 1];
                enqueue(new GamepadStickEvent(
                            now_usecs(), kHIDUsage_GD_X + x_component[usage], x, y));
            }
            break;
          case kHIDUsage_GD_Z:
          case kHIDUsage_GD_Rz:
            button_event(result, element, value);
            break;
        }
    }

    void enqueue(Event* event) {
        event_queue.emplace(event);
        antares_event_translator_cancel(translator.c_obj());
    }

    void send_all() {
        if (event_queue.empty()) {
            return;
        }
        while (!event_queue.empty()) {
            event_queue.front()->send(&event_tracker);
            event_queue.front()->send(main_loop.top());
            event_queue.pop();
        }
        main_loop.draw();
        CGLFlushDrawable(context.c_obj());
    }
};

void CocoaVideoDriver::loop(Card* initial) {
    CGLPixelFormatAttribute attrs[] = {
        kCGLPFAOpenGLProfile, (CGLPixelFormatAttribute)kCGLOGLPVersion_3_2_Core,
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
        fullscreen.reset(new CocoaFullscreen(pixel_format, context, _screen_size));
        antares_event_translator_set_window(_translator.c_obj(), fullscreen->window());
        _viewport_size = fullscreen->viewport_size();
    } else {
        windowed.reset(new CocoaWindowed(pixel_format, context, _screen_size, false, true));
        antares_event_translator_set_window(_translator.c_obj(), windowed->window());
        _viewport_size = windowed->viewport_size();
    }
    GLint swap_interval = 1;
    CGLSetParameter(context.c_obj(), kCGLCPSwapInterval, &swap_interval);
    CGLSetCurrentContext(context.c_obj());

    MainLoop main_loop(*this, initial);
    main_loop.draw();
    CGLFlushDrawable(context.c_obj());
    EventBridge bridge = {_event_tracker, main_loop, context, _translator};

    antares_event_translator_set_mouse_down_callback(
            _translator.c_obj(), EventBridge::mouse_down, &bridge);
    antares_event_translator_set_mouse_up_callback(
            _translator.c_obj(), EventBridge::mouse_up, &bridge);
    antares_event_translator_set_mouse_move_callback(
            _translator.c_obj(), EventBridge::mouse_move, &bridge);
    antares_event_translator_set_caps_lock_callback(
            _translator.c_obj(), EventBridge::caps_lock, &bridge);
    antares_event_translator_set_caps_unlock_callback(
            _translator.c_obj(), EventBridge::caps_unlock, &bridge);

    cf::MutableDictionary keyboard(CFDictionaryCreateMutable(
                NULL, 0,
                &kCFCopyStringDictionaryKeyCallBacks,
                &kCFTypeDictionaryValueCallBacks));
    keyboard.set(CFSTR(kIOHIDDeviceUsagePageKey), cf::wrap(kHIDPage_GenericDesktop).c_obj());
    keyboard.set(CFSTR(kIOHIDDeviceUsageKey), cf::wrap(kHIDUsage_GD_Keyboard).c_obj());
    cf::MutableDictionary gamepad(CFDictionaryCreateMutable(
                NULL, 0,
                &kCFCopyStringDictionaryKeyCallBacks,
                &kCFTypeDictionaryValueCallBacks));
    gamepad.set(CFSTR(kIOHIDDeviceUsagePageKey), cf::wrap(kHIDPage_GenericDesktop).c_obj());
    gamepad.set(CFSTR(kIOHIDDeviceUsageKey), cf::wrap(kHIDUsage_GD_GamePad).c_obj());
    cf::MutableArray criteria(CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks));
    criteria.append(keyboard.c_obj());
    criteria.append(gamepad.c_obj());

    auto hid_manager = IOHIDManagerCreate(kCFAllocatorDefault, kIOHIDOptionsTypeNone);
    IOHIDManagerSetDeviceMatchingMultiple(hid_manager, criteria.c_obj());
    IOHIDManagerScheduleWithRunLoop(hid_manager, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
    IOReturn r = IOHIDManagerOpen(hid_manager, kIOHIDOptionsTypeNone);
    if (r != 0) {
        throw Exception("IOHIDManagerOpen");
    }
    IOHIDManagerRegisterInputValueCallback(hid_manager, EventBridge::hid_event, &bridge);

    while (!main_loop.done()) {
        int64_t at;
        if (main_loop.top()->next_timer(at)) {
            at += _start_time;
            if (antares_event_translator_next(_translator.c_obj(), at)) {
                bridge.send_all();
            } else {
                main_loop.top()->fire_timer();
                main_loop.draw();
                CGLFlushDrawable(context.c_obj());
            }
        } else {
            at = std::numeric_limits<int64_t>::max();
            antares_event_translator_next(_translator.c_obj(), at);
            bridge.send_all();
        }
    }
}

}  // namespace antares

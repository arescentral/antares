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
#include <IOKit/hid/IOHIDLib.h>
#include <OpenGL/OpenGL.h>
#include <OpenGL/gl.h>
#include <sfz/sfz.hpp>
#include <sys/time.h>

#include "cocoa/c/CocoaVideoDriver.h"
#include "cocoa/core-opengl.hpp"
#include "cocoa/core-foundation.hpp"
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
    EventTranslator& translator;

    double gamepad[6];

    static void mouse_down(int button, int32_t x, int32_t y, void* userdata) {
        EventBridge* self = reinterpret_cast<EventBridge*>(userdata);
        self->send(MouseDownEvent(now_usecs(), button, Point(x, y)));
    }

    static void mouse_up(int button, int32_t x, int32_t y, void* userdata) {
        EventBridge* self = reinterpret_cast<EventBridge*>(userdata);
        self->send(MouseUpEvent(now_usecs(), button, Point(x, y)));
    }

    static void mouse_move(int32_t x, int32_t y, void* userdata) {
        EventBridge* self = reinterpret_cast<EventBridge*>(userdata);
        self->send(MouseMoveEvent(now_usecs(), Point(x, y)));
    }

    static void caps_lock(void* userdata) {
        EventBridge* self = reinterpret_cast<EventBridge*>(userdata);
        self->send(CapsLockEvent(now_usecs()));
    }

    static void caps_unlock(void* userdata) {
        EventBridge* self = reinterpret_cast<EventBridge*>(userdata);
        self->send(CapsUnlockEvent(now_usecs()));
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
            send(KeyDownEvent(now_usecs(), scan_code));
        } else {
            send(KeyUpEvent(now_usecs(), scan_code));
        }
        antares_event_translator_cancel(translator.c_obj());
    }

    void button_event(IOReturn result, IOHIDElementRef element, IOHIDValueRef value) {
        if (!antares_is_active()) {
            return;
        }
        bool down = IOHIDValueGetIntegerValue(value);
        uint16_t usage = IOHIDElementGetUsage(element);
        if (down) {
            send(GamepadButtonDownEvent(now_usecs(), usage));
        } else {
            send(GamepadButtonUpEvent(now_usecs(), usage));
        }
        antares_event_translator_cancel(translator.c_obj());
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
                static int x_component[] = {0, 0, -1, 3, 3, -1};
                double x = gamepad[x_component[usage]];
                double y = gamepad[x_component[usage] + 1];
                send(GamepadStickEvent(now_usecs(), kHIDUsage_GD_X + x_component[usage], x, y));
            }
            break;
          case kHIDUsage_GD_Z:
          case kHIDUsage_GD_Rz:
            button_event(result, element, value);
            break;
        }
    }

    void send(const Event& event) {
        event.send(&event_tracker);
        event.send(main_loop.top());
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

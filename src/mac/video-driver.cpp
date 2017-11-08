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

#include "mac/video-driver.hpp"

#include <ApplicationServices/ApplicationServices.h>
#include <IOKit/hid/IOHIDLib.h>
#include <OpenGL/OpenGL.h>
#include <OpenGL/gl.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/time.h>
#include <algorithm>
#include <sfz/sfz.hpp>

#include "game/time.hpp"
#include "mac/c/CocoaVideoDriver.h"
#include "mac/core-foundation.hpp"
#include "mac/core-opengl.hpp"
#include "math/geometry.hpp"
#include "ui/card.hpp"
#include "ui/event.hpp"

using sfz::Exception;
using std::min;
using std::unique_ptr;

namespace antares {

namespace {

class AntaresWindow {
  public:
    AntaresWindow(const cgl::PixelFormat& pixel_format, const cgl::Context& context)
            : _c_obj(antares_window_create(pixel_format.c_obj(), context.c_obj())) {}

    ~AntaresWindow() { antares_window_destroy(_c_obj); }

    ::AntaresWindow* c_obj() const { return _c_obj; }

  private:
    ::AntaresWindow* _c_obj;
};

class InputModeTracker : public EventReceiver {
  public:
    InputModeTracker(InputMode* mode) : _mode(mode) {}

    virtual void key_down(const KeyDownEvent&) { *_mode = KEYBOARD_MOUSE; }
    virtual void mouse_down(const MouseDownEvent&) { *_mode = KEYBOARD_MOUSE; }
    virtual void mouse_move(const MouseMoveEvent&) { *_mode = KEYBOARD_MOUSE; }
    virtual void gamepad_button_down(const GamepadButtonDownEvent&) { *_mode = GAMEPAD; }
    virtual void gamepad_stick(const GamepadStickEvent&) { *_mode = GAMEPAD; }

  private:
    InputMode* _mode;

    DISALLOW_COPY_AND_ASSIGN(InputModeTracker);
};

}  // namespace

CocoaVideoDriver::CocoaVideoDriver() {}

Size CocoaVideoDriver::viewport_size() const {
    return {
            antares_window_viewport_width(_window), antares_window_viewport_height(_window),
    };
}

Size CocoaVideoDriver::screen_size() const {
    return {
            antares_window_screen_width(_window), antares_window_screen_height(_window),
    };
}

Point CocoaVideoDriver::get_mouse() {
    Point p;
    antares_get_mouse_location(_translator.c_obj(), &p.h, &p.v);
    return p;
}

InputMode CocoaVideoDriver::input_mode() const {
    return _input_mode;
}

wall_time CocoaVideoDriver::now() const {
    return _now();
}

struct CocoaVideoDriver::EventBridge {
    InputMode&         input_mode;
    MainLoop&          main_loop;
    cgl::Context&      context;
    EventTranslator&   translator;
    std::queue<Event*> event_queue;

    double gamepad[6];

    static void mouse_down(int button, int32_t x, int32_t y, int count, void* userdata) {
        EventBridge* self = reinterpret_cast<EventBridge*>(userdata);
        self->enqueue(new MouseDownEvent(_now(), button, count, Point(x, y)));
    }

    static void mouse_up(int button, int32_t x, int32_t y, void* userdata) {
        EventBridge* self = reinterpret_cast<EventBridge*>(userdata);
        self->enqueue(new MouseUpEvent(_now(), button, Point(x, y)));
    }

    static void mouse_move(int32_t x, int32_t y, void* userdata) {
        EventBridge* self = reinterpret_cast<EventBridge*>(userdata);
        self->enqueue(new MouseMoveEvent(_now(), Point(x, y)));
    }

    static void caps_lock(void* userdata) {
        EventBridge* self = reinterpret_cast<EventBridge*>(userdata);
        self->enqueue(new KeyDownEvent(_now(), Keys::CAPS_LOCK));
    }

    static void caps_unlock(void* userdata) {
        EventBridge* self = reinterpret_cast<EventBridge*>(userdata);
        self->enqueue(new KeyUpEvent(_now(), Keys::CAPS_LOCK));
    }

    static void hid_event(void* userdata, IOReturn result, void* sender, IOHIDValueRef value) {
        EventBridge*    self       = reinterpret_cast<EventBridge*>(userdata);
        IOHIDElementRef element    = IOHIDValueGetElement(value);
        uint32_t        usage_page = IOHIDElementGetUsagePage(element);
        switch (usage_page) {
            case kHIDPage_KeyboardOrKeypad: self->key_event(result, element, value); break;
            case kHIDPage_GenericDesktop: self->analog_event(result, element, value); break;
            case kHIDPage_Button: self->button_event(result, element, value); break;
            default: sfz::print(sfz::io::err, sfz::format("{0}\n", usage_page)); break;
        }
    }

    void key_event(IOReturn result, IOHIDElementRef element, IOHIDValueRef value) {
        if (!antares_is_active()) {
            return;
        }
        bool     down      = IOHIDValueGetIntegerValue(value);
        uint16_t scan_code = IOHIDElementGetUsage(element);
        if ((scan_code < 4) || (231 < scan_code)) {
            return;
        } else if (scan_code == Keys::CAPS_LOCK) {
            return;
        }

        if (down) {
            enqueue(new KeyDownEvent(_now(), scan_code));
        } else {
            enqueue(new KeyUpEvent(_now(), scan_code));
        }
    }

    void button_event(IOReturn result, IOHIDElementRef element, IOHIDValueRef value) {
        if (!antares_is_active()) {
            return;
        }
        bool     down  = IOHIDValueGetIntegerValue(value);
        uint16_t usage = IOHIDElementGetUsage(element);
        if (down) {
            enqueue(new GamepadButtonDownEvent(_now(), usage));
        } else {
            enqueue(new GamepadButtonUpEvent(_now(), usage));
        }
    }

    void analog_event(IOReturn result, IOHIDElementRef element, IOHIDValueRef value) {
        int      int_value = IOHIDValueGetIntegerValue(value);
        uint16_t usage     = IOHIDElementGetUsage(element);
        switch (usage) {
            case kHIDUsage_GD_X:
            case kHIDUsage_GD_Y:
            case kHIDUsage_GD_Rx:
            case kHIDUsage_GD_Ry: {
                int    min          = IOHIDElementGetLogicalMin(element);
                int    max          = IOHIDElementGetLogicalMax(element);
                double double_value = int_value;
                if (int_value < 0) {
                    double_value = -(double_value / min);
                } else {
                    double_value = (double_value / max);
                }

                usage -= kHIDUsage_GD_X;
                gamepad[usage]                 = double_value;
                static const int x_component[] = {0, 0, -1, 3, 3, -1};
                double           x             = gamepad[x_component[usage]];
                double           y             = gamepad[x_component[usage] + 1];
                enqueue(new GamepadStickEvent(_now(), kHIDUsage_GD_X + x_component[usage], x, y));
            } break;
            case kHIDUsage_GD_Z:
            case kHIDUsage_GD_Rz: button_event(result, element, value); break;
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
            InputModeTracker tracker(&input_mode);
            event_queue.front()->send(&tracker);
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
            kCGLPFAColorSize, static_cast<CGLPixelFormatAttribute>(24), kCGLPFADoubleBuffer,
            kCGLPFAAccelerated, static_cast<CGLPixelFormatAttribute>(0),
    };

    cgl::PixelFormat pixel_format(attrs);
    cgl::Context     context(pixel_format.c_obj(), NULL);
    AntaresWindow    window(pixel_format, context);
    _window = window.c_obj();
    antares_event_translator_set_window(_translator.c_obj(), window.c_obj());
    GLint swap_interval = 1;
    CGLSetParameter(context.c_obj(), kCGLCPSwapInterval, &swap_interval);
    CGLSetCurrentContext(context.c_obj());

    MainLoop main_loop(*this, initial);
    main_loop.draw();
    CGLFlushDrawable(context.c_obj());
    EventBridge bridge = {_input_mode, main_loop, context, _translator};

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
            NULL, 0, &kCFCopyStringDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks));
    keyboard.set(CFSTR(kIOHIDDeviceUsagePageKey), cf::wrap(kHIDPage_GenericDesktop).c_obj());
    keyboard.set(CFSTR(kIOHIDDeviceUsageKey), cf::wrap(kHIDUsage_GD_Keyboard).c_obj());
    cf::MutableDictionary gamepad(CFDictionaryCreateMutable(
            NULL, 0, &kCFCopyStringDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks));
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
        wall_time at;
        if (main_loop.top()->next_timer(at)) {
            if (antares_event_translator_next(
                        _translator.c_obj(), at.time_since_epoch().count())) {
                bridge.send_all();
            } else {
                main_loop.top()->fire_timer();
                main_loop.draw();
                CGLFlushDrawable(context.c_obj());
            }
        } else {
            at = wall_time::max();
            antares_event_translator_next(_translator.c_obj(), at.time_since_epoch().count());
            bridge.send_all();
        }
    }
}

wall_time CocoaVideoDriver::_now() {
    timeval tv;
    gettimeofday(&tv, NULL);
    return wall_time(usecs(tv.tv_sec * 1000000ll + tv.tv_usec));
}

}  // namespace antares

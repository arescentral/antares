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
#include <pn/output>

#include "game/time.hpp"
#include "mac/c/CocoaVideoDriver.h"
#include "mac/core-foundation.hpp"
#include "mac/core-opengl.hpp"
#include "math/geometry.hpp"
#include "ui/card.hpp"
#include "ui/event.hpp"

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
};

}  // namespace

static int32_t get_iohid_property(IOHIDDeviceRef device, CFStringRef key) {
    int32_t    vendor      = 0;
    cf::Type   vendor_type = CFRetain(IOHIDDeviceGetProperty(device, key));
    cf::Number vendor_number;
    if (vendor_type.c_obj() &&
        (vendor_number = cf::cast<cf::Number>(std::move(vendor_type))).c_obj() &&
        cf::unwrap(vendor_number, vendor)) {
        return vendor;
    }
    return 0;
}

CocoaVideoDriver::CocoaVideoDriver() {}

Size CocoaVideoDriver::viewport_size() const {
    return {
            antares_window_viewport_width(_window),
            antares_window_viewport_height(_window),
    };
}

Size CocoaVideoDriver::screen_size() const {
    return {
            antares_window_screen_width(_window),
            antares_window_screen_height(_window),
    };
}

Point CocoaVideoDriver::get_mouse() {
    Point p;
    antares_get_mouse_location(_window, &p.h, &p.v);
    return p;
}

InputMode CocoaVideoDriver::input_mode() const { return _input_mode; }

static antares_window_callback_range c_range(TextReceiver::range<int> range) {
    return {range.begin, range.end};
}

wall_time CocoaVideoDriver::now() const { return _now(); }

static const int key_code_count            = 0x80;
static const Key key_codes[key_code_count] = {
        Key::A,         Key::S,          Key::D,           Key::F,          Key::H,
        Key::G,         Key::Z,          Key::X,           Key::C,          Key::V,
        Key::NONE,      Key::B,          Key::Q,           Key::W,          Key::E,
        Key::R,

        Key::Y,         Key::T,          Key::K1,          Key::K2,         Key::K3,
        Key::K4,        Key::K6,         Key::K5,          Key::EQUALS,     Key::K9,
        Key::K7,        Key::MINUS,      Key::K8,          Key::K0,         Key::R_BRACKET,
        Key::O,

        Key::U,         Key::L_BRACKET,  Key::I,           Key::P,          Key::RETURN,
        Key::L,         Key::J,          Key::QUOTE,       Key::K,          Key::SEMICOLON,
        Key::BACKSLASH, Key::COMMA,      Key::SLASH,       Key::N,          Key::M,
        Key::PERIOD,

        Key::TAB,       Key::SPACE,      Key::BACKTICK,    Key::BACKSPACE,  Key::NONE,
        Key::ESCAPE,    Key::R_COMMAND,  Key::COMMAND,     Key::SHIFT,      Key::CAPS_LOCK,
        Key::OPTION,    Key::CONTROL,    Key::R_SHIFT,     Key::R_OPTION,   Key::R_CONTROL,
        Key::NONE,

        Key::F17,       Key::N_PERIOD,   Key::NONE,        Key::N_TIMES,    Key::NONE,
        Key::N_PLUS,    Key::NONE,       Key::N_CLEAR,     Key::VOL_UP,     Key::VOL_DOWN,
        Key::MUTE,      Key::N_DIVIDE,   Key::N_ENTER,     Key::NONE,       Key::N_MINUS,
        Key::F18,

        Key::F19,       Key::N_EQUALS,   Key::K0,          Key::K1,         Key::K2,
        Key::K3,        Key::K4,         Key::K5,          Key::K6,         Key::K7,
        Key::F20,       Key::K8,         Key::K9,          Key::J_YEN,      Key::J_UNDERSCORE,
        Key::K_COMMA,

        Key::F5,        Key::F6,         Key::F7,          Key::F3,         Key::F8,
        Key::F9,        Key::NONE,       Key::F11,         Key::J_KANA,     Key::F13,
        Key::F16,       Key::F14,        Key::NONE,        Key::F10,        Key::NONE,
        Key::F12,

        Key::NONE,      Key::F15,        Key::HELP,        Key::HOME,       Key::PAGE_UP,
        Key::DELETE,    Key::F4,         Key::END,         Key::F2,         Key::PAGE_DOWN,
        Key::F1,        Key::LEFT_ARROW, Key::RIGHT_ARROW, Key::DOWN_ARROW, Key::UP_ARROW,
        Key::NONE,
};

struct CocoaVideoDriver::EventBridge {
    InputMode&         input_mode;
    MainLoop&          main_loop;
    cgl::Context&      context;
    AntaresWindow&     window;
    std::queue<Event*> event_queue;
    TextReceiver*      text_receiver;

    double gamepad[6];
    bool   switch_dpad[4];

    static void text(
            antares_window_callback_type type, antares_window_callback_data data, void* userdata) {
        EventBridge* self = reinterpret_cast<EventBridge*>(userdata);

        switch (type) {
            case ANTARES_WINDOW_CALLBACK_KEY_DOWN:
                self->enqueue(new KeyDownEvent(_now(), key_codes[data.key_down]));
                break;

            case ANTARES_WINDOW_CALLBACK_KEY_UP:
                self->enqueue(new KeyUpEvent(_now(), key_codes[data.key_up]));
                break;

            case ANTARES_WINDOW_CALLBACK_CAPS_LOCK:
                self->enqueue(new KeyDownEvent(_now(), Key::CAPS_LOCK));
                break;

            case ANTARES_WINDOW_CALLBACK_CAPS_UNLOCK:
                self->enqueue(new KeyUpEvent(_now(), Key::CAPS_LOCK));
                break;

            case ANTARES_WINDOW_CALLBACK_MOUSE_DOWN:
                self->enqueue(new MouseDownEvent(
                        _now(), data.mouse_down.button, data.mouse_down.count,
                        Point(data.mouse_down.x, data.mouse_down.y)));
                break;

            case ANTARES_WINDOW_CALLBACK_MOUSE_UP:
                self->enqueue(new MouseUpEvent(
                        _now(), data.mouse_up.button, Point(data.mouse_up.x, data.mouse_up.y)));
                break;

            case ANTARES_WINDOW_CALLBACK_MOUSE_MOVE:
                self->enqueue(
                        new MouseMoveEvent(_now(), Point(data.mouse_move.x, data.mouse_move.y)));
                break;

            case ANTARES_WINDOW_CALLBACK_REPLACE:
                if (self->text_receiver) {
                    self->text_receiver->replace(
                            {data.replace.range.begin, data.replace.range.end},
                            pn::string_view(data.replace.data, data.replace.size));
                }
                break;

            case ANTARES_WINDOW_CALLBACK_SELECT:
                if (self->text_receiver) {
                    self->text_receiver->select({data.select.begin, data.select.end});
                }
                break;
            case ANTARES_WINDOW_CALLBACK_MARK:
                if (self->text_receiver) {
                    self->text_receiver->mark({data.mark.begin, data.mark.end});
                }
                break;

            case ANTARES_WINDOW_CALLBACK_ACCEPT:
                if (self->text_receiver) {
                    self->text_receiver->accept();
                }
                break;

            case ANTARES_WINDOW_CALLBACK_NEWLINE:
                if (self->text_receiver) {
                    self->text_receiver->newline();
                }
                break;

            case ANTARES_WINDOW_CALLBACK_TAB:
                if (self->text_receiver) {
                    self->text_receiver->tab();
                }
                break;

            case ANTARES_WINDOW_CALLBACK_ESCAPE:
                if (self->text_receiver) {
                    self->text_receiver->escape();
                }
                break;

            case ANTARES_WINDOW_CALLBACK_GET_EDITING:
                *data.get_editing = (self->text_receiver != nullptr);
                break;

            case ANTARES_WINDOW_CALLBACK_GET_OFFSET:
                if (self->text_receiver) {
                    *data.get_offset.offset = self->text_receiver->offset(
                            data.get_offset.origin, data.get_offset.by,
                            (TextReceiver::OffsetUnit)data.get_offset.unit);
                }
                break;

            case ANTARES_WINDOW_CALLBACK_GET_SIZE:
                if (self->text_receiver) {
                    *data.get_size = self->text_receiver->size();
                }
                break;

            case ANTARES_WINDOW_CALLBACK_GET_SELECTION:
                if (self->text_receiver) {
                    *data.get_selection = c_range(self->text_receiver->selection());
                }
                break;

            case ANTARES_WINDOW_CALLBACK_GET_MARK:
                if (self->text_receiver) {
                    *data.get_mark = c_range(self->text_receiver->mark());
                }
                break;

            case ANTARES_WINDOW_CALLBACK_GET_TEXT:
                if (self->text_receiver) {
                    pn::string_view s = self->text_receiver->text(
                            {data.get_text.range.begin, data.get_text.range.end});
                    *data.get_text.data = s.data();
                    *data.get_text.size = s.size();
                }
                break;
        }
    }

    static void hid_event(void* userdata, IOReturn result, void* sender, IOHIDValueRef value) {
        EventBridge*    self       = reinterpret_cast<EventBridge*>(userdata);
        IOHIDElementRef element    = IOHIDValueGetElement(value);
        uint32_t        usage_page = IOHIDElementGetUsagePage(element);
        switch (usage_page) {
            case kHIDPage_GenericDesktop: self->analog_event(result, element, value); break;
            case kHIDPage_Button: self->button_event(result, element, value); break;
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
        }
        Key usb_key = static_cast<Key>(scan_code);
        if (usb_key == Key::CAPS_LOCK) {
            return;
        }

        if (down) {
            enqueue(new KeyDownEvent(_now(), usb_key));
        } else {
            enqueue(new KeyUpEvent(_now(), usb_key));
        }
    }

    void button_event(IOReturn result, IOHIDElementRef element, IOHIDValueRef value) {
        if (!antares_is_active()) {
            return;
        }

        IOHIDDeviceRef device  = IOHIDElementGetDevice(element);
        int32_t        vendor  = get_iohid_property(device, CFSTR(kIOHIDVendorIDKey));
        int32_t        product = get_iohid_property(device, CFSTR(kIOHIDProductIDKey));
        switch (vendor) {
            case 0x057e:  // Nintendo
                switch (product) {
                    case 0x2006:  // Joy-Con (L)
                        return joy_con_l_button_event(element, value);
                    case 0x2007:  // Joy-Con (R)
                        return joy_con_r_button_event(element, value);
                    case 0x2009:  // Switch Pro controller
                        return switch_pro_button_event(element, value);
                }
        }
        default_button_event(element, value);
    }

    void analog_event(IOReturn result, IOHIDElementRef element, IOHIDValueRef value) {
        if (!antares_is_active()) {
            return;
        }

        IOHIDDeviceRef device  = IOHIDElementGetDevice(element);
        int32_t        vendor  = get_iohid_property(device, CFSTR(kIOHIDVendorIDKey));
        int32_t        product = get_iohid_property(device, CFSTR(kIOHIDProductIDKey));
        switch (vendor) {
            case 0x057e:  // Nintendo
                switch (product) {
                    case 0x2006:  // Joy-Con (L)
                        return joy_con_l_analog_event(element, value);
                    case 0x2007:  // Joy-Con (R)
                        return joy_con_r_analog_event(element, value);
                    case 0x2009:  // Switch Pro controller
                        return switch_pro_analog_event(element, value);
                }
        }
        default_analog_event(element, value);
    }

    void joy_con_l_button_event(IOHIDElementRef element, IOHIDValueRef value) {
        bool            down   = IOHIDValueGetIntegerValue(value);
        uint16_t        usage  = IOHIDElementGetUsage(element);
        Gamepad::Button button = Gamepad::Button::NONE;

        switch (usage) {
            case 1: button = Gamepad::Button::LEFT; break;
            case 2: button = Gamepad::Button::DOWN; break;
            case 3: button = Gamepad::Button::UP; break;
            case 4: button = Gamepad::Button::RIGHT; break;
            case 9: button = Gamepad::Button::BACK; break;
            case 11: button = Gamepad::Button::LSB; break;
            case 14: button = Gamepad::Button::NONE; break;
            case 15: button = Gamepad::Button::LB; break;
            case 16: button = Gamepad::Button::LT; break;
        }

        if (button == Gamepad::Button::NONE) {
            return;
        } else if (down) {
            enqueue(new GamepadButtonDownEvent(_now(), button));
        } else {
            enqueue(new GamepadButtonUpEvent(_now(), button));
        }
    }

    void joy_con_r_button_event(IOHIDElementRef element, IOHIDValueRef value) {
        bool            down   = IOHIDValueGetIntegerValue(value);
        uint16_t        usage  = IOHIDElementGetUsage(element);
        Gamepad::Button button = Gamepad::Button::NONE;

        switch (usage) {
            case 1: button = Gamepad::Button::A; break;
            case 2: button = Gamepad::Button::X; break;
            case 3: button = Gamepad::Button::B; break;
            case 4: button = Gamepad::Button::Y; break;
            case 10: button = Gamepad::Button::START; break;
            case 12: button = Gamepad::Button::RSB; break;
            case 13: button = Gamepad::Button::NONE; break;
            case 15: button = Gamepad::Button::RB; break;
            case 16: button = Gamepad::Button::RT; break;
        }

        if (button == Gamepad::Button::NONE) {
            return;
        } else if (down) {
            enqueue(new GamepadButtonDownEvent(_now(), button));
        } else {
            enqueue(new GamepadButtonUpEvent(_now(), button));
        }
    }

    void joy_con_l_analog_event(IOHIDElementRef element, IOHIDValueRef value) {
        using Stick       = Gamepad::Stick;
        int64_t int_value = IOHIDValueGetIntegerValue(value);
        switch (int_value) {
            case 0: enqueue(new GamepadStickEvent(_now(), Stick::LS, +1.000, +0.000)); break;
            case 1: enqueue(new GamepadStickEvent(_now(), Stick::LS, +0.707, +0.707)); break;
            case 2: enqueue(new GamepadStickEvent(_now(), Stick::LS, +0.000, +1.000)); break;
            case 3: enqueue(new GamepadStickEvent(_now(), Stick::LS, -0.707, +0.707)); break;
            case 4: enqueue(new GamepadStickEvent(_now(), Stick::LS, -1.000, +0.000)); break;
            case 5: enqueue(new GamepadStickEvent(_now(), Stick::LS, -0.707, -0.707)); break;
            case 6: enqueue(new GamepadStickEvent(_now(), Stick::LS, +0.000, -1.000)); break;
            case 7: enqueue(new GamepadStickEvent(_now(), Stick::LS, +0.707, -0.707)); break;
            case 8: enqueue(new GamepadStickEvent(_now(), Stick::LS, 0.000, 0.000)); break;
        }
    }

    void joy_con_r_analog_event(IOHIDElementRef element, IOHIDValueRef value) {
        using Stick       = Gamepad::Stick;
        int64_t int_value = IOHIDValueGetIntegerValue(value);
        switch (int_value) {
            case 0: enqueue(new GamepadStickEvent(_now(), Stick::RS, +1.000, +0.000)); break;
            case 1: enqueue(new GamepadStickEvent(_now(), Stick::RS, +0.707, +0.707)); break;
            case 2: enqueue(new GamepadStickEvent(_now(), Stick::RS, +0.000, +1.000)); break;
            case 3: enqueue(new GamepadStickEvent(_now(), Stick::RS, -0.707, +0.707)); break;
            case 4: enqueue(new GamepadStickEvent(_now(), Stick::RS, -1.000, +0.000)); break;
            case 5: enqueue(new GamepadStickEvent(_now(), Stick::RS, -0.707, -0.707)); break;
            case 6: enqueue(new GamepadStickEvent(_now(), Stick::RS, +0.000, -1.000)); break;
            case 7: enqueue(new GamepadStickEvent(_now(), Stick::RS, +0.707, -0.707)); break;
            case 8: enqueue(new GamepadStickEvent(_now(), Stick::RS, 0.000, 0.000)); break;
        }
    }

    void switch_pro_button_event(IOHIDElementRef element, IOHIDValueRef value) {
        if (!antares_is_active()) {
            return;
        }
        bool            down   = IOHIDValueGetIntegerValue(value);
        uint16_t        usage  = IOHIDElementGetUsage(element);
        Gamepad::Button button = Gamepad::Button::NONE;

        switch (usage) {
            case 1: button = Gamepad::Button::B; break;
            case 2: button = Gamepad::Button::A; break;
            case 3: button = Gamepad::Button::Y; break;
            case 4: button = Gamepad::Button::X; break;
            case 5: button = Gamepad::Button::LB; break;
            case 6: button = Gamepad::Button::RB; break;
            case 7: button = Gamepad::Button::LT; break;
            case 8: button = Gamepad::Button::RT; break;
            case 9: button = Gamepad::Button::BACK; break;
            case 10: button = Gamepad::Button::START; break;
            case 11: button = Gamepad::Button::LSB; break;
            case 12: button = Gamepad::Button::RSB; break;
            case 13: button = Gamepad::Button::NONE; break;
            case 14: button = Gamepad::Button::NONE; break;
        }

        if (button == Gamepad::Button::NONE) {
            return;
        } else if (down) {
            enqueue(new GamepadButtonDownEvent(_now(), button));
        } else {
            enqueue(new GamepadButtonUpEvent(_now(), button));
        }
    }

    void switch_pro_analog_event(IOHIDElementRef element, IOHIDValueRef value) {
        uint16_t usage = IOHIDElementGetUsage(element);
        if (usage == 57) {  // D-Pad
            int64_t int_value = IOHIDValueGetIntegerValue(value);
            switch (int_value) {
                case 0: set_switch_dpad(true, false, false, false); break;
                case 1: set_switch_dpad(true, true, false, false); break;
                case 2: set_switch_dpad(false, true, false, false); break;
                case 3: set_switch_dpad(false, true, true, false); break;
                case 4: set_switch_dpad(false, false, true, false); break;
                case 5: set_switch_dpad(false, false, true, true); break;
                case 6: set_switch_dpad(false, false, false, true); break;
                case 7: set_switch_dpad(true, false, false, true); break;
                case 8: set_switch_dpad(false, false, false, false); break;
            }
            return;
        }

        return default_analog_event(element, value);
    }

    void set_switch_dpad(bool up, bool right, bool down, bool left) {
        set_switch_dpad_button(Gamepad::Button::UP, 0, up);
        set_switch_dpad_button(Gamepad::Button::RIGHT, 1, right);
        set_switch_dpad_button(Gamepad::Button::DOWN, 2, down);
        set_switch_dpad_button(Gamepad::Button::LEFT, 3, left);
    }

    void set_switch_dpad_button(Gamepad::Button button, int index, bool pressed) {
        if (pressed != switch_dpad[index]) {
            if (pressed) {
                enqueue(new GamepadButtonDownEvent(_now(), button));
            } else {
                enqueue(new GamepadButtonUpEvent(_now(), button));
            }
            switch_dpad[index] = pressed;
        }
    }

    void default_button_event(IOHIDElementRef element, IOHIDValueRef value) {
        if (!antares_is_active()) {
            return;
        }
        bool     down  = IOHIDValueGetIntegerValue(value);
        uint16_t usage = IOHIDElementGetUsage(element);
        if (down) {
            enqueue(new GamepadButtonDownEvent(_now(), static_cast<Gamepad::Button>(usage)));
        } else {
            enqueue(new GamepadButtonUpEvent(_now(), static_cast<Gamepad::Button>(usage)));
        }
    }

    void default_analog_event(IOHIDElementRef element, IOHIDValueRef value) {
        int64_t  int_value = IOHIDValueGetIntegerValue(value);
        uint16_t usage     = IOHIDElementGetUsage(element);
        switch (usage) {
            case kHIDUsage_GD_X:
            case kHIDUsage_GD_Y:
            case kHIDUsage_GD_Rx:
            case kHIDUsage_GD_Ry: {
                int64_t min          = IOHIDElementGetLogicalMin(element);
                int64_t max          = IOHIDElementGetLogicalMax(element);
                int64_t range        = max - min;
                double  double_value = ((double(int_value - min) / range) * 2.0) - 1.0;

                usage -= kHIDUsage_GD_X;
                gamepad[usage]                 = double_value;
                static const int x_component[] = {0, 0, -1, 3, 3, -1};
                double           x             = gamepad[x_component[usage]];
                double           y             = gamepad[x_component[usage] + 1];
                enqueue(new GamepadStickEvent(
                        _now(), static_cast<Gamepad::Stick>(kHIDUsage_GD_X + x_component[usage]),
                        x, y));
            } break;
            case kHIDUsage_GD_Z:
            case kHIDUsage_GD_Rz: default_button_event(element, value); break;
        }
    }

    void enqueue(Event* event) {
        event_queue.emplace(event);
        antares_window_cancel_event(window.c_obj());
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

bool CocoaVideoDriver::start_editing(TextReceiver* text) {
    _bridge->text_receiver = text;
    return true;
}

void CocoaVideoDriver::stop_editing(TextReceiver* text) {
    if (_bridge->text_receiver == text) {
        _bridge->text_receiver = nullptr;
    }
}

void CocoaVideoDriver::loop(Card* initial) {
    CGLPixelFormatAttribute attrs[] = {
            kCGLPFAOpenGLProfile,
            (CGLPixelFormatAttribute)kCGLOGLPVersion_3_2_Core,
            kCGLPFADisplayMask,
            static_cast<CGLPixelFormatAttribute>(
                    CGDisplayIDToOpenGLDisplayMask(kCGDirectMainDisplay)),
            kCGLPFAColorSize,
            static_cast<CGLPixelFormatAttribute>(24),
            kCGLPFADoubleBuffer,
            kCGLPFAAccelerated,
            static_cast<CGLPixelFormatAttribute>(0),
    };

    cgl::PixelFormat pixel_format(attrs);
    cgl::Context     context(pixel_format.c_obj(), NULL);
    AntaresWindow    window(pixel_format, context);
    _window             = window.c_obj();
    GLint swap_interval = 1;
    CGLSetParameter(context.c_obj(), kCGLCPSwapInterval, &swap_interval);
    CGLSetCurrentContext(context.c_obj());

    MainLoop main_loop(*this, initial);
    main_loop.draw();
    CGLFlushDrawable(context.c_obj());
    EventBridge bridge = {_input_mode, main_loop, context, window};
    _bridge            = &bridge;

    antares_window_set_callback(_window, EventBridge::text, &bridge);

    cf::MutableDictionary gamepad(CFDictionaryCreateMutable(
            NULL, 0, &kCFCopyStringDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks));
    gamepad.set(CFSTR(kIOHIDDeviceUsagePageKey), cf::wrap(kHIDPage_GenericDesktop).c_obj());
    gamepad.set(CFSTR(kIOHIDDeviceUsageKey), cf::wrap(kHIDUsage_GD_GamePad).c_obj());
    cf::MutableArray criteria(CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks));
    criteria.append(gamepad.c_obj());

    auto hid_manager = IOHIDManagerCreate(kCFAllocatorDefault, kIOHIDOptionsTypeNone);
    IOHIDManagerSetDeviceMatchingMultiple(hid_manager, criteria.c_obj());
    IOHIDManagerScheduleWithRunLoop(hid_manager, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
    IOReturn r = IOHIDManagerOpen(hid_manager, kIOHIDOptionsTypeNone);
    if (r != 0) {
        throw std::runtime_error("IOHIDManagerOpen");
    }
    IOHIDManagerRegisterInputValueCallback(hid_manager, EventBridge::hid_event, &bridge);

    while (!main_loop.done()) {
        wall_time at;
        if (main_loop.top()->next_timer(at)) {
            if (antares_window_next_event(_window, at.time_since_epoch().count())) {
                bridge.send_all();
            } else {
                main_loop.top()->fire_timer();
                main_loop.draw();
                CGLFlushDrawable(context.c_obj());
            }
        } else {
            at = wall_time::max();
            antares_window_next_event(_window, at.time_since_epoch().count());
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

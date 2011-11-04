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

#include "cocoa/c/CocoaVideoDriver.h"

#include <Carbon/Carbon.h>
#include <Cocoa/Cocoa.h>

void antares_menu_bar_hide() {
    [NSMenu setMenuBarVisible:NO];
}

void antares_menu_bar_show() {
    [NSMenu setMenuBarVisible:YES];
}

int64_t antares_double_click_interval_usecs() {
    return GetDblTime() * 1000000 / 60;
    // 10.6+: return [NSEvent doubleClickInterval] * 1e6;
}

struct AntaresEventTranslator {
    void (*mouse_down_callback)(int button, int32_t x, int32_t y, void* userdata);
    void* mouse_down_userdata;

    void (*mouse_up_callback)(int button, int32_t x, int32_t y, void* userdata);
    void* mouse_up_userdata;

    void (*mouse_move_callback)(int32_t x, int32_t y, void* userdata);
    void* mouse_move_userdata;

    void (*key_down_callback)(int32_t key, void* userdata);
    void* key_down_userdata;

    void (*key_up_callback)(int32_t key, void* userdata);
    void* key_up_userdata;

    int32_t screen_width;
    int32_t screen_height;

    int32_t last_flags;
};

static NSPoint translate_coords(AntaresEventTranslator* translator, NSPoint input) {
    return NSMakePoint(input.x, translator->screen_height - 1 - input.y);
}

AntaresEventTranslator* antares_event_translator_create(
        int32_t screen_width, int32_t screen_height) {
    AntaresEventTranslator* translator = malloc(sizeof(AntaresEventTranslator));
    memset(translator, 0, sizeof(AntaresEventTranslator));
    translator->screen_width = screen_width;
    translator->screen_height = screen_height;
    translator->last_flags = 0;
    return translator;
}

void antares_event_translator_destroy(AntaresEventTranslator* translator) {
    free(translator);
}

void antares_get_mouse_location(AntaresEventTranslator* translator, int32_t* x, int32_t* y) {
    NSPoint location = translate_coords(translator, [NSEvent mouseLocation]);
    *x = location.x;
    *y = location.y;
}

void antares_get_mouse_button(AntaresEventTranslator* translator, int32_t* button) {
    *button = CGEventSourceButtonState(
            kCGEventSourceStateCombinedSessionState, kCGMouseButtonLeft);
}

void antares_event_translator_set_mouse_down_callback(
        AntaresEventTranslator* translator,
        void (*callback)(int button, int32_t x, int32_t y, void* userdata), void* userdata) {
    translator->mouse_down_callback = callback;
    translator->mouse_down_userdata = userdata;
}

void antares_event_translator_set_mouse_up_callback(
        AntaresEventTranslator* translator,
        void (*callback)(int button, int32_t x, int32_t y, void* userdata), void* userdata) {
    translator->mouse_up_callback = callback;
    translator->mouse_up_userdata = userdata;
}

void antares_event_translator_set_mouse_move_callback(
        AntaresEventTranslator* translator,
        void (*callback)(int32_t x, int32_t y, void* userdata), void* userdata) {
    translator->mouse_move_callback = callback;
    translator->mouse_move_userdata = userdata;
}

void antares_event_translator_set_key_down_callback(
        AntaresEventTranslator* translator,
        void (*callback)(int32_t key, void* userdata), void* userdata) {
    translator->key_down_callback = callback;
    translator->key_down_userdata = userdata;
}

void antares_event_translator_set_key_up_callback(
        AntaresEventTranslator* translator,
        void (*callback)(int32_t key, void* userdata), void* userdata) {
    translator->key_up_callback = callback;
    translator->key_up_userdata = userdata;
}

static void flags_changed(AntaresEventTranslator* translator, int32_t flags) {
    struct ModifierFlag {
        uint32_t bit;
        uint32_t code;
    };
    static const int modifier_flag_count = 5;
    static const struct ModifierFlag modifier_flags[5] = {
        {NSAlphaShiftKeyMask, 0x39},  // caps lock.
        {NSShiftKeyMask,      0x38},
        {NSControlKeyMask,    0x3b},
        {NSAlternateKeyMask,  0x3a},  // option.
        {NSCommandKeyMask,    0x37},
    };

    int i;
    for (i = 0; i < modifier_flag_count; ++i) {
        if ((translator->last_flags ^ flags) & modifier_flags[i].bit) {
            if (flags & modifier_flags[i].bit) {
                translator->key_down_callback(
                        modifier_flags[i].code, translator->key_down_userdata);
            } else {
                translator->key_up_callback(
                        modifier_flags[i].code, translator->key_up_userdata);
            }
        }
    }
    translator->last_flags = flags;
}

void antares_event_translator_enqueue(AntaresEventTranslator* translator, int64_t until) {
    NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
    NSDate* date = [NSDate dateWithTimeIntervalSince1970:(until * 1e-6)];
    NSEvent* event =
        [NSApp nextEventMatchingMask:NSAnyEventMask untilDate:date inMode:NSDefaultRunLoopMode
            dequeue:YES];
    if (event) {
        switch ([event type]) {
          case NSLeftMouseDown:
            {
                NSPoint where = translate_coords(translator, [event locationInWindow]);
                translator->mouse_down_callback(
                        0, where.x, where.y, translator->mouse_down_userdata);
            }
            break;

          case NSLeftMouseUp:
            {
                NSPoint where = translate_coords(translator, [event locationInWindow]);
                translator->mouse_up_callback(0, where.x, where.y, translator->mouse_up_userdata);
            }
            break;

          case NSMouseMoved:
          case NSLeftMouseDragged:
            {
                NSPoint where = translate_coords(translator, [event locationInWindow]);
                translator->mouse_move_callback(where.x, where.y, translator->mouse_move_userdata);
            }
            break;

          case NSKeyDown:
            if (![event isARepeat]) {
                translator->key_down_callback(
                        [event keyCode] & 0xffff, translator->key_down_userdata);
            }
            break;

          case NSKeyUp:
            if (![event isARepeat]) {
                translator->key_up_callback(
                        [event keyCode] & 0xffff, translator->key_up_userdata);
            }
            break;

          case NSFlagsChanged:
            flags_changed(translator, [event modifierFlags]);
            break;

          default:
            break;
        }
    }
    [pool drain];
}

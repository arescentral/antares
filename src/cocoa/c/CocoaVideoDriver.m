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

#include "cocoa/c/CocoaVideoDriver.h"

#include <Carbon/Carbon.h>
#include <Cocoa/Cocoa.h>

static bool mouse_visible = true;

void antares_menu_bar_hide() {
    [NSMenu setMenuBarVisible:NO];
}

void antares_menu_bar_show() {
    [NSMenu setMenuBarVisible:YES];
}

void antares_mouse_hide() {
    if (mouse_visible) {
        [NSCursor hide];
        mouse_visible = false;
    }
}

void antares_mouse_show() {
    if (mouse_visible) {
        [NSCursor unhide];
        mouse_visible = true;
    }
}

int64_t antares_double_click_interval_usecs() {
    return GetDblTime() * 1000000 / 60;
    // 10.6+: return [NSEvent doubleClickInterval] * 1e6;
}

struct AntaresWindow {
    int32_t screen_width;
    int32_t screen_height;
    NSOpenGLPixelFormat* pixel_format;
    NSOpenGLContext* context;
    NSOpenGLView* view;
    NSWindow* window;
};

AntaresWindow* antares_window_create(
        CGLPixelFormatObj pixel_format, CGLContextObj context,
        int32_t screen_width, int32_t screen_height) {
    AntaresWindow* window = malloc(sizeof(AntaresWindow));
    window->screen_width = screen_width;
    window->screen_height = screen_height;
    window->pixel_format = [[NSOpenGLPixelFormat alloc] initWithCGLPixelFormatObj:pixel_format];
    window->context = [[NSOpenGLContext alloc] initWithCGLContextObj:context];
    NSRect screen_rect = NSMakeRect(0, 0, screen_width, screen_height);
    window->view = [[NSOpenGLView alloc] initWithFrame:screen_rect
        pixelFormat:window->pixel_format];
    [window->view setOpenGLContext:window->context];
    window->window = [[NSWindow alloc] initWithContentRect:screen_rect
        styleMask:(NSTitledWindowMask | NSMiniaturizableWindowMask)
        backing:NSBackingStoreBuffered
        defer:NO];
    [window->window setAcceptsMouseMovedEvents:YES];
    [window->window setContentView:window->view];
    [window->window makeKeyAndOrderFront:NSApp];
    [window->window center];
    return window;
}

void antares_window_destroy(AntaresWindow* window) {
    [window->window release];
    [window->view release];
    [window->context release];
    [window->pixel_format release];
    free(window);
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
    AntaresWindow* window;

    int32_t last_flags;
};

static NSPoint translate_coords(
        AntaresEventTranslator* translator, NSWindow* from_window, NSPoint input) {
    NSWindow* to_window = (translator->window != nil) ? translator->window->window : nil;
    if (from_window != to_window) {
        if (from_window != nil) {
            input = [from_window convertBaseToScreen:input];
        }
        if (to_window != nil) {
            input = [to_window convertScreenToBase:input];
        }
    }
    return NSMakePoint(input.x, translator->screen_height - 1 - input.y);
}

AntaresEventTranslator* antares_event_translator_create(
        int32_t screen_width, int32_t screen_height) {
    AntaresEventTranslator* translator = malloc(sizeof(AntaresEventTranslator));
    memset(translator, 0, sizeof(AntaresEventTranslator));
    translator->screen_width = screen_width;
    translator->screen_height = screen_height;
    translator->window = nil;
    translator->last_flags = 0;
    return translator;
}

void antares_event_translator_destroy(AntaresEventTranslator* translator) {
    free(translator);
}

void antares_event_translator_set_window(
        AntaresEventTranslator* translator, AntaresWindow* window) {
    translator->window = window;
}

void antares_get_mouse_location(AntaresEventTranslator* translator, int32_t* x, int32_t* y) {
    NSPoint location = translate_coords(translator, nil, [NSEvent mouseLocation]);
    *x = location.x;
    *y = location.y;
}

void antares_get_mouse_button(AntaresEventTranslator* translator, int32_t* button, int which) {
    switch (which) {
      case 0:
        *button = CGEventSourceButtonState(
                kCGEventSourceStateCombinedSessionState, kCGMouseButtonLeft);
        break;

      case 1:
        *button = CGEventSourceButtonState(
                kCGEventSourceStateCombinedSessionState, kCGMouseButtonRight);
        break;

      case 2:
        *button = CGEventSourceButtonState(
                kCGEventSourceStateCombinedSessionState, kCGMouseButtonCenter);
        break;
    }
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

static void hide_unhide(AntaresWindow* window, NSPoint location) {
    if (!window) {
        return;
    }
    bool in_window =
        location.x >= 0 && location.y >= 0 &&
        location.x < window->screen_width && location.y < window->screen_height;
    if (in_window) {
        antares_mouse_hide();
    } else {
        antares_mouse_show();
    }
}

static int button_for(NSEvent* event) {
    switch ([event type]) {
      case NSLeftMouseDown:
      case NSLeftMouseUp:
        return 0;

      case NSRightMouseDown:
      case NSRightMouseUp:
        return 1;

      case NSOtherMouseDown:
      case NSOtherMouseUp:
        return 2;

      default:
        return -1;
    }
}

static void mouse_down(AntaresEventTranslator* translator, NSEvent* event) {
    NSPoint where = translate_coords(translator, [event window], [event locationInWindow]);
    hide_unhide(translator->window, where);
    int button = button_for(event);
    translator->mouse_down_callback(button, where.x, where.y, translator->mouse_down_userdata);
}

static void mouse_up(AntaresEventTranslator* translator, NSEvent* event) {
    NSPoint where = translate_coords(translator, [event window], [event locationInWindow]);
    hide_unhide(translator->window, where);
    int button = button_for(event);
    translator->mouse_up_callback(button, where.x, where.y, translator->mouse_up_userdata);
}

static void mouse_move(AntaresEventTranslator* translator, NSEvent* event) {
    NSPoint where = translate_coords(translator, [event window], [event locationInWindow]);
    hide_unhide(translator->window, where);
    translator->mouse_move_callback(where.x, where.y, translator->mouse_move_userdata);
}

bool antares_event_translator_next(AntaresEventTranslator* translator, int64_t until) {
    NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
    NSDate* date = [NSDate dateWithTimeIntervalSince1970:(until * 1e-6)];
    bool result = false;
    while (!result) {
        NSEvent* event =
            [NSApp nextEventMatchingMask:NSAnyEventMask untilDate:date inMode:NSDefaultRunLoopMode
             dequeue:YES];
        if (!event) {
            break;
        }
        switch ([event type]) {
          case NSLeftMouseDown:
          case NSRightMouseDown:
            mouse_down(translator, event);
            result = true;
            break;

          case NSLeftMouseUp:
          case NSRightMouseUp:
            mouse_up(translator, event);
            result = true;
            break;

          case NSMouseMoved:
          case NSLeftMouseDragged:
          case NSRightMouseDragged:
            mouse_move(translator, event);
            result = true;
            break;

          case NSKeyDown:
            if (![event isARepeat]) {
                translator->key_down_callback(
                        [event keyCode] & 0xffff, translator->key_down_userdata);
                result = true;
            }
            break;

          case NSKeyUp:
            if (![event isARepeat]) {
                translator->key_up_callback(
                        [event keyCode] & 0xffff, translator->key_up_userdata);
                result = true;
            }
            break;

          case NSFlagsChanged:
            flags_changed(translator, [event modifierFlags]);
            result = true;
            break;

          default:
            break;
        }
    }
    [pool drain];
    return result;
}

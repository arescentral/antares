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

#include "mac/c/CocoaVideoDriver.h"

#include <Carbon/Carbon.h>
#include <Cocoa/Cocoa.h>
#include <mach/clock.h>
#include <mach/mach.h>

static bool mouse_visible = true;

@implementation NSDate (AntaresAdditions)
- (NSTimeInterval)timeIntervalSinceSystemStart {
    clock_serv_t  system_clock;
    kern_return_t status = host_get_clock_service(mach_host_self(), SYSTEM_CLOCK, &system_clock);
    (void)status;  // TODO(sfiera): abort? don't know how to proceed.
    mach_timespec_t now;
    clock_get_time(system_clock, &now);
    NSTimeInterval now_secs = now.tv_sec + (now.tv_nsec / 1e9);
    return now_secs + [self timeIntervalSinceNow];
}
@end

bool antares_is_active() { return [NSApp isActive]; }

void antares_menu_bar_hide() { [NSMenu setMenuBarVisible:NO]; }

void antares_menu_bar_show() { [NSMenu setMenuBarVisible:YES]; }

void antares_mouse_hide() {
    if (mouse_visible) {
        [NSCursor hide];
        mouse_visible = false;
    }
}

void antares_mouse_show() {
    if (!mouse_visible) {
        [NSCursor unhide];
        mouse_visible = true;
    }
}

struct AntaresWindow {
    NSOpenGLPixelFormat* pixel_format;
    NSOpenGLContext*     context;
    NSOpenGLView*        view;
    NSWindow*            window;
};

AntaresWindow* antares_window_create(CGLPixelFormatObj pixel_format, CGLContextObj context) {
    AntaresWindow* window = malloc(sizeof(AntaresWindow));
    window->pixel_format  = [[NSOpenGLPixelFormat alloc] initWithCGLPixelFormatObj:pixel_format];
    window->context       = [[NSOpenGLContext alloc] initWithCGLContextObj:context];

    NSRect window_rect = NSMakeRect(0, 0, 640, 480);
    int    style_mask  = NSTitledWindowMask | NSMiniaturizableWindowMask | NSResizableWindowMask;

    window->view =
            [[NSOpenGLView alloc] initWithFrame:window_rect pixelFormat:window->pixel_format];
    [window->view setWantsBestResolutionOpenGLSurface:YES];
    [window->view setOpenGLContext:window->context];

    window->window = [[NSWindow alloc] initWithContentRect:window_rect
                                                 styleMask:style_mask
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

int32_t antares_window_screen_width(const AntaresWindow* window) {
    return [window->view bounds].size.width;
}

int32_t antares_window_screen_height(const AntaresWindow* window) {
    return [window->view bounds].size.height;
}

int32_t antares_window_viewport_width(const AntaresWindow* window) {
    return [window->view convertRectToBacking:[window->view bounds]].size.width;
}

int32_t antares_window_viewport_height(const AntaresWindow* window) {
    return [window->view convertRectToBacking:[window->view bounds]].size.height;
}

struct AntaresEventTranslator {
    void (*mouse_down_callback)(int button, int32_t x, int32_t y, int count, void* userdata);
    void* mouse_down_userdata;

    void (*mouse_up_callback)(int button, int32_t x, int32_t y, void* userdata);
    void* mouse_up_userdata;

    void (*mouse_move_callback)(int32_t x, int32_t y, void* userdata);
    void* mouse_move_userdata;

    void (*caps_lock_callback)(void* userdata);
    void* caps_lock_userdata;

    void (*caps_unlock_callback)(void* userdata);
    void* caps_unlock_userdata;

    AntaresWindow* window;

    int32_t last_flags;
};

static bool translate_coords(AntaresEventTranslator* translator, NSEvent* event, NSPoint* p) {
    if ([event window] != translator->window->window) {
        return false;
    }
    NSPoint input    = [event locationInWindow];
    input            = [translator->window->view convertPoint:input fromView:nil];
    NSSize view_size = [translator->window->view bounds].size;
    *p               = NSMakePoint(input.x, view_size.height - input.y);
    return true;
}

AntaresEventTranslator* antares_event_translator_create() {
    AntaresEventTranslator* translator = malloc(sizeof(AntaresEventTranslator));
    memset(translator, 0, sizeof(AntaresEventTranslator));
    translator->window     = nil;
    translator->last_flags = 0;
    return translator;
}

void antares_event_translator_destroy(AntaresEventTranslator* translator) { free(translator); }

void antares_event_translator_set_window(
        AntaresEventTranslator* translator, AntaresWindow* window) {
    translator->window = window;
}

void antares_get_mouse_location(AntaresEventTranslator* translator, int32_t* x, int32_t* y) {
    NSPoint location = [NSEvent mouseLocation];
    NSRect  r        = NSMakeRect(location.x, location.y, 1, 1);
    location         = [translator->window->window convertRectFromScreen:r].origin;
    location         = [translator->window->view convertPoint:location fromView:nil];
    NSSize view_size = [translator->window->view bounds].size;
    *x               = round(location.x);
    *y               = round(location.y);
    *y               = view_size.height - *y;
}

void antares_event_translator_set_mouse_down_callback(
        AntaresEventTranslator* translator,
        void (*callback)(int button, int32_t x, int32_t y, int count, void* userdata),
        void* userdata) {
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
        AntaresEventTranslator* translator, void (*callback)(int32_t x, int32_t y, void* userdata),
        void*                   userdata) {
    translator->mouse_move_callback = callback;
    translator->mouse_move_userdata = userdata;
}

void antares_event_translator_set_caps_lock_callback(
        AntaresEventTranslator* translator, void (*callback)(void* userdata), void* userdata) {
    translator->caps_lock_callback = callback;
    translator->caps_lock_userdata = userdata;
}

void antares_event_translator_set_caps_unlock_callback(
        AntaresEventTranslator* translator, void (*callback)(void* userdata), void* userdata) {
    translator->caps_unlock_callback = callback;
    translator->caps_unlock_userdata = userdata;
}

static void hide_unhide(AntaresWindow* window, NSPoint location) {
    if (!window) {
        return;
    }
    NSSize size      = [window->view bounds].size;
    bool   in_window = location.x >= 0 && location.y >= 0 && location.x < size.width &&
                     location.y < size.height;
    if (in_window) {
        antares_mouse_hide();
    } else {
        antares_mouse_show();
    }
}

static int button_for(NSEvent* event) {
    switch ([event type]) {
        case NSLeftMouseDown:
        case NSLeftMouseUp: return 0;

        case NSRightMouseDown:
        case NSRightMouseUp: return 1;

        case NSOtherMouseDown:
        case NSOtherMouseUp: return 2;

        default: return -1;
    }
}

static void mouse_down(AntaresEventTranslator* translator, NSEvent* event) {
    NSPoint where;
    if (!translate_coords(translator, event, &where)) {
        return;
    }
    hide_unhide(translator->window, where);
    int button = button_for(event);
    translator->mouse_down_callback(
            button, where.x, where.y, [event clickCount], translator->mouse_down_userdata);
}

static void mouse_up(AntaresEventTranslator* translator, NSEvent* event) {
    NSPoint where;
    if (!translate_coords(translator, event, &where)) {
        return;
    }
    hide_unhide(translator->window, where);
    int button = button_for(event);
    translator->mouse_up_callback(button, where.x, where.y, translator->mouse_up_userdata);
}

static void mouse_move(AntaresEventTranslator* translator, NSEvent* event) {
    NSPoint where;
    if (!translate_coords(translator, event, &where)) {
        return;
    }
    hide_unhide(translator->window, where);
    translator->mouse_move_callback(where.x, where.y, translator->mouse_move_userdata);
}

bool antares_event_translator_next(AntaresEventTranslator* translator, int64_t until) {
    NSAutoreleasePool* pool    = [[NSAutoreleasePool alloc] init];
    NSDate*            date    = [NSDate dateWithTimeIntervalSince1970:(until * 1e-6)];
    bool               waiting = true;
    while (waiting) {
        NSEvent* event = [NSApp nextEventMatchingMask:NSAnyEventMask
                                            untilDate:date
                                               inMode:NSDefaultRunLoopMode
                                              dequeue:YES];
        if (!event) {
            break;
        }
        // Put events after `until` back in the queue.
        if ([event timestamp] > [date timeIntervalSinceSystemStart]) {
            [NSApp postEvent:event atStart:true];
            break;
        }

        // Send non-key events.
        switch ([event type]) {
            case NSKeyDown:
            case NSKeyUp: break;

            default: [NSApp sendEvent:event]; break;
        };

        // Handle events.
        switch ([event type]) {
            case NSLeftMouseDown:
            case NSRightMouseDown:
                mouse_down(translator, event);
                waiting = false;
                break;

            case NSLeftMouseUp:
            case NSRightMouseUp:
                mouse_up(translator, event);
                waiting = false;
                break;

            case NSMouseMoved:
            case NSLeftMouseDragged:
            case NSRightMouseDragged:
                mouse_move(translator, event);
                waiting = false;
                break;

            case NSApplicationDefined: waiting = false; break;

            case NSFlagsChanged:
                if ([event modifierFlags] & NSAlphaShiftKeyMask) {
                    translator->caps_lock_callback(translator->caps_lock_userdata);
                } else {
                    translator->caps_unlock_callback(translator->caps_unlock_userdata);
                }
                break;

            default: break;
        }
    }
    [pool drain];
    return !waiting;
}

void antares_event_translator_cancel(AntaresEventTranslator* translator) {
    NSAutoreleasePool* pool  = [[NSAutoreleasePool alloc] init];
    NSEvent*           event = [NSEvent otherEventWithType:NSApplicationDefined
                                        location:NSMakePoint(0, 0)
                                   modifierFlags:0
                                       timestamp:[[NSDate date] timeIntervalSinceSystemStart]
                                    windowNumber:0
                                         context:nil
                                         subtype:0
                                           data1:0
                                           data2:0];
    [NSApp postEvent:event atStart:true];
    [pool drain];
}

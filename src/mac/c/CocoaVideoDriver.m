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
#include <sys/time.h>

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

@interface AntaresView : NSOpenGLView {
  @public
    void (*key_down_callback)(int key, void* userdata);
    void* key_down_userdata;

    void (*key_up_callback)(int key, void* userdata);
    void* key_up_userdata;

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

    int32_t modifier_flags;
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
    AntaresView*         view;
    NSWindow*            window;
};

static void* memdup(void* data, size_t size) {
    void* copy = malloc(size);
    memcpy(copy, data, size);
    return copy;
}

AntaresWindow* antares_window_create(CGLPixelFormatObj pixel_format, CGLContextObj context) {
    AntaresWindow window = {};
    window.pixel_format  = [[NSOpenGLPixelFormat alloc] initWithCGLPixelFormatObj:pixel_format];
    window.context       = [[NSOpenGLContext alloc] initWithCGLContextObj:context];

    NSRect window_rect = NSMakeRect(0, 0, 640, 480);
    int    style_mask  = NSTitledWindowMask | NSMiniaturizableWindowMask | NSResizableWindowMask;

    window.view = [[AntaresView alloc] initWithFrame:window_rect pixelFormat:window.pixel_format];
    [window.view setWantsBestResolutionOpenGLSurface:YES];
    [window.view setOpenGLContext:window.context];

    window.window = [[NSWindow alloc] initWithContentRect:window_rect
                                                styleMask:style_mask
                                                  backing:NSBackingStoreBuffered
                                                    defer:NO];
    [window.window setAcceptsMouseMovedEvents:YES];
    [window.window setContentView:window.view];
    [window.window makeKeyAndOrderFront:NSApp];
    [window.window makeFirstResponder:window.view];
    [window.window center];
    return memdup(&window, sizeof(AntaresWindow));
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

static bool translate_coords(AntaresView* view, NSEvent* event, NSPoint* p) {
    if ([event window] != [view window]) {
        return false;
    }
    NSPoint input    = [event locationInWindow];
    input            = [view convertPoint:input fromView:nil];
    NSSize view_size = [view bounds].size;
    *p               = NSMakePoint(input.x, view_size.height - input.y);
    return true;
}

void antares_get_mouse_location(AntaresWindow* window, int32_t* x, int32_t* y) {
    NSPoint location = [NSEvent mouseLocation];
    NSRect  r        = NSMakeRect(location.x, location.y, 1, 1);
    location         = [window->window convertRectFromScreen:r].origin;
    location         = [window->view convertPoint:location fromView:nil];
    NSSize view_size = [window->view bounds].size;
    *x               = round(location.x);
    *y               = round(location.y);
    *y               = view_size.height - *y;
}

void antares_window_set_key_down_callback(
        AntaresWindow* window, void (*callback)(int key, void* userdata), void* userdata) {
    window->view->key_down_callback = callback;
    window->view->key_down_userdata = userdata;
}

void antares_window_set_key_up_callback(
        AntaresWindow* window, void (*callback)(int key, void* userdata), void* userdata) {
    window->view->key_up_callback = callback;
    window->view->key_up_userdata = userdata;
}

void antares_window_set_mouse_down_callback(
        AntaresWindow* window,
        void (*callback)(int button, int32_t x, int32_t y, int count, void* userdata),
        void* userdata) {
    window->view->mouse_down_callback = callback;
    window->view->mouse_down_userdata = userdata;
}

void antares_window_set_mouse_up_callback(
        AntaresWindow* window, void (*callback)(int button, int32_t x, int32_t y, void* userdata),
        void*          userdata) {
    window->view->mouse_up_callback = callback;
    window->view->mouse_up_userdata = userdata;
}

void antares_window_set_mouse_move_callback(
        AntaresWindow* window, void (*callback)(int32_t x, int32_t y, void* userdata),
        void*          userdata) {
    window->view->mouse_move_callback = callback;
    window->view->mouse_move_userdata = userdata;
}

void antares_window_set_caps_lock_callback(
        AntaresWindow* window, void (*callback)(void* userdata), void* userdata) {
    window->view->caps_lock_callback = callback;
    window->view->caps_lock_userdata = userdata;
}

void antares_window_set_caps_unlock_callback(
        AntaresWindow* window, void (*callback)(void* userdata), void* userdata) {
    window->view->caps_unlock_callback = callback;
    window->view->caps_unlock_userdata = userdata;
}

static void key_down(AntaresView* view, NSEvent* event) {
    NSPoint where;
    if (!translate_coords(view, event, &where)) {
        return;
    }
    view->key_down_callback([event keyCode], view -> key_down_userdata);
}

static void key_up(AntaresView* view, NSEvent* event) {
    NSPoint where;
    if (!translate_coords(view, event, &where)) {
        return;
    }
    view->key_up_callback([event keyCode], view -> key_up_userdata);
}

static void hide_unhide(AntaresView* view, NSPoint location) {
    if (!view) {
        return;
    }
    NSSize size      = [view bounds].size;
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

static void mouse_down(AntaresView* view, NSEvent* event) {
    NSPoint where;
    if (!translate_coords(view, event, &where)) {
        return;
    }
    hide_unhide(view, where);
    int button = button_for(event);
    view->mouse_down_callback(
            button, where.x, where.y, [event clickCount], view -> mouse_down_userdata);
}

static void mouse_up(AntaresView* view, NSEvent* event) {
    NSPoint where;
    if (!translate_coords(view, event, &where)) {
        return;
    }
    hide_unhide(view, where);
    int button = button_for(event);
    view->mouse_up_callback(button, where.x, where.y, view->mouse_up_userdata);
}

static void mouse_move(AntaresView* view, NSEvent* event) {
    NSPoint where;
    if (!translate_coords(view, event, &where)) {
        return;
    }
    hide_unhide(view, where);
    view->mouse_move_callback(where.x, where.y, view->mouse_move_userdata);
}

static void flags_changed(AntaresView* view, NSEvent* event) {
    int32_t old_flags = view->modifier_flags;
    int32_t new_flags = view->modifier_flags = [event modifierFlags];
    if ([event keyCode] == 0x39) {  // Caps Lock
        if ((new_flags | old_flags) == new_flags) {
            view->caps_lock_callback(view->caps_lock_userdata);
        } else {
            view->caps_unlock_callback(view->caps_unlock_userdata);
        }
        return;
    }
    if ((new_flags | old_flags) == new_flags) {
        view->key_down_callback([event keyCode], view -> key_down_userdata);
    } else {
        view->key_up_callback([event keyCode], view -> key_up_userdata);
    }
}

bool antares_window_next_event(AntaresWindow* window, int64_t until) {
    @autoreleasepool {
        NSDate*  date  = [NSDate dateWithTimeIntervalSince1970:(until * 1e-6)];
        NSEvent* event = [NSApp nextEventMatchingMask:NSAnyEventMask
                                            untilDate:date
                                               inMode:NSDefaultRunLoopMode
                                              dequeue:YES];
        if (!event) {
            return false;
        }

        // Don’t handle events after wait time; put them back in the queue
        // so they can be handled after the timer fires.
        if ([event timestamp] > [date timeIntervalSinceSystemStart]) {
            [NSApp postEvent:event atStart:true];
            return false;
        }

        // Forward to the view so it can fire a callback.
        switch ([event type]) {
            case NSEventTypeKeyDown: [window->view keyDown:event]; break;
            case NSEventTypeKeyUp: [window->view keyUp:event]; break;
            case NSEventTypeFlagsChanged: [window->view flagsChanged:event]; break;
            default: [NSApp sendEvent:event];
        }
        return true;
    }
}

void antares_window_cancel_event(AntaresWindow* window) {
    @autoreleasepool {
        NSEvent* event = [NSEvent otherEventWithType:NSApplicationDefined
                                            location:NSMakePoint(0, 0)
                                       modifierFlags:0
                                           timestamp:[[NSDate date] timeIntervalSinceSystemStart]
                                        windowNumber:0
                                             context:nil
                                             subtype:0
                                               data1:0
                                               data2:0];
        [NSApp postEvent:event atStart:true];
    }
}

@implementation AntaresView

- (BOOL)acceptsFirstResponder {
    return YES;
}

- (BOOL)acceptsMouseMovedEvents {
    return YES;
}

- (void)keyDown:(NSEvent*)event {
    if (![event isARepeat]) {
        key_down(self, event);
    }
}

- (void)keyUp:(NSEvent*)event {
    key_up(self, event);
}

- (void)flagsChanged:(NSEvent*)event {
    flags_changed(self, event);
}

- (void)mouseDown:(NSEvent*)event {
    mouse_down(self, event);
}

- (void)mouseUp:(NSEvent*)event {
    mouse_up(self, event);
}

- (void)mouseDragged:(NSEvent*)event {
    mouse_move(self, event);
}

- (void)mouseMoved:(NSEvent*)event {
    mouse_move(self, event);
}

@end

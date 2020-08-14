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

@implementation NSString (AntaresAdditions)
- (NSData*)utf8Data {
    return [self dataUsingEncoding:NSUTF8StringEncoding];
}

- (NSUInteger)utf8Length {
    return [self utf8Data].length;
}

- (NSUInteger)utf8IndexOf:(NSUInteger)runeIndex {
    return [self substringToIndex:runeIndex].utf8Length;
}
@end

typedef enum {
    SelectionDirectionNeither,
    SelectionDirectionRight,
    SelectionDirectionLeft,
} SelectionDirection;

@interface AntaresView : NSOpenGLView <NSTextInputClient, NSWindowDelegate> {
  @public
    void (*callback)(
            antares_window_callback_type type, antares_window_callback_data data, void* userdata);

    void* userdata;

    SelectionDirection selectionDir;
    int32_t            modifier_flags;
}
- (BOOL)isEditing;
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

AntaresWindow* antares_window_create(
        CGLPixelFormatObj pixel_format, CGLContextObj context, bool fullscreen, int width,
        int height) {
    AntaresWindow window = {};
    window.pixel_format  = [[NSOpenGLPixelFormat alloc] initWithCGLPixelFormatObj:pixel_format];
    window.context       = [[NSOpenGLContext alloc] initWithCGLContextObj:context];

    NSRect window_rect = NSMakeRect(0, 0, width, height);
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
    if (fullscreen) {
        [window.window toggleFullScreen:nil];
    } else {
        [window.window center];
    }
    window.window.delegate = window.view;
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

void antares_window_set_callback(
        AntaresWindow* window,
        void (*callback)(
                antares_window_callback_type type, antares_window_callback_data data,
                void* userdata),
        void* userdata) {
    window->view->callback = callback;
    window->view->userdata = userdata;
}

static void key_down(AntaresView* view, NSEvent* event) {
    NSPoint where;
    if (!translate_coords(view, event, &where)) {
        return;
    }
    antares_window_callback_data data = {.key_down = [event keyCode]};
    view->callback(ANTARES_WINDOW_CALLBACK_KEY_DOWN, data, view->userdata);
}

static void key_up(AntaresView* view, NSEvent* event) {
    NSPoint where;
    if (!translate_coords(view, event, &where)) {
        return;
    }
    antares_window_callback_data data = {.key_up = [event keyCode]};
    view->callback(ANTARES_WINDOW_CALLBACK_KEY_UP, data, view->userdata);
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

static void mouse_down(AntaresView* view, NSEvent* event, int button) {
    NSPoint where;
    if (!translate_coords(view, event, &where)) {
        return;
    }
    hide_unhide(view, where);
    antares_window_callback_data data = {
            .mouse_down = {button, where.x, where.y, [event clickCount]}};
    view->callback(ANTARES_WINDOW_CALLBACK_MOUSE_DOWN, data, view->userdata);
}

static void mouse_up(AntaresView* view, NSEvent* event, int button) {
    NSPoint where;
    if (!translate_coords(view, event, &where)) {
        return;
    }
    hide_unhide(view, where);
    antares_window_callback_data data = {.mouse_up = {button, where.x, where.y}};
    view->callback(ANTARES_WINDOW_CALLBACK_MOUSE_UP, data, view->userdata);
}

static void mouse_move(AntaresView* view, NSEvent* event) {
    NSPoint where;
    if (!translate_coords(view, event, &where)) {
        return;
    }
    hide_unhide(view, where);
    antares_window_callback_data data = {.mouse_move = {where.x, where.y}};
    view->callback(ANTARES_WINDOW_CALLBACK_MOUSE_MOVE, data, view->userdata);
}

static void flags_changed(AntaresView* view, NSEvent* event) {
    int32_t old_flags = view->modifier_flags;
    int32_t new_flags = view->modifier_flags = [event modifierFlags];
    if ([event keyCode] == 0x39) {  // Caps Lock
        antares_window_callback_data data = {};
        if ((new_flags | old_flags) == new_flags) {
            view->callback(ANTARES_WINDOW_CALLBACK_CAPS_LOCK, data, view->userdata);
        } else {
            view->callback(ANTARES_WINDOW_CALLBACK_CAPS_UNLOCK, data, view->userdata);
        }
        return;
    }
    if ((new_flags | old_flags) == new_flags) {
        antares_window_callback_data data = {.key_down = [event keyCode]};
        view->callback(ANTARES_WINDOW_CALLBACK_KEY_DOWN, data, view->userdata);
    } else {
        antares_window_callback_data data = {.key_up = [event keyCode]};
        view->callback(ANTARES_WINDOW_CALLBACK_KEY_UP, data, view->userdata);
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

        // Unless in text mode, keypresses should be handled directly
        // by the view; Cmd+A should trigger the keypresses for Cmd and
        // A, not the Select All menu item.
        if (!window->view.isEditing) {
            switch ([event type]) {
                case NSEventTypeKeyDown: [window->view keyDown:event]; return true;
                case NSEventTypeKeyUp: [window->view keyUp:event]; return true;
                case NSEventTypeFlagsChanged: [window->view flagsChanged:event]; return true;
                default: break;
            }
        }
        [NSApp sendEvent:event];
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

static const NSRange kNoRange = {NSNotFound, 0};
static BOOL          isNoRange(NSRange range) { return NSEqualRanges(range, kNoRange); }

@implementation AntaresView

// NSResponder

- (BOOL)acceptsFirstResponder {
    return YES;
}

- (BOOL)acceptsMouseMovedEvents {
    return YES;
}

- (void)keyDown:(NSEvent*)event {
    if (self.isEditing) {
        [self interpretKeyEvents:[NSArray arrayWithObject:event]];
    } else {
        if (![event isARepeat]) {
            key_down(self, event);
        }
    }
}

- (void)keyUp:(NSEvent*)event {
    key_up(self, event);
}

- (void)flagsChanged:(NSEvent*)event {
    flags_changed(self, event);
}

- (void)mouseDown:(NSEvent*)event {
    mouse_down(self, event, 0);
}

- (void)mouseUp:(NSEvent*)event {
    mouse_up(self, event, 0);
}

- (void)rightMouseDown:(NSEvent*)event {
    mouse_down(self, event, 1);
}

- (void)rightMouseUp:(NSEvent*)event {
    mouse_up(self, event, 1);
}

- (void)otherMouseDown:(NSEvent*)event {
    mouse_down(self, event, 2);
}

- (void)otherMouseUp:(NSEvent*)event {
    mouse_up(self, event, 2);
}

- (void)mouseDragged:(NSEvent*)event {
    mouse_move(self, event);
}

- (void)mouseMoved:(NSEvent*)event {
    mouse_move(self, event);
}

// NSWindowDelegate

- (void)windowDidResize:(NSNotification*)notification {
    antares_window_callback_data data = {.resize = {
                                                 .width  = self.bounds.size.width,
                                                 .height = self.bounds.size.height,
                                         }};
    callback(ANTARES_WINDOW_CALLBACK_RESIZE, data, userdata);
}

- (void)windowWillEnterFullScreen:(NSNotification*)notification {
    antares_window_callback_data data = {.fullscreen = true};
    callback(ANTARES_WINDOW_CALLBACK_FULLSCREEN, data, userdata);
}

- (void)windowDidExitFullScreen:(NSNotification*)notification {
    antares_window_callback_data data = {.fullscreen = false};
    callback(ANTARES_WINDOW_CALLBACK_FULLSCREEN, data, userdata);
}

// NSResponder

- (void)doCommandBySelector:(SEL)selector {
    [super doCommandBySelector:selector];
}

- (void)moveTo:(antares_window_callback_offset)to unit:(antares_window_callback_unit)unit {
    NSRange selection = self.selectedRange;
    if (selection.length > 0) {
        [self selectAt:(to > 0) ? NSMaxRange(selection) : selection.location];
    } else {
        [self selectAt:[self offset:selection.location to:to unit:unit]];
    }
}

- (void)moveRight:(id)sender {
    [self moveTo:ANTARES_WINDOW_CALLBACK_OFFSET_NEXT_START
            unit:ANTARES_WINDOW_CALLBACK_UNIT_GLYPHS];
}

- (void)moveLeft:(id)sender {
    [self moveTo:ANTARES_WINDOW_CALLBACK_OFFSET_PREV_START
            unit:ANTARES_WINDOW_CALLBACK_UNIT_GLYPHS];
}

- (void)moveWordRight:(id)sender {
    [self moveTo:ANTARES_WINDOW_CALLBACK_OFFSET_NEXT_END unit:ANTARES_WINDOW_CALLBACK_UNIT_WORDS];
}

- (void)moveWordLeft:(id)sender {
    [self moveTo:ANTARES_WINDOW_CALLBACK_OFFSET_PREV_START
            unit:ANTARES_WINDOW_CALLBACK_UNIT_WORDS];
}

- (void)moveDown:(id)sender {
    [self moveTo:ANTARES_WINDOW_CALLBACK_OFFSET_NEXT_SAME unit:ANTARES_WINDOW_CALLBACK_UNIT_LINES];
}

- (void)moveUp:(id)s {
    [self moveTo:ANTARES_WINDOW_CALLBACK_OFFSET_PREV_SAME unit:ANTARES_WINDOW_CALLBACK_UNIT_LINES];
}

- (void)moveToRightEndOfLine:(id)sender {
    [self moveTo:ANTARES_WINDOW_CALLBACK_OFFSET_THIS_END unit:ANTARES_WINDOW_CALLBACK_UNIT_LINES];
}

- (void)moveToLeftEndOfLine:(id)s {
    [self moveTo:ANTARES_WINDOW_CALLBACK_OFFSET_THIS_START
            unit:ANTARES_WINDOW_CALLBACK_UNIT_LINES];
}

- (void)moveParagraphForward:(id)sender {
    [self moveTo:ANTARES_WINDOW_CALLBACK_OFFSET_NEXT_END
            unit:ANTARES_WINDOW_CALLBACK_UNIT_PARAGRAPHS];
}

- (void)moveParagraphBackward:(id)s {
    [self moveTo:ANTARES_WINDOW_CALLBACK_OFFSET_PREV_START
            unit:ANTARES_WINDOW_CALLBACK_UNIT_PARAGRAPHS];
}

- (void)moveToEndOfParagraph:(id)sender {
    [self moveTo:ANTARES_WINDOW_CALLBACK_OFFSET_THIS_END
            unit:ANTARES_WINDOW_CALLBACK_UNIT_PARAGRAPHS];
}

- (void)moveToBeginningOfParagraph:(id)s {
    [self moveTo:ANTARES_WINDOW_CALLBACK_OFFSET_THIS_START
            unit:ANTARES_WINDOW_CALLBACK_UNIT_PARAGRAPHS];
}

- (void)moveToEndOfDocument:(id)sender {
    [self selectAt:self.textLength];
}

- (void)moveToBeginningOfDocument:(id)sender {
    [self selectAt:0];
}

- (void)moveAndModifySelectionTo:(antares_window_callback_offset)to
                            unit:(antares_window_callback_unit)unit {
    NSRange selection = self.selectedRange;
    int     begin     = selection.location;
    int     end       = NSMaxRange(selection);

    SelectionDirection forwards = (to > 0) ? SelectionDirectionRight : SelectionDirectionLeft;
    if (selectionDir == SelectionDirectionNeither) {
        selectionDir = forwards;
    }

    int* cursor = (selectionDir == SelectionDirectionRight) ? &end : &begin;
    int  anchor = (selectionDir == SelectionDirectionRight) ? begin : end;
    *cursor     = [self offset:*cursor to:to unit:unit];
    if (begin > end) {
        *cursor = anchor;
    }

    [self selectFrom:begin to:end in:selectionDir];
}

- (void)moveRightAndModifySelection:(id)sender {
    [self moveAndModifySelectionTo:ANTARES_WINDOW_CALLBACK_OFFSET_NEXT_START
                              unit:ANTARES_WINDOW_CALLBACK_UNIT_GLYPHS];
}

- (void)moveLeftAndModifySelection:(id)sender {
    [self moveAndModifySelectionTo:ANTARES_WINDOW_CALLBACK_OFFSET_PREV_START
                              unit:ANTARES_WINDOW_CALLBACK_UNIT_GLYPHS];
}

- (void)moveWordRightAndModifySelection:(id)sender {
    [self moveAndModifySelectionTo:ANTARES_WINDOW_CALLBACK_OFFSET_NEXT_END
                              unit:ANTARES_WINDOW_CALLBACK_UNIT_WORDS];
}

- (void)moveWordLeftAndModifySelection:(id)sender {
    [self moveAndModifySelectionTo:ANTARES_WINDOW_CALLBACK_OFFSET_PREV_START
                              unit:ANTARES_WINDOW_CALLBACK_UNIT_WORDS];
}

- (void)moveDownAndModifySelection:(id)s {
    [self moveAndModifySelectionTo:ANTARES_WINDOW_CALLBACK_OFFSET_NEXT_SAME
                              unit:ANTARES_WINDOW_CALLBACK_UNIT_LINES];
}

- (void)moveUpAndModifySelection:(id)s {
    [self moveAndModifySelectionTo:ANTARES_WINDOW_CALLBACK_OFFSET_PREV_SAME
                              unit:ANTARES_WINDOW_CALLBACK_UNIT_LINES];
}

- (void)moveToRightEndOfLineAndModifySelection:(id)s {
    [self moveAndModifySelectionTo:ANTARES_WINDOW_CALLBACK_OFFSET_THIS_END
                              unit:ANTARES_WINDOW_CALLBACK_UNIT_LINES];
}

- (void)moveToLeftEndOfLineAndModifySelection:(id)s {
    [self moveAndModifySelectionTo:ANTARES_WINDOW_CALLBACK_OFFSET_PREV_END
                              unit:ANTARES_WINDOW_CALLBACK_UNIT_LINES];
}

- (void)moveParagraphForwardAndModifySelection:(id)s {
    [self moveAndModifySelectionTo:ANTARES_WINDOW_CALLBACK_OFFSET_NEXT_START
                              unit:ANTARES_WINDOW_CALLBACK_UNIT_PARAGRAPHS];
}

- (void)moveParagraphBackwardAndModifySelection:(id)s {
    [self moveAndModifySelectionTo:ANTARES_WINDOW_CALLBACK_OFFSET_PREV_START
                              unit:ANTARES_WINDOW_CALLBACK_UNIT_PARAGRAPHS];
}

- (void)moveToEndOfParagraphAndModifySelection:(id)s {
    [self moveAndModifySelectionTo:ANTARES_WINDOW_CALLBACK_OFFSET_THIS_END
                              unit:ANTARES_WINDOW_CALLBACK_UNIT_PARAGRAPHS];
}

- (void)moveToBeginningOfParagraphAndModifySelection:(id)s {
    [self moveAndModifySelectionTo:ANTARES_WINDOW_CALLBACK_OFFSET_THIS_START
                              unit:ANTARES_WINDOW_CALLBACK_UNIT_PARAGRAPHS];
}

- (void)moveToEndOfDocumentAndModifySelection:(id)sender {
    NSRange selection = self.selectedRange;
    if (selectionDir == SelectionDirectionNeither) {
        selectionDir = SelectionDirectionRight;
    }
    [self selectFrom:selection.location to:self.textLength in:selectionDir];
}

- (void)moveToBeginningOfDocumentAndModifySelection:(id)sender {
    NSRange selection = self.selectedRange;
    if (selectionDir == SelectionDirectionNeither) {
        selectionDir = SelectionDirectionLeft;
    }
    [self selectFrom:0 to:NSMaxRange(selection) in:selectionDir];
}

- (void)insertTab:(id)sender {
    antares_window_callback_data data = {};
    callback(ANTARES_WINDOW_CALLBACK_TAB, data, userdata);
}

- (void)insertTabIgnoringFieldEditor:(id)sender {
    [self insertTab:sender];
}

- (void)insertBacktab:(id)sender {
    [self insertTab:sender];
}

- (void)insertNewline:(id)sender {
    antares_window_callback_data data = {};
    callback(ANTARES_WINDOW_CALLBACK_ACCEPT, data, userdata);
}

- (void)insertNewlineIgnoringFieldEditor:(id)sender {
    antares_window_callback_data data = {};
    callback(ANTARES_WINDOW_CALLBACK_NEWLINE, data, userdata);
}

- (void)insertSingleQuoteIgnoringSubstitution:(id)sender {
    [self replaceRange:self.selectedRange with:@"'"];
}

- (void)insertDoubleQuoteIgnoringSubstitution:(id)sender {
    [self replaceRange:self.selectedRange with:@"\""];
}

- (void)selectAll:(id)sender {
    [self selectFrom:0 to:self.textLength in:SelectionDirectionNeither];
}

- (void)delete:(id)sender {
    [self replaceRange:self.selectedRange with:@""];
}

- (void)deleteTo:(antares_window_callback_offset)to unit:(antares_window_callback_unit)unit {
    NSRange selection = self.selectedRange;
    if (selection.length > 0) {
        [self replaceRange:selection with:@""];
    } else {
        [self replaceFrom:[self offset:selection.location to:to unit:unit]
                       to:selection.location
                     with:@""];
    }
}

- (void)deleteBackward:(id)sender {
    [self deleteTo:ANTARES_WINDOW_CALLBACK_OFFSET_PREV_START
              unit:ANTARES_WINDOW_CALLBACK_UNIT_GLYPHS];
}

- (void)deleteForward:(id)sender {
    [self deleteTo:ANTARES_WINDOW_CALLBACK_OFFSET_NEXT_START
              unit:ANTARES_WINDOW_CALLBACK_UNIT_GLYPHS];
}

- (void)deleteWordBackward:(id)s {
    [self deleteTo:ANTARES_WINDOW_CALLBACK_OFFSET_PREV_START
              unit:ANTARES_WINDOW_CALLBACK_UNIT_WORDS];
}

- (void)deleteWordForward:(id)s {
    [self deleteTo:ANTARES_WINDOW_CALLBACK_OFFSET_NEXT_END
              unit:ANTARES_WINDOW_CALLBACK_UNIT_WORDS];
}

- (void)deleteToBeginningOfLine:(id)sender {
    [self deleteTo:ANTARES_WINDOW_CALLBACK_OFFSET_THIS_START
              unit:ANTARES_WINDOW_CALLBACK_UNIT_LINES];
}

- (void)deleteToEndOfLine:(id)sender {
    [self deleteTo:ANTARES_WINDOW_CALLBACK_OFFSET_THIS_END
              unit:ANTARES_WINDOW_CALLBACK_UNIT_LINES];
}

- (void)cancelOperation:(id)sender {
    antares_window_callback_data data = {};
    callback(ANTARES_WINDOW_CALLBACK_ESCAPE, data, userdata);
}

// clang-format off
- (void)moveForward:(id)s { [self moveRight:s]; }
- (void)moveForwardAndModifySelection:(id)s { [self moveRightAndModifySelection:s]; }
- (void)moveBackward:(id)s { [self moveLeft:s]; }
- (void)moveBackwardAndModifySelection:(id)s { [self moveLeftAndModifySelection:s]; }

- (void)pageDown:(id)s { [self moveToEndOfDocument:s]; }
- (void)pageDownAndModifySelection:(id)s { [self moveToEndOfDocumentAndModifySelection:s]; }
- (void)pageUp:(id)s { [self moveToBeginningOfDocument:s]; }
- (void)pageUpAndModifySelection:(id)s { [self moveToBeginningOfDocumentAndModifySelection:s]; }

- (void)deleteBackwardByDecomposingPreviousCharacter:(id)s { [self deleteBackward:s]; }
// clang-format on

- (void)cut:(id)sender {
    [self copy:sender];
    [self delete:sender];
}

- (void)copy:(id)sender {
    [[NSPasteboard generalPasteboard] declareTypes:[NSArray arrayWithObject:NSPasteboardTypeString]
                                             owner:nil];
    [[NSPasteboard generalPasteboard] setString:[self textInRange:self.selectedRange]
                                        forType:NSPasteboardTypeString];
}

- (void)paste:(id)sender {
    [self replaceRange:self.selectedRange
                  with:[[NSPasteboard generalPasteboard] stringForType:NSPasteboardTypeString]];
}

- (BOOL)validateUserInterfaceItem:(id<NSValidatedUserInterfaceItem>)item {
    if (!self.isEditing) {
        return false;
    } else if (
            (item.action == @selector(delete:)) || (item.action == @selector(cut:)) ||
            (item.action == @selector(copy:))) {
        return self.selectedRange.length > 0;
    } else if (item.action == @selector(paste:)) {
        return [[NSPasteboard generalPasteboard]
                       availableTypeFromArray:@[ NSPasteboardTypeString ]] != nil;
    } else if (item.action == @selector(selectAll:)) {
        return true;
    }
    return false;
}

// NSTextInputClient

- (void)insertString:(NSString*)string replacementRange:(NSRange)replacementRange {
    const NSRange baseRange = self.hasMarkedText ? self.markedRange : self.selectedRange;
    if (isNoRange(replacementRange)) {
        replacementRange = baseRange;
    } else {
        replacementRange.location += baseRange.location;
        if (NSMaxRange(replacementRange) > NSMaxRange(baseRange)) {
            replacementRange.length = NSMaxRange(baseRange) - replacementRange.location;
        }
    }

    [self replaceRange:replacementRange with:string];
}

- (void)insertText:(id)string replacementRange:(NSRange)replacementRange {
    if ([string isKindOfClass:[NSString class]]) {
        [self insertString:string replacementRange:replacementRange];
    } else if ([string isKindOfClass:[NSAttributedString class]]) {
        [self insertString:[string string] replacementRange:replacementRange];
    }
}

- (void)setMarkedString:(NSString*)string
           selectedRange:(NSRange)selectedRange
        replacementRange:(NSRange)replacementRange {
    const NSRange baseRange = self.hasMarkedText ? self.markedRange : self.selectedRange;
    if (isNoRange(replacementRange)) {
        replacementRange = baseRange;
    } else {
        replacementRange.location += baseRange.location;
        if (NSMaxRange(replacementRange) > NSMaxRange(baseRange)) {
            replacementRange.length = NSMaxRange(baseRange) - replacementRange.location;
        }
    }
    NSUInteger selectedStart =
            replacementRange.location + [string utf8IndexOf:selectedRange.location];
    NSUInteger selectedEnd =
            replacementRange.location + [string utf8IndexOf:NSMaxRange(selectedRange)];

    [self replaceRange:replacementRange with:string];
    [self selectFrom:selectedStart to:selectedEnd in:SelectionDirectionNeither];
    [self markText:NSMakeRange(replacementRange.location, string.utf8Length)];
}

- (void)setMarkedText:(id)string
           selectedRange:(NSRange)selectedRange
        replacementRange:(NSRange)replacementRange {
    if ([string isKindOfClass:[NSString class]]) {
        [self setMarkedString:string
                   selectedRange:selectedRange
                replacementRange:replacementRange];
    } else if ([string isKindOfClass:[NSAttributedString class]]) {
        [self setMarkedString:[string string]
                   selectedRange:selectedRange
                replacementRange:replacementRange];
    }
}

- (void)unmarkText {
    [self markText:kNoRange];
}

- (NSRange)selectedRange {
    antares_window_callback_range selection;
    antares_window_callback_data  data = {.get_selection = &selection};
    callback(ANTARES_WINDOW_CALLBACK_GET_SELECTION, data, userdata);
    return NSMakeRange(selection.begin, selection.end - selection.begin);
}

- (NSRange)markedRange {
    antares_window_callback_range mark;
    antares_window_callback_data  data = {.get_mark = &mark};
    callback(ANTARES_WINDOW_CALLBACK_GET_MARK, data, userdata);
    if (mark.begin == mark.end) {
        return kNoRange;
    }
    return NSMakeRange(mark.begin, mark.end - mark.begin);
}

- (BOOL)hasMarkedText {
    return !isNoRange(self.markedRange);
}

- (NSUInteger)offset:(NSUInteger)origin
                  to:(antares_window_callback_offset)to
                unit:(antares_window_callback_unit)unit {
    int                          offset;
    antares_window_callback_data data = {.get_offset = {origin, to, unit, &offset}};
    callback(ANTARES_WINDOW_CALLBACK_GET_OFFSET, data, userdata);
    return offset;
}

- (NSAttributedString*)attributedSubstringForProposedRange:(NSRange)range
                                               actualRange:(NSRangePointer)actualRange {
    return nil;
}

- (NSArray*)validAttributesForMarkedText {
    return [NSArray array];
}

- (NSRect)firstRectForCharacterRange:(NSRange)range actualRange:(NSRangePointer)actualRange {
    return NSMakeRect(0, 0, 0, 0);
}

- (NSUInteger)characterIndexForPoint:(NSPoint)point {
    return 0;
}

// Helpers

- (BOOL)isEditing {
    bool                         editing;
    antares_window_callback_data data = {.get_editing = &editing};
    callback(ANTARES_WINDOW_CALLBACK_GET_EDITING, data, userdata);
    return editing;
}

- (NSUInteger)textLength {
    int                          size;
    antares_window_callback_data data = {.get_size = &size};
    callback(ANTARES_WINDOW_CALLBACK_GET_SIZE, data, userdata);
    return size;
}

- (void)replaceFrom:(NSUInteger)from to:(NSUInteger)to with:(NSString*)string {
    [self replaceRange:NSMakeRange(from, to - from) with:string];
}

- (void)replaceRange:(NSRange)range with:(NSString*)string {
    NSData*                      utf8 = [string utf8Data];
    antares_window_callback_data data = {.replace = {
                                                 .range = {range.location, NSMaxRange(range)},
                                                 .data  = [utf8 bytes],
                                                 .size  = [utf8 length],
                                         }};
    callback(ANTARES_WINDOW_CALLBACK_REPLACE, data, userdata);
    selectionDir = SelectionDirectionNeither;
}

- (void)selectAt:(NSUInteger)at {
    [self selectFrom:at to:at in:SelectionDirectionNeither];
}

- (void)selectFrom:(NSUInteger)from to:(NSUInteger)to in:(SelectionDirection)direction {
    antares_window_callback_data data = {.select = {from, to}};
    callback(ANTARES_WINDOW_CALLBACK_SELECT, data, userdata);
    selectionDir = (from < to) ? direction : SelectionDirectionNeither;
}

- (void)markText:(NSRange)range {
    antares_window_callback_data data = {.mark = {range.location, NSMaxRange(range)}};
    callback(ANTARES_WINDOW_CALLBACK_MARK, data, userdata);
}

- (NSString*)textInRange:(NSRange)range {
    const char*                  bytes;
    int                          length;
    antares_window_callback_data data = {
            .get_text = {{range.location, NSMaxRange(range)}, &bytes, &length}};
    callback(ANTARES_WINDOW_CALLBACK_GET_TEXT, data, userdata);
    return [[[NSString alloc] initWithBytes:bytes length:length
                                   encoding:NSUTF8StringEncoding] autorelease];
}

@end

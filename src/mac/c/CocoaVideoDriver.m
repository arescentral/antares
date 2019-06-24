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

@interface AntaresView : NSOpenGLView <NSTextInputClient> {
  @public
    antares_window_text_callback_range (*text_callback)(
            antares_window_text_callback_type type, int int_start, int int_end,
            const char* char_start, const char* char_end, void* userdata);
    void* text_userdata;

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

    SelectionDirection selectionDir;
    int32_t            modifier_flags;
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

void antares_window_set_text_callback(
        AntaresWindow* window,
        antares_window_text_callback_range (*callback)(
                antares_window_text_callback_type type, int int_start, int int_end,
                const char* char_start, const char* char_end, void* userdata),
        void* userdata) {
    window->view->text_callback = callback;
    window->view->text_userdata = userdata;
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

        // Unless in text mode, keypresses should be handled directly
        // by the view; Cmd+A should trigger the keypresses for Cmd and
        // A, not the Select All menu item.
        if (!window->view->text_callback) {
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
    if (text_callback) {
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

// NSResponder

- (void)doCommandBySelector:(SEL)selector {
    [super doCommandBySelector:selector];
}

- (void)moveRight:(id)sender {
    NSRange selection = self.selectedRange;
    if (selection.length > 0) {
        [self selectAt:NSMaxRange(selection)];
    } else {
        [self selectAt:[self offset:selection.location by:1]];
    }
}

- (void)moveToEndOfDocument:(id)sender {
    [self selectAt:self.textLength];
}

- (void)moveLeft:(id)sender {
    NSRange selection = self.selectedRange;
    if (selection.length > 0) {
        [self selectAt:selection.location];
    } else {
        [self selectAt:[self offset:selection.location by:-1]];
    }
}

- (void)moveToBeginningOfDocument:(id)sender {
    [self selectAt:0];
}

- (void)moveRightAndModifySelection:(id)sender {
    NSRange selection = self.selectedRange;
    if (selectionDir == SelectionDirectionNeither) {
        selectionDir = SelectionDirectionRight;
    }
    if (selectionDir == SelectionDirectionLeft) {
        [self selectFrom:[self offset:selection.location by:1]
                      to:NSMaxRange(selection)
                      in:SelectionDirectionLeft];
    } else {
        [self selectFrom:selection.location
                      to:[self offset:NSMaxRange(selection) by:1]
                      in:SelectionDirectionRight];
    }
}

- (void)moveToEndOfDocumentAndModifySelection:(id)sender {
    NSRange selection = self.selectedRange;
    if (selectionDir == SelectionDirectionNeither) {
        selectionDir = SelectionDirectionRight;
    }
    [self selectFrom:selection.location to:self.textLength in:selectionDir];
}

- (void)moveLeftAndModifySelection:(id)sender {
    NSRange selection = self.selectedRange;
    if (selectionDir == SelectionDirectionNeither) {
        selectionDir = SelectionDirectionLeft;
    }
    if (selectionDir == SelectionDirectionRight) {
        [self selectFrom:selection.location
                      to:[self offset:NSMaxRange(selection) by:-1]
                      in:SelectionDirectionRight];
    } else {
        [self selectFrom:[self offset:selection.location by:-1]
                      to:NSMaxRange(selection)
                      in:SelectionDirectionLeft];
    }
}

- (void)moveToBeginningOfDocumentAndModifySelection:(id)sender {
    NSRange selection = self.selectedRange;
    if (selectionDir == SelectionDirectionNeither) {
        selectionDir = SelectionDirectionLeft;
    }
    [self selectFrom:0 to:NSMaxRange(selection) in:selectionDir];
}

- (void)insertTab:(id)sender {
    text_callback(ANTARES_WINDOW_TEXT_CALLBACK_TAB, 0, 0, nil, nil, text_userdata);
}

- (void)insertTabIgnoringFieldEditor:(id)sender {
    [self insertTab:sender];
}

- (void)insertBacktab:(id)sender {
    [self insertTab:sender];
}

- (void)insertNewline:(id)sender {
    text_callback(ANTARES_WINDOW_TEXT_CALLBACK_NEWLINE, 0, 0, nil, nil, text_userdata);
}

- (void)insertNewlineIgnoringFieldEditor:(id)sender {
    [self insertNewline:sender];
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

- (void)deleteBackward:(id)sender {
    NSRange selection = self.selectedRange;
    if (selection.length > 0) {
        [self replaceRange:selection with:@""];
    } else {
        [self replaceFrom:[self offset:selection.location by:-1] to:selection.location with:@""];
    }
}

- (void)deleteForward:(id)sender {
    NSRange selection = self.selectedRange;
    if (selection.length > 0) {
        [self replaceRange:selection with:@""];
    } else {
        [self replaceFrom:selection.location to:[self offset:selection.location by:1] with:@""];
    }
}

- (void)deleteToBeginningOfLine:(id)sender {
    NSRange selection = self.selectedRange;
    if (selection.length > 0) {
        [self replaceRange:selection with:@""];
    } else {
        [self replaceFrom:0 to:selection.location with:@""];
    }
}

- (void)deleteToEndOfLine:(id)sender {
    NSRange selection = self.selectedRange;
    if (selection.length > 0) {
        [self replaceRange:selection with:@""];
    } else {
        [self replaceFrom:selection.location to:self.textLength with:@""];
    }
}

- (void)cancelOperation:(id)sender {
    text_callback(ANTARES_WINDOW_TEXT_CALLBACK_ESCAPE, 0, 0, nil, nil, text_userdata);
}

// clang-format off
- (void)moveForward:(id)s { [self moveRight:s]; }
- (void)moveWordRight:(id)s { [self moveRight:s]; }
- (void)moveForwardAndModifySelection:(id)s { [self moveRightAndModifySelection:s]; }
- (void)moveWordRightAndModifySelection:(id)s { [self moveRightAndModifySelection:s]; }

- (void)moveBackward:(id)s { [self moveLeft:s]; }
- (void)moveWordLeft:(id)s { [self moveLeft:s]; }
- (void)moveBackwardAndModifySelection:(id)s { [self moveLeftAndModifySelection:s]; }
- (void)moveWordLeftAndModifySelection:(id)s { [self moveLeftAndModifySelection:s]; }

- (void)moveDown:(id)s { [self moveToEndOfDocument:s]; }
- (void)moveToRightEndOfLine:(id)s { [self moveToEndOfDocument:s]; }
- (void)moveParagraphForward:(id)s { [self moveToEndOfDocument:s]; }
- (void)moveToEndOfParagraph:(id)s { [self moveToEndOfDocument:s]; }
- (void)pageDown:(id)s { [self moveToEndOfDocument:s]; }
- (void)moveDownAndModifySelection:(id)s { [self moveToEndOfDocumentAndModifySelection:s]; }
- (void)moveToRightEndOfLineAndModifySelection:(id)s { [self moveToEndOfDocumentAndModifySelection:s]; }
- (void)moveParagraphForwardAndModifySelection:(id)s { [self moveToEndOfDocumentAndModifySelection:s]; }
- (void)moveToEndOfParagraphAndModifySelection:(id)s { [self moveToEndOfDocumentAndModifySelection:s]; }
- (void)pageDownAndModifySelection:(id)s { [self moveToEndOfDocumentAndModifySelection:s]; }

- (void)moveUp:(id)s { [self moveToBeginningOfDocument:s]; }
- (void)moveToLeftEndOfLine:(id)s { [self moveToBeginningOfDocument:s]; }
- (void)moveParagraphBackward:(id)s { [self moveToBeginningOfDocument:s]; }
- (void)moveToBeginningOfParagraph:(id)s { [self moveToBeginningOfDocument:s]; }
- (void)pageUp:(id)s { [self moveToBeginningOfDocument:s]; }
- (void)moveUpAndModifySelection:(id)s { [self moveToBeginningOfDocumentAndModifySelection:s]; }
- (void)moveToLeftEndOfLineAndModifySelection:(id)s { [self moveToBeginningOfDocumentAndModifySelection:s]; }
- (void)moveParagraphBackwardAndModifySelection:(id)s { [self moveToBeginningOfDocumentAndModifySelection:s]; }
- (void)moveToBeginningOfParagraphAndModifySelection:(id)s { [self moveToBeginningOfDocumentAndModifySelection:s]; }
- (void)pageUpAndModifySelection:(id)s { [self moveToBeginningOfDocumentAndModifySelection:s]; }

- (void)deleteWordBackward:(id)s { [self deleteBackward:s]; }
- (void)deleteBackwardByDecomposingPreviousCharacter:(id)s { [self deleteBackward:s]; }
- (void)deleteWordForward:(id)s { [self deleteForward:s]; }
// clang-format on

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
    antares_window_text_callback_range selection = text_callback(
            ANTARES_WINDOW_TEXT_CALLBACK_GET_SELECTION, 0, 0, nil, nil, text_userdata);
    return NSMakeRange(selection.begin, selection.end - selection.begin);
}

- (NSRange)markedRange {
    antares_window_text_callback_range mark =
            text_callback(ANTARES_WINDOW_TEXT_CALLBACK_GET_MARK, 0, 0, nil, nil, text_userdata);
    if (mark.begin == mark.end) {
        return kNoRange;
    }
    return NSMakeRange(mark.begin, mark.end - mark.begin);
}

- (BOOL)hasMarkedText {
    return !isNoRange(self.markedRange);
}

- (NSUInteger)offset:(NSUInteger)origin by:(NSInteger)by {
    antares_window_text_callback_range offset = text_callback(
            ANTARES_WINDOW_TEXT_CALLBACK_GET_OFFSET, origin, origin + by, nil, nil, text_userdata);
    return offset.end - offset.begin;
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

- (NSUInteger)textLength {
    antares_window_text_callback_range text =
            text_callback(ANTARES_WINDOW_TEXT_CALLBACK_GET_SIZE, 0, 0, nil, nil, text_userdata);
    return text.end - text.begin;
}

- (void)replaceFrom:(NSUInteger)from to:(NSUInteger)to with:(NSString*)string {
    [self replaceRange:NSMakeRange(from, to - from) with:string];
}

- (void)replaceRange:(NSRange)range with:(NSString*)string {
    NSData*     utf8       = [string utf8Data];
    const char* utf8_start = [utf8 bytes];
    const char* utf8_end   = utf8_start + [utf8 length];
    text_callback(
            ANTARES_WINDOW_TEXT_CALLBACK_REPLACE, range.location, NSMaxRange(range), utf8_start,
            utf8_end, text_userdata);
    selectionDir = SelectionDirectionNeither;
}

- (void)selectAt:(NSUInteger)at {
    [self selectFrom:at to:at in:SelectionDirectionNeither];
}

- (void)selectFrom:(NSUInteger)from to:(NSUInteger)to in:(SelectionDirection)direction {
    text_callback(ANTARES_WINDOW_TEXT_CALLBACK_SELECT, from, to, nil, nil, text_userdata);
    selectionDir = (from < to) ? direction : SelectionDirectionNeither;
}

- (void)markText:(NSRange)range {
    text_callback(
            ANTARES_WINDOW_TEXT_CALLBACK_MARK, range.location, NSMaxRange(range), nil, nil,
            text_userdata);
}

@end

// Ares, a tactical space combat game.
// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// 
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

#include "CocoaVideoDriver.hpp"

#include <stdlib.h>
#include <strings.h>
#include <Cocoa/Cocoa.h>
#include <OpenGL/OpenGL.h>
#include <OpenGL/gl.h>
#include "sfz/Exception.hpp"
#include "Base.h"
#include "Card.hpp"
#include "CardStack.hpp"
#include "FakeDrawing.hpp"
#include "Geometry.hpp"
#include "Time.hpp"

using sfz::Exception;

namespace antares {

namespace {

int64_t usecs() {
    timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000000ll + tv.tv_usec;
}

Point translate_coords(NSPoint input) {
    const double width = CGDisplayPixelsWide(kCGDirectMainDisplay);
    const double height = CGDisplayPixelsHigh(kCGDirectMainDisplay);
    return Point(round(input.x / width * 640), 479 - round(input.y / height * 480));
}

void set_key(KeyMap map, int key, bool down) {
    uint32_t* byte = map + (key / 32);
    uint32_t mask = OSSwapInt32(1 << (key % 32));
    if (down) {
        *byte |= mask;
    } else {
        *byte &= ~mask;
    }
}

}  // namespace

CocoaVideoDriver::CocoaVideoDriver()
        : _start_time(usecs()),
          _button(false),
          _mouse() {
    bzero(_keys, sizeof(uint32_t[4]));
}

void CocoaVideoDriver::send_event(EventRecord) {
    throw Exception("CocoaVideoDriver::send_event() called");
}

void CocoaVideoDriver::mouse_down(int button, const Point& where) {
    _button = true;
    _mouse = where;

    EventRecord evt;
    evt.what = mouseDown;
    evt.where = _mouse;
    _event_queue.push(evt);
}

void CocoaVideoDriver::mouse_up(int button, const Point& where) {
    _button = false;
    _mouse = where;

    EventRecord evt;
    evt.what = mouseUp;
    evt.where = _mouse;
    _event_queue.push(evt);
}

void CocoaVideoDriver::key_down(int key_code) {
    set_key(_keys, key_code, true);

    EventRecord evt;
    evt.what = autoKey;
    evt.message = key_code << 8;
    _event_queue.push(evt);
}

void CocoaVideoDriver::key_up(int key_code) {
    set_key(_keys, key_code, false);

    EventRecord evt;
    evt.what = keyUp;
    evt.message = key_code << 8;
    _event_queue.push(evt);
}

void CocoaVideoDriver::flags_changed(int flags) {
    struct ModifierFlag {
        uint32_t bit;
        uint32_t code;
    };
    static const int modifier_flag_count = 5;
    static const ModifierFlag modifier_flags[modifier_flag_count] = {
        {NSAlphaShiftKeyMask, 0x39},  // caps lock.
        {NSShiftKeyMask,      0x38},
        {NSControlKeyMask,    0x3B},
        {NSAlternateKeyMask,  0x3A},
        {NSCommandKeyMask,    0x37},
    };

    for (int i = 0; i < modifier_flag_count; ++i) {
        if ((_last_modifiers ^ flags) & modifier_flags[i].bit) {
            EventRecord evt;
            evt.message = modifier_flags[i].code << 8;
            if (flags & modifier_flags[i].bit) {
                evt.what = autoKey;
                set_key(_keys, modifier_flags[i].code, true);
            } else {
                evt.what = keyUp;
                set_key(_keys, modifier_flags[i].code, false);
            }
            _event_queue.push(evt);
        }
    }
    _last_modifiers = flags;
}

void CocoaVideoDriver::enqueue_events(id until) {
    NSEvent* const event =
        [NSApp nextEventMatchingMask:NSAnyEventMask untilDate:until inMode:NSDefaultRunLoopMode
            dequeue:YES];
    if (event) {
        switch ([event type]) {
          case NSLeftMouseDown:
            mouse_down(0, translate_coords([event locationInWindow]));
            break;

          case NSLeftMouseUp:
            mouse_up(0, translate_coords([event locationInWindow]));
            break;

          case NSMouseMoved:
          case NSLeftMouseDragged:
            _mouse = translate_coords([event locationInWindow]);
            break;

          case NSKeyDown:
            key_down([event keyCode]);
            break;

          case NSKeyUp:
            key_up([event keyCode]);
            break;

          case NSFlagsChanged:
            flags_changed([event modifierFlags]);
            break;

          default:
            break;
        }
    }
}

bool CocoaVideoDriver::wait_next_event(EventRecord* evt, double sleep) {
    if (!_event_queue.empty()) {
        *evt = _event_queue.front();
        _event_queue.pop();
        return true;
    }

    NSDate* until = [NSDate dateWithTimeIntervalSinceNow:sleep];
    while ([until timeIntervalSinceNow] > 0) {
        enqueue_events(until);
        if (!_event_queue.empty()) {
            *evt = _event_queue.front();
            _event_queue.pop();
            return true;
        }
    };

    return false;
}

bool CocoaVideoDriver::button() {
    return _button;
}

Point CocoaVideoDriver::get_mouse() {
    return _mouse;
}

void CocoaVideoDriver::get_keys(KeyMap keys) {
    memcpy(keys, _keys, sizeof(KeyMap));
}

void CocoaVideoDriver::set_game_state(GameState state) {
}

int CocoaVideoDriver::get_demo_scenario() {
    int levels[] = { 0, 5, 23 };
    return levels[rand() % 3];
}

void CocoaVideoDriver::main_loop_iteration_complete(uint32_t) { }

int CocoaVideoDriver::ticks() {
    return (usecs() - _start_time) * 60 / 1000000;
}

void CocoaVideoDriver::loop(CardStack* stack) {
    int int_attrs[] = {
        NSOpenGLPFAFullScreen,
        NSOpenGLPFAScreenMask, CGDisplayIDToOpenGLDisplayMask(kCGDirectMainDisplay),
        NSOpenGLPFAColorSize, 24,
        NSOpenGLPFADepthSize, 16,
        NSOpenGLPFADoubleBuffer,
        NSOpenGLPFAAccelerated,
        0,
    };
    NSOpenGLPixelFormatAttribute* attrs
        = reinterpret_cast<NSOpenGLPixelFormatAttribute*>(int_attrs);

    NSOpenGLPixelFormat* pixel_format = [[NSOpenGLPixelFormat alloc] initWithAttributes:attrs];
    NSOpenGLContext* context
        = [[NSOpenGLContext alloc] initWithFormat:pixel_format shareContext:nil];
    [pixel_format release];

    if (context == nil) {
        throw Exception("failed to create context");
    }

    CGDisplayErr err = CGCaptureAllDisplays();
    if (err != CGDisplayNoErr) {
        throw Exception("CGCaptureAllDisplays() failed");
        [context release];
        return;
    }

    [context setFullScreen];
    [context makeCurrentContext];

    CGLContextObj cgl_context = CGLGetCurrentContext();
    GLint swap_interval = 1;
    CGLSetParameter(cgl_context, kCGLCPSwapInterval, &swap_interval);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glClearColor(0, 0, 0, 1);
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_TEXTURE_RECTANGLE_EXT);
    glBindTexture(GL_TEXTURE_RECTANGLE_EXT, 1);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glTextureRangeAPPLE(GL_TEXTURE_RECTANGLE_EXT, 0, NULL);
    glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);

    int width = gRealWorld->bounds().width();
    int height = gRealWorld->bounds().height();

    while (!stack->empty()) {
        NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];

        glClear(GL_COLOR_BUFFER_BIT);
        glLoadIdentity();
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();

        glColor4f(1, 1, 1, 1);
        glBindTexture(GL_TEXTURE_RECTANGLE_EXT, 1);
#if defined(__LITTLE_ENDIAN__)
        glTexImage2D(
                GL_TEXTURE_RECTANGLE_EXT, 0, GL_RGBA, width, height, 0, GL_BGRA,
                GL_UNSIGNED_INT_8_8_8_8, gRealWorld->bytes());
#elif defined(__BIG_ENDIAN__)
        glTexImage2D(
                GL_TEXTURE_RECTANGLE_EXT, 0, GL_RGBA, width, height, 0, GL_BGRA,
                GL_UNSIGNED_INT_8_8_8_8_REV, gRealWorld->bytes());
#else
#error "Couldn't determine endianness of platform"
#endif

        glBegin(GL_QUADS);
        glTexCoord2f(width, 0);
        glVertex2f(1, 1);
        glTexCoord2f(0, 0);
        glVertex2f(-1, 1);
        glTexCoord2f(0, height);
        glVertex2f(-1, -1);
        glTexCoord2f(width, height);
        glVertex2f(1, -1);
        glEnd();

        glPopMatrix();
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();

        const antares::RgbColor& transition = gRealWorld->transition_to();
        glBindTexture(GL_TEXTURE_RECTANGLE_EXT, 0);
        double f = gRealWorld->transition_fraction();
        glColor4ub(transition.red, transition.green, transition.blue, 0xff * f);
        glBegin(GL_QUADS);
        glVertex2f(1, 1);
        glVertex2f(-1, 1);
        glVertex2f(-1, -1);
        glVertex2f(1, -1);
        glEnd();

        glFinish();
        [context flushBuffer];

        EventRecord evt;
        double at;
        double now = now_secs();
        Card* card = stack->next_event(&at);
        if (wait_next_event(&evt, at - now)) {
            stack->send(evt);
        } else if (card) {
            card->fire_timer();
        }

        [pool release];
    }

    [context release];
    CGReleaseAllDisplays();
}

}  // namespace antares

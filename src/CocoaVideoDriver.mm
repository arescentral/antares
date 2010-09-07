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
#include <algorithm>
#include <Cocoa/Cocoa.h>
#include <OpenGL/OpenGL.h>
#include <OpenGL/gl.h>
#include <sfz/sfz.hpp>
#include "Base.h"
#include "Card.hpp"
#include "CardStack.hpp"
#include "Event.hpp"
#include "FakeDrawing.hpp"
#include "Geometry.hpp"
#include "Time.hpp"

using sfz::Exception;
using sfz::scoped_ptr;
using std::min;

namespace antares {

namespace {

int64_t usecs() {
    timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000000ll + tv.tv_usec;
}

bool translate_coords(
        const NSPoint& input, const Rect& bounds, const Rect& game_area, Point* output) {
    if (!bounds.contains(Point(input.x, input.y))) {
        return false;
    }
    double x_scale = static_cast<double>(game_area.width()) / bounds.width();
    double y_scale = static_cast<double>(game_area.height()) / bounds.height();
    *output = Point(
            round((input.x - bounds.left) * x_scale),
            game_area.height() - 1 - round((input.y - bounds.top) * y_scale));
    return true;
}

}  // namespace

CocoaVideoDriver::CocoaVideoDriver()
        : _start_time(usecs()),
          _button(false),
          _mouse() { }

void CocoaVideoDriver::mouse_down(int button, const Point& where) {
    _button = true;
    _mouse = where;
    _event_queue.push(new MouseDownEvent(button, where));
}

void CocoaVideoDriver::mouse_up(int button, const Point& where) {
    _button = false;
    _mouse = where;
    _event_queue.push(new MouseUpEvent(button, where));
}

void CocoaVideoDriver::key_down(int key_code) {
    _keys.set(key_code, true);
    _event_queue.push(new KeyDownEvent(key_code));
}

void CocoaVideoDriver::key_up(int key_code) {
    _keys.set(key_code, false);
    _event_queue.push(new KeyUpEvent(key_code));
}

void CocoaVideoDriver::flags_changed(int flags) {
    struct ModifierFlag {
        uint32_t bit;
        uint32_t code;
    };
    static const int modifier_flag_count = 5;
    static const ModifierFlag modifier_flags[modifier_flag_count] = {
        {NSAlphaShiftKeyMask, Keys::CAPS_LOCK},  // caps lock.
        {NSShiftKeyMask,      Keys::SHIFT},
        {NSControlKeyMask,    Keys::CONTROL},
        {NSAlternateKeyMask,  Keys::OPTION},
        {NSCommandKeyMask,    Keys::COMMAND},
    };

    for (int i = 0; i < modifier_flag_count; ++i) {
        if ((_last_modifiers ^ flags) & modifier_flags[i].bit) {
            if (flags & modifier_flags[i].bit) {
                key_down(modifier_flags[i].code);
            } else {
                key_up(modifier_flags[i].code);
            }
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
            {
                Point p;
                if (translate_coords([event locationInWindow], _bounds, _game_area, &p)) {
                    mouse_down(0, p);
                }
            }
            break;

          case NSLeftMouseUp:
            {
                Point p;
                if (translate_coords([event locationInWindow], _bounds, _game_area, &p)) {
                    mouse_up(0, p);
                }
            }
            break;

          case NSMouseMoved:
          case NSLeftMouseDragged:
            translate_coords([event locationInWindow], _bounds, _game_area, &_mouse);
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

Event* CocoaVideoDriver::wait_next_event(double sleep) {
    if (!_event_queue.empty()) {
        Event* event = _event_queue.front();
        _event_queue.pop();
        return event;
    }

    NSDate* until = [NSDate dateWithTimeIntervalSinceNow:sleep];
    while ([until timeIntervalSinceNow] > 0) {
        enqueue_events(until);
        if (!_event_queue.empty()) {
            Event* event = _event_queue.front();
            _event_queue.pop();
            return event;
        }
    };

    return NULL;
}

bool CocoaVideoDriver::button() {
    return _button;
}

Point CocoaVideoDriver::get_mouse() {
    return _mouse;
}

void CocoaVideoDriver::get_keys(KeyMap* keys) {
    keys->copy(_keys);
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
    glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);

    _game_area = gRealWorld->bounds();
    const Rect screen(
            0, 0, CGDisplayPixelsWide(kCGDirectMainDisplay),
            CGDisplayPixelsHigh(kCGDirectMainDisplay));
    _bounds = screen;
    _bounds.right = min(
            _bounds.right, _bounds.height() * _game_area.width() / _game_area.height());
    _bounds.bottom = min(
            _bounds.bottom, _bounds.width() * _game_area.height() / _game_area.width());
    _bounds.center_in(screen);

    while (!stack->empty()) {
        NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];

        glClear(GL_COLOR_BUFFER_BIT);
        glLoadIdentity();
        glViewport(_bounds.left, _bounds.top, _bounds.width(), _bounds.height());
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();

        glColor4f(1, 1, 1, 1);
        glBindTexture(GL_TEXTURE_RECTANGLE_EXT, 1);
#if defined(__LITTLE_ENDIAN__)
        glTexImage2D(
                GL_TEXTURE_RECTANGLE_EXT, 0, GL_RGBA, _game_area.width(), _game_area.height(), 0,
                GL_BGRA, GL_UNSIGNED_INT_8_8_8_8, gRealWorld->bytes());
#elif defined(__BIG_ENDIAN__)
        glTexImage2D(
                GL_TEXTURE_RECTANGLE_EXT, 0, GL_RGBA, _game_area.width(), _game_area.height(), 0,
                GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, gRealWorld->bytes());
#else
#error "Couldn't determine endianness of platform"
#endif

        glBegin(GL_QUADS);
        glTexCoord2f(_game_area.width(), 0);
        glVertex2f(1, 1);
        glTexCoord2f(0, 0);
        glVertex2f(-1, 1);
        glTexCoord2f(0, _game_area.height());
        glVertex2f(-1, -1);
        glTexCoord2f(_game_area.width(), _game_area.height());
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

        double at = stack->top()->next_timer();
        double now = now_secs();
        if (at == 0.0) {
            at = std::numeric_limits<double>::infinity();
        }
        scoped_ptr<Event> event(wait_next_event(at - now));
        if (event.get()) {
            event->send(stack->top());
        } else if (at != std::numeric_limits<double>::infinity()) {
            stack->top()->fire_timer();
        }

        [pool release];
    }

    [context release];
    CGReleaseAllDisplays();
}

}  // namespace antares

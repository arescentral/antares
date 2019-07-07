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

#ifndef ANTARES_MAC_C_COCOA_VIDEO_DRIVER_H_
#define ANTARES_MAC_C_COCOA_VIDEO_DRIVER_H_

#include <OpenGL/OpenGL.h>
#include <OpenGL/gl.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

bool antares_is_active();
void antares_menu_bar_hide();
void antares_menu_bar_show();
void antares_mouse_hide();
void antares_mouse_show();

typedef struct AntaresWindow AntaresWindow;
AntaresWindow* antares_window_create(CGLPixelFormatObj pixel_format, CGLContextObj context);
void           antares_window_destroy(AntaresWindow* window);
int32_t        antares_window_screen_width(const AntaresWindow* window);
int32_t        antares_window_screen_height(const AntaresWindow* window);
int32_t        antares_window_viewport_width(const AntaresWindow* window);
int32_t        antares_window_viewport_height(const AntaresWindow* window);

void antares_get_mouse_location(AntaresWindow* window, int32_t* x, int32_t* y);

typedef enum {
    ANTARES_WINDOW_CALLBACK_KEY_DOWN,
    ANTARES_WINDOW_CALLBACK_KEY_UP,
    ANTARES_WINDOW_CALLBACK_CAPS_LOCK,
    ANTARES_WINDOW_CALLBACK_CAPS_UNLOCK,

    ANTARES_WINDOW_CALLBACK_MOUSE_DOWN,
    ANTARES_WINDOW_CALLBACK_MOUSE_UP,
    ANTARES_WINDOW_CALLBACK_MOUSE_MOVE,

    ANTARES_WINDOW_CALLBACK_REPLACE,
    ANTARES_WINDOW_CALLBACK_SELECT,
    ANTARES_WINDOW_CALLBACK_MARK,
    ANTARES_WINDOW_CALLBACK_ACCEPT,
    ANTARES_WINDOW_CALLBACK_NEWLINE,
    ANTARES_WINDOW_CALLBACK_TAB,
    ANTARES_WINDOW_CALLBACK_ESCAPE,

    ANTARES_WINDOW_CALLBACK_GET_EDITING,
    ANTARES_WINDOW_CALLBACK_GET_OFFSET,
    ANTARES_WINDOW_CALLBACK_GET_SIZE,
    ANTARES_WINDOW_CALLBACK_GET_SELECTION,
    ANTARES_WINDOW_CALLBACK_GET_MARK,
    ANTARES_WINDOW_CALLBACK_GET_TEXT,
} antares_window_callback_type;

typedef struct {
    int begin, end;
} antares_window_callback_range;

typedef enum {
    ANTARES_WINDOW_CALLBACK_UNIT_GLYPHS           = 0,
    ANTARES_WINDOW_CALLBACK_UNIT_WORDS            = 1,
    ANTARES_WINDOW_CALLBACK_UNIT_LINES            = 2,
    ANTARES_WINDOW_CALLBACK_UNIT_LINE_GLYPHS      = 3,
    ANTARES_WINDOW_CALLBACK_UNIT_PARAGRAPHS       = 4,
    ANTARES_WINDOW_CALLBACK_UNIT_PARAGRAPH_GLYPHS = 5,
} antares_window_callback_unit;

typedef union {
    int key_down;
    int key_up;

    struct {
        int     button;
        int32_t x, y;
        int     count;
    } mouse_down;

    struct {
        int     button;
        int32_t x, y;
    } mouse_up;

    struct {
        int32_t x, y;
    } mouse_move;

    struct {
        antares_window_callback_range range;
        const char*                   data;
        int                           size;
    } replace;

    antares_window_callback_range select;

    antares_window_callback_range mark;

    bool* get_editing;

    struct {
        int                          origin, by;
        antares_window_callback_unit unit;
        int*                         offset;
    } get_offset;

    int*                           get_size;
    antares_window_callback_range* get_selection;
    antares_window_callback_range* get_mark;

    struct {
        antares_window_callback_range range;
        const char**                  data;
        int*                          size;
    } get_text;
} antares_window_callback_data;

void antares_window_set_callback(
        AntaresWindow* window,
        void (*callback)(
                antares_window_callback_type type, antares_window_callback_data data,
                void* userdata),
        void* userdata);

bool antares_window_next_event(AntaresWindow* window, int64_t until);
void antares_window_cancel_event(AntaresWindow* window);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // ANTARES_MAC_C_COCOA_VIDEO_DRIVER_H_

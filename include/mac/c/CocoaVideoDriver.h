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
    ANTARES_WINDOW_TEXT_CALLBACK_REPLACE,
    ANTARES_WINDOW_TEXT_CALLBACK_SELECT,
    ANTARES_WINDOW_TEXT_CALLBACK_MARK,
    ANTARES_WINDOW_TEXT_CALLBACK_NEWLINE,
    ANTARES_WINDOW_TEXT_CALLBACK_TAB,
    ANTARES_WINDOW_TEXT_CALLBACK_ESCAPE,

    ANTARES_WINDOW_TEXT_CALLBACK_GET_OFFSET,
    ANTARES_WINDOW_TEXT_CALLBACK_GET_SIZE,
    ANTARES_WINDOW_TEXT_CALLBACK_GET_SELECTION,
    ANTARES_WINDOW_TEXT_CALLBACK_GET_MARK,
} antares_window_text_callback_type;

typedef struct {
    int begin, end;
} antares_window_text_callback_range;

void antares_window_set_text_callback(
        AntaresWindow* window,
        antares_window_text_callback_range (*callback)(
                antares_window_text_callback_type type, int int_start, int int_end,
                const char* char_start, const char* char_end, void* userdata),
        void* userdata);
void antares_window_set_key_down_callback(
        AntaresWindow* window, void (*callback)(int key, void* userdata), void* userdata);
void antares_window_set_key_up_callback(
        AntaresWindow* window, void (*callback)(int key, void* userdata), void* userdata);
void antares_window_set_mouse_down_callback(
        AntaresWindow* window,
        void (*callback)(int button, int32_t x, int32_t y, int count, void* userdata),
        void* userdata);
void antares_window_set_mouse_up_callback(
        AntaresWindow* window, void (*callback)(int button, int32_t x, int32_t y, void* userdata),
        void*          userdata);
void antares_window_set_mouse_move_callback(
        AntaresWindow* window, void (*callback)(int32_t x, int32_t y, void* userdata),
        void*          userdata);
void antares_window_set_caps_lock_callback(
        AntaresWindow* window, void (*callback)(void* userdata), void* userdata);
void antares_window_set_caps_unlock_callback(
        AntaresWindow* window, void (*callback)(void* userdata), void* userdata);

bool antares_window_next_event(AntaresWindow* window, int64_t until);
void antares_window_cancel_event(AntaresWindow* window);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // ANTARES_MAC_C_COCOA_VIDEO_DRIVER_H_

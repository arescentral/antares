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

#ifndef ANTARES_COCOA_C_COCOA_VIDEO_DRIVER_H_
#define ANTARES_COCOA_C_COCOA_VIDEO_DRIVER_H_

#include <stdint.h>
#include <stdbool.h>
#include <OpenGL/OpenGL.h>
#include <OpenGL/gl.h>

#ifdef __cplusplus
extern "C" {
#endif

bool antares_is_active();
void antares_menu_bar_hide();
void antares_menu_bar_show();
void antares_mouse_hide();
void antares_mouse_show();
int64_t antares_double_click_interval_usecs();

typedef struct AntaresWindow AntaresWindow;
AntaresWindow* antares_window_create(
        CGLPixelFormatObj pixel_format, CGLContextObj context,
        int32_t screen_width, int32_t screen_height,
        bool fullscreen, bool retina);
void antares_window_destroy(AntaresWindow* window);
void antares_window_show(AntaresWindow* window);
void antares_window_hide(AntaresWindow* window);

typedef struct AntaresEventTranslator AntaresEventTranslator;
AntaresEventTranslator* antares_event_translator_create(
        int32_t screen_width, int32_t screen_height);
void antares_event_translator_destroy(AntaresEventTranslator* translator);

void antares_event_translator_set_window(
        AntaresEventTranslator* translator, AntaresWindow* window);
void antares_get_mouse_location(AntaresEventTranslator* translator, int32_t* x, int32_t* y);
void antares_get_mouse_button(AntaresEventTranslator* translator, int32_t* button, int which);

void antares_event_translator_set_mouse_down_callback(
        AntaresEventTranslator* translator,
        void (*callback)(int button, int32_t x, int32_t y, void* userdata), void* userdata);
void antares_event_translator_set_mouse_up_callback(
        AntaresEventTranslator* translator,
        void (*callback)(int button, int32_t x, int32_t y, void* userdata), void* userdata);
void antares_event_translator_set_mouse_move_callback(
        AntaresEventTranslator* translator,
        void (*callback)(int32_t x, int32_t y, void* userdata), void* userdata);
void antares_event_translator_set_caps_lock_callback(
        AntaresEventTranslator* translator,
        void (*callback)(void* userdata), void* userdata);
void antares_event_translator_set_caps_unlock_callback(
        AntaresEventTranslator* translator,
        void (*callback)(void* userdata), void* userdata);

bool antares_event_translator_next(AntaresEventTranslator* translator, int64_t until);
void antares_event_translator_cancel(AntaresEventTranslator* translator);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // ANTARES_COCOA_C_COCOA_VIDEO_DRIVER_H_

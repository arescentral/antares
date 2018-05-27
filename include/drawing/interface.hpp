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

#ifndef ANTARES_DRAWING_INTERFACE_HPP_
#define ANTARES_DRAWING_INTERFACE_HPP_

#include <pn/string>
#include <vector>

#include "data/interface.hpp"
#include "ui/event.hpp"

namespace antares {

enum class ButtonState {
    DISABLED,
    ENABLED,
    ACTIVE,
};

const int32_t kInterfaceTextVBuffer = 2;
const int32_t kInterfaceTextHBuffer = 3;

void draw_text_in_rect(Rect tRect, pn::string_view text, InterfaceStyle style, Hue hue);

int16_t GetInterfaceTextHeightFromWidth(pn::string_view text, InterfaceStyle style, int16_t width);

void draw_box_rect(Point origin, const BoxRectData& item);
void draw_labeled_box(Point origin, const BoxRectData& item);
void draw_text_rect(Point origin, const TextRectData& item);
void draw_picture_rect(Point origin, const PictureRectData& item, const Texture& texture);
void draw_button(Point origin, InputMode mode, const PlainButtonData& item, ButtonState state);
void draw_checkbox(Point origin, const CheckboxButtonData& item, ButtonState state, bool on);
void draw_tab_box(Point origin, const TabBoxData& item);
void draw_tab_box_button(Point origin, const TabBoxButtonData& item, ButtonState state, bool on);

Rect box_rect_bounds(const BoxRectData& item);
Rect labeled_box_bounds(const BoxRectData& item);
Rect text_rect_bounds(const TextRectData& item);
Rect picture_rect_bounds(const PictureRectData& item);
Rect plain_button_bounds(const PlainButtonData& item);
Rect checkbox_button_bounds(const CheckboxButtonData& item);
Rect radio_button_bounds(const RadioButtonData& item);
Rect tab_box_bounds(const TabBoxData& item);
Rect tab_box_button_bounds(const TabBoxButtonData& item);

}  // namespace antares

#endif  // ANTARES_DRAWING_INTERFACE_HPP_

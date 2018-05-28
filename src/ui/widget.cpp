// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// Copyright (C) 2008-2018 The Antares Authors
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

#include "ui/widget.hpp"

using std::unique_ptr;
using std::vector;

namespace antares {

void               BoxRect::draw(Point offset, InputMode) const { draw_box_rect(offset, data); }
Rect               BoxRect::bounds() const { return box_rect_bounds(data); }
BoxRectData*       BoxRect::item() { return &data; }
const BoxRectData* BoxRect::item() const { return &data; }

void                TextRect::draw(Point offset, InputMode) const { draw_text_rect(offset, data); }
Rect                TextRect::bounds() const { return text_rect_bounds(data); }
TextRectData*       TextRect::item() { return &data; }
const TextRectData* TextRect::item() const { return &data; }

void PictureRect::draw(Point offset, InputMode) const { draw_picture_rect(offset, data, texture); }
Rect PictureRect::bounds() const { return picture_rect_bounds(data); }
PictureRectData*       PictureRect::item() { return &data; }
const PictureRectData* PictureRect::item() const { return &data; }

void PlainButton::draw(Point offset, InputMode mode) const {
    draw_button(offset, mode, data, state);
}
Rect                   PlainButton::bounds() const { return plain_button_bounds(data); }
PlainButtonData*       PlainButton::item() { return &data; }
const PlainButtonData* PlainButton::item() const { return &data; }

void CheckboxButton::draw(Point offset, InputMode) const {
    draw_checkbox(offset, data, state, on);
}
Rect                      CheckboxButton::bounds() const { return checkbox_button_bounds(data); }
CheckboxButtonData*       CheckboxButton::item() { return &data; }
const CheckboxButtonData* CheckboxButton::item() const { return &data; }

void                   RadioButton::draw(Point offset, InputMode) const {}
Rect                   RadioButton::bounds() const { return radio_button_bounds(data); }
RadioButtonData*       RadioButton::item() { return &data; }
const RadioButtonData* RadioButton::item() const { return &data; }

void TabBoxButton::draw(Point offset, InputMode) const {
    draw_tab_box_button(offset, data, state, on);
}
Rect                    TabBoxButton::bounds() const { return tab_box_button_bounds(data); }
TabBoxButtonData*       TabBoxButton::item() { return &data; }
const TabBoxButtonData* TabBoxButton::item() const { return &data; }

void              TabBox::draw(Point offset, InputMode) const { draw_tab_box(offset, data); }
Rect              TabBox::bounds() const { return tab_box_bounds(data); }
TabBoxData*       TabBox::item() { return &data; }
const TabBoxData* TabBox::item() const { return &data; }

}  // namespace antares

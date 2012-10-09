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

#ifndef ANTARES_DRAWING_INTERFACE_HPP_
#define ANTARES_DRAWING_INTERFACE_HPP_

#include <vector>
#include <sfz/sfz.hpp>

#include "data/interface.hpp"

namespace antares {

const int32_t kInterfaceTextVBuffer = 2;
const int32_t kInterfaceTextHBuffer = 3;

// the inline pictType struct is for keeping track of picts included in my text boxes.
struct inlinePictType {
    Rect bounds;
    short id;
};

void DrawPlayerInterfaceLabeledBox(const interfaceItemType& item, PixMap* pix);

void draw_text_in_rect(
        const Rect& tRect, const sfz::StringSlice& text, interfaceStyleType style,
        unsigned char textcolor, std::vector<inlinePictType>& inlinePict);
void DrawInterfaceTextInRect(
        const Rect& rect, const sfz::StringSlice& text, interfaceStyleType style,
        unsigned char textcolor, PixMap* pix, std::vector<inlinePictType>& inlinePict);

short GetInterfaceTextHeightFromWidth(
        const sfz::StringSlice& text, interfaceStyleType style, short width);
void DrawAnyInterfaceItem(const interfaceItemType& item, PixMap* pix);
void draw_interface_item(const interfaceItemType& item);

void GetAnyInterfaceItemGraphicBounds(const interfaceItemType& item, Rect* rect);
void GetAnyInterfaceItemContentBounds(const interfaceItemType& item, Rect* rect);

short GetInterfaceStringWidth(const sfz::StringSlice& s, interfaceStyleType style);
short GetInterfaceFontHeight(interfaceStyleType style);
short GetInterfaceFontAscent(interfaceStyleType style);
short GetInterfaceFontWidth(interfaceStyleType style);

}  // namespace antares

#endif // ANTARES_DRAWING_INTERFACE_HPP_

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

#ifndef ANTARES_PLAYER_INTERFACE_DRAWING_HPP_
#define ANTARES_PLAYER_INTERFACE_DRAWING_HPP_

#include <sfz/sfz.hpp>
#include "AnyChar.hpp"
#include "PlayerInterfaceItems.hpp"

namespace antares {

#define kInterfaceTextVBuffer       2
#define kInterfaceTextHBuffer       3

#define kMaxInlinePictNum           8   // max # of inline picts it'll keep track of

// the inline pictType struct is for keeping track of picts included in my text boxes.
struct inlinePictType {
    Rect bounds;
    short id;
};

void DrawPlayerInterfacePlainRect(
        const Rect& rect, uint8_t color, interfaceStyleType style, PixMap* pix);
void DrawPlayerInterfaceTabBox(
        const Rect& rect, uint8_t color, interfaceStyleType style, PixMap* pix,
        int top_right_border_size);
void DrawPlayerInterfaceButton(const interfaceItemType& item, PixMap* pix);
void DrawPlayerInterfaceTabBoxButton(const interfaceItemType& item, PixMap* pix);
void DrawPlayerInterfaceRadioButton(const interfaceItemType& item, PixMap* pix);
void DrawPlayerInterfaceCheckBox(const interfaceItemType& item, PixMap* pix);
void DrawPlayerInterfaceLabeledBox(const interfaceItemType& item, PixMap* pix);
void DrawPlayerInterfaceList(const interfaceItemType& item, PixMap* pix);
void DrawPlayerInterfaceListEntry(const interfaceItemType& item, int which_entry, PixMap* pix);

void DrawPlayerListLineUp(const interfaceItemType& item, interfaceItemStatusEnum status, PixMap* pix);
void GetPlayerListLineUpRect(const interfaceItemType& item, Rect* rect);
void DrawPlayerListPageUp(const interfaceItemType& item, interfaceItemStatusEnum status, PixMap* pix);
void GetPlayerListPageUpRect(const interfaceItemType& item, Rect* rect);
void DrawPlayerListLineDown(const interfaceItemType& item, interfaceItemStatusEnum status, PixMap* pix);
void GetPlayerListLineDownRect(const interfaceItemType& item, Rect* rect);
void DrawPlayerListPageDown(const interfaceItemType& item, interfaceItemStatusEnum status, PixMap* pix);
void GetPlayerListPageDownRect(const interfaceItemType& item, Rect* rect);

void DrawInterfaceTextRect(const interfaceItemType& item, PixMap* pix);
void DrawInterfaceTextInRect(
        const Rect& rect, const sfz::StringPiece& text, interfaceStyleType style,
        unsigned char textcolor, PixMap* pix, inlinePictType* inlinePict);

short GetInterfaceTextHeightFromWidth(
        const sfz::StringPiece& text, interfaceStyleType style, short width);
void DrawInterfacePictureRect(const interfaceItemType& item, PixMap* pix);
void DrawAnyInterfaceItem(const interfaceItemType& item, PixMap* pix);

void GetAnyInterfaceItemGraphicBounds(const interfaceItemType& item, Rect* rect);
void GetAnyInterfaceItemContentBounds(const interfaceItemType& item, Rect* rect);

short GetInterfaceStringWidth(const sfz::StringPiece& s, interfaceStyleType style);
short GetInterfaceFontHeight(interfaceStyleType style);
short GetInterfaceFontAscent(interfaceStyleType style);
short GetInterfaceFontWidth(interfaceStyleType style);

void DrawInterfaceString(
        const sfz::StringPiece& s, interfaceStyleType style, PixMap* pix, const RgbColor& color);

void SetInterfaceLargeUpperFont(interfaceStyleType style);
void SetInterfaceLargeLowerFont(interfaceStyleType style);

}  // namespace antares

#endif // ANTARES_PLAYER_INTERFACE_DRAWING_HPP_

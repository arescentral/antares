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

// Player Interface Drawing.h

#include "AnyChar.hpp"
#include "PlayerInterfaceItems.hpp"

#define kInterfaceTextVBuffer       2
#define kInterfaceTextHBuffer       3

#define kMaxInlinePictNum           8   // max # of inline picts it'll keep track of

// the inline pictType struct is for keeping track of picts included in my text boxes.
struct inlinePictType
{
    Rect    bounds;
    short   id;
};

void DrawPlayerInterfacePlainRect( longRect *, unsigned char, interfaceStyleType, PixMap *, long,
                        long);
void DrawPlayerInterfaceTabBox( longRect *, unsigned char, interfaceStyleType, PixMap *, long,
                        long, short);
void DrawPlayerInterfaceButton( interfaceItemType *, PixMap *, long,
                        long);
void DrawPlayerInterfaceTabBoxButton( interfaceItemType *, PixMap *, long,
                        long);
void DrawPlayerInterfaceRadioButton( interfaceItemType *, PixMap *, long,
                        long);
void DrawPlayerInterfaceCheckBox( interfaceItemType *, PixMap *, long,
                        long);
void DrawPlayerInterfaceLabeledBox( interfaceItemType *, PixMap *, long,
                        long);
void DrawPlayerInterfaceList( interfaceItemType *, PixMap *, long,
                        long);
void DrawPlayerInterfaceListEntry( interfaceItemType *, short, PixMap *, long,
                        long);
void DrawPlayerListLineUp( interfaceItemType *);
void GetPlayerListLineUpRect( interfaceItemType *, Rect *);
void DrawPlayerListPageUp( interfaceItemType *);
void GetPlayerListPageUpRect( interfaceItemType *, Rect *);
void DrawPlayerListLineDown( interfaceItemType *);
void GetPlayerListLineDownRect( interfaceItemType *, Rect *);
void DrawPlayerListPageDown( interfaceItemType *);
void GetPlayerListPageDownRect( interfaceItemType *, Rect *);
void DrawInterfaceTextRect( interfaceItemType *, PixMap *, long,
                        long);
void DrawInterfaceTextInRect( Rect *, const unsigned char *, long, interfaceStyleType, unsigned char, PixMap *, long,
                        long, inlinePictType *);
short GetInterfaceTextHeightFromWidth(unsigned char*, long, interfaceStyleType, short);
void DrawInterfacePictureRect( interfaceItemType *, PixMap *, long,
                        long);
void DrawAnyInterfaceItem( interfaceItemType *, PixMap *, long,
                        long);
void GetAnyInterfaceItemGraphicBounds( interfaceItemType *, Rect *);
void GetAnyInterfaceItemContentBounds( interfaceItemType *, Rect *);
short GetInterfaceStringWidth(unsigned char*, interfaceStyleType);
short GetInterfaceFontHeight( interfaceStyleType);
short GetInterfaceFontAscent( interfaceStyleType);
short GetInterfaceFontWidth( interfaceStyleType);
void DrawInterfaceString(unsigned char*, interfaceStyleType, PixMap *, long,
                        long, unsigned char);
void SetInterfaceLargeUpperFont( interfaceStyleType);
void SetInterfaceLargeLowerFont( interfaceStyleType);
void BiggestRect( Rect  *, Rect *);
void LongRectToRect( longRect *, Rect *);
void RectToLongRect( Rect *, longRect *);
void SetLongRect( longRect *, long, long, long, long);

#endif // ANTARES_PLAYER_INTERFACE_DRAWING_HPP_

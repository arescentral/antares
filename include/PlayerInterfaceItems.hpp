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

#ifndef ANTARES_PLAYER_INTERFACE_ITEMS_HPP_
#define ANTARES_PLAYER_INTERFACE_ITEMS_HPP_

// Player Interface Items.h

#include <TextEdit.h>

#include "AnyChar.hpp"
#include "NateDraw.hpp"

#pragma options align=mac68k

enum interfaceKindType {
    kPlainRect = 1,
    kLabeledRect = 2,
    kListRect = 3,
    kTextRect = 4,
    kPlainButton = 5,
    kRadioButton = 6,
    kCheckboxButton = 7,
    kPictureRect = 8,
    kTabBox = 9,
    kTabBoxTop = 10,
    kTabBoxButton = 11
};

enum interfaceItemStatusType {
    kDimmed = 1,
    kActive = 2,
    kIH_Hilite = 3
};

enum interfaceStyleType {
    kLarge = 1,
    kSmall = 2
};

struct interfaceLabelType {
    short               stringID;
    short               stringNumber;
};

struct interfaceLabeledRectType {
    interfaceLabelType  label;
    unsigned char       color;
    TEHandle            teData;
    Boolean             editable;
};

struct interfaceListType {
    interfaceLabelType          label;
    short                       (*getListLength)( void);
    void                        (*getItemString)( short, anyCharType *);
    Boolean                     (*itemHilited)( short, Boolean);
//  void                        (*hiliteItem)( short);
    short                       topItem;
    interfaceItemStatusType     lineUpStatus;
    interfaceItemStatusType     lineDownStatus;
    interfaceItemStatusType     pageUpStatus;
    interfaceItemStatusType     pageDownStatus;
};

struct interfaceTextRectType {
    short               textID;
    Boolean             visibleBounds;
};

struct interfaceTabBoxType {
    short               topRightBorderSize;
};

struct interfacePictureRectType {
    short               pictureID;
    Boolean             visibleBounds;
};

struct interfaceButtonType {
    interfaceLabelType          label;
    short                       key;
    Boolean                     defaultButton;
    interfaceItemStatusType     status;
};

struct interfaceRadioType {
    interfaceLabelType          label;
    short                       key;
    Boolean                     on;
    interfaceItemStatusType     status;
}; // also tab box button type

struct interfaceCheckboxType {
    interfaceLabelType          label;
    short                       key;
    Boolean                     on;
    interfaceItemStatusType     status;
};

struct interfaceItemType {
    longRect            bounds;
    union
    {
        interfaceLabeledRectType    labeledRect;
        interfaceListType           listRect;
        interfaceTextRectType       textRect;
        interfaceButtonType         plainButton;
        interfaceRadioType          radioButton;
        interfaceCheckboxType       checkboxButton;
        interfacePictureRectType    pictureRect;
        interfaceTabBoxType         tabBox;
    } item;

    unsigned char       color;
    interfaceKindType   kind;
    interfaceStyleType  style;
};

#pragma options align=reset

#endif // ANTARES_PLAYER_INTERFACE_ITEMS_HPP_

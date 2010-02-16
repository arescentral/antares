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

#include "AnyChar.hpp"
#include "NateDraw.hpp"

namespace sfz { class BinaryReader; }
namespace sfz { class StringPiece; }

namespace antares {

enum interfaceKindEnum {
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
typedef uint8_t interfaceKindType;

enum interfaceItemStatusEnum {
    kDimmed = 1,
    kActive = 2,
    kIH_Hilite = 3
};
typedef uint8_t interfaceItemStatusType;

enum interfaceStyleEnum {
    kLarge = 1,
    kSmall = 2
};
typedef uint8_t interfaceStyleType;

struct interfaceLabelType {
    short               stringID;
    short               stringNumber;

    void read(sfz::BinaryReader* bin);
};

struct interfaceLabeledRectType {
    interfaceLabelType  label;
    unsigned char       color;
    uint32_t             unused;
    bool                editable;
    // bool             editable;

    void read(sfz::BinaryReader* bin);
};

struct interfaceListType {
    interfaceLabelType          label;
    short                       (*getListLength)( void);
    sfz::StringPiece            (*getItemString)(short);
    bool                     (*itemHilited)( short, bool);
    short                       topItem;

    void read(sfz::BinaryReader* bin);
};

struct interfaceTextRectType {
    short               textID;
    uint8_t             visibleBounds;
    // bool             visibleBounds;

    void read(sfz::BinaryReader* bin);
};

struct interfaceTabBoxType {
    short               topRightBorderSize;

    void read(sfz::BinaryReader* bin);
};

struct interfacePictureRectType {
    short               pictureID;
    uint8_t             visibleBounds;
    // bool             visibleBounds;

    void read(sfz::BinaryReader* bin);
};

struct interfaceButtonType {
    interfaceLabelType          label;
    short                       key;
    uint8_t                     defaultButton;
    // bool                     defaultButton;
    interfaceItemStatusType     status;

    void read(sfz::BinaryReader* bin);
};

struct interfaceRadioType {
    interfaceLabelType          label;
    short                       key;
    uint8_t                     on;
    // bool                     on;
    interfaceItemStatusType     status;

    void read(sfz::BinaryReader* bin);
}; // also tab box button type

struct interfaceCheckboxType {
    interfaceLabelType          label;
    short                       key;
    uint8_t                     on;
    // bool                     on;
    interfaceItemStatusType     status;

    void read(sfz::BinaryReader* bin);
};

struct interfaceItemType {
    Rect            bounds;
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

    uint8_t             color;
    interfaceKindType   kind;
    interfaceStyleType  style;

    interfaceItemStatusType status() const;
    void set_status(interfaceItemStatusType status);
    int key() const;
    void set_key(int key);

    void read(sfz::BinaryReader* bin);
};

}  // namespace antares

#endif // ANTARES_PLAYER_INTERFACE_ITEMS_HPP_

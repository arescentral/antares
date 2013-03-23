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

#ifndef ANTARES_DATA_INTERFACE_HPP_
#define ANTARES_DATA_INTERFACE_HPP_

#include <sfz/sfz.hpp>

#include "math/geometry.hpp"

namespace antares {

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
    uint32_t             unused;
    bool                editable;
    // bool             editable;
};

struct interfaceListType {
    interfaceLabelType          label;
    short                       (*getListLength)( void);
    sfz::StringSlice            (*getItemString)(short);
    bool                     (*itemHilited)( short, bool);
    short                       topItem;
};

struct interfaceTextRectType {
    short               textID;
    uint8_t             visibleBounds;
    // bool             visibleBounds;
};

struct interfaceTabBoxType {
    short               topRightBorderSize;
};

struct interfacePictureRectType {
    short               pictureID;
    uint8_t             visibleBounds;
    // bool             visibleBounds;
};

struct interfaceButtonType {
    interfaceLabelType          label;
    short                       key;
    uint8_t                     defaultButton;
    // bool                     defaultButton;
    interfaceItemStatusType     status;
};

struct interfaceRadioType {
    interfaceLabelType          label;
    short                       key;
    uint8_t                     on;
    // bool                     on;
    interfaceItemStatusType     status;
}; // also tab box button type

struct interfaceCheckboxType {
    interfaceLabelType          label;
    short                       key;
    uint8_t                     on;
    // bool                     on;
    interfaceItemStatusType     status;
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
    std::vector<interfaceItemType> tab_content;

    interfaceItemStatusType status() const;
    void set_status(interfaceItemStatusType status);
    int key() const;
    void set_key(int key);
    void set_label(interfaceLabelType label);
};

std::vector<interfaceItemType> interface_items(const sfz::Json& json);

}  // namespace antares

#endif // ANTARES_DATA_INTERFACE_HPP_

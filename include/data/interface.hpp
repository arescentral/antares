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
    int16_t             stringID;
    int16_t             stringNumber;
};

struct interfaceLabeledRectType {
    interfaceLabelType  label;
};

struct interfaceListType {
    interfaceLabelType          label;
    short                       (*getListLength)( void);
    sfz::StringSlice            (*getItemString)(short);
    bool                     (*itemHilited)( short, bool);
    short                       topItem;
};

struct interfaceTextRectType {
    int16_t             textID;
    bool                visibleBounds;
};

struct interfaceTabBoxType {
    int16_t             topRightBorderSize;
};

struct interfacePictureRectType {
    int16_t             pictureID;
    bool                visibleBounds;
};

struct interfaceButtonType {
    interfaceLabelType          label;
    int16_t                     key;
    interfaceItemStatusType     status;
};

struct interfaceRadioType {
    interfaceLabelType          label;
    int16_t                     key;
    bool                        on;
    interfaceItemStatusType     status;
}; // also tab box button type

struct interfaceCheckboxType {
    interfaceLabelType          label;
    int16_t                     key;
    bool                        on;
    interfaceItemStatusType     status;
};

class InterfaceItem {
  public:
    InterfaceItem(InterfaceItem&&) = default;
    InterfaceItem& operator=(InterfaceItem&&) = default;

    // TODO(sfiera): don't clone.  It's usually frivolous.
    std::unique_ptr<InterfaceItem> clone() const;

    interfaceKindType kind() const { return _kind; }
    const Rect& bounds() const { return _bounds; }
    uint8_t hue() const { return _hue; }
    interfaceStyleType style() const { return _style; }
    interfaceItemStatusType status() const;
    int key() const;
    bool on() const;
    interfaceLabelType label() const;
    int16_t id() const;
    int16_t top_right_border_size() const;
    const std::vector<std::unique_ptr<InterfaceItem>>& tab_content() const { return _tab_content; }

    Rect& bounds() { return _bounds; }
    void set_hue(uint8_t hue);
    void set_status(interfaceItemStatusType status);
    void set_key(int key);
    void set_on(bool on);
    void set_label(interfaceLabelType label);

  private:
    friend std::vector<std::unique_ptr<InterfaceItem>> interface_items(const sfz::Json& json);
    friend InterfaceItem labeled_rect(
            Rect bounds, uint8_t hue, interfaceStyleType style, int16_t str_id, int str_num);

    InterfaceItem();
    InterfaceItem(const InterfaceItem&);

    Rect            _bounds;
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
    } _item;

    uint8_t             _hue;
    interfaceKindType   _kind;
    interfaceStyleType  _style;
    std::vector<std::unique_ptr<InterfaceItem>> _tab_content;
};

std::vector<std::unique_ptr<InterfaceItem>> interface_items(const sfz::Json& json);

InterfaceItem labeled_rect(
        Rect bounds, uint8_t hue, interfaceStyleType style, int16_t str_id, int str_num);

}  // namespace antares

#endif // ANTARES_DATA_INTERFACE_HPP_

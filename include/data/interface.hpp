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

#ifndef ANTARES_DATA_INTERFACE_HPP_
#define ANTARES_DATA_INTERFACE_HPP_

#include <sfz/sfz.hpp>

#include "data/picture.hpp"
#include "math/geometry.hpp"
#include "video/driver.hpp"

namespace antares {

enum interfaceItemStatusType { kDimmed = 1, kActive = 2, kIH_Hilite = 3 };

enum interfaceStyleType { kLarge = 1, kSmall = 2 };

struct interfaceLabelType {
    int16_t stringID;
    int16_t stringNumber;
};

class InterfaceItem {
  public:
    class Visitor;

    InterfaceItem(InterfaceItem&&) = default;
    InterfaceItem& operator=(InterfaceItem&&) = default;
    virtual ~InterfaceItem() {}

    const int    id;
    const Rect&  bounds() const { return _bounds; }
    Rect&        bounds() { return _bounds; }
    virtual void accept(const Visitor& visitor) const = 0;

  protected:
    InterfaceItem(int id, Rect bounds);

  private:
    friend std::vector<std::unique_ptr<InterfaceItem>> interface_items(
            int id0, const sfz::Json& json);

    Rect _bounds;
};

std::vector<std::unique_ptr<InterfaceItem>> interface_items(int id0, const sfz::Json& json);

struct PlainRect : public InterfaceItem {
    PlainRect(int id, Rect bounds, uint8_t hue, interfaceStyleType style);
    virtual void accept(const Visitor& visitor) const;

    uint8_t            hue;
    interfaceStyleType style;
};

struct LabeledItem : public InterfaceItem {
    LabeledItem(int id, Rect bounds, interfaceLabelType label);

    sfz::String label;
};

struct LabeledRect : public LabeledItem {
    LabeledRect(
            int id, Rect bounds, interfaceLabelType label, uint8_t hue, interfaceStyleType style);
    virtual void accept(const Visitor& visitor) const;

    uint8_t            hue;
    interfaceStyleType style;
};

struct TextRect : public InterfaceItem {
    TextRect(int id, Rect bounds, uint8_t hue, interfaceStyleType style);
    TextRect(int id, Rect bounds, sfz::StringSlice name, uint8_t hue, interfaceStyleType style);
    virtual void accept(const Visitor& visitor) const;

    sfz::String        text;
    uint8_t            hue;
    interfaceStyleType style;
};

struct PictureRect : public InterfaceItem {
    PictureRect(int id, Rect bounds, sfz::StringSlice name);
    virtual void accept(const Visitor& visitor) const;

    Picture            picture;
    Texture            texture;
    bool               visible_bounds;
    uint8_t            hue;
    interfaceStyleType style;
};

struct Button : public LabeledItem {
    Button(int id, Rect bounds, int16_t key, int16_t gamepad, interfaceLabelType label,
           uint8_t hue, interfaceStyleType style);

    int16_t                 key;
    int16_t                 gamepad;
    uint8_t                 hue;
    interfaceStyleType      style;
    interfaceItemStatusType status;
};

struct PlainButton : public Button {
    PlainButton(
            int id, Rect bounds, int16_t key, int16_t gamepad, interfaceLabelType label,
            uint8_t hue, interfaceStyleType style);
    virtual void accept(const Visitor& visitor) const;
};

struct CheckboxButton : public Button {
    CheckboxButton(
            int id, Rect bounds, int16_t key, int16_t gamepad, interfaceLabelType label,
            uint8_t hue, interfaceStyleType style);
    virtual void accept(const Visitor& visitor) const;

    bool on;
};

struct RadioButton : public Button {
    RadioButton(
            int id, Rect bounds, int16_t key, int16_t gamepad, interfaceLabelType label,
            uint8_t hue, interfaceStyleType style);
    virtual void accept(const Visitor& visitor) const;

    bool on;
};

struct TabBoxButton : public Button {
    TabBoxButton(
            int id, Rect bounds, int16_t key, int16_t gamepad, interfaceLabelType label,
            uint8_t hue, interfaceStyleType style, const sfz::Json& tab_content);
    virtual void accept(const Visitor& visitor) const;

    bool      on;
    sfz::Json tab_content;
};

struct TabBox : public InterfaceItem {
    TabBox(int id, Rect bounds, uint8_t hue, interfaceStyleType style,
           int16_t top_right_border_size);
    virtual void accept(const Visitor& visitor) const;

    uint8_t            hue;
    interfaceStyleType style;
    int16_t            top_right_border_size;
};

class InterfaceItem::Visitor {
  public:
    ~Visitor();
    virtual void visit_plain_rect(const PlainRect&) const           = 0;
    virtual void visit_labeled_rect(const LabeledRect&) const       = 0;
    virtual void visit_text_rect(const TextRect&) const             = 0;
    virtual void visit_picture_rect(const PictureRect&) const       = 0;
    virtual void visit_plain_button(const PlainButton&) const       = 0;
    virtual void visit_radio_button(const RadioButton&) const       = 0;
    virtual void visit_checkbox_button(const CheckboxButton&) const = 0;
    virtual void visit_tab_box(const TabBox&) const                 = 0;
    virtual void visit_tab_box_button(const TabBoxButton&) const    = 0;
};

}  // namespace antares

#endif  // ANTARES_DATA_INTERFACE_HPP_

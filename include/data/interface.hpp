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

#include <pn/array>
#include <pn/string>
#include <sfz/sfz.hpp>

#include "data/enums.hpp"
#include "math/geometry.hpp"
#include "video/driver.hpp"

namespace antares {

class path_value;
struct InterfaceItem;

enum interfaceItemStatusType { kDimmed = 1, kActive = 2, kIH_Hilite = 3 };

enum class InterfaceStyle { LARGE, SMALL };

struct interfaceLabelType {
    int64_t stringID;
    int64_t stringNumber;
};

struct Interface {
    std::vector<std::unique_ptr<InterfaceItem>> items;
};

struct InterfaceItem {
  public:
    class Visitor;

    InterfaceItem()                = default;
    InterfaceItem(InterfaceItem&&) = default;
    InterfaceItem& operator=(InterfaceItem&&) = default;
    virtual ~InterfaceItem() {}

    virtual std::unique_ptr<InterfaceItem> copy() const                         = 0;
    virtual void                           accept(const Visitor& visitor) const = 0;

    int64_t id = -1;
    Rect    bounds;
};

Interface interface(path_value x);

struct LabeledItem : public InterfaceItem {
    pn::string label;
};

struct BoxRect : public InterfaceItem {
    virtual std::unique_ptr<InterfaceItem> copy() const;
    virtual void                           accept(const Visitor& visitor) const;

    sfz::optional<pn::string> label;
    Hue                       hue   = Hue::GRAY;
    InterfaceStyle            style = InterfaceStyle::LARGE;
};

struct TextRect : public InterfaceItem {
    virtual std::unique_ptr<InterfaceItem> copy() const;
    virtual void                           accept(const Visitor& visitor) const;

    pn::string     text;
    Hue            hue   = Hue::GRAY;
    InterfaceStyle style = InterfaceStyle::LARGE;
};

struct PictureRect : public InterfaceItem {
    virtual std::unique_ptr<InterfaceItem> copy() const;
    virtual void                           accept(const Visitor& visitor) const;

    Texture texture;
};

struct Button : public LabeledItem {
    int16_t                 key     = 0;
    int16_t                 gamepad = 0;
    Hue                     hue     = Hue::GRAY;
    InterfaceStyle          style   = InterfaceStyle::LARGE;
    interfaceItemStatusType status  = kActive;
};

struct PlainButton : public Button {
    virtual std::unique_ptr<InterfaceItem> copy() const;
    virtual void                           accept(const Visitor& visitor) const;
};

struct CheckboxButton : public Button {
    virtual std::unique_ptr<InterfaceItem> copy() const;
    virtual void                           accept(const Visitor& visitor) const;

    bool on = false;
};

struct RadioButton : public Button {
    virtual std::unique_ptr<InterfaceItem> copy() const;
    virtual void                           accept(const Visitor& visitor) const;

    bool on = false;
};

struct TabBoxButton : public Button {
    virtual std::unique_ptr<InterfaceItem> copy() const;
    virtual void                           accept(const Visitor& visitor) const;

    bool      on = false;
    Interface tab_content;
};

struct TabBox : public InterfaceItem {
    virtual std::unique_ptr<InterfaceItem> copy() const;
    virtual void                           accept(const Visitor& visitor) const;

    Hue            hue                   = Hue::GRAY;
    InterfaceStyle style                 = InterfaceStyle::LARGE;
    int16_t        top_right_border_size = 0;
};

class InterfaceItem::Visitor {
  public:
    ~Visitor();
    virtual void visit_box_rect(const BoxRect&) const               = 0;
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

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
struct InterfaceItemData;

enum interfaceItemStatusType { kDimmed = 1, kActive = 2, kIH_Hilite = 3 };

enum class InterfaceStyle { LARGE, SMALL };

struct interfaceLabelType {
    int64_t stringID;
    int64_t stringNumber;
};

struct InterfaceData {
    std::vector<std::unique_ptr<InterfaceItemData>> items;
};

struct InterfaceItemData {
  public:
    class Visitor;

    InterfaceItemData()                    = default;
    InterfaceItemData(InterfaceItemData&&) = default;
    InterfaceItemData& operator=(InterfaceItemData&&) = default;
    virtual ~InterfaceItemData() {}

    virtual std::unique_ptr<InterfaceItemData> copy() const                         = 0;
    virtual void                               accept(const Visitor& visitor) const = 0;

    Rect bounds;
};

InterfaceData interface(path_value x);

struct BoxRectData : public InterfaceItemData {
    virtual std::unique_ptr<InterfaceItemData> copy() const;
    virtual void                               accept(const Visitor& visitor) const;

    sfz::optional<pn::string> label;
    Hue                       hue   = Hue::GRAY;
    InterfaceStyle            style = InterfaceStyle::LARGE;
};

struct TextRectData : public InterfaceItemData {
    virtual std::unique_ptr<InterfaceItemData> copy() const;
    virtual void                               accept(const Visitor& visitor) const;

    pn::string     text;
    Hue            hue   = Hue::GRAY;
    InterfaceStyle style = InterfaceStyle::LARGE;
};

struct PictureRectData : public InterfaceItemData {
    virtual std::unique_ptr<InterfaceItemData> copy() const;
    virtual void                               accept(const Visitor& visitor) const;

    Texture texture;
};

struct ButtonData : public InterfaceItemData {
    int64_t                 id = -1;
    pn::string              label;
    int16_t                 key     = 0;
    int16_t                 gamepad = 0;
    Hue                     hue     = Hue::GRAY;
    InterfaceStyle          style   = InterfaceStyle::LARGE;
    interfaceItemStatusType status  = kActive;
};

struct PlainButtonData : public ButtonData {
    virtual std::unique_ptr<InterfaceItemData> copy() const;
    virtual void                               accept(const Visitor& visitor) const;
};

struct CheckboxButtonData : public ButtonData {
    virtual std::unique_ptr<InterfaceItemData> copy() const;
    virtual void                               accept(const Visitor& visitor) const;

    bool on = false;
};

struct RadioButtonData : public ButtonData {
    virtual std::unique_ptr<InterfaceItemData> copy() const;
    virtual void                               accept(const Visitor& visitor) const;

    bool on = false;
};

struct TabBoxButtonData : public ButtonData {
    virtual std::unique_ptr<InterfaceItemData> copy() const;
    virtual void                               accept(const Visitor& visitor) const;

    bool          on = false;
    InterfaceData tab_content;
};

struct TabBoxData : public InterfaceItemData {
    virtual std::unique_ptr<InterfaceItemData> copy() const;
    virtual void                               accept(const Visitor& visitor) const;

    Hue            hue                   = Hue::GRAY;
    InterfaceStyle style                 = InterfaceStyle::LARGE;
    int16_t        top_right_border_size = 0;
};

class InterfaceItemData::Visitor {
  public:
    ~Visitor();
    virtual void visit_box_rect(const BoxRectData&) const               = 0;
    virtual void visit_text_rect(const TextRectData&) const             = 0;
    virtual void visit_picture_rect(const PictureRectData&) const       = 0;
    virtual void visit_plain_button(const PlainButtonData&) const       = 0;
    virtual void visit_radio_button(const RadioButtonData&) const       = 0;
    virtual void visit_checkbox_button(const CheckboxButtonData&) const = 0;
    virtual void visit_tab_box(const TabBoxData&) const                 = 0;
    virtual void visit_tab_box_button(const TabBoxButtonData&) const    = 0;
};

}  // namespace antares

#endif  // ANTARES_DATA_INTERFACE_HPP_

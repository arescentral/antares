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

#include "data/interface.hpp"

#include <pn/file>
#include <pn/map>

#include <sfz/sfz.hpp>
#include "config/gamepad.hpp"
#include "config/keys.hpp"
#include "data/field.hpp"
#include "data/resource.hpp"
#include "drawing/color.hpp"
#include "lang/casts.hpp"
#include "video/driver.hpp"

using sfz::range;

namespace antares {

DECLARE_FIELD_READER(InterfaceItemData);

InterfaceItemData::Type InterfaceItemData::type() const { return base.type; }

InterfaceItemData::InterfaceItemData() : base{} {}
InterfaceItemData::InterfaceItemData(BoxRectData d) : rect(std::move(d)) {}
InterfaceItemData::InterfaceItemData(TextRectData d) : text(std::move(d)) {}
InterfaceItemData::InterfaceItemData(PictureRectData d) : picture(std::move(d)) {}
InterfaceItemData::InterfaceItemData(PlainButtonData d) : button(std::move(d)) {}
InterfaceItemData::InterfaceItemData(CheckboxButtonData d) : checkbox(std::move(d)) {}
InterfaceItemData::InterfaceItemData(RadioButtonData d) : radio(std::move(d)) {}
InterfaceItemData::InterfaceItemData(TabBoxData d) : tab_box(std::move(d)) {}

InterfaceItemData::InterfaceItemData(InterfaceItemData&& a) {
    switch (a.type()) {
        case InterfaceItemDataBase::Type::NONE: new (this) InterfaceItemData(); break;
        case InterfaceItemDataBase::Type::RECT:
            new (this) InterfaceItemData(std::move(a.rect));
            break;
        case InterfaceItemDataBase::Type::TEXT:
            new (this) InterfaceItemData(std::move(a.text));
            break;
        case InterfaceItemDataBase::Type::PICTURE:
            new (this) InterfaceItemData(std::move(a.picture));
            break;
        case InterfaceItemDataBase::Type::BUTTON:
            new (this) InterfaceItemData(std::move(a.button));
            break;
        case InterfaceItemDataBase::Type::CHECKBOX:
            new (this) InterfaceItemData(std::move(a.checkbox));
            break;
        case InterfaceItemDataBase::Type::RADIO:
            new (this) InterfaceItemData(std::move(a.radio));
            break;
        case InterfaceItemDataBase::Type::TAB_BOX:
            new (this) InterfaceItemData(std::move(a.tab_box));
            break;
    }
}

InterfaceItemData& InterfaceItemData::operator=(InterfaceItemData&& a) {
    this->~InterfaceItemData();
    new (this) InterfaceItemData(std::move(a));
    return *this;
}

InterfaceItemData::~InterfaceItemData() {
    switch (type()) {
        case InterfaceItemDataBase::Type::NONE: base.~InterfaceItemDataBase(); break;
        case InterfaceItemDataBase::Type::RECT: rect.~BoxRectData(); break;
        case InterfaceItemDataBase::Type::TEXT: text.~TextRectData(); break;
        case InterfaceItemDataBase::Type::PICTURE: picture.~PictureRectData(); break;
        case InterfaceItemDataBase::Type::BUTTON: button.~PlainButtonData(); break;
        case InterfaceItemDataBase::Type::CHECKBOX: checkbox.~CheckboxButtonData(); break;
        case InterfaceItemDataBase::Type::RADIO: radio.~RadioButtonData(); break;
        case InterfaceItemDataBase::Type::TAB_BOX: tab_box.~TabBoxData(); break;
    }
}

FIELD_READER(InterfaceStyle) {
    return required_enum<InterfaceStyle>(
            x, {{"small", InterfaceStyle::SMALL}, {"large", InterfaceStyle::LARGE}});
}

FIELD_READER(Key) {
    auto k = read_field<sfz::optional<pn::string>>(x);
    if (!k.has_value()) {
        return Key::NONE;
    }
    Key i;
    if (!GetKeyNameNum(*k, i)) {
        throw std::runtime_error(pn::format("{0}must be a key", x.prefix()).c_str());
    }
    return i;
}

FIELD_READER(Gamepad::Button) {
    auto k = read_field<sfz::optional<pn::string>>(x);
    if (!k.has_value()) {
        return Gamepad::Button::NONE;
    }
    Gamepad::Button i = Gamepad::num(*k);
    if (i == Gamepad::Button::NONE) {
        throw std::runtime_error(pn::format("{0}must be a gamepad button", x.prefix()).c_str());
    }
    return i;
}

FIELD_READER(InterfaceItemData::Type) {
    return required_enum<InterfaceItemData::Type>(
            x, {{"rect", InterfaceItemData::Type::RECT},
                {"button", InterfaceItemData::Type::BUTTON},
                {"checkbox", InterfaceItemData::Type::CHECKBOX},
                {"radio", InterfaceItemData::Type::RADIO},
                {"picture", InterfaceItemData::Type::PICTURE},
                {"text", InterfaceItemData::Type::TEXT},
                {"tab-box", InterfaceItemData::Type::TAB_BOX}});
}

FIELD_READER(TabBoxData::Tab) {
    return required_struct<TabBoxData::Tab>(
            x, {{"id", &TabBoxData::Tab::id},
                {"width", &TabBoxData::Tab::width},
                {"label", &TabBoxData::Tab::label},
                {"content", &TabBoxData::Tab::content}});
}

static InterfaceItemData rect_interface_item(path_value x) {
    return BoxRectData(required_struct<BoxRectData>(
            x, {{"type", &InterfaceItemDataBase::type},
                {"bounds", &InterfaceItemDataBase::bounds},
                {"id", &InterfaceItemDataBase::id},
                {"label", &BoxRectData::label},
                {"hue", &BoxRectData::hue},
                {"style", &BoxRectData::style}}));
}

static InterfaceItemData button_interface_item(path_value x) {
    return required_struct<PlainButtonData>(
            x, {{"type", &InterfaceItemDataBase::type},
                {"bounds", &InterfaceItemDataBase::bounds},
                {"id", &InterfaceItemDataBase::id},
                {"label", &PlainButtonData::label},
                {"key", &PlainButtonData::key},
                {"gamepad", &PlainButtonData::gamepad},
                {"hue", &PlainButtonData::hue},
                {"style", &PlainButtonData::style}});
}

static InterfaceItemData checkbox_interface_item(path_value x) {
    return required_struct<CheckboxButtonData>(
            x, {{"type", &InterfaceItemDataBase::type},
                {"bounds", &InterfaceItemDataBase::bounds},
                {"id", &InterfaceItemDataBase::id},
                {"label", &CheckboxButtonData::label},
                {"key", &CheckboxButtonData::key},
                {"gamepad", &CheckboxButtonData::gamepad},
                {"hue", &CheckboxButtonData::hue},
                {"style", &CheckboxButtonData::style}});
}

static InterfaceItemData radio_interface_item(path_value x) {
    return required_struct<RadioButtonData>(
            x, {{"type", &InterfaceItemDataBase::type},
                {"bounds", &InterfaceItemDataBase::bounds},
                {"id", &InterfaceItemDataBase::id},
                {"label", &RadioButtonData::label},
                {"key", &RadioButtonData::key},
                {"gamepad", &RadioButtonData::gamepad},
                {"hue", &RadioButtonData::hue},
                {"style", &RadioButtonData::style}});
}

static InterfaceItemData picture_interface_item(path_value x) {
    return required_struct<PictureRectData>(
            x, {{"type", &InterfaceItemDataBase::type},
                {"bounds", &InterfaceItemDataBase::bounds},
                {"id", &InterfaceItemDataBase::id},
                {"picture", &PictureRectData::picture}});
}

static InterfaceItemData text_interface_item(path_value x) {
    return required_struct<TextRectData>(
            x, {{"type", &InterfaceItemDataBase::type},
                {"bounds", &InterfaceItemDataBase::bounds},
                {"id", &InterfaceItemDataBase::id},
                {"text", &TextRectData::text},
                {"hue", &TextRectData::hue},
                {"style", &TextRectData::style}});
}

static InterfaceItemData tab_box_interface_item(path_value x) {
    return required_struct<TabBoxData>(
            x, {{"type", &InterfaceItemDataBase::type},
                {"bounds", &InterfaceItemDataBase::bounds},
                {"id", &InterfaceItemDataBase::id},
                {"hue", &TabBoxData::hue},
                {"style", &TabBoxData::style},
                {"tabs", &TabBoxData::tabs}});
}

DEFINE_FIELD_READER(InterfaceItemData) {
    switch (required_object_type(x, read_field<InterfaceItemData::Type>)) {
        case InterfaceItemData::Type::NONE: throw std::runtime_error("interface item type none?");
        case InterfaceItemData::Type::RECT: return rect_interface_item(x);
        case InterfaceItemData::Type::BUTTON: return button_interface_item(x);
        case InterfaceItemData::Type::CHECKBOX: return checkbox_interface_item(x);
        case InterfaceItemData::Type::RADIO: return radio_interface_item(x);
        case InterfaceItemData::Type::PICTURE: return picture_interface_item(x);
        case InterfaceItemData::Type::TEXT: return text_interface_item(x);
        case InterfaceItemData::Type::TAB_BOX: return tab_box_interface_item(x);
    }
}

InterfaceData interface(path_value x) {
    return required_struct<InterfaceData>(
            x, {{"fullscreen", &InterfaceData::fullscreen}, {"items", &InterfaceData::items}});
}

}  // namespace antares

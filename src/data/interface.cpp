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

static std::unique_ptr<InterfaceItemData> interface_item(path_value x);

template <typename T>
struct default_reader;
template <>
struct default_reader<std::unique_ptr<InterfaceItemData>> {
    static std::unique_ptr<InterfaceItemData> read(path_value x) { return interface_item(x); }
};

static InterfaceStyle required_interface_style(path_value x) {
    return required_enum<InterfaceStyle>(
            x, {{"small", InterfaceStyle::SMALL}, {"large", InterfaceStyle::LARGE}});
}
DEFAULT_READER(InterfaceStyle, required_interface_style);

static Key optional_key_num(path_value x) {
    auto k = optional_string(x);
    if (!k.has_value()) {
        return Key::NONE;
    }
    Key i;
    if (!GetKeyNameNum(*k, i)) {
        throw std::runtime_error(pn::format("{0}: must be a key", x.path()).c_str());
    }
    return i;
}

static int16_t optional_gamepad_button(path_value x) {
    auto k = optional_string(x);
    if (!k.has_value()) {
        return 0;
    }
    int i;
    if (!(i = Gamepad::num(*k))) {
        throw std::runtime_error(pn::format("{0}: must be a gamepad button", x.path()).c_str());
    }
    return i;
}

static InterfaceItemData::Type required_interface_item_type(path_value x) {
    return required_enum<InterfaceItemData::Type>(
            x, {{"rect", InterfaceItemData::Type::RECT},
                {"button", InterfaceItemData::Type::BUTTON},
                {"checkbox", InterfaceItemData::Type::CHECKBOX},
                {"radio", InterfaceItemData::Type::RADIO},
                {"picture", InterfaceItemData::Type::PICTURE},
                {"text", InterfaceItemData::Type::TEXT},
                {"tab-box", InterfaceItemData::Type::TAB_BOX}});
}
DEFAULT_READER(InterfaceItemData::Type, required_interface_item_type);

static TabBoxData::Tab required_tab(path_value x) {
    return required_struct<TabBoxData::Tab>(
            x, {{"id", &TabBoxData::Tab::id},
                {"width", &TabBoxData::Tab::width},
                {"label", &TabBoxData::Tab::label},
                {"content", &TabBoxData::Tab::content}});
}
DEFAULT_READER(TabBoxData::Tab, required_tab);

static std::unique_ptr<InterfaceItemData> rect_interface_item(path_value x) {
    return std::unique_ptr<InterfaceItemData>(new BoxRectData(required_struct<BoxRectData>(
            x, {{"type", &InterfaceItemData::type},
                {"bounds", &InterfaceItemData::bounds},
                {"id", &InterfaceItemData::id},
                {"label", &BoxRectData::label},
                {"hue", &BoxRectData::hue},
                {"style", &BoxRectData::style}})));
}

static std::unique_ptr<InterfaceItemData> button_interface_item(path_value x) {
    return std::unique_ptr<InterfaceItemData>(new PlainButtonData(required_struct<PlainButtonData>(
            x, {{"type", &InterfaceItemData::type},
                {"bounds", &InterfaceItemData::bounds},
                {"id", &InterfaceItemData::id},
                {"label", &PlainButtonData::label},
                {"key", {&PlainButtonData::key, optional_key_num}},
                {"gamepad", {&PlainButtonData::gamepad, optional_gamepad_button}},
                {"hue", &PlainButtonData::hue},
                {"style", &PlainButtonData::style}})));
}

static std::unique_ptr<InterfaceItemData> checkbox_interface_item(path_value x) {
    return std::unique_ptr<InterfaceItemData>(
            new CheckboxButtonData(required_struct<CheckboxButtonData>(
                    x, {{"type", &InterfaceItemData::type},
                        {"bounds", &InterfaceItemData::bounds},
                        {"id", &InterfaceItemData::id},
                        {"label", &CheckboxButtonData::label},
                        {"key", {&CheckboxButtonData::key, optional_key_num}},
                        {"gamepad", {&CheckboxButtonData::gamepad, optional_gamepad_button}},
                        {"hue", &CheckboxButtonData::hue},
                        {"style", &CheckboxButtonData::style}})));
}

static std::unique_ptr<InterfaceItemData> radio_interface_item(path_value x) {
    return std::unique_ptr<InterfaceItemData>(new RadioButtonData(required_struct<RadioButtonData>(
            x, {{"type", &InterfaceItemData::type},
                {"bounds", &InterfaceItemData::bounds},
                {"id", &InterfaceItemData::id},
                {"label", &RadioButtonData::label},
                {"key", {&RadioButtonData::key, optional_key_num}},
                {"gamepad", {&RadioButtonData::gamepad, optional_gamepad_button}},
                {"hue", &RadioButtonData::hue},
                {"style", &RadioButtonData::style}})));
}

static std::unique_ptr<InterfaceItemData> picture_interface_item(path_value x) {
    return std::unique_ptr<InterfaceItemData>(new PictureRectData(required_struct<PictureRectData>(
            x, {{"type", &InterfaceItemData::type},
                {"bounds", &InterfaceItemData::bounds},
                {"id", &InterfaceItemData::id},
                {"picture", &PictureRectData::picture}})));
}

static std::unique_ptr<InterfaceItemData> text_interface_item(path_value x) {
    return std::unique_ptr<InterfaceItemData>(new TextRectData(required_struct<TextRectData>(
            x, {{"type", &InterfaceItemData::type},
                {"bounds", &InterfaceItemData::bounds},
                {"id", &InterfaceItemData::id},
                {"text", &TextRectData::text},
                {"hue", &TextRectData::hue},
                {"style", &TextRectData::style}})));
}

static std::unique_ptr<InterfaceItemData> tab_box_interface_item(path_value x) {
    return std::unique_ptr<InterfaceItemData>(new TabBoxData(required_struct<TabBoxData>(
            x, {{"type", &InterfaceItemData::type},
                {"bounds", &InterfaceItemData::bounds},
                {"id", &InterfaceItemData::id},
                {"hue", &TabBoxData::hue},
                {"style", &TabBoxData::style},
                {"tabs", &TabBoxData::tabs}})));
}

static std::unique_ptr<InterfaceItemData> interface_item(path_value x) {
    switch (required_object_type(x, required_interface_item_type)) {
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

void BoxRectData::accept(const Visitor& visitor) const { visitor.visit_box_rect(*this); }
void TextRectData::accept(const Visitor& visitor) const { visitor.visit_text_rect(*this); }
void PictureRectData::accept(const Visitor& visitor) const { visitor.visit_picture_rect(*this); }
void PlainButtonData::accept(const Visitor& visitor) const { visitor.visit_plain_button(*this); }
void CheckboxButtonData::accept(const Visitor& visitor) const {
    visitor.visit_checkbox_button(*this);
}
void RadioButtonData::accept(const Visitor& visitor) const { visitor.visit_radio_button(*this); }
void TabBoxData::accept(const Visitor& visitor) const { visitor.visit_tab_box(*this); }

InterfaceItemData::Visitor::~Visitor() {}

}  // namespace antares

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

DECLARE_FIELD_READER(WidgetData);

WidgetData::Type WidgetData::type() const { return base.type; }

WidgetData::WidgetData() : base{} {}
WidgetData::WidgetData(BoxRectData d) : rect(std::move(d)) {}
WidgetData::WidgetData(TextRectData d) : text(std::move(d)) {}
WidgetData::WidgetData(PictureRectData d) : picture(std::move(d)) {}
WidgetData::WidgetData(PlainButtonData d) : button(std::move(d)) {}
WidgetData::WidgetData(CheckboxButtonData d) : checkbox(std::move(d)) {}
WidgetData::WidgetData(RadioButtonData d) : radio(std::move(d)) {}
WidgetData::WidgetData(TabBoxData d) : tab_box(std::move(d)) {}

WidgetData::WidgetData(WidgetData&& a) {
    switch (a.type()) {
        case WidgetDataBase::Type::NONE: new (this) WidgetData(); break;
        case WidgetDataBase::Type::RECT: new (this) WidgetData(std::move(a.rect)); break;
        case WidgetDataBase::Type::TEXT: new (this) WidgetData(std::move(a.text)); break;
        case WidgetDataBase::Type::PICTURE: new (this) WidgetData(std::move(a.picture)); break;
        case WidgetDataBase::Type::BUTTON: new (this) WidgetData(std::move(a.button)); break;
        case WidgetDataBase::Type::CHECKBOX: new (this) WidgetData(std::move(a.checkbox)); break;
        case WidgetDataBase::Type::RADIO: new (this) WidgetData(std::move(a.radio)); break;
        case WidgetDataBase::Type::TAB_BOX: new (this) WidgetData(std::move(a.tab_box)); break;
    }
}

WidgetData& WidgetData::operator=(WidgetData&& a) {
    this->~WidgetData();
    new (this) WidgetData(std::move(a));
    return *this;
}

WidgetData::~WidgetData() {
    switch (type()) {
        case WidgetDataBase::Type::NONE: base.~WidgetDataBase(); break;
        case WidgetDataBase::Type::RECT: rect.~BoxRectData(); break;
        case WidgetDataBase::Type::TEXT: text.~TextRectData(); break;
        case WidgetDataBase::Type::PICTURE: picture.~PictureRectData(); break;
        case WidgetDataBase::Type::BUTTON: button.~PlainButtonData(); break;
        case WidgetDataBase::Type::CHECKBOX: checkbox.~CheckboxButtonData(); break;
        case WidgetDataBase::Type::RADIO: radio.~RadioButtonData(); break;
        case WidgetDataBase::Type::TAB_BOX: tab_box.~TabBoxData(); break;
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

FIELD_READER(WidgetData::Type) {
    return required_enum<WidgetData::Type>(
            x, {{"rect", WidgetData::Type::RECT},
                {"button", WidgetData::Type::BUTTON},
                {"checkbox", WidgetData::Type::CHECKBOX},
                {"radio", WidgetData::Type::RADIO},
                {"picture", WidgetData::Type::PICTURE},
                {"text", WidgetData::Type::TEXT},
                {"tab-box", WidgetData::Type::TAB_BOX}});
}

FIELD_READER(TabBoxData::Tab) {
    return required_struct<TabBoxData::Tab>(
            x, {{"id", &TabBoxData::Tab::id},
                {"width", &TabBoxData::Tab::width},
                {"label", &TabBoxData::Tab::label},
                {"content", &TabBoxData::Tab::content}});
}

static WidgetData rect_interface_item(path_value x) {
    return BoxRectData(required_struct<BoxRectData>(
            x, {{"type", &WidgetDataBase::type},
                {"bounds", &WidgetDataBase::bounds},
                {"id", &WidgetDataBase::id},
                {"label", &BoxRectData::label},
                {"hue", &BoxRectData::hue},
                {"style", &BoxRectData::style}}));
}

static WidgetData button_interface_item(path_value x) {
    return required_struct<PlainButtonData>(
            x, {{"type", &WidgetDataBase::type},
                {"bounds", &WidgetDataBase::bounds},
                {"id", &WidgetDataBase::id},
                {"label", &PlainButtonData::label},
                {"key", &PlainButtonData::key},
                {"gamepad", &PlainButtonData::gamepad},
                {"hue", &PlainButtonData::hue},
                {"style", &PlainButtonData::style}});
}

static WidgetData checkbox_interface_item(path_value x) {
    return required_struct<CheckboxButtonData>(
            x, {{"type", &WidgetDataBase::type},
                {"bounds", &WidgetDataBase::bounds},
                {"id", &WidgetDataBase::id},
                {"label", &CheckboxButtonData::label},
                {"key", &CheckboxButtonData::key},
                {"gamepad", &CheckboxButtonData::gamepad},
                {"hue", &CheckboxButtonData::hue},
                {"style", &CheckboxButtonData::style}});
}

static WidgetData radio_interface_item(path_value x) {
    return required_struct<RadioButtonData>(
            x, {{"type", &WidgetDataBase::type},
                {"bounds", &WidgetDataBase::bounds},
                {"id", &WidgetDataBase::id},
                {"label", &RadioButtonData::label},
                {"key", &RadioButtonData::key},
                {"gamepad", &RadioButtonData::gamepad},
                {"hue", &RadioButtonData::hue},
                {"style", &RadioButtonData::style}});
}

static WidgetData picture_interface_item(path_value x) {
    return required_struct<PictureRectData>(
            x, {{"type", &WidgetDataBase::type},
                {"bounds", &WidgetDataBase::bounds},
                {"id", &WidgetDataBase::id},
                {"picture", &PictureRectData::picture}});
}

static WidgetData text_interface_item(path_value x) {
    return required_struct<TextRectData>(
            x, {{"type", &WidgetDataBase::type},
                {"bounds", &WidgetDataBase::bounds},
                {"id", &WidgetDataBase::id},
                {"text", &TextRectData::text},
                {"hue", &TextRectData::hue},
                {"style", &TextRectData::style}});
}

static WidgetData tab_box_interface_item(path_value x) {
    return required_struct<TabBoxData>(
            x, {{"type", &WidgetDataBase::type},
                {"bounds", &WidgetDataBase::bounds},
                {"id", &WidgetDataBase::id},
                {"hue", &TabBoxData::hue},
                {"style", &TabBoxData::style},
                {"tabs", &TabBoxData::tabs}});
}

DEFINE_FIELD_READER(WidgetData) {
    switch (required_object_type(x, read_field<WidgetData::Type>)) {
        case WidgetData::Type::NONE: throw std::runtime_error("interface item type none?");
        case WidgetData::Type::RECT: return rect_interface_item(x);
        case WidgetData::Type::BUTTON: return button_interface_item(x);
        case WidgetData::Type::CHECKBOX: return checkbox_interface_item(x);
        case WidgetData::Type::RADIO: return radio_interface_item(x);
        case WidgetData::Type::PICTURE: return picture_interface_item(x);
        case WidgetData::Type::TEXT: return text_interface_item(x);
        case WidgetData::Type::TAB_BOX: return tab_box_interface_item(x);
    }
}

InterfaceData interface(path_value x) {
    return required_struct<InterfaceData>(
            x, {{"fullscreen", &InterfaceData::fullscreen}, {"items", &InterfaceData::items}});
}

}  // namespace antares

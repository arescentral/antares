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

static InterfaceStyle required_interface_style(path_value x) {
    return required_enum<InterfaceStyle>(
            x, {{"small", InterfaceStyle::SMALL}, {"large", InterfaceStyle::LARGE}});
}

static int16_t optional_key_num(path_value x) {
    auto k = optional_string(x);
    if (!k.has_value()) {
        return 0;
    }
    int i;
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

static std::unique_ptr<InterfaceItemData> interface_item(path_value x) {
    if (!x.value().is_map()) {
        throw std::runtime_error(pn::format("{0}must be a map", x.prefix()).c_str());
    }

    pn::string_view type = required_string(x.get("type"));
    if (type == "rect") {
        return std::unique_ptr<InterfaceItemData>(new BoxRectData(required_struct<BoxRectData>(
                x, {
                           {"type", nullptr},
                           {"bounds", {&InterfaceItemData::bounds, required_rect}},
                           {"id", {&InterfaceItemData::id, optional_int}},
                           {"label", {&BoxRectData::label, optional_string_copy}},
                           {"hue", {&BoxRectData::hue, required_hue}},
                           {"style", {&BoxRectData::style, required_interface_style}},
                   })));
    } else if (type == "button") {
        return std::unique_ptr<InterfaceItemData>(
                new PlainButtonData(required_struct<PlainButtonData>(
                        x,
                        {
                                {"type", nullptr},
                                {"bounds", {&InterfaceItemData::bounds, required_rect}},
                                {"id", {&InterfaceItemData::id, optional_int}},
                                {"label", {&PlainButtonData::label, required_string_copy}},
                                {"key", {&PlainButtonData::key, optional_key_num}},
                                {"gamepad", {&PlainButtonData::gamepad, optional_gamepad_button}},
                                {"hue", {&PlainButtonData::hue, required_hue}},
                                {"style", {&PlainButtonData::style, required_interface_style}},
                        })));
    } else if (type == "checkbox") {
        return std::unique_ptr<InterfaceItemData>(
                new CheckboxButtonData(required_struct<CheckboxButtonData>(
                        x,
                        {
                                {"type", nullptr},
                                {"bounds", {&InterfaceItemData::bounds, required_rect}},
                                {"id", {&InterfaceItemData::id, optional_int}},
                                {"label", {&CheckboxButtonData::label, required_string_copy}},
                                {"key", {&CheckboxButtonData::key, optional_key_num}},
                                {"gamepad",
                                 {&CheckboxButtonData::gamepad, optional_gamepad_button}},
                                {"hue", {&CheckboxButtonData::hue, required_hue}},
                                {"style", {&CheckboxButtonData::style, required_interface_style}},
                        })));
    } else if (type == "radio") {
        return std::unique_ptr<InterfaceItemData>(
                new RadioButtonData(required_struct<RadioButtonData>(
                        x,
                        {
                                {"type", nullptr},
                                {"bounds", {&InterfaceItemData::bounds, required_rect}},
                                {"id", {&InterfaceItemData::id, optional_int}},
                                {"label", {&RadioButtonData::label, required_string_copy}},
                                {"key", {&RadioButtonData::key, optional_key_num}},
                                {"gamepad", {&RadioButtonData::gamepad, optional_gamepad_button}},
                                {"hue", {&RadioButtonData::hue, required_hue}},
                                {"style", {&RadioButtonData::style, required_interface_style}},
                        })));
    } else if (type == "picture") {
        return std::unique_ptr<InterfaceItemData>(
                new PictureRectData(required_struct<PictureRectData>(
                        x, {
                                   {"type", nullptr},
                                   {"bounds", {&InterfaceItemData::bounds, required_rect}},
                                   {"id", {&InterfaceItemData::id, optional_int}},
                                   {"picture", {&PictureRectData::picture, required_string_copy}},
                           })));
    } else if (type == "text") {
        return std::unique_ptr<InterfaceItemData>(new TextRectData(required_struct<TextRectData>(
                x, {
                           {"type", nullptr},
                           {"bounds", {&InterfaceItemData::bounds, required_rect}},
                           {"id", {&InterfaceItemData::id, optional_int}},
                           {"text", {&TextRectData::text, optional_string, ""}},
                           {"hue", {&TextRectData::hue, required_hue}},
                           {"style", {&TextRectData::style, required_interface_style}},
                   })));
    } else if (type == "tab-box") {
        TabBoxData tab_box = required_struct<TabBoxData>(
                x, {
                           {"type", nullptr},
                           {"bounds", {&InterfaceItemData::bounds, required_rect}},
                           {"id", {&InterfaceItemData::id, optional_int}},
                           {"hue", {&TabBoxData::hue, required_hue}},
                           {"style", {&TabBoxData::style, required_interface_style}},
                           {"tabs", nullptr},
                   });

        path_value tabs = x.get("tabs");
        for (auto i : range(tabs.value().as_array().size())) {
            using Tab = TabBoxData::Tab;
            tab_box.tabs.emplace_back(required_struct<Tab>(
                    tabs.get(i),
                    {
                            {"id", {&Tab::id, optional_int}},
                            {"width", {&Tab::width, required_int}},
                            {"label", {&Tab::label, required_string_copy}},
                            {"content",
                             {&Tab::content,
                              required_array<std::unique_ptr<InterfaceItemData>, interface_item>}},
                    }));
        }

        return std::unique_ptr<InterfaceItemData>(new TabBoxData(std::move(tab_box)));
    } else {
        throw std::runtime_error(pn::format("{0}: unknown type: {1}", x.path(), type).c_str());
    }
}

InterfaceData interface(path_value x) {
    return required_struct<InterfaceData>(
            x, {{"fullscreen", {&InterfaceData::fullscreen, required_bool}},
                {"items",
                 {&InterfaceData::items,
                  required_array<std::unique_ptr<InterfaceItemData>, interface_item>}}});
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

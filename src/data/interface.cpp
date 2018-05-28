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
                           {"bounds", {&BoxRectData::bounds, required_rect}},
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
                                {"id", {&PlainButtonData::id, required_int}},
                                {"bounds", {&PlainButtonData::bounds, required_rect}},
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
                                {"id", {&CheckboxButtonData::id, required_int}},
                                {"bounds", {&CheckboxButtonData::bounds, required_rect}},
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
                                {"id", {&RadioButtonData::id, required_int}},
                                {"bounds", {&RadioButtonData::bounds, required_rect}},
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
                                   {"bounds", {&PictureRectData::bounds, required_rect}},
                                   {"picture", {&PictureRectData::picture, required_string_copy}},
                           })));
    } else if (type == "text") {
        return std::unique_ptr<InterfaceItemData>(new TextRectData(required_struct<TextRectData>(
                x, {
                           {"type", nullptr},
                           {"bounds", {&TextRectData::bounds, required_rect}},
                           {"text", {&TextRectData::text, optional_string, ""}},
                           {"hue", {&TextRectData::hue, required_hue}},
                           {"style", {&TextRectData::style, required_interface_style}},
                   })));
    } else if (type == "tab-box") {
        TabBoxData tab_box = required_struct<TabBoxData>(
                x, {
                           {"type", nullptr},
                           {"bounds", {&TabBoxData::bounds, required_rect}},
                           {"hue", {&TabBoxData::hue, required_hue}},
                           {"style", {&TabBoxData::style, required_interface_style}},
                           {"tabs", nullptr},
                   });
        Rect button_bounds = {
                tab_box.bounds.left + 22, tab_box.bounds.top - 20, 0, tab_box.bounds.top - 10,
        };

        path_value tabs = x.get("tabs");
        for (auto i : range(tabs.value().as_array().size())) {
            struct Tab {
                int64_t                                         id;
                int64_t                                         width;
                pn::string                                      label;
                std::vector<std::unique_ptr<InterfaceItemData>> content;
            };

            auto tab = required_struct<Tab>(
                    tabs.get(i),
                    {
                            {"id", {&Tab::id, required_int}},
                            {"width", {&Tab::width, required_int}},
                            {"label", {&Tab::label, required_string_copy}},
                            {"content",
                             {&Tab::content,
                              required_array<std::unique_ptr<InterfaceItemData>, interface_item>}},
                    });
            button_bounds.right = button_bounds.left + tab.width;
            TabBoxButtonData button;
            button.id      = tab.id;
            button.bounds  = button_bounds;
            button.label   = std::move(tab.label);
            button.hue     = tab_box.hue;
            button.style   = tab_box.style;
            button.content = std::move(tab.content);
            tab_box.buttons.emplace_back(std::move(button));
            button_bounds.left = button_bounds.right + 37;
        }

        tab_box.top_right_border_size = tab_box.bounds.right - button_bounds.right - 17;
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

BoxRectData BoxRectData::copy() const {
    std::unique_ptr<BoxRectData> copy(new BoxRectData);
    copy->bounds = bounds;
    if (label.has_value()) {
        copy->label.emplace(label->copy());
    }
    copy->hue   = hue;
    copy->style = style;
    return std::move(*copy);
}

TextRectData TextRectData::copy() const {
    std::unique_ptr<TextRectData> copy(new TextRectData);
    copy->bounds = bounds;
    copy->text   = text.copy();
    copy->hue    = hue;
    copy->style  = style;
    return std::move(*copy);
}

PictureRectData PictureRectData::copy() const {
    std::unique_ptr<PictureRectData> copy(new PictureRectData);
    copy->bounds  = bounds;
    copy->picture = picture.copy();
    return std::move(*copy);
}

PlainButtonData PlainButtonData::copy() const {
    std::unique_ptr<PlainButtonData> copy(new PlainButtonData);
    copy->bounds  = bounds;
    copy->id      = id;
    copy->label   = label.copy();
    copy->key     = key;
    copy->gamepad = gamepad;
    copy->hue     = hue;
    copy->style   = style;
    return std::move(*copy);
}

CheckboxButtonData CheckboxButtonData::copy() const {
    std::unique_ptr<CheckboxButtonData> copy(new CheckboxButtonData);
    copy->bounds  = bounds;
    copy->id      = id;
    copy->label   = label.copy();
    copy->key     = key;
    copy->gamepad = gamepad;
    copy->hue     = hue;
    copy->style   = style;
    return std::move(*copy);
}

RadioButtonData RadioButtonData::copy() const {
    std::unique_ptr<RadioButtonData> copy(new RadioButtonData);
    copy->bounds  = bounds;
    copy->id      = id;
    copy->label   = label.copy();
    copy->key     = key;
    copy->gamepad = gamepad;
    copy->hue     = hue;
    copy->style   = style;
    return std::move(*copy);
}

TabBoxButtonData TabBoxButtonData::copy() const {
    std::unique_ptr<TabBoxButtonData> copy(new TabBoxButtonData);
    copy->bounds  = bounds;
    copy->id      = id;
    copy->label   = label.copy();
    copy->key     = key;
    copy->gamepad = gamepad;
    copy->hue     = hue;
    copy->style   = style;

    struct EmplaceBackVisitor : InterfaceItemData::Visitor {
        std::vector<std::unique_ptr<InterfaceItemData>>* vec;
        EmplaceBackVisitor(std::vector<std::unique_ptr<InterfaceItemData>>* v) : vec{v} {}

        void visit_box_rect(const BoxRectData& data) const override {
            vec->emplace_back(new BoxRectData{data.copy()});
        }

        void visit_text_rect(const TextRectData& data) const override {
            vec->emplace_back(new TextRectData{data.copy()});
        }

        void visit_picture_rect(const PictureRectData& data) const override {
            vec->emplace_back(new PictureRectData{data.copy()});
        }

        void visit_plain_button(const PlainButtonData& data) const override {
            vec->emplace_back(new PlainButtonData{data.copy()});
        }

        void visit_radio_button(const RadioButtonData& data) const override {
            vec->emplace_back(new RadioButtonData{data.copy()});
        }

        void visit_checkbox_button(const CheckboxButtonData& data) const override {
            vec->emplace_back(new CheckboxButtonData{data.copy()});
        }

        void visit_tab_box(const TabBoxData& data) const override {
            vec->emplace_back(new TabBoxData{data.copy()});
        }

        void visit_tab_box_button(const TabBoxButtonData& data) const override {
            vec->emplace_back(new TabBoxButtonData{data.copy()});
        }
    };
    for (const auto& item : content) {
        item->accept(EmplaceBackVisitor{&copy->content});
    }
    return std::move(*copy);
}

TabBoxData TabBoxData::copy() const {
    std::unique_ptr<TabBoxData> copy(new TabBoxData);
    copy->bounds                = bounds;
    copy->hue                   = hue;
    copy->style                 = style;
    copy->top_right_border_size = top_right_border_size;
    return std::move(*copy);
}

void BoxRectData::accept(const Visitor& visitor) const { visitor.visit_box_rect(*this); }
void TextRectData::accept(const Visitor& visitor) const { visitor.visit_text_rect(*this); }
void PictureRectData::accept(const Visitor& visitor) const { visitor.visit_picture_rect(*this); }
void PlainButtonData::accept(const Visitor& visitor) const { visitor.visit_plain_button(*this); }
void CheckboxButtonData::accept(const Visitor& visitor) const {
    visitor.visit_checkbox_button(*this);
}
void RadioButtonData::accept(const Visitor& visitor) const { visitor.visit_radio_button(*this); }
void TabBoxButtonData::accept(const Visitor& visitor) const {
    visitor.visit_tab_box_button(*this);
}
void TabBoxData::accept(const Visitor& visitor) const { visitor.visit_tab_box(*this); }

InterfaceItemData::Visitor::~Visitor() {}

}  // namespace antares

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

Texture required_texture(path_value x) { return Resource::texture(required_string(x)); }

struct Tab {
    int64_t    width;
    pn::string label;
};

Interface interface(int id0, path_value x) {
    if (!x.value().is_array()) {
        throw std::runtime_error(pn::format("{0}must be array", x.prefix()).c_str());
    }

    Interface data;
    int       id = id0;
    for (auto i : range(x.value().as_array().size())) {
        path_value item = x.get(i);

        pn::string_view type = required_string(item.get("type"));
        if (type == "rect") {
            data.items.emplace_back(
                    required_struct<BoxRect>(
                            item, {{"type", nullptr},
                                   {"bounds", {&BoxRect::bounds, required_rect}},
                                   {"label", {&BoxRect::label, optional_string_copy}},
                                   {"hue", {&BoxRect::hue, required_hue}},
                                   {"style", {&BoxRect::style, required_interface_style}}})
                            .copy());
            data.items.back()->id = id++;
        } else if (type == "button") {
            data.items.emplace_back(new PlainButton(required_struct<PlainButton>(
                    item, {
                                  {"type", nullptr},
                                  {"bounds", {&PlainButton::bounds, required_rect}},
                                  {"label", {&PlainButton::label, required_string_copy}},
                                  {"key", {&PlainButton::key, optional_key_num}},
                                  {"gamepad", {&PlainButton::gamepad, optional_gamepad_button}},
                                  {"hue", {&PlainButton::hue, required_hue}},
                                  {"style", {&PlainButton::style, required_interface_style}},
                          })));
            data.items.back()->id = id++;
        } else if (type == "checkbox") {
            data.items.emplace_back(new CheckboxButton(required_struct<CheckboxButton>(
                    item, {
                                  {"type", nullptr},
                                  {"bounds", {&CheckboxButton::bounds, required_rect}},
                                  {"label", {&CheckboxButton::label, required_string_copy}},
                                  {"key", {&CheckboxButton::key, optional_key_num}},
                                  {"gamepad", {&CheckboxButton::gamepad, optional_gamepad_button}},
                                  {"hue", {&CheckboxButton::hue, required_hue}},
                                  {"style", {&CheckboxButton::style, required_interface_style}},
                          })));
            data.items.back()->id = id++;
        } else if (type == "radio") {
            data.items.emplace_back(new RadioButton(required_struct<RadioButton>(
                    item, {
                                  {"type", nullptr},
                                  {"bounds", {&RadioButton::bounds, required_rect}},
                                  {"label", {&RadioButton::label, required_string_copy}},
                                  {"key", {&RadioButton::key, optional_key_num}},
                                  {"gamepad", {&RadioButton::gamepad, optional_gamepad_button}},
                                  {"hue", {&RadioButton::hue, required_hue}},
                                  {"style", {&RadioButton::style, required_interface_style}},
                          })));
            data.items.back()->id = id++;
        } else if (type == "picture") {
            data.items.emplace_back(new PictureRect(required_struct<PictureRect>(
                    item, {
                                  {"type", nullptr},
                                  {"bounds", {&PictureRect::bounds, required_rect}},
                                  {"id", {&PictureRect::texture, required_texture}},
                          })));
            data.items.back()->id = id++;
        } else if (type == "text") {
            data.items.emplace_back(new TextRect(required_struct<TextRect>(
                    item, {
                                  {"type", nullptr},
                                  {"bounds", {&TextRect::bounds, required_rect}},
                                  {"text", {&TextRect::text, optional_string, ""}},
                                  {"hue", {&TextRect::hue, required_hue}},
                                  {"style", {&TextRect::style, required_interface_style}},
                          })));
            data.items.back()->id = id++;
        } else if (type == "tab-box") {
            TabBox tab_box = required_struct<TabBox>(
                    item, {
                                  {"type", nullptr},
                                  {"bounds", {&TabBox::bounds, required_rect}},
                                  {"hue", {&TabBox::hue, required_hue}},
                                  {"style", {&TabBox::style, required_interface_style}},
                                  {"tabs", nullptr},
                          });
            Rect button_bounds = {
                    tab_box.bounds.left + 22, tab_box.bounds.top - 20, 0, tab_box.bounds.top - 10,
            };

            path_value tabs = item.get("tabs");
            for (auto i : range(tabs.value().as_array().size())) {
                auto tab = required_struct<Tab>(
                        tabs.get(i), {
                                             {"width", {&Tab::width, required_int}},
                                             {"label", {&Tab::label, required_string_copy}},
                                             {"content", nullptr},
                                     });
                button_bounds.right = button_bounds.left + tab.width;
                TabBoxButton button;
                button.bounds      = button_bounds;
                button.label       = std::move(tab.label);
                button.hue         = tab_box.hue;
                button.style       = tab_box.style;
                button.tab_content = interface(0, tabs.get(i).get("content")).items;
                button.id          = id++;
                data.items.emplace_back(new TabBoxButton(std::move(button)));
                button_bounds.left = button_bounds.right + 37;
            }

            tab_box.top_right_border_size = tab_box.bounds.right - button_bounds.right - 17;
            tab_box.id                    = id++;
            data.items.emplace_back(new TabBox(std::move(tab_box)));
        } else {
            throw std::runtime_error(
                    pn::format("{0}: unknown type: {1}", item.path(), type).c_str());
        }
    }
    return data;
}

std::unique_ptr<InterfaceItem> BoxRect::copy() const {
    std::unique_ptr<BoxRect> copy(new BoxRect);
    copy->bounds = bounds;
    copy->id     = id;
    if (label.has_value()) {
        copy->label.emplace(label->copy());
    }
    copy->hue   = hue;
    copy->style = style;
    return std::move(copy);
}

std::unique_ptr<InterfaceItem> TextRect::copy() const {
    std::unique_ptr<TextRect> copy(new TextRect);
    copy->bounds = bounds;
    copy->id     = id;
    copy->text   = text.copy();
    copy->hue    = hue;
    copy->style  = style;
    return std::move(copy);
}

std::unique_ptr<InterfaceItem> PictureRect::copy() const {
    std::unique_ptr<PictureRect> copy(new PictureRect);
    copy->bounds = bounds;
    copy->id     = id;
    return std::move(copy);
}

std::unique_ptr<InterfaceItem> PlainButton::copy() const {
    std::unique_ptr<PlainButton> copy(new PlainButton);
    copy->bounds  = bounds;
    copy->id      = id;
    copy->label   = label.copy();
    copy->key     = key;
    copy->gamepad = gamepad;
    copy->hue     = hue;
    copy->style   = style;
    copy->status  = status;
    return std::move(copy);
}

std::unique_ptr<InterfaceItem> CheckboxButton::copy() const {
    std::unique_ptr<CheckboxButton> copy(new CheckboxButton);
    copy->bounds  = bounds;
    copy->id      = id;
    copy->label   = label.copy();
    copy->key     = key;
    copy->gamepad = gamepad;
    copy->hue     = hue;
    copy->style   = style;
    copy->status  = status;
    copy->on      = on;
    return std::move(copy);
}

std::unique_ptr<InterfaceItem> RadioButton::copy() const {
    std::unique_ptr<RadioButton> copy(new RadioButton);
    copy->bounds  = bounds;
    copy->id      = id;
    copy->label   = label.copy();
    copy->key     = key;
    copy->gamepad = gamepad;
    copy->hue     = hue;
    copy->style   = style;
    copy->status  = status;
    copy->on      = on;
    return std::move(copy);
}

std::unique_ptr<InterfaceItem> TabBoxButton::copy() const {
    std::unique_ptr<TabBoxButton> copy(new TabBoxButton);
    copy->bounds  = bounds;
    copy->id      = id;
    copy->label   = label.copy();
    copy->key     = key;
    copy->gamepad = gamepad;
    copy->hue     = hue;
    copy->style   = style;
    copy->status  = status;
    copy->on      = on;
    return std::move(copy);
}

std::unique_ptr<InterfaceItem> TabBox::copy() const {
    std::unique_ptr<TabBox> copy(new TabBox);
    copy->bounds                = bounds;
    copy->id                    = id;
    copy->hue                   = hue;
    copy->style                 = style;
    copy->top_right_border_size = top_right_border_size;
    return std::move(copy);
}

void BoxRect::accept(const Visitor& visitor) const { visitor.visit_box_rect(*this); }
void TextRect::accept(const Visitor& visitor) const { visitor.visit_text_rect(*this); }
void PictureRect::accept(const Visitor& visitor) const { visitor.visit_picture_rect(*this); }
void PlainButton::accept(const Visitor& visitor) const { visitor.visit_plain_button(*this); }
void CheckboxButton::accept(const Visitor& visitor) const { visitor.visit_checkbox_button(*this); }
void RadioButton::accept(const Visitor& visitor) const { visitor.visit_radio_button(*this); }
void TabBoxButton::accept(const Visitor& visitor) const { visitor.visit_tab_box_button(*this); }
void TabBox::accept(const Visitor& visitor) const { visitor.visit_tab_box(*this); }

InterfaceItem::Visitor::~Visitor() {}

}  // namespace antares

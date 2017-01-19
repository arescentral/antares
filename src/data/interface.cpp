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

#include <sfz/sfz.hpp>
#include "config/gamepad.hpp"
#include "config/keys.hpp"
#include "data/resource.hpp"
#include "data/string-list.hpp"
#include "drawing/color.hpp"
#include "lang/casts.hpp"
#include "video/driver.hpp"

using sfz::BytesSlice;
using sfz::Exception;
using sfz::Json;
using sfz::ReadSource;
using sfz::StringMap;
using sfz::StringSlice;
using sfz::format;
using sfz::range;
using std::unique_ptr;
using std::vector;

namespace utf8 = sfz::utf8;

namespace antares {

static Rect rect(const Json& json) {
    return {implicit_cast<int>(json.get("left").number()),
            implicit_cast<int>(json.get("top").number()),
            implicit_cast<int>(json.get("right").number()),
            implicit_cast<int>(json.get("bottom").number())};
}

static interfaceLabelType label(const Json& json) {
    return {
            int16_t(json.get("id").number()), int16_t(json.get("index").number() + 1),
    };
}

static uint8_t hue(const Json& json) {
    StringMap<uint8_t> hues;
    hues["gray"]        = GRAY;
    hues["orange"]      = ORANGE;
    hues["yellow"]      = YELLOW;
    hues["blue"]        = BLUE;
    hues["green"]       = GREEN;
    hues["purple"]      = PURPLE;
    hues["indigo"]      = INDIGO;
    hues["salmon"]      = SALMON;
    hues["gold"]        = GOLD;
    hues["aqua"]        = AQUA;
    hues["pink"]        = PINK;
    hues["pale-green"]  = PALE_GREEN;
    hues["pale-purple"] = PALE_PURPLE;
    hues["sky-blue"]    = SKY_BLUE;
    hues["tan"]         = TAN;
    hues["red"]         = RED;
    return hues[json.string()];
}

static interfaceStyleType style(const Json& json) {
    StringMap<interfaceStyleType> styles;
    styles["large"] = kLarge;
    styles["small"] = kSmall;
    return styles[json.string()];
}

static int16_t key(const Json& json) {
    int k;
    if (!GetKeyNameNum(json.string(), k)) {
        k = 0;
    }
    return k;
}

vector<unique_ptr<InterfaceItem>> interface_items(int id0, const Json& json) {
    vector<unique_ptr<InterfaceItem>> items;
    int                               id = id0;
    for (auto i : range(json.size())) {
        Json item_json = json.at(i);

        sfz::StringSlice kind;
        for (auto key : {"rect", "button", "checkbox", "radio", "picture", "text", "tab-box"}) {
            if (item_json.has(key)) {
                if (kind.empty()) {
                    kind = key;
                } else {
                    throw Exception(format("interface item has both {0} and {1}", kind, key));
                }
            }
        }

        Json               sub      = item_json.get(kind);
        Rect               bounds   = rect(item_json.get("bounds"));
        StringSlice        resource = sub.get("resource").string();
        uint8_t            hue      = antares::hue(sub.get("hue"));
        interfaceStyleType style    = antares::style(sub.get("style"));
        int16_t            key      = sub.has("key") ? antares::key(sub.get("key")) : 0;
        int16_t gamepad = sub.has("gamepad") ? Gamepad::num(sub.get("gamepad").string()) : 0;
        interfaceLabelType label =
                sub.has("label") ? antares::label(sub.get("label")) : interfaceLabelType{};

        if (kind == "rect") {
            if (sub.has("label")) {
                items.emplace_back(new LabeledRect(id++, bounds, label, hue, style));
            } else {
                items.emplace_back(new PlainRect(id++, bounds, hue, style));
            }
        } else if (kind == "button") {
            items.emplace_back(new PlainButton(id++, bounds, key, gamepad, label, hue, style));
        } else if (kind == "checkbox") {
            items.emplace_back(new CheckboxButton(id++, bounds, key, gamepad, label, hue, style));
        } else if (kind == "radio") {
            items.emplace_back(new RadioButton(id++, bounds, key, gamepad, label, hue, style));
        } else if (kind == "picture") {
            items.emplace_back(new PictureRect(id++, bounds, resource));
        } else if (kind == "text") {
            if (sub.has("resource")) {
                items.emplace_back(new TextRect(id++, bounds, resource, hue, style));
            } else {
                items.emplace_back(new TextRect(id++, bounds, hue, style));
            }
        } else if (kind == "tab-box") {
            Rect button_bounds = {
                    bounds.left + 22, bounds.top - 20, 0, bounds.top - 10,
            };
            Json tabs = sub.get("tabs");
            for (auto i : range(tabs.size())) {
                Json tab                 = tabs.at(i);
                button_bounds.right      = button_bounds.left + tab.get("width").number();
                interfaceLabelType label = antares::label(tab.get("label"));
                items.emplace_back(new TabBoxButton(
                        id++, button_bounds, key, gamepad, label, hue, style, tab.get("content")));
                button_bounds.left = button_bounds.right + 37;
            }
            int16_t top_right_border_size = bounds.right - button_bounds.right - 17;
            items.emplace_back(new TabBox(id++, bounds, hue, style, top_right_border_size));
        }
    }
    return items;
}

InterfaceItem::InterfaceItem(int id, Rect bounds) : id(id), _bounds(bounds) {}

PlainRect::PlainRect(int id, Rect bounds, uint8_t hue, interfaceStyleType style)
        : InterfaceItem(id, bounds), hue(hue), style(style) {}

void PlainRect::accept(const Visitor& visitor) const {
    visitor.visit_plain_rect(*this);
}

LabeledItem::LabeledItem(int id, Rect bounds, interfaceLabelType label)
        : InterfaceItem(id, bounds),
          label(StringList(label.stringID).at(label.stringNumber - 1)) {}

LabeledRect::LabeledRect(
        int id, Rect bounds, interfaceLabelType label, uint8_t hue, interfaceStyleType style)
        : LabeledItem(id, bounds, label), hue(hue), style(style) {}

void LabeledRect::accept(const Visitor& visitor) const {
    visitor.visit_labeled_rect(*this);
}

TextRect::TextRect(int id, Rect bounds, uint8_t hue, interfaceStyleType style)
        : InterfaceItem(id, bounds), hue(hue), style(style) {}

TextRect::TextRect(
        int id, Rect bounds, StringSlice resource, uint8_t hue, interfaceStyleType style)
        : InterfaceItem(id, bounds),
          text(utf8::decode(Resource(resource).data())),
          hue(hue),
          style(style) {}

void TextRect::accept(const Visitor& visitor) const {
    visitor.visit_text_rect(*this);
}

PictureRect::PictureRect(int id, Rect bounds, StringSlice resource)
        : InterfaceItem(id, bounds),
          picture(resource),
          texture(picture.texture()),
          visible_bounds(false),
          hue(GRAY),
          style(kSmall) {}

void PictureRect::accept(const Visitor& visitor) const {
    visitor.visit_picture_rect(*this);
}

Button::Button(
        int id, Rect bounds, int16_t key, int16_t gamepad, interfaceLabelType label, uint8_t hue,
        interfaceStyleType style)
        : LabeledItem(id, bounds, label),
          key(key),
          gamepad(gamepad),
          hue(hue),
          style(style),
          status(kActive) {}

PlainButton::PlainButton(
        int id, Rect bounds, int16_t key, int16_t gamepad, interfaceLabelType label, uint8_t hue,
        interfaceStyleType style)
        : Button(id, bounds, key, gamepad, label, hue, style) {}

void PlainButton::accept(const Visitor& visitor) const {
    visitor.visit_plain_button(*this);
}

CheckboxButton::CheckboxButton(
        int id, Rect bounds, int16_t key, int16_t gamepad, interfaceLabelType label, uint8_t hue,
        interfaceStyleType style)
        : Button(id, bounds, key, gamepad, label, hue, style), on(false) {}

void CheckboxButton::accept(const Visitor& visitor) const {
    visitor.visit_checkbox_button(*this);
}

RadioButton::RadioButton(
        int id, Rect bounds, int16_t key, int16_t gamepad, interfaceLabelType label, uint8_t hue,
        interfaceStyleType style)
        : Button(id, bounds, key, gamepad, label, hue, style), on(false) {}

void RadioButton::accept(const Visitor& visitor) const {
    visitor.visit_radio_button(*this);
}

TabBoxButton::TabBoxButton(
        int id, Rect bounds, int16_t key, int16_t gamepad, interfaceLabelType label, uint8_t hue,
        interfaceStyleType style, const Json& tab_content)
        : Button(id, bounds, key, gamepad, label, hue, style),
          on(false),
          tab_content(tab_content) {}

void TabBoxButton::accept(const Visitor& visitor) const {
    visitor.visit_tab_box_button(*this);
}

TabBox::TabBox(
        int id, Rect bounds, uint8_t hue, interfaceStyleType style, int16_t top_right_border_size)
        : InterfaceItem(id, bounds),
          hue(hue),
          style(style),
          top_right_border_size(top_right_border_size) {}

void TabBox::accept(const Visitor& visitor) const {
    visitor.visit_tab_box(*this);
}

InterfaceItem::Visitor::~Visitor() {}

}  // namespace antares

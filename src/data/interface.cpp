// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// Copyright (C) 2008-2012 The Antares Authors
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
#include "config/keys.hpp"
#include "drawing/color.hpp"

using sfz::BytesSlice;
using sfz::Exception;
using sfz::Json;
using sfz::ReadSource;
using sfz::StringMap;
using sfz::range;
using std::vector;

namespace antares {

interfaceItemStatusType interfaceItemType::status() const {
    switch (kind) {
      case kPlainButton:
        return item.plainButton.status;
      case kRadioButton:
      case kTabBoxButton:
        return item.radioButton.status;
      case kCheckboxButton:
        return item.checkboxButton.status;
      case kTextRect:
        return item.textRect.visibleBounds ? kActive : kDimmed;
      case kPictureRect:
        return item.pictureRect.visibleBounds ? kActive : kDimmed;
      default:
        return kDimmed;
    }
}

void interfaceItemType::set_status(interfaceItemStatusType status) {
    switch (kind) {
      case kPlainButton:
        item.plainButton.status = status;
        break;
      case kRadioButton:
      case kTabBoxButton:
        item.radioButton.status = status;
        break;
      case kCheckboxButton:
        item.checkboxButton.status = status;
        break;
      case kTextRect:
        item.textRect.visibleBounds = (status == kActive);
        break;
      case kPictureRect:
        item.pictureRect.visibleBounds = (status == kActive);
        break;
      default:
        break;
    }
}

int interfaceItemType::key() const {
    switch (kind) {
      case kPlainButton:
        return item.plainButton.key;
      case kTabBoxButton:
        return item.radioButton.key;
      default:
        return 0;
    }
}

void interfaceItemType::set_key(int key) {
    switch (kind) {
      case kPlainButton:
        item.plainButton.key = key;
        break;
      case kTabBoxButton:
        item.radioButton.key = key;
        break;
      default:
        break;
    }
}

void interfaceItemType::set_label(interfaceLabelType label) {
    switch (kind) {
      case kLabeledRect:
        item.labeledRect.label = label;
        break;
      case kListRect:
        item.listRect.label = label;
        break;
      case kPlainButton:
        item.plainButton.label = label;
        break;
      case kRadioButton:
      case kTabBoxButton:
        item.radioButton.label = label;
        break;
      case kCheckboxButton:
        item.checkboxButton.label = label;
        break;
      default:
        break;
    }
}

static Rect rect(const Json& json) {
    return {
        json.get("left").number(),
        json.get("top").number(),
        json.get("right").number(),
        json.get("bottom").number()
    };
}

static interfaceLabelType label(const Json& json) {
    return {
        int16_t(json.get("id").number()),
        int16_t(json.get("index").number() + 1),
    };
}

static uint8_t hue(const Json& json) {
    StringMap<uint8_t> hues;
    hues["gray"] = GRAY;
    hues["orange"] = ORANGE;
    hues["yellow"] = YELLOW;
    hues["blue"] = BLUE;
    hues["green"] = GREEN;
    hues["purple"] = PURPLE;
    hues["indigo"] = INDIGO;
    hues["salmon"] = SALMON;
    hues["gold"] = GOLD;
    hues["aqua"] = AQUA;
    hues["pink"] = PINK;
    hues["pale-green"] = PALE_GREEN;
    hues["pale-purple"] = PALE_PURPLE;
    hues["sky-blue"] = SKY_BLUE;
    hues["tan"] = TAN;
    hues["red"] = RED;
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

vector<interfaceItemType> interface_items(const Json& json) {
    vector<interfaceItemType> items;
    for (auto i: range(json.size())) {
        Json item_json = json.at(i);
        interfaceItemType item = {};
        item.bounds = rect(item_json.get("bounds"));

        sfz::StringSlice kind;
        for (auto key: {"rect", "button", "checkbox", "radio", "picture", "text", "tab-box"}) {
            if (item_json.has(key)) {
                if (kind.empty()) {
                    kind = key;
                } else {
                    throw Exception(format("interface item has both {0} and {1}", kind, key));
                }
            }
        }

        Json sub = item_json.get(kind);
        if (kind == "rect") {
            if (sub.has("label")) {
                item.kind = kLabeledRect;
            } else {
                item.kind = kPlainRect;
                item.item.pictureRect.visibleBounds = true;
            }
        } else if (kind == "button") {
            item.kind = kPlainButton;
            item.set_status(kActive);
        } else if (kind == "checkbox") {
            item.kind = kCheckboxButton;
            item.set_status(kActive);
        } else if (kind == "radio") {
            item.kind = kRadioButton;
            item.set_status(kActive);
        } else if (kind == "picture") {
            item.kind = kPictureRect;
            item.item.pictureRect.pictureID = sub.get("id").number();
        } else if (kind == "text") {
            item.kind = kTextRect;
            item.item.textRect.textID = sub.get("id").number();
        } else if (kind == "tab-box") {
            item.kind = kTabBox;
            interfaceItemType button = {};
            button.kind = kTabBoxButton;
            button.bounds = {
                item.bounds.left + 22,
                item.bounds.top - 20,
                0,
                item.bounds.top - 10,
            };
            button.color = hue(sub.get("hue"));
            button.style = style(sub.get("style"));
            button.set_status(kActive);
            Json tabs = sub.get("tabs");
            for (auto i: range(tabs.size())) {
                Json tab = tabs.at(i);
                button.bounds.right = button.bounds.left + tab.get("width").number();
                button.set_label(label(tab.get("label")));
                button.tab_content = interface_items(tab.get("content"));
                items.emplace_back(button);
                button.bounds.left = button.bounds.right + 37;
            }
            item.item.tabBox.topRightBorderSize = item.bounds.right - button.bounds.right - 17;
        }

        item.color = hue(sub.get("hue"));
        item.style = style(sub.get("style"));
        if (sub.has("key")) {
            item.set_key(key(sub.get("key")));
        }
        if (sub.has("label")) {
            item.set_label(label(sub.get("label")));
        }
        items.emplace_back(item);
    }
    return items;
}

}  // namespace antares

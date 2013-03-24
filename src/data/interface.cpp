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
using std::unique_ptr;
using std::vector;

namespace antares {

InterfaceItem::InterfaceItem() {
    memset(&_item, 0, sizeof(_item));
    _hue = 0;
    _kind = kPlainRect;
    _style = kLarge;
}

InterfaceItem::InterfaceItem(const InterfaceItem& other) {
    _bounds = other._bounds;
    memcpy(&_item, &other._item, sizeof(_item));
    _hue = other._hue;
    _kind = other._kind;
    _style = other._style;
    for (const auto& item: other.tab_content()) {
        _tab_content.emplace_back(item->clone());
    }
}

unique_ptr<InterfaceItem> InterfaceItem::clone() const {
    return unique_ptr<InterfaceItem>(new InterfaceItem(*this));
}

void InterfaceItem::set_hue(uint8_t hue) {
    _hue = hue;
}

interfaceItemStatusType InterfaceItem::status() const {
    switch (_kind) {
      case kPlainButton:
        return _item.plainButton.status;
      case kRadioButton:
      case kTabBoxButton:
        return _item.radioButton.status;
      case kCheckboxButton:
        return _item.checkboxButton.status;
      case kTextRect:
        return _item.textRect.visibleBounds ? kActive : kDimmed;
      case kPictureRect:
        return _item.pictureRect.visibleBounds ? kActive : kDimmed;
      default:
        return kDimmed;
    }
}

void InterfaceItem::set_status(interfaceItemStatusType status) {
    switch (_kind) {
      case kPlainButton:
        _item.plainButton.status = status;
        break;
      case kRadioButton:
      case kTabBoxButton:
        _item.radioButton.status = status;
        break;
      case kCheckboxButton:
        _item.checkboxButton.status = status;
        break;
      case kTextRect:
        _item.textRect.visibleBounds = (status == kActive);
        break;
      case kPictureRect:
        _item.pictureRect.visibleBounds = (status == kActive);
        break;
      default:
        break;
    }
}

bool InterfaceItem::on() const {
    switch (_kind) {
      case kCheckboxButton:
        return _item.checkboxButton.on;
      case kRadioButton:
      case kTabBoxButton:
        return _item.radioButton.on;
      default:
        return false;
    }
}

void InterfaceItem::set_on(bool on) {
    switch (_kind) {
      case kCheckboxButton:
        _item.checkboxButton.on = on;
        break;
      case kRadioButton:
      case kTabBoxButton:
        _item.radioButton.on = on;
        break;
      default:
        break;
    }
}

int InterfaceItem::key() const {
    switch (_kind) {
      case kPlainButton:
        return _item.plainButton.key;
      case kTabBoxButton:
        return _item.radioButton.key;
      default:
        return 0;
    }
}

void InterfaceItem::set_key(int key) {
    switch (_kind) {
      case kPlainButton:
        _item.plainButton.key = key;
        break;
      case kTabBoxButton:
        _item.radioButton.key = key;
        break;
      default:
        break;
    }
}

interfaceLabelType InterfaceItem::label() const {
    switch (_kind) {
      case kLabeledRect:
        return _item.labeledRect.label;
      case kListRect:
        return _item.listRect.label;
      case kPlainButton:
        return _item.plainButton.label;
      case kRadioButton:
      case kTabBoxButton:
        return _item.radioButton.label;
      case kCheckboxButton:
        return _item.checkboxButton.label;
      default:
        return interfaceLabelType{};
    }
}

void InterfaceItem::set_label(interfaceLabelType label) {
    switch (_kind) {
      case kLabeledRect:
        _item.labeledRect.label = label;
        break;
      case kListRect:
        _item.listRect.label = label;
        break;
      case kPlainButton:
        _item.plainButton.label = label;
        break;
      case kRadioButton:
      case kTabBoxButton:
        _item.radioButton.label = label;
        break;
      case kCheckboxButton:
        _item.checkboxButton.label = label;
        break;
      default:
        break;
    }
}

int16_t InterfaceItem::id() const {
    switch (_kind) {
      case kPictureRect:
        return _item.pictureRect.pictureID;
      case kTextRect:
        return _item.textRect.textID;
      default:
        return -1;
    }
}

int16_t InterfaceItem::top_right_border_size() const {
    switch (_kind) {
      case kTabBox:
        return _item.tabBox.topRightBorderSize;
      default:
        return 0;
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

vector<unique_ptr<InterfaceItem>> interface_items(const Json& json) {
    vector<unique_ptr<InterfaceItem>> items;
    for (auto i: range(json.size())) {
        Json item_json = json.at(i);
        InterfaceItem item = {};
        item._bounds = rect(item_json.get("bounds"));

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
                item._kind = kLabeledRect;
            } else {
                item._kind = kPlainRect;
                item._item.pictureRect.visibleBounds = true;
            }
        } else if (kind == "button") {
            item._kind = kPlainButton;
            item.set_status(kActive);
        } else if (kind == "checkbox") {
            item._kind = kCheckboxButton;
            item.set_status(kActive);
        } else if (kind == "radio") {
            item._kind = kRadioButton;
            item.set_status(kActive);
        } else if (kind == "picture") {
            item._kind = kPictureRect;
            item._item.pictureRect.pictureID = sub.get("id").number();
        } else if (kind == "text") {
            item._kind = kTextRect;
            item._item.textRect.textID = sub.get("id").number();
        } else if (kind == "tab-box") {
            item._kind = kTabBox;
            InterfaceItem button = {};
            button._kind = kTabBoxButton;
            button._bounds = {
                item._bounds.left + 22,
                item._bounds.top - 20,
                0,
                item._bounds.top - 10,
            };
            button._hue = hue(sub.get("hue"));
            button._style = style(sub.get("style"));
            button.set_status(kActive);
            Json tabs = sub.get("tabs");
            for (auto i: range(tabs.size())) {
                Json tab = tabs.at(i);
                button._bounds.right = button._bounds.left + tab.get("width").number();
                button.set_label(label(tab.get("label")));
                button._tab_content = interface_items(tab.get("content"));
                items.emplace_back(button.clone());
                button._bounds.left = button._bounds.right + 37;
            }
            item._item.tabBox.topRightBorderSize = item._bounds.right - button._bounds.right - 17;
        }

        item._hue = hue(sub.get("hue"));
        item._style = style(sub.get("style"));
        if (sub.has("key")) {
            item.set_key(key(sub.get("key")));
        }
        if (sub.has("label")) {
            item.set_label(label(sub.get("label")));
        }
        items.emplace_back(item.clone());
    }
    return items;
}

InterfaceItem labeled_rect(
        Rect bounds, uint8_t hue, interfaceStyleType style, int16_t str_id, int str_num) {
    InterfaceItem item = {};
    item._bounds = bounds;
    item._kind = kLabeledRect;
    item._hue = hue;
    item._style = style;
    item._item.labeledRect.label.stringID = str_id;
    item._item.labeledRect.label.stringNumber = str_num;
    return item;
}

}  // namespace antares

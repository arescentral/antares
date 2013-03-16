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

#include "ui/screen.hpp"

#include <sfz/sfz.hpp>

#include "config/keys.hpp"
#include "data/resource.hpp"
#include "drawing/color.hpp"
#include "drawing/pix-map.hpp"
#include "game/time.hpp"
#include "sound/fx.hpp"
#include "ui/interface-handling.hpp"
#include "video/driver.hpp"

using sfz::Exception;
using sfz::Json;
using sfz::JsonDefaultVisitor;
using sfz::String;
using sfz::StringMap;
using sfz::StringSlice;
using sfz::format;
using sfz::read;
using sfz::string_to_json;
using std::unique_ptr;
using std::vector;

namespace utf8 = sfz::utf8;

namespace antares {

namespace {

struct InterfaceScreenVisitor : public JsonDefaultVisitor {
    enum StateEnum {
        NEW,
        ITEM,
        BOUNDS, BOUNDS_LEFT, BOUNDS_TOP, BOUNDS_RIGHT, BOUNDS_BOTTOM,
        LABEL,
        RECT, BUTTON, CHECKBOX, RADIO, PICTURE, TEXT,
        TAB_BOX, TAB_BOX_TABS, TAB_BOX_TAB,
        KEY, COLOR, STYLE, ID, INDEX, WIDTH,
        DONE,
    };
    struct State {
        StateEnum state;
        Rect bounds;
        bool has_label;
        bool has_key;
        int16_t id;
        int16_t index;
        int key;
        int color;
        int style;
        int next_tab_box_button;
        int top_right_border_size;
        int width;
        State(): state(NEW) { }
    };
    State& state;
    vector<interfaceItemType>& items;

    InterfaceScreenVisitor(State& state, vector<interfaceItemType>& items):
            state(state),
            items(items) { }

    bool descend(StateEnum state, const StringMap<Json>& value, StringSlice key) const {
        auto it = value.find(key);
        if (it == value.end()) {
            return false;
        }
        StateEnum saved_state = this->state.state;
        this->state.state = state;
        it->second.accept(*this);
        this->state.state = saved_state;
        return true;
    }

    void build() const {
        items.emplace_back();
        interfaceItemType& item = items.back();
        item.bounds = state.bounds;
        item.color = state.color;
        item.style = state.style;
        switch (state.state) {
          case RECT:
            if (state.has_label) {
                item.kind = kLabeledRect;
                item.item.labeledRect.label.stringID = state.id;
                item.item.labeledRect.label.stringNumber = state.index + 1;
                item.item.labeledRect.color = 0;
                item.item.labeledRect.editable = false;
            } else {
                item.kind = kPlainRect;
                item.item.pictureRect.pictureID = 0;
                item.item.pictureRect.visibleBounds = true;
            }
            break;

          case BUTTON:
            item.kind = kPlainButton;
            item.item.plainButton.label.stringID = state.id;
            item.item.plainButton.label.stringNumber = state.index + 1;
            item.item.plainButton.defaultButton = 0;
            item.item.plainButton.key = state.has_key ? state.key : 0;
            item.item.plainButton.status = kActive;
            break;

          case CHECKBOX:
            item.kind = kCheckboxButton;
            item.item.checkboxButton.label.stringID = state.id;
            item.item.checkboxButton.label.stringNumber = state.index + 1;
            item.item.checkboxButton.on = false;
            item.item.checkboxButton.key = state.has_key ? state.key : 0;
            item.item.checkboxButton.status = kActive;
            break;

          case RADIO:
            item.kind = kRadioButton;
            item.item.radioButton.label.stringID = state.id;
            item.item.radioButton.label.stringNumber = state.index + 1;
            item.item.radioButton.on = false;
            item.item.radioButton.key = state.has_key ? state.key : 0;
            item.item.radioButton.status = kActive;
            break;

          case PICTURE:
            item.kind = kPictureRect;
            item.item.pictureRect.pictureID = state.id;
            item.item.pictureRect.visibleBounds = false;
            break;

          case TEXT:
            item.kind = kTextRect;
            item.item.textRect.textID = state.id;
            item.item.textRect.visibleBounds = false;
            break;

          case TAB_BOX:
            item.kind = kTabBox;
            item.item.tabBox.topRightBorderSize = state.top_right_border_size;
            break;

          case TAB_BOX_TAB:
            item.kind = kTabBoxButton;
            item.item.radioButton.label.stringID = state.id;
            item.item.radioButton.label.stringNumber = state.index + 1;
            item.item.radioButton.on = false;
            item.item.radioButton.key = state.has_key ? state.key : 0;
            item.item.radioButton.status = kActive;
            break;

          default:
            break;
        }
    }

    virtual void visit_object(const StringMap<Json>& value) const {
        switch (state.state) {
          case ITEM:
            {
                state.has_key = false;
                state.has_label = false;
                state.color = GRAY;
                state.style = kLarge;
                if (!descend(BOUNDS, value, "bounds")) {
                    throw Exception("missing bounds in sprite json");
                }
                int kind_count =
                    descend(RECT, value, "rect") +
                    descend(BUTTON, value, "button") +
                    descend(CHECKBOX, value, "checkbox") +
                    descend(RADIO, value, "radio") +
                    descend(PICTURE, value, "picture") +
                    descend(TEXT, value, "text") +
                    descend(TAB_BOX, value, "tab-box") +
                    descend(TAB_BOX_TAB, value, "tab-box-button");
                if (kind_count == 0) {
                    throw Exception("missing item kind in interface json");
                } else if (kind_count > 1) {
                    throw Exception("too many item kinds in interface json");
                }
            }
            break;

          case BOUNDS:
            if (!descend(BOUNDS_LEFT, value, "left") ||
                    !descend(BOUNDS_TOP, value, "top") ||
                    !descend(BOUNDS_RIGHT, value, "right") ||
                    !descend(BOUNDS_BOTTOM, value, "bottom")) {
                throw Exception("bad bounds rect");
            }
            break;

          case LABEL:
            if (!descend(ID, value, "id") ||
                    !descend(INDEX, value, "index")) {
                throw Exception("bad label");
            }
            break;

          case RECT:
            state.has_label = descend(LABEL, value, "label");
            if (!descend(COLOR, value, "color") ||
                    !descend(STYLE, value, "style")) {
                throw Exception("bad rect");
            }
            build();
            break;

          case BUTTON:
          case CHECKBOX:
          case RADIO:
            state.has_label = true;
            state.has_key = descend(KEY, value, "key");
            if (!descend(LABEL, value, "label") ||
                    !descend(COLOR, value, "color") ||
                    !descend(STYLE, value, "style")) {
                throw Exception("bad button");
            }
            build();
            break;

          case PICTURE:
            if (!descend(ID, value, "id")) {
                throw Exception("bad text");
            }
            build();
            break;

          case TEXT:
            if (!descend(COLOR, value, "color") ||
                    !descend(STYLE, value, "style") ||
                    !descend(ID, value, "id")) {
                throw Exception("bad text");
            }
            build();
            break;

          case TAB_BOX:
            if (!descend(COLOR, value, "color") ||
                    !descend(STYLE, value, "style")) {
                throw Exception("bad tab box");
            }
            {
                Rect box_bounds = state.bounds;
                state.bounds.top -= 20;
                state.bounds.bottom = state.bounds.top + 10;
                state.next_tab_box_button = state.bounds.left + 22;
                if (!descend(TAB_BOX_TABS, value, "tabs")) {
                    throw Exception("bad tab box");
                }
                state.bounds = box_bounds;
                state.top_right_border_size = box_bounds.right - state.next_tab_box_button + 20;
                build();
            }
            break;

          case TAB_BOX_TAB:
            state.has_label = true;
            if (!descend(LABEL, value, "label") ||
                    !descend(WIDTH, value, "width")) {
                throw Exception("bad tab box tab");
            }
            state.bounds.left = state.next_tab_box_button;
            state.bounds.right = state.bounds.left + state.width;
            state.next_tab_box_button += state.width + 37;
            build();
            break;

          default:
            return visit_default("object");
        }
    }

    virtual void visit_array(const std::vector<Json>& value) const {
        switch (state.state) {
          case NEW:
            state.state = ITEM;
            for (const auto& v: value) {
                v.accept(*this);
            }
            state.state = DONE;
            break;

          case TAB_BOX_TABS:
            state.state = TAB_BOX_TAB;
            for (const auto& v: value) {
                v.accept(*this);
            }
            state.state = TAB_BOX_TABS;
            break;

          default:
            return visit_default("array");
        }
    }

    virtual void visit_string(const StringSlice& value) const {
        switch (state.state) {
          case KEY:
            if (!GetKeyNameNum(value, state.key)) {
                throw Exception(format("invalid key {0} in interface json", quote(value)));
            }
            break;

          case COLOR:
            {
                StringMap<int> color_map;
                color_map["gray"] = GRAY;
                color_map["orange"] = ORANGE;
                color_map["yellow"] = YELLOW;
                color_map["blue"] = BLUE;
                color_map["green"] = GREEN;
                color_map["purple"] = PURPLE;
                color_map["indigo"] = INDIGO;
                color_map["salmon"] = SALMON;
                color_map["gold"] = GOLD;
                color_map["aqua"] = AQUA;
                color_map["pink"] = PINK;
                color_map["pale-green"] = PALE_GREEN;
                color_map["pale-purple"] = PALE_PURPLE;
                color_map["sky-blue"] = SKY_BLUE;
                color_map["tan"] = TAN;
                color_map["red"] = RED;
                if (color_map.find(value) == color_map.end()) {
                    throw Exception(format("invalid color {0} in interface json", quote(value)));
                }
                state.color = color_map[value];
            }
            break;

          case STYLE:
            if (value == "large") {
                state.style = kLarge;
            } else if (value == "small") {
                state.style = kSmall;
            } else {
                throw Exception(format("invalid style {0} in interface json", quote(value)));
            }
            break;

          default:
            return visit_default("string");
        }
    }

    virtual void visit_number(double value) const {
        switch (state.state) {
          case ID: state.id = value; break;
          case INDEX: state.index = value; break;
          case WIDTH: state.width = value; break;
          case BOUNDS_LEFT: state.bounds.left = value; break;
          case BOUNDS_TOP: state.bounds.top = value; break;
          case BOUNDS_RIGHT: state.bounds.right = value; break;
          case BOUNDS_BOTTOM: state.bounds.bottom = value; break;
          default:
            return visit_default("number");
        }
    }

    virtual void visit_default(const char* type) const {
        throw Exception(format("unexpected {0} in sprite json", type));
    }
};

}  // namespace

InterfaceScreen::InterfaceScreen(Id id, const Rect& bounds, bool full_screen)
        : _state(NORMAL),
          _bounds(bounds),
          _full_screen(full_screen),
          _hit_item(0) {
    Resource rsrc("interfaces", "json", id);
    String in(utf8::decode(rsrc.data()));
    Json json;
    if (!string_to_json(in, json)) {
        throw Exception("invalid interface JSON");
    }
    InterfaceScreenVisitor::State state;
    json.accept(InterfaceScreenVisitor(state, _items));
    const int offset_x = (_bounds.width() / 2) - 320;
    const int offset_y = (_bounds.height() / 2) - 240;
    for (auto& item: _items) {
        item.bounds.offset(offset_x, offset_y);
    }
}

InterfaceScreen::~InterfaceScreen() { }

void InterfaceScreen::become_front() {
    this->adjust_interface();
    // half-second fade from black.
}

void InterfaceScreen::draw() const {
    Rect copy_area;
    if (_full_screen) {
        copy_area = _bounds;
    } else {
        next()->draw();
        GetAnyInterfaceItemGraphicBounds(_items[0], &copy_area);
        for (size_t i = 1; i < _items.size(); ++i) {
            Rect r;
            GetAnyInterfaceItemGraphicBounds(_items[i], &r);
            copy_area.enlarge_to(r);
        }
    }

    copy_area.offset(_bounds.left, _bounds.top);
    VideoDriver::driver()->fill_rect(copy_area, RgbColor::kBlack);

    for (vector<interfaceItemType>::const_iterator it = _items.begin(); it != _items.end(); ++it) {
        interfaceItemType copy = *it;
        copy.bounds.left += _bounds.left;
        copy.bounds.top += _bounds.top;
        copy.bounds.right += _bounds.left;
        copy.bounds.bottom += _bounds.top;
        draw_interface_item(copy);
    }
}

void InterfaceScreen::mouse_down(const MouseDownEvent& event) {
    Point where = event.where();
    where.h -= _bounds.left;
    where.v -= _bounds.top;
    if (event.button() != 0) {
        return;
    }
    for (size_t i = 0; i < _items.size(); ++i) {
        interfaceItemType* const item = &_items[i];
        Rect bounds;
        GetAnyInterfaceItemGraphicBounds(*item, &bounds);
        if (item->status() != kDimmed && bounds.contains(where)) {
            switch (item->kind) {
              case kPlainButton:
              case kCheckboxButton:
              case kRadioButton:
              case kTabBoxButton:
                _state = MOUSE_DOWN;
                item->set_status(kIH_Hilite);
                PlayVolumeSound(kComputerBeep1, kMediumLoudVolume, kShortPersistence,
                        kMustPlaySound);
                _hit_item = i;
                return;

              case kLabeledRect:
                return;

              case kListRect:
                throw Exception("kListRect not yet handled");

              default:
                break;
            }
        }
    }
    return;
}

void InterfaceScreen::mouse_up(const MouseUpEvent& event) {
    Point where = event.where();
    where.h -= _bounds.left;
    where.v -= _bounds.top;
    if (event.button() != 0) {
        return;
    }
    if (_state == MOUSE_DOWN) {
        // Save _hit_item and set it to 0 before calling handle_button(), as calling
        // handle_button() can result in the deletion of `this`.
        int hit_item = _hit_item;
        _hit_item = 0;

        _state = NORMAL;
        interfaceItemType* const item = &_items[hit_item];
        Rect bounds;
        GetAnyInterfaceItemGraphicBounds(*item, &bounds);
        item->set_status(kActive);
        if (bounds.contains(where)) {
            handle_button(hit_item);
        }
    }
    return;
}

void InterfaceScreen::mouse_move(const MouseMoveEvent& event) {
    // TODO(sfiera): highlight and un-highlight clicked button as dragged in and out.
    static_cast<void>(event);
}

void InterfaceScreen::key_down(const KeyDownEvent& event) {
    const int32_t key_code = event.key() + 1;
    for (size_t i = 0; i < _items.size(); ++i) {
        interfaceItemType* const item = &_items[i];
        if (item->status() != kDimmed && item->key() == key_code) {
            _state = KEY_DOWN;
            item->set_status(kIH_Hilite);
            PlayVolumeSound(kComputerBeep1, kMediumLoudVolume, kShortPersistence, kMustPlaySound);
            _hit_item = i;
            return;
        }
    }
}

void InterfaceScreen::key_up(const KeyUpEvent& event) {
    // TODO(sfiera): verify that the same key that was pressed was released.
    static_cast<void>(event);
    if (_state == KEY_DOWN) {
        // Save _hit_item and set it to 0 before calling handle_button(), as calling
        // handle_button() can result in the deletion of `this`.
        int hit_item = _hit_item;
        _hit_item = 0;

        _state = NORMAL;
        interfaceItemType* const item = &_items[hit_item];
        item->set_status(kActive);
        if (item->kind == kTabBoxButton) {
            item->item.radioButton.on = true;
        }
        handle_button(hit_item);
    }
}

void InterfaceScreen::adjust_interface() { }

void InterfaceScreen::truncate(size_t size) {
    if (size > _items.size()) {
        throw Exception("");
    }
    _items.resize(size);
}

void InterfaceScreen::extend(Id id, size_t within) {
    if (size() <= within) {
        throw Exception("interfaces must be extended within existing elements");
    }
    vector<interfaceItemType> new_items;

    Resource rsrc("interfaces", "json", id);
    String in(utf8::decode(rsrc.data()));
    Json json;
    if (!string_to_json(in, json)) {
        throw Exception("invalid interface JSON");
    }
    InterfaceScreenVisitor::State state;
    json.accept(InterfaceScreenVisitor(state, new_items));
    Rect all_bounds = new_items[0].bounds;
    for (const auto& item: new_items) {
        all_bounds.enlarge_to(item.bounds);
    }

    Rect centered_bounds(all_bounds);
    centered_bounds.center_in(_items[within].bounds);
    const int off_x = centered_bounds.left - all_bounds.left;
    const int off_y = centered_bounds.top - all_bounds.top;
    for (vector<interfaceItemType>::iterator it = new_items.begin(); it != new_items.end(); ++it) {
        it->bounds.offset(off_x, off_y);
    }
    _items.insert(_items.end(), new_items.begin(), new_items.end());
}

size_t InterfaceScreen::size() const {
    return _items.size();
}

const interfaceItemType& InterfaceScreen::item(int i) const {
    return _items[i];
}

interfaceItemType* InterfaceScreen::mutable_item(int i) {
    return &_items[i];
}

void InterfaceScreen::offset(int offset_x, int offset_y) {
    for (vector<interfaceItemType>::iterator it = _items.begin(); it != _items.end(); ++it) {
        it->bounds.offset(offset_x, offset_y);
    }
}

}  // namespace antares

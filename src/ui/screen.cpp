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

#include "ui/screen.hpp"

#include <sfz/sfz.hpp>

#include "config/gamepad.hpp"
#include "config/keys.hpp"
#include "data/resource.hpp"
#include "drawing/color.hpp"
#include "drawing/pix-map.hpp"
#include "game/sys.hpp"
#include "game/time.hpp"
#include "sound/fx.hpp"
#include "ui/interface-handling.hpp"
#include "video/driver.hpp"

using sfz::Exception;
using sfz::Json;
using sfz::PrintItem;
using sfz::String;
using sfz::StringMap;
using sfz::StringSlice;
using sfz::format;
using sfz::range;
using sfz::read;
using sfz::string_to_json;
using std::unique_ptr;
using std::vector;

namespace utf8 = sfz::utf8;

namespace antares {

InterfaceScreen::InterfaceScreen(PrintItem name, const Rect& bounds, bool full_screen)
        : InterfaceScreen(load_json(name), bounds, full_screen) {}

InterfaceScreen::InterfaceScreen(sfz::Json json, const Rect& bounds, bool full_screen)
        : _state(NORMAL), _bounds(bounds), _full_screen(full_screen), _hit_button(nullptr) {
    _items = interface_items(0, json);
    for (auto& item : _items) {
        item->bounds().offset(bounds.left, bounds.top);
    }
}

InterfaceScreen::~InterfaceScreen() {}

Json InterfaceScreen::load_json(sfz::PrintItem id) {
    Resource rsrc(sfz::format("interfaces/{0}.json", id));
    String   in(utf8::decode(rsrc.data()));
    Json     json;
    if (!string_to_json(in, json)) {
        throw Exception("invalid interface JSON");
    }
    return json;
}

void InterfaceScreen::become_front() {
    this->adjust_interface();
    // half-second fade from black.
}

void InterfaceScreen::resign_front() {
    become_normal();
}

void InterfaceScreen::become_normal() {
    _state      = NORMAL;
    _hit_button = nullptr;
    for (auto& item : _items) {
        Button* button = dynamic_cast<Button*>(item.get());
        if (button && button->status == kIH_Hilite) {
            button->status = kActive;
        }
    }
}

void InterfaceScreen::draw() const {
    Point off = offset();
    Rect  copy_area;
    if (_full_screen) {
        copy_area = _bounds;
    } else {
        next()->draw();
        GetAnyInterfaceItemGraphicBounds(*_items[0], &copy_area);
        for (const auto& item : _items) {
            Rect r;
            GetAnyInterfaceItemGraphicBounds(*item, &r);
            copy_area.enlarge_to(r);
        }
    }
    copy_area.offset(off.h, off.v);

    Rects().fill(copy_area, RgbColor::black());

    for (const auto& item : _items) {
        draw_interface_item(*item, sys.video->input_mode(), off);
    }
    overlay();
    if (stack()->top() == this) {
        _cursor.draw();
    }
}

void InterfaceScreen::mouse_down(const MouseDownEvent& event) {
    Point where = event.where();
    Point off   = offset();
    where.h -= off.h;
    where.v -= off.v;
    if (event.button() != 0) {
        return;
    }
    for (auto& item : _items) {
        Rect bounds;
        GetAnyInterfaceItemGraphicBounds(*item, &bounds);
        Button* button = dynamic_cast<Button*>(item.get());
        if (button && (button->status != kDimmed) && (bounds.contains(where))) {
            become_normal();
            _state         = MOUSE_DOWN;
            button->status = kIH_Hilite;
            sys.sound.select();
            _hit_button = button;
            return;
        }
    }
}

void InterfaceScreen::mouse_up(const MouseUpEvent& event) {
    Point where = event.where();
    Point off   = offset();
    where.h -= _bounds.left + off.h;
    where.v -= _bounds.top + off.v;
    if (event.button() != 0) {
        return;
    }
    if (_state == MOUSE_DOWN) {
        _state = NORMAL;
        Rect bounds;
        GetAnyInterfaceItemGraphicBounds(*_hit_button, &bounds);
        _hit_button->status = kActive;
        if (bounds.contains(where)) {
            handle_button(*_hit_button);
        }
    }
}

void InterfaceScreen::mouse_move(const MouseMoveEvent& event) {
    // TODO(sfiera): highlight and un-highlight clicked button as dragged in and out.
    static_cast<void>(event);
}

void InterfaceScreen::key_down(const KeyDownEvent& event) {
    const int32_t key_code = event.key() + 1;
    for (auto& item : _items) {
        Button* button = dynamic_cast<Button*>(item.get());
        if (button && button->status != kDimmed && button->key == key_code) {
            become_normal();
            _state         = KEY_DOWN;
            button->status = kIH_Hilite;
            sys.sound.select();
            _hit_button = button;
            _pressed    = key_code;
            return;
        }
    }
}

void InterfaceScreen::key_up(const KeyUpEvent& event) {
    const int32_t key_code = event.key() + 1;
    if ((_state == KEY_DOWN) && (_pressed == key_code)) {
        _state              = NORMAL;
        _hit_button->status = kActive;
        if (TabBoxButton* b = dynamic_cast<TabBoxButton*>(_hit_button)) {
            b->on = true;
        }
        handle_button(*_hit_button);
    }
}

void InterfaceScreen::gamepad_button_down(const GamepadButtonDownEvent& event) {
    for (auto& item : _items) {
        Button* button = dynamic_cast<Button*>(item.get());
        if (button && button->status != kDimmed && button->gamepad == event.button) {
            become_normal();
            _state         = GAMEPAD_DOWN;
            button->status = kIH_Hilite;
            sys.sound.select();
            _hit_button = button;
            _pressed    = event.button;
            return;
        }
    }
}

void InterfaceScreen::gamepad_button_up(const GamepadButtonUpEvent& event) {
    if ((_state == GAMEPAD_DOWN) && (_pressed == event.button)) {
        _state              = NORMAL;
        _hit_button->status = kActive;
        if (TabBoxButton* b = dynamic_cast<TabBoxButton*>(_hit_button)) {
            b->on = true;
        }
        handle_button(*_hit_button);
    }
}

void InterfaceScreen::overlay() const {}

void InterfaceScreen::adjust_interface() {}

void InterfaceScreen::truncate(size_t size) {
    if (size > _items.size()) {
        throw Exception("");
    }
    _items.resize(size);
}

void InterfaceScreen::extend(const Json& json) {
    const int offset_x = (_bounds.width() / 2) - 320;
    const int offset_y = (_bounds.height() / 2) - 240;
    for (auto&& item : interface_items(_items.size(), json)) {
        _items.emplace_back(std::move(item));
        _items.back()->bounds().offset(offset_x, offset_y);
    }
}

Point InterfaceScreen::offset() const {
    Rect bounds = {0, 0, 640, 480};
    bounds.center_in(sys.video->screen_size().as_rect());
    return bounds.origin();
}

size_t InterfaceScreen::size() const {
    return _items.size();
}

const InterfaceItem& InterfaceScreen::item(int i) const {
    return *_items[i];
}

InterfaceItem& InterfaceScreen::mutable_item(int i) {
    return *_items[i];
}

}  // namespace antares

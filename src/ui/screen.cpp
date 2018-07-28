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

#include <pn/array>
#include <pn/file>

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

using std::unique_ptr;
using std::vector;

namespace antares {

InterfaceScreen::InterfaceScreen(pn::string_view name, const Rect& bounds) : _bounds(bounds) {
    try {
        InterfaceData data = Resource::interface(name);
        _full_screen       = data.fullscreen;
        for (auto& item : data.items) {
            _widgets.push_back(Widget::from(item));
        }
    } catch (...) {
        std::throw_with_nested(std::runtime_error(name.copy().c_str()));
    }
}

InterfaceScreen::~InterfaceScreen() {}

void InterfaceScreen::become_front() {
    // half-second fade from black.
}

void InterfaceScreen::resign_front() { set_state(NORMAL); }

void InterfaceScreen::set_state(State state, Widget* widget, Key key, Gamepad::Button gamepad) {
    _state           = state;
    _key_pressed     = key;
    _gamepad_pressed = gamepad;
    if (widget) {  // Even if already the active widget.
        sys.sound.select();
    }
    if (widget != _active_widget) {
        if (_active_widget) {
            _active_widget->deactivate();
            _active_widget = nullptr;
        }
        if (widget) {
            widget->activate();
            _active_widget = widget;
        }
    }
}

template <typename Widgets>
static void enlarge_to_outer_bounds(Rect* r, const Widgets& widgets) {
    for (const auto& w : widgets) {
        r->enlarge_to(w->outer_bounds());
        enlarge_to_outer_bounds(r, w->children());
    }
}

void InterfaceScreen::draw() const {
    Rect copy_area;
    if (_full_screen) {
        copy_area = _bounds;
    } else {
        next()->draw();
        copy_area = _widgets[0]->outer_bounds();
        enlarge_to_outer_bounds(&copy_area, _widgets);
    }
    Point off = offset();
    copy_area.offset(off.h, off.v);

    Rects().fill(copy_area, RgbColor::black());

    for (const auto& w : _widgets) {
        w->draw(off, sys.video->input_mode());
    }
    overlay();
    if (stack()->top() == this) {
        _cursor.draw();
    }
}

void InterfaceScreen::mouse_down(const MouseDownEvent& event) {
    Point where = event.where();
    Point off   = offset();
    where.offset(-off.h, -off.v);
    if (event.button() != 0) {
        return;
    }
    for (auto& widget : _widgets) {
        if (Widget* item = widget->accept_click(where)) {
            set_state(MOUSE_DOWN, item);
            return;
        }
    }
}

void InterfaceScreen::mouse_up(const MouseUpEvent& event) {
    Point where = event.where();
    Point off   = offset();
    where.offset(-off.h, -off.v);
    if (event.button() != 0) {
        return;
    }
    if (_state == MOUSE_DOWN) {
        if (_active_widget->accept_click(where)) {
            Widget* w = _active_widget;
            set_state(NORMAL);
            w->action();
        } else {
            set_state(NORMAL);
        }
    }
}

void InterfaceScreen::mouse_move(const MouseMoveEvent& event) {
    // TODO(sfiera): highlight and un-highlight clicked button as dragged in and out.
    static_cast<void>(event);
}

void InterfaceScreen::key_down(const KeyDownEvent& event) {
    for (auto& widget : _widgets) {
        if (Widget* item = widget->accept_key(event.key())) {
            set_state(KEY_DOWN, item, event.key());
            return;
        }
    }
}

void InterfaceScreen::key_up(const KeyUpEvent& event) {
    if ((_state == KEY_DOWN) && (_key_pressed == event.key())) {
        Widget* w = _active_widget;
        set_state(NORMAL);
        w->action();
    }
}

void InterfaceScreen::gamepad_button_down(const GamepadButtonDownEvent& event) {
    for (auto& widget : _widgets) {
        if (Widget* item = widget->accept_button(event.button)) {
            set_state(GAMEPAD_DOWN, item, Key::NONE, event.button);
            return;
        }
    }
}

void InterfaceScreen::gamepad_button_up(const GamepadButtonUpEvent& event) {
    if ((_state == GAMEPAD_DOWN) && (_gamepad_pressed == event.button)) {
        Widget* w = _active_widget;
        set_state(NORMAL);
        w->action();
    }
}

void InterfaceScreen::overlay() const {}

Point InterfaceScreen::offset() const {
    Rect screen = {0, 0, 640, 480};
    screen.center_in(sys.video->screen_size().as_rect());
    return {_bounds.left + screen.left, _bounds.top + screen.top};
}

template <typename Vector>
static Widget* find_id(const Vector& v, int64_t id) {
    for (auto& widget : v) {
        if (widget->id().has_value() && (*widget->id() == id)) {
            return &*widget;
        }
        auto in_children = find_id(widget->children(), id);
        if (in_children) {
            return in_children;
        }
    }
    return nullptr;
}

const Widget* InterfaceScreen::widget(int id) const { return find_id(_widgets, id); }

Widget* InterfaceScreen::widget(int id) { return find_id(_widgets, id); }

const PlainButton* InterfaceScreen::button(int id) const {
    return dynamic_cast<const PlainButton*>(widget(id));
}

PlainButton* InterfaceScreen::button(int id) { return dynamic_cast<PlainButton*>(widget(id)); }

const CheckboxButton* InterfaceScreen::checkbox(int id) const {
    return dynamic_cast<const CheckboxButton*>(widget(id));
}

CheckboxButton* InterfaceScreen::checkbox(int id) {
    return dynamic_cast<CheckboxButton*>(widget(id));
}

}  // namespace antares

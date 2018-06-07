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

namespace {

struct EmplaceBackVisitor : InterfaceItemData::Visitor {
    std::vector<std::unique_ptr<Widget>>* vec;

    EmplaceBackVisitor(std::vector<std::unique_ptr<Widget>>* v) : vec{v} {}

    void visit_box_rect(const BoxRectData& data) const override {
        vec->emplace_back(new BoxRect{data});
    }

    void visit_text_rect(const TextRectData& data) const override {
        vec->emplace_back(new TextRect{data});
    }

    void visit_picture_rect(const PictureRectData& data) const override {
        vec->emplace_back(new PictureRect{data});
    }

    void visit_plain_button(const PlainButtonData& data) const override {
        vec->emplace_back(new PlainButton{data});
    }

    void visit_radio_button(const RadioButtonData& data) const override {
        vec->emplace_back(new RadioButton{data});
    }

    void visit_checkbox_button(const CheckboxButtonData& data) const override {
        vec->emplace_back(new CheckboxButton{data});
    }

    void visit_tab_box(const TabBoxData& data) const override {
        vec->emplace_back(new TabBox{data});
    }
};

}  // namespace

InterfaceScreen::InterfaceScreen(pn::string_view name, const Rect& bounds) : _bounds(bounds) {
    try {
        InterfaceData data = Resource::interface(name);
        _full_screen       = data.fullscreen;
        for (auto& item : data.items) {
            item->accept(EmplaceBackVisitor{&_widgets});
        }
    } catch (...) {
        std::throw_with_nested(std::runtime_error(name.copy().c_str()));
    }
}

InterfaceScreen::~InterfaceScreen() {}

void InterfaceScreen::become_front() {
    this->adjust_interface();
    // half-second fade from black.
}

void InterfaceScreen::resign_front() { become_normal(); }

void InterfaceScreen::become_normal() {
    _state = NORMAL;
    if (_active_widget) {
        _active_widget->deactivate();
        _active_widget = nullptr;
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
            Button* button = dynamic_cast<Button*>(item);
            become_normal();
            _state           = MOUSE_DOWN;
            button->active() = true;
            sys.sound.select();
            _active_widget = button;
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
        _state      = NORMAL;
        Rect bounds = _active_widget->outer_bounds();
        _active_widget->deactivate();
        if (bounds.contains(where) && _active_widget->id().has_value()) {
            handle_button(_active_widget);
        }
    }
}

void InterfaceScreen::mouse_move(const MouseMoveEvent& event) {
    // TODO(sfiera): highlight and un-highlight clicked button as dragged in and out.
    static_cast<void>(event);
}

void InterfaceScreen::key_down(const KeyDownEvent& event) {
    const int32_t key_code = event.key() + 1;
    for (auto& widget : _widgets) {
        if (Widget* item = widget->accept_key(key_code)) {
            Button* button = dynamic_cast<Button*>(item);
            become_normal();
            _state           = KEY_DOWN;
            button->active() = true;
            sys.sound.select();
            _active_widget = button;
            _pressed       = key_code;
            return;
        }
    }
}

void InterfaceScreen::key_up(const KeyUpEvent& event) {
    const int32_t key_code = event.key() + 1;
    if ((_state == KEY_DOWN) && (_pressed == key_code) && _active_widget->id().has_value()) {
        _state = NORMAL;
        _active_widget->deactivate();
        handle_button(_active_widget);
    }
}

void InterfaceScreen::gamepad_button_down(const GamepadButtonDownEvent& event) {
    for (auto& widget : _widgets) {
        if (Widget* item = widget->accept_button(event.button)) {
            Button* button = dynamic_cast<Button*>(item);
            become_normal();
            _state           = GAMEPAD_DOWN;
            button->active() = true;
            sys.sound.select();
            _active_widget = button;
            _pressed       = event.button;
            return;
        }
    }
}

void InterfaceScreen::gamepad_button_up(const GamepadButtonUpEvent& event) {
    if ((_state == GAMEPAD_DOWN) && (_pressed == event.button) &&
        _active_widget->id().has_value()) {
        _state = NORMAL;
        _active_widget->deactivate();
        handle_button(_active_widget);
    }
}

void InterfaceScreen::overlay() const {}

void InterfaceScreen::adjust_interface() {}

void InterfaceScreen::handle_button(Widget* widget) {
    if (auto* b = dynamic_cast<PlainButton*>(widget)) {
        b->action();
        return;
    }
    if (auto* b = dynamic_cast<CheckboxButton*>(widget)) {
        b->set(!b->get());
        return;
    }
    if (auto* b = dynamic_cast<TabButton*>(widget)) {
        b->parent()->select(*b);
        return;
    }
}

Point InterfaceScreen::offset() const {
    Rect screen = {0, 0, 640, 480};
    screen.center_in(sys.video->screen_size().as_rect());
    return {_bounds.left + screen.left, _bounds.top + screen.top};
}

size_t InterfaceScreen::size() const { return _widgets.size(); }

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

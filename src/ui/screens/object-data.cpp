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

#include "ui/screens/object-data.hpp"

#include "drawing/styled-text.hpp"
#include "drawing/text.hpp"
#include "game/globals.hpp"
#include "game/sys.hpp"
#include "game/time.hpp"
#include "ui/interface-handling.hpp"
#include "video/driver.hpp"

namespace antares {

namespace {

const int32_t kShipDataWidth = 240;
const usecs   kTypingDelay   = kMinorTick;

Rect object_data_bounds(Point origin, Size size) {
    Rect bounds(Point(0, 0), size);
    bounds.center_in(Rect(origin, Size(0, 0)));
    Rect inside = world();
    inside.inset(9, 5);
    if (bounds.left < inside.left) {
        bounds.offset(inside.left - bounds.left, 0);
    }
    if (bounds.right > inside.right) {
        bounds.offset(inside.right - bounds.right, 0);
    }
    if (bounds.top < inside.top) {
        bounds.offset(0, inside.top - bounds.top);
    }
    if (bounds.bottom > inside.bottom) {
        bounds.offset(0, inside.bottom - bounds.bottom);
    }
    return bounds;
}

}  // namespace

ObjectDataScreen::ObjectDataScreen(
        Point origin, const BaseObject& object, Trigger trigger, int mouse, Key key,
        Gamepad::Button gamepad)
        : _trigger(trigger), _mouse(mouse), _key(key), _gamepad(gamepad), _state(TYPING) {
    pn::string text;
    CreateObjectDataText(text, object);
    _text.reset(new StyledText(sys.fonts.button));
    _text->set_fore_color(GetRGBTranslateColorShade(Hue::GREEN, LIGHTEST));
    _text->set_back_color(GetRGBTranslateColorShade(Hue::GREEN, DARKEST));
    _text->set_retro_text(text);
    _text->wrap_to(kShipDataWidth, 0, 0);
    _bounds = object_data_bounds(origin, Size(_text->auto_width(), _text->height()));
}

ObjectDataScreen::~ObjectDataScreen() {}

void ObjectDataScreen::become_front() {
    _state       = TYPING;
    _typed_chars = 0;
    _next_update = now() + kTypingDelay;
    _next_sound  = _next_update;
}

bool ObjectDataScreen::next_timer(wall_time& time) {
    if (_state == TYPING) {
        time = _next_update;
        return true;
    }
    return false;
}

void ObjectDataScreen::fire_timer() {
    wall_time now = antares::now();
    if (_next_sound <= now) {
        sys.sound.teletype();
        _next_sound += 3 * kTypingDelay;
        while (_next_sound <= now) {
            _next_sound += kTypingDelay;
        }
    }
    while (_next_update <= now) {
        if (_typed_chars < _text->size()) {
            _next_update += kTypingDelay;
            ++_typed_chars;
        } else {
            _next_update = wall_time();
            _state       = DONE;
            break;
        }
    }
}

void ObjectDataScreen::mouse_up(const MouseUpEvent& event) {
    if ((_trigger == MOUSE) && (event.button() == _mouse)) {
        stack()->pop(this);
    }
}

void ObjectDataScreen::key_up(const KeyUpEvent& event) {
    if ((_trigger == KEY) && (event.key() == _key)) {
        stack()->pop(this);
    }
}

void ObjectDataScreen::gamepad_button_up(const GamepadButtonUpEvent& event) {
    if ((_trigger == GAMEPAD) && (event.button == _gamepad)) {
        stack()->pop(this);
    }
}

void ObjectDataScreen::draw() const {
    next()->draw();
    Rect outside = _bounds;
    outside.inset(-8, -4);
    const RgbColor light_green = GetRGBTranslateColorShade(Hue::GREEN, LIGHTEST);
    Rects().fill(outside, light_green);
    outside.inset(1, 1);
    Rects().fill(outside, RgbColor::black());
    _text->draw_range(_bounds, 0, _typed_chars);
    if (_typed_chars < _text->size()) {
        _text->draw_cursor(_bounds, _typed_chars);
    }
}

}  // namespace antares

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

#include "video/transitions.hpp"

#include "config/keys.hpp"
#include "data/picture.hpp"
#include "drawing/color.hpp"
#include "game/globals.hpp"
#include "game/main.hpp"
#include "game/time.hpp"
#include "math/units.hpp"
#include "sound/music.hpp"
#include "ui/card.hpp"
#include "video/driver.hpp"

using sfz::format;

namespace antares {

namespace {

const int32_t kStartAnimation = -255;
const int32_t kEndAnimation   = 255;

}  // namespace

Transitions::Transitions() : _active(false) {}
Transitions::~Transitions() {}

void Transitions::reset() {
    _active = false;
}

void Transitions::start_boolean(int32_t in_speed, int32_t out_speed, uint8_t goal_color) {
    _step        = kStartAnimation;
    _in_speed    = in_speed;
    _out_speed   = out_speed;
    _color       = GetRGBTranslateColor(GetRetroIndex(goal_color));
    _color.alpha = 127;
    if (!_active) {
        _active = true;
    }
}

void Transitions::update_boolean(ticks time_passed) {
    if (_active) {
        if (_step < 0) {
            _step += _in_speed * time_passed.count();
        } else if ((_step + _out_speed * time_passed.count()) < kEndAnimation) {
            _step += _out_speed * time_passed.count();
        } else {
            _active = false;
        }
    }
}

void Transitions::draw() const {
    if (_active) {
        Rects().fill(world(), _color);
    }
}

ColorFade::ColorFade(
        Direction direction, const RgbColor& color, usecs duration, bool allow_skip, bool* skipped)
        : _direction(direction),
          _color(color),
          _allow_skip(allow_skip),
          _skipped(skipped),
          _duration(duration) {}

void ColorFade::become_front() {
    _start      = now();
    _next_event = _start + kMinorTick;
}

void ColorFade::mouse_down(const MouseDownEvent& event) {
    static_cast<void>(event);
    if (_allow_skip) {
        *_skipped = true;
        stack()->pop(this);
    }
}

void ColorFade::key_down(const KeyDownEvent& event) {
    static_cast<void>(event);
    if (_allow_skip) {
        *_skipped = true;
        stack()->pop(this);
    }
}

void ColorFade::gamepad_button_down(const GamepadButtonDownEvent& event) {
    static_cast<void>(event);
    if (_allow_skip) {
        *_skipped = true;
        stack()->pop(this);
    }
}

bool ColorFade::next_timer(wall_time& time) {
    time = _next_event;
    return true;
}

void ColorFade::fire_timer() {
    wall_time now = antares::now();
    while (_next_event < now) {
        _next_event = _next_event + kMinorTick;
    }
    double fraction = static_cast<double>((now - _start).count()) / _duration.count();
    if (fraction >= 1.0) {
        stack()->pop(this);
    }
}

void ColorFade::draw() const {
    next()->draw();
    wall_time now      = antares::now();
    double    fraction = static_cast<double>((now - _start).count()) / _duration.count();
    if (fraction > 1.0) {
        fraction = 1.0;
    }
    RgbColor fill_color = _color;
    if (_direction == TO_COLOR) {
        fill_color.alpha = 0xff * fraction;
    } else {
        fill_color.alpha = 0xff * (1.0 - fraction);
    }
    Rects().fill(world(), fill_color);
}

PictFade::PictFade(int pict_id, bool* skipped)
        : _state(NEW), _skipped(skipped), _texture(Picture(pict_id).texture()) {}

PictFade::~PictFade() {}

void PictFade::become_front() {
    switch (_state) {
        case NEW: wax(); break;

        case WAXING:
            if (!this->skip()) {
                _state      = FULL;
                _wane_start = now() + this->display_time();
                break;
            }
        // fall through.

        case WANING:
            _state = NEW;
            stack()->pop(this);
            break;

        default: break;
    }
}

void PictFade::mouse_down(const MouseDownEvent& event) {
    static_cast<void>(event);
    *_skipped = true;
    if (this->skip()) {
        stack()->pop(this);
    } else {
        wane();
    }
}

void PictFade::key_down(const KeyDownEvent& event) {
    static_cast<void>(event);
    *_skipped = true;
    if (this->skip()) {
        stack()->pop(this);
    } else {
        wane();
    }
}

bool PictFade::next_timer(wall_time& time) {
    if (_state == FULL) {
        time = _wane_start;
        return true;
    }
    return false;
}

void PictFade::fire_timer() {
    // Timer only fires when _state == FULL.
    wane();
}

void PictFade::draw() const {
    Rect bounds = _texture.size().as_rect();
    bounds.center_in(world());
    _texture.draw(bounds.left, bounds.top);
}

void PictFade::wax() {
    _state = WAXING;
    stack()->push(new ColorFade(
            ColorFade::FROM_COLOR, RgbColor::black(), this->fade_time(), true, _skipped));
}

void PictFade::wane() {
    _state = WANING;
    stack()->push(new ColorFade(
            ColorFade::TO_COLOR, RgbColor::black(), this->fade_time(), true, _skipped));
}

usecs PictFade::fade_time() const {
    return ticks(100);
}

usecs PictFade::display_time() const {
    return ticks(80);
}

bool PictFade::skip() const {
    return *_skipped;
}

}  // namespace antares

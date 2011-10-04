// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// Copyright (C) 2008-2011 Ares Central
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
// License along with this program.  If not, see
// <http://www.gnu.org/licenses/>.

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
const int32_t kEndAnimation = 255;

}  // namespace

Transitions::Transitions() { }
Transitions::~Transitions() { }

void Transitions::start_boolean(int32_t in_speed, int32_t out_speed, uint8_t goal_color) {
    _step = kStartAnimation;
    _in_speed = in_speed;
    _out_speed = out_speed;
    const RgbColor goal = GetRGBTranslateColor(GetRetroIndex(goal_color));
    VideoDriver::driver()->set_transition_to(goal);
    if (!_active) {
        VideoDriver::driver()->set_transition_fraction(0.5);
        _active = true;
    }
}

void Transitions::update_boolean(int32_t time_passed) {
    if (_active) {
        if (_step < 0) {
            _step += _in_speed * time_passed;
        } else if ((_step + _out_speed * time_passed) < kEndAnimation) {
            _step += _out_speed * time_passed;
        } else {
            VideoDriver::driver()->set_transition_fraction(0.0);
            _active = false;
        }
    }
}

ColorFade::ColorFade(
        Direction direction, const RgbColor& color, int64_t duration, bool allow_skip,
        bool* skipped)
        : _direction(direction),
          _color(color),
          _allow_skip(allow_skip),
          _skipped(skipped),
          _next_event(0.0),
          _duration(duration) { }

void ColorFade::become_front() {
    _start = now_usecs();
    _next_event = _start + kTimeUnit;
    VideoDriver::driver()->set_transition_to(_color);
    VideoDriver::driver()->set_transition_fraction(_direction);
}

void ColorFade::resign_front() {
    VideoDriver::driver()->set_transition_to(_color);
    VideoDriver::driver()->set_transition_fraction(0.0);
}

void ColorFade::mouse_down(const MouseDownEvent& event) {
    static_cast<void>(event);
    if (_allow_skip) {
        *_skipped = true;
        stack()->pop(this);
    }
}

bool ColorFade::next_timer(int64_t& time) {
    time = _next_event;
    return true;
}

void ColorFade::fire_timer() {
    int64_t now = now_usecs();
    while (_next_event < now) {
        _next_event += kTimeUnit;
    }
    double fraction = static_cast<double>(now - _start) / _duration;
    if (fraction < 1.0) {
        if (_direction == TO_COLOR) {
            VideoDriver::driver()->set_transition_fraction(fraction);
        } else {
            VideoDriver::driver()->set_transition_fraction(1.0 - fraction);
        }
    } else {
        stack()->pop(this);
    }
}

void ColorFade::draw() const {
    next()->draw();
}

PictFade::PictFade(int pict_id, bool* skipped)
        : _state(NEW),
          _skipped(skipped) {
    Picture pict(pict_id);
    _sprite.reset(VideoDriver::driver()->new_sprite(format("/pictures/{0}.png", pict_id), pict));
}

PictFade::~PictFade() { }

void PictFade::become_front() {
    switch (_state) {
      case NEW:
        wax();
        break;

      case WAXING:
        if (!this->skip()) {
            _state = FULL;
            _wane_start = now_usecs() + this->display_time();
            break;
        }
        // fall through.

      case WANING:
        _state = NEW;
        stack()->pop(this);
        break;

      default:
        break;
    }
}

void PictFade::resign_front() {
    if (_state == NEW) {
        VideoDriver::driver()->set_transition_fraction(0.0);
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

bool PictFade::next_timer(int64_t& time) {
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
    Rect bounds = _sprite->size().as_rect();
    bounds.center_in(world);
    _sprite->draw(bounds.left, bounds.top);
}

void PictFade::wax() {
    _state = WAXING;
    stack()->push(new ColorFade(ColorFade::FROM_COLOR, RgbColor::kBlack, this->fade_time(), true,
                _skipped));
}

void PictFade::wane() {
    _state = WANING;
    stack()->push(new ColorFade(ColorFade::TO_COLOR, RgbColor::kBlack, this->fade_time(), true,
                _skipped));
}

int64_t PictFade::fade_time() const {
    return 5e6 / 3;
}

int64_t PictFade::display_time() const {
    return 4e6 / 3;
}

bool PictFade::skip() const {
    return *_skipped;
}

}  // namespace antares

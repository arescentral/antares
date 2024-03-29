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

#include "ui/screens/loading.hpp"

#include "data/resource.hpp"
#include "drawing/styled-text.hpp"
#include "drawing/text.hpp"
#include "game/globals.hpp"
#include "game/level.hpp"
#include "game/sys.hpp"
#include "game/time.hpp"
#include "sound/fx.hpp"
#include "video/driver.hpp"

namespace antares {

static const Hue      kLoadingScreenColor = Hue::PALE_GREEN;
static const RgbColor kLoadingForeColor   = GetRGBTranslateColorShade(Hue::PALE_GREEN, LIGHTEST);
static const ticks    kTypingDelay        = kMinorTick;
static constexpr char kLoadingRect[]      = "loading";

LoadingScreen::LoadingScreen(const Level& level, bool* cancelled)
        : InterfaceScreen("loading", {0, 0, 640, 480}),
          _state(TYPING),
          _level(level),
          _cancelled(cancelled),
          _name_text{StyledText::retro(
                  level.base.name, {sys.fonts.title, 640, 0, 2, 220}, kLoadingForeColor)},
          _next_update(now() + kTypingDelay),
          _next_teletype(_next_update) {
    _name_text.hide();
}

LoadingScreen::~LoadingScreen() {}

void LoadingScreen::become_front() {}

bool LoadingScreen::next_timer(wall_time& time) {
    switch (_state) {
        case TYPING:
        case DONE: time = _next_update; return true;
        case LOADING: time = wall_time(); return true;
    }
    return false;
}

void LoadingScreen::fire_timer() {
    switch (_state) {
        case TYPING:
            while (_next_update < now()) {
                if (_name_text.done()) {
                    _state      = LOADING;
                    _load_state = start_construct_level(_level);
                    return;
                }
                _next_update += kTypingDelay;
                _name_text.advance();
            }
            if (_next_teletype < now()) {
                sys.sound.teletype();
                while (_next_teletype < now()) {
                    _next_teletype += 3 * kTypingDelay;
                }
            }
            break;

        case LOADING:
            _next_update = now() + kTypingDelay;
            while (now() < _next_update) {
                if (!_load_state.done) {
                    construct_level(&_load_state);
                } else {
                    _state = DONE;
                    return;
                }
            }
            break;

        case DONE: stack()->pop(this); break;
    }
}

void LoadingScreen::overlay() const {
    Rect above_content(0, 0, 640, 480);
    above_content.center_in(world());
    above_content.bottom = widget(kLoadingRect)->inner_bounds().top;
    Rect bounds(0, 0, _name_text.auto_width(), _name_text.height());
    bounds.center_in(above_content);

    _name_text.draw(bounds);
    _name_text.draw_cursor(bounds, kLoadingForeColor);

    const RgbColor& light = GetRGBTranslateColorShade(kLoadingScreenColor, LIGHT);
    const RgbColor& dark  = GetRGBTranslateColorShade(kLoadingScreenColor, DARK);
    Point           off   = offset();
    Rect            bar   = widget(kLoadingRect)->inner_bounds();
    bar.offset(off.h, off.v);
    Rects rects;
    rects.fill(bar, dark);
    bar.right = bar.left + (bar.width() * _load_state.step / _load_state.max);
    rects.fill(bar, light);
}

}  // namespace antares

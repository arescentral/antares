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

#include <sfz/sfz.hpp>

#include "data/string-list.hpp"
#include "drawing/styled-text.hpp"
#include "drawing/text.hpp"
#include "game/scenario-maker.hpp"
#include "game/time.hpp"
#include "video/driver.hpp"

namespace antares {

static const int16_t kLevelNameID = 4600;
static const int16_t kLoadingScreenResID = 6001;
static const uint8_t kLoadingScreenColor = PALE_GREEN;
static const int64_t kTypingDelay = 16667;

LoadingScreen::LoadingScreen(const Scenario* scenario, bool* cancelled):
        InterfaceScreen(kLoadingScreenResID, world, true),
        _state(TYPING),
        _scenario(scenario),
        _cancelled(cancelled),
        _next_update(now_usecs() + kTypingDelay),
        _chars_typed(0),
        _current(0),
        _max(1) {
    StringList strings(kLevelNameID);
    _name_text.reset(new StyledText(title_font));
    _name_text->set_fore_color(GetRGBTranslateColorShade(PALE_GREEN, VERY_LIGHT));
    _name_text->set_retro_text(strings.at(_scenario->levelNameStrNum - 1));
    _name_text->set_tab_width(220);
    _name_text->wrap_to(640, 0, 2);
}

LoadingScreen::~LoadingScreen() {
}

void LoadingScreen::become_front() {
}

bool LoadingScreen::next_timer(int64_t& time) {
    switch (_state) {
      case TYPING:
      case DONE:
        time = _next_update;
        return true;
      case LOADING:
        time = 0;
        return true;
    }
    return false;
}

void LoadingScreen::fire_timer() {
    switch (_state) {
      case TYPING:
        while (_next_update < now_usecs()) {
            if (_chars_typed >= _name_text->size()) {
                _state = LOADING;
                if (!start_construct_scenario(_scenario, &_max)) {
                    *_cancelled = true;
                    stack()->pop(this);
                }
                return;
            }
            if ((_chars_typed % 3) == 0) {
                PlayVolumeSound(kTeletype, kMediumLowVolume, kShortPersistence, kLowPrioritySound);
            }
            _next_update += kTypingDelay;
            ++_chars_typed;
        }
        break;

      case LOADING:
        _next_update = now_usecs() + kTypingDelay;
        while (now_usecs() < _next_update) {
            if (_current < _max) {
                construct_scenario(_scenario, &_current);
            } else {
                _state = DONE;
                return;
            }
        }
        break;

      case DONE:
        stack()->pop(this);
        break;
    }
}

void LoadingScreen::handle_button(int button) {
}

void LoadingScreen::draw() const {
    InterfaceScreen::draw();

    Rect above_content(0, 0, 640, 480);
    above_content.center_in(world);
    above_content.bottom = item(0).bounds.top;
    Rect bounds(0, 0, _name_text->auto_width(), _name_text->height());
    bounds.center_in(above_content);

    for (int32_t i = 0; i < _chars_typed; ++i) {
        _name_text->draw_char(bounds, i);
    }
    if (_chars_typed < _name_text->size()) {
        _name_text->draw_cursor(bounds, _chars_typed);
    }

    const RgbColor& light = GetRGBTranslateColorShade(kLoadingScreenColor, LIGHT);
    const RgbColor& dark = GetRGBTranslateColorShade(kLoadingScreenColor, DARK);
    Rect bar = item(0).bounds;
    VideoDriver::driver()->fill_rect(bar, dark);
    bar.right = bar.left + (bar.width() * _current / _max);
    VideoDriver::driver()->fill_rect(bar, light);
}

}  // namespace antares

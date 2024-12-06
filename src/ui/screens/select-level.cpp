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

#include "ui/screens/select-level.hpp"

#include <algorithm>
#include <pn/output>

#include "config/keys.hpp"
#include "config/ledger.hpp"
#include "config/preferences.hpp"
#include "data/plugin.hpp"
#include "drawing/color.hpp"
#include "drawing/styled-text.hpp"
#include "drawing/text.hpp"
#include "game/globals.hpp"
#include "game/level.hpp"
#include "game/main.hpp"
#include "game/sys.hpp"
#include "sound/driver.hpp"
#include "ui/card.hpp"
#include "ui/interface-handling.hpp"
#include "video/driver.hpp"
#include "video/transitions.hpp"

namespace antares {

static constexpr char kBeginButton[]    = "begin";
static constexpr char kCancelButton[]   = "cancel";
static constexpr char kPreviousButton[] = "previous";
static constexpr char kNextButton[]     = "next";
static constexpr char kNameRect[]       = "rect";

SelectLevelScreen::SelectLevelScreen(bool* cancelled, const Level** level)
        : InterfaceScreen("select-level", {0, 0, 640, 480}),
          _state(SELECTING),
          _cancelled(cancelled),
          _level(level) {
    update_chapters(nullptr);

    button(kBeginButton)->bind({[this] {
        _state      = FADING_OUT;
        *_cancelled = false;
        stack()->push(new ColorFade(ColorFade::TO_COLOR, RgbColor::black(), secs(1), false, NULL));
    }});

    button(kCancelButton)->bind({[this] {
        *_cancelled = true;
        stack()->pop(this);
    }});

    button(kPreviousButton)
            ->bind({
                    [this] {
                        if (_index > 0) {
                            --_index;
                            *_level = Level::get(_chapters[_index]);
                        }
                    },
                    [this] { return _index > 0; },
            });

    button(kNextButton)
            ->bind({
                    [this] {
                        if (_index < _chapters.size() - 1) {
                            ++_index;
                            *_level = Level::get(_chapters[_index]);
                        }
                    },
                    [this] { return _index < _chapters.size() - 1; },
            });
}

SelectLevelScreen::~SelectLevelScreen() {}

void SelectLevelScreen::become_front() {
    switch (_state) {
        case SELECTING:
        case UNLOCKING: InterfaceScreen::become_front(); break;

        case FADING_OUT: stack()->pop(this); break;
    }
}

static int ndigits(size_t n) {
    if (n == 0) {
        return 1;
    }
    int result = 1;
    while (n >= 10) {
        ++result;
        n /= 10;
    }
    return result;
}

void SelectLevelScreen::key_down(const KeyDownEvent& event) {
    switch (_state) {
        case SELECTING:
            switch (event.key().value()) {
                case Key::K8:
                case Key::N_TIMES:
                    _state          = UNLOCKING;
                    _unlock_chapter = 0;
                    _unlock_digits  = ndigits(plug.levels.size());
                    sys.sound.cloak_on();
                    return;
                default: break;
            }
            break;

        case UNLOCKING: {
            int digit = event.key().digit();
            if (digit < 0) {
                _state = SELECTING;
                break;
            }
            _unlock_chapter = (_unlock_chapter * 10) + digit;
            if (--_unlock_digits == 0) {
                _state = SELECTING;
                if (plug.chapters.find(_unlock_chapter) == plug.chapters.end()) {
                    return;
                }
                sys.sound.cloak_off();
                sys.ledger->unlock_chapter(_unlock_chapter);
                update_chapters(&_unlock_chapter);
            }
            return;
        } break;

        case FADING_OUT: return;
    }
    InterfaceScreen::key_down(event);
}

void SelectLevelScreen::overlay() const { draw_level_name(); }

void SelectLevelScreen::update_chapters(size_t* select) {
    sys.ledger->unlocked_chapters(&_chapters);
    _chapters.erase(
            std::remove_if(
                    _chapters.begin(), _chapters.end(),
                    [](int i) { return Level::get(i) == nullptr; }),
            _chapters.end());

    if (select) {
        _index =
                std::find(_chapters.begin(), _chapters.end(), _unlock_chapter) - _chapters.begin();
        *_level = Level::get(_chapters[_index]);
    } else {
        _index = _chapters.size() - 1;
    }
    *_level = Level::get(_chapters[_index]);
}

void SelectLevelScreen::draw_level_name() const {
    const pn::string_view chapter_name = (*_level)->base.name;

    const Widget& i = *widget(kNameRect);

    const StyledText retro = StyledText::retro(
            chapter_name, {sys.fonts.title, 440, 0, 2},
            GetRGBTranslateColorShade(Hue::AQUA, LIGHTEST));

    Rect  bounds = i.inner_bounds();
    Point off    = offset();
    bounds.offset(off.h, off.v);
    retro.draw(bounds);
}

}  // namespace antares

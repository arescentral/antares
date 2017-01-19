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

#include <sfz/sfz.hpp>

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

using sfz::BytesSlice;
using sfz::Exception;
using sfz::String;
using sfz::StringSlice;
using sfz::format;
using std::unique_ptr;

namespace antares {

SelectLevelScreen::SelectLevelScreen(bool* cancelled, Handle<Level>* level)
        : InterfaceScreen("select-level", {0, 0, 640, 480}, true),
          _state(SELECTING),
          _cancelled(cancelled),
          _level(level) {
    Ledger::ledger()->unlocked_chapters(&_chapters);
    _index  = _chapters.size() - 1;
    *_level = Handle<Level>(_chapters[_index] - 1);
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
            switch (event.key()) {
                case Keys::K8:
                case Keys::N_TIMES:
                    _state          = UNLOCKING;
                    _unlock_chapter = 0;
                    _unlock_digits  = ndigits(plug.levels.size());
                    sys.sound.cloak_on();
                    return;
            }
        case UNLOCKING: {
            int digit = key_digit(event.key());
            if (digit < 0) {
                _state = SELECTING;
                break;
            }
            _unlock_chapter = (_unlock_chapter * 10) + digit;
            if (--_unlock_digits == 0) {
                _state = SELECTING;
                if (_unlock_chapter > plug.levels.size()) {
                    return;
                }
                sys.sound.cloak_off();
                Ledger::ledger()->unlock_chapter(_unlock_chapter);
                Ledger::ledger()->unlocked_chapters(&_chapters);
                adjust_interface();
            }
            return;
        } break;
        case FADING_OUT: return;
    }
    InterfaceScreen::key_down(event);
}

void SelectLevelScreen::adjust_interface() {
    if (_index > 0) {
        dynamic_cast<Button&>(mutable_item(PREVIOUS)).status = kActive;
    } else {
        dynamic_cast<Button&>(mutable_item(PREVIOUS)).status = kDimmed;
    }
    if (_index < _chapters.size() - 1) {
        dynamic_cast<Button&>(mutable_item(NEXT)).status = kActive;
    } else {
        dynamic_cast<Button&>(mutable_item(NEXT)).status = kDimmed;
    }
}

void SelectLevelScreen::handle_button(Button& button) {
    switch (button.id) {
        case OK:
            _state      = FADING_OUT;
            *_cancelled = false;
            stack()->push(
                    new ColorFade(ColorFade::TO_COLOR, RgbColor::black(), secs(1), false, NULL));
            break;

        case CANCEL:
            *_cancelled = true;
            stack()->pop(this);
            break;

        case PREVIOUS:
            if (_index > 0) {
                --_index;
                *_level = Handle<Level>(_chapters[_index] - 1);
            }
            adjust_interface();
            break;

        case NEXT:
            if (_index < _chapters.size() - 1) {
                ++_index;
                *_level = Handle<Level>(_chapters[_index] - 1);
            }
            adjust_interface();
            break;

        default: throw Exception(format("Got unknown button {0}.", button.id));
    }
}

void SelectLevelScreen::overlay() const {
    draw_level_name();
}

void SelectLevelScreen::draw_level_name() const {
    const String chapter_name((*_level)->name);

    const InterfaceItem& i = item(NAME);

    RgbColor   color = GetRGBTranslateColorShade(AQUA, VERY_LIGHT);
    StyledText retro(sys.fonts.title);
    retro.set_fore_color(color);
    retro.set_retro_text(chapter_name);
    retro.wrap_to(440, 0, 2);

    Rect  bounds = i.bounds();
    Point off    = offset();
    bounds.offset(off.h, off.v);
    retro.draw(bounds);
}

}  // namespace antares

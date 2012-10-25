// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// Copyright (C) 2008-2012 The Antares Authors
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

#include "config/ledger.hpp"
#include "config/preferences.hpp"
#include "drawing/color.hpp"
#include "drawing/styled-text.hpp"
#include "drawing/text.hpp"
#include "game/globals.hpp"
#include "game/main.hpp"
#include "game/scenario-maker.hpp"
#include "ui/card.hpp"
#include "ui/interface-handling.hpp"
#include "video/driver.hpp"
#include "video/transitions.hpp"

using sfz::BytesSlice;
using sfz::Exception;
using sfz::String;
using sfz::StringSlice;
using sfz::scoped_ptr;
using sfz::format;

namespace macroman = sfz::macroman;

namespace antares {
namespace {

const int kSelectLevelScreenResID = 5011;

}  // namespace

SelectLevelScreen::SelectLevelScreen(bool* cancelled, const Scenario** scenario)
        : InterfaceScreen(kSelectLevelScreenResID, world, true),
          _state(SELECTING),
          _cancelled(cancelled),
          _scenario(scenario) {
    Ledger::ledger()->unlocked_chapters(&_chapters);
    _index = _chapters.size() - 1;
    *_scenario = GetScenarioPtrFromChapter(_chapters[_index]);
}

SelectLevelScreen::~SelectLevelScreen() { }

void SelectLevelScreen::become_front() {
    switch (_state) {
      case SELECTING:
        InterfaceScreen::become_front();
        break;

      case FADING_OUT:
        stack()->pop(this);
        break;
    }
}

void SelectLevelScreen::adjust_interface() {
    if (_index > 0) {
        mutable_item(PREVIOUS)->set_status(kActive);
    } else {
        mutable_item(PREVIOUS)->set_status(kDimmed);
    }
    if (_index < _chapters.size() - 1) {
        mutable_item(NEXT)->set_status(kActive);
    } else {
        mutable_item(NEXT)->set_status(kDimmed);
    }
}

void SelectLevelScreen::handle_button(int button) {
    switch (button) {
      case OK:
        _state = FADING_OUT;
        *_cancelled = false;
        stack()->push(new ColorFade(ColorFade::TO_COLOR, RgbColor::kBlack, 1e6, false, NULL));
        break;

      case CANCEL:
        *_cancelled = true;
        stack()->pop(this);
        break;

      case PREVIOUS:
        if (_index > 0) {
            --_index;
            *_scenario = GetScenarioPtrFromChapter(_chapters[_index]);
        }
        adjust_interface();
        break;

      case NEXT:
        if (_index < _chapters.size() - 1) {
            ++_index;
            *_scenario = GetScenarioPtrFromChapter(_chapters[_index]);
        }
        adjust_interface();
        break;

      default:
        throw Exception(format("Got unknown button {0}.", button));
    }
}

void SelectLevelScreen::draw() const {
    InterfaceScreen::draw();
    draw_level_name();
}

void SelectLevelScreen::draw_level_name() const {
    const String chapter_name((*_scenario)->name());

    const interfaceItemType& i = item(NAME);

    RgbColor color = GetRGBTranslateColorShade(AQUA, VERY_LIGHT);
    StyledText retro(title_font);
    retro.set_fore_color(color);
    retro.set_retro_text(chapter_name);
    retro.wrap_to(440, 0, 2);

    retro.draw(i.bounds);
}

}  // namespace antares

// Ares, a tactical space combat game.
// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// 
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

#include "SelectLevelScreen.hpp"

#include "rezin/MacRoman.hpp"
#include "AresMain.hpp"
#include "CardStack.hpp"
#include "ColorTranslation.hpp"
#include "DirectText.hpp"
#include "Error.hpp"
#include "Ledger.hpp"
#include "OffscreenGWorld.hpp"
#include "Preferences.hpp"
#include "PlayerInterface.hpp"
#include "RetroText.hpp"
#include "ScenarioMaker.hpp"
#include "VideoDriver.hpp"

using rezin::mac_roman_encoding;
using sfz::BytesPiece;
using sfz::String;

namespace antares {

extern PixMap* gRealWorld;

namespace {

const int kSelectLevelScreenResID = 5011;

}  // namespace

SelectLevelScreen::SelectLevelScreen(bool* cancelled, int* scenario)
        : InterfaceScreen(kSelectLevelScreenResID, gRealWorld->bounds(), true),
          _cancelled(cancelled),
          _scenario(scenario) {
    Ledger::ledger()->unlocked_chapters(&_chapters);
    _index = _chapters.size() - 1;
    *_scenario = GetScenarioNumberFromChapterNumber(_chapters[_index]);
}

SelectLevelScreen::~SelectLevelScreen() { }

void SelectLevelScreen::become_front() {
    InterfaceScreen::become_front();
    VideoDriver::driver()->set_game_state(SELECT_LEVEL_INTERFACE);
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
      case CANCEL:
        *_cancelled = (button == CANCEL);
        VideoDriver::driver()->set_game_state(UNKNOWN);
        stack()->pop(this);
        break;

      case PREVIOUS:
        if (_index > 0) {
            --_index;
            *_scenario = GetScenarioNumberFromChapterNumber(_chapters[_index]);
        }
        adjust_interface();
        draw();
        break;

      case NEXT:
        if (_index < _chapters.size() - 1) {
            ++_index;
            *_scenario = GetScenarioNumberFromChapterNumber(_chapters[_index]);
        }
        adjust_interface();
        draw();
        break;

      default:
        fail("Got unknown button %d.", button);
    }
}

void SelectLevelScreen::draw() const {
    InterfaceScreen::draw();
    unsigned char chapter_name[256];
    GetScenarioName(*_scenario, chapter_name);
    draw_level_name(chapter_name, kTitleFontNum, NAME);
}

void SelectLevelScreen::draw_level_name(unsigned char* name, long fontNum, long itemNum) const {
    const interfaceItemType& i = item(itemNum);
    RgbColor color;
    GetRGBTranslateColorShade(&color, AQUA, VERY_LIGHT);
    String text(
            BytesPiece(reinterpret_cast<const uint8_t*>(name + 1), name[0]),
            mac_roman_encoding());
    RetroText retro(text, fontNum, color, RgbColor::kBlack);
    retro.wrap_to(440, 2);

    pix()->view(i.bounds).fill(RgbColor::kBlack);
    retro.draw(pix(), i.bounds);
    gRealWorld->copy(*pix());
}

}  // namespace antares

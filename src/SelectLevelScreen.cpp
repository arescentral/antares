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

#include "AresMain.hpp"
#include "AresPreferences.hpp"
#include "CardStack.hpp"
#include "ColorTranslation.hpp"
#include "DirectText.hpp"
#include "Error.hpp"
#include "OffscreenGWorld.hpp"
#include "PlayerInterface.hpp"
#include "RetroText.hpp"
#include "ScenarioMaker.hpp"
#include "VideoDriver.hpp"

namespace antares {

extern PixMap* gActiveWorld;
extern PixMap* gOffWorld;

namespace {

const int kSelectLevelScreenResID = 5011;

}  // namespace

SelectLevelScreen::SelectLevelScreen(bool* cancelled, int* scenario)
        : InterfaceScreen(kSelectLevelScreenResID),
          _cancelled(cancelled),
          _chapter(GetStartingLevelPreference()),
          _scenario(scenario) {
    *_scenario = GetScenarioNumberFromChapterNumber(_chapter);
}

SelectLevelScreen::~SelectLevelScreen() { }

void SelectLevelScreen::become_front() {
    InterfaceScreen::become_front();
    VideoDriver::driver()->set_game_state(SELECT_LEVEL_INTERFACE);
}

void SelectLevelScreen::adjust_interface() {
    if (_chapter > 1) {
        mutable_item(PREVIOUS)->set_status(kActive);
    } else {
        mutable_item(PREVIOUS)->set_status(kDimmed);
    }
    if (_chapter < GetStartingLevelPreference()) {
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
        if (_chapter > 1) {
            --_chapter;
            *_scenario = GetScenarioNumberFromChapterNumber(_chapter);
        }
        adjust_interface();
        draw();
        break;

      case NEXT:
        if (_chapter < GetStartingLevelPreference()) {
            ++_chapter;
            *_scenario = GetScenarioNumberFromChapterNumber(_chapter);
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
    RetroText retro(
            reinterpret_cast<char*>(name + 1), name[0], fontNum, color, RgbColor::kBlack);
    retro.wrap_to(440, 2);

    DrawInOffWorld();
    gActiveWorld->view(i.bounds).fill(RgbColor::kBlack);
    retro.draw(gActiveWorld, i.bounds);
    DrawInRealWorld();
    CopyOffWorldToRealWorld(i.bounds);
}

}  // namespace antares

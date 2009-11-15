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

#include "LoadingScreen.hpp"

#include "AresMain.hpp"
#include "AresPreferences.hpp"
#include "CardStack.hpp"
#include "ColorTranslation.hpp"
#include "DirectText.hpp"
#include "OffscreenGWorld.hpp"
#include "PlayerInterface.hpp"
#include "RetroText.hpp"
#include "ScenarioMaker.hpp"
#include "StringList.hpp"

namespace antares {

extern PixMap* gRealWorld;
extern PixMap* gActiveWorld;
extern PixMap* gOffWorld;
extern long WORLD_WIDTH;

namespace {

const int kLoadingScreenResID = 6001;
const int kLevelNameID = 4600;

std::string chapter_name(scenarioType* scenario) {
    StringList strings;
    strings.load(kLevelNameID);
    return strings.at(scenario->levelNameStrNum);
}

}  // namespace

LoadingScreen::LoadingScreen(scenarioType* scenario)
        : InterfaceScreen(kLoadingScreenResID, gRealWorld->bounds(), true),
          _chapter_name(chapter_name(scenario)),
          _loading_progress(0.0) {
}

LoadingScreen::~LoadingScreen() { }

double LoadingScreen::next_timer() {
    return 0.0;
}

void LoadingScreen::fire_timer() {
    _loading_progress += 1.0 / 60.0;
    draw();
    if (_loading_progress >= 1.0) {
        stack()->pop(this);
    }
}

void LoadingScreen::handle_button(int button) {
    static_cast<void>(button);
}

void LoadingScreen::draw() const {
    InterfaceScreen::draw();

    const interfaceItemType& i = item(0);
    DrawInOffWorld();

    RgbColor color;
    GetRGBTranslateColorShade(&color, PALE_GREEN, LIGHT);

    // TODO(sfiera): type this out one character at a time.
    RetroText retro(
            _chapter_name.c_str(), _chapter_name.size(), kTitleFontNum, color, RgbColor::kBlack);
    retro.wrap_to(640, 2);
    Rect bounds(0, 0, retro.auto_width(), retro.height());
    bounds.offset(WORLD_WIDTH / 2 - bounds.width() / 2, i.bounds.top / 2 - bounds.height() / 2);
    retro.draw(gActiveWorld, bounds);
    CopyOffWorldToRealWorld(bounds);

    Rect r(i.bounds);
    r.right = std::min<int>(r.right, r.left + _loading_progress * r.width());

    GetRGBTranslateColorShade(&color, PALE_GREEN, LIGHT);
    gActiveWorld->view(r).fill(color);

    r.left = r.right;
    r.right = i.bounds.right;
    GetRGBTranslateColorShade(&color, PALE_GREEN, DARK);
    gActiveWorld->view(r).fill(color);
    CopyOffWorldToRealWorld(i.bounds);
}

}  // namespace antares

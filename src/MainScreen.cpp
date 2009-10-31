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

#include "MainScreen.hpp"

#include "AresGlobalType.hpp"
#include "AresMain.hpp"
#include "CardStack.hpp"
#include "DirectText.hpp"
#include "FakeDrawing.hpp"
#include "Fakes.hpp"
#include "InterfaceHandling.hpp"
#include "Options.hpp"
#include "PlayerInterface.hpp"
#include "Randomize.hpp"
#include "ReplayGame.hpp"
#include "ScrollTextScreen.hpp"
#include "SoloGame.hpp"
#include "Time.hpp"
#include "VideoDriver.hpp"

namespace antares {

namespace {

const int kMainScreenResID = 5000;
const double kMainDemoTimeOutTime = 30.0;
const int kTitleTextScrollWidth = 450;

}  // namespace

MainScreen::MainScreen()
        : InterfaceScreen(kMainScreenResID) { }

MainScreen::~MainScreen() { }

void MainScreen::become_front() {
    _next_card.reset();
    InterfaceScreen::become_front();
    VideoDriver::driver()->set_game_state(MAIN_SCREEN_INTERFACE);
}

double MainScreen::delay() {
    return _last_event + kMainDemoTimeOutTime;
}

void MainScreen::fire_timer() {
    if (Randomize(4) == 2) {
        DoScrollText(5600, 4, kTitleTextScrollWidth, kTitleFontNum, -1);
    }
    _next_card.reset(new ReplayGame(GetDemoScenario()));
    stack()->push(_next_card.get());
}

void MainScreen::adjust_interface() {
    if (!(globals()->gOptions & kOptionNetworkAvailable)) {
        mutable_item(START_NETWORK_GAME)->set_status(kDimmed);
    }
    if (globals()->gOptions & kOptionNoSinglePlayer) {
        mutable_item(START_NEW_GAME)->set_status(kDimmed);
    }
}

void MainScreen::handle_button(int button) {
    switch (button) {
      case QUIT:
        // 1-second fade-out.
        stack()->pop(this);
        break;

      case DEMO:
        _next_card.reset(new ReplayGame(GetDemoScenario()));
        stack()->push(_next_card.get());
        break;

      case REPLAY_INTRO:
        _next_card.reset(new ScrollTextScreen(5600, kTitleTextScrollWidth, 15.0));
        stack()->push(_next_card.get());
        break;

      case START_NEW_GAME:
        globals()->gOptions &= ~kOptionReplay;
        _next_card.reset(new SoloGame);
        stack()->push(_next_card.get());
        break;

      case START_NETWORK_GAME:
        fprintf(stderr, "Networked games not yet implemented.\n");
        exit(1);
        break;

      case ABOUT_ARES:
        _next_card.reset(new ScrollTextScreen(6500, 540, 30.0));
        stack()->push(_next_card.get());
        break;

      case OPTIONS:
        DoOptionsInterface();
        become_front();
        break;
    }
}

}  // namespace antares

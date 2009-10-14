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
#include "DirectText.hpp"
#include "FakeDrawing.hpp"
#include "InterfaceHandling.hpp"
#include "Options.hpp"
#include "PlayerInterface.hpp"
#include "Randomize.hpp"
#include "ScrollTextScreen.hpp"

namespace antares {

namespace {

const int kMainScreenResID = 5000;
const double kMainDemoTimeOutTime = 30.0;
const int kTitleTextScrollWidth = 450;

double now() {
    uint64_t usecs;
    Microseconds(&usecs);
    return usecs / 1000000.0;
}

}  // namespace

MainScreen::MainScreen()
        : InterfaceScreen(kMainScreenResID) { }

MainScreen::~MainScreen() { }

void MainScreen::become_front() {
    _scroll_text.reset();
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
    StartReplay();
    become_front();
}

void MainScreen::adjust_interface() {
    if (!(globals()->gOptions & kOptionNetworkAvailable)) {
        SetStatusOfAnyInterfaceItem(START_NETWORK_GAME, kDimmed, false);
    }
    if (globals()->gOptions & kOptionNoSinglePlayer) {
        SetStatusOfAnyInterfaceItem(START_NEW_GAME, kDimmed, false);
    }
}

void MainScreen::handle_button(int button) {
    switch (button) {
      case QUIT:
        // 1-second fade-out.
        VideoDriver::driver()->pop_listener(this);
        break;

      case DEMO:
        StartReplay();
        become_front();
        break;

      case REPLAY_INTRO:
        _scroll_text.reset(new ScrollTextScreen(5600, kTitleTextScrollWidth, 15.0));
        VideoDriver::driver()->push_listener(_scroll_text.get());
        break;

      case START_NEW_GAME:
        StartNewGame();
        become_front();
        break;

      case START_NETWORK_GAME:
        fprintf(stderr, "Networked games not yet implemented.\n");
        exit(1);
        break;

      case ABOUT_ARES:
        _scroll_text.reset(new ScrollTextScreen(6500, 540, 30.0));
        VideoDriver::driver()->push_listener(_scroll_text.get());
        break;

      case OPTIONS:
        DoOptionsInterface();
        become_front();
        break;
    }
}

}  // namespace antares

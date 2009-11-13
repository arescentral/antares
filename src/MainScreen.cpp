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
#include "Error.hpp"
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

extern PixMap* gRealWorld;

namespace {

const int kMainScreenResID = 5000;
const double kMainDemoTimeOutTime = 30.0;
const int kTitleTextScrollWidth = 450;

class MainScreenTimeOut : public Card {
  public:
    MainScreenTimeOut()
            : _state(NEW) { }

    virtual void become_front() {
        switch (_state) {
          case NEW:
            if (Randomize(4) == 2) {
                _state = REPLAY_INTRO;
                stack()->push(new ScrollTextScreen(5600, kTitleTextScrollWidth, 15.0));
                break;
            }
            // else fall through

          case REPLAY_INTRO:
            _state = DEMO;
            stack()->push(new ReplayGame(GetDemoScenario()));
            break;

          case DEMO:
            stack()->pop(this);
            break;
        }
    }

  private:
    enum State {
        NEW,
        REPLAY_INTRO,
        DEMO,
    };
    State _state;
};

}  // namespace

MainScreen::MainScreen()
        : InterfaceScreen(kMainScreenResID, gRealWorld->bounds(), true) { }

MainScreen::~MainScreen() { }

void MainScreen::become_front() {
    InterfaceScreen::become_front();
    VideoDriver::driver()->set_game_state(MAIN_SCREEN_INTERFACE);
}

double MainScreen::delay() {
    if (stack()->top() == this) {
        return std::max(last_event() + kMainDemoTimeOutTime - now_secs(), 0.001);
    } else {
        return 0.0;
    }
}

void MainScreen::fire_timer() {
    stack()->push(new MainScreenTimeOut);
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
        stack()->push(new ReplayGame(GetDemoScenario()));
        break;

      case REPLAY_INTRO:
        stack()->push(new ScrollTextScreen(5600, kTitleTextScrollWidth, 15.0));
        break;

      case START_NEW_GAME:
        globals()->gOptions &= ~kOptionReplay;
        stack()->push(new SoloGame);
        break;

      case START_NETWORK_GAME:
        fail("Networked games not yet implemented.");
        break;

      case ABOUT_ARES:
        stack()->push(new ScrollTextScreen(6500, 540, 30.0));
        break;

      case OPTIONS:
        DoOptionsInterface();
        become_front();
        break;
    }
}

}  // namespace antares

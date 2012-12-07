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

#include "ui/screens/main.hpp"

#include "config/preferences.hpp"
#include "data/replay-list.hpp"
#include "drawing/text.hpp"
#include "game/globals.hpp"
#include "game/main.hpp"
#include "game/scenario-maker.hpp"
#include "game/time.hpp"
#include "math/random.hpp"
#include "sound/music.hpp"
#include "ui/card.hpp"
#include "ui/flows/replay-game.hpp"
#include "ui/flows/solo-game.hpp"
#include "ui/interface-handling.hpp"
#include "ui/screens/options.hpp"
#include "ui/screens/scroll-text.hpp"
#include "video/driver.hpp"

using sfz::Exception;

namespace antares {

namespace {

const int kMainScreenResID = 5000;
const int64_t kMainDemoTimeOutTime = 30e6;
const int kTitleTextScrollWidth = 450;

}  // namespace

MainScreen::MainScreen()
        : InterfaceScreen(kMainScreenResID, world, true) { }

MainScreen::~MainScreen() { }

void MainScreen::become_front() {
    InterfaceScreen::become_front();
    if (Preferences::preferences()->play_idle_music() && !SongIsPlaying()) {
        LoadSong(kTitleSongID);
        SetSongVolume(kMaxMusicVolume);
        PlaySong();
    }
}

bool MainScreen::next_timer(int64_t& time) {
    time = last_event() + kMainDemoTimeOutTime;
    return true;
}

void MainScreen::fire_timer() {
    Randomize(1);
    ReplayList replays;
    size_t demo = rand() % (replays.size() + 1);
    if (demo == replays.size()) {
        stack()->push(new ScrollTextScreen(5600, kTitleTextScrollWidth, 15.0));
    } else {
        stack()->push(new ReplayGame(replays.at(demo)));
    }
}

void MainScreen::adjust_interface() {
    // TODO(sfiera): switch on whether or not network games are available.
    mutable_item(START_NETWORK_GAME)->set_status(kDimmed);

    // TODO(sfiera): switch on whether or not there is a single-player campaign.
    mutable_item(START_NEW_GAME)->set_status(kActive);

    ReplayList replays;
    if (replays.size() == 0) {
        mutable_item(DEMO)->set_status(kDimmed);
    }
}

void MainScreen::handle_button(int button) {
    switch (button) {
      case QUIT:
        // 1-second fade-out.
        stack()->pop(this);
        break;

      case DEMO:
        {
            ReplayList replays;
            stack()->push(new ReplayGame(replays.at(rand() % replays.size())));
        }
        break;

      case REPLAY_INTRO:
        stack()->push(new ScrollTextScreen(5600, kTitleTextScrollWidth, 15.0));
        break;

      case START_NEW_GAME:
        stack()->push(new SoloGame);
        break;

      case START_NETWORK_GAME:
        throw Exception("Networked games not yet implemented.");
        break;

      case ABOUT_ARES:
        stack()->push(new ScrollTextScreen(6500, 540, 30.0));
        break;

      case OPTIONS:
        stack()->push(new OptionsScreen);
        break;
    }
}

}  // namespace antares

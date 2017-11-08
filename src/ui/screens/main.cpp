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

#include "ui/screens/main.hpp"

#include "config/preferences.hpp"
#include "drawing/text.hpp"
#include "game/globals.hpp"
#include "game/level.hpp"
#include "game/main.hpp"
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

const secs kMainDemoTimeOutTime  = secs(30);
const int  kTitleTextScrollWidth = 450;

}  // namespace

MainScreen::MainScreen() : InterfaceScreen("main", {0, 0, 640, 480}, true), _state(NORMAL) {}

MainScreen::~MainScreen() {}

void MainScreen::become_front() {
    switch (_state) {
        case NORMAL:
            InterfaceScreen::become_front();
            sys.music.play(Music::IDLE, kTitleSongID);
            _next_timer = (now() + kMainDemoTimeOutTime);
            break;
        case QUITTING: stack()->pop(this); break;
    }
}

bool MainScreen::next_timer(wall_time& time) {
    time = _next_timer;
    return true;
}

void MainScreen::fire_timer() {
    Randomize(1);
    size_t demo = rand() % (_replays.size() + 1);
    if (demo == _replays.size()) {
        stack()->push(new ScrollTextScreen(5600, kTitleTextScrollWidth, kSlowScrollInterval));
    } else {
        stack()->push(new ReplayGame(_replays.at(demo)));
    }
}

void MainScreen::mouse_down(const MouseDownEvent& event) {
    InterfaceScreen::mouse_down(event);
    _next_timer = (now() + kMainDemoTimeOutTime);
}

void MainScreen::mouse_up(const MouseUpEvent& event) {
    InterfaceScreen::mouse_up(event);
    _next_timer = (now() + kMainDemoTimeOutTime);
}

void MainScreen::key_down(const KeyDownEvent& event) {
    InterfaceScreen::key_down(event);
    _next_timer = (now() + kMainDemoTimeOutTime);
}

void MainScreen::key_up(const KeyUpEvent& event) {
    InterfaceScreen::key_up(event);
    _next_timer = (now() + kMainDemoTimeOutTime);
}

void MainScreen::gamepad_button_down(const GamepadButtonDownEvent& event) {
    InterfaceScreen::gamepad_button_down(event);
    _next_timer = (now() + kMainDemoTimeOutTime);
}

void MainScreen::gamepad_button_up(const GamepadButtonUpEvent& event) {
    InterfaceScreen::gamepad_button_up(event);
    _next_timer = (now() + kMainDemoTimeOutTime);
}

void MainScreen::adjust_interface() {
    // TODO(sfiera): switch on whether or not network games are available.
    dynamic_cast<PlainButton&>(mutable_item(START_NETWORK_GAME)).status = kDimmed;

    // TODO(sfiera): switch on whether or not there is a single-player campaign.
    dynamic_cast<PlainButton&>(mutable_item(START_NEW_GAME)).status = kActive;

    if (_replays.size() == 0) {
        dynamic_cast<PlainButton&>(mutable_item(DEMO)).status = kDimmed;
    }
}

void MainScreen::handle_button(antares::Button& button) {
    switch (button.id) {
        case QUIT:
            // 1-second fade-out.
            _state = QUITTING;
            stack()->push(
                    new ColorFade(ColorFade::TO_COLOR, RgbColor::black(), secs(1), false, NULL));
            break;

        case DEMO: stack()->push(new ReplayGame(_replays.at(rand() % _replays.size()))); break;

        case REPLAY_INTRO:
            stack()->push(new ScrollTextScreen(5600, kTitleTextScrollWidth, kSlowScrollInterval));
            break;

        case START_NEW_GAME: stack()->push(new SoloGame); break;

        case START_NETWORK_GAME: throw Exception("Networked games not yet implemented."); break;

        case ABOUT_ARES:
            stack()->push(new ScrollTextScreen(6500, 540, kFastScrollInterval));
            break;

        case OPTIONS: stack()->push(new OptionsScreen); break;
    }
}

}  // namespace antares

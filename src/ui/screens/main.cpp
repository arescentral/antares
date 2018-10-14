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
#include "data/plugin.hpp"
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

namespace antares {

namespace {

const secs kMainDemoTimeOutTime  = secs(30);
const int  kTitleTextScrollWidth = 450;

}  // namespace

MainScreen::MainScreen()
        : InterfaceScreen("main", {0, 0, 640, 480}),
          _state(NORMAL),
          _replays(Resource::list_replays()) {
    button(START_NEW_GAME)
            ->bind({
                    [this] { stack()->push(new SoloGame); },
                    [] { return plug.chapters.find(1) != plug.chapters.end(); },
            });

    button(START_NETWORK_GAME)
            ->bind({
                    [] { throw std::runtime_error("Networked games not yet implemented."); },
                    [] { return false; },
            });

    button(REPLAY_INTRO)
            ->bind({
                    [this] {
                        stack()->push(new ScrollTextScreen(
                                *plug.info.intro, kTitleTextScrollWidth, kSlowScrollInterval));
                    },
                    [] { return plug.info.intro.has_value(); },
            });

    button(DEMO)->bind({
            [this] { stack()->push(new ReplayGame(_replays.at(rand() % _replays.size()))); },
            [this] { return !_replays.empty(); },
    });

    button(ABOUT_ARES)
            ->bind({
                    [this] {
                        stack()->push(
                                new ScrollTextScreen(*plug.info.about, 540, kFastScrollInterval));
                    },
                    [] { return plug.info.about.has_value(); },
            });

    button(OPTIONS)->bind({
            [this] { stack()->push(new OptionsScreen); },
    });

    button(QUIT)->bind({
            [this] {
                // 1-second fade-out.
                _state = QUITTING;
                stack()->push(new ColorFade(
                        ColorFade::TO_COLOR, RgbColor::black(), secs(1), false, NULL));
            },
    });
}

MainScreen::~MainScreen() {}

void MainScreen::become_front() {
    switch (_state) {
        case NORMAL:
            InterfaceScreen::become_front();
            sys.music.play(Music::IDLE, Music::title_song);
            _next_timer = (now() + kMainDemoTimeOutTime);
            break;
        case QUITTING: stack()->pop(this); break;
    }
}

bool MainScreen::next_timer(wall_time& time) {
    if (_replays.size() || plug.info.intro.has_value()) {
        time = _next_timer;
        return true;
    }
    return false;
}

void MainScreen::fire_timer() {
    Randomize(1);
    int option_count = _replays.size() + (plug.info.intro.has_value() ? 1 : 0);
    if (option_count == 0) {
        return;
    }
    int option = rand() % option_count;
    if (option == _replays.size()) {
        stack()->push(new ScrollTextScreen(
                *plug.info.intro, kTitleTextScrollWidth, kSlowScrollInterval));
    } else {
        stack()->push(new ReplayGame(_replays[option]));
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

}  // namespace antares

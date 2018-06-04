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

#ifndef ANTARES_UI_SCREENS_MAIN_HPP_
#define ANTARES_UI_SCREENS_MAIN_HPP_

#include <pn/string>
#include <vector>

#include "math/units.hpp"
#include "ui/screen.hpp"

namespace antares {

class MainScreen : public InterfaceScreen {
  public:
    MainScreen();
    ~MainScreen();

    virtual void become_front();

    virtual bool next_timer(wall_time& time);
    virtual void fire_timer();

    virtual void mouse_down(const MouseDownEvent& event);
    virtual void mouse_up(const MouseUpEvent& event);
    virtual void key_down(const KeyDownEvent& event);
    virtual void key_up(const KeyUpEvent& event);
    virtual void gamepad_button_down(const GamepadButtonDownEvent& event);
    virtual void gamepad_button_up(const GamepadButtonUpEvent& event);

  private:
    enum Button {
        START_NEW_GAME     = 0,
        START_NETWORK_GAME = 1,
        OPTIONS            = 2,
        QUIT               = 3,
        ABOUT_ARES         = 4,
        DEMO               = 5,
        REPLAY_INTRO       = 6,
    };
    enum State {
        NORMAL   = 0,
        QUITTING = 1,
    };
    State                   _state;
    std::vector<pn::string> _replays;
    wall_time               _next_timer;
};

}  // namespace antares

#endif  // ANTARES_UI_SCREENS_MAIN_HPP_

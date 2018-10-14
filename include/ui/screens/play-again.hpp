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

#ifndef ANTARES_UI_SCREENS_PLAY_AGAIN_HPP_
#define ANTARES_UI_SCREENS_PLAY_AGAIN_HPP_

#include "ui/screen.hpp"

namespace antares {

class PlayAgainScreen : public InterfaceScreen {
  public:
    enum Item {
        RESTART = 0,
        QUIT    = 1,
        RESUME  = 4,
        SKIP    = 5,
    };

    PlayAgainScreen(bool allow_resume, bool allow_skip, Item* button_pressed);
    ~PlayAgainScreen();

    virtual void become_front();

  private:
    enum State {
        ASKING,
        FADING_OUT,
    };
    State _state;

    Item* _button_pressed;
};

const char* stringify(PlayAgainScreen::Item item);

}  // namespace antares

#endif  // ANTARES_UI_SCREENS_PLAY_AGAIN_HPP_

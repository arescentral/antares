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

#ifndef ANTARES_PLAY_AGAIN_SCREEN_HPP_
#define ANTARES_PLAY_AGAIN_SCREEN_HPP_

#include <sfz/sfz.hpp>
#include "InterfaceScreen.hpp"

namespace antares {

class PlayAgainScreen : public InterfaceScreen {
  public:
    enum Item {
        RESTART = 0,
        QUIT = 1,
        RESUME = 4,
        SKIP = 5,

        BOX = 3,
    };

    PlayAgainScreen(bool allow_resume, bool allow_skip, Item* button_pressed);
    ~PlayAgainScreen();

    virtual void become_front();
    virtual void resign_front();

  protected:
    virtual void adjust_interface();
    virtual void handle_button(int button);

  private:
    Item* _button_pressed;

    DISALLOW_COPY_AND_ASSIGN(PlayAgainScreen);
};

}  // namespace antares

#endif  // ANTARES_PLAY_AGAIN_SCREEN_HPP_

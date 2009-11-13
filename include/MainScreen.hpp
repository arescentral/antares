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

#ifndef ANTARES_MAIN_SCREEN_HPP_
#define ANTARES_MAIN_SCREEN_HPP_

#include "InterfaceScreen.hpp"
#include "SmartPtr.hpp"

namespace antares {

class MainScreen : public InterfaceScreen {
  public:
    MainScreen();
    ~MainScreen();

    virtual void become_front();

    virtual double delay();
    virtual void fire_timer();

  protected:
    virtual void adjust_interface();
    virtual void handle_button(int button);

  private:
    enum Button {
        START_NEW_GAME = 0,
        START_NETWORK_GAME = 1,
        OPTIONS = 2,
        QUIT = 3,
        ABOUT_ARES = 4,
        DEMO = 5,
        REPLAY_INTRO = 6,
    };

    DISALLOW_COPY_AND_ASSIGN(MainScreen);
};

}  // namespace antares

#endif  // ANTARES_MAIN_SCREEN_HPP_

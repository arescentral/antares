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

#ifndef ANTARES_SELECT_LEVEL_SCREEN_HPP_
#define ANTARES_SELECT_LEVEL_SCREEN_HPP_

#include "InterfaceScreen.hpp"
#include "SmartPtr.hpp"

namespace antares {

class ScrollTextScreen;

class SelectLevelScreen : public InterfaceScreen {
  public:
    SelectLevelScreen(bool* cancelled, int* scenario);
    ~SelectLevelScreen();

    virtual void become_front();

  protected:
    virtual void adjust_interface();
    virtual void handle_button(int button);
    virtual void draw() const;

  private:
    enum Item {
        // Buttons:
        OK = 0,
        CANCEL = 1,
        PREVIOUS = 2,
        NEXT = 3,

        // Text box:
        NAME = 4,
    };

    void draw_level_name(unsigned char* name, long fontNum, long itemNum) const;

    bool* _cancelled;
    int _chapter;
    int* _scenario;

    DISALLOW_COPY_AND_ASSIGN(SelectLevelScreen);
};

}  // namespace antares

#endif  // ANTARES_SELECT_LEVEL_SCREEN_HPP_

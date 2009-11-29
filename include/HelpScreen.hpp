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

#ifndef ANTARES_HELP_SCREEN_HPP_
#define ANTARES_HELP_SCREEN_HPP_

#include "InterfaceScreen.hpp"
#include "SmartPtr.hpp"

namespace antares {

class RetroText;

class HelpScreen : public InterfaceScreen {
  public:
    HelpScreen();
    ~HelpScreen();

    virtual void become_front();
    virtual void resign_front();

  protected:
    virtual void handle_button(int button);
    virtual void draw() const;

  private:
    enum Item {
        DONE = 0,

        BOX = 1,
    };

    scoped_ptr<RetroText> _text;
    Rect _text_bounds;

    DISALLOW_COPY_AND_ASSIGN(HelpScreen);
};

}  // namespace antares

#endif  // ANTARES_HELP_SCREEN_HPP_

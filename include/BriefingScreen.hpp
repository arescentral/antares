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

#ifndef ANTARES_BRIEFING_SCREEN_HPP_
#define ANTARES_BRIEFING_SCREEN_HPP_

#include "sfz/Macros.hpp"
#include "InterfaceScreen.hpp"

namespace antares {

class BriefingScreen : public InterfaceScreen {
  public:
    BriefingScreen(int scenario, bool* cancelled);
    ~BriefingScreen();

    virtual void become_front();

    virtual void key_down(int key);

  protected:
    virtual void adjust_interface();
    virtual void handle_button(int button);
    virtual void draw() const;

  private:
    enum Item {
        // Buttons:
        DONE = 0,
        PREVIOUS = 1,
        NEXT = 2,

        // Map area:
        MAP_RECT = 7,
    };

    const int _scenario;
    bool* const _cancelled;
    int _briefing_point;
    const int _briefing_point_count;
    mutable Rect _used_rect;
    mutable interfaceItemType _data_item;

    DISALLOW_COPY_AND_ASSIGN(BriefingScreen);
};

}  // namespace antares

#endif  // ANTARES_BRIEFING_SCREEN_HPP_

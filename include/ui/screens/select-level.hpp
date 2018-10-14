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

#ifndef ANTARES_UI_SCREENS_SELECT_LEVEL_HPP_
#define ANTARES_UI_SCREENS_SELECT_LEVEL_HPP_

#include <vector>

#include "data/handle.hpp"
#include "ui/screen.hpp"

namespace antares {

union Level;

class SelectLevelScreen : public InterfaceScreen {
  public:
    SelectLevelScreen(bool* cancelled, const Level** level);
    ~SelectLevelScreen();

    virtual void become_front();
    virtual void key_down(const KeyDownEvent& event);

  protected:
    virtual void overlay() const;

  private:
    enum Item {
        // Buttons:
        OK       = 0,
        CANCEL   = 1,
        PREVIOUS = 2,
        NEXT     = 3,

        // Text box:
        NAME = 4,
    };

    enum State {
        SELECTING,
        FADING_OUT,
        UNLOCKING,
    };
    State _state;

    void draw_level_name() const;

    bool*            _cancelled;
    size_t           _index;
    const Level**    _level;
    std::vector<int> _chapters;
    int              _unlock_digits;
    size_t           _unlock_chapter;
};

}  // namespace antares

#endif  // ANTARES_UI_SCREENS_SELECT_LEVEL_HPP_

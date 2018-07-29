// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// Copyright (C) 2008-2011 Ares Central
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
// License along with this program.  If not, see
// <http://www.gnu.org/licenses/>.

#ifndef ANTARES_UI_SCREENS_LOADING_HPP_
#define ANTARES_UI_SCREENS_LOADING_HPP_

#include <vector>

#include "data/handle.hpp"
#include "drawing/styled-text.hpp"
#include "game/level.hpp"
#include "math/units.hpp"
#include "ui/screen.hpp"

namespace antares {

union Level;

class LoadingScreen : public InterfaceScreen {
  public:
    LoadingScreen(const Level& level, bool* cancelled);
    ~LoadingScreen();

    virtual void become_front();
    virtual bool next_timer(wall_time& time);
    virtual void fire_timer();

    void update(int32_t current, int32_t max);

  protected:
    virtual void overlay() const;

  private:
    enum State {
        TYPING,
        LOADING,
        DONE,
    };
    State _state;

    const Level& _level;
    bool* const  _cancelled;

    std::unique_ptr<StyledText> _name_text;
    wall_time                   _next_update;
    int32_t                     _chars_typed;

    LoadState _load_state;
};

}  // namespace antares

#endif  // ANTARES_UI_SCREENS_LOADING_HPP_

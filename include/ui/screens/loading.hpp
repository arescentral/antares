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
#include <sfz/sfz.hpp>

#include "drawing/styled-text.hpp"
#include "math/units.hpp"
#include "ui/screen.hpp"

namespace antares {

struct Scenario;

class LoadingScreen : public InterfaceScreen {
  public:
    LoadingScreen(const Scenario* scenario, bool* cancelled);
    ~LoadingScreen();

    virtual void become_front();
    virtual bool next_timer(int64_t& time);
    virtual void fire_timer();

    void update(int32_t current, int32_t max);

  protected:
    virtual void handle_button(Button& button);
    virtual void overlay() const;

  private:
    enum State {
        TYPING,
        LOADING,
        DONE,
    };
    State _state;

    const Scenario* const _scenario;
    bool* const _cancelled;

    std::unique_ptr<StyledText> _name_text;
    wall_time _next_update;
    int32_t _chars_typed;

    int32_t _current;
    int32_t _max;

    DISALLOW_COPY_AND_ASSIGN(LoadingScreen);
};

}  // namespace antares

#endif  // ANTARES_UI_SCREENS_LOADING_HPP_

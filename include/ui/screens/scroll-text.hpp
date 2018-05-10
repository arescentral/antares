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

#ifndef ANTARES_UI_SCREENS_SCROLL_TEXT_HPP_
#define ANTARES_UI_SCREENS_SCROLL_TEXT_HPP_

#include "drawing/build-pix.hpp"
#include "math/geometry.hpp"
#include "math/units.hpp"
#include "ui/card.hpp"

namespace antares {

const ticks kSlowScrollInterval = ticks(4);
const ticks kFastScrollInterval = ticks(2);

class ScrollTextScreen : public Card {
  public:
    ScrollTextScreen(pn::string_view text, int width, ticks interval);
    ScrollTextScreen(pn::string_view text, int width, ticks interval, pn::string_view song);

    virtual void become_front();
    virtual void resign_front();

    virtual void mouse_down(const MouseDownEvent& event);
    virtual void key_down(const KeyDownEvent& event);
    virtual void gamepad_button_down(const GamepadButtonDownEvent& event);

    virtual bool next_timer(wall_time& time);
    virtual void fire_timer();

    virtual void draw() const;

  private:
    BuildPix         _build_pix;
    const ticks      _interval;
    const bool       _play_song;
    const pn::string _song;

    wall_time _start;
    wall_time _next_shift;
    int32_t   _position;
};

}  // namespace antares

#endif  // ANTARES_UI_SCREENS_SCROLL_TEXT_HPP_

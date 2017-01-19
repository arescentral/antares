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

#ifndef ANTARES_UI_SCREENS_OBJECT_DATA_HPP_
#define ANTARES_UI_SCREENS_OBJECT_DATA_HPP_

#include <sfz/sfz.hpp>

#include "data/handle.hpp"
#include "drawing/styled-text.hpp"
#include "math/geometry.hpp"
#include "math/units.hpp"
#include "ui/card.hpp"
#include "ui/event.hpp"

namespace antares {

class ObjectDataScreen : public Card {
  public:
    enum Trigger { MOUSE, KEY, GAMEPAD };

    ObjectDataScreen(Point origin, Handle<BaseObject> object, Trigger trigger, int which);
    ~ObjectDataScreen();

    virtual void become_front();

    virtual bool next_timer(wall_time& time);
    virtual void fire_timer();

    virtual void mouse_up(const MouseUpEvent& event);
    virtual void key_up(const KeyUpEvent& event);
    virtual void gamepad_button_up(const GamepadButtonUpEvent& event);

    virtual void draw() const;

  private:
    const Trigger _trigger;
    const int     _which;

    enum State { TYPING, DONE };
    State     _state;
    wall_time _next_update;
    wall_time _next_sound;
    int       _typed_chars;

    Rect                        _bounds;
    std::unique_ptr<StyledText> _text;

    DISALLOW_COPY_AND_ASSIGN(ObjectDataScreen);
};

}  // namespace antares

#endif  // ANTARES_UI_SCREENS_OBJECT_DATA_HPP_

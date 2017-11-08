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

#ifndef ANTARES_UI_SCREEN_HPP_
#define ANTARES_UI_SCREEN_HPP_

#include <sfz/sfz.hpp>
#include <vector>

#include "data/interface.hpp"
#include "drawing/interface.hpp"
#include "game/cursor.hpp"
#include "math/geometry.hpp"
#include "ui/card.hpp"

namespace antares {

class InterfaceScreen : public Card {
  public:
    InterfaceScreen(sfz::PrintItem name, const Rect& bounds, bool full_screen);
    InterfaceScreen(sfz::Json json, const Rect& bounds, bool full_screen);
    ~InterfaceScreen();

    virtual void become_front();
    virtual void resign_front();

    virtual void draw() const;

    virtual void mouse_down(const MouseDownEvent& event);
    virtual void mouse_up(const MouseUpEvent& event);
    virtual void mouse_move(const MouseMoveEvent& event);
    virtual void key_down(const KeyDownEvent& event);
    virtual void key_up(const KeyUpEvent& event);
    virtual void gamepad_button_down(const GamepadButtonDownEvent& event);
    virtual void gamepad_button_up(const GamepadButtonUpEvent& event);

  protected:
    virtual void overlay() const;
    virtual void adjust_interface();
    virtual void handle_button(Button& button) = 0;

    void truncate(size_t size);
    void extend(const sfz::Json& json);

    Point                offset() const;
    size_t               size() const;
    const InterfaceItem& item(int index) const;
    InterfaceItem& mutable_item(int index);

  private:
    enum State {
        NORMAL,
        MOUSE_DOWN,
        KEY_DOWN,
        GAMEPAD_DOWN,
    };
    State _state;

    sfz::Json load_json(sfz::PrintItem id);
    void become_normal();

    const Rect                                  _bounds;
    const bool                                  _full_screen;
    std::vector<std::unique_ptr<InterfaceItem>> _items;
    Button*                                     _hit_button;
    uint32_t                                    _pressed;
    Cursor                                      _cursor;

    DISALLOW_COPY_AND_ASSIGN(InterfaceScreen);
};

}  // namespace antares

#endif  // ANTARES_UI_SCREEN_HPP_

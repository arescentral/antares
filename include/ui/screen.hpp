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

#include <vector>

#include "data/interface.hpp"
#include "drawing/interface.hpp"
#include "game/cursor.hpp"
#include "math/geometry.hpp"
#include "ui/card.hpp"
#include "ui/widget.hpp"

namespace antares {

class InterfaceScreen : public Card {
  public:
    InterfaceScreen(pn::string_view name, const Rect& bounds);
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

    Point offset() const;

    const PlainButton*    button(int id) const;
    PlainButton*          button(int id);
    const CheckboxButton* checkbox(int id) const;
    CheckboxButton*       checkbox(int id);
    const Widget*         widget(int id) const;
    Widget*               widget(int id);

  private:
    enum State {
        NORMAL,
        MOUSE_DOWN,
        KEY_DOWN,
        GAMEPAD_DOWN,
    };
    State _state = NORMAL;

    pn::value load_pn(pn::string_view id);
    void      set_state(
                 State state, Widget* widget = nullptr, Key key = Key::NONE,
                 Gamepad::Button gamepad = Gamepad::Button::NONE);

    const Rect                           _bounds;
    bool                                 _full_screen = false;
    std::vector<std::unique_ptr<Widget>> _widgets;
    Widget*                              _active_widget   = nullptr;
    Key                                  _key_pressed     = Key::NONE;
    Gamepad::Button                      _gamepad_pressed = Gamepad::Button::NONE;
    Cursor                               _cursor;
};

}  // namespace antares

#endif  // ANTARES_UI_SCREEN_HPP_

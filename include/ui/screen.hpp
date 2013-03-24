// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// Copyright (C) 2008-2012 The Antares Authors
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
#include <sfz/sfz.hpp>

#include "data/interface.hpp"
#include "math/geometry.hpp"
#include "ui/card.hpp"

namespace antares {

class InterfaceScreen : public Card {
  public:
    enum Id {
        MAIN = 5000,
        SELECT_LEVEL = 5011,
        BRIEFING = 6000,
        LOADING = 6001,
        HELP = 5012,

        OPTIONS_SOUND = 5007,
        OPTIONS_KEYS = 5030,
        OPTIONS_KEYS_SHIP = 5031,
        OPTIONS_KEYS_COMMAND = 5032,
        OPTIONS_KEYS_SHORTCUT = 5033,
        OPTIONS_KEYS_UTILITY = 5034,
        OPTIONS_KEYS_HOTKEY = 5035,

        PLAY_AGAIN = 5008,
        PLAY_AGAIN_RESUME = 5009,
        PLAY_AGAIN_SKIP_RESUME = 5017,
    };

    InterfaceScreen(Id id, const Rect& bounds, bool full_screen);
    ~InterfaceScreen();

    virtual void become_front();

    virtual void draw() const;

    virtual void mouse_down(const MouseDownEvent& event);
    virtual void mouse_up(const MouseUpEvent& event);
    virtual void mouse_move(const MouseMoveEvent& event);
    virtual void key_down(const KeyDownEvent& event);
    virtual void key_up(const KeyUpEvent& event);

  protected:
    virtual void adjust_interface();
    virtual void handle_button(int button) = 0;

    void truncate(size_t size);
    void extend(const std::vector<std::unique_ptr<InterfaceItem>>& items);

    size_t size() const;
    const InterfaceItem& item(int index) const;
    InterfaceItem& mutable_item(int index);
    void offset(int offset_x, int offset_y);

  private:
    enum State {
        NORMAL,
        MOUSE_DOWN,
        KEY_DOWN,
    };
    State _state;

    const Rect _bounds;
    const bool _full_screen;
    std::vector<std::unique_ptr<InterfaceItem>> _items;
    int _hit_item;

    DISALLOW_COPY_AND_ASSIGN(InterfaceScreen);
};

}  // namespace antares

#endif  // ANTARES_UI_SCREEN_HPP_

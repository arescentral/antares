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

#ifndef ANTARES_UI_INTERFACE_SCREEN_HPP_
#define ANTARES_UI_INTERFACE_SCREEN_HPP_

#include <vector>
#include <sfz/sfz.hpp>

#include "data/interface.hpp"
#include "math/geometry.hpp"
#include "ui/card.hpp"

namespace antares {

class InterfaceScreen : public Card {
  public:
    InterfaceScreen(int id, const Rect& bounds, bool full_screen);
    ~InterfaceScreen();

    virtual void become_front();

    virtual void draw() const;

    virtual void mouse_down(const MouseDownEvent& event);
    virtual void mouse_up(const MouseUpEvent& event);
    virtual void mouse_move(const MouseMoveEvent& event);
    virtual void key_down(const KeyDownEvent& event);
    virtual void key_up(const KeyUpEvent& event);

  protected:
    double last_event() const;
    virtual void adjust_interface();
    virtual void handle_button(int button) = 0;

    void truncate(size_t size);
    void extend(int id, size_t within);

    size_t size() const;
    const interfaceItemType& item(int index) const;
    interfaceItemType* mutable_item(int index);
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
    double _last_event;
    std::vector<interfaceItemType> _items;
    int _hit_item;

    DISALLOW_COPY_AND_ASSIGN(InterfaceScreen);
};

}  // namespace antares

#endif  // ANTARES_UI_INTERFACE_SCREEN_HPP_

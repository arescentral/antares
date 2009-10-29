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

#ifndef ANTARES_CARD_HPP_
#define ANTARES_CARD_HPP_

#include "Geometry.hpp"

namespace antares {

class CardStack;

class Card {
  public:
    Card();
    virtual ~Card();

    // Stack-related.
    virtual void become_front();
    virtual void resign_front();

    // Mouse-related.
    virtual bool mouse_down(int button, const Point& loc);
    virtual bool mouse_up(int button, const Point& loc);
    virtual bool mouse_moved(const Point& loc);

    // Key-related.
    virtual bool key_down(int key);
    virtual bool key_up(int key);

    // Timer-related.
    virtual double delay();
    virtual void fire_timer();

    CardStack* stack() const;

  private:
    friend class CardStack;

    void set_stack(CardStack* stack);

    CardStack* _stack;
};

}  // namespace antares

#endif  // ANTARES_CARD_HPP_

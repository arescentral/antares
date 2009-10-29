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

#include "Card.hpp"

#include <assert.h>

namespace antares {

Card::Card()
        : _stack(NULL) { }

Card::~Card() { }

void Card::become_front() { }

void Card::resign_front() { }

bool Card::mouse_down(int button, const Point& loc) {
    static_cast<void>(button);
    static_cast<void>(loc);
    return false;
}

bool Card::mouse_up(int button, const Point& loc) {
    static_cast<void>(button);
    static_cast<void>(loc);
    return false;
}

bool Card::mouse_moved(const Point& loc) {
    static_cast<void>(loc);
    return false;
}

bool Card::key_down(int key) {
    static_cast<void>(key);
    return false;
}

bool Card::key_up(int key) {
    static_cast<void>(key);
    return false;
}

double Card::delay() {
    return 0.0;
}

void Card::fire_timer() { }

CardStack* Card::stack() const {
    return _stack;
}

void Card::set_stack(CardStack* stack) {
    // Can add or remove from stack, not move between.
    assert(stack == NULL || _stack == NULL);
    _stack = stack;
}

}  // namespace antares

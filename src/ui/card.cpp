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

#include "ui/card.hpp"

#include <algorithm>
#include <stdlib.h>
#include <sfz/sfz.hpp>

#include "video/driver.hpp"

using sfz::Exception;
using sfz::format;
using std::unique_ptr;

namespace antares {

Card::Card()
        : _stack(NULL),
          _next(NULL) { }

Card::~Card() { }

void Card::become_front() { }

void Card::resign_front() { }

void Card::draw() const {
    next()->draw();
}

bool Card::next_timer(int64_t& time) {
    static_cast<void>(time);
    return false;
}

void Card::fire_timer() { }

CardStack* Card::stack() const {
    return _stack;
}

Card* Card::next() const {
    return _next;
}

void Card::set_stack(CardStack* stack) {
    // Can add or remove from stack, not move between.
    if (_stack != NULL) {
        throw Exception("Card is already on a stack");
    }
    _stack = stack;
    _next = stack->top();
}

CardStack::CardStack(Card* top)
        : _top(NULL) {
    push(top);
}

bool CardStack::empty() const {
    return _top == NULL;
}

void CardStack::push(Card* card) {
    if (!empty()) {
        _top->resign_front();
    }
    card->set_stack(this);
    _top = card;
    card->become_front();
}

void CardStack::pop(Card* card) {
    if (card != _top) {
        throw Exception(format("tried to pop card {0} when not frontmost", card));
    }
    card->resign_front();
    _top = card->next();
    delete card;
    if (!empty()) {
        _top->become_front();
    }
}

Card* CardStack::top() const {
    return _top;
}

}  // namespace antares

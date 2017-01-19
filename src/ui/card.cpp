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

#include "ui/card.hpp"

#include <stdlib.h>
#include <algorithm>
#include <sfz/sfz.hpp>

#include "video/driver.hpp"

using sfz::Exception;
using sfz::format;
using std::unique_ptr;

namespace antares {

Card::Card() : _stack(NULL) {}

Card::~Card() {}

void Card::become_front() {}

void Card::resign_front() {}

void Card::draw() const {
    next()->draw();
}

bool Card::next_timer(wall_time& time) {
    static_cast<void>(time);
    return false;
}

void Card::fire_timer() {}

CardStack* Card::stack() const {
    return _stack;
}

Card* Card::next() const {
    return _next.get();
}

CardStack::CardStack(Card* top) {
    push(top);
}

bool CardStack::empty() const {
    return _top == nullptr;
}

void CardStack::push(Card* card) {
    if (!empty()) {
        _top->resign_front();
    }
    card->_stack = this;
    unique_ptr<Card> c(card);
    swap(_top, c->_next);
    swap(_top, c);
    card->become_front();
}

void CardStack::pop(Card* card) {
    if (card != _top.get()) {
        throw Exception(format("tried to pop card {0} when not frontmost", card));
    }
    unique_ptr<Card> old;
    card->resign_front();
    swap(_top, old);
    swap(_top, old->_next);
    if (!empty()) {
        _top->become_front();
    }
}

Card* CardStack::top() const {
    return _top.get();
}

}  // namespace antares

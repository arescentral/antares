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

#include "CardStack.hpp"

#include <algorithm>
#include "sfz/sfz.hpp"
#include "Base.h"
#include "Card.hpp"

using sfz::Exception;
using sfz::format;

namespace antares {

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

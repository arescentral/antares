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
#include <Base.h>
#include "Card.hpp"

namespace antares {

CardStack::CardStack(Card* top) {
    push(top);
}

bool CardStack::empty() const {
    return _list.empty();
}

void CardStack::push(Card* card) {
    if (!_list.empty()) {
        _list.back()->resign_front();
    }
    _list.push_back(card);
    card->set_stack(this);
    card->become_front();
}

void CardStack::pop(Card* card) {
    if (card != _list.back()) {
        fprintf(stderr, "tried to pop card %p when not frontmost\n", card);
        exit(1);
    }
    card->resign_front();
    delete card;
    _list.pop_back();
    if (!_list.empty()) {
        _list.back()->become_front();
    }
}

void CardStack::send(const EventRecord& evt) {
    for (std::vector<Card*>::reverse_iterator it = _list.rbegin(); it != _list.rend(); ++it) {
        switch (evt.what) {
          case mouseDown:
            if ((*it)->mouse_down(0, evt.where)) {
                return;
            }
            break;
          case mouseUp:
            if ((*it)->mouse_up(0, evt.where)) {
                return;
            }
            break;
          case autoKey:
          case keyDown:
            if ((*it)->key_down(evt.message)) {
                return;
            }
            break;
          case keyUp:
            if ((*it)->key_up(evt.message)) {
                return;
            }
            break;
        }
    }
}

double CardStack::next_delay() {
    int i;
    if (min_delay_index(&i)) {
        return _list[i]->delay();
    } else {
        return std::numeric_limits<double>::infinity();
    }
}

void CardStack::fire_next_timer() {
    int i;
    if (min_delay_index(&i)) {
        _list[i]->fire_timer();
    }
}

bool CardStack::min_delay_index(int* min_index) {
    bool result = false;
    double min_delay = std::numeric_limits<double>::infinity();
    for (size_t i = 0; i < _list.size(); ++i) {
        if (_list[i]->delay() > 0.0 && _list[i]->delay() < min_delay) {
            min_delay = _list[i]->delay();
            *min_index = i;
            result = true;
        }
    }
    return result;
}

}  // namespace antares

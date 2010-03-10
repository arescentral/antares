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

#ifndef ANTARES_CARD_STACK_HPP_
#define ANTARES_CARD_STACK_HPP_

namespace antares {

class Card;

// A stack of Card objects that constitutes an application.
//
// There should be exactly one CardStack running in the application; it defines the user interface
// and the control flow between different screens of the interface.  A stack starts with a single
// Card on top.  Eventually, when all Card objects have been popped from the stack, the application
// should quit.
//
// The `push()` and `pop()` methods are provided mainly for the benefit of Card objects in the
// stack, so that they may push new cards and eventually pop themselves.  The remaining methods are
// primarily useful for the platform-specific code which needs to dispatch events to Cards.
class CardStack {
  public:
    // Initializes the stack with a single card.
    //
    // @param [in] top      The initial top of the stack.  Must be non-NULL; takes ownership.
    CardStack(Card* top);

    // @returns             true if the stack is empty, false otherwise.
    bool empty() const;

    // Pushes a new Card on top of the stack.
    //
    // Results in a call to `resign_front()` on the old top of the stack, and a call to
    // `card->become_front()`.
    //
    // @param [in] card     The Card to push.  Must be non-NULL; takes ownership.
    void push(Card* card);

    // Pops the top-most card from the stack.
    //
    // Results in a call to `card->resign_front()`, the deletion of `card`, and a call
    // `become_front()` on the new top of the stack.
    //
    // @param [in] card     The Card to pop.  Must be non-NULL, and the top-most card on the stack.
    void pop(Card* card);

    // @returns             The top-most card on the stack.
    Card* top() const;

  private:
    // A linked list of cards.  The card here is on top.
    Card* _top;
};

}  // namespace antares

#endif  // ANTARES_CARD_STACK_HPP_

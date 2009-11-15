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

#include <vector>

namespace antares {

class Card;
class EventRecord;

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
    const Card* top() const;

    // Dispatches an event to the appropriate Card on the stack.
    //
    // Given an EventRecord which can be mapped to some event method of `Card`, tries dispatching
    // the appropriate method to each Card on the stack, working from the top down until a Card
    // claims to have handled it (represented by returning true from the event method).
    //
    // The use of this method is largely considered a legacy at this point, but will continue until
    // WaitNextEvent() is fully excised from the code and EventRecord can be removed with it.
    //
    // @param [in] evt      A record containing the event to dispatch and its parameters.
    void send(const EventRecord& evt);

    // Figures out which Card should have its timer fired next, and returns that card.  The time at
    // which the timer should be fired is placed into `at`.
    //
    // @param [out] at      The time at which to fire the timer.
    // @returns             The Card whose timer should be fired.
    Card* next_event(double* at);

  private:
    // The stack of cards.  The back/bottom is at index 0, and the top at `size() - 1`.
    std::vector<Card*> _list;
};

}  // namespace antares

#endif  // ANTARES_CARD_STACK_HPP_

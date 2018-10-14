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

#ifndef ANTARES_UI_CARD_HPP_
#define ANTARES_UI_CARD_HPP_

#include <memory>

#include "math/units.hpp"
#include "ui/event.hpp"

namespace antares {

class CardStack;

// A unit of user interface and control flow.
//
// Logically, the game is thought of as consisting of a single stack of Card objects.  A Card may
// have a visual presence, or it may not.  A Card which has a visual presence behaves very much
// like a window, except that Card objects are never reordered (at present).
//
// On the other hand, many Card objects do not have a visual presence, and exist only to track
// movement between different cards.  As an example, the bottom-most card in Antares is the
// `Master` Card, which cycles between the various splash screen Card objects, then pushes the
// `MainScreen` Card, which handles the main screen's user interface.
//
// Many Card objects will find it useful to implement a state machine to implement control flow
// between the various Card objects they may push on top of themselves.  The base Card
// implementation does not provide a state machine implementation, although it would probably be
// useful to have a standard one somewhere.
class Card : public EventReceiver {
  public:
    Card();
    virtual ~Card();

    // Stack-related methods.
    //
    // There are two stack-related methods, `become_front()` and `resign_front()`, which notify a
    // Card when it is and is not the front-most card on the stack.  This may be expanded two-fold
    // in the future, to distinguish between when these events happen because of the addition or
    // the removal of Card objects.

    // Called when this Card becomes the front-most card on the stack.
    //
    // This can happen either by virtue of the Card being pushed to the top, or by virtue of all
    // Card objects above it being popped.  In the future, we may choose to provide separate
    // methods for these two cases.
    virtual void become_front();

    // Called when this Card ceases to be the front-most card on the stack.
    //
    // This can happen either by virtue of the Card being popped, or by virtue of another Card
    // being pushed on top of it.  In the former case, the destructor will be called immediately
    // after `resign_front()`, so in the future, we may choose to call `resign_front()` only in the
    // latter case.
    virtual void resign_front();

    // Draws the card.
    virtual void draw() const;

    // Timer-related methods.
    //
    // There are two timer-related methods, `next_timer()` and `fire_timer()`.  Each time the run
    // loop prepares to wait for the next event, it asks each Card in the stack when it would like
    // its timer to next fire, and ensures that the timer either fires at that time, or that it
    // will ask for a new time before then.

    // Specifies the time at which which to fire this Card's timer in microseconds since the epoch.
    //
    // The specified value is an absolute time, e.g., if a Card wants `fire_timer()` to be called
    // in one second, it should return `now_usecs() + 1e6`.
    //
    // It is not guaranteed that the timer will fire in that period of time, but if not, then
    // `next_timer()` will be called again before then.  So, if the Card wants `fire_timer()` to be
    // called in one second, but `next_timer()` is called again after 0.6 seconds, then the second
    // time, it should instead return 0.4e6.
    //
    // If false is returned, then `fire_timer()` will not be called at all.
    //
    // @param [out] time    If true is returned, the referent is set to the time point at which to
    //                      fire the timer.  If false is returned, is unchanged.
    // @returns             If `fire_timer()` should be called, true; otherwise, false.
    virtual bool next_timer(wall_time& time);

    // Called when a Card's timer should be fired.
    //
    // If this Card has requested that its timer fire at a certain time, then this method is called
    // at that time, subject to the caveat given in the documentation for `next_timer()`.
    virtual void fire_timer();

    // Returns the stack this Card is in.
    //
    // If this Card has not yet been added to a stack, then returns NULL.  This method is probably
    // most useful in subclasses of Card, where the Card wants to either push a new Card on top of
    // itself, or to remove itself from the stack.
    //
    // @returns             The stack which contains this object.
    CardStack* stack() const;

  protected:
    // Returns the Card below this card.
    //
    // If this card is the bottom-most card, then returns NULL.  For any other card, returns a
    // non-NULL card which represents the next card down on the stack.  In the implementation of
    // the event methods above, it may be useful to use `next()` to forward the call down the
    // stack.
    //
    // @returns             The Card below this one.
    Card* next() const;

  private:
    friend class CardStack;

    // The containing stack.  Initially NULL and set by `set_stack()`.
    CardStack* _stack;

    // The next card down on the stack.  Is NULL for the bottom-most card, and non-NULL for every
    // card above it.
    std::unique_ptr<Card> _next;
};

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
    std::unique_ptr<Card> _top;
};

}  // namespace antares

#endif  // ANTARES_UI_CARD_HPP_

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

#include "Event.hpp"

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
//
// In the future, it is planned that two methods will be added to Card for drawing the user
// interface, `draw()` and `opaque()`.  These will allow Cards to display content on-demand instead
// of indirectly displaying content by updating gRealWorld.
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

    // Timer-related methods.
    //
    // There are two timer-related methods, `next_timer()` and `fire_timer()`.  Each time the run
    // loop prepares to wait for the next event, it asks each Card in the stack when it would like
    // its timer to next fire, and ensures that the timer either fires at that time, or that it
    // will ask for a new time before then.

    // Returns the time at which which to fire this Card's timer.
    //
    // The returned value is an absolute time, e.g., if a Card wants `fire_timer()` to be called in
    // one second, it should return `now_secs() + 1.0`.
    //
    // It is not guaranteed that the timer will fire in that period of time, but if not, then
    // `next_timer()` will be called again before then.  So, if the Card wants `fire_timer()` to be
    // called in one second, but `next_timer()` is called again after 0.6 seconds, then the second
    // time, it should instead return 0.4.
    //
    // If 0.0 is returned, then `fire_timer()` will not be called at all.
    //
    // @returns             The time delta to the next requested `fire_timer()` call.
    virtual double next_timer();

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
    // CardStack needs access to `set_stack()`.
    friend class CardStack;

    // Sets the stack.
    //
    // After a CardStack pushes a Card to the top, it calls this method to set `_stack` to point to
    // itself, then calls `become_front()`.  This method is called at most once per object, since
    // Card objects are deleted after they are removed from the stack (exactly once, if one assumes
    // that every Card gets pushed to a stack at some point).
    //
    // @param [in] stack    The stack that this Card is now in.
    void set_stack(CardStack* stack);

    // The containing stack.  Initially NULL and set by `set_stack()`.
    CardStack* _stack;

    // The next card down on the stack.  Is NULL for the bottom-most card, and non-NULL for every
    // card above it.
    Card* _next;
};

}  // namespace antares

#endif  // ANTARES_CARD_HPP_

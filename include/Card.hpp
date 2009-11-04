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
class Card {
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

    // Mouse-related methods.
    //
    // There are three mouse-related methods, `mouse_down()`, `mouse_up()`, and `mouse_moved()`.
    // In the future, we expect to add methods for the scroll wheel, at a minimum.  Between
    // successive calls to `mouse_down()` and `mouse_up()`, we assume that the mouse button is
    // down, and that `mouse_moved()` events correspond to mouse drags; and outside of them, we
    // assume that the mouse button is up.  That may not be strictly true (e.g., if we switch
    // between apps), but we're willing to tolerate bugs which depend on this behavior for the time
    // being.

    // Called when the numbered button is pressed.
    //
    // Buttons are numbered from 0 to 2, for the primary, secondary, and tertiary buttons (normally
    // the left, right, and middle buttons).  On some systems, the scroll wheel is implemented as
    // additional buttons above these, but when we support the scroll wheel, we will probably
    // choose to implement it in terms of additional methods instead.
    //
    // At present, we only really deal with the first mouse button, as an artifact of Ares' origins
    // on the classic MacOS.  There are plenty of desirable features one could expect from multiple
    // mouse buttons, though.
    //
    // @param [in] button   The button that was pressed.  Is in [0, 1, 2].
    // @param [in] loc      The location of the press, relative to the top left of the screen.
    // @returns             true if this Card handled the event, false otherwise.
    virtual bool mouse_down(int button, const Point& loc);

    // Called when the numbered button is released.
    //
    // Takes the same parameters as `mouse_down()` and returns the same values.
    virtual bool mouse_up(int button, const Point& loc);

    // Called when the mouse is moved.
    //
    // This method is invoked regardless of whether any mouse buttons are pressed.
    //
    // @param [in] loc      The new location of the mouse.
    // @returns             true if this Card handled the event, false otherwise.
    virtual bool mouse_moved(const Point& loc);

    // Key-related methods.
    //
    // There are two key-related methods, `key_up()` and `key_down()`.  Between successive calls to
    // the two, we assume that the key is down, and outside of them, we assume that the key is up.
    // That may not be strictly true (e.g., if we switch between apps) but we're willing to
    // tolerate bugs which depend on this behavior for the time being.

    // Called when a key is pressed.
    //
    // The value provided in `key` is an ADB key-code, and is a horrible artifact of Ares' origins
    // on the classic MacOS.  For the time being, this means we are not tolerant of non-US keyboard
    // layouts, although it remains relatively easy to implement within Cocoa.
    //
    // We will eventually want to use a more cross-platform/international/sane numbering of keys,
    // such as perhaps the Cocoa numbering scheme, which uses part of the Unicode E000-F8FF
    // "private use area" to represent non-literal characters such as arrows.  It doesn't, however,
    // seem to include modifier keys, so maybe xkeysyms would be better, even though it collides
    // with Unicode.
    //
    // Currently, the best documentation of ADB key-codes is on page 2-43 of "Macintosh Toolbox
    // Essentials", a PDF of which is available at http://developer.apple.com/legacy/mac/library/
    // documentation/mac/pdf/MacintoshToolboxEssentials.pdf
    //
    // @param [in] key      The key-code that was pressed.
    // @returns             true if this Card handled the event, false otherwise.
    virtual bool key_down(int key);

    // Called when a key is released.
    //
    // Takes the same parameters as `key_down()` and returns the same values.
    virtual bool key_up(int key);

    // Timer-related methods.
    //
    // There are two timer-related methods, `delay()` and `fire_timer()`.  Each time the run loop
    // prepares to wait for the next event, it asks each Card in the stack when it would like its
    // timer to next fire, and ensures that the timer either fires at that time, or that it will
    // ask for a new delay before then.

    // Returns the delay after which to fire this Card's timer.
    //
    // The returned value is a delta from the current time, e.g., if a Card wants `fire_timer()` to
    // be called in one second, it should return 1.0.
    //
    // It is not guaranteed that the timer will fire in that period of time, but if not, then
    // `delay()` will be called again before then.  So, if the Card wants `fire_timer()` to be
    // called in one second, but `delay()` is called again after 0.6 seconds, then the second time,
    // it should instead return 0.4.
    //
    // If 0.0 is returned, then `fire_timer()` will not be called at all.
    //
    // @returns             The time delta to the next requested `fire_timer()` call.
    virtual double delay();

    // Called when a Card's delay expires.
    //
    // If this Card has requested that its timer fire after a certain delay, then this method is
    // called after that delay, subject to the caveat given in the documentation for `delay()`.
    virtual void fire_timer();

    // Returns the stack this Card is in.
    //
    // If this Card has not yet been added to a stack, then returns NULL.  This method is probably
    // most useful in subclasses of Card, where the Card wants to either push a new Card on top of
    // itself, or to remove itself from the stack.
    //
    // @returns             The stack which contains this object.
    CardStack* stack() const;

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
};

}  // namespace antares

#endif  // ANTARES_CARD_HPP_

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

#ifndef ANTARES_DEBRIEFING_SCREEN_HPP_
#define ANTARES_DEBRIEFING_SCREEN_HPP_

#include "sfz/Bytes.hpp"
#include "sfz/SmartPtr.hpp"
#include "Card.hpp"

namespace antares {

class PixMap;
class RetroText;

class DebriefingScreen : public Card {
  public:
    DebriefingScreen(int text_id);

    DebriefingScreen(
            int text_id, int your_length, int par_length, int your_loss, int par_loss,
            int your_kill, int par_kill);

    virtual void become_front();
    virtual void resign_front();

    virtual bool mouse_down(int button, const Point& where);
    virtual bool key_down(int key);

    virtual double next_timer();
    virtual void fire_timer();

  private:
    void initialize(bool do_score);

    enum State {
        TYPING,
        DONE,
    };
    State _state;

    sfz::Bytes _message;
    sfz::scoped_ptr<RetroText> _score;
    Rect _pix_bounds;
    Rect _message_bounds;
    Rect _score_bounds;
    sfz::scoped_ptr<PixMap> _pix;

    double _next_update;
    int _typed_chars;

    DISALLOW_COPY_AND_ASSIGN(DebriefingScreen);
};

}  // namespace antares

#endif  // ANTARES_DEBRIEFING_SCREEN_HPP_

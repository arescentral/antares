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

#ifndef ANTARES_TRANSITIONS_HPP_
#define ANTARES_TRANSITIONS_HPP_

#include "Base.h"
#include "Card.hpp"
#include "ColorTable.hpp"

namespace antares {

void InitTransitions( void);
void ResetTransitions( void);
void CleanupTransitions( void);
void StartBooleanColorAnimation( long, long, unsigned char);
void UpdateBooleanColorAnimation( long);
void RestoreOriginalColors( void);
void InstantGoalTransition( void);
bool AutoFadeTo( long, const RgbColor&, bool);
bool AutoFadeFrom( long, bool);
bool AutoMusicFadeTo( long, const RgbColor&, bool);

class ColorFade : public Card {
  public:
    enum Direction {
        TO_COLOR = 0,
        FROM_COLOR = 1,
    };

    ColorFade(
            int clut_id, Direction direction, const RgbColor& color, double duration,
            bool allow_skip, bool* skipped);

    virtual void become_front();
    virtual void resign_front();

    virtual void mouse_down(const MouseDownEvent& event);
    virtual double next_timer();
    virtual void fire_timer();

  private:
    const Direction _direction;
    const ColorTable _transition_colors;
    ColorTable _current_colors;
    const RgbColor _color;

    const bool _allow_skip;
    bool* _skipped;

    double _start;
    double _next_event;
    const double _duration;
};

class PictFade : public Card {
  public:
    PictFade(int pict_id, int clut_id, bool* skipped);

    virtual void become_front();
    virtual void resign_front();

    virtual void mouse_down(const MouseDownEvent& event);
    virtual double next_timer();
    virtual void fire_timer();

  protected:
    virtual double fade_time() const;
    virtual double display_time() const;
    virtual bool skip() const;

  private:
    void wax();
    void wane();

    enum State {
        NEW,
        WAXING,
        FULL,
        WANING,
    };

    State _state;
    const int _pict_id;
    const int _clut_id;
    bool* _skipped;

    double _wane_start;
};

}  // namespace antares

#endif // ANTARES_TRANSITIONS_HPP_

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

#ifndef ANTARES_SCROLL_TEXT_HPP_
#define ANTARES_SCROLL_TEXT_HPP_

#include "Card.hpp"
#include "Geometry.hpp"
#include "SmartPtr.hpp"

namespace antares {

class PixMap;

class ScrollTextScreen : public Card {
  public:
    ScrollTextScreen(int text_id, int width, double speed);
    ScrollTextScreen(int text_id, int width, double speed, int song_id);

    virtual void become_front();
    virtual void resign_front();

    virtual bool mouse_down(int button, const Point& where);
    virtual bool key_down(int key);

    virtual double next_timer();
    virtual void fire_timer();

  private:
    scoped_ptr<PixMap> _pix_map;
    const double _speed;
    const bool _play_song;
    const int _song_id;

    double _start;
    double _next_shift;
    Rect _window;
};

}  // namespace antares

#endif  // ANTARES_SCROLL_TEXT_HPP_

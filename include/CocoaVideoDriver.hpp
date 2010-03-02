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

#ifndef ANTARES_COCOA_VIDEO_DRIVER_HPP_
#define ANTARES_COCOA_VIDEO_DRIVER_HPP_

#include <queue>
#include "sfz/Macros.hpp"
#include "Base.h"
#include "VideoDriver.hpp"

namespace antares {

class CocoaVideoDriver : public VideoDriver {
  public:
    CocoaVideoDriver();
    virtual void send_event(EventRecord evt);
    virtual bool wait_next_event(EventRecord* evt, double sleep);
    virtual bool button();
    virtual Point get_mouse();
    virtual void get_keys(KeyMap k);

    virtual void set_game_state(GameState state);
    virtual int get_demo_scenario();
    virtual void main_loop_iteration_complete(uint32_t game_time);
    virtual int ticks();

    virtual void loop(CardStack* stack);

  private:
    void enqueue_events(id until);

    void mouse_down(int button, const Point& where);
    void mouse_up(int button, const Point& where);
    void key_down(int key_code);
    void key_up(int key_code);
    void flags_changed(int flags);

    int64_t _start_time;

    bool _button;
    Point _mouse;
    int _last_modifiers;
    KeyMap _keys;
    std::queue<EventRecord> _event_queue;

    DISALLOW_COPY_AND_ASSIGN(CocoaVideoDriver);
};

}  // namespace antares

#endif  // ANTARES_COCOA_VIDEO_DRIVER_HPP_

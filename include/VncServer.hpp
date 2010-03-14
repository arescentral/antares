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

#ifndef ANTARES_VNC_SERVER_HPP_
#define ANTARES_VNC_SERVER_HPP_

#include <queue>
#include <map>
#include "VideoDriver.hpp"
#include "sfz/ScopedFd.hpp"

class Event;

namespace antares {

class VncVideoDriver : public VideoDriver {
  public:
    VncVideoDriver(int port);
    virtual bool button();
    virtual Point get_mouse();
    virtual void get_keys(KeyMap* k);

    virtual void set_game_state(GameState state);
    virtual int get_demo_scenario();
    virtual void main_loop_iteration_complete(uint32_t game_time);
    virtual int ticks();

    virtual void loop(CardStack* stack);

  private:
    bool vnc_poll(int64_t timeout);
    Event* wait_next_event(double sleep);

    const int64_t _start_time;
    sfz::ScopedFd _listen;
    sfz::ScopedFd _socket;
    bool _button;
    Point _mouse;
    std::queue<Event*> _event_queue;
    std::map<int, int> _key_map;

    sfz::scoped_ptr<sfz::BinaryReader> _in;
};

}  // namespace antares

#endif  // ANTARES_VNC_SERVER_HPP_

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

#ifndef ANTARES_VIDEO_DRIVER_HPP_
#define ANTARES_VIDEO_DRIVER_HPP_

#include <stdint.h>
#include <limits>
#include "Base.h"

namespace antares {

class CardStack;

enum GameState {
    UNKNOWN,
    MAIN_SCREEN_INTERFACE,
    OPTIONS_INTERFACE,
    KEY_CONTROL_INTERFACE,
    SELECT_LEVEL_INTERFACE,
    MISSION_INTERFACE,
    PLAY_GAME,
    DONE_GAME,
};

class VideoDriver {
  public:
    virtual ~VideoDriver() { }
    virtual void send_event(EventRecord evt) = 0;
    virtual bool wait_next_event(EventRecord* evt, double sleep) = 0;
    virtual bool button() = 0;
    virtual Point get_mouse() = 0;
    virtual void get_keys(KeyMap k) = 0;

    virtual void set_game_state(GameState state) = 0;
    virtual int get_demo_scenario() = 0;
    virtual void main_loop_iteration_complete(uint32_t game_time) = 0;
    virtual int ticks() = 0;

    // Event loop interface.  Should eventually be its own class.
    virtual void loop(CardStack* stack) = 0;

    static VideoDriver* driver();
    static void set_driver(VideoDriver* mode);
};

}  // namespace antares

#endif  // ANTARES_VIDEO_DRIVER_HPP_

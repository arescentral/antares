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

#ifndef ANTARES_TEST_VIDEO_DRIVER_HPP_
#define ANTARES_TEST_VIDEO_DRIVER_HPP_

#include "VideoDriver.hpp"

class TestingVideoDriver : public VideoDriver {
  public:
    TestingVideoDriver();

    virtual void send_event(EventRecord);
    virtual bool button();
    virtual Point get_mouse();
    virtual void get_keys(KeyMap keys);
    virtual int ticks();
    virtual void main_loop_iteration_complete(uint32_t);
    virtual void set_game_state(GameState state);

  protected:
    GameState state() const;

  private:
    int _current_time;
    GameState _state;
};

class MainScreenVideoDriver : public TestingVideoDriver {
  public:
    virtual bool wait_next_event(EventRecord*, int);
    virtual int get_demo_scenario();
};

class MissionBriefingVideoDriver : public TestingVideoDriver {
  public:
    MissionBriefingVideoDriver(int level);

    virtual bool wait_next_event(EventRecord* evt, int);
    virtual int get_demo_scenario();

  private:
    const int _level;
    int _briefing_num;
};

class DemoVideoDriver : public TestingVideoDriver {
  public:
    DemoVideoDriver(int level);

    virtual bool wait_next_event(EventRecord*, int);
    void set_game_state(GameState state);
    virtual int get_demo_scenario();
    virtual void main_loop_iteration_complete(uint32_t game_time);

  private:
    int _level;
};

#endif  // ANTARES_TEST_VIDEO_DRIVER_HPP_

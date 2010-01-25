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

#include "sfz/String.hpp"
#include "CardStack.hpp"
#include "VideoDriver.hpp"

namespace antares {

class TestingVideoDriver : public VideoDriver {
  public:
    TestingVideoDriver(const sfz::StringPiece& output_dir);

    virtual void send_event(EventRecord);
    virtual bool button();
    virtual Point get_mouse();
    virtual void get_keys(KeyMap keys);
    virtual int ticks();
    virtual void main_loop_iteration_complete(uint32_t);
    virtual void set_game_state(GameState state);

    virtual void loop(CardStack* stack);

  protected:
    GameState state() const;
    sfz::StringPiece output_dir() const;

  private:
    int _current_time;
    GameState _state;
    const sfz::String _output_dir;
};

class MainScreenVideoDriver : public TestingVideoDriver {
  public:
    MainScreenVideoDriver(const sfz::StringPiece& output_dir);

    virtual bool wait_next_event(EventRecord*, double);
    virtual int get_demo_scenario();

  public:
    bool _key_down;
};

class MissionBriefingVideoDriver : public TestingVideoDriver {
  public:
    MissionBriefingVideoDriver(const sfz::StringPiece& output_dir, int level);

    virtual bool wait_next_event(EventRecord* evt, double);
    virtual int get_demo_scenario();
    virtual void get_keys(KeyMap keys);

  private:
    const int _level;
    int _briefing_num;
    bool _key_down;
};

class DemoVideoDriver : public TestingVideoDriver {
  public:
    DemoVideoDriver(const sfz::StringPiece& output_dir, int level);

    virtual bool wait_next_event(EventRecord*, double);
    virtual int get_demo_scenario();
    virtual void main_loop_iteration_complete(uint32_t game_time);

  private:
    int _level;
    bool _started_replay;
    bool _key_down;
};

}  // namespace antares

#endif  // ANTARES_TEST_VIDEO_DRIVER_HPP_

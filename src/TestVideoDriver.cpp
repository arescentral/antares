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

#include "TestVideoDriver.hpp"

#include <strings.h>
#include "AresPreferences.hpp"
#include "FakeDrawing.hpp"
#include "Fakes.hpp"

TestingVideoDriver::TestingVideoDriver()
        : _current_time(0) { }

void TestingVideoDriver::send_event(EventRecord) { }

bool TestingVideoDriver::button() { return false; }

void TestingVideoDriver::get_keys(KeyMap keys) { bzero(keys, sizeof(KeyMap)); }

int TestingVideoDriver::ticks() {
    if (_state == PLAY_GAME) {
        return _current_time;
    } else {
        return ++_current_time;
    }
}

void TestingVideoDriver::main_loop_iteration_complete(uint32_t) {
    ++_current_time;
}

void TestingVideoDriver::set_game_state(GameState state) {
    _state = state;
}

GameState TestingVideoDriver::state() const { return _state; }

bool MainScreenVideoDriver::wait_next_event(EventRecord*, int) {
    if (state() == MAIN_SCREEN_INTERFACE) {
        if (!get_output_dir().empty()) {
            DumpTo(get_output_dir() + "/main-screen.bin");
        }
        exit(0);
    }
    return true;
}

int MainScreenVideoDriver::get_demo_scenario() { return -1; }

MissionBriefingVideoDriver::MissionBriefingVideoDriver(int level)
        : _level(level),
          _briefing_num(0) { }

bool MissionBriefingVideoDriver::wait_next_event(EventRecord* evt, int) {
    switch (state()) {
      case MAIN_SCREEN_INTERFACE:
        {
            evt->what = autoKey;
            evt->message = 0x0100;  // S
            globals()->gPreferencesData->startingLevel = _level;
        }
        break;
      case SELECT_LEVEL_INTERFACE:
        {
            evt->what = autoKey;
            evt->message = 0x2400;  // RTRN
            if (!get_output_dir().empty()) {
                DumpTo(get_output_dir() + "/select-level.bin");
            }
        }
        break;
      case MISSION_INTERFACE:
        {
            char path[64];
            sprintf(path, "/mission-%u.bin", _briefing_num);
            if (!get_output_dir().empty()) {
                DumpTo(get_output_dir() + path);
            }
            ++_briefing_num;
            if (_briefing_num >= 9) {
                exit(0);
            } else {
                evt->what = autoKey;
                evt->message = 0x7C00;  // RGHT
            }
        }
        break;
      default:
        break;
    };
    return true;
}

int MissionBriefingVideoDriver::get_demo_scenario() { return -1; }

DemoVideoDriver::DemoVideoDriver(int level)
        : _level(level) {
    if (level != 0 && level != 5 && level != 23) {
        fprintf(stderr, "Only have demos of levels 0, 5, and 23; not %d.\n", level);
        exit(1);
    }
    if (!get_output_dir().empty()) {
        SoundDriver::set_driver(new LogSoundDriver(get_output_dir() + "/sound.log"));
    }
}

bool DemoVideoDriver::wait_next_event(EventRecord*, int) { return true; }

int DemoVideoDriver::get_demo_scenario() {
    return _level;
}

void DemoVideoDriver::main_loop_iteration_complete(uint32_t game_time) {
    TestingVideoDriver::main_loop_iteration_complete(game_time);
    if (game_time % 60 == 1) {
        char path[64];
        uint32_t seconds = game_time / 60;
        sprintf(path, "/screens/%03um%02u.bin", seconds / 60, seconds % 60);
        if (!get_output_dir().empty()) {
            DumpTo(get_output_dir() + path);
        }
    }
}

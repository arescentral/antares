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

#include "sfz/Format.hpp"
#include "sfz/Formatter.hpp"
#include "AresPreferences.hpp"
#include "Card.hpp"
#include "Error.hpp"
#include "FakeDrawing.hpp"
#include "Fakes.hpp"
#include "Ledger.hpp"
#include "Time.hpp"

using sfz::String;
using sfz::StringPiece;
using sfz::ascii_encoding;
using sfz::dec;
using sfz::format;

namespace antares {

TestingVideoDriver::TestingVideoDriver(const StringPiece& output_dir)
        : _current_time(0),
          _state(UNKNOWN),
          _output_dir(output_dir) { }

void TestingVideoDriver::send_event(EventRecord) { }

bool TestingVideoDriver::button() { return false; }

Point TestingVideoDriver::get_mouse() {
    return Point(320, 240);
}

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

void TestingVideoDriver::loop(CardStack* stack) {
    while (!stack->empty()) {
        EventRecord evt;
        double at;
        Card* card = stack->next_event(&at);
        if (wait_next_event(&evt, at - now_secs())) {
            stack->send(evt);
        } else if (card) {
            card->fire_timer();
        }
    }
}

GameState TestingVideoDriver::state() const { return _state; }

StringPiece TestingVideoDriver::output_dir() const {
    return _output_dir;
}

MainScreenVideoDriver::MainScreenVideoDriver(const StringPiece& output_dir)
        : TestingVideoDriver(output_dir),
          _key_down(false) { }

bool MainScreenVideoDriver::wait_next_event(EventRecord* evt, double) {
    if (state() == MAIN_SCREEN_INTERFACE) {
        if (_key_down) {
            evt->what = keyUp;
            _key_down = false;
        } else {
            if (!output_dir().empty()) {
                String out(output_dir());
                out.append("/main-screen.png", ascii_encoding());
                DumpTo(out);
            }
            evt->what = autoKey;
            _key_down = true;
        }
        evt->message = 0x0C00;  // Q
        return true;
    } else {
        return false;
    }
}

int MainScreenVideoDriver::get_demo_scenario() { return -1; }

MissionBriefingVideoDriver::MissionBriefingVideoDriver(const StringPiece& output_dir, int level)
        : TestingVideoDriver(output_dir),
          _level(level),
          _briefing_num(0),
          _key_down(false) { }

bool MissionBriefingVideoDriver::wait_next_event(EventRecord* evt, double) {
    switch (state()) {
      case MAIN_SCREEN_INTERFACE:
        {
            if (_key_down) {
                evt->what = keyUp;
                _key_down = false;
            } else {
                evt->what = autoKey;
                _key_down = true;
            }
            if (_briefing_num >= 9) {
                evt->message = 0x0C00;  // Q
            } else {
                evt->message = 0x0100;  // S
            }
            Ledger::ledger()->unlock_chapter(_level);
        }
        return true;
      case SELECT_LEVEL_INTERFACE:
        {
            if (_key_down) {
                evt->what = keyUp;
                _key_down = false;
            } else {
                if (!output_dir().empty()) {
                    String out(output_dir());
                    out.append("/select-level.png", ascii_encoding());
                    DumpTo(out);
                }
                evt->what = autoKey;
                _key_down = true;
            }
            evt->message = 0x2400;  // RTRN
        }
        return true;
      case MISSION_INTERFACE:
        {
            if (_key_down) {
                evt->what = keyUp;
                _key_down = false;
            } else {
                if (!output_dir().empty()) {
                    String out(output_dir());
                    format(&out, "/mission-{0}.png", _briefing_num);
                    DumpTo(out);
                }
                ++_briefing_num;
                evt->what = autoKey;
                _key_down = true;
            }
            if (_briefing_num >= 9) {
                evt->message = 0x3500;  // ESC
            } else {
                evt->message = 0x7C00;  // RGHT
            }
        }
        return true;
      default:
        return false;
    };
}

int MissionBriefingVideoDriver::get_demo_scenario() { return -1; }

void MissionBriefingVideoDriver::get_keys(KeyMap keys) {
    TestingVideoDriver::get_keys(keys);
    if (state() == MISSION_INTERFACE && _briefing_num >= 9 && keyDown) {
        keys[1] |= (0x01 << 13);
    }
}

DemoVideoDriver::DemoVideoDriver(const StringPiece& output_dir, int level)
        : TestingVideoDriver(output_dir),
          _level(level),
          _started_replay(false),
          _key_down(false) {
    if (level != 0 && level != 5 && level != 23) {
        fail("Only have demos of levels 0, 5, and 23; not %d.", level);
    }
}

bool DemoVideoDriver::wait_next_event(EventRecord* evt, double) {
    if (_started_replay && state() == MAIN_SCREEN_INTERFACE) {
        if (_key_down) {
            evt->what = keyUp;
            _key_down = false;
        } else {
            evt->what = autoKey;
            _key_down = true;
        }
        evt->message = 0x0C00;  // Q
        return true;
    } else {
        return false;
    }
}

int DemoVideoDriver::get_demo_scenario() {
    return _level;
}

void DemoVideoDriver::main_loop_iteration_complete(uint32_t game_time) {
    TestingVideoDriver::main_loop_iteration_complete(game_time);
    _started_replay = true;
    if (game_time % 60 == 1) {
        char path[64];
        uint32_t seconds = game_time / 60;
        sprintf(path, "/screens/%03um%02u.png", seconds / 60, seconds % 60);
        if (!output_dir().empty()) {
            String out(output_dir());
            format(&out, "/screens/{0}m{1}.png", dec(seconds / 60, 3), dec(seconds % 60, 2));
            DumpTo(out);
        }
    }
}

}  // namespace antares

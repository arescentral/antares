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

#include <fcntl.h>
#include <limits>
#include "sfz/BinaryWriter.hpp"
#include "sfz/Bytes.hpp"
#include "sfz/Exception.hpp"
#include "sfz/Format.hpp"
#include "sfz/Formatter.hpp"
#include "sfz/PosixFormatter.hpp"
#include "sfz/ScopedFd.hpp"
#include "sfz/SmartPtr.hpp"
#include "AresPreferences.hpp"
#include "BuildPix.hpp"
#include "Card.hpp"
#include "CardStack.hpp"
#include "Error.hpp"
#include "Event.hpp"
#include "FakeDrawing.hpp"
#include "Fakes.hpp"
#include "File.hpp"
#include "Ledger.hpp"
#include "PlayerInterface.hpp"
#include "SpaceObject.hpp"
#include "Time.hpp"

using sfz::Bytes;
using sfz::BytesBinaryWriter;
using sfz::Exception;
using sfz::ScopedFd;
using sfz::String;
using sfz::StringPiece;
using sfz::ascii_encoding;
using sfz::dec;
using sfz::format;
using sfz::posix_strerror;
using sfz::scoped_ptr;

namespace antares {

namespace {

enum Keys {
    KEY_K = 0x28,
    KEY_O = 0x1f,
    KEY_S = 0x01,
    KEY_Q = 0x0c,
    KEY_RETURN = 0x24,
    KEY_ESCAPE = 0x35,
    KEY_RIGHT = 0x7C,
};

}  // namespace

TestingVideoDriver::TestingVideoDriver(const StringPiece& output_dir)
        : _current_time(0),
          _state(UNKNOWN),
          _output_dir(output_dir) { }

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
        double at = stack->top()->next_timer();
        if (at == 0.0) {
            at = std::numeric_limits<double>::infinity();
        }
        scoped_ptr<Event> event(wait_next_event(at - now_secs()));
        if (event.get()) {
            event->send(stack->top());
        } else if (at != std::numeric_limits<double>::infinity()) {
            stack->top()->fire_timer();
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

Event* MainScreenVideoDriver::wait_next_event(double) {
    if (state() == MAIN_SCREEN_INTERFACE) {
        if (_key_down) {
            _key_down = false;
            return new KeyUpEvent(KEY_Q);
        } else {
            if (!output_dir().empty()) {
                String out(output_dir());
                out.append("/main-screen.png", ascii_encoding());
                DumpTo(out);
            }
            _key_down = true;
            return new KeyDownEvent(KEY_Q);
        }
    }
    return NULL;
}

int MainScreenVideoDriver::get_demo_scenario() { return -1; }

MissionBriefingVideoDriver::MissionBriefingVideoDriver(const StringPiece& output_dir, int level)
        : TestingVideoDriver(output_dir),
          _level(level),
          _briefing_num(0),
          _key_down(false) { }

Event* MissionBriefingVideoDriver::wait_next_event(double) {
    scoped_ptr<Event> event;
    switch (state()) {
      case MAIN_SCREEN_INTERFACE:
        {
            Ledger::ledger()->unlock_chapter(_level);
            uint32_t key = (_briefing_num >= 9) ? KEY_Q : KEY_S;
            if (_key_down) {
                _key_down = false;
                return new KeyUpEvent(key);
            } else {
                _key_down = true;
                return new KeyDownEvent(key);
            }
        }
        break;

      case SELECT_LEVEL_INTERFACE:
        {
            if (_key_down) {
                _key_down = false;
                return new KeyUpEvent(KEY_RETURN);
            } else {
                if (!output_dir().empty()) {
                    String out(output_dir());
                    out.append("/select-level.png", ascii_encoding());
                    DumpTo(out);
                }
                _key_down = true;
                return new KeyDownEvent(KEY_RETURN);
            }
        }
        break;

      case MISSION_INTERFACE:
        {
            uint32_t key = (_briefing_num >= 8) ? KEY_ESCAPE : KEY_RIGHT;
            if (_key_down) {
                _key_down = false;
                return new KeyUpEvent(key);
            } else {
                if (!output_dir().empty()) {
                    String out(output_dir());
                    format(&out, "/mission-{0}.png", _briefing_num);
                    DumpTo(out);
                }
                ++_briefing_num;
                _key_down = true;
                return new KeyDownEvent(key);
            }
        }
        break;

      default:
        break;
    };
    return NULL;
}

int MissionBriefingVideoDriver::get_demo_scenario() { return -1; }

DemoVideoDriver::DemoVideoDriver(const StringPiece& output_dir, int level)
        : TestingVideoDriver(output_dir),
          _level(level),
          _started_replay(false),
          _key_down(false) {
    if (level != 0 && level != 5 && level != 23) {
        fail("Only have demos of levels 0, 5, and 23; not %d.", level);
    }
}

Event* DemoVideoDriver::wait_next_event(double) {
    if (_started_replay && state() == MAIN_SCREEN_INTERFACE) {
        if (_key_down) {
            _key_down = false;
            return new KeyUpEvent(KEY_Q);
        } else {
            _key_down = true;
            return new KeyDownEvent(KEY_Q);
        }
    }
    return NULL;
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

OptionsVideoDriver::OptionsVideoDriver(const StringPiece& output_dir)
        : TestingVideoDriver(output_dir),
          _key_tab(0),
          _key_down(false) { }

Event* OptionsVideoDriver::wait_next_event(double) {
    switch (state()) {
      case MAIN_SCREEN_INTERFACE:
        {
            uint32_t key = (_key_tab >= 5) ? KEY_Q : KEY_O;
            if (_key_down) {
                _key_down = false;
                return new KeyUpEvent(key);
            } else {
                _key_down = true;
                return new KeyDownEvent(key);
            }
        }
        break;

      case OPTIONS_INTERFACE:
        {
            if (_key_down) {
                _key_down = false;
                return new KeyUpEvent(KEY_K);
            } else {
                if (!output_dir().empty()) {
                    String out(output_dir());
                    out.append("/options.png", ascii_encoding());
                    DumpTo(out);
                }
                _key_down = true;
                return new KeyDownEvent(KEY_K);
            }
        }
        break;

      case KEY_CONTROL_INTERFACE:
        {
            if (_key_down) {
                _key_down = false;
                return new MouseUpEvent(0, get_mouse());
            } else {
                if (!output_dir().empty()) {
                    String out(output_dir());
                    format(&out, "/key-control-{0}.png", _key_tab);
                    DumpTo(out);
                }
                ++_key_tab;
                _key_down = true;
                return new MouseDownEvent(0, get_mouse());
            }
        }
        break;

      default:
        break;
    };
    return NULL;
}

Point OptionsVideoDriver::get_mouse() {
    if (_key_tab < 5) {
        return Point(100 * (1 + _key_tab), 50);
    } else if (_key_tab == 5) {
        return Point(550, 430);
    } else {
        throw Exception("_key_tab == {0}", _key_tab);
    }
}

int OptionsVideoDriver::get_demo_scenario() { return -1; }

ObjectDataVideoDriver::ObjectDataVideoDriver(const StringPiece& output_dir)
        : TestingVideoDriver(output_dir),
          _key_down(false) { }

Event* ObjectDataVideoDriver::wait_next_event(double) {
    if (state() == MAIN_SCREEN_INTERFACE) {
        if (_key_down) {
            _key_down = false;
            return new KeyUpEvent(KEY_Q);
        } else {
            if (!output_dir().empty()) {
                for (int i = 0; i < globals()->maxBaseObject; ++i) {
                    int pict_id = gBaseObjectData.get()[i].pictPortraitResID;
                    if (pict_id <= 0) {
                        continue;
                    }

                    String path;
                    format(&path, "{0}/{1}.txt", output_dir(), dec(pict_id, 5));
                    ScopedFd fd(open_path(path, O_WRONLY | O_CREAT, 0644));
                    if (fd.get() < 0) {
                        throw Exception("open: {0}: {1}", path, posix_strerror());
                    }

                    String data;
                    CreateObjectDataText(&data, i);
                    print(fd.get(), "{0}", data);
                }
            }
            _key_down = true;
            return new KeyDownEvent(KEY_Q);
        }
    }
    return NULL;
}

int ObjectDataVideoDriver::get_demo_scenario() { return -1; }

BuildPixVideoDriver::BuildPixVideoDriver(const StringPiece& output_dir)
        : TestingVideoDriver(output_dir),
          _key_down(false) { }

Event* BuildPixVideoDriver::wait_next_event(double) {
    if (state() == MAIN_SCREEN_INTERFACE) {
        if (_key_down) {
            _key_down = false;
            return new KeyUpEvent(KEY_Q);
        } else {
            if (!output_dir().empty()) {
                const int text_count = 13;
                const int text[text_count] = {
                    3020, 3025, 3080, 3081, 3120, 3211, 4063, 4509, 4606, 5600, 6500, 6501, 10199,
                };
                const int width[text_count] = {
                    450, 450, 450, 450, 450, 450, 450, 450, 450, 450, 540, 450, 450,
                };
                for (int i = 0; i < text_count; ++i) {
                    scoped_ptr<PixMap> pix(build_pix(text[i], width[i]));

                    String path;
                    format(&path, "{0}/{1}.png", output_dir(), dec(text[i], 5));
                    ScopedFd fd(open_path(path, O_WRONLY | O_CREAT, 0644));
                    if (fd.get() < 0) {
                        throw Exception("open: {0}: {1}", path, posix_strerror());
                    }

                    Bytes data;
                    BytesBinaryWriter(&data).write(*pix);
                    write(fd.get(), data.data(), data.size());
                }
            }
            _key_down = true;
            return new KeyDownEvent(KEY_Q);
        }
    }
    return NULL;
}

int BuildPixVideoDriver::get_demo_scenario() { return -1; }

}  // namespace antares

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
#include "sfz/sfz.hpp"
#include "BuildPix.hpp"
#include "Card.hpp"
#include "CardStack.hpp"
#include "Error.hpp"
#include "Event.hpp"
#include "FakeDrawing.hpp"
#include "Fakes.hpp"
#include "Ledger.hpp"
#include "PlayerInterface.hpp"
#include "Preferences.hpp"
#include "PrefsDriver.hpp"
#include "SpaceObject.hpp"
#include "Time.hpp"

using sfz::Bytes;
using sfz::Exception;
using sfz::ScopedFd;
using sfz::String;
using sfz::StringPiece;
using sfz::dec;
using sfz::format;
using sfz::open;
using sfz::posix_strerror;
using sfz::scoped_ptr;
using sfz::write;

namespace utf8 = sfz::utf8;

namespace antares {

namespace {

class TestPrefsDriver : public PrefsDriver {
  public:
    TestPrefsDriver() { }

    virtual void load(Preferences* preferences) {
        preferences->copy(_saved);
    }

    virtual void save(const Preferences& preferences) {
        _saved.copy(preferences);
    }

  private:
    Preferences _saved;

    DISALLOW_COPY_AND_ASSIGN(TestPrefsDriver);
};

}  // namespace

TestingVideoDriver::TestingVideoDriver(const StringPiece& output_dir)
        : _current_time(0),
          _state(UNKNOWN),
          _output_dir(output_dir) {
    PrefsDriver::set_driver(new TestPrefsDriver);
}

bool TestingVideoDriver::button() { return false; }

Point TestingVideoDriver::get_mouse() {
    return Point(320, 240);
}

void TestingVideoDriver::get_keys(KeyMap* keys) {
    keys->clear();
}

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
            return new KeyUpEvent(Keys::Q);
        } else {
            if (!output_dir().empty()) {
                String out(format("{0}/main-screen.png", output_dir()));
                DumpTo(out);
            }
            _key_down = true;
            return new KeyDownEvent(Keys::Q);
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
            uint32_t key = (_briefing_num >= 9) ? Keys::Q : Keys::S;
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
                return new KeyUpEvent(Keys::RETURN);
            } else {
                if (!output_dir().empty()) {
                    String out(format("{0}/select-level.png", output_dir()));
                    DumpTo(out);
                }
                _key_down = true;
                return new KeyDownEvent(Keys::RETURN);
            }
        }
        break;

      case MISSION_INTERFACE:
        {
            uint32_t key = (_briefing_num >= 8) ? Keys::ESCAPE : Keys::RIGHT_ARROW;
            if (_key_down) {
                _key_down = false;
                return new KeyUpEvent(key);
            } else {
                if (!output_dir().empty()) {
                    String out(format("{0}/mission-{1}.png", output_dir(), _briefing_num));
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
    Preferences preferences;
    preferences.set_volume(8);
    PrefsDriver::driver()->save(preferences);
}

Event* DemoVideoDriver::wait_next_event(double) {
    if (_started_replay && state() == MAIN_SCREEN_INTERFACE) {
        if (_key_down) {
            _key_down = false;
            return new KeyUpEvent(Keys::Q);
        } else {
            _key_down = true;
            return new KeyDownEvent(Keys::Q);
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
            String out(format("{0}/screens/{1}m{2}.png",
                        output_dir(), dec(seconds / 60, 3), dec(seconds % 60, 2)));
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
            uint32_t key = (_key_tab >= 5) ? Keys::Q : Keys::O;
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
                return new KeyUpEvent(Keys::K);
            } else {
                if (!output_dir().empty()) {
                    String out(format("{0}/options.png", output_dir()));
                    DumpTo(out);
                }
                _key_down = true;
                return new KeyDownEvent(Keys::K);
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
                    String out(format("{0}/key-control-{1}.png", output_dir(), _key_tab));
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
        throw Exception(format("_key_tab == {0}", _key_tab));
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
            return new KeyUpEvent(Keys::Q);
        } else {
            if (!output_dir().empty()) {
                for (int i = 0; i < globals()->maxBaseObject; ++i) {
                    int pict_id = gBaseObjectData.get()[i].pictPortraitResID;
                    if (pict_id <= 0) {
                        continue;
                    }

                    String path(format("{0}/{1}.txt", output_dir(), dec(pict_id, 5)));
                    ScopedFd fd(open(path, O_WRONLY | O_CREAT, 0644));

                    String data;
                    CreateObjectDataText(&data, i);
                    write(&fd, utf8::encode(data));
                }
            }
            _key_down = true;
            return new KeyDownEvent(Keys::Q);
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
            return new KeyUpEvent(Keys::Q);
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

                    String path(format("{0}/{1}.png", output_dir(), dec(text[i], 5)));
                    ScopedFd fd(open(path, O_WRONLY | O_CREAT, 0644));

                    Bytes data;
                    write(&data, *pix);
                    ::write(fd.get(), data.data(), data.size());
                }
            }
            _key_down = true;
            return new KeyDownEvent(Keys::Q);
        }
    }
    return NULL;
}

int BuildPixVideoDriver::get_demo_scenario() { return -1; }

}  // namespace antares

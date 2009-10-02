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

#include "Fakes.hpp"

#include <sys/time.h>
#include <getopt.h>
#include <queue>
#include <string>

#include "AresPreferences.hpp"
#include "FakeDrawing.hpp"
#include "FakeHandles.hpp"
#include "FakeMath.hpp"
#include "FakeSounds.hpp"
#include "File.hpp"
#include "Threading.hpp"
#include "VncServer.hpp"

namespace {

std::string output_dir;

class RealMode : public Mode {
  public:
    RealMode()
            : _start_time(usecs()),
              _button(false) { }

    virtual void send_event(EventRecord evt) {
        {
            MutexLock lock(&_mu);
            if (evt.what == mouseDown) {
                _button = true;
            } else if (evt.what == mouseUp) {
                _button = false;
            } else if (evt.what == autoKey) {
            }
            _event_queue.push(evt);
        }
        _have_event.broadcast();
    }

    virtual bool button() {
        MutexLock lock(&_mu);
        return _button;
    }

    virtual void get_keys(KeyMap keys) {
        MutexLock lock(&_mu);
        bzero(keys, sizeof(KeyMap));
    }

    virtual bool wait_next_event(EventRecord* evt, int sleep) {
        MutexLock lock(&_mu);
        timeval tv;
        gettimeofday(&tv, NULL);
        int64_t timeout = (sleep + tv.tv_sec) * 1000000000ll + tv.tv_usec;
        while (_event_queue.empty()) {
            if (!_mu.await_with_timeout(&_have_event, timeout)) {
                return false;
            }
        }
        *evt = _event_queue.front();
        if (evt->what == mouseDown) {
            printf("received mouse down at (%d, %d)\n", evt->where.h, evt->where.v);
        } else if (evt->what == mouseUp) {
            printf("received mouse up at (%d, %d)\n", evt->where.h, evt->where.v);
        }
        _event_queue.pop();
        return true;
    }

    virtual void set_game_state(GameState) { }

    virtual int get_demo_scenario() {
        int levels[] = { 0, 5, 23 };
        return levels[rand() % 3];
    }

    virtual void main_loop_iteration_complete(uint32_t) { }

    virtual int ticks() {
        return (usecs() - _start_time) * 60 / 1000000;
    }

  private:
    int64_t usecs() const {
        timeval tv;
        gettimeofday(&tv, NULL);
        return tv.tv_sec * 1000000ll + tv.tv_usec;
    }

    Mutex _mu;
    Condition _have_event;
    const int64_t _start_time;
    bool _button;
    std::queue<EventRecord> _event_queue;
};

class TestingMode : public Mode {
  public:
    TestingMode()
            : _current_time(0) { }

    virtual void send_event(EventRecord) { }

    virtual bool button() { return false; }
    virtual void get_keys(KeyMap keys) { bzero(keys, sizeof(KeyMap)); }

    virtual int ticks() {
        if (globals()->gGameTime > 0) {
            return _current_time + globals()->gGameTime;
        } else {
            return ++_current_time;
        }
    }

  private:
    int _current_time;
};

class MainScreenMode : public TestingMode {
  public:
    MainScreenMode()
            : _ready(false) { }

    virtual bool wait_next_event(EventRecord*, int) {
        if (_ready) {
            if (!output_dir.empty()) {
                DumpTo(output_dir + "/main-screen.bin");
            }
            exit(0);
        }
        return true;
    }

    virtual void set_game_state(GameState state) {
        if (state == MAIN_SCREEN_INTERFACE) {
            _ready = true;
        }
    }

    virtual int get_demo_scenario() { return -1; }
    virtual void main_loop_iteration_complete(uint32_t) { }

  public:
    bool _ready;
};

class MissionBriefingMode : public TestingMode {
  public:
    MissionBriefingMode(int level)
            : _level(level),
              _briefing_num(0) { }

    virtual bool wait_next_event(EventRecord* evt, int) {
        switch (_state) {
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
                if (!output_dir.empty()) {
                    DumpTo(output_dir + "/select-level.bin");
                }
            }
            break;
          case MISSION_INTERFACE:
            {
                char path[64];
                sprintf(path, "/mission-%u.bin", _briefing_num);
                if (!output_dir.empty()) {
                    DumpTo(output_dir + path);
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

    virtual void set_game_state(GameState state) {
        _state = state;
    }

    virtual int get_demo_scenario() { return -1; }
    virtual void main_loop_iteration_complete(uint32_t) { }

  public:
    const int _level;
    int _briefing_num;
    GameState _state;
};

class DemoMode : public TestingMode {
  public:
    DemoMode(int level)
            : _level(level) {
        if (level != 0 && level != 5 && level != 23) {
            fprintf(stderr, "Only have demos of levels 0, 5, and 23; not %d.\n", level);
            exit(1);
        }
        if (!output_dir.empty()) {
            SoundDriver::set_driver(new LogSoundDriver(output_dir + "/sound.log"));
        }
    }

    virtual bool wait_next_event(EventRecord*, int) { return true; }
    virtual void set_game_state(GameState) { }

    virtual int get_demo_scenario() {
        return _level;
    }

    virtual void main_loop_iteration_complete(uint32_t game_time) {
        if (game_time % 60 == 1) {
            char path[64];
            uint32_t seconds = game_time / 60;
            sprintf(path, "/screens/%03um%02u.bin", seconds / 60, seconds % 60);
            if (!output_dir.empty()) {
                DumpTo(output_dir + path);
            }
        }
    }

  public:
    int _level;
};

scoped_ptr<Mode> mode;

}  // namespace

Mode* Mode::mode() {
    return ::mode.get();
}

void Mode::set_mode(Mode* mode) {
    ::mode.reset(mode);
}

int GetDemoScenario() {
    return mode->get_demo_scenario();
}

void ModalDialog(void*, short* item) {
    *item = 1;
}

void SetGameState(GameState state) {
    mode->set_game_state(state);
}

void MainLoopIterationComplete(uint32_t game_time) {
    mode->main_loop_iteration_complete(game_time);
}

bool WaitNextEvent(long mask, EventRecord* evt, unsigned long sleep, Rgn** mouseRgn) {
    static_cast<void>(mask);
    static_cast<void>(mouseRgn);
    evt->what = 0;
    return mode->wait_next_event(evt, sleep);
}

bool Button() {
    return mode->button();
}

void GetKeys(KeyMap keys) {
    mode->get_keys(keys);
}

int TickCount() {
    return mode->ticks();
}

void Microseconds(uint64_t* wide) {
    *wide = TickCount();
}

void StringToNum(unsigned char* p_str, long* value) {
    size_t len = *p_str;
    char c_str[256];
    memcpy(c_str, p_str + 1, len);
    c_str[len] = '\0';

    char* end;
    *value = strtol(c_str, &end, 10);
    assert(end == c_str + len);
}

void usage(const char* bin) {
    fprintf(stderr,
            "usage: %s [-m|--mode=<mode>] [<options>]\n"
            "options:\n"
            "    -l|--level=<int>   choose a level to use in the given mode\n"
            "    -o|--output=<dir>  directory to save dumps to\n"
            "    -w|--width=<int>   width of screen (default: 640)\n"
            "    -h|--height=<int>  height of screen (default: 480)\n"
            "modes:\n"
            "    main-screen        dumps the main screen, then exits\n"
            "    mission-briefing   dumps the mission briefing screens for <level>\n"
            "    demo               runs the demo for <level>\n",
            bin);
    exit(1);
}

int string_to_int(const char* string) {
    int value;
    char* end = NULL;
    if (string && *string) {
        value = strtol(string, &end, 10);
    }
    if (!string || end != string + strlen(string)) {
        fprintf(stderr, "Couldn't parse '%s' as an integer\n", string);
        exit(1);
    }
    return value;
}

void FakeInit(int argc, char* const* argv) {
    const char* bin = argv[0];
    int mode_int = -1;
    int level = -1;
    int width = 640;
    int height = 480;
    option longopts[] = {
        { "mode",   required_argument,  NULL,   'm' },
        { "level",  required_argument,  NULL,   'l' },
        { "output", required_argument,  NULL,   'o' },
        { "width",  required_argument,  NULL,   'w' },
        { "height", required_argument,  NULL,   'h' },
        { NULL,     0,                  NULL,   0 }
    };

    char ch;
    while ((ch = getopt_long(argc, argv, "m:l:o:w:h:", longopts, NULL)) != -1) {
        switch (ch) {
          case 'm':
            {
                std::string arg = optarg;
                if (arg == "main-screen") {
                    mode_int = 0;
                } else if (arg == "mission-briefing") {
                    mode_int = 1;
                } else if (arg == "demo") {
                    mode_int = 2;
                } else if (arg == "real") {
                    mode_int = 3;
                } else {
                    fprintf(stderr, "%s: unknown mode '%s'\n", bin, optarg);
                    usage(bin);
                }
            }
            break;
          case 'l':
            level = string_to_int(optarg);
            break;
          case 'o':
            output_dir = optarg;
            break;
          case 'w':
            width = string_to_int(optarg);
            break;
          case 'h':
            height = string_to_int(optarg);
            break;
          default:
            fprintf(stderr, "%s: unknown argument %s\n", bin, argv[optind]);
            usage(bin);
            break;
        }
    }

    argc -= optind;
    argv += optind;
    if (argc != 0) {
        fprintf(stderr, "%s: too many arguments\n", bin);
        usage(bin);
    }

    switch (mode_int) {
      case 0:
        Mode::set_mode(new MainScreenMode());
        break;
      case 1:
        Mode::set_mode(new MissionBriefingMode(level));
        break;
      case 2:
        Mode::set_mode(new DemoMode(level));
        break;
      case 3:
        Mode::set_mode(new RealMode);
        break;
      default:
        fprintf(stderr, "%s: must specify --mode\n", bin);
        usage(bin);
        break;
    }

    if (!output_dir.empty()) {
        MakeDirs(output_dir, 0755);
    }

    FakeDrawingInit(width, height);
    FakeHandlesInit();
    FakeMathInit();
    FakeSoundsInit();
    VncServerInit();
}

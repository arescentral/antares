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

#include <getopt.h>
#include <string>

#include "AresPreferences.hpp"
#include "FakeDrawing.hpp"
#include "FakeHandles.hpp"
#include "FakeMath.hpp"
#include "FakeSounds.hpp"
#include "FakeTime.hpp"
#include "File.hpp"
#include "VncServer.hpp"

bool mission_briefing_test = false;
bool main_screen_test = false;
int briefing_num = 0;

int level = 23;
int GetDemoScenario() {
    return level;
}

std::string output_dir;
std::string GetOutputDir() {
    return output_dir;
}

void ModalDialog(void*, short* item) {
    *item = 1;
}

GameState game_state = UNKNOWN;
void SetGameState(GameState state) {
    game_state = state;
}

void MainLoopIterationComplete(uint32_t game_time) {
    if (game_time % 60 == 1) {
        char path[64];
        uint32_t seconds = game_time / 60;
        sprintf(path, "/screens/%03um%02u.bin", seconds / 60, seconds % 60);
        DumpTo(GetOutputDir() + path);
    }
}

bool WaitNextEvent(long mask, EventRecord* evt, unsigned long sleep, Rgn** mouseRgn) {
    static_cast<void>(mask);
    static_cast<void>(sleep);
    static_cast<void>(mouseRgn);
    switch (game_state) {
    case MAIN_SCREEN_INTERFACE:
        {
            if (mission_briefing_test) {
                evt->what = autoKey;
                evt->message = 0x0100;  // S
                globals()->gPreferencesData->startingLevel = level;
            } else if (main_screen_test) {
                DumpTo(GetOutputDir() + "/main-screen.bin");
                exit(0);
            } else {
                evt->what = 0;
            }
        }
        break;
    case SELECT_LEVEL_INTERFACE:
        {
            evt->what = autoKey;
            evt->message = 0x2400;  // RTRN
            DumpTo(GetOutputDir() + "/select-level.bin");
        }
        break;
    case MISSION_INTERFACE:
        {
            char path[64];
            sprintf(path, "/mission-%u.bin", briefing_num);
            DumpTo(GetOutputDir() + path);
            ++briefing_num;
            if (briefing_num >= 9) {
                exit(0);
            } else {
                evt->what = autoKey;
                evt->message = 0x7C00;  // RGHT
            }
        }
        break;
    default:
        {
            evt->what = 0;
        }
        break;
    }
    return true;
}

bool Button() {
    return false;
}

void GetKeys(KeyMap keys) {
    bzero(keys, sizeof(KeyMap));
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
    int mode = -1;
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
                    mode = 0;
                } else if (arg == "mission-briefing") {
                    mode = 1;
                } else if (arg == "demo") {
                    mode = 2;
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

    switch (mode) {
      case 0:
        main_screen_test = true;
        break;
      case 1:
        mission_briefing_test = true;
        break;
      case 2:
        {
            if (level != 0 && level != 5 && level != 23) {
                fprintf(stderr, "Only have demos of levels 0, 5, and 23; not %d.\n", level);
                exit(1);
            }
            SetDoSounds(true);
        }
        break;
      default:
        fprintf(stderr, "%s: must specify --mode\n", bin);
        usage(bin);
        break;
    }

    if (output_dir.empty()) {
        fprintf(stderr, "%s: must specify --output\n", bin);
        usage(bin);
    }

    MakeDirs(output_dir, 0755);

    FakeDrawingInit(width, height);
    FakeHandlesInit();
    FakeMathInit();
    FakeSoundsInit();
    FakeTimeInit();
    VncServerInit();
}

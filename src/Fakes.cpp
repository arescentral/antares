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

int demo_scenario = 23;
int GetDemoScenario() {
    return demo_scenario;
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
                (*gAresGlobal->gPreferencesData)->startingLevel = 22;
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
            DumpTo(GetOutputDir() + "/screens/select-level.bin");
        }
        break;
    case MISSION_INTERFACE:
        {
            char path[64];
            sprintf(path, "/screens/mission-%u.bin", briefing_num);
            DumpTo(GetOutputDir() + path);
            ++briefing_num;
            if (briefing_num >= 9) {
                exit(1);
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
    strncpy(c_str, reinterpret_cast<char*>(p_str + 1), len);
    c_str[len] = '\0';

    char* end;
    *value = strtol(c_str, &end, 10);
    assert(end == c_str + len);
}

void Usage() {
    fprintf(stderr, "usage: ./Antares space-race <dump-prefix>\n"
                    "       ./Antares the-stars-have-ears <dump-prefix>\n"
                    "       ./Antares while-the-iron-is-hot <dump-prefix>\n"
                    "       ./Antares main-screen <dump-prefix>\n"
                    "       ./Antares mission-briefing <dump-prefix>\n");
    exit(1);
}

void FakeInit(int argc, const char** argv) {
    if (argc == 3) {
        std::string demo = argv[1];
        output_dir = argv[2];
        if (demo == "space-race") {
            demo_scenario = 23;
            SetDoSounds(true);
        } else if (demo == "the-stars-have-ears") {
            demo_scenario = 0;
            SetDoSounds(true);
        } else if (demo == "while-the-iron-is-hot") {
            demo_scenario = 5;
            SetDoSounds(true);
        } else if (demo == "main-screen") {
            main_screen_test = true;
        } else if (demo == "mission-briefing") {
            mission_briefing_test = true;
        } else {
            Usage();
        }
    } else {
        Usage();
    }

    MakeDirs(output_dir, 0755);

    FakeDrawingInit();
    FakeHandlesInit();
    FakeMathInit();
    FakeSoundsInit();
    FakeTimeInit();
    VncServerInit();
}

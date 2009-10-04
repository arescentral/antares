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

#include "AresMain.hpp"
#include "AresPreferences.hpp"
#include "FakeDrawing.hpp"
#include "FakeHandles.hpp"
#include "FakeMath.hpp"
#include "FakeSounds.hpp"
#include "File.hpp"
#include "TestVideoDriver.hpp"
#include "Threading.hpp"
#include "VideoDriver.hpp"
#include "VncServer.hpp"

namespace {

std::string output_dir;

}  // namespace

const std::string& get_output_dir() {
    return output_dir;
}

int GetDemoScenario() {
    return VideoDriver::driver()->get_demo_scenario();
}

void ModalDialog(void*, short* item) {
    *item = 1;
}

bool WaitNextEvent(long mask, EventRecord* evt, unsigned long sleep, Rgn** mouseRgn) {
    static_cast<void>(mask);
    static_cast<void>(mouseRgn);
    evt->what = 0;
    return VideoDriver::driver()->wait_next_event(evt, sleep);
}

void GetMouse(Point* point) {
    *point = VideoDriver::driver()->get_mouse();
}

bool Button() {
    return VideoDriver::driver()->button();
}

void GetKeys(KeyMap keys) {
    VideoDriver::driver()->get_keys(keys);
}

int TickCount() {
    return VideoDriver::driver()->ticks();
}

void Microseconds(uint64_t* wide) {
    *wide = 16667 * TickCount();
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
            "usage: %s [-v|--video-driver=<driver>] [<options>]\n"
            "options:\n"
            "    -l|--level=<int>   choose a level to use in the given mode\n"
            "    -o|--output=<dir>  directory to save dumps to\n"
            "    -w|--width=<int>   width of screen (default: 640)\n"
            "    -h|--height=<int>  height of screen (default: 480)\n"
            "video drivers:\n"
            "    main-screen        dumps the main screen, then exits\n"
            "    mission-briefing   dumps the mission briefing screens for <level>\n"
            "    demo               runs the demo for <level>\n"
            "    vnc                exposes the video over VNC\n",
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
    int video_int = -1;
    int level = -1;
    int width = 640;
    int height = 480;
    option longopts[] = {
        { "video-driver",   required_argument,  NULL,   'v' },
        { "level",          required_argument,  NULL,   'l' },
        { "output",         required_argument,  NULL,   'o' },
        { "width",          required_argument,  NULL,   'w' },
        { "height",         required_argument,  NULL,   'h' },
        { NULL,             0,                  NULL,   0 }
    };

    char ch;
    while ((ch = getopt_long(argc, argv, "v:l:o:w:h:", longopts, NULL)) != -1) {
        switch (ch) {
          case 'v':
            {
                std::string arg = optarg;
                if (arg == "main-screen") {
                    video_int = 0;
                } else if (arg == "mission-briefing") {
                    video_int = 1;
                } else if (arg == "demo") {
                    video_int = 2;
                } else if (arg == "vnc") {
                    video_int = 3;
                } else {
                    fprintf(stderr, "%s: unknown video driver '%s'\n", bin, optarg);
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

    FakeDrawingInit(width, height);
    switch (video_int) {
      case 0:
        VideoDriver::set_driver(new MainScreenVideoDriver());
        break;
      case 1:
        VideoDriver::set_driver(new MissionBriefingVideoDriver(level));
        break;
      case 2:
        VideoDriver::set_driver(new DemoVideoDriver(level));
        break;
      case 3:
        VideoDriver::set_driver(new VncVideoDriver(5901));
        break;
      default:
        fprintf(stderr, "%s: must specify --video-driver\n", bin);
        usage(bin);
        break;
    }

    if (!output_dir.empty()) {
        MakeDirs(output_dir, 0755);
    }

    FakeHandlesInit();
    FakeMathInit();
    FakeSoundsInit();

    AresMain();
}

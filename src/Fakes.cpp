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
#include "Error.hpp"
#include "FakeDrawing.hpp"
#include "FakeSounds.hpp"
#include "File.hpp"
#include "TestVideoDriver.hpp"
#include "Threading.hpp"
#include "VideoDriver.hpp"
#include "VncServer.hpp"

namespace antares {

namespace {

std::string output_dir;

}  // namespace

const std::string& get_output_dir() {
    return output_dir;
}

int GetDemoScenario() {
    return VideoDriver::driver()->get_demo_scenario();
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

void StringToNum(unsigned char* p_str, long* value) {
    size_t len = *p_str;
    char c_str[256];
    memcpy(c_str, p_str + 1, len);
    c_str[len] = '\0';

    char* end;
    *value = strtol(c_str, &end, 10);
    check(end == c_str + len, "couldn't interpret '%s' as an integer", c_str);
}

int Munger(std::string* data, int pos, const unsigned char* search, size_t search_len,
        const unsigned char* replace, size_t replace_len) {
    std::string s(reinterpret_cast<const char*>(search), search_len);
    std::string r(reinterpret_cast<const char*>(replace), replace_len);
    std::string d = *data;
    std::string::size_type at = d.find(s, pos);
    if (at != std::string::npos) {
        data->resize(at);
        *data += r;
        *data += d.substr(at + s.size());
    }
    return at;
}

}  // namespace antares

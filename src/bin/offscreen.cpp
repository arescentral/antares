// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// Copyright (C) 2008-2011 Ares Central
//
// This file is part of Antares, a tactical space combat game.
//
// Antares is free software: you can redistribute it and/or modify it
// under the terms of the Lesser GNU General Public License as published
// by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Antares is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this program.  If not, see
// <http://www.gnu.org/licenses/>.

#include <sys/time.h>
#include <getopt.h>
#include <queue>
#include <sfz/sfz.hpp>

#include "config/ledger.hpp"
#include "config/preferences.hpp"
#include "game/main.hpp"
#include "sound/driver.hpp"
#include "ui/card.hpp"
#include "video/driver.hpp"
#include "video/offscreen-driver.hpp"

using sfz::Bytes;
using sfz::Exception;
using sfz::Optional;
using sfz::Rune;
using sfz::String;
using sfz::StringSlice;
using sfz::args::help;
using sfz::args::store;
using sfz::makedirs;
using sfz::make_linked_ptr;
using sfz::print;
using sfz::quote;
using sfz::scoped_ptr;
using sfz::string_to_int;

namespace args = sfz::args;
namespace io = sfz::io;
namespace utf8 = sfz::utf8;

namespace antares {
namespace {

const int32_t kScreenWidth = 640;
const int32_t kScreenHeight = 480;

void usage(const StringSlice& program_name);
void main_screen(OffscreenVideoDriver& driver);
void options(OffscreenVideoDriver& driver);
void mission_briefing(OffscreenVideoDriver& driver, Ledger& ledger);
void pause(OffscreenVideoDriver& driver);

void main(int argc, char* const* argv) {
    args::Parser parser(argv[0], "Simulates a game off-screen");

    String script;
    parser.add_argument("script", store(script))
        .metavar("main-screen|options|mission-briefing|pause")
        .required()
        .help("the script to execute");

    Optional<String> output_dir;
    parser.add_argument("-o", "--output", store(output_dir))
        .help("place output in this directory");
    parser.add_argument("-h", "--help", help(parser, 0))
        .help("display this help screen");

    String error;
    if (!parser.parse_args(argc - 1, argv + 1, error)) {
        print(io::err, format("{0}: {1}\n", parser.name(), error));
        exit(1);
    }

    if (output_dir.has()) {
        makedirs(*output_dir, 0755);
    }

    Preferences::set_preferences(new Preferences);
    PrefsDriver::set_driver(new NullPrefsDriver);
    OffscreenVideoDriver video(
            Preferences::preferences()->screen_size(), output_dir);
    scoped_ptr<Ledger> ledger(new NullLedger);
    if (script == "main-screen") {
        main_screen(video);
    } else if (script == "options") {
        options(video);
    } else if (script == "mission-briefing") {
        mission_briefing(video, *ledger);
    } else if (script == "pause") {
        pause(video);
    } else {
        print(io::err, format("no such script {0}\n", quote(script)));
        exit(1);
    }
    VideoDriver::set_driver(&video);

    if (output_dir.has()) {
        String out(format("{0}/sound.log", *output_dir));
        SoundDriver::set_driver(new LogSoundDriver(out));
    } else {
        SoundDriver::set_driver(new NullSoundDriver);
    }
    Ledger::set_ledger(ledger.release());

    video.loop(AresInit());
}

void usage(const StringSlice& program_name) {
    print(io::err, format(
                "usage: {0} [<options>] <script>\n"
                "options:\n"
                "    -o|--output=<dir>  directory to save dumps to\n",
                program_name));
    exit(1);
}

void main_screen(OffscreenVideoDriver& driver) {
    driver.schedule_key(Keys::Q, 900, 901);
    driver.schedule_snapshot(900);
    driver.schedule_key(Keys::Q, 902, 903);
}

void options(OffscreenVideoDriver& driver) {
    driver.schedule_key(Keys::Q, 899, 900);

    // Head to the Options screens.  Grab the sound options screen.
    driver.schedule_key(Keys::O, 960, 1020);
    driver.schedule_snapshot(900);
    driver.schedule_snapshot(960);
    driver.schedule_snapshot(1020);

    // Switch to the key settings screen.  View all the tabs.
    driver.schedule_key(Keys::K, 1080, 1140);
    driver.schedule_snapshot(1080);
    driver.schedule_snapshot(1140);

    driver.schedule_mouse(0, Point(200, 50), 1200, 1260);
    driver.schedule_snapshot(1200);
    driver.schedule_snapshot(1260);

    driver.schedule_mouse(0, Point(300, 50), 1320, 1380);
    driver.schedule_snapshot(1320);
    driver.schedule_snapshot(1380);

    driver.schedule_mouse(0, Point(400, 50), 1440, 1500);
    driver.schedule_snapshot(1440);
    driver.schedule_snapshot(1500);

    driver.schedule_mouse(0, Point(500, 50), 1560, 1620);
    driver.schedule_snapshot(1560);
    driver.schedule_snapshot(1620);

    // Exit to the main menu, then quit.
    driver.schedule_mouse(0, Point(550, 430), 1680, 1740);
    driver.schedule_snapshot(1680);
    driver.schedule_snapshot(1740);

    driver.schedule_key(Keys::Q, 1800, 1860);
    driver.schedule_snapshot(1800);
    driver.schedule_snapshot(1860);
}

void mission_briefing(OffscreenVideoDriver& driver, Ledger& ledger) {
    ledger.unlock_chapter(22);

    driver.schedule_key(Keys::Q, 900, 901);
    driver.schedule_snapshot(900);

    // Head to the level selection screen.
    driver.schedule_key(Keys::S, 960, 1020);
    driver.schedule_snapshot(960);
    driver.schedule_snapshot(1020);

    // Start the level.  Level selection screen fades out.
    driver.schedule_key(Keys::RETURN, 1080, 1140);
    driver.schedule_snapshot(1080);
    driver.schedule_snapshot(1140);
    driver.schedule_snapshot(1170);
    driver.schedule_snapshot(1200);

    int64_t time_ticks = 1260;
    for (int i = 0; i < 10; ++i) {
        driver.schedule_key(Keys::RIGHT_ARROW, time_ticks, time_ticks + 60);
        driver.schedule_snapshot(time_ticks);
        driver.schedule_snapshot(time_ticks + 60);
        time_ticks += 120;
    }

    driver.schedule_key(Keys::ESCAPE, time_ticks, time_ticks + 1);
    driver.schedule_snapshot(time_ticks);

    driver.schedule_key(Keys::Q, time_ticks + 60, time_ticks + 120);
    driver.schedule_snapshot(time_ticks + 60);
    driver.schedule_snapshot(time_ticks + 120);
}

void pause(OffscreenVideoDriver& driver) {
    driver.schedule_event(make_linked_ptr(new MouseMoveEvent(0, Point(320, 240))));

    // Skip the intro.  Start the first tutorial and skip the prologue.
    driver.schedule_key(Keys::Q, 1798, 1799);
    driver.schedule_key(Keys::S, 1858, 1859);
    driver.schedule_key(Keys::RETURN, 1917, 1918);
    driver.schedule_snapshot(1917);
    driver.schedule_snapshot(1947);
    driver.schedule_snapshot(1977);
    driver.schedule_key(Keys::RETURN, 1978, 1979);

    driver.schedule_snapshot(1980);
    driver.schedule_snapshot(2000);

    driver.schedule_key(Keys::CAPS_LOCK, 2020, 2140);
    driver.schedule_snapshot(2020);
    driver.schedule_snapshot(2040);
    driver.schedule_snapshot(2060);
    driver.schedule_snapshot(2080);
    driver.schedule_snapshot(2100);
    driver.schedule_snapshot(2120);
    driver.schedule_snapshot(2140);

    driver.schedule_snapshot(2160);
    driver.schedule_snapshot(2180);

    // Exit play.
    driver.schedule_key(Keys::ESCAPE, 2200, 2260);
    driver.schedule_snapshot(2200);
    driver.schedule_snapshot(2260);

    driver.schedule_key(Keys::Q, 2320, 2380);
    driver.schedule_snapshot(2320);
    driver.schedule_snapshot(2380);

    // Quit game.
    driver.schedule_key(Keys::Q, 2440, 2500);
    driver.schedule_snapshot(2440);
    driver.schedule_snapshot(2500);
}

}  // namespace
}  // namespace antares

int main(int argc, char* const* argv) {
    antares::main(argc, argv);
    return 0;
}

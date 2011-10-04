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
void the_stars_have_ears(OffscreenVideoDriver& driver);
void while_the_iron_is_hot(OffscreenVideoDriver& driver);
void space_race(OffscreenVideoDriver& driver);
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
    scoped_ptr<OffscreenVideoDriver> video(new OffscreenVideoDriver(
                Preferences::preferences()->screen_size(), output_dir));
    scoped_ptr<Ledger> ledger(new NullLedger);
    if (script == "main-screen") {
        main_screen(*video);
    } else if (script == "options") {
        options(*video);
    } else if (script == "mission-briefing") {
        mission_briefing(*video, *ledger);
    } else if (script == "pause") {
        pause(*video);
    } else {
        print(io::err, format("no such script {0}\n", quote(script)));
        exit(1);
    }
    VideoDriver::set_driver(video.release());

    if (output_dir.has()) {
        String out(format("{0}/sound.log", *output_dir));
        SoundDriver::set_driver(new LogSoundDriver(out));
    } else {
        SoundDriver::set_driver(new NullSoundDriver);
    }
    Ledger::set_ledger(ledger.release());

    VideoDriver::driver()->loop(AresInit());
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

void demo(OffscreenVideoDriver& driver, int demo, int64_t duration_ticks) {
    driver.set_demo_scenario(demo);

    driver.schedule_event(new MouseMoveEvent(Point(320, 240)), 0);

    // Ego Pict fades in and out.
    driver.schedule_snapshot(50);
    driver.schedule_snapshot(100);
    driver.schedule_snapshot(150);
    driver.schedule_snapshot(180);
    driver.schedule_snapshot(230);

    // Intermission (black).
    driver.schedule_snapshot(280);

    // Title Screen fades in and out.
    driver.schedule_snapshot(330);
    driver.schedule_snapshot(380);
    driver.schedule_snapshot(530);
    driver.schedule_snapshot(680);
    driver.schedule_snapshot(730);

    // Intermission (black).
    driver.schedule_snapshot(780);

    // Introduction scrolls by.
    for (int64_t i = 840; i < 3600; i += 60) {
        driver.schedule_snapshot(i);
    }

    // Intro is skipped.  Main Screen fades out after 30 seconds.
    driver.schedule_key(Keys::Q, 3599, 3600);
    driver.schedule_snapshot(3598);
    driver.schedule_snapshot(3599);
    driver.schedule_snapshot(5400);
    driver.schedule_snapshot(5430);

    // Demo plays.
    duration_ticks += 5460;  // Compensate for picts, intro.
    for (int64_t i = 5460; i < duration_ticks; i += 60) {
        driver.schedule_snapshot(i);
    }

    // Quit game.
    driver.schedule_key(Keys::Q, duration_ticks, duration_ticks + 60);
    driver.schedule_snapshot(duration_ticks);
}

void the_stars_have_ears(OffscreenVideoDriver& driver) {
    demo(driver, 600, 8400);  // 2:20
}

void while_the_iron_is_hot(OffscreenVideoDriver& driver) {
    demo(driver, 605, 12000);  // 3:20
}

void space_race(OffscreenVideoDriver& driver) {
    demo(driver, 623, 10200);  // 2:50
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
    driver.set_demo_scenario(623);

    driver.schedule_event(new MouseMoveEvent(Point(320, 240)), 0);

    // Intro is skipped.  Main Screen fades out after 30 seconds.
    driver.schedule_key(Keys::Q, 1798, 1799);
    driver.schedule_snapshot(1799);
    driver.schedule_snapshot(3599);
    driver.schedule_snapshot(3629);

    driver.schedule_key(Keys::CAPS_LOCK, 3660, 3780);
    driver.schedule_snapshot(3660);
    driver.schedule_snapshot(3680);
    driver.schedule_snapshot(3700);
    driver.schedule_snapshot(3720);
    driver.schedule_snapshot(3740);
    driver.schedule_snapshot(3760);
    driver.schedule_snapshot(3780);

    driver.schedule_key(Keys::Q, 3840, 3900);
    driver.schedule_snapshot(3840);
    driver.schedule_snapshot(3900);

    // Quit game.
    driver.schedule_key(Keys::Q, 3960, 4020);
    driver.schedule_snapshot(3960);
    driver.schedule_snapshot(4020);
}

}  // namespace
}  // namespace antares

int main(int argc, char* const* argv) {
    antares::main(argc, argv);
    return 0;
}

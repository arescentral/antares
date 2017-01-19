// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// Copyright (C) 2008-2017 The Antares Authors
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
// License along with Antares.  If not, see http://www.gnu.org/licenses/

#include <getopt.h>
#include <sys/time.h>
#include <queue>
#include <sfz/sfz.hpp>

#include "config/ledger.hpp"
#include "config/preferences.hpp"
#include "sound/driver.hpp"
#include "ui/card.hpp"
#include "ui/flows/master.hpp"
#include "video/driver.hpp"
#include "video/offscreen-driver.hpp"
#include "video/text-driver.hpp"

using sfz::Bytes;
using sfz::Exception;
using sfz::Optional;
using sfz::Rune;
using sfz::String;
using sfz::StringSlice;
using sfz::args::help;
using sfz::args::store;
using sfz::args::store_const;
using sfz::makedirs;
using sfz::print;
using sfz::quote;
using sfz::string_to_int;
using std::unique_ptr;

namespace args = sfz::args;
namespace io   = sfz::io;
namespace utf8 = sfz::utf8;

namespace antares {
namespace {

void main_screen(EventScheduler& scheduler);
void options(EventScheduler& scheduler);
void mission_briefing(EventScheduler& scheduler, Ledger& ledger);
void pause(EventScheduler& scheduler);

void main(int argc, char* const* argv) {
    args::Parser parser(argv[0], "Simulates a game off-screen");

    String script;
    parser.add_argument("script", store(script))
            .metavar("main-screen|options|mission-briefing|pause")
            .required()
            .help("the script to execute");

    Optional<String> output_dir;
    bool             text = false;
    parser.add_argument("-o", "--output", store(output_dir))
            .help("place output in this directory");
    parser.add_argument("-t", "--text", store_const(text, true)).help("produce text output");
    parser.add_argument("-h", "--help", help(parser, 0)).help("display this help screen");

    String error;
    if (!parser.parse_args(argc - 1, argv + 1, error)) {
        print(io::err, format("{0}: {1}\n", parser.name(), error));
        exit(1);
    }

    if (output_dir.has()) {
        makedirs(*output_dir, 0755);
    }

    NullPrefsDriver prefs;
    EventScheduler  scheduler;
    NullLedger      ledger;
    if (script == "main-screen") {
        main_screen(scheduler);
    } else if (script == "options") {
        options(scheduler);
    } else if (script == "mission-briefing") {
        mission_briefing(scheduler, ledger);
    } else if (script == "pause") {
        pause(scheduler);
    } else {
        print(io::err, format("no such script {0}\n", quote(script)));
        exit(1);
    }

    unique_ptr<SoundDriver> sound;
    if (output_dir.has()) {
        String out(format("{0}/sound.log", *output_dir));
        sound.reset(new LogSoundDriver(out));
    } else {
        sound.reset(new NullSoundDriver);
    }

    if (text) {
        TextVideoDriver video({640, 480}, output_dir);
        video.loop(new Master(14586), scheduler);
    } else {
        OffscreenVideoDriver video({640, 480}, output_dir);
        video.loop(new Master(14586), scheduler);
    }
}

void main_screen(EventScheduler& scheduler) {
    scheduler.schedule_key(Keys::Q, 900, 901);
    scheduler.schedule_snapshot(900);
    scheduler.schedule_key(Keys::Q, 902, 903);
}

void options(EventScheduler& scheduler) {
    scheduler.schedule_key(Keys::Q, 899, 900);

    // Head to the Options screens.  Grab the sound options screen.
    scheduler.schedule_key(Keys::O, 960, 1020);
    scheduler.schedule_snapshot(900);
    scheduler.schedule_snapshot(960);
    scheduler.schedule_snapshot(1020);

    // Switch to the key settings screen.  View all the tabs.
    scheduler.schedule_key(Keys::K, 1080, 1140);
    scheduler.schedule_snapshot(1080);
    scheduler.schedule_snapshot(1140);

    scheduler.schedule_mouse(0, Point(200, 50), 1200, 1260);
    scheduler.schedule_snapshot(1200);
    scheduler.schedule_snapshot(1260);

    scheduler.schedule_mouse(0, Point(300, 50), 1320, 1380);
    scheduler.schedule_snapshot(1320);
    scheduler.schedule_snapshot(1380);

    scheduler.schedule_mouse(0, Point(400, 50), 1440, 1500);
    scheduler.schedule_snapshot(1440);
    scheduler.schedule_snapshot(1500);

    scheduler.schedule_mouse(0, Point(500, 50), 1560, 1620);
    scheduler.schedule_snapshot(1560);
    scheduler.schedule_snapshot(1620);

    // Exit to the main menu, then quit.
    scheduler.schedule_mouse(0, Point(550, 430), 1680, 1740);
    scheduler.schedule_snapshot(1680);
    scheduler.schedule_snapshot(1740);

    scheduler.schedule_key(Keys::Q, 1800, 1860);
    scheduler.schedule_snapshot(1800);
}

void mission_briefing(EventScheduler& scheduler, Ledger& ledger) {
    ledger.unlock_chapter(22);

    scheduler.schedule_key(Keys::Q, 900, 901);
    scheduler.schedule_snapshot(900);

    // Head to the level selection screen.
    scheduler.schedule_key(Keys::S, 960, 1020);
    scheduler.schedule_snapshot(960);
    scheduler.schedule_snapshot(1020);

    // Start the level.  Level selection screen fades out.
    scheduler.schedule_key(Keys::RETURN, 1080, 1140);
    scheduler.schedule_snapshot(1080);
    scheduler.schedule_snapshot(1140);
    scheduler.schedule_snapshot(1170);
    scheduler.schedule_snapshot(1200);

    int64_t time_ticks = 1260;
    for (int i = 0; i < 10; ++i) {
        scheduler.schedule_key(Keys::RIGHT_ARROW, time_ticks, time_ticks + 60);
        scheduler.schedule_snapshot(time_ticks);
        scheduler.schedule_snapshot(time_ticks + 60);
        time_ticks += 120;
    }

    scheduler.schedule_key(Keys::ESCAPE, time_ticks, time_ticks + 1);
    scheduler.schedule_snapshot(time_ticks);

    scheduler.schedule_key(Keys::Q, time_ticks + 60, time_ticks + 120);
    scheduler.schedule_snapshot(time_ticks + 60);
    scheduler.schedule_snapshot(time_ticks + 119);
    scheduler.schedule_snapshot(time_ticks + 120);
    scheduler.schedule_snapshot(time_ticks + 150);
    scheduler.schedule_snapshot(time_ticks + 179);
    scheduler.schedule_snapshot(time_ticks + 180);
}

void pause(EventScheduler& scheduler) {
    scheduler.schedule_event(unique_ptr<Event>(new MouseMoveEvent(wall_time(), Point(320, 240))));

    // Skip the intro.  Start the first tutorial and skip the prologue.
    scheduler.schedule_key(Keys::Q, 1756, 1757);
    scheduler.schedule_key(Keys::S, 1816, 1817);
    scheduler.schedule_key(Keys::RETURN, 1875, 1876);
    scheduler.schedule_snapshot(1875);
    scheduler.schedule_snapshot(1905);
    scheduler.schedule_snapshot(1935);
    scheduler.schedule_key(Keys::RETURN, 1936, 1937);

    scheduler.schedule_snapshot(1937);
    scheduler.schedule_snapshot(1957);
    scheduler.schedule_snapshot(1977);

    scheduler.schedule_snapshot(1980);
    scheduler.schedule_snapshot(2000);

    scheduler.schedule_key(Keys::CAPS_LOCK, 2020, 2140);
    scheduler.schedule_snapshot(2020);
    scheduler.schedule_snapshot(2040);
    scheduler.schedule_snapshot(2060);
    scheduler.schedule_snapshot(2080);
    scheduler.schedule_snapshot(2100);
    scheduler.schedule_snapshot(2120);
    scheduler.schedule_snapshot(2140);

    scheduler.schedule_snapshot(2160);
    scheduler.schedule_snapshot(2180);

    // Exit play.
    scheduler.schedule_key(Keys::ESCAPE, 2200, 2260);
    scheduler.schedule_snapshot(2200);
    scheduler.schedule_snapshot(2260);

    scheduler.schedule_key(Keys::Q, 2320, 2380);
    scheduler.schedule_snapshot(2320);
    scheduler.schedule_snapshot(2380);

    // Quit game.
    scheduler.schedule_key(Keys::Q, 2440, 2500);
    scheduler.schedule_snapshot(2440);
}

}  // namespace
}  // namespace antares

int main(int argc, char* const* argv) {
    antares::main(argc, argv);
    return 0;
}

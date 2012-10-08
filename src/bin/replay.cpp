// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// Copyright (C) 2008-2012 The Antares Authors
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

#include <sfz/sfz.hpp>
#include <getopt.h>

#include "config/ledger.hpp"
#include "config/preferences.hpp"
#include "data/replay.hpp"
#include "drawing/color.hpp"
#include "drawing/offscreen-gworld.hpp"
#include "drawing/pix-map.hpp"
#include "drawing/text.hpp"
#include "game/admiral.hpp"
#include "game/cheat.hpp"
#include "game/cursor.hpp"
#include "game/globals.hpp"
#include "game/input-source.hpp"
#include "game/instruments.hpp"
#include "game/labels.hpp"
#include "game/main.hpp"
#include "game/messages.hpp"
#include "game/motion.hpp"
#include "game/scenario-maker.hpp"
#include "math/random.hpp"
#include "math/rotation.hpp"
#include "sound/driver.hpp"
#include "sound/music.hpp"
#include "ui/card.hpp"
#include "ui/interface-handling.hpp"
#include "video/driver.hpp"
#include "video/offscreen-driver.hpp"

using sfz::BytesSlice;
using sfz::MappedFile;
using sfz::Optional;
using sfz::String;
using sfz::StringSlice;
using sfz::args::store;
using sfz::format;
using sfz::make_linked_ptr;
using sfz::mkdir;
using sfz::scoped_ptr;
namespace args = sfz::args;
namespace io = sfz::io;
namespace path = sfz::path;
namespace utf8 = sfz::utf8;

namespace antares {

class ReplayMaster : public Card {
  public:
    ReplayMaster(BytesSlice data):
            _state(NEW),
            _replay_data(data),
            _random_seed(_replay_data.global_seed),
            _game_result(NO_GAME) { }

    virtual void become_front() {
        switch (_state) {
          case NEW:
            _state = REPLAY;
            init();
            Randomize(4);  // For the decision to replay intro.
            _game_result = NO_GAME;
            gRandomSeed = _random_seed;
            globals()->gInputSource.reset(new ReplayInputSource(&_replay_data));
            stack()->push(new MainPlay(
                        GetScenarioPtrFromChapter(_replay_data.chapter_id), true, false,
                        &_game_result));
            break;

          case REPLAY:
            stack()->pop(this);
            break;
        }
    }

  private:
    void init();

    enum State {
        NEW,
        REPLAY,
    };
    State _state;

    ReplayData _replay_data;
    const int32_t _random_seed;
    GameResult _game_result;

    DISALLOW_COPY_AND_ASSIGN(ReplayMaster);
};

void ReplayMaster::init() {
    init_globals();

    SoundDriver::driver()->set_global_volume(8);  // Max volume.

    world = Rect(Point(0, 0), Preferences::preferences()->screen_size());
    play_screen = Rect(
        world.left + kLeftPanelWidth, world.top,
        world.right - kRightPanelWidth, world.bottom);
    viewport = play_screen;

    gRealWorld = new ArrayPixMap(world.width(), world.height());
    gRealWorld->fill(RgbColor::kBlack);
    CreateOffscreenWorld();
    InitSpriteCursor();
    RotationInit();
    InterfaceHandlingInit();
    InitDirectText();
    ScreenLabelInit();
    InitMessageScreen();
    InstrumentInit();
    SpriteHandlingInit();
    AresCheatInit();
    ScenarioMakerInit();
    SpaceObjectHandlingInit();  // MUST be after ScenarioMakerInit()
    InitSoundFX();
    MusicInit();
    InitMotion();
    AdmiralInit();
    InitBeams();
}

void demo(OffscreenVideoDriver& driver) {
}

void space_race(OffscreenVideoDriver& driver) {
    demo(driver);  // 2:50
}

void usage(StringSlice program_name) {
    print(io::err, format("usage: {0} replay_path output_dir\n", program_name));
    exit(1);
}

void main(int argc, char** argv) {
    args::Parser parser(argv[0], "Plays a replay into a set of images and a log of sounds");

    String replay_path(utf8::decode(argv[0]));
    parser.add_argument("replay", store(replay_path))
        .help("an Antares replay script")
        .required();

    Optional<String> output_dir;
    parser.add_argument("-o", "--output", store(output_dir))
        .help("place output in this directory");

    int interval = 60;
    int width = 640;
    int height = 480;
    parser.add_argument("-i", "--interval", store(interval))
        .help("take one screenshot per this many ticks (default: 60)");
    parser.add_argument("-w", "--width", store(width))
        .help("screen width (default: 640)");
    parser.add_argument("-h", "--height", store(height))
        .help("screen height (default: 480)");

    parser.add_argument("--help", help(parser, 0))
        .help("display this help screen");

    String error;
    if (!parser.parse_args(argc - 1, argv + 1, error)) {
        print(io::err, format("{0}: {1}\n", parser.name(), error));
        exit(1);
    }

    if (output_dir.has()) {
        makedirs(*output_dir, 0755);
    }

    Preferences preferences;
    preferences.set_screen_size(Size(width, height));
    preferences.set_play_music_in_game(true);
    NullPrefsDriver prefs(preferences);

    OffscreenVideoDriver video(
            Preferences::preferences()->screen_size(), output_dir);
    video.schedule_event(make_linked_ptr(new MouseMoveEvent(0, Point(320, 240))));
    // TODO(sfiera): add recurring snapshots to OffscreenVideoDriver.
    for (int64_t i = 1; i < 72000; i += interval) {
        video.schedule_snapshot(i);
    }

    scoped_ptr<SoundDriver> sound;
    if (output_dir.has()) {
        String out(format("{0}/sound.log", *output_dir));
        sound.reset(new LogSoundDriver(out));
    } else {
        sound.reset(new NullSoundDriver);
    }
    NullLedger ledger;

    MappedFile replay_file(replay_path);
    video.loop(new ReplayMaster(replay_file.data()));
}

}  // namespace antares

int main(int argc, char** argv) {
    antares::main(argc, argv);
    return 0;
}

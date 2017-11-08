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

#include <fcntl.h>
#include <getopt.h>
#include <sfz/sfz.hpp>

#include "config/ledger.hpp"
#include "config/preferences.hpp"
#include "data/plugin.hpp"
#include "data/replay.hpp"
#include "data/resource.hpp"
#include "drawing/color.hpp"
#include "drawing/pix-map.hpp"
#include "game/admiral.hpp"
#include "game/cheat.hpp"
#include "game/cursor.hpp"
#include "game/globals.hpp"
#include "game/input-source.hpp"
#include "game/instruments.hpp"
#include "game/labels.hpp"
#include "game/level.hpp"
#include "game/main.hpp"
#include "game/messages.hpp"
#include "game/motion.hpp"
#include "game/space-object.hpp"
#include "game/sys.hpp"
#include "game/vector.hpp"
#include "math/random.hpp"
#include "math/rotation.hpp"
#include "sound/driver.hpp"
#include "sound/music.hpp"
#include "ui/card.hpp"
#include "ui/interface-handling.hpp"
#include "ui/screens/debriefing.hpp"
#include "video/driver.hpp"
#include "video/offscreen-driver.hpp"
#include "video/text-driver.hpp"

using sfz::BytesSlice;
using sfz::MappedFile;
using sfz::Optional;
using sfz::ScopedFd;
using sfz::String;
using sfz::StringSlice;
using sfz::args::store;
using sfz::args::store_const;
using sfz::format;
using sfz::mkdir;
using sfz::open;
using std::unique_ptr;

namespace args = sfz::args;
namespace io   = sfz::io;
namespace path = sfz::path;
namespace utf8 = sfz::utf8;

namespace antares {

class ReplayMaster : public Card {
  public:
    ReplayMaster(BytesSlice data, Optional<String> output_path)
            : _state(NEW),
              _output_path(output_path),
              _replay_data(data),
              _random_seed(_replay_data.global_seed),
              _game_result(NO_GAME),
              _input_source(&_replay_data) {}

    virtual void become_front() {
        switch (_state) {
            case NEW:
                _state = REPLAY;
                init();
                Randomize(4);  // For the decision to replay intro.
                _game_result  = NO_GAME;
                g.random.seed = _random_seed;
                stack()->push(new MainPlay(
                        Handle<Level>(_replay_data.chapter_id - 1), true, &_input_source, false,
                        &_game_result));
                break;

            case REPLAY:
                if (_output_path.has()) {
                    String path(format("{0}/debriefing.txt", *_output_path));
                    makedirs(path::dirname(path), 0755);
                    ScopedFd outcome(open(path, O_WRONLY | O_CREAT, 0644));
                    if ((g.victory_text >= 0)) {
                        Resource rsrc("text", "txt", g.victory_text);
                        sfz::write(outcome, rsrc.data());
                        if (_game_result == WIN_GAME) {
                            sfz::write(outcome, "\n\n");
                            Handle<Admiral> player(0);
                            String          text = DebriefingScreen::build_score_text(
                                    g.time, g.level->parTime, GetAdmiralLoss(player),
                                    g.level->parLosses, GetAdmiralKill(player), g.level->parKills);
                            sfz::write(outcome, utf8::encode(text));
                        }
                        sfz::write(outcome, "\n");
                    }
                }
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

    Optional<String>  _output_path;
    ReplayData        _replay_data;
    const int32_t     _random_seed;
    GameResult        _game_result;
    ReplayInputSource _input_source;

    DISALLOW_COPY_AND_ASSIGN(ReplayMaster);
};

void ReplayMaster::init() {
    init_globals();

    sys.audio->set_global_volume(8);  // Max volume.

    sys_init();
    Label::init();
    Messages::init();
    InstrumentInit();
    SpriteHandlingInit();
    PluginInit();
    SpaceObjectHandlingInit();  // MUST be after PluginInit()
    InitMotion();
    Admiral::init();
    Vectors::init();
}

void usage(StringSlice program_name) {
    print(io::err, format("usage: {0} replay_path output_dir\n", program_name));
    exit(1);
}

void main(int argc, char** argv) {
    args::Parser parser(argv[0], "Plays a replay into a set of images and a log of sounds");

    String replay_path(utf8::decode(argv[0]));
    parser.add_argument("replay", store(replay_path)).help("an Antares replay script").required();

    Optional<String> output_dir;
    parser.add_argument("-o", "--output", store(output_dir))
            .help("place output in this directory");

    int  interval = 60;
    int  width    = 640;
    int  height   = 480;
    bool text     = false;
    bool smoke    = false;
    parser.add_argument("-i", "--interval", store(interval))
            .help("take one screenshot per this many ticks (default: 60)");
    parser.add_argument("-w", "--width", store(width)).help("screen width (default: 640)");
    parser.add_argument("-h", "--height", store(height)).help("screen height (default: 480)");
    parser.add_argument("-t", "--text", store_const(text, true)).help("produce text output");
    parser.add_argument("-s", "--smoke", store_const(smoke, true)).help("run as smoke text");

    parser.add_argument("--help", help(parser, 0)).help("display this help screen");

    String error;
    if (!parser.parse_args(argc - 1, argv + 1, error)) {
        print(io::err, format("{0}: {1}\n", parser.name(), error));
        exit(1);
    }

    if (output_dir.has()) {
        makedirs(*output_dir, 0755);
    }

    Preferences preferences;
    preferences.play_music_in_game = true;
    NullPrefsDriver prefs(preferences);

    EventScheduler scheduler;
    scheduler.schedule_event(unique_ptr<Event>(new MouseMoveEvent(wall_time(), Point(320, 240))));
    // TODO(sfiera): add recurring snapshots to OffscreenVideoDriver.
    for (int64_t i = 1; i < 72000; i += interval) {
        scheduler.schedule_snapshot(i);
    }

    unique_ptr<SoundDriver> sound;
    if (!smoke && output_dir.has()) {
        String out(format("{0}/sound.log", *output_dir));
        sound.reset(new LogSoundDriver(out));
    } else {
        sound.reset(new NullSoundDriver);
    }
    NullLedger ledger;

    MappedFile replay_file(replay_path);
    if (smoke) {
        TextVideoDriver video({width, height}, Optional<String>());
        video.loop(new ReplayMaster(replay_file.data(), output_dir), scheduler);
    } else if (text) {
        TextVideoDriver video({width, height}, output_dir);
        video.loop(new ReplayMaster(replay_file.data(), output_dir), scheduler);
    } else {
        OffscreenVideoDriver video({width, height}, output_dir);
        video.loop(new ReplayMaster(replay_file.data(), output_dir), scheduler);
    }
}

}  // namespace antares

int main(int argc, char** argv) {
    antares::main(argc, argv);
    return 0;
}

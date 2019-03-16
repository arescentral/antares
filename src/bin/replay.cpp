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
#include <pn/file>
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
#include "lang/exception.hpp"
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

using std::unique_ptr;

namespace args = sfz::args;
namespace path = sfz::path;

namespace antares {
namespace {

class ReplayMaster : public Card {
  public:
    ReplayMaster(pn::data_view data, const sfz::optional<pn::string>& output_path)
            : _state(NEW),
              _replay_data(data),
              _random_seed(_replay_data.global_seed),
              _game_result(NO_GAME),
              _input_source(&_replay_data) {
        if (output_path.has_value()) {
            _output_path.emplace(output_path->copy());
        }
    }

    virtual void become_front() {
        switch (_state) {
            case NEW:
                _state = REPLAY;
                init();
                Randomize(4);  // For the decision to replay intro.
                _game_result  = NO_GAME;
                g.random.seed = _random_seed;
                stack()->push(new MainPlay(
                        *Level::get(_replay_data.chapter_id), true, &_input_source, false,
                        &_game_result));
                break;

            case REPLAY:
                if (_output_path.has_value()) {
                    pn::string path = pn::format("{0}/debriefing.txt", *_output_path);
                    sfz::makedirs(path::dirname(path), 0755);
                    pn::file outcome = pn::open(path, "w");
                    if (g.victory_text.has_value()) {
                        outcome.write(*g.victory_text);
                        if (_game_result == WIN_GAME) {
                            outcome.write("\n");
                            Handle<Admiral> player(0);
                            pn::string      text = DebriefingScreen::build_score_text(
                                    g.time, g.level->solo.par.time, GetAdmiralLoss(player),
                                    g.level->solo.par.losses, GetAdmiralKill(player),
                                    g.level->solo.par.kills);
                            outcome.write(text);
                            outcome.write("\n");
                        }
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

    sfz::optional<pn::string> _output_path;
    ReplayData                _replay_data;
    const int32_t             _random_seed;
    GameResult                _game_result;
    ReplayInputSource         _input_source;
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

void usage(pn::file_view out, pn::string_view progname, int retcode) {
    out.format(
            "usage: {0} [OPTIONS]\n"
            "\n"
            "  Plays a replay into a set of images and a log of sounds\n"
            "\n"
            "  arguments:\n"
            "    replay              an Antares replay script\n"
            "\n"
            "  options:\n"
            "    -o, --output=OUTPUT place output in this directory\n"
            "    -i, --interval=INTERVAL\n"
            "                        take one screenshot per this many ticks (default: 60)\n"
            "    -w, --width=WIDTH   screen width (default: 640)\n"
            "    -h, --height=HEIGHT screen height (default: 480)\n"
            "    -t, --text          produce text output\n"
            "    -s, --smoke         run as smoke text\n"
            "        --help          display this help screen\n",
            progname);
    exit(retcode);
}

void main(int argc, char* const* argv) {
    args::callbacks callbacks;

    sfz::optional<pn::string> replay_path;
    callbacks.argument = [&replay_path](pn::string_view arg) {
        if (!replay_path.has_value()) {
            replay_path.emplace(arg.copy());
        } else {
            return false;
        }
        return true;
    };

    sfz::optional<pn::string> output_dir;
    int                       interval = 60;
    int                       width    = 640;
    int                       height   = 480;
    bool                      text     = false;
    bool                      smoke    = false;
    callbacks.short_option             = [&output_dir, &interval, &width, &height, &text, &smoke](
                                     pn::rune opt, const args::callbacks::get_value_f& get_value) {
        switch (opt.value()) {
            case 'o': output_dir.emplace(get_value().copy()); return true;
            case 'i': sfz::args::integer_option(get_value(), &interval); return true;
            case 'w': sfz::args::integer_option(get_value(), &width); return true;
            case 'h': sfz::args::integer_option(get_value(), &height); return true;
            case 't': text = true; return true;
            case 's': smoke = true; return true;
            default: return false;
        }
    };

    callbacks.long_option = [&argv, &callbacks](
                                    pn::string_view                     opt,
                                    const args::callbacks::get_value_f& get_value) {
        if (opt == "output") {
            return callbacks.short_option(pn::rune{'o'}, get_value);
        } else if (opt == "interval") {
            return callbacks.short_option(pn::rune{'i'}, get_value);
        } else if (opt == "width") {
            return callbacks.short_option(pn::rune{'w'}, get_value);
        } else if (opt == "height") {
            return callbacks.short_option(pn::rune{'h'}, get_value);
        } else if (opt == "text") {
            return callbacks.short_option(pn::rune{'t'}, get_value);
        } else if (opt == "smoke") {
            return callbacks.short_option(pn::rune{'s'}, get_value);
        } else if (opt == "help") {
            usage(stdout, sfz::path::basename(argv[0]), 0);
            return true;
        } else {
            return false;
        }
    };

    args::parse(argc - 1, argv + 1, callbacks);
    if (!replay_path.has_value()) {
        throw std::runtime_error("missing required argument 'replay'");
    }

    if (output_dir.has_value()) {
        sfz::makedirs(*output_dir, 0755);
    }

    Preferences preferences;
    preferences.play_music_in_game = true;
    NullPrefsDriver prefs(preferences.copy());

    EventScheduler scheduler;
    scheduler.schedule_event(unique_ptr<Event>(new MouseMoveEvent(wall_time(), Point(320, 240))));
    // TODO(sfiera): add recurring snapshots to OffscreenVideoDriver.
    for (int64_t i = 1; i < 72000; i += interval) {
        scheduler.schedule_snapshot(i);
    }

    unique_ptr<SoundDriver> sound;
    if (!smoke && output_dir.has_value()) {
        pn::string out = pn::format("{0}/sound.log", *output_dir);
        sound.reset(new LogSoundDriver(out));
    } else {
        sound.reset(new NullSoundDriver);
    }
    NullLedger ledger;

    sfz::mapped_file replay_file(*replay_path);
    if (smoke) {
        TextVideoDriver video({width, height}, sfz::optional<pn::string>());
        video.loop(new ReplayMaster(replay_file.data(), output_dir), scheduler);
    } else if (text) {
        TextVideoDriver video({width, height}, output_dir);
        video.loop(new ReplayMaster(replay_file.data(), output_dir), scheduler);
    } else {
        OffscreenVideoDriver video({width, height}, output_dir);
        video.loop(new ReplayMaster(replay_file.data(), output_dir), scheduler);
    }
}

}  // namespace
}  // namespace antares

int main(int argc, char* const* argv) { return antares::wrap_main(antares::main, argc, argv); }

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

#include "game/main.hpp"

#include <fcntl.h>
#include <math.h>
#include <algorithm>
#include <set>

#include "config/gamepad.hpp"
#include "config/keys.hpp"
#include "config/preferences.hpp"
#include "data/replay.hpp"
#include "data/string-list.hpp"
#include "data/scenario-list.hpp"
#include "drawing/color.hpp"
#include "drawing/shapes.hpp"
#include "drawing/sprite-handling.hpp"
#include "drawing/text.hpp"
#include "game/action.hpp"
#include "game/admiral.hpp"
#include "game/beam.hpp"
#include "game/cursor.hpp"
#include "game/globals.hpp"
#include "game/input-source.hpp"
#include "game/instruments.hpp"
#include "game/labels.hpp"
#include "game/messages.hpp"
#include "game/minicomputer.hpp"
#include "game/motion.hpp"
#include "game/non-player-ship.hpp"
#include "game/player-ship.hpp"
#include "game/scenario-maker.hpp"
#include "game/starfield.hpp"
#include "game/time.hpp"
#include "lang/defines.hpp"
#include "math/units.hpp"
#include "sound/driver.hpp"
#include "sound/fx.hpp"
#include "sound/music.hpp"
#include "ui/card.hpp"
#include "ui/flows/master.hpp"
#include "ui/interface-handling.hpp"
#include "ui/screens/briefing.hpp"
#include "ui/screens/debriefing.hpp"
#include "ui/screens/help.hpp"
#include "ui/screens/loading.hpp"
#include "ui/screens/play-again.hpp"
#include "video/driver.hpp"

using sfz::Exception;
using sfz::ScopedFd;
using sfz::String;
using sfz::StringSlice;
using sfz::format;
using sfz::makedirs;
using sfz::open;
using std::max;
using std::min;
using std::set;
using std::unique_ptr;

namespace path = sfz::path;

namespace antares {

#ifdef DATA_COVERAGE
extern set<int32_t> covered_objects;
extern set<int32_t> covered_actions;
#endif  // DATA_COVERAGE

Rect world() {
    return Rect({0, 0}, VideoDriver::driver()->screen_size());
}

Rect play_screen() {
    const Size size = VideoDriver::driver()->screen_size();
    return Rect(kLeftPanelWidth, 0, size.width - kRightPanelWidth, size.height);
}

Rect viewport() {
    const Size size = VideoDriver::driver()->screen_size();
    return Rect(kLeftPanelWidth, 0, size.width - kRightPanelWidth, size.height - g.bottom_border);
}

class GamePlay : public Card {
  public:
    GamePlay(bool replay, ReplayBuilder& replay_builder, GameResult* game_result, secs* seconds);

    virtual void become_front();
    virtual void resign_front();

    virtual void draw() const;

    virtual bool next_timer(wall_time& time);
    virtual void fire_timer();

    virtual void key_down(const KeyDownEvent& event);
    virtual void key_up(const KeyUpEvent& event);

    virtual void mouse_down(const MouseDownEvent& event);
    virtual void mouse_up(const MouseUpEvent& event);
    virtual void mouse_move(const MouseMoveEvent& event);

    virtual void gamepad_button_down(const GamepadButtonDownEvent& event);
    virtual void gamepad_button_up(const GamepadButtonUpEvent& event);
    virtual void gamepad_stick(const GamepadStickEvent& event);

  private:
    enum State {
        PLAYING,
        PAUSED,
        PLAY_AGAIN,
        DEBRIEFING,
        HELP,
    };
    State _state;

    GameCursor _cursor;
    const bool _replay;
    GameResult* const _game_result;
    secs* const _seconds;
    wall_time _next_timer;
    const Rect _play_area;
    const bool _command_and_q;
    bool _fast_motion;
    bool _entering_message;
    bool _player_paused;
    ticks _decide_cycle;
    ticks _scenario_check_time;
    PlayAgainScreen::Item _play_again;
    PlayerShip _player_ship;
    ReplayBuilder& _replay_builder;

    // The wall_time that g.time corresponds to. Under normal operation,
    // this increases in lockstep with g.time, but during fast motion or
    // paused games, it tracks now() without regard for the in-game
    // clock.
    wall_time _real_time;
};

MainPlay::MainPlay(
        const Scenario* scenario, bool replay, bool show_loading_screen,
        GameResult* game_result, secs* seconds):
    _state(NEW),
    _scenario(scenario),
    _replay(replay),
    _show_loading_screen(show_loading_screen),
    _cancelled(false),
    _game_result(game_result),
    _seconds(seconds) { }

void MainPlay::become_front() {
    switch (_state) {
      case NEW:
        {
            _state = LOADING;
            RemoveAllSpaceObjects();
            g.game_over = false;

            _replay_builder.init(
                    Preferences::preferences()->scenario_identifier(),
                    String(u32_to_version(plug.meta.version)),
                    _scenario->chapter_number(),
                    g.random.seed);

            if (Preferences::preferences()->play_idle_music()) {
                LoadSong(3000);
                SetSongVolume( kMaxMusicVolume);
                PlaySong();
            }

            if (_show_loading_screen) {
                stack()->push(new LoadingScreen(_scenario, &_cancelled));
                break;
            } else {
                int32_t max;
                int32_t current = 0;
                if (!start_construct_scenario(_scenario, &max)) {
                    *_game_result = QUIT_GAME;
                    stack()->pop(this);
                    return;
                }
                while (current < max) {
                    construct_scenario(_scenario, &current);
                }
            }
        }
        // fall through.

      case LOADING:
        {
            if (_cancelled) {
                *_game_result = QUIT_GAME;
                stack()->pop(this);
                return;
            }
            if (!_replay) {
                _state = BRIEFING;
                stack()->push(new BriefingScreen(_scenario, &_cancelled));
                break;
            }
        }
        // fall through

      case BRIEFING:
        {
            if (Preferences::preferences()->play_idle_music()) {
                StopAndUnloadSong();
            }

            if (_cancelled) {
                *_game_result = QUIT_GAME;
                stack()->pop(this);
                break;
            }

            _state = PLAYING;

            set_up_instruments();

            if (Preferences::preferences()->play_music_in_game()) {
                LoadSong(g.level->songID);
                SetSongVolume(kMusicVolume);
                PlaySong();
            }

            if (!_replay) {
                _replay_builder.start();
            }
            stack()->push(new GamePlay(_replay, _replay_builder, _game_result, _seconds));
        }
        break;

      case PLAYING:
        globals()->transitions.reset();
        quiet_all();
        if (Preferences::preferences()->play_music_in_game()) {
            StopAndUnloadSong();
        }
        _replay_builder.finish();
#ifdef DATA_COVERAGE
        {
            sfz::print(sfz::io::err, sfz::format("{{ \"level\": {0},\n", g.level->chapter_number()));
            const char* sep = "";
            sfz::print(sfz::io::err, "  \"objects\": [");
            for (auto object: covered_objects) {
                sfz::print(sfz::io::err, sfz::format("{0}{1}", sep, object));
                sep = ", ";
            }
            sfz::print(sfz::io::err, "],\n");
            covered_objects.clear();

            sep = "";
            sfz::print(sfz::io::err, "  \"actions\": [");
            for (auto action: covered_actions) {
                sfz::print(sfz::io::err, sfz::format("{0}{1}", sep, action));
                sep = ", ";
            }
            sfz::print(sfz::io::err, "]\n");
            sfz::print(sfz::io::err, "}\n");
            covered_actions.clear();
        }
#endif  // DATA_COVERAGE
        stack()->pop(this);
        break;
    }
}

int new_replay_file() {
    String path;
    makedirs(path::basename(path), 0755);
    return open(path, O_WRONLY | O_CREAT | O_EXCL, 0644);
}

GamePlay::GamePlay(
        bool replay, ReplayBuilder& replay_builder, GameResult* game_result, secs* seconds):
        _state(PLAYING),
        _replay(replay),
        _game_result(game_result),
        _seconds(seconds),
        _next_timer(now() + kMinorTick),
        _play_area(viewport().left, viewport().top, viewport().right, viewport().bottom),
        _command_and_q(BothCommandAndQ()),
        _fast_motion(false),
        _entering_message(false),
        _player_paused(false),
        _decide_cycle(0),
        _scenario_check_time(0),
        _replay_builder(replay_builder),
        _real_time(now()) { }

static const usecs kSwitchAfter = usecs(1000000 / 3);
static const usecs kSleepAfter = usecs(60 * 1000000);

class PauseScreen : public Card {
  public:
    PauseScreen() {
        const StringList list(3100);
        _pause_string.assign(list.at(10));
        int32_t width = title_font->string_width(_pause_string);
        Rect bounds(0, 0, width, title_font->height);
        bounds.center_in(play_screen());
        _text_origin = Point(bounds.left, bounds.top + title_font->ascent);

        bounds.inset(-4, -4);
        _bracket_bounds = bounds;
    }

    virtual void become_front() {
        // TODO(sfiera): cancel any active transition.
        PlayVolumeSound(kComputerBeep4, kMaxSoundVolume, kShortPersistence, kMustPlaySound);
        _visible = true;
        _next_switch = now() + kSwitchAfter;
        _sleep_at = now() + kSleepAfter;
    }

    virtual void mouse_up(const MouseUpEvent& event) { wake(); }
    virtual void mouse_down(const MouseDownEvent& event) { wake(); }
    virtual void mouse_move(const MouseMoveEvent& event) { wake(); }
    virtual void key_down(const KeyDownEvent& event) { wake(); }

    virtual void key_up(const KeyUpEvent& event) {
        wake();
        if (event.key() == Keys::CAPS_LOCK) {
            stack()->pop(this);
        }
    }

    virtual bool next_timer(wall_time& time) {
        time = std::min(_next_switch, _sleep_at);
        return true;
    }

    virtual void fire_timer() {
        show_hide();
    }

    virtual void draw() const {
        next()->draw();
        if (asleep() || _visible) {
            const RgbColor& light_green = GetRGBTranslateColorShade(GREEN, LIGHTER);
            const RgbColor& dark_green = GetRGBTranslateColorShade(GREEN, DARKER);

            {
                Rects rects;
                for (int32_t y = _bracket_bounds.top + 2; y < _bracket_bounds.bottom; y += 2) {
                    rects.fill({_bracket_bounds.left, y, _bracket_bounds.right, y + 1}, dark_green);
                }
                draw_vbracket(rects, _bracket_bounds, light_green);
            }

            title_font->draw(_text_origin, _pause_string, light_green);
        }
        if (asleep()) {
            Rects().fill(world(), RgbColor(63, 0, 0, 0));
        }
    }

  private:
    void show_hide() {
        const wall_time now = antares::now();
        while (_next_switch < now) {
            _visible = !_visible;
            _next_switch += kSwitchAfter;
        }
    }

    bool asleep() const {
        return _sleep_at < now();
    }

    void wake() {
        _sleep_at = now() + kSleepAfter;
    }

    bool _visible;
    wall_time _next_switch;
    wall_time _sleep_at;
    String _pause_string;
    Point _text_origin;
    Rect _bracket_bounds;

    DISALLOW_COPY_AND_ASSIGN(PauseScreen);
};

void GamePlay::become_front() {
    switch (_state) {
      case PLAYING:
        if (_replay) {
            _cursor.show = false;
        } else {
            _cursor.show = true;
        }
        HintLine::reset();

        CheckScenarioConditions();
        break;

      case PAUSED:
      case HELP:
        _state = PLAYING;
        break;

      case PLAY_AGAIN:
        switch (_play_again) {
          case PlayAgainScreen::QUIT:
            *_game_result = QUIT_GAME;
            g.game_over = true;
            g.next_level = -1;
            g.victory_text = -1;
            stack()->pop(this);
            break;

          case PlayAgainScreen::RESTART:
            *_game_result = RESTART_GAME;
            g.game_over = true;
            g.next_level = -1;
            g.victory_text = -1;
            stack()->pop(this);
            break;

          case PlayAgainScreen::RESUME:
            _state = PLAYING;
            break;

          case PlayAgainScreen::SKIP:
            *_game_result = WIN_GAME;
            g.game_over = true;
            g.victor = g.admiral;
            g.next_level = g.level->chapter_number() + 1;
            g.victory_text = -1;
            stack()->pop(this);
            break;

          default:
            throw Exception(format("invalid play again result {0}", _play_again));
        }
        break;

      case DEBRIEFING:
        if (*_game_result == WIN_GAME) {
            stack()->pop(this);
        } else {  // LOSE_GAME
            _state = PLAY_AGAIN;
            stack()->push(new PlayAgainScreen(false, false, &_play_again));
        }
        break;
    }
}

void GamePlay::resign_front() {
    minicomputer_cancel();
}

void GamePlay::draw() const {
    globals()->starfield.draw();
    draw_sector_lines();
    Beams::draw();
    draw_sprites();
    Label::draw();

    Messages::draw_message();
    draw_site(_player_ship);
    draw_instruments();
    if (stack()->top() == this) {
        _cursor.draw();
    }
    HintLine::draw();
    globals()->transitions.draw();
}

bool GamePlay::next_timer(wall_time& time) {
    if (_state == PLAYING) {
        time = _next_timer;
        return true;
    }
    return false;
}

void GamePlay::fire_timer() {
    while (_next_timer < now()) {
        _next_timer = _next_timer + kMinorTick;
    }

    ticks unitsPassed;
    if (_fast_motion && !_entering_message) {
        unitsPassed = ticks(12);
        _real_time = now();
    } else {
        unitsPassed = ticks(0);
        wall_time new_now = now();
        while (_real_time <= (new_now - kMinorTick)) {
            unitsPassed += kMinorTick;
            _real_time += kMinorTick;
        }
    }

    if (unitsPassed <= ticks(0)) {
        return;
    }

    globals()->starfield.prepare_to_move();
    EraseSite();

    if (_player_paused) {
        _player_paused = false;
        unitsPassed = ticks(0);
        _real_time = now();
    }

    const ticks unitsDone = unitsPassed;
    while (unitsPassed > ticks(0)) {
        ticks unitsToDo = unitsPassed;
        if ((_decide_cycle + unitsToDo) > kMajorTick) {
            unitsToDo = kMajorTick - _decide_cycle;
        }
        _decide_cycle += unitsToDo;

        if (unitsToDo > ticks(0)) {
            // executed arbitrarily, but at least once every major tick
            globals()->starfield.move(unitsToDo);
            MoveSpaceObjects(unitsToDo);
        }

        g.time += unitsToDo;

        if ( _decide_cycle == kMajorTick) {
            // everything in here gets executed once every major tick
            _player_paused = false;

            NonplayerShipThink();
            AdmiralThink();
            execute_action_queue();

            if (globals()->gInputSource && !globals()->gInputSource->next(_player_ship)) {
                g.game_over = true;
                g.game_over_at = g.time;
            }
            _replay_builder.next();
            _player_ship.update(_cursor, _entering_message);

            CollideSpaceObjects();
            _decide_cycle = ticks(0);
            _scenario_check_time += kMajorTick;
            if (_scenario_check_time == ticks(90)) {
                _scenario_check_time = ticks(0);
                CheckScenarioConditions();
            }
        }
        unitsPassed -= unitsToDo;
    }

    MiniComputerHandleNull(unitsDone);

    Messages::clip();
    Messages::draw_long_message(unitsDone);

    update_sector_lines();
    Beams::update();
    Label::update_positions(unitsDone);
    Label::update_contents(unitsDone);
    update_site(_replay);

    CullSprites();
    Label::show_all();
    Beams::show_all();
    globals()->starfield.show();

    Messages::draw_message_screen(unitsDone);
    UpdateRadar(unitsDone);
    globals()->transitions.update_boolean(unitsDone);

    if (g.game_over && (g.time >= g.game_over_at)) {
        *_seconds = std::chrono::duration_cast<secs>(g.time.time_since_epoch());

        if (*_game_result == NO_GAME) {
            if (g.victor == g.admiral) {
                *_game_result = WIN_GAME;
            } else {
                *_game_result = LOSE_GAME;
            }
        }
    }

    switch (*_game_result) {
      case QUIT_GAME:
      case RESTART_GAME:
        stack()->pop(this);
        break;

      case WIN_GAME:
        if (_replay || (g.victory_text < 0)) {
            stack()->pop(this);
        } else {
            _state = DEBRIEFING;
            const auto& a = g.admiral;
            stack()->push(new DebriefingScreen(
                        g.victory_text, *_seconds, g.level->parTime,
                        GetAdmiralLoss(a), g.level->parLosses,
                        GetAdmiralKill(a), g.level->parKills));
        }
        break;

      case LOSE_GAME:
        if (_replay || (g.victory_text < 0)) {
            stack()->pop(this);
        } else {
            _state = DEBRIEFING;
            stack()->push(new DebriefingScreen(g.victory_text));
        }
        break;

      case NO_GAME:
        // Continue playing.
        break;
    }
}

void GamePlay::key_down(const KeyDownEvent& event) {
    if (globals()->gInputSource) {
        *_game_result = QUIT_GAME;
        g.game_over = true;
        g.game_over_at = g.time;
        return;
    }

    switch (event.key()) {
      case Keys::CAPS_LOCK:
        _state = PAUSED;
        _player_paused = true;
        stack()->push(new PauseScreen);
        return;

      case Keys::ESCAPE:
        _state = PLAY_AGAIN;
        _player_paused = true;
        stack()->push(new PlayAgainScreen(true, g.level->is_training, &_play_again));
        break;

      default:
        if (event.key() == Preferences::preferences()->key(kHelpKeyNum) - 1) {
            _state = HELP;
            _player_paused = true;
            stack()->push(new HelpScreen);
        } else if (event.key() == Preferences::preferences()->key(kVolumeDownKeyNum) - 1) {
            Preferences::preferences()->set_volume(Preferences::preferences()->volume() - 1);
            SoundDriver::driver()->set_global_volume(Preferences::preferences()->volume());
        } else if (event.key() == Preferences::preferences()->key(kVolumeUpKeyNum) - 1) {
            Preferences::preferences()->set_volume(Preferences::preferences()->volume() + 1);
            SoundDriver::driver()->set_global_volume(Preferences::preferences()->volume());
        } else if (event.key() == Preferences::preferences()->key(kActionMusicKeyNum) - 1) {
            if (Preferences::preferences()->play_music_in_game()) {
                ToggleSong();
            }
        } else if (event.key() == Preferences::preferences()->key(kFastMotionKeyNum) - 1) {
            _fast_motion = true;
        }
        break;
    }

    _player_ship.key_down(event);
    _replay_builder.key_down(event);
}

void GamePlay::key_up(const KeyUpEvent& event) {
    if (globals()->gInputSource) {
        return;
    }

    if (event.key() == Preferences::preferences()->key(kFastMotionKeyNum) - 1) {
        _fast_motion = false;
    }

    _player_ship.key_up(event);
    _replay_builder.key_up(event);
}

void GamePlay::mouse_down(const MouseDownEvent& event) {
    _cursor.mouse_down(event);

    if (_replay) {
        *_game_result = QUIT_GAME;
        g.game_over = true;
        g.game_over_at = g.time;
        return;
    }

    switch (event.button()) {
        case 0:
            if (event.count() == 2) {
                InstrumentsHandleDoubleClick(_cursor);
            } else if (event.count() == 1) {
                InstrumentsHandleClick(_cursor);
            }
            break;
        case 1:
            if (event.count() == 1) {
                PlayerShipHandleClick(VideoDriver::driver()->get_mouse(), 1);
            }
            break;
    }
}

void GamePlay::mouse_up(const MouseUpEvent& event) {
    _cursor.mouse_up(event);

    if (event.button() == 0) {
        InstrumentsHandleMouseStillDown(_cursor);
        InstrumentsHandleMouseUp(_cursor);
    }
}

void GamePlay::mouse_move(const MouseMoveEvent& event) {
    _cursor.mouse_move(event);
}

void GamePlay::gamepad_button_down(const GamepadButtonDownEvent& event) {
    if (globals()->gInputSource) {
        *_game_result = QUIT_GAME;
        g.game_over = true;
        g.game_over_at = g.time;
        return;
    }

    switch (event.button) {
      case Gamepad::START:
        _state  = PLAY_AGAIN;
        _player_paused = true;
        stack()->push(new PlayAgainScreen(true, g.level->is_training, &_play_again));
        break;
    }

    _player_ship.gamepad_button_down(event);
}

void GamePlay::gamepad_button_up(const GamepadButtonUpEvent& event) {
    if (globals()->gInputSource) {
        return;
    }

    _player_ship.gamepad_button_up(event);
}

void GamePlay::gamepad_stick(const GamepadStickEvent& event) {
    _player_ship.gamepad_stick(event);
}

}  // namespace antares

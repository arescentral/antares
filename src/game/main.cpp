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

Rect world;
Rect play_screen;
Rect viewport;

#ifdef DATA_COVERAGE
extern set<int32_t> covered_objects;
extern set<int32_t> covered_actions;
#endif  // DATA_COVERAGE

class GamePlay : public Card {
  public:
    GamePlay(
            bool replay, ReplayBuilder& replay_builder, GameResult* game_result, int32_t* seconds);

    virtual void become_front();
    virtual void resign_front();

    virtual void draw() const;

    virtual bool next_timer(int64_t& time);
    virtual void fire_timer();

    virtual void key_down(const KeyDownEvent& event);
    virtual void key_up(const KeyUpEvent& event);
    virtual void caps_lock(const CapsLockEvent& event);

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
    int32_t* const _seconds;
    int64_t _next_timer;
    const Rect _play_area;
    const int64_t _scenario_start_time;
    const bool _command_and_q;
    bool _left_mouse_down;
    bool _right_mouse_down;
    bool _entering_message;
    bool _player_paused;
    KeyMap _key_map;
    KeyMap _last_key_map;
    uint32_t _decide_cycle;
    int64_t _last_click_time;
    int _scenario_check_time;
    PlayAgainScreen::Item _play_again;
    PlayerShip _player_ship;
    ReplayBuilder& _replay_builder;
};

MainPlay::MainPlay(
        const Scenario* scenario, bool replay, bool show_loading_screen,
        GameResult* game_result, int32_t* seconds):
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
                    String(u32_to_version(globals()->scenarioFileInfo.version)),
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

            ResetInstruments();
            DrawInstrumentPanel();

            if (Preferences::preferences()->play_music_in_game()) {
                LoadSong(gThisScenario->songID);
                SetSongVolume(kMusicVolume);
                PlaySong();
            }
            globals()->gLastTime = now_usecs();

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
            sfz::print(sfz::io::err, sfz::format("{{ \"level\": {0},\n", gThisScenario->chapter_number()));
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
        bool replay, ReplayBuilder& replay_builder, GameResult* game_result, int32_t* seconds):
        _state(PLAYING),
        _replay(replay),
        _game_result(game_result),
        _seconds(seconds),
        _next_timer(add_ticks(now_usecs(), 1)),
        _play_area(viewport.left, viewport.top, viewport.right, viewport.bottom),
        _scenario_start_time(add_ticks(
                    0,
                    (gThisScenario->startTime & kScenario_StartTimeMask)
                    * kScenarioTimeMultiple)),
        _command_and_q(BothCommandAndQ()),
        _left_mouse_down(false),
        _right_mouse_down(false),
        _entering_message(false),
        _player_paused(false),
        _decide_cycle(0),
        _last_click_time(0),
        _scenario_check_time(0),
        _replay_builder(replay_builder) { }

class PauseScreen : public Card {
  public:
    PauseScreen() {
        const StringList list(3100);
        _pause_string.assign(list.at(10));
        int32_t width = title_font->string_width(_pause_string);
        Rect bounds(0, 0, width, title_font->height);
        bounds.center_in(play_screen);
        _text_origin = Point(bounds.left, bounds.top + title_font->ascent);

        bounds.inset(-4, -4);
        _bracket_bounds = bounds;
    }

    virtual void become_front() {
        // TODO(sfiera): cancel any active transition.
        PlayVolumeSound(kComputerBeep4, kMaxSoundVolume, kShortPersistence, kMustPlaySound);
        _visible = true;
        _next_switch = now_usecs() + kSwitchAfter;
        _sleep_at = now_usecs() + kSleepAfter;
    }

    virtual void mouse_up(const MouseUpEvent& event) { wake(); }
    virtual void mouse_down(const MouseDownEvent& event) { wake(); }
    virtual void mouse_move(const MouseMoveEvent& event) { wake(); }
    virtual void key_up(const KeyUpEvent& event) { wake(); }
    virtual void key_down(const KeyDownEvent& event) { wake(); }

    virtual void caps_unlock(const CapsUnlockEvent& event) {
        stack()->pop(this);
    }

    virtual bool next_timer(int64_t& time) {
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
            Rects().fill(world, RgbColor(63, 0, 0, 0));
        }
    }

  private:
    void show_hide() {
        const int64_t now = now_usecs();
        while (_next_switch < now) {
            _visible = !_visible;
            _next_switch += kSwitchAfter;
        }
    }

    bool asleep() const {
        return _sleep_at < now_usecs();
    }

    void wake() {
        _sleep_at = now_usecs() + kSleepAfter;
    }

    static const int64_t kSwitchAfter = 1000000 / 3;
    static const int64_t kSleepAfter = 60 * 1000000;

    bool _visible;
    int64_t _next_switch;
    int64_t _sleep_at;
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

        CheckScenarioConditions(0);
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
            g.next_level = gThisScenario->chapter_number() + 1;
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
    if ((_state == PLAYING) && !globals()->gInputSource) {
        KeyMap keys;
        VideoDriver::driver()->get_keys(&keys);
        _player_ship.update_keys(keys);
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

bool GamePlay::next_timer(int64_t& time) {
    if (_state == PLAYING) {
        time = _next_timer;
        return true;
    }
    return false;
}

void GamePlay::fire_timer() {
    uint64_t thisTime;
    uint64_t scrapTime;

    while (_next_timer < now_usecs()) {
        _next_timer = add_ticks(_next_timer, 1);
    }

    thisTime = now_usecs();
    scrapTime = thisTime;
    thisTime -= globals()->gLastTime;
    int64_t newGameTime = thisTime + _scenario_start_time;

    if ((mNOFFastMotionKey(_key_map)) && !_entering_message) {
        newGameTime = add_ticks(g.time, 12);
        thisTime = newGameTime - _scenario_start_time;
        globals()->gLastTime = scrapTime - thisTime;
    }

    int unitsPassed = usecs_to_ticks(newGameTime - g.time);
    int unitsDone = unitsPassed;

    if (unitsPassed <= 0) {
        return;
    }

    globals()->starfield.prepare_to_move();
    EraseSite();

    if (_player_paused) {
        _player_paused = false;
        unitsDone = unitsPassed = 0;
        newGameTime = g.time;
        thisTime = newGameTime - _scenario_start_time;
        globals()->gLastTime = scrapTime - thisTime;
    }

    while (unitsPassed > 0) {
        int unitsToDo = unitsPassed;
        if (unitsToDo > kMaxTimePerCycle) {
            unitsToDo = kMaxTimePerCycle;
        }
        if ((_decide_cycle + unitsToDo) > kDecideEveryCycles) {
            unitsToDo = kDecideEveryCycles - _decide_cycle;
        }
        _decide_cycle += unitsToDo;

        if (unitsToDo > 0) {
            // executed arbitrarily, but at least once every kDecideEveryCycles
            globals()->starfield.move(unitsToDo);
            MoveSpaceObjects(unitsToDo);
        }

        g.time = add_ticks(g.time, unitsToDo);

        if ( _decide_cycle == kDecideEveryCycles) {
            // everything in here gets executed once every kDecideEveryCycles
            _player_paused = false;

            NonplayerShipThink( kDecideEveryCycles);
            AdmiralThink();
            execute_action_queue( kDecideEveryCycles);

            if (globals()->gInputSource && !globals()->gInputSource->next(_player_ship)) {
                g.game_over = true;
                g.game_over_at = g.time;
            }
            _replay_builder.next();
            _player_ship.update(kDecideEveryCycles, _cursor, _entering_message);

            if (VideoDriver::driver()->button(0)) {
                if (_replay) {
                    *_game_result = QUIT_GAME;
                    g.game_over = true;
                    g.game_over_at = g.time;
                } else {
                    if (!_left_mouse_down) {
                        int64_t double_click_interval
                            = VideoDriver::driver()->double_click_interval_usecs();
                        if ((g.time - _last_click_time) <= double_click_interval) {
                            InstrumentsHandleDoubleClick(_cursor);
                            _last_click_time -= double_click_interval;
                        } else {
                            InstrumentsHandleClick(_cursor);
                            _last_click_time = g.time;
                        }
                        _left_mouse_down = true;
                    } else {
                        InstrumentsHandleMouseStillDown(_cursor);
                    }
                }
            } else if (_left_mouse_down) {
                _left_mouse_down = false;
                InstrumentsHandleMouseUp(_cursor);
            }

            if (VideoDriver::driver()->button(1)) {
                if (_replay) {
                    *_game_result = QUIT_GAME;
                    g.game_over = true;
                    g.game_over_at = g.time;
                } else {
                    if (!_right_mouse_down) {
                        PlayerShipHandleClick(VideoDriver::driver()->get_mouse(), 1);
                        _right_mouse_down = true;
                    }
                }
            } else if (_right_mouse_down) {
                _right_mouse_down = false;
            }

            CollideSpaceObjects();
            _decide_cycle = 0;
            _scenario_check_time++;
            if (_scenario_check_time == 30) {
                _scenario_check_time = 0;
                CheckScenarioConditions( 0);
            }
        }
        unitsPassed -= unitsToDo;
    }

    bool newKeyMap = false;
    _last_key_map.copy(_key_map);
    VideoDriver::driver()->get_keys(&_key_map);
    newKeyMap = (_last_key_map != _key_map);

    if (!_replay && mVolumeDownKey(_key_map) && !mVolumeDownKey(_last_key_map)) {
        Preferences::preferences()->set_volume(Preferences::preferences()->volume() - 1);
        SoundDriver::driver()->set_global_volume(Preferences::preferences()->volume());
    }

    if (!_replay && mVolumeUpKey(_key_map) && !mVolumeUpKey(_last_key_map)) {
        Preferences::preferences()->set_volume(Preferences::preferences()->volume() + 1);
        SoundDriver::driver()->set_global_volume(Preferences::preferences()->volume());
    }

    if (!_replay && mActionMusicKey(_key_map) && !mActionMusicKey(_last_key_map)) {
        if (Preferences::preferences()->play_music_in_game()) {
            ToggleSong();
        }
    }

    MiniComputerHandleNull(unitsDone);

    Messages::clip();
    Messages::draw_long_message( unitsDone);

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
        thisTime = now_usecs();
        thisTime -= globals()->gLastTime;
        *_seconds = thisTime / 1000000; // divide by a million to get seconds

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
        if (_replay || (g.victory_text >= 0)) {
            stack()->pop(this);
        } else {
            _state = DEBRIEFING;
            const auto& a = g.admiral;
            stack()->push(new DebriefingScreen(
                        g.victory_text, *_seconds, gThisScenario->parTime,
                        GetAdmiralLoss(a), gThisScenario->parLosses,
                        GetAdmiralKill(a), gThisScenario->parKills));
        }
        break;

      case LOSE_GAME:
        if (_replay || (g.victory_text >= 0)) {
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

void GamePlay::caps_lock(const CapsLockEvent& event) {
    _state = PAUSED;
    _player_paused = true;
    stack()->push(new PauseScreen);
}

void GamePlay::key_down(const KeyDownEvent& event) {
    if (globals()->gInputSource) {
        *_game_result = QUIT_GAME;
        g.game_over = true;
        g.game_over_at = g.time;
        return;
    }

    switch (event.key()) {
      case Keys::ESCAPE:
        {
            _state = PLAY_AGAIN;
            _player_paused = true;
            bool is_training = gThisScenario->startTime & kScenario_IsTraining_Bit;
            stack()->push(new PlayAgainScreen(true, is_training, &_play_again));
        }
        break;

      default:
        if (event.key() == Preferences::preferences()->key(kHelpKeyNum) - 1) {
            _state = HELP;
            _player_paused = true;
            stack()->push(new HelpScreen);
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

    _player_ship.key_up(event);
    _replay_builder.key_up(event);
}

void GamePlay::mouse_down(const MouseDownEvent& event) {
    _cursor.mouse_down(event);
}

void GamePlay::mouse_up(const MouseUpEvent& event) {
    _cursor.mouse_up(event);
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
        {
            _state  = PLAY_AGAIN;
            _player_paused = true;
            bool is_training = gThisScenario->startTime & kScenario_IsTraining_Bit;
            stack()->push(new PlayAgainScreen(true, is_training, &_play_again));
        }
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

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

#include "game/main.hpp"

#include <fcntl.h>
#include <math.h>
#include <algorithm>
#include <pn/file>
#include <set>

#include "config/gamepad.hpp"
#include "config/keys.hpp"
#include "config/preferences.hpp"
#include "data/plugin.hpp"
#include "data/replay.hpp"
#include "data/resource.hpp"
#include "data/scenario-list.hpp"
#include "drawing/color.hpp"
#include "drawing/shapes.hpp"
#include "drawing/sprite-handling.hpp"
#include "drawing/text.hpp"
#include "game/action.hpp"
#include "game/admiral.hpp"
#include "game/condition.hpp"
#include "game/cursor.hpp"
#include "game/globals.hpp"
#include "game/input-source.hpp"
#include "game/instruments.hpp"
#include "game/labels.hpp"
#include "game/level.hpp"
#include "game/messages.hpp"
#include "game/minicomputer.hpp"
#include "game/motion.hpp"
#include "game/non-player-ship.hpp"
#include "game/player-ship.hpp"
#include "game/starfield.hpp"
#include "game/sys.hpp"
#include "game/time.hpp"
#include "game/vector.hpp"
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

using std::max;
using std::min;
using std::set;
using std::unique_ptr;

namespace antares {

#ifdef DATA_COVERAGE
extern set<int32_t> covered_objects;
extern set<int32_t> covered_actions;
#endif  // DATA_COVERAGE

Rect world() { return Rect({0, 0}, sys.video->screen_size()); }

Rect play_screen() {
    const Size size = sys.video->screen_size();
    return Rect(kLeftPanelWidth, 0, size.width - kRightPanelWidth, size.height);
}

Rect viewport() {
    const Size size = sys.video->screen_size();
    return Rect(kLeftPanelWidth, 0, size.width - kRightPanelWidth, size.height - g.bottom_border);
}

class GamePlay : public Card {
  public:
    GamePlay(bool replay, InputSource* input, GameResult* game_result);

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

    const bool            _replay;
    GameResult* const     _game_result;
    wall_time             _next_timer;
    const Rect            _play_area;
    const bool            _command_and_q;
    bool                  _fast_motion;
    bool                  _entering_message;
    bool                  _player_paused;
    PlayAgainScreen::Item _play_again;
    PlayerShip            _player_ship;

    // The wall_time that g.time corresponds to. Under normal operation,
    // this increases in lockstep with g.time, but during fast motion or
    // paused games, it tracks now() without regard for the in-game
    // clock.
    wall_time _real_time;

    InputSource* _input_source;
};

MainPlay::MainPlay(
        const Level& level, bool replay, InputSource* input, bool show_loading_screen,
        GameResult* game_result)
        : _state(NEW),
          _level(level),
          _replay(replay),
          _show_loading_screen(show_loading_screen),
          _cancelled(false),
          _game_result(game_result),
          _input_source(input) {}

void MainPlay::become_front() {
    switch (_state) {
        case NEW: {
            _state = LOADING;
            RemoveAllSpaceObjects();
            g.game_over = false;

            // _replay_builder.init(
            //         sys.prefs->scenario_identifier(),
            //         String(u32_to_version(plug.meta.version)),
            //         *_level->chapter,
            //         g.random.seed);

            sys.music.play(Music::IDLE, Music::briefing_song);

            if (_show_loading_screen) {
                stack()->push(new LoadingScreen(_level, &_cancelled));
                break;
            } else {
                LoadState s = start_construct_level(_level);
                while (!s.done) {
                    construct_level(&s);
                }
            }
        }
        // fall through.

        case LOADING: {
            if (_cancelled) {
                *_game_result = QUIT_GAME;
                stack()->pop(this);
                return;
            }
            if (!_replay) {
                _state = BRIEFING;
                stack()->push(new BriefingScreen(_level, &_cancelled));
                break;
            }
        }
        // fall through

        case BRIEFING: {
            sys.music.stop();

            if (_cancelled) {
                *_game_result = QUIT_GAME;
                stack()->pop(this);
                break;
            }

            _state = PLAYING;

            set_up_instruments();

            if (g.level->base.song.has_value()) {
                sys.music.play(Music::IN_GAME, *g.level->base.song);
            }

            stack()->push(new GamePlay(_replay, _input_source, _game_result));
        } break;

        case PLAYING:
            globals()->transitions.reset();
            sys.sound.stop();
            sys.music.stop();
#ifdef DATA_COVERAGE
            {
                pn::format(stderr, "{{ \"level\": {0},\n", *g.level->base.chapter);
                const char* sep = "";
                pn::format(stderr, "  \"objects\": [");
                for (auto object : covered_objects) {
                    pn::format(stderr, "{0}{1}", sep, object);
                    sep = ", ";
                }
                pn::format(stderr, "],\n");
                covered_objects.clear();

                sep = "";
                pn::format(stderr, "  \"actions\": [");
                for (auto action : covered_actions) {
                    pn::format(stderr, "{0}{1}", sep, action);
                    sep = ", ";
                }
                pn::format(stderr, "]\n");
                pn::format(stderr, "}\n");
                covered_actions.clear();
            }
#endif  // DATA_COVERAGE
            stack()->pop(this);
            break;
    }
}

GamePlay::GamePlay(bool replay, InputSource* input, GameResult* game_result)
        : _state(PLAYING),
          _replay(replay),
          _game_result(game_result),
          _next_timer(now() + kMinorTick),
          _play_area(viewport().left, viewport().top, viewport().right, viewport().bottom),
          _command_and_q(BothCommandAndQ()),
          _fast_motion(false),
          _entering_message(false),
          _player_paused(false),
          _real_time(now()),
          _input_source(input) {}

static const usecs kSwitchAfter = usecs(1000000 / 3);  // TODO(sfiera): ticks(20)
static const usecs kSleepAfter  = secs(60);

class PauseScreen : public Card {
  public:
    PauseScreen() {
        _pause_string = Messages::pause_string().copy();
        int32_t width = sys.fonts.title.string_width(_pause_string);
        Rect    bounds(0, 0, width, sys.fonts.title.height);
        bounds.center_in(play_screen());
        _text_origin = Point(bounds.left, bounds.top + sys.fonts.title.ascent);

        bounds.inset(-4, -4);
        _bracket_bounds = bounds;
    }

    virtual void become_front() {
        // TODO(sfiera): cancel any active transition.
        sys.sound.pause();
        _visible     = true;
        _next_switch = now() + kSwitchAfter;
        _sleep_at    = now() + kSleepAfter;
    }

    virtual void mouse_up(const MouseUpEvent& event) { wake(); }
    virtual void mouse_down(const MouseDownEvent& event) { wake(); }
    virtual void mouse_move(const MouseMoveEvent& event) { wake(); }
    virtual void key_down(const KeyDownEvent& event) { wake(); }

    virtual void key_up(const KeyUpEvent& event) {
        wake();
        if (event.key() == Key::CAPS_LOCK) {
            stack()->pop(this);
        }
    }

    virtual bool next_timer(wall_time& time) {
        time = std::min(_next_switch, _sleep_at);
        return true;
    }

    virtual void fire_timer() { show_hide(); }

    virtual void draw() const {
        next()->draw();
        if (asleep() || _visible) {
            const RgbColor& light_green = GetRGBTranslateColorShade(Hue::GREEN, LIGHTER);
            const RgbColor& dark_green  = GetRGBTranslateColorShade(Hue::GREEN, DARKER);

            {
                Rects rects;
                for (int32_t y = _bracket_bounds.top + 2; y < _bracket_bounds.bottom; y += 2) {
                    rects.fill(
                            {_bracket_bounds.left, y, _bracket_bounds.right, y + 1}, dark_green);
                }
                draw_vbracket(rects, _bracket_bounds, light_green);
            }

            sys.fonts.title.draw(_text_origin, _pause_string, light_green);
        }
        if (asleep()) {
            Rects().fill(world(), rgba(0, 0, 0, 63));
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

    bool asleep() const { return _sleep_at < now(); }

    void wake() { _sleep_at = now() + kSleepAfter; }

    bool       _visible;
    wall_time  _next_switch;
    wall_time  _sleep_at;
    pn::string _pause_string;
    Point      _text_origin;
    Rect       _bracket_bounds;
};

void GamePlay::become_front() {
    switch (_state) {
        case PLAYING:
            _input_source->start();
            if (_replay) {
                _player_ship.cursor().show = false;
            } else {
                _player_ship.cursor().show = true;
            }
            HintLine::reset();

            CheckLevelConditions();
            break;

        case PAUSED:
        case HELP: _state = PLAYING; break;

        case PLAY_AGAIN:
            switch (_play_again) {
                case PlayAgainScreen::QUIT:
                    *_game_result  = QUIT_GAME;
                    g.game_over    = true;
                    g.next_level   = nullptr;
                    g.victory_text = sfz::nullopt;
                    stack()->pop(this);
                    break;

                case PlayAgainScreen::RESTART:
                    *_game_result  = RESTART_GAME;
                    g.game_over    = true;
                    g.next_level   = nullptr;
                    g.victory_text = sfz::nullopt;
                    stack()->pop(this);
                    break;

                case PlayAgainScreen::RESUME: _state = PLAYING; break;

                case PlayAgainScreen::SKIP:
                    *_game_result  = WIN_GAME;
                    g.game_over    = true;
                    g.victor       = g.admiral;
                    g.next_level   = g.level->solo.skip->get();
                    g.victory_text = sfz::nullopt;
                    stack()->pop(this);
                    break;

                default:
                    throw std::runtime_error(
                            pn::format("invalid play again result {0}", stringify(_play_again))
                                    .c_str());
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

void GamePlay::resign_front() { minicomputer_cancel(); }

void GamePlay::draw() const {
    globals()->starfield.draw();
    draw_sector_lines();
    Vectors::draw();
    draw_sprites();
    Label::draw();

    Messages::draw_message();
    draw_site(_player_ship);
    draw_instruments();
    if (stack()->top() == this) {
        _player_ship.cursor().draw();
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

    ticks     unitsPassed = ticks(0);
    wall_time new_now     = now();
    while (_real_time <= (new_now - kMinorTick)) {
        unitsPassed += kMinorTick;
        _real_time += kMinorTick;
    }

    if (_fast_motion && !_entering_message) {
        unitsPassed *= 12;
        _real_time = now();
    }

    if (unitsPassed <= ticks(0)) {
        return;
    }

    EraseSite();

    if (_player_paused) {
        _player_paused = false;
        unitsPassed    = ticks(0);
        _real_time     = now();
    }

    while (unitsPassed > ticks(0)) {
        ticks unitsToDo   = unitsPassed;
        ticks minor_ticks = g.time.time_since_epoch() % kMajorTick;
        if (minor_ticks + unitsToDo > kMajorTick) {
            unitsToDo = kMajorTick - minor_ticks;
        }

        // executed arbitrarily, but at least once every major tick
        globals()->starfield.prepare_to_move();
        globals()->starfield.move(unitsToDo);
        MoveSpaceObjects(unitsToDo);

        g.time += unitsToDo;

        if ((g.time.time_since_epoch() % kMajorTick) == ticks(0)) {
            // everything in here gets executed once every major tick
            _player_paused = false;

            NonplayerShipThink();
            AdmiralThink();
            execute_action_queue();

            if (!_input_source->get(g.admiral, g.time, _player_ship)) {
                g.game_over    = true;
                g.game_over_at = g.time;
            }
            _player_ship.update(_entering_message);

            CollideSpaceObjects();
            if ((g.time.time_since_epoch() % kConditionTick) == ticks(0)) {
                CheckLevelConditions();
            }
        }

        UpdateMiniScreenLines();

        Messages::clip();
        Messages::draw_long_message(unitsToDo);

        update_sector_lines();
        Vectors::update();
        Label::update_positions(unitsToDo);
        Label::update_contents(unitsToDo);
        update_site(_replay);

        CullSprites();
        Label::show_all();
        Vectors::show_all();
        globals()->starfield.show();

        Messages::draw_message_screen(unitsToDo);
        UpdateRadar(unitsToDo);
        globals()->transitions.update_boolean(unitsToDo);

        unitsPassed -= unitsToDo;
    }

    if (g.game_over && (g.time >= g.game_over_at)) {
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
        case RESTART_GAME: stack()->pop(this); break;

        case WIN_GAME:
            if (_replay || !g.victory_text.has_value()) {
                stack()->pop(this);
            } else {
                _state        = DEBRIEFING;
                const auto& a = g.admiral;
                switch (g.level->type()) {
                    case Level::Type::SOLO:
                        stack()->push(new DebriefingScreen(
                                *g.victory_text, g.time, g.level->solo.par.time, GetAdmiralLoss(a),
                                g.level->solo.par.losses, GetAdmiralKill(a),
                                g.level->solo.par.kills));
                        break;

                    default: stack()->push(new DebriefingScreen(*g.victory_text)); break;
                }
            }
            break;

        case LOSE_GAME:
            if (_replay) {
                *_game_result = QUIT_GAME;
                stack()->pop(this);
            } else if (!g.victory_text.has_value()) {
                _state = PLAY_AGAIN;
                stack()->push(new PlayAgainScreen(false, false, &_play_again));
            } else {
                _state = DEBRIEFING;
                stack()->push(new DebriefingScreen(*g.victory_text));
            }
            break;

        case NO_GAME:
            // Continue playing.
            break;
    }
}

void GamePlay::key_down(const KeyDownEvent& event) {
    switch (event.key()) {
        case Key::CAPS_LOCK:
            _state         = PAUSED;
            _player_paused = true;
            stack()->push(new PauseScreen);
            return;

        case Key::ESCAPE:
            if (_replay) {
                break;
            } else {
                _state         = PLAY_AGAIN;
                _player_paused = true;
                stack()->push(new PlayAgainScreen(
                        true,
                        (g.level->type() == Level::Type::SOLO) && g.level->solo.skip.has_value(),
                        &_play_again));
                return;
            }

        default:
            if (event.key() == sys.prefs->key(kHelpKeyNum)) {
                if (_replay) {
                    break;
                } else {
                    _state         = HELP;
                    _player_paused = true;
                    stack()->push(new HelpScreen);
                    return;
                }
            } else if (event.key() == sys.prefs->key(kVolumeDownKeyNum)) {
                sys.prefs->set_volume(sys.prefs->volume());
                sys.audio->set_global_volume(sys.prefs->volume());
                return;
            } else if (event.key() == sys.prefs->key(kVolumeUpKeyNum)) {
                sys.prefs->set_volume(sys.prefs->volume() + 1);
                sys.audio->set_global_volume(sys.prefs->volume());
                return;
            } else if (event.key() == sys.prefs->key(kActionMusicKeyNum)) {
                if (sys.prefs->play_music_in_game()) {
                    sys.music.toggle();
                }
                return;
            } else if (event.key() == sys.prefs->key(kFastMotionKeyNum)) {
                _fast_motion = true;
                return;
            }
    }

    _input_source->key_down(event);
}

void GamePlay::key_up(const KeyUpEvent& event) {
    if (event.key() == sys.prefs->key(kFastMotionKeyNum)) {
        _fast_motion = false;
        return;
    }

    _input_source->key_up(event);
}

void GamePlay::mouse_down(const MouseDownEvent& event) { _input_source->mouse_down(event); }

void GamePlay::mouse_up(const MouseUpEvent& event) { _input_source->mouse_up(event); }

void GamePlay::mouse_move(const MouseMoveEvent& event) { _input_source->mouse_move(event); }

void GamePlay::gamepad_button_down(const GamepadButtonDownEvent& event) {
    switch (event.button) {
        case Gamepad::Button::START:
            if (_replay) {
                break;
            } else {
                _state         = PLAY_AGAIN;
                _player_paused = true;
                stack()->push(new PlayAgainScreen(
                        true,
                        (g.level->type() == Level::Type::SOLO) && g.level->solo.skip.has_value(),
                        &_play_again));
                return;
            }
        default: break;
    }

    _input_source->gamepad_button_down(event);
}

void GamePlay::gamepad_button_up(const GamepadButtonUpEvent& event) {
    _input_source->gamepad_button_up(event);
}

void GamePlay::gamepad_stick(const GamepadStickEvent& event) {
    _input_source->gamepad_stick(event);
}

}  // namespace antares

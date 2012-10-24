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

#include <math.h>
#include <algorithm>

#include "config/keys.hpp"
#include "config/preferences.hpp"
#include "data/string-list.hpp"
#include "drawing/color.hpp"
#include "drawing/offscreen-gworld.hpp"
#include "drawing/shapes.hpp"
#include "drawing/sprite-handling.hpp"
#include "drawing/text.hpp"
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
using sfz::String;
using sfz::StringSlice;
using sfz::format;
using sfz::scoped_ptr;
using sfz::scoped_array;
using std::min;
using std::max;

namespace antares {

Rect world;
Rect play_screen;
Rect viewport;

class GamePlay : public Card {
  public:
    GamePlay(bool replay, GameResult* game_result);

    virtual void become_front();

    virtual void draw() const;

    virtual bool next_timer(int64_t& time);
    virtual void fire_timer();

    virtual void key_down(const KeyDownEvent& event);

  private:
    enum State {
        PLAYING,
        PAUSED,
        PLAY_AGAIN,
        DEBRIEFING,
        HELP,
    };
    State _state;

    const bool _replay;
    GameResult* const _game_result;
    int64_t _next_timer;
    long _seconds;
    const Rect _play_area;
    const int64_t _scenario_start_time;
    const bool _command_and_q;
    bool _mouse_down;
    bool _entering_message;
    bool _player_paused;
    KeyMap _key_map;
    KeyMap _last_key_map;
    uint32_t _decide_cycle;
    int64_t _last_click_time;
    int _scenario_check_time;
    PlayAgainScreen::Item _play_again;
};

Card* AresInit() {
    return new Master;
}

MainPlay::MainPlay(
        const Scenario* scenario, bool replay, bool show_loading_screen,
        GameResult* game_result):
    _state(NEW),
    _scenario(scenario),
    _replay(replay),
    _show_loading_screen(show_loading_screen),
    _cancelled(false),
    _game_result(game_result) { }

void MainPlay::become_front() {
    switch (_state) {
      case NEW:
        {
            _state = LOADING;
            RemoveAllSpaceObjects();
            globals()->gGameOver = 0;

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

            stack()->push(new GamePlay(_replay, _game_result));
        }
        break;

      case PLAYING:
        {
            if (Preferences::preferences()->play_music_in_game()) {
                StopAndUnloadSong();
            }
            stack()->pop(this);
        }
        break;
    }
}

GamePlay::GamePlay(bool replay, GameResult* game_result)
        : _state(PLAYING),
          _replay(replay),
          _game_result(game_result),
          _next_timer(add_ticks(now_usecs(), 1)),
          _seconds(0),
          _play_area(viewport.left, viewport.top, viewport.right, viewport.bottom),
          _scenario_start_time(add_ticks(
                      0,
                      (gThisScenario->startTime & kScenario_StartTimeMask)
                      * kScenarioTimeMultiple)),
          _command_and_q(BothCommandAndQ()),
          _mouse_down(false),
          _entering_message(false),
          _player_paused(false),
          _decide_cycle(0),
          _last_click_time(0),
          _scenario_check_time(0) { }

class PauseScreen : public Card {
  public:
    PauseScreen():
            _visible(false),
            _next_switch(0) {
        const StringList list(3100);
        _pause_string.assign(list.at(10));
        mSetDirectFont(title_font);
        long width = title_font->string_width(_pause_string);
        Rect bounds(0, 0, width, title_font->height);
        bounds.center_in(play_screen);
        _text_origin = Point(bounds.left, bounds.top + title_font->ascent);

        bounds.inset(-4, -4);
        _bracket_bounds = bounds;
    }

    virtual void become_front() {
        // TODO(sfiera): cancel any active transition.
        PlayVolumeSound(kComputerBeep4, kMaxSoundVolume, kShortPersistence, kMustPlaySound);
        show_hide();
    }

    virtual void key_up(const KeyUpEvent& event) {
        if (event.key() == Keys::CAPS_LOCK) {
            stack()->pop(this);
        }
    }

    virtual bool next_timer(int64_t& time) {
        time = _next_switch;
        return true;
    }

    virtual void fire_timer() {
        show_hide();
    }

    virtual void draw() const {
        next()->draw();
        if (_visible) {
            const RgbColor& light_green = GetRGBTranslateColorShade(GREEN, LIGHTER);
            const RgbColor& dark_green = GetRGBTranslateColorShade(GREEN, DARKER);

            for (int32_t y = _bracket_bounds.top + 2; y < _bracket_bounds.bottom; y += 2) {
                VideoDriver::driver()->draw_line(
                        Point(_bracket_bounds.left, y),
                        Point(_bracket_bounds.right - 1, y),
                        dark_green);
            }
            draw_vbracket(_bracket_bounds, light_green);

            mSetDirectFont(title_font);
            gDirectText->draw_sprite(_text_origin, _pause_string, light_green);
        }
    }

  public:
    void show_hide() {
        _visible = !_visible;
        _next_switch = now_usecs() + (1000000 / 3);
    }

    bool _visible;
    int64_t _next_switch;
    String _pause_string;
    Point _text_origin;
    Rect _bracket_bounds;

    DISALLOW_COPY_AND_ASSIGN(PauseScreen);
};

void GamePlay::become_front() {
    switch (_state) {
      case PLAYING:
        SetSpriteCursorTable(500);
        ShowSpriteCursor();
        ResetHintLine();

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
            globals()->gGameOver = 1;
            globals()->gScenarioWinner.next = -1;
            globals()->gScenarioWinner.text = -1;
            stack()->pop(this);
            break;

          case PlayAgainScreen::RESTART:
            *_game_result = RESTART_GAME;
            globals()->gGameOver = 1;
            globals()->gScenarioWinner.next = -1;
            globals()->gScenarioWinner.text = -1;
            stack()->pop(this);
            break;

          case PlayAgainScreen::RESUME:
            _state = PLAYING;
            break;

          case PlayAgainScreen::SKIP:
            *_game_result = WIN_GAME;
            globals()->gGameOver = 1;
            globals()->gScenarioWinner.player = globals()->gPlayerAdmiralNumber;
            globals()->gScenarioWinner.next = gThisScenario->chapter_number() + 1;
            globals()->gScenarioWinner.text = -1;
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

void GamePlay::draw() const {
    globals()->starfield.draw();
    draw_sector_lines();
    draw_beams();
    draw_sprites();
    draw_labels();

    draw_instruments();
    draw_message();
    draw_site();
    draw_cursor();
    draw_hint_line();
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
    const Rect clip_rect = viewport;

    while (_next_timer < now_usecs()) {
        _next_timer = add_ticks(_next_timer, 1);
    }

    thisTime = now_usecs();
    scrapTime = thisTime;
    thisTime -= globals()->gLastTime;
    int64_t newGameTime = thisTime + _scenario_start_time;

    if ((mNOFFastMotionKey(_key_map)) && !_entering_message) {
        newGameTime = add_ticks(globals()->gGameTime, 12);
        thisTime = newGameTime - _scenario_start_time;
        globals()->gLastTime = scrapTime - thisTime;
    }

    int unitsPassed = usecs_to_ticks(newGameTime - globals()->gGameTime);
    int unitsDone = unitsPassed;

    if (unitsPassed <= 0) {
        return;
    }

    globals()->starfield.prepare_to_move();
    EraseSite();

    if (_player_paused) {
        _player_paused = false;
        unitsDone = unitsPassed = 0;
        newGameTime = globals()->gGameTime;
        thisTime = newGameTime - _scenario_start_time;
        globals()->gLastTime = scrapTime - thisTime;
    }

    if (globals()->gGameOver < 0) {
        globals()->gGameOver += unitsPassed;
        if ( globals()->gGameOver == 0)
            globals()->gGameOver = 1;
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
            MoveSpaceObjects(gSpaceObjectData.get(), kMaxSpaceObject, unitsToDo);
        }

        globals()->gGameTime = add_ticks(globals()->gGameTime, unitsToDo);

        if ( _decide_cycle == kDecideEveryCycles) {
            // everything in here gets executed once every kDecideEveryCycles
            _player_paused = false;

            NonplayerShipThink( kDecideEveryCycles);
            AdmiralThink();
            ExecuteActionQueue( kDecideEveryCycles);

            if (!PlayerShipGetKeys(
                        kDecideEveryCycles, *globals()->gInputSource, &_entering_message)) {
                globals()->gGameOver = 1;
            }

            if (VideoDriver::driver()->button()) {
                if (_replay) {
                    *_game_result = QUIT_GAME;
                    globals()->gGameOver = 1;
                } else {
                    if (!_mouse_down) {
                        int64_t double_click_interval
                            = VideoDriver::driver()->double_click_interval_usecs();
                        if ((globals()->gGameTime - _last_click_time) <= double_click_interval) {
                            InstrumentsHandleDoubleClick();
                            _last_click_time -= double_click_interval;
                        } else {
                            InstrumentsHandleClick();
                            _last_click_time = globals()->gGameTime;
                        }
                        _mouse_down = true;
                    } else {
                        InstrumentsHandleMouseStillDown();
                    }
                }
            } else if (_mouse_down) {
                _mouse_down = false;
                InstrumentsHandleMouseUp();
            }

            CollideSpaceObjects(gSpaceObjectData.get(), kMaxSpaceObject);
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

    if (mPauseKey(_key_map)) {
        _state = PAUSED;
        _player_paused = true;
        stack()->push(new PauseScreen);
        return;
    }

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

    ClipToCurrentLongMessage();
    DrawCurrentLongMessage( unitsDone);

    update_sector_lines();
    update_beams();
    update_all_label_positions(unitsDone);
    update_all_label_contents(unitsDone);
    update_site();

    CullSprites();
    ShowAllLabels();
    ShowAllBeams();
    globals()->starfield.show();

    DrawMessageScreen(unitsDone);
    UpdateRadar(unitsDone);
    globals()->transitions.update_boolean(unitsDone);

    if (globals()->gGameOver > 0) {
        thisTime = now_usecs();
        thisTime -= globals()->gLastTime;
        _seconds = thisTime / 1000000; // divide by a million to get seconds

        if (*_game_result == NO_GAME) {
            if (globals()->gScenarioWinner.player == globals()->gPlayerAdmiralNumber) {
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
        if (_replay || (globals()->gScenarioWinner.text == -1)) {
            stack()->pop(this);
        } else {
            _state = DEBRIEFING;
            stack()->push(new DebriefingScreen(
                        globals()->gScenarioWinner.text, _seconds, gThisScenario->parTime,
                        GetAdmiralLoss(0), gThisScenario->parLosses,
                        GetAdmiralKill(0), gThisScenario->parKills));
        }
        break;

      case LOSE_GAME:
        if (_replay || (globals()->gScenarioWinner.text == -1)) {
            stack()->pop(this);
        } else {
            _state = DEBRIEFING;
            stack()->push(new DebriefingScreen(globals()->gScenarioWinner.text));
        }
        break;

      case NO_GAME:
        // Continue playing.
        break;
    }
}

void GamePlay::key_down(const KeyDownEvent& event) {
    if (_replay) {
        switch (event.key()) {
          case Keys::CAPS_LOCK:
            // TODO(sfiera): also F6.
            break;

          default:
            *_game_result = QUIT_GAME;
            globals()->gGameOver = 1;
            return;
        }
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

      case Keys::F1:
        // Help key is hard-coded to F1 at the moment.
        // TODO(sfiera): use the help key configured in preferences.
        _state = HELP;
        _player_paused = true;
        stack()->push(new HelpScreen);
        break;
    }
}

}  // namespace antares

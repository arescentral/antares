// Ares, a tactical space combat game.
// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// 
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

#include "AresMain.hpp"

#include <math.h>
#include <algorithm>

#include "Quickdraw.h"

#include "Admiral.hpp"
#include "AresCheat.hpp"
#include "AresGlobalType.hpp"
#include "AresPreferences.hpp"

#include "Beam.hpp"

#include "Card.hpp"
#include "CardStack.hpp"
#include "ColorTable.hpp"
#include "ColorTranslation.hpp"

#include "Debug.hpp"
#include "DirectText.hpp"

#include "Error.hpp"

#include "FakeDrawing.hpp"
#include "Fakes.hpp"

#include "InputSource.hpp"
#include "Instruments.hpp"
#include "InterfaceHandling.hpp"

#include "HelpScreen.hpp"

#include "KeyCodes.hpp"
#include "KeyMapTranslation.hpp"

#include "MainScreen.hpp"
#include "MathSpecial.hpp"
#include "MessageScreen.hpp"
#include "Minicomputer.hpp"
#include "BriefingScreen.hpp"
#include "Motion.hpp"
#include "Music.hpp"

#include "NateDraw.hpp"
#include "NetSetupScreen.hpp"
#include "NonPlayerShip.hpp"

#include "OffscreenGWorld.hpp"
#include "Options.hpp"

#include "PlayAgainScreen.hpp"
#include "PlayerInterface.hpp"
#include "PlayerShip.hpp"

#include "Randomize.hpp"
#include "Rotation.hpp"

#include "ScenarioMaker.hpp"
#include "ScreenLabel.hpp"
#include "ScrollStars.hpp"
#include "ScrollTextScreen.hpp"
#include "SoundFX.hpp"
#include "SpaceObjectHandling.hpp"
#include "SpriteCursor.hpp"
#include "SpriteHandling.hpp"
#include "StringHandling.hpp"
#include "StringList.hpp"

#include "Time.hpp"
#include "TimeUnit.hpp"
#include "Transitions.hpp"

#include "VideoDriver.hpp"

using sfz::String;
using sfz::scoped_ptr;
using sfz::scoped_array;
using std::min;
using std::max;

namespace antares {

namespace {

const int kReplayResID = 600;

const int kTitleTextScrollWidth = 450;

}  // namespace

extern scoped_array<spaceObjectType> gSpaceObjectData;
extern int32_t gRandomSeed;
extern scenarioType *gThisScenario;
extern PixMap* gActiveWorld;

long            WORLD_WIDTH = 640,
                WORLD_HEIGHT = 480,
                CLIP_LEFT = 128,
                CLIP_TOP = 0,
                CLIP_RIGHT = 608,
                CLIP_BOTTOM = 480,
                gPlayScreenWidth = 480,
                gPlayScreenHeight = 480;

void Pause( long time);
void MainLoop();

class TitleScreenFade : public PictFade {
  public:
    TitleScreenFade(bool* fast)
            : PictFade(502, 2001, fast),
              _fast(fast) { }

  protected:
    virtual double fade_time() const {
        return (*_fast ? 1.0 : 5.0) / 3.0;
    }

    virtual double display_time() const {
        return *_fast ? 1.0 : 5.0;
    }

    virtual bool skip() const {
        return false;
    }

  private:
    bool* _fast;
};

class Master : public Card {
  public:
    Master()
        : _state(START),
          _skipped(false) { }

    virtual void become_front() {
        switch (_state) {
          case START:
            _state = PUBLISHER_PICT;
            // We don't have permission to display the Ambrosia logo.
            // stack()->push(new PictFade(2000, 2000, &_skipped));
            // break;

          case PUBLISHER_PICT:
            _state = EGO_PICT;
            if (!_skipped) {
                stack()->push(new PictFade(2001, 2000, &_skipped));
                break;
            }
            // fall through.

          case EGO_PICT:
            _state = TITLE_SCREEN_PICT;
            stack()->push(new TitleScreenFade(&_skipped));
            break;

          case TITLE_SCREEN_PICT:
            _state = INTRO_SCROLL;
            // TODO(sfiera): prevent the intro screen from displaying on subsequent launches.
            stack()->push(new ScrollTextScreen(5600, 450, 15.0));
            break;

          case INTRO_SCROLL:
            _state = MAIN_SCREEN;
            stack()->push(new MainScreen);
            break;

          case MAIN_SCREEN:
            // When the main screen returns, exit loop.
            stack()->pop(this);
            break;
        }
    }

  private:
    enum State {
        START,
        PUBLISHER_PICT,
        EGO_PICT,
        TITLE_SCREEN_PICT,
        INTRO_SCROLL,
        MAIN_SCREEN,
    };

    State _state;
    bool _skipped;
};

class GamePlay : public Card {
  public:
    GamePlay(bool replay, GameResult* game_result, long* seconds);

    virtual void become_front();

    virtual double next_timer();
    virtual void fire_timer();

    virtual void key_down(const KeyDownEvent& event);

  private:
    enum State {
        PLAYING,
        PAUSED,
        PLAY_AGAIN,
        HELP,
    };
    State _state;

    const bool _replay;
    GameResult* const _game_result;
    long* const _seconds;
    const Rect _play_area;
    const int64_t _scenario_start_time;
    const bool _command_and_q;
    bool _mouse_down;
    bool _entering_message;
    bool _player_paused;
    KeyMap _key_map;
    KeyMap _last_key_map;
    uint32_t _decide_cycle;
    int _last_click_time;
    int _scenario_check_time;
    PlayAgainScreen::Item _play_again;
};

Card* AresInit() {
    RgbColor                initialFadeColor;
    scoped_ptr<ColorTable>  theClut;

    init_globals();
    WORLD_WIDTH = gRealWorld->bounds().right;
    WORLD_HEIGHT = gRealWorld->bounds().bottom;
    CLIP_RIGHT = WORLD_WIDTH - kRightPanelWidth;
    CLIP_BOTTOM = WORLD_HEIGHT;
    gPlayScreenWidth = CLIP_RIGHT - CLIP_LEFT;
    gPlayScreenHeight = CLIP_BOTTOM - CLIP_TOP;

    globals()->gPreferencesData.reset(new Preferences);

    GetDateTime( reinterpret_cast<unsigned long *>(&gRandomSeed));

    theClut.reset(new ColorTable(256));
    gActiveWorld = gRealWorld;
    CreateOffscreenWorld(gRealWorld->bounds(), *theClut);
    ColorTranslatorInit(*theClut);

    InitSpriteCursor();
    InitTransitions();

    initialFadeColor.red = initialFadeColor.green = initialFadeColor.blue = 0;

    gActiveWorld->fill(RgbColor::kBlack);
    ResetTransitions();

    MusicInit();

    RotationInit();
    NormalizeColors();
    DrawInRealWorld();

    InterfaceHandlingInit();

    if (globals()->gPreferencesData->play_idle_music()) {
        LoadSong( kTitleSongID);
        SetSongVolume( kMaxMusicVolume);
        PlaySong();
    }

    InitDirectText();
    ScreenLabelInit();
    InitMessageScreen();
    InitScrollStars();
    InstrumentInit();
    SpriteHandlingInit();
    AresCheatInit();
    ScenarioMakerInit();
    SpaceObjectHandlingInit();  // MUST be after ScenarioMakerInit()
    InitSoundFX();
    InitMotion();
    AdmiralInit();
    InitBeams();

    return new Master;
}

MainPlay::MainPlay(int scenario, bool replay, GameResult* game_result, long* game_length)
        : _state(NEW),
          _scenario(scenario),
          _replay(replay),
          _cancelled(false),
          _game_result(game_result),
          _game_length(game_length) { }

void MainPlay::become_front() {
    switch (_state) {
      case NEW:
        {
            _state = FADING_OUT;
            *_game_result = NO_GAME;
            stack()->push(new ColorFade(256, ColorFade::TO_COLOR, RgbColor::kBlack, 1.0, false, NULL));
        }
        break;

      case FADING_OUT:
        {
            _state = LOADING;
            ColorTable colors(256);
            RestoreEntries(colors);
            RemoveAllSpaceObjects();
            globals()->gGameOver = 0;

            if (globals()->gPreferencesData->play_idle_music()) {
                LoadSong(3000);
                SetSongVolume( kMaxMusicVolume);
                PlaySong();
            }

            // TODO(sfiera): implement as a Card.
            if (!ConstructScenario(_scenario)) {
                *_game_result = QUIT_GAME;
                stack()->pop(this);
                return;
            }
            if (!_replay) {
                stack()->push(new BriefingScreen(_scenario, &_cancelled));
                break;
            }
        }
        // fall through.

      case LOADING:
        // fall through

      case BRIEFING:
        {
            if (_cancelled) {
                *_game_result = QUIT_GAME;
                stack()->pop(this);
                break;
            }

            _state = PLAYING;
            if (globals()->gPreferencesData->play_idle_music()) {
                StopAndUnloadSong();
            }

            ResetInstruments();
            DrawInstrumentPanel();

            if (globals()->gPreferencesData->play_music_in_game()) {
                LoadSong(gThisScenario->songID);
                SetSongVolume(kMusicVolume);
                PlaySong();
            }
            globals()->gLastTime = now_usecs();
            globals()->gGameStartTime = TickCount();// - defecit;

            VideoDriver::driver()->set_game_state(PLAY_GAME);
            stack()->push(new GamePlay(_replay, _game_result, _game_length));
        }
        break;

      case PLAYING:
        {
            VideoDriver::driver()->set_game_state(DONE_GAME);
            if (globals()->gPreferencesData->play_music_in_game()) {
                StopAndUnloadSong();
            }
            stack()->pop(this);
        }
        break;
    }
}

GamePlay::GamePlay(bool replay, GameResult* game_result, long* seconds)
        : _state(PLAYING),
          _replay(replay),
          _game_result(game_result),
          _seconds(seconds),
          _play_area(CLIP_LEFT, CLIP_TOP, CLIP_RIGHT, CLIP_BOTTOM),
          _scenario_start_time(
                  (gThisScenario->startTime & kScenario_StartTimeMask) * kScenarioTimeMultiple),
          _command_and_q(BothCommandAndQ()),
          _mouse_down(false),
          _entering_message(false),
          _player_paused(false),
          _decide_cycle(0),
          _last_click_time(0),
          _scenario_check_time(0) {
    globals()->gLastKeys = globals()->gTheseKeys = 0;

    globals()->gFrameCount = 0;
}

class PauseScreen : public Card {
  public:
    PauseScreen()
            : _visible(false),
              _next_switch(0.0) {
        StringList list(3100);
        _text.assign(list.at(10));
    }

    virtual void become_front() {
        // TODO(sfiera): cancel any active transition.
        PlayVolumeSound(kComputerBeep4, kMaxSoundVolume, kShortPersistence, kMustPlaySound);
        show_hide();
    }

    virtual void resign_front() {
        if (_visible) {
            StopPauseIndicator(_text);
        }
    }

    virtual void key_up(const KeyUpEvent& event) {
        if (event.key() == 0x3900) {
            stack()->pop(this);
        }
    }

    virtual double next_timer() {
        return _next_switch;
    }

    virtual void fire_timer() {
        show_hide();
    }

  public:
    void show_hide() {
        _visible = !_visible;
        _next_switch = now_secs() + (1.0 / 3.0);

        if (_visible) {
            StartPauseIndicator(_text, Randomize(16));
        } else {
            StopPauseIndicator(_text);
        }
    }

    String _text;
    bool _visible;
    double _next_switch;
};

void GamePlay::become_front() {
    switch (_state) {
      case PLAYING:
        SetSpriteCursorTable(500);
        ShowSpriteCursor(true);
        ResetHintLine();

        HideCursor();

        CheckScenarioConditions(0);
        break;

      case PAUSED:
      case HELP:
        _state = PLAYING;
        break;
        
      case PLAY_AGAIN:
        _state = PLAYING;
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
            break;

          case PlayAgainScreen::SKIP:
            *_game_result = WIN_GAME;
            globals()->gGameOver = 1;
            globals()->gScenarioWinner.player = globals()->gPlayerAdmiralNumber;
            globals()->gScenarioWinner.next =
                GetChapterNumberFromScenarioNumber(globals()->gThisScenarioNumber) + 1;
            globals()->gScenarioWinner.text = -1;
            stack()->pop(this);
            break;

          default:
            fail("invalid play again result %d", _play_again);
        }
        break;
    }
}

double GamePlay::next_timer() {
    if (_state == PLAYING) {
        return ceil(now_secs() * 60.0) / 60.0;
    } else {
        return 0.0;
    }
}

void GamePlay::fire_timer() {
    uint64_t thisTime;
    uint64_t scrapTime;
    int newGameTime;
    Rect clipRect;

    thisTime = now_usecs();
    scrapTime = thisTime;
    thisTime -= globals()->gLastTime;
    newGameTime = (thisTime / kTimeUnit) + _scenario_start_time;

    if ((mNOFFastMotionKey(_key_map)) && !_entering_message) {
        newGameTime = globals()->gGameTime + 12;
        thisTime = (newGameTime - _scenario_start_time) * kTimeUnit;
        globals()->gLastTime = scrapTime - thisTime;
    }

    int unitsPassed = newGameTime - globals()->gGameTime;
    int unitsDone = unitsPassed;

    if (unitsPassed <= 0) {
        return;
    }

    EraseSpriteCursorSprite();
    EraseSpriteTable();
    EraseAllLabels();
    EraseSectorLines();
    PrepareToMoveScrollStars();
    EraseSite();

    if (_player_paused) {
        _player_paused = false;
        unitsDone = unitsPassed = 0;
        newGameTime = globals()->gGameTime;
        thisTime = (newGameTime - _scenario_start_time) * kTimeUnit;
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
            MoveScrollStars(unitsToDo);
            MoveSpaceObjects(gSpaceObjectData.get(), kMaxSpaceObject, unitsToDo);
        }

        globals()->gGameTime += unitsToDo;

        if ( _decide_cycle == kDecideEveryCycles) {
            // everything in here gets executed once every kDecideEveryCycles
            _player_paused = false;

            NonplayerShipThink( kDecideEveryCycles);
            AdmiralThink();
            ExecuteActionQueue( kDecideEveryCycles);

            if (_replay) {
                uint32_t keys;
                if (!globals()->gInputSource->next(&keys)) {
                    globals()->gGameOver = 1;
                }

                if (!_player_paused) {
                    _player_paused = PlayerShipGetKeys(
                            kDecideEveryCycles, keys, &_entering_message);
                } else {
                    PlayerShipGetKeys( kDecideEveryCycles, keys, &_entering_message);
                }
            } else {
                if (!_player_paused) {
                    _player_paused = PlayerShipGetKeys(
                            kDecideEveryCycles, 0xffffffff, &_entering_message);
                } else {
                    PlayerShipGetKeys(kDecideEveryCycles, 0xffffffff, &_entering_message);
                }
            }

            if (Button()) {
                if (_replay) {
                    *_game_result = QUIT_GAME;
                    globals()->gGameOver = 1;
                } else {
                    if (!_mouse_down) {
                        if (((globals()->gGameTime - _last_click_time)) <= now_secs()) {
                            InstrumentsHandleDoubleClick();
                            _last_click_time -= now_secs();
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
    GetKeys(&_key_map);
    newKeyMap = (_last_key_map != _key_map);

    if (mPauseKey(_key_map)) {
        _state = PAUSED;
        _player_paused = true;
        stack()->push(new PauseScreen);
        return;
    }

    if (!_replay
            && mVolumeDownKey(_key_map)
            && !mVolumeDownKey(_last_key_map)) {
        globals()->gPreferencesData->set_volume(
                max(0, globals()->gPreferencesData->volume() - 1));
        if ( globals()->gPreferencesData->play_music_in_game()) {
            SetSongVolume(kMusicVolume);
        }
    }

    if (!_replay
            && mVolumeUpKey(_key_map)
            && !mVolumeUpKey(_last_key_map)) {
        globals()->gPreferencesData->set_volume(
                min(kMaxVolumePreference, globals()->gPreferencesData->volume() + 1));
        if (globals()->gPreferencesData->play_music_in_game()) {
            SetSongVolume( kMusicVolume);
        }
    }

    if (!_replay
            && mActionMusicKey(_key_map)
            && !mActionMusicKey(_last_key_map)) {
        if (globals()->gPreferencesData->play_music_in_game()) {
            ToggleSong();
        }
    }

    MiniComputerHandleNull(unitsDone);

    ClipToCurrentLongMessage();
    clipRect = Rect(CLIP_LEFT, CLIP_TOP, CLIP_RIGHT, CLIP_BOTTOM);
    DrawScrollStars(true);
    DrawCurrentLongMessage( unitsDone);

    DrawSectorLines();
    DrawAllBeams();
    DrawSpriteTableInOffWorld(&clipRect);
    UpdateAllLabelPositions(unitsDone);
    DrawAllLabels();
    DrawSite();
    clipRect = Rect(0, 0, WORLD_WIDTH, WORLD_HEIGHT);
    DrawSpriteCursorSprite(&clipRect);

    ShowSpriteCursorSprite();
    ShowSpriteTable();
    ShowAllLabels();
    ShowAllBeams();
    ShowScrollStars( true);
    ShowSectorLines();
    ShowSite();
    CopyOffWorldToRealWorld(_play_area);

    DrawMessageScreen(unitsDone);
    UpdateRadar(unitsDone);
    UpdateBooleanColorAnimation(unitsDone);

    ++globals()->gFrameCount;
    VideoDriver::driver()->main_loop_iteration_complete(globals()->gGameTime);

    if (globals()->gGameOver > 0) {
        thisTime = now_usecs();
        thisTime -= globals()->gLastTime;
        newGameTime = thisTime / 1000000; // divide by a million to get seconds
        *_seconds = newGameTime;
        RestoreOriginalColors();

        if (*_game_result == NO_GAME) {
            if (globals()->gScenarioWinner.player == globals()->gPlayerAdmiralNumber) {
                *_game_result = WIN_GAME;
            } else {
                *_game_result = LOSE_GAME;
            }
        }
    }

    if (*_game_result != NO_GAME) {
        stack()->pop(this);
    }
}

void GamePlay::key_down(const KeyDownEvent& event) {
    if (_replay) {
        switch (event.key()) {
          case 0x39:  // Caps lock.
            // TODO(sfiera): also F6.
            break;

          default:
            *_game_result = QUIT_GAME;
            globals()->gGameOver = 1;
            return;
        }
    }

    switch (event.key()) {
      case 0x35:
        {
            _state = PLAY_AGAIN;
            _player_paused = true;
            bool is_training = gThisScenario->startTime & kScenario_IsTraining_Bit;
            stack()->push(new PlayAgainScreen(true, is_training, &_play_again));
        }
        break;

      case 0x7A:
        // Help key is hard-coded to F1 at the moment.
        // TODO(sfiera): use the help key configured in preferences.
        _state = HELP;
        _player_paused = true;
        stack()->push(new HelpScreen);
        break;
    }
}

void Pause( long time)

{
    long    starttime = TickCount();

    while (( TickCount() - starttime) < time) {
        // DO NOTHING
    }
}

}  // namespace antares

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

#include "SoloGame.hpp"

#include "Admiral.hpp"
#include "AresGlobalType.hpp"
#include "AresPreferences.hpp"
#include "AresMain.hpp"
#include "ColorTranslation.hpp"
#include "DirectText.hpp"
#include "ScenarioMaker.hpp"
#include "SelectLevelScreen.hpp"
#include "Music.hpp"
#include "Options.hpp"
#include "Transitions.hpp"

namespace antares {

extern PixMap* gActiveWorld;
extern scenarioType* gThisScenario;

SoloGame::SoloGame()
        : _state(NEW),
          _game_result(NO_GAME) { }

SoloGame::~SoloGame() { }

void SoloGame::become_front() {
    switch (_state) {
      case NEW:
        _state = SELECT_LEVEL;
        _select_level.reset(new SelectLevelScreen);
        VideoDriver::driver()->push_listener(_select_level.get());
        break;

      case SELECT_LEVEL:
        _state = PLAYING;
        _scenario = GetScenarioNumberFromChapterNumber(_select_level->chapter());
        _select_level.reset();
        StopAndUnloadSong();
        start_main_play();
        break;

      case PLAYING:
        VideoDriver::driver()->pop_listener(this);
        start_main_play();
        break;
    }
}

void SoloGame::start_main_play() {
    if ((_game_result == QUIT_GAME)
            || (GetChapterNumberFromScenarioNumber(_scenario) > kHackLevelMax)
            || (GetChapterNumberFromScenarioNumber(_scenario) > GetScenarioNumber())
            || (_scenario < 0)) {
        VideoDriver::driver()->pop_listener(this);
        return;
    }

    long game_length;

    if ((_game_result == NO_GAME) || (_game_result == WIN_GAME)) {
        if (GetScenarioPrologueID(_scenario) > 0) {
            DoScrollText(
                    GetScenarioPrologueID(_scenario), 4, 450, kTitleFontNum, 4002);
        }
    }

    _game_result = NO_GAME;

    MainPlay(_scenario, &_game_result, &game_length);

    switch (_game_result) {
      case LOSE_GAME:
        if (globals()->gScenarioWinner.text != -1) {
            DoMissionDebriefingText(
                    globals()->gScenarioWinner.text, -1, -1, -1, -1, -1, -1, -1);
        }
        switch (DoPlayAgain(false, false)) {
          case PLAY_AGAIN_RESTART:
            _game_result = RESTART_GAME;
            break;

          case PLAY_AGAIN_QUIT:
            _game_result = QUIT_GAME;
            break;

          default:
            fprintf(stderr, "DoPlayAgain(false, false) returned bad value\n");
            exit(1);
        }
        break;

      case WIN_GAME:
        {
            if (globals()->gScenarioWinner.text != -1) {
                DoMissionDebriefingText(
                        globals()->gScenarioWinner.text,
                        game_length, gThisScenario->parTime,
                        GetAdmiralLoss(0), gThisScenario->parLosses,
                        GetAdmiralKill(0), gThisScenario->parKills,
                        100);
            }

            RGBColor black = {0, 0, 0};
            if (globals()->gOptions & kOptionMusicPlay) {
                AutoMusicFadeTo(60, &black, false);
                StopAndUnloadSong();
            } else {
                AutoFadeTo(60, &black, false);
            }
            gActiveWorld->fill(BLACK);
            AutoFadeFrom(1, false);

            // normal scrolltext song
            int scroll_song = 4002;
            if (globals()->gScenarioWinner.next == -1) {
                // we win but no next level? Play triumph song
                scroll_song = 4003;
            }

            if (GetScenarioEpilogueID(_scenario) > 0) {
                DoScrollText(
                        GetScenarioEpilogueID(_scenario), 4, 450, kTitleFontNum,
                        scroll_song);
            }

            if (globals()->gOptions & kOptionMusicIdle) {
                gActiveWorld->fill(BLACK);
                StopAndUnloadSong();
            }

            if (globals()->gScenarioWinner.next == -1) {
                _scenario = -1;
            } else {
                _scenario = GetScenarioNumberFromChapterNumber(globals()->gScenarioWinner.next);
            }

            if (_scenario <= GetScenarioNumber() && _scenario >= 0) {
                int chapter = GetChapterNumberFromScenarioNumber(_scenario);
                if ((chapter >= 0) && (chapter <= kHackLevelMax)) {
                    SaveStartingLevelPreferences(chapter);
                } else {
                    _scenario = 0;
                }
            }
        }
        break;

      default:
        break;
    }

    become_front();
}

}  // namespace antares

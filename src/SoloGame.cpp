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
#include "CardStack.hpp"
#include "ColorTranslation.hpp"
#include "DirectText.hpp"
#include "ScenarioMaker.hpp"
#include "SelectLevelScreen.hpp"
#include "Music.hpp"
#include "Options.hpp"
#include "ScrollTextScreen.hpp"
#include "Transitions.hpp"

namespace antares {

extern PixMap* gActiveWorld;
extern scenarioType* gThisScenario;

SoloGame::SoloGame()
        : _state(NEW),
          _game_result(NO_GAME),
          _game_length(0) { }

SoloGame::~SoloGame() { }

void SoloGame::become_front() {
    switch (_state) {
      case NEW:
        _state = SELECT_LEVEL;
        _next_card.reset(new SelectLevelScreen(&_cancelled, &_scenario));
        stack()->push(_next_card.get());
        break;

      case SELECT_LEVEL:
        _state = START_LEVEL;
        if (_cancelled) {
            _state = QUIT;
            stack()->pop(this);
            break;
        }
        // else fall through.

      case START_LEVEL:
        _state = PROLOGUE;
        if (GetScenarioPrologueID(_scenario) > 0) {
            _next_card.reset(new ScrollTextScreen(
                        GetScenarioPrologueID(_scenario), 450, 15.0, 4002));
            stack()->push(_next_card.get());
            break;
        }
        // else fall through

      case PROLOGUE:
      case RESTART_LEVEL:
        _state = PLAYING;
        _game_result = NO_GAME;
        _game_length = 0;
        _next_card.reset(new MainPlay(_scenario, &_game_result, &_game_length));
        stack()->push(_next_card.get());
        break;

      case PLAYING:
        handle_game_result();
        // TODO(sfiera): make less hacky.
        become_front();
        break;

      case QUIT:
        stack()->pop(this);
        break;
    }
}

void SoloGame::handle_game_result() {
    switch (_game_result) {
      case LOSE_GAME:
        if (globals()->gScenarioWinner.text != -1) {
            DoMissionDebriefingText(
                    globals()->gScenarioWinner.text, -1, -1, -1, -1, -1, -1, -1);
        }
        switch (DoPlayAgain(false, false)) {
          case PLAY_AGAIN_RESTART:
            _state = RESTART_LEVEL;
            break;

          case PLAY_AGAIN_QUIT:
            _state = QUIT;
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
                        _game_length, gThisScenario->parTime,
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

            if (_scenario >= 0) {
                _state = START_LEVEL;
            } else {
                _state = QUIT;
            }
        }
        break;

      case RESTART_GAME:
        _state = RESTART_LEVEL;
        break;

      case QUIT_GAME:
        _state = QUIT;
        break;

      default:
        fprintf(stderr, "MainPlay() resulted in bad game_result\n");
        exit(1);
    }
}

}  // namespace antares

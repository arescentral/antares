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
#include "AresMain.hpp"
#include "CardStack.hpp"
#include "ColorTranslation.hpp"
#include "DirectText.hpp"
#include "Error.hpp"
#include "Ledger.hpp"
#include "DebriefingScreen.hpp"
#include "ScenarioMaker.hpp"
#include "SelectLevelScreen.hpp"
#include "Music.hpp"
#include "Options.hpp"
#include "Preferences.hpp"
#include "ScrollTextScreen.hpp"
#include "Transitions.hpp"

namespace antares {

extern PixMap* gRealWorld;
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
        stack()->push(new SelectLevelScreen(&_cancelled, &_scenario));
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
            stack()->push(new ScrollTextScreen(
                        GetScenarioPrologueID(_scenario), 450, 15.0, 4002));
            break;
        }
        // else fall through

      case PROLOGUE:
      case RESTART_LEVEL:
        _state = PLAYING;
        _game_result = NO_GAME;
        _game_length = 0;
        stack()->push(new MainPlay(_scenario, false, &_game_result, &_game_length));
        break;

      case PLAYING:
        handle_game_result();
        break;

      case DEBRIEFING:
        debriefing_done();
        break;

      case PLAY_AGAIN:
        switch (_play_again) {
          case PlayAgainScreen::RESTART:
            _state = RESTART_LEVEL;
            become_front();
            break;

          case PlayAgainScreen::QUIT:
            _state = QUIT;
            become_front();
            break;

          default:
            fail("_play_again was invalid after PLAY_AGAIN (%d)", _play_again);
        }
        break;

      case EPILOGUE:
        epilogue_done();
        break;

      case QUIT:
        stack()->pop(this);
        break;
    }
}

void SoloGame::handle_game_result() {
    switch (_game_result) {
      case LOSE_GAME:
        _state = DEBRIEFING;
        if (globals()->gScenarioWinner.text != -1) {
            stack()->push(new DebriefingScreen(globals()->gScenarioWinner.text));
        } else {
            become_front();
        }
        break;

      case WIN_GAME:
        _state = DEBRIEFING;
        if (globals()->gScenarioWinner.text != -1) {
            stack()->push(new DebriefingScreen(
                    globals()->gScenarioWinner.text,
                    _game_length, gThisScenario->parTime,
                    GetAdmiralLoss(0), gThisScenario->parLosses,
                    GetAdmiralKill(0), gThisScenario->parKills));
        } else {
            become_front();
        }
        break;

      case RESTART_GAME:
        _state = RESTART_LEVEL;
        become_front();
        break;

      case QUIT_GAME:
        _state = QUIT;
        become_front();
        break;

      default:
        fail("_game_result was invalid after PLAYING (%d)", _game_result);
    }
}

void SoloGame::debriefing_done() {
    switch (_game_result) {
      case LOSE_GAME:
        _state = PLAY_AGAIN;
        stack()->push(new PlayAgainScreen(false, false, &_play_again));
        break;

      case WIN_GAME:
        gRealWorld->fill(RgbColor::kBlack);

        _state = EPILOGUE;
        if (GetScenarioEpilogueID(_scenario) > 0) {
            // normal scrolltext song
            int scroll_song = 4002;
            if (globals()->gScenarioWinner.next == -1) {
                // we win but no next level? Play triumph song
                scroll_song = 4003;
            }

            stack()->push(new ScrollTextScreen(
                        GetScenarioEpilogueID(_scenario), 450, 15.0, scroll_song));
        } else {
            become_front();
        }
        break;

      default:
        fail("_game_result was invalid after DEBRIEFING (%d)", _game_result);
    }
}

void SoloGame::epilogue_done() {
    if (Preferences::preferences()->play_idle_music()) {
        gRealWorld->fill(RgbColor::kBlack);
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
            Ledger::ledger()->unlock_chapter(chapter);
        } else {
            _scenario = 0;
        }
    }

    if (_scenario >= 0) {
        _state = START_LEVEL;
    } else {
        _state = QUIT;
    }
    become_front();
}

}  // namespace antares

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

#include "ui/flows/solo-game.hpp"

#include <pn/file>

#include "config/ledger.hpp"
#include "config/preferences.hpp"
#include "data/plugin.hpp"
#include "data/resource.hpp"
#include "drawing/color.hpp"
#include "drawing/text.hpp"
#include "game/admiral.hpp"
#include "game/globals.hpp"
#include "game/input-source.hpp"
#include "game/level.hpp"
#include "game/main.hpp"
#include "game/sys.hpp"
#include "sound/music.hpp"
#include "ui/card.hpp"
#include "ui/screens/debriefing.hpp"
#include "ui/screens/scroll-text.hpp"
#include "ui/screens/select-level.hpp"
#include "video/transitions.hpp"

namespace antares {

SoloGame::SoloGame() : _state(NEW), _game_result(NO_GAME) {}

SoloGame::~SoloGame() {}

void SoloGame::become_front() {
    switch (_state) {
        case NEW:
            _state = SELECT_LEVEL;
            stack()->push(new SelectLevelScreen(&_cancelled, &_level));
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
            if (!_level->prologue.empty()) {
                stack()->push(
                        new ScrollTextScreen(_level->prologue, 450, kSlowScrollInterval, 4002));
                break;
            }
        // else fall through

        case PROLOGUE:
        case RESTART_LEVEL:
            _state       = PLAYING;
            _game_result = NO_GAME;
            stack()->push(new MainPlay(*_level, false, &_input_source, true, &_game_result));
            break;

        case PLAYING: handle_game_result(); break;

        case EPILOGUE: epilogue_done(); break;

        case QUIT: stack()->pop(this); break;
    }
}

void SoloGame::handle_game_result() {
    switch (_game_result) {
        case WIN_GAME: {
            _state                         = EPILOGUE;
            const pn::string_view epilogue = _level->epilogue;
            if (!epilogue.empty()) {
                // normal scrolltext song
                int scroll_song = 4002;
                if (!g.next_level) {
                    // we win but no next level? Play triumph song
                    scroll_song = 4003;
                }
                stack()->push(
                        new ScrollTextScreen(epilogue, 450, kSlowScrollInterval, scroll_song));
            } else {
                become_front();
            }
        } break;

        case RESTART_GAME:
            _state = RESTART_LEVEL;
            become_front();
            break;

        case QUIT_GAME:
            _state = QUIT;
            become_front();
            break;

        default:
            throw std::runtime_error(pn::format(
                                             "_play_again was invalid after PLAY_AGAIN ({0})",
                                             stringify(_play_again))
                                             .c_str());
    }
}

void SoloGame::epilogue_done() {
    _level = nullptr;
    _state = QUIT;

    if (g.next_level) {
        const int32_t chapter = g.next_level->chapter_number();
        if (chapter >= 0) {
            Ledger::ledger()->unlock_chapter(chapter);
            _level = g.next_level;
            _state = START_LEVEL;
        }
    }

    become_front();
}

}  // namespace antares

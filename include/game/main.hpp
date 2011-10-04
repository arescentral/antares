// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// Copyright (C) 2008-2011 Ares Central
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
// License along with this program.  If not, see
// <http://www.gnu.org/licenses/>.

#ifndef ANTARES_GAME_MAIN_HPP_
#define ANTARES_GAME_MAIN_HPP_

#include "ui/card.hpp"
#include "ui/interface-handling.hpp"

namespace antares {

enum GameResult {
    NO_GAME = -1,
    LOSE_GAME = 0,
    WIN_GAME = 1,
    RESTART_GAME = 2,
    QUIT_GAME = 3,
};

Card* AresInit();
void Pause();

class MainPlay : public Card {
  public:
    MainPlay(const Scenario* scenario, bool replay, GameResult* game_result);

    virtual void become_front();

  private:
    enum State {
        NEW,
        LOADING,
        BRIEFING,
        PLAYING,
    };
    State _state;

    const Scenario* _scenario;
    const bool _replay;
    bool _cancelled;
    GameResult* _game_result;
};

}  // namespace antares

#endif // ANTARES_GAME_MAIN_HPP_

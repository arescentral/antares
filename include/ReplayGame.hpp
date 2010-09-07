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

#ifndef ANTARES_REPLAY_GAME_HPP_
#define ANTARES_REPLAY_GAME_HPP_

#include <sfz/Macros.hpp>
#include "AresMain.hpp"
#include "Card.hpp"

namespace antares {

class SelectLevelScreen;

class ReplayGame : public Card {
  public:
    ReplayGame(int scenario);
    ~ReplayGame();

    virtual void become_front();

  private:
    enum State {
        NEW,
        PLAYING,
        QUIT,
    };
    State _state;

    const int _scenario;
    GameResult _game_result;
    long _game_length;
    int _saved_seed;
};

}  // namespace antares

#endif  // ANTARES_REPLAY_GAME_HPP_

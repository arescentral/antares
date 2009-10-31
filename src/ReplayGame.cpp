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

#include "ReplayGame.hpp"

#include "AresGlobalType.hpp"
#include "CardStack.hpp"
#include "InputSource.hpp"
#include "Options.hpp"

namespace antares {

namespace {

const int kReplayResId = 600;

}  // namespace

extern long gRandomSeed;

ReplayGame::ReplayGame(int scenario)
        : _state(NEW),
          _scenario(scenario),
          _game_result(NO_GAME),
          _game_length(0) { }

ReplayGame::~ReplayGame() { }

void ReplayGame::become_front() {
    switch (_state) {
      case NEW:
        _state = PLAYING;
        globals()->gOptions |= kOptionReplay;
        globals()->gInputSource.reset(new ReplayInputSource(kReplayResId + _scenario));
        _saved_seed = gRandomSeed;
        gRandomSeed = globals()->gInputSource->random_seed();
        _game_result = NO_GAME;
        _game_length = 0;
        stack()->push(new MainPlay(_scenario, &_game_result, &_game_length));
        break;

      case PLAYING:
        _state = QUIT;
        become_front();
        break;

      case QUIT:
        gRandomSeed = _saved_seed;
        globals()->gInputSource.reset();
        globals()->gOptions &= ~kOptionReplay;
        stack()->pop(this);
        break;
    }
}

}  // namespace antares

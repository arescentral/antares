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

#include "ui/flows/replay-game.hpp"

#include <algorithm>

#include "data/plugin.hpp"
#include "game/globals.hpp"
#include "game/level.hpp"
#include "math/random.hpp"
#include "ui/card.hpp"
#include "video/transitions.hpp"

namespace antares {

using sfz::read;
using std::swap;

ReplayGame::ReplayGame(int16_t replay_id)
        : _state(NEW),
          _resource("replays", "NLRP", replay_id),
          _data(_resource.data()),
          _random_seed{_data.global_seed},
          _level(_data.chapter_id - 1),
          _game_result(NO_GAME),
          _input_source(&_data) {}

ReplayGame::~ReplayGame() {}

void ReplayGame::become_front() {
    switch (_state) {
        case NEW:
            _state = FADING_OUT;
            stack()->push(
                    new ColorFade(ColorFade::TO_COLOR, RgbColor::black(), secs(1), false, NULL));
            break;

        case FADING_OUT: {
            _state = PLAYING;
            swap(_random_seed, g.random);
            _game_result = NO_GAME;
            stack()->push(new MainPlay(_level, true, &_input_source, true, &_game_result));
        } break;

        case PLAYING:
            swap(_random_seed, g.random);
            stack()->pop(this);
            break;
    }
}

}  // namespace antares

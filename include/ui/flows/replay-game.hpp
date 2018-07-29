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

#ifndef ANTARES_UI_FLOWS_REPLAY_GAME_HPP_
#define ANTARES_UI_FLOWS_REPLAY_GAME_HPP_

#include "data/replay.hpp"
#include "data/resource.hpp"
#include "game/input-source.hpp"
#include "game/main.hpp"
#include "math/random.hpp"
#include "ui/card.hpp"

namespace antares {

union Level;

class ReplayGame : public Card {
  public:
    ReplayGame(pn::string_view replay_name);
    ~ReplayGame();

    virtual void become_front();

  private:
    enum State {
        NEW,
        FADING_OUT,
        PLAYING,
    };
    State _state;

    ReplayData        _data;
    Random            _random_seed;
    const Level&      _level;
    GameResult        _game_result;
    ReplayInputSource _input_source;
};

}  // namespace antares

#endif  // ANTARES_UI_FLOWS_REPLAY_GAME_HPP_

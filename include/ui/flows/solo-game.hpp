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

#ifndef ANTARES_UI_FLOWS_SOLO_GAME_HPP_
#define ANTARES_UI_FLOWS_SOLO_GAME_HPP_

#include <sfz/sfz.hpp>

#include "game/input-source.hpp"
#include "game/main.hpp"
#include "ui/card.hpp"
#include "ui/screens/play-again.hpp"

namespace antares {

class SelectLevelScreen;

class SoloGame : public Card {
  public:
    SoloGame();
    ~SoloGame();

    virtual void become_front();

  private:
    enum State {
        NEW,
        SELECT_LEVEL,
        PROLOGUE,
        START_LEVEL,
        RESTART_LEVEL,
        PLAYING,
        EPILOGUE,
        QUIT,
    };
    State _state;

    void handle_game_result();
    void debriefing_done();
    void epilogue_done();

    bool                  _cancelled;
    Handle<Level>         _level;
    GameResult            _game_result;
    PlayAgainScreen::Item _play_again;
    RealInputSource       _input_source;
};

}  // namespace antares

#endif  // ANTARES_UI_FLOWS_SOLO_GAME_HPP_

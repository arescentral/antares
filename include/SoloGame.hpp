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

#ifndef ANTARES_SOLO_GAME_HPP_
#define ANTARES_SOLO_GAME_HPP_

#include "AresMain.hpp"
#include "Card.hpp"
#include "PlayAgainScreen.hpp"
#include "SmartPtr.hpp"

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
        DEBRIEFING,
        PLAY_AGAIN,
        EPILOGUE,
        QUIT,
    };
    State _state;

    void handle_game_result();
    void debriefing_done();
    void epilogue_done();

    bool _cancelled;
    int _scenario;
    GameResult _game_result;
    long _game_length;
    PlayAgainScreen::Item _play_again;
};

}  // namespace antares

#endif  // ANTARES_SOLO_GAME_HPP_

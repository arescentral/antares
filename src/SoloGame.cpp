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

#include "AresMain.hpp"
#include "SelectLevelScreen.hpp"

namespace antares {

SoloGame::SoloGame()
        : _state(NEW) { }

SoloGame::~SoloGame() { }

void SoloGame::become_front() {
    switch (_state) {
      case NEW:
        _state = SELECT_LEVEL;
        _select_level.reset(new SelectLevelScreen);
        VideoDriver::driver()->push_listener(_select_level.get());
        break;

      case SELECT_LEVEL:
        _state = PLAYING;
        start_main_play(_select_level->level());
        _select_level.reset();
        break;

      case PLAYING:
        VideoDriver::driver()->pop_listener(this);
        break;
    }
}

void SoloGame::start_main_play(int level) {
    MainPlay(level);
    become_front();
}

}  // namespace antares

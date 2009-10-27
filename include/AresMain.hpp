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

#ifndef ANTARES_ARES_MAIN_HPP_
#define ANTARES_ARES_MAIN_HPP_

#include <Base.h>
#include "PlayerInterface.hpp"

namespace antares {

#define kHackLevelMax   26//4//21

enum GameResult {
    NO_GAME = -1,
    LOSE_GAME = 0,
    WIN_GAME = 1,
    RESTART_GAME = 2,
    QUIT_GAME = 3,
};

void AresMain();
void Pause();
void MainPlay(int whichScenario, GameResult* gameResult, long* gameLength);
void StartReplay();

}  // namespace antares

#endif // ANTARES_ARES_MAIN_HPP_

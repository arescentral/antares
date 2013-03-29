// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// Copyright (C) 2008-2012 The Antares Authors
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

#ifndef ANTARES_GAME_PLAYER_SHIP_HPP_
#define ANTARES_GAME_PLAYER_SHIP_HPP_

#include "data/space-object.hpp"

namespace antares {

class Cursor;
class InputSource;

void ResetPlayerShip( long);
bool PlayerShipGetKeys(
        int32_t timePass, InputSource& input_source, const Cursor& cursor, bool *enterMessage);
void PlayerShipHandleClick(Point where, int button);
void SetPlayerSelectShip( long, bool, long);
void ChangePlayerShipNumber( long, long);
void TogglePlayerAutoPilot( spaceObjectType *);
bool IsPlayerShipOnAutoPilot( void);
void PlayerShipGiveCommand( long);
void PlayerShipBodyExpire( spaceObjectType *, bool);
void HandleTextMessageKeys(const KeyMap&, const KeyMap&, bool *);

}  // namespace antares

#endif // ANTARES_GAME_PLAYER_SHIP_HPP_

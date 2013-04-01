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

#ifndef ANTARES_GAME_NON_PLAYER_SHIP_HPP_
#define ANTARES_GAME_NON_PLAYER_SHIP_HPP_

// NonPlayerShip.h

#include "data/space-object.hpp"

namespace antares {

spaceObjectType *HackNewNonplayerShip(int32_t, int16_t, Rect *);
void NonplayerShipThink(int32_t);
void UpdateMyNonplayerShip( void);
void HackShowShipID( void);
void HitObject( spaceObjectType *, spaceObjectType *);
int32_t GetManualSelectObject( spaceObjectType *, uint32_t,
                            uint32_t, uint32_t,
                            const uint64_t*, int32_t, int16_t);
int32_t GetSpritePointSelectObject( Rect *, spaceObjectType *, uint32_t,
                            uint32_t, uint32_t, int32_t, int16_t);

}  // namespace antares

#endif // ANTARES_GAME_NON_PLAYER_SHIP_HPP_

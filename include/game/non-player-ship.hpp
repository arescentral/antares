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

#ifndef ANTARES_GAME_NON_PLAYER_SHIP_HPP_
#define ANTARES_GAME_NON_PLAYER_SHIP_HPP_

// NonPlayerShip.h

#include "data/base-object.hpp"
#include "game/space-object.hpp"

namespace antares {

enum Allegiance {
    FRIENDLY_OR_HOSTILE = 0,
    FRIENDLY,
    HOSTILE,
};

void fire_weapon(
        Handle<SpaceObject> subject, Handle<SpaceObject> target, SpaceObject::Weapon& weapon,
        const std::vector<fixedPointType>& positions);
void                NonplayerShipThink();
void                HitObject(Handle<SpaceObject> anObject, Handle<SpaceObject> sObject);
Handle<SpaceObject> GetManualSelectObject(
        Handle<SpaceObject> sourceObject, int32_t direction, uint32_t inclusiveAttributes,
        uint32_t exclusiveAttributes, const uint64_t* fartherThan, Handle<SpaceObject> currentShip,
        Allegiance allegiance);
Handle<SpaceObject> GetSpritePointSelectObject(
        Rect* bounds, Handle<SpaceObject> sourceObject, uint32_t anyOneAttribute,
        Handle<SpaceObject> currentShip, Allegiance allegiance);

}  // namespace antares

#endif  // ANTARES_GAME_NON_PLAYER_SHIP_HPP_

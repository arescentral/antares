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

#ifndef ANTARES_GAME_MOTION_HPP_
#define ANTARES_GAME_MOTION_HPP_

#include "data/base-object.hpp"
#include "math/units.hpp"

namespace antares {

const int32_t kUnitsToCheckNumber = 5;

struct adjacentUnitType {
    uint8_t adjacentUnit;  // the normal adjacent unit
    Point   superOffset;   // the offset of the super unit (for wrap-around)
};

struct proximityUnitType {
    Handle<SpaceObject> nearObject;                         // for collision checking
    Handle<SpaceObject> farObject;                          // for distance checking
    adjacentUnitType    unitsToCheck[kUnitsToCheckNumber];  // adjacent units to check
};

extern coordPointType gGlobalCorner;

void InitMotion();
void ResetMotionGlobals();

Size center_scale();

void MotionCleanup();
void MoveSpaceObjects(ticks unitsToDo);
void CollideSpaceObjects();

}  // namespace antares

#endif  // ANTARES_GAME_MOTION_HPP_

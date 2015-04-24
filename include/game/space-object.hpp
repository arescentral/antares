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

#ifndef ANTARES_GAME_SPACE_OBJECT_HPP_
#define ANTARES_GAME_SPACE_OBJECT_HPP_

#include "data/space-object.hpp"

namespace antares {

const int16_t kBaseObjectResID      = 500;
const int16_t kObjectActionResID    = 500;

extern SpaceObject* gRootObject;
extern int32_t gRootObjectNumber;

void SpaceObjectHandlingInit( void);
void ResetAllSpaceObjects( void);
void RemoveAllSpaceObjects( void);
void CorrectAllBaseObjectColor( void);
void ChangeObjectBaseType( SpaceObject *, int32_t, int32_t, bool);

SpaceObject* CreateAnySpaceObject(
        int32_t whichBase, fixedPointType *velocity, coordPointType *location, int32_t direction,
        Handle<Admiral> owner, uint32_t specialAttributes, int16_t spriteIDOverride);
int32_t CountObjectsOfBaseType(int32_t whichType, Handle<Admiral> owner);
void AlterObjectOwner(SpaceObject* object, Handle<Admiral> owner, bool message);
void AlterObjectOccupation(
        SpaceObject* object, Handle<Admiral> owner, int32_t howMuch, bool message);
void AlterObjectCloakState( SpaceObject *, bool);
void DestroyObject( SpaceObject *);
void CreateFloatingBodyOfPlayer( SpaceObject *);

BaseObject* mGetBaseObjectPtr(int32_t whichObject);
SpaceObject* mGetSpaceObjectPtr(int32_t whichObject);
objectActionType* mGetObjectActionPtr(int32_t whichAction);

int32_t mGetBaseObjectFromClassRace(int class_, int race);

sfz::StringSlice get_object_name(int16_t id);
sfz::StringSlice get_object_short_name(int16_t id);

}  // namespace antares

#endif // ANTARES_GAME_SPACE_OBJECT_HPP_

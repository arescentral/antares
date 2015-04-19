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

extern spaceObjectType* gRootObject;
extern int32_t gRootObjectNumber;

void SpaceObjectHandlingInit( void);
void ResetAllSpaceObjects( void);
void RemoveAllSpaceObjects( void);
void CorrectAllBaseObjectColor( void);
void ChangeObjectBaseType( spaceObjectType *, int32_t, int32_t, bool);

int32_t CreateAnySpaceObject(int32_t, fixedPointType *, coordPointType *, int32_t, int32_t, uint32_t,
                            int16_t);
int32_t CountObjectsOfBaseType(int32_t, int32_t);
void AlterObjectOwner( spaceObjectType *, int32_t, bool);
void AlterObjectOccupation( spaceObjectType *, int32_t, int32_t, bool);
void AlterObjectCloakState( spaceObjectType *, bool);
void DestroyObject( spaceObjectType *);
void CreateFloatingBodyOfPlayer( spaceObjectType *);

baseObjectType* mGetBaseObjectPtr(int32_t whichObject);
spaceObjectType* mGetSpaceObjectPtr(int32_t whichObject);
objectActionType* mGetObjectActionPtr(int32_t whichAction);

void mGetBaseObjectFromClassRace(
        baseObjectType*& mbaseObject, int32_t& mcount, int mbaseClass, int mbaseRace);

sfz::StringSlice get_object_name(int16_t id);
sfz::StringSlice get_object_short_name(int16_t id);

}  // namespace antares

#endif // ANTARES_GAME_SPACE_OBJECT_HPP_

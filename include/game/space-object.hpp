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

extern Handle<SpaceObject> gRootObject;

void SpaceObjectHandlingInit( void);
void ResetAllSpaceObjects( void);
void RemoveAllSpaceObjects( void);
void CorrectAllBaseObjectColor( void);
void ChangeObjectBaseType(
        Handle<SpaceObject> obj, Handle<BaseObject> base, int32_t spriteIDOverride, bool relative);

Handle<SpaceObject> CreateAnySpaceObject(
        Handle<BaseObject> whichBase, fixedPointType *velocity, coordPointType *location,
        int32_t direction, Handle<Admiral> owner, uint32_t specialAttributes,
        int16_t spriteIDOverride);
int32_t CountObjectsOfBaseType(Handle<BaseObject> whichType, Handle<Admiral> owner);
void AlterObjectOwner(Handle<SpaceObject> object, Handle<Admiral> owner, bool message);
void AlterObjectOccupation(
        Handle<SpaceObject> object, Handle<Admiral> owner, int32_t howMuch, bool message);
void AlterObjectCloakState(Handle<SpaceObject> object, bool cloak);
void DestroyObject(Handle<SpaceObject> object);
void CreateFloatingBodyOfPlayer(Handle<SpaceObject> obj);

inline Action* mGetObjectActionPtr(int32_t whichAction) { return Action::get(whichAction); }

Handle<BaseObject> mGetBaseObjectFromClassRace(int class_, int race);

sfz::StringSlice get_object_name(Handle<BaseObject> id);
sfz::StringSlice get_object_short_name(Handle<BaseObject> id);

}  // namespace antares

#endif // ANTARES_GAME_SPACE_OBJECT_HPP_

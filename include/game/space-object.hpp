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
extern long gRootObjectNumber;
extern sfz::scoped_array<spaceObjectType> gSpaceObjectData;
extern sfz::scoped_array<baseObjectType> gBaseObjectData;
extern sfz::scoped_array<objectActionType> gObjectActionData;

void SpaceObjectHandlingInit( void);
void CleanupSpaceObjectHandling( void);
void ResetAllSpaceObjects( void);
void ResetActionQueueData( void);
int AddSpaceObject( spaceObjectType *);
//int AddSpaceObject( spaceObjectType *, long *, short, short);
int AddNumberedSpaceObject( spaceObjectType *, long);
void RemoveAllSpaceObjects( void);
void CorrectAllBaseObjectColor( void);
void InitSpaceObjectFromBaseObject( spaceObjectType *, long, short, long, fixedPointType *, long,
                                    short);
void ChangeObjectBaseType( spaceObjectType *, long, long, bool);
void AddActionToQueue( objectActionType *, long, long, long, spaceObjectType *,
                        spaceObjectType *, Point*);
void ExecuteActionQueue( long);
void ExecuteObjectActions( long, long, spaceObjectType *, spaceObjectType *, Point*, bool);
long CreateAnySpaceObject( long, fixedPointType *, coordPointType *, long, long, unsigned long,
                            short);
long CountObjectsOfBaseType( long, long);
long GetNextObjectWithAttributes( long, unsigned long, bool);
void AlterObjectHealth( spaceObjectType *, long);
void AlterObjectEnergy( spaceObjectType *, long);
void AlterObjectBattery( spaceObjectType *, long);
void AlterObjectOwner( spaceObjectType *, long, bool);
void AlterObjectOccupation( spaceObjectType *, long, long, bool);
void AlterObjectCloakState( spaceObjectType *, bool);
void DestroyObject( spaceObjectType *);
void ActivateObjectSpecial( spaceObjectType *);
void CreateFloatingBodyOfPlayer( spaceObjectType *);

inline baseObjectType* mGetBaseObjectPtr(long whichObject) {
    return gBaseObjectData.get() + whichObject;
}

inline void mGetBaseObjectFromClassRace(
        baseObjectType*& mbaseObject, long& mcount, int mbaseClass, int mbaseRace) {
    mcount = 0;
    if ( mbaseClass >= kLiteralClass)
    {
        mcount = mbaseClass - kLiteralClass;
        mbaseObject = mGetBaseObjectPtr(mcount);
    }
    else
    {
        mbaseObject = mGetBaseObjectPtr( 0);
        while (( mcount < globals()->maxBaseObject) && (( mbaseObject->baseClass != mbaseClass) || ( mbaseObject->baseRace != mbaseRace)))
        {
            mcount++;
            mbaseObject++;
        }
        if ( mcount >= globals()->maxBaseObject) mbaseObject = NULL;
    }
}

}  // namespace antares

#endif // ANTARES_GAME_SPACE_OBJECT_HPP_

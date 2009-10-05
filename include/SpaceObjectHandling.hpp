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

#ifndef ANTARES_SPACE_OBJECT_HANDLING_HPP_
#define ANTARES_SPACE_OBJECT_HANDLING_HPP_

// Space Object Handling.h

#include "SpaceObject.hpp"

#define kBaseObjectResType      'bsob'
#define kBaseObjectResID        500

#define kObjectActionResType    'obac'
#define kObjectActionResID      500

int SpaceObjectHandlingInit( void);
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
                        spaceObjectType *, longPointType *);
void ExecuteActionQueue( long);
void ExecuteObjectActions( long, long, spaceObjectType *, spaceObjectType *, longPointType *, bool);
void DebugExecuteObjectActions( long, long, spaceObjectType *, spaceObjectType *, longPointType *, char *, long);
//long CreateAnySpaceObject( long, fixedPointType *, coordPointType *, long, long, unsigned long,
//                          long *, short, short, short);
long CreateAnySpaceObject( long, fixedPointType *, coordPointType *, long, long, unsigned long,
                            short);
long DebugCreateAnySpaceObject( long, fixedPointType *, coordPointType *, long, long, unsigned long,
                            long *, short, short, short, char *, long);
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

#endif // ANTARES_SPACE_OBJECT_HANDLING_HPP_

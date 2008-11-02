/*
Ares, a tactical space combat game.
Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

// Space Object Handling.h

#ifndef kSpaceObject
#include "Space Object.h"
#endif

#pragma options align=mac68k

#define	kBaseObjectResType		'bsob'
#define	kBaseObjectResID		500

#define	kObjectActionResType	'obac'
#define	kObjectActionResID		500


#define	mFindSpaceObjectByID( object, counter, idnum)\
				(object) = ( spaceObjectType *)*gSpaceObjectData;\
				(counter) = 0;\
				while (( (counter) < kMaxSpaceObject) && ( (object)->id != (idnum)))\
				{\
					(object)++;\
					(counter)++;\
				}\
				if ( (counter) == kMaxSpaceObject) (object) = nil;
				

//#define	CreateAnySpaceObject( mwhichBase, mvel, mloc, mdir, mowner, mspecial, mcanbuild, mnameres, mnamestr, msprite)\
//DebugCreateAnySpaceObject( mwhichBase, mvel, mloc, mdir, mowner, mspecial, mcanbuild, mnameres, mnamestr, msprite, __FILE__, __LINE__)

//#define ExecuteObjectActions( mwhich, mnum, msobj, mdobj, moffset)\
//DebugExecuteObjectActions( mwhich, mnum, msobj, mdobj, moffset, __FILE__, __LINE__)
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
void ChangeObjectBaseType( spaceObjectType *, long, long, Boolean);
void AddActionToQueue( objectActionType *, long, long, long, spaceObjectType *,
						spaceObjectType *, longPointType *);
void ExecuteActionQueue( long);
void ExecuteObjectActions( long, long, spaceObjectType *, spaceObjectType *, longPointType *, Boolean);
void DebugExecuteObjectActions( long, long, spaceObjectType *, spaceObjectType *, longPointType *, char *, long);
//long CreateAnySpaceObject( long, fixedPointType *, coordPointType *, long, long, unsigned long,
//							long *, short, short, short);
long CreateAnySpaceObject( long, fixedPointType *, coordPointType *, long, long, unsigned long,
							short);
long DebugCreateAnySpaceObject( long, fixedPointType *, coordPointType *, long, long, unsigned long,
							long *, short, short, short, char *, long);
long CountObjectsOfBaseType( long, long);
long GetNextObjectWithAttributes( long, unsigned long, Boolean);
void ResolveSpaceObjectData( Handle);
void ResolveObjectActionData( Handle);
void ResolveActionQueueData( Handle);
void AlterObjectHealth( spaceObjectType *, long);
void AlterObjectEnergy( spaceObjectType *, long);
void AlterObjectBattery( spaceObjectType *, long);
void AlterObjectOwner( spaceObjectType *, long, Boolean);
void AlterObjectOccupation( spaceObjectType *, long, long, Boolean);
void AlterObjectCloakState( spaceObjectType *, Boolean);
void DestroyObject( spaceObjectType *);
void ActivateObjectSpecial( spaceObjectType *);
void CreateFloatingBodyOfPlayer( spaceObjectType *);

#pragma options align=reset

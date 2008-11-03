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

#ifndef ANTARES_MOTION_HPP_
#define ANTARES_MOTION_HPP_

// Motion.h

#include "MathSpecial.hpp"
#include "SpaceObject.hpp"

#pragma options align=mac68k

#define kMotionResolution           100     // must be same as kRotUnit ( see Rotation.h)

#define kProximitySuperSize         16      // number of cUnits in cSuperUnits
#define kProximityGridDataLength    256     // kCollisionSuperSize * kCollisionSuperSize
#define kProximityUnitAndModulo     0x0000000f  // & a long with this and get modulo kCollisionSuperSize
#define kProximityWidthMultiply     4L      // for speed = * kCollisionSuperSize

#define kCollisionUnit              128     // 128 pixels in smallest collision grid unit
#define kCollisionSuperUnit         2048    // 2048 pixels in super unit (super unit = 16x16 normal)

#define kCollisionUnitBitShift      7L      // >> 7L = / 128
#define kCollisionSuperUnitBitShift 11L     // >> 11L = / 2048
#define kCollisionSuperExtraShift   4L      // for speed's sake, kCollisionSuperUnitBitShift - kCollisionUnitBitShift

/*
#define kDistanceUnit               16384   // = smallest distance grid unit
#define kDistanceSuperUnit          262144  // = kProximitySuperSize * kDistanceUnit
#define kDistanceUnitBitShift       14L     // >> 14L = / 16384
#define kDistanceSuperUnitBitShift  18L     // >> 18L = / 262144
#define kDistanceSuperExtraShift    4L      // for sake of speed = kDistanceSperUnitBitShift - kDistanceUnitBitShift
#define kDistanceUnitExtraShift     3L      // speed from kCollisionSuperUnitBitShift to kDistanceUnitBitShift
*/

#define kDistanceUnit               2048    // = smallest distance grid unit
#define kDistanceSuperUnit          32768   // = kProximitySuperSize * kDistanceUnit
#define kDistanceUnitBitShift       11L     // >> 14L = / 2048
#define kDistanceSuperUnitBitShift  15L     // >> 18L = / 262144
#define kDistanceSuperExtraShift    4L      // for sake of speed = kDistanceSperUnitBitShift - kDistanceUnitBitShift
#define kDistanceUnitExtraShift     0L      // speed from kCollisionSuperUnitBitShift to kDistanceUnitBitShift

// for the macro mRanged, time is assumed to be a long game ticks, velocity a fixed, result long, scratch fixed
#define mRange( result, time, velocity, scratch) \
    scratch = mLongToFixed( time); \
    scratch = mMultiplyFixed (scratch, velocity); \
    result = mFixedToLong( scratch);

int InitMotion( void);
void ResetMotionGlobals( void);
void HackCheckProxGrid( long);

void MotionCleanup( void);
void MoveSpaceObjects( spaceObjectType *, const long, const long);
void CollideSpaceObjects( spaceObjectType *, const long);
void CorrectPhysicalSpace( spaceObjectType *, spaceObjectType *);

#pragma options align=reset

#endif // ANTARES_MOTION_HPP_

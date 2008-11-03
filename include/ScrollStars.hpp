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

#ifndef ANTARES_SCROLL_STARS_HPP_
#define ANTARES_SCROLL_STARS_HPP_

// Scroll Stars.h

#include "SpaceObject.h"

#pragma options align=mac68k

#define kMaxSparkAge            1023L
#define kSparkAgeToShadeShift   6L


typedef struct
{
    longPointType   oldOldLocation;
    longPointType   oldLocation;
    longPointType   location;
    fixedPointType  motionFraction;
    fixedPointType  velocity;
    long            age;
    long            speed;
    unsigned char   color;
} scrollStarType;


int InitScrollStars( void);
void CleanupScrollStars( void);
void ResetScrollStars ( long);
void MakeNewSparks( long, long, smallFixedType, unsigned char, longPointType *);
void PrepareToMoveScrollStars( void);
void MoveScrollStars( const long );
void DrawScrollStars( Boolean);
void ShowScrollStars( Boolean);
void DontShowScrollStars( void);
void DrawAllBeams( void);
void Reset3DStars( Point, longRect *);
void Move3DStars( Point, long, longRect *);
void Draw3DStars( Boolean, longRect *, PixMapHandle);
void Show3DStars( Boolean, longRect *, PixMapHandle);

#pragma options align=reset

#endif // ANTARES_SCROLL_STARS_HPP_

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

#ifndef ANTARES_ROTATION_HPP_
#define ANTARES_ROTATION_HPP_

/* ROTATION.H */

#pragma options align=mac68k

/*
#define ROT_RES     5
#define ROT_POS     72
#define ROT_90      18
#define ROT_270     54
#define ROT_315     63
#define ROT_180     36
#define ROT_45      9
#define ROT_30      6
#define ROT_15      3
#define ROT_0       0
*/
/*
#define ROT_RES     3
#define ROT_POS     120
#define ROT_90      30
#define ROT_270     90
#define ROT_315     105
#define ROT_180     60
#define ROT_45      15
#define ROT_30      10
#define ROT_15      5
#define ROT_0       0
*/

#define ROT_RES     1
#define ROT_POS     360
#define ROT_90      90
#define ROT_270     270
#define ROT_315     315
#define ROT_180     180
#define ROT_45      45
#define ROT_30      30
#define ROT_15      15
#define ROT_0       0


#define kRotUnit    100     // must be same as kMotionResolution ( see Motion.h)

#define mGetRotPoint( x, y, angle)  (x) = *((Fixed *)*gRotTable + (long)(angle) * (long)2);\
                                    (y) = *((Fixed *)*gRotTable + (long)((angle) * (long)2) + (long)1);

// mAngleDifference: get the smallest difference from theta to other

#define mAngleDifference( theta, other) \
    ( (theta) >= (other)) ? \
    ((( (theta) - (other)) > ROT_180) ? ((other) - (theta) + ROT_POS) : ( (other) - (theta))) : \
    ((( (other) - (theta)) > ROT_180) ? ((other) - (theta) - ROT_POS) : ( (other) - (theta)))

#define mAddAngle( theta, other) theta += (other); if ( theta >= ROT_POS) theta -= ROT_POS; else if ( theta < 0) theta += ROT_POS;

int RotationInit( void);
void RotationCleanup( void);
void SetRotPoint( long, long, long);
void GetRotPoint( long *x, long *y, long rotpos);
void RotatePoint( long, long, Fixed *, Fixed *, long);
long GetAngleFromVector( long, long);

#pragma options align=reset

#endif // ANTARES_ROTATION_HPP_

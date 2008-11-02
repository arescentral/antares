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

// Rotation.c

#ifndef __CONDITIONALMACROS__
#include "ConditionalMacros.h"
#endif // __CONDITIONALMACROS__

#if TARGET_OS_WIN32

	#ifndef __QUICKTIMEVR__
	#include <QuickTimeVR.h>
	#endif

	#ifndef __QTUtilities__
	#include "QTUtilities.h"
	#endif

	#ifndef __QTVRUtilities__
	#include "QTVRUtilities.h"
	#endif

	#include <TextUtils.h>
	#include <Script.h>
	#include <string.h>
#endif // TARGET_OS_WIN32

#include "Resources.h"

#include "math.h"
#include "Error.h"
#include "Debug.h"
#include "Handle Handling.h"

#ifndef kMathSpecial
#include "Math Special.h"
#endif

#include "Math Macros.h"

#include "Rotation.h"

#define	kRotationError	"\pROTN"

Handle	gRotTable = nil;


int RotationInit( void)

{
	long		i = 0;
	
/*
	gRotTable = NewHandle( sizeof( smallFixedType) * 2L * (long)ROT_POS);
	if ( gRotTable == 0L)
	{
		ShowErrorRecover( MEMORY_ERROR, kRotationError, 1);
		return( MEMORY_ERROR);
	} else
	{
		MoveHHi( gRotTable);
		HLock( gRotTable);
		for ( d = 0; d < 360; d += ROT_RES)
		{
			sx = 0;
			sy = 1;	// kRotUnit
			RotatePoint( sx, sy, &x, &y, (long)(d + .5));
			SetRotPoint( x, y, i);
			i++;
		}
		AddResource( gRotTable, 'rot ', 500, "\pRotation Table");
	}


*/

	gRotTable = GetResource( 'rot ', 500);
	if ( gRotTable == nil)
	{
		ShowErrorAny( eQuitErr, kErrorStrID, nil, nil, nil, nil, kReadRotationDataError, -1, -1, -1, __FILE__, 1);
		return( MEMORY_ERROR);
	} else
	{
		DetachResource( gRotTable);
		/*
		MoveHHi( gRotTable);
		HLock( gRotTable);
		*/
		mDataHandleLockAndRegister( gRotTable, nil, nil, nil, "\pgRotTable")

	}

	return( kNoError);
}

void RotationCleanup( void)

{
//	WriteResource( gRotTable);
//	DisposeHandle( gRotTable);
	ReleaseResource( gRotTable);
}

void SetRotPoint( smallFixedType x, smallFixedType y, long rotpos)

{
	smallFixedType		*i;
	
	i = (smallFixedType *)*gRotTable + (long)rotpos * 2L;
	*i = x;
	i++;
	*i = y;
}

void GetRotPoint( smallFixedType *x, smallFixedType *y, long rotpos)

{
	smallFixedType		*i;
	
	i = (smallFixedType *)*gRotTable + (long)rotpos * 2L;
	*x = *i;
	i++;
	*y = *i;
}

		
void RotatePoint( long sx, long sy, smallFixedType *x, smallFixedType *y, long theta)

{
#pragma unused( sx, sy, x, y, theta)

/*
	double fx, fx2, fy, fcos, fsin, rot, d;
	long		rx, ry;
	
	fx = fx2 = sx;
	fy = sy;
	d = theta;
	rot = d / 57.295827909;
	fcos = cos( rot);
	fsin = sin( rot);
	if ( theta == 0)
	{
		fcos = 1;
		fsin = 0;
	} else if ( theta == 90)
	{
		fcos = 0;
		fsin = 1;
	} else if ( theta == 180)
	{
		fcos = -1;
		fsin = 0;
	} else if ( theta == 270)
	{
		fcos = 0;
		fsin = -1;
	}
	fx = fx * fcos - fy * fsin;
	fy = fx2 * fsin + fy * fcos;	
	
	*x = fx * kFixedWholeMultiplier;	//  * 65536
	*y = fy * kFixedWholeMultiplier;
*/
}

long GetAngleFromVector( long x, long y)

{
	long		*h, *v, a, b, test = 0, best = 0, whichBest = -1, whichAngle;
	
	a = x;
	b = y;
	
	if ( a < 0) a = -a;
	if ( b < 0) b = -b;
	if ( b < a)
	{
		h = (long *)*gRotTable + (long)ROT_45 * 2L;
		whichAngle = ROT_45;
		v = h + 1;
		do
		{
			test = (*v * a) + (*h * b);	// we're adding b/c in my table 45¡-90¡, h < 0
			if ( test < 0) test = -test;
			if (( whichBest < 0)  || ( test < best))
			{
				best = test;
				whichBest = whichAngle;
			}
			h += 2;
			v += 2;
			whichAngle++;
		} while ( ( test == best) && ( whichAngle <= ROT_90));
	} else
	{
		h = (long *)*gRotTable + (long)ROT_0 * 2L;
		whichAngle = ROT_0;
		v = h + 1;
		do
		{
			test = (*v * a) + (*h * b);
			if ( test < 0) test = -test;
			if (( whichBest < 0)  || ( test < best))
			{
				best = test;
				whichBest = whichAngle;
			}
			h += 2;
			v += 2;
			whichAngle++;
		} while (( test == best) &&  ( whichAngle <= ROT_45));
	}
	if ( x > 0)
	{
		if ( y < 0) whichBest = whichBest + ROT_180;
		else whichBest = ROT_POS - whichBest;
	} else if ( y < 0) whichBest = ROT_180 - whichBest;
	if ( whichBest == ROT_POS) whichBest = ROT_0;
	return ( whichBest);
	
}

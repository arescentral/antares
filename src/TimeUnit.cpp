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

//#include <math routines.h>
#include "GXMath.h"
#include "ToolUtils.h"

#include "AresGlobalType.h"
#include <Timer.h>

#include "TimeUnit.h"
#ifndef kMathSpecial
#include "MathSpecial.h"
#endif

extern aresGlobalType   *gAresGlobal;
//long          gAresGlobal->gGameTime = 0, gAresGlobal->gGameStartTime = 0;
//UnsignedWide  gAresGlobal->gLastTime;

void ResetLastTime( long defecit)

{
    UnsignedWide defecitTime;

    defecitTime.hi = 0;
    defecitTime.lo = defecit * kTimeUnit;

    Microseconds( &gAresGlobal->gLastTime);
//  WideSubtract( (wide *)&gAresGlobal->gLastTime, (wide *)&defecitTime);
    gAresGlobal->gGameStartTime = TickCount();// - defecit;
}

long GetTimeLapse( void)

{
    UnsignedWide            thisTime;

    Microseconds( &thisTime);
    WideSubtract( (wide *)&thisTime, (wide *)&gAresGlobal->gLastTime);
    return ( thisTime.lo / kTimeUnit);
}

void UpdateGameTime( void)

{
//  gAresGlobal->gGameTime += GetTimeLapse();
//  ResetLastTime();
}

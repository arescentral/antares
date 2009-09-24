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

// Randomize.c

#include "Randomize.hpp"

#include "AresGlobalType.hpp"
#include "AresPreferences.hpp"
#include "ConditionalMacros.h"
#include "Debug.hpp"
#include "Error.hpp"
#include "Resources.h"
#include "StringHandling.hpp"
#include "StringNumerics.hpp"

#define kRandomError    "\pRAND"

#define kRandomTableSize    512L

#define kRandomDebugTableSize   100000

//#define   kRandomRecord
//#define   kRandomReplay
#define     kSaveToDebugFile

#define kDebugFileName  "\p_Debug Ares "

struct debugRandomType {
    long    seed;
    OSType  sig;
};

extern aresGlobalType           *gAresGlobal;
//extern long gAresGlobal->gGameTime;
//extern short  gAresGlobal->gMainResRefNum;

long    gRandomSeed = 14586, gDebugWhichRandom = 0;

Handle  gDebugRandomSave = nil;

int RandomInit( void)
{

    // short   refNum = CurResFile();

#ifdef kSaveToDebugFile
    DebugFileInit();
    DebugFileAppendString("\p<START DEBUG FILE>\r\r");
#endif
#ifdef kRandomRecord
    gDebugRandomSave = NewHandle( sizeof( debugRandomType) * kRandomDebugTableSize);
    if ( gDebugRandomSave != nil)
    {
        MoveHHi( gDebugRandomSave);
        HLock( gDebugRandomSave);
        return ( 0);
    } else
    {
        return ( -1);
    }
#endif
#ifdef kRandomReplay
    UseResFile( gAresGlobal->gMainResRefNum);
    gDebugRandomSave = GetResource( 'Rand', 500);
    UseResFile( refNum);
    if ( gDebugRandomSave != nil)
    {
        DetachResource( gDebugRandomSave);
        MoveHHi( gDebugRandomSave);
        HLock( gDebugRandomSave);
        return( 0);
    } else
    {
        return( -1);
    }
#endif
    return( 0);
}

void RandomCleanup( void)
{
    if ( gDebugRandomSave != nil)
    {
#ifdef kRandomRecord
        SetHandleSize( gDebugRandomSave, (gDebugWhichRandom + 8) * sizeof( debugRandomType));
        if ( SaveAnyResourceInPreferences( 'Rand', 500, gDebugRandomSave, true) == noErr)
        {
        } else SysBeep( 20);
#endif
        gDebugRandomSave = nil;
    }
#ifdef kSaveToDebugFile
    DebugFileSave( kDebugFileName);
    DebugFileCleanup();
#endif
}

int Randomize( int range)
{
    long rawResult;

    if ( range == 0) return ( 0);
    rawResult = Random();
    if (rawResult < 0) rawResult *= -1;
    return( (rawResult * range) / 32768);
}

void DebugRandomReset( void)
{
#ifdef kSaveToDebugFile
    DebugFileCleanup();
    DebugFileInit();
    DebugFileAppendString("\p<START DEBUG FILE>\r\r");
#endif
    gDebugWhichRandom = 0;
}

short XRandomSeeded( short range, long *seed)

{
    short   r;
    long    l;

    /*
    qd.randSeed = 150000; //128000;
    WriteDebugInt( Random());

    qd.randSeed = 150000;
    WriteDebugInt( Randomize( range));
    */

//  *seed = (*seed * 16807L) & 0x7fffffff;// % 2147483647L;
    *seed = 1664525 * *seed + 1013904223;
    l = *seed & 0x00007fff;
    l *= range;

    l >>= 15L;
    r = l;

    return (r);
//  return ( Randomize( range));
}

void SetMyRandomSeed( long l)

{
    gRandomSeed = l;
}

//
// From develop 21 p105:
//
// The Random function in QuickDraw is based on the formula
// randSeed := (randSeed * 16807) MOD 2,147,483,647
// It returns a signed 16-bit number, and updates the unsigned 32-bit low-memory global randSeed. The reference used when implementing this random number generator was Linus Schrage, "A More Portable FORTRAN Random Number Generator," ACM Transactions on Mathematical Software Vol. 5, No. 2, June 1979, pages 132-138.
// The RandomX function in SANE uses the iteration formula
// r = (7^5 * r) mod (2^31 - 1)
// as documented on page 67 of the Apple Numerics Manual, Second Edition.
//

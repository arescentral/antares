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
        HUnlock( gDebugRandomSave);
        DisposeHandle( gDebugRandomSave);
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

#if 0
short DebugRandomSeeded( short range, long *seed, char *file, OSType sig, long objectType)
{
    short           result = 0;
    debugRandomType *dr = reinterpret_cast<debugRandomType*>(*gDebugRandomSave) + gDebugWhichRandom;
    Str255          s21;
    char            *hackchar2;

#pragma unused( file)

        hackchar2 = reinterpret_cast<char *>(&sig);
        s21[0] = 4;
        s21[1] = *hackchar2++;
        s21[2] = *hackchar2++;
        s21[3] = *hackchar2++;
        s21[4] = *hackchar2++;
    /*
    DebugFileAppendLong( gAresGlobal->gGameTime);
    DebugFileAppendString( "\p\t");
    DebugFileAppendCString( file);
    DebugFileAppendString( "\p\t");
    DebugFileAppendLong( line);
    DebugFileAppendString( "\p\t");
    result = XRandomSeeded( range, seed);
    DebugFileAppendLong( result);
    DebugFileAppendString( "\p\r");
    */

#ifdef kSaveToDebugFile
    if ( *seed == gRandomSeed)
    {
        DebugFileAppendString( "\pRG\t");
    } else
    {
        DebugFileAppendString( "\pLO\t");
    }
        DebugFileAppendLong( gAresGlobal->gGameTime);
        DebugFileAppendString( "\p\t");
        DebugFileAppendLong( *seed);
        DebugFileAppendString( "\p\t");
        DebugFileAppendString( s21);
        DebugFileAppendString( "\p\t");
        DebugFileAppendLong( objectType);
        DebugFileAppendString( "\p\r");
#endif

    if ( gDebugRandomSave != nil)
    {
#ifdef kRandomRecord
        Str255  s1;
        char    *hackchar;

        hackchar = (char *)&sig;
        s1[0] = 4;
        s1[1] = *hackchar++;
        s1[2] = *hackchar++;
        s1[3] = *hackchar++;
        s1[4] = *hackchar++;

        dr->seed = *seed;
        dr->sig = sig;
        if ( gDebugWhichRandom >= kRandomDebugTableSize)
        {
            ShowErrorRecover( MEMORY_ERROR, "\pDebug Table Full", 0);
#ifdef kSaveToDebugFile
            DebugFileAppendString("\p\r\r<END DEBUG FILE>");
            DebugFileSave( kDebugFileName);
#endif
            RandomCleanup();
        }
#else
        if ( gDebugWhichRandom >= kRandomDebugTableSize)
        {
            ShowSimpleStringAlert( "\pEnd of Debug Random Table.", nil, nil, nil);
            RandomCleanup();
        } else if ( dr->sig != sig)
        {
            Str255  s1, s2;
            char    *hackchar;

            hackchar = reinterpret_cast<char *>(&sig);
            s2[0] = 4;
            s2[1] = *hackchar++;
            s2[2] = *hackchar++;
            s2[3] = *hackchar++;
            s2[4] = *hackchar++;

            hackchar = reinterpret_cast<char *>(&dr->sig);
            s1[0] = 4;
            s1[1] = *hackchar++;
            s1[2] = *hackchar++;
            s1[3] = *hackchar++;
            s1[4] = *hackchar++;

            ShowSimpleStringAlert( "\pWrong SIG (want/got): ", s1, "\p / ", s2);

#ifdef kSaveToDebugFile
            DebugFileAppendString("\p\rWrong SIG. Wanted: ");
            DebugFileAppendString(s1);
            DebugFileAppendString("\p Got: ");
            DebugFileAppendString(s2);

            DebugFileAppendString("\p\r\r<END DEBUG FILE>");
            DebugFileSave( kDebugFileName);
#endif
            RandomCleanup();

        } else if ( dr->seed != *seed)
        {
            Str255  s1, s2, s3;
            char    *hackchar;

            hackchar = reinterpret_cast<char *>(&dr->sig);
            s1[0] = 4;
            s1[1] = *hackchar++;
            s1[2] = *hackchar++;
            s1[3] = *hackchar++;
            s1[4] = *hackchar++;

            NumToString( dr->seed, s2);
            NumToString( *seed, s3);
            ShowSimpleStringAlert( "\pWrong SEED (want/got): ", s2, s1, s3);

#ifdef kSaveToDebugFile
            DebugFileAppendString("\p\rWrong SEED. Wanted: ");
            DebugFileAppendString(s2);
            DebugFileAppendString("\p Got: ");
            DebugFileAppendString(s3);

            DebugFileAppendString("\p\r\r<END DEBUG FILE>");
            DebugFileSave( kDebugFileName);
#endif
            RandomCleanup();
        }
#endif
        gDebugWhichRandom++;
    }
    result = XRandomSeeded( range, seed);
    return ( result);
}
#endif

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

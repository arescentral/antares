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

#ifndef ANTARES_RANDOMIZE_HPP_
#define ANTARES_RANDOMIZE_HPP_

// Randomize.h

#include <Base.h>

#pragma options align=mac68k

#define kRandomRange        32767L

int RandomInit( void);
void RandomCleanup( void);
int Randomize( int);
void RandomTableCleanup( void);
void DebugRandomReset( void);
short DebugRandomSeeded( short, long *, char *, OSType, long);
short XRandomSeeded( short, long *);
void SetMyRandomSeed( long);

inline short RandomSeeded(short mrange, long* mseed, long, long) {
    return XRandomSeeded(mrange, mseed);
}

#pragma options align=reset

#endif // ANTARES_RANDOMIZE_HPP_

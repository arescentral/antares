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

#ifndef ANTARES_DEBUG_HPP_
#define ANTARES_DEBUG_HPP_

// Debug.h

#include <Base.h>

#define DEBUG_ON        TRUE
#include "MathSpecial.hpp"

#pragma options align=mac68k

void DebugWindowInit( WindowPtr);
void BringDebugToFront( void);
void DebugWindowCleanup ( void);
void ScrollDebugWindowUp ( void);
void WriteDebugLine( const unsigned char *);
void WriteDebugHex( unsigned long, unsigned long);
void WriteDebugFixed( Fixed);
void WriteDebugSmallFixed( smallFixedType);
void WriteDebugInt( int);
void WriteDebug2Int ( int, int);
void WriteDebugLong ( long);
void MoveDebugToFront ( void);
void WriteDebugDivider( void);
void WriteDebugHexDump( Ptr, long);
void WriteDebugChar( char);

inline void mWriteDebugString(const unsigned char* mdstring) {
    WriteDebugLine(mdstring);
}

unsigned long powerto ( unsigned long, unsigned long);
Boolean CommandPeriod( void);

void DebugFileInit( void);
void DebugFileCleanup( void);
void DebugFileAppendString( const unsigned char*);
void DebugFileAppendCString( char *);
void DebugFileAppendLong( long);
void DebugFileAppendLongHex( long);
void DebugFileAppendSmallFixed( smallFixedType);
void DebugFileSave( const unsigned char*);

#pragma options align=reset

#endif // ANTARES_DEBUG_HPP_

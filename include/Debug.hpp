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

#include "MathSpecial.hpp"

inline void DebugWindowInit(WindowPtr) { }
inline void BringDebugToFront() { }
inline void DebugWindowCleanup() { }
inline void ScrollDebugWindowUp() { }
inline void WriteDebugLine(const unsigned char*) { }
inline void WriteDebugHex(unsigned long, unsigned long) { }
inline void WriteDebugFixed(Fixed) { }
inline void WriteDebugSmallFixed(smallFixedType) { }
inline void WriteDebugInt(int) { }
inline void WriteDebug2Int(int, int) { }
inline void WriteDebugLong(long) { }
inline void MoveDebugToFront() { }
inline void WriteDebugDivider() { }
inline void WriteDebugHexDump(Ptr, long) { }
inline void WriteDebugChar(char) { }

inline void mWriteDebugString(const unsigned char* mdstring) {
    WriteDebugLine(mdstring);
}

inline void DebugFileInit() { }
inline void DebugFileCleanup() { }
inline void DebugFileAppendString(const unsigned char*) { }
inline void DebugFileAppendCString(char*) { }
inline void DebugFileAppendLong(long) { }
inline void DebugFileAppendLongHex(long) { }
inline void DebugFileAppendSmallFixed(smallFixedType) { }
inline void DebugFileSave(const unsigned char*) { }

#endif // ANTARES_DEBUG_HPP_

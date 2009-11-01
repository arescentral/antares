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

#ifndef ANTARES_ARES_CHEAT_HPP_
#define ANTARES_ARES_CHEAT_HPP_

// Ares Cheat.h

#include "Base.h"

namespace antares {

#define kCheatActiveBit                 0x00000001
#define kAutoPlayBit                    0x00000002
#define kNameObjectBit                  0x00000004
#define kObserverBit                    0x00000008
#define kBuildFastBit                   0x00000010
#define kRaisePayRateBit                0x00000020
#define kLowerPayRateBit                0x00000040
#define kCheatBit8                      0x00000080
#define kCheatBit9                      0x00000100
#define kCheatBit10                     0x00000200
#define kCheatBit11                     0x00000400
#define kCheatBit12                     0x00000800
#define kCheatBit13                     0x00001000
#define kCheatBit14                     0x00002000
#define kCheatBit15                     0x00004000
#define kCheatBit16                     0x00008000
#define kCheatBit17                     0x00010000
#define kCheatBit18                     0x00020000
#define kCheatBit19                     0x00040000
#define kCheatBit20                     0x00080000
#define kCheatBit21                     0x00100000
#define kCheatBit22                     0x00200000
#define kCheatBit23                     0x00400000
#define kCheatBit24                     0x00800000
#define kCheatBit25                     0x01000000
#define kCheatBit26                     0x02000000
#define kCheatBit27                     0x04000000
#define kCheatBit28                     0x08000000
#define kCheatBit29                     0x10000000
#define kCheatBit30                     0x20000000
#define kCheatBit31                     0x40000000
#define kCheatBit32                     0x80000000

void AresCheatInit( void);
void CleanupAresCheat( void);
short GetCheatNumFromString(unsigned char*);
void ExecuteCheat( short, long);

}  // namespace antares

#endif // ANTARES_ARES_CHEAT_HPP_

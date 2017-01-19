// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// Copyright (C) 2008-2017 The Antares Authors
//
// This file is part of Antares, a tactical space combat game.
//
// Antares is free software: you can redistribute it and/or modify it
// under the terms of the Lesser GNU General Public License as published
// by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Antares is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with Antares.  If not, see http://www.gnu.org/licenses/

#ifndef ANTARES_GAME_CHEAT_HPP_
#define ANTARES_GAME_CHEAT_HPP_

#include <sfz/sfz.hpp>

#include "game/globals.hpp"

namespace antares {

enum {
    kCheatActiveBit  = 0x00000001,
    kAutoPlayBit     = 0x00000002,
    kNameObjectBit   = 0x00000004,
    kObserverBit     = 0x00000008,
    kBuildFastBit    = 0x00000010,
    kRaisePayRateBit = 0x00000020,
    kLowerPayRateBit = 0x00000040,
    kCheatBit8       = 0x00000080,
    kCheatBit9       = 0x00000100,
    kCheatBit10      = 0x00000200,
    kCheatBit11      = 0x00000400,
    kCheatBit12      = 0x00000800,
    kCheatBit13      = 0x00001000,
    kCheatBit14      = 0x00002000,
    kCheatBit15      = 0x00004000,
    kCheatBit16      = 0x00008000,
    kCheatBit17      = 0x00010000,
    kCheatBit18      = 0x00020000,
    kCheatBit19      = 0x00040000,
    kCheatBit20      = 0x00080000,
    kCheatBit21      = 0x00100000,
    kCheatBit22      = 0x00200000,
    kCheatBit23      = 0x00400000,
    kCheatBit24      = 0x00800000,
    kCheatBit25      = 0x01000000,
    kCheatBit26      = 0x02000000,
    kCheatBit27      = 0x04000000,
    kCheatBit28      = 0x08000000,
    kCheatBit29      = 0x10000000,
    kCheatBit30      = 0x20000000,
    kCheatBit31      = 0x40000000,
    kCheatBit32      = 0x80000000,
};

int16_t GetCheatNumFromString(const sfz::StringSlice& string);
void ExecuteCheat(int16_t whichCheat, Handle<Admiral> whichPlayer);

}  // namespace antares

#endif  // ANTARES_GAME_CHEAT_HPP_

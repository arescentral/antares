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

int32_t gRandomSeed = 14586;

int RandomInit() {
    return 0;
}

void RandomCleanup() {
}

int Randomize(int range) {
    int32_t rawResult;

    if (range == 0) {
        return 0;
    }
    rawResult = Random();
    if (rawResult < 0) {
        rawResult *= -1;
    }
    return (rawResult * range) / 32768;
}

int16_t XRandomSeeded(int16_t range, int32_t* seed) {
    *seed = 1664525 * *seed + 1013904223;
    int32_t l = *seed & 0x00007fff;
    return (l * range) >> 15;
}

//
// From develop 21 p105:
//
// The Random function in QuickDraw is based on the formula
//
//   randSeed := (randSeed * 16807) MOD 2,147,483,647
//
// It returns a signed 16-bit number, and updates the unsigned 32-bit low-memory global randSeed.
// The reference used when implementing this random number generator was Linus Schrage, "A More
// Portable FORTRAN Random Number Generator," ACM Transactions on Mathematical Software Vol. 5, No.
// 2, June 1979, pages 132-138.
//
// The RandomX function in SANE uses the iteration formula
//
//   r = (7^5 * r) mod (2^31 - 1)
//
// as documented on page 67 of the Apple Numerics Manual, Second Edition.
//

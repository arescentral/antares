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

#ifndef ANTARES_STRING_NUMERICS_HPP_
#define ANTARES_STRING_NUMERICS_HPP_

#include "sfz/PrintTarget.hpp"
#include "MathSpecial.hpp"

namespace antares {

struct SmallFixed {
    smallFixedType value;
    explicit SmallFixed(smallFixedType v) : value(v) { }
};
SmallFixed small_fixed(smallFixedType value);
void print_to(sfz::PrintTarget out, const SmallFixed& fixed);

void SmallFixedToString( smallFixedType, Str255);
smallFixedType StringToSmallFixed( Str255);
void DoubleToString(double, unsigned char*, long);
double StringToDouble(unsigned char* s);
void NumToHexString( unsigned long, Str255, long);
void HexStringToNum( Str255, unsigned long *);
void UnsignedLongToString(unsigned long, unsigned char*);
unsigned long StringToUnsignedLong(unsigned char*);
unsigned char* LongToString(long, unsigned char*);
long StringToLong(unsigned char*);

inline void NumToString(unsigned long num, unsigned char* string) {
    UnsignedLongToString(num, string);
}

}  // namespace antares

#endif // ANTARES_STRING_NUMERICS_HPP_

// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// Copyright (C) 2008-2012 The Antares Authors
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

#ifndef ANTARES_MATH_FIXED_HPP_
#define ANTARES_MATH_FIXED_HPP_

#include <math.h>
#include <sfz/sfz.hpp>

namespace antares {

typedef int32_t Fixed;

//
//  MAX VALUE FOR SMALLFIXEDTYPE:
//
//  8,388,607       normal
//  4,194,303       addition
//  2,896           multiplication -- see below
//  32,768          division
//

// Convert a fixed-point number to long.
//
// Both the evil and more evil variants return correct results when `value` is zero or positive
// (that is, ``value / 256``), but they differ in how wrong they are with negative values.  The
// evil variant returns ``(value / 256) + 1`` when 256 evenly divides `value`, returning correct
// results otherwise.  The more evil variant returns the evil result - 1, returning correct results
// only when 256 evenly divides `value`.
inline int32_t evil_fixed_to_long(Fixed value) {
    if (value < 0) {
        return (value >> 8) + 1;
    } else {
        return value >> 8;
    }
}
inline int32_t more_evil_fixed_to_long(Fixed value) {
    return value >> 8;
}

inline Fixed mLongToFixed(int32_t m_l)  { return m_l << 8; }
inline Fixed mFloatToFixed(float m_r)   { return roundf(m_r * 256.0); }
inline float mFixedToFloat(Fixed m_f)   { return floorf(m_f * 1e3 / 256.0) / 1e3; }
inline int32_t mFixedToLong(Fixed m_f)  { return evil_fixed_to_long(m_f); }

struct PrintableFixed {
    Fixed value;
    explicit PrintableFixed(Fixed v) : value(v) { }
};
PrintableFixed fixed(Fixed value);
void print_to(sfz::PrintTarget out, const PrintableFixed& fixed);

// the max safe # we can do is 181 for signed multiply if we don't know other value
// if -1 <= other value <= 1 then we can do 32767
inline Fixed mMultiplyFixed(Fixed m_f1, Fixed m_f2) {
    return (m_f1 * m_f2) >> 8;
}
inline Fixed mDivideFixed(Fixed m_f1, Fixed m_f2) {
    return (m_f1 << 8) / m_f2;
}

struct fixedPointType {
    Fixed               h;
    Fixed               v;
};
void read_from(sfz::ReadSource in, fixedPointType& point);

}  // namespace antares

#endif // ANTARES_MATH_FIXED_HPP_

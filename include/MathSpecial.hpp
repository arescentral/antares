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

#ifndef ANTARES_MATH_SPECIAL_HPP_
#define ANTARES_MATH_SPECIAL_HPP_

// Math Special.h

#include "Base.h"

#include "Casts.hpp"

namespace antares {

class BinaryReader;

#define kMathSpecial

#ifndef powerc
#define WideSubtract    MyWideSubtract
#endif

//
//  MAX VALUE FOR SMALLFIXEDTYPE:
//
//  8,388,607       normal
//  4,194,303       addition
//  2,896           multiplication -- see below
//  32,768          division
//

#define kFixedOneQuarter            0x00000040
#define kFixedPlusPointFive         0x00000080
#define kFixedNegativePointFive     0xffffff80
#define kFixedOne                   0x00000100
#define kFixedNegativeOne           0xfffff100
#define kFixedTwo                   0x00000200
#define kFixedNegativeTwo           0xfffff200

#define kFixedWholeMultiplier       256.0
#define kFixedBitShiftNumber        implicit_cast<long>(8)

inline Fixed mLongToFixed(int32_t m_l)  { return m_l << kFixedBitShiftNumber; }
inline Fixed mFloatToFixed(float m_r)   { return m_r * kFixedWholeMultiplier; }
inline float mFixedToFloat(Fixed m_f)   { return implicit_cast<float>(m_f) / kFixedWholeMultiplier; }
inline int32_t mFixedToLong(Fixed m_f) {
    if (m_f < 0) {
        return (m_f >> kFixedBitShiftNumber) + 1;
    } else {
        return m_f >> kFixedBitShiftNumber;
    }
}

// the max safe # we can do is 181 for signed multiply if we don't know other value
// if -1 <= other value <= 1 then we can do 32767
inline Fixed mMultiplyFixed(Fixed m_f1, Fixed m_f2) {
    return (m_f1 * m_f2) >> kFixedBitShiftNumber;
}
inline Fixed mDivideFixed(Fixed m_f1, Fixed m_f2) {
    return (m_f1 << kFixedBitShiftNumber) / m_f2;
}

typedef int32_t smallFixedType;    // distinct from Mac OS's Fixed type

struct fixedPointType {
    smallFixedType      h;
    smallFixedType      v;

    void read(BinaryReader* bin);
};


uint32_t lsqrt(uint32_t);

Fixed MyFixRatio(int16_t, int16_t);
void MyMulDoubleLong(int32_t, int32_t, int64_t*);

template <typename T>
inline void MyWideMul(int32_t mlong1, int32_t mlong2, T* mwide) {
    *mwide = implicit_cast<int64_t>(mlong1) * implicit_cast<int64_t>(mlong2);
}

}  // namespace antares

#endif // ANTARES_MATH_SPECIAL_HPP_

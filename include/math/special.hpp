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

#ifndef ANTARES_MATH_SPECIAL_HPP_
#define ANTARES_MATH_SPECIAL_HPP_

#include "lang/casts.hpp"
#include "math/fixed.hpp"

namespace antares {

uint32_t     lsqrt(uint32_t n);
inline Fixed lsqrt(Fixed n) { return Fixed::from_val(lsqrt(n.val())); }
uint64_t     wsqrt(uint64_t n);

Fixed          MyFixRatio(int16_t, int16_t);
int16_t        ratio_to_angle(Fixed x, Fixed y);
inline int16_t ratio_to_angle(int32_t x, int32_t y) {
    return ratio_to_angle(Fixed::from_val(x), Fixed::from_val(y));
}
void MyMulDoubleLong(int32_t, int32_t, int64_t*);

template <typename T>
inline void MyWideMul(int32_t mlong1, int32_t mlong2, T* mwide) {
    *mwide = implicit_cast<int64_t>(mlong1) * implicit_cast<int64_t>(mlong2);
}

template <typename T>
inline T MyWideMul(int32_t mlong1, int32_t mlong2) {
    return implicit_cast<int64_t>(mlong1) * implicit_cast<int64_t>(mlong2);
}

int32_t AngleFromSlope(Fixed slope);

}  // namespace antares

#endif  // ANTARES_MATH_SPECIAL_HPP_

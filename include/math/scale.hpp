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

#ifndef ANTARES_MATH_SCALE_HPP_
#define ANTARES_MATH_SCALE_HPP_

#include <stdint.h>

#include "math/fixed.hpp"
#include "math/geometry.hpp"

namespace antares {

struct Scale {
    int32_t factor;

    bool operator==(Scale y) const { return factor == y.factor; }
    bool operator!=(Scale y) const { return factor != y.factor; }
    bool operator<(Scale y) const { return factor < y.factor; }
    bool operator<=(Scale y) const { return factor <= y.factor; }
    bool operator>(Scale y) const { return factor > y.factor; }
    bool operator>=(Scale y) const { return factor >= y.factor; }

    Scale   operator*(int32_t y) const { return Scale{factor * y}; }
    Scale   operator/(int32_t y) const { return Scale{factor / y}; }
    int32_t operator/(Scale y) const { return factor / y.factor; }

    Scale& operator*=(int32_t y) { return *this = *this * y; }
    Scale& operator/=(int32_t y) { return *this = *this / y; }
};

inline Scale operator*(int32_t x, Scale y) { return y * x; }

// Scale `value` by `scale`.
//
// The normal variant calculates the final scale as ``(value * scale) >>
// 12``. The star variant calculates the final scale as ``(value *
// scale) / 4096``, which is potentially more appropriate when negative
// values are likely.
int32_t scale_by(int32_t value, Scale scale);
Fixed   scale_by(Fixed value, Scale scale);
Scale   scale_by(Scale value, Scale scale);
Size    scale_by(Size s, Scale scale);
Fixed   star_scale_by(Fixed value, Scale scale);

const int32_t SHIFT_SCALE = 12;
const Scale   SCALE_SCALE{1 << SHIFT_SCALE};
const Scale   MIN_SCALE{256};

const Scale kOneEighthScale{SCALE_SCALE.factor / 8};
const Scale kOneQuarterScale{SCALE_SCALE.factor / 4};
const Scale kOneHalfScale{SCALE_SCALE.factor / 2};
const Scale kTimesTwoScale{SCALE_SCALE.factor * 2};

}  // namespace antares

#endif  // ANTARES_MATH_SCALE_HPP_

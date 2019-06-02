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

namespace antares {

struct Scale {
    int32_t factor;
};

// Scale `value` by `scale`.
//
// The regular variant calculates the final scale as ``(value * scale) / 4096``.  The evil variant
// calculates the final scale as ``(value * scale) >> 12``, which results in off-by-one errors when
// `value` is negative.
Fixed   scale_by(Fixed value, Scale scale);
Fixed   evil_scale_by(Fixed value, Scale scale);
int32_t evil_scale_by(int32_t value, Scale scale);

const int32_t SHIFT_SCALE = 12;
const Scale   SCALE_SCALE{1 << SHIFT_SCALE};
const Scale   MIN_SCALE{256};

const Scale kOneEighthScale{SCALE_SCALE.factor / 8};
const Scale kOneQuarterScale{SCALE_SCALE.factor / 4};
const Scale kOneHalfScale{SCALE_SCALE.factor / 2};
const Scale kTimesTwoScale{SCALE_SCALE.factor * 2};

}  // namespace antares

#endif  // ANTARES_MATH_SCALE_HPP_

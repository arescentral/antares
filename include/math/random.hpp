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

#ifndef ANTARES_MATH_RANDOM_HPP_
#define ANTARES_MATH_RANDOM_HPP_

#include <stdint.h>

#include "math/fixed.hpp"
#include "math/units.hpp"

namespace antares {

struct Random {
    int32_t seed;

    int16_t next(int16_t range);
    ticks   next(ticks range) { return ticks(next(range.count())); }
    Fixed   next(Fixed range) { return Fixed::from_val(next(range.val())); }
};

int          Randomize(int range);
inline Fixed Randomize(Fixed range) { return Fixed::from_val(Randomize(range.val())); }

}  // namespace antares

#endif  // ANTARES_MATH_RANDOM_HPP_

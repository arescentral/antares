// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// Copyright (C) 2008-2011 Ares Central
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
// License along with this program.  If not, see
// <http://www.gnu.org/licenses/>.

#ifndef ANTARES_MATH_MACROS_HPP_
#define ANTARES_MATH_MACROS_HPP_

#include "math/geometry.hpp"

namespace antares {

template <typename T>
inline T ABS(T x) {
    if (x >= 0) {
        return x;
    } else {
        return -x;
    }
}

inline int mClipCode(int x, int y, const Rect& bounds) {
    return ((x < bounds.left) << 3)
        | ((x > (bounds.right - 1)) << 2)
        | ((y < bounds.top) << 1)
        | (y > ( bounds.bottom - 1));
}

}  // namespace antares

#endif // ANTARES_MATH_MACROS_HPP_

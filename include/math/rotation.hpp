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

#ifndef ANTARES_MATH_ROTATION_HPP_
#define ANTARES_MATH_ROTATION_HPP_

#include <stdint.h>

#include "math/fixed.hpp"

namespace antares {

enum {
    ROT_0   = 0,
    ROT_45  = 45,
    ROT_90  = 90,
    ROT_180 = 180,
    ROT_POS = 360,
};

// mAngleDifference: get the smallest difference from theta to other
inline int32_t mAngleDifference(int32_t theta, int32_t other) {
    if (theta >= other) {
        if ((theta - other) > ROT_180) {
            return other - theta + ROT_POS;
        } else {
            return other - theta;
        }
    } else {
        if ((other - theta) > ROT_180) {
            return other - theta - ROT_POS;
        } else {
            return other - theta;
        }
    }
}

// XXX: There are places where the change from a macro to an inline function could have had a real
// effect.  In particular, NonPlayerShip.cpp:1350, :1673, :1804, :2216, :2513 look like they would
// have had their "else if" clauses reinterpreted as a branch of the below statement, rather than
// the intended ones in that file.
template <typename T>
inline void mAddAngle(T& theta, int32_t other) {
    theta += (other);
    if (theta >= ROT_POS) {
        theta -= ROT_POS;
    } else if (theta < 0) {
        theta += ROT_POS;
    }
}

void GetRotPoint(Fixed* x, Fixed* y, int32_t rotpos);
int32_t GetAngleFromVector(int32_t x, int32_t y);

}  // namespace antares

#endif  // ANTARES_MATH_ROTATION_HPP_

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

#include "math/rotation.hpp"

#include "data/resource.hpp"
#include "game/sys.hpp"
#include "lang/defines.hpp"
#include "math/macros.hpp"

namespace antares {

void GetRotPoint(Fixed* x, Fixed* y, int32_t rotpos) {
    int32_t* i;

    i  = sys.rot_table.data() + rotpos * 2L;
    *x = Fixed::from_val(*i);
    i++;
    *y = Fixed::from_val(*i);
}

int32_t GetAngleFromVector(int32_t x, int32_t y) {
    int32_t* h;
    int32_t* v;
    int32_t  a, b, test = 0, best = 0, whichBest = -1, whichAngle;

    a = x;
    b = y;

    if (a < 0)
        a = -a;
    if (b < 0)
        b = -b;
    if (b < a) {
        h          = sys.rot_table.data() + ROT_45 * 2;
        whichAngle = ROT_45;
        v          = h + 1;
        do {
            test = (*v * a) + (*h * b);  // we're adding b/c in my table 45-90 degrees, h < 0
            if (test < 0)
                test = -test;
            if ((whichBest < 0) || (test < best)) {
                best      = test;
                whichBest = whichAngle;
            }
            h += 2;
            v += 2;
            whichAngle++;
        } while ((test == best) && (whichAngle <= ROT_90));
    } else {
        h          = sys.rot_table.data() + ROT_0 * 2;
        whichAngle = ROT_0;
        v          = h + 1;
        do {
            test = (*v * a) + (*h * b);
            if (test < 0)
                test = -test;
            if ((whichBest < 0) || (test < best)) {
                best      = test;
                whichBest = whichAngle;
            }
            h += 2;
            v += 2;
            whichAngle++;
        } while ((test == best) && (whichAngle <= ROT_45));
    }
    if (x > 0) {
        if (y < 0)
            whichBest = whichBest + ROT_180;
        else
            whichBest = ROT_POS - whichBest;
    } else if (y < 0)
        whichBest = ROT_180 - whichBest;
    if (whichBest == ROT_POS)
        whichBest = ROT_0;
    return (whichBest);
}

}  // namespace antares

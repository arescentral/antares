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

#include "math/fixed.hpp"

#include <cmath>
#include <sfz/sfz.hpp>

using sfz::PrintTarget;
using sfz::range;
using std::abs;

namespace antares {

PrintableFixed fixed(Fixed value) {
    return PrintableFixed(value);
}

void print_to(PrintTarget out, const PrintableFixed& fixed) {
    if (fixed.value < 0) {
        out.push(1, '-');
    }
    const int32_t value = abs(fixed.value);
    const int32_t integral = mFixedToLong(value);
    print(out, integral);
    out.push(1, '.');
    double fraction = (value - mLongToFixed(integral)) / 256.0;
    SFZ_FOREACH(int i, range(3), {
        fraction -= floor(fraction);
        fraction *= 10;
        int digit = fraction;
        out.push(1, '0' + digit);
    });
}

}  // namespace antares

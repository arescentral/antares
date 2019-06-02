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

#include "math/scale.hpp"

namespace antares {

int32_t scale_by(int32_t value, Scale scale) { return (value * scale.factor) >> SHIFT_SCALE; }

Scale scale_by(Scale value, Scale scale) { return Scale{scale_by(value.factor, scale)}; }

Fixed scale_by(Fixed value, Scale scale) { return (value * scale.factor) >> SHIFT_SCALE; }

Size scale_by(Size s, Scale scale) {
    return Size{scale_by(s.width, scale), scale_by(s.height, scale)};
}

Fixed star_scale_by(Fixed value, Scale scale) {
    return (value * scale.factor) / SCALE_SCALE.factor;
}

}  // namespace antares

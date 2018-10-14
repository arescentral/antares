// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// Copyright (C) 2018 The Antares Authors
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

#ifndef ANTARES_DATA_DISTANCE_HPP_
#define ANTARES_DATA_DISTANCE_HPP_

#include <stdint.h>

namespace antares {

class path_value;

struct Distance {
    int64_t squared;
};

template <typename T>
struct field_reader;
template <>
struct field_reader<Distance> {
    static Distance read(path_value x);
};

}  // namespace antares

#endif  // ANTARES_DATA_DISTANCE_HPP_

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

#ifndef ANTARES_DATA_RACES_HPP_
#define ANTARES_DATA_RACES_HPP_

#include <stdint.h>
#include <stdlib.h>
#include <map>
#include <pn/string>

#include "data/enums.hpp"
#include "data/handle.hpp"
#include "math/fixed.hpp"

namespace antares {

class BaseObject;

struct Race {
    int32_t    numeric;
    pn::string singular;
    pn::string plural;
    pn::string military;
    pn::string homeworld;
    Hue        apparentColor;
    Fixed      advantage;
    // uint32_t   illegalColors;

    std::map<pn::string, Handle<BaseObject>> ships;

    static Race*        get(int n);
    static Handle<Race> none() { return Handle<Race>(-1); }

    static const size_t byte_size = 14;
};

Race race(pn::value_cref x);

}  // namespace antares

#endif  // ANTARES_DATA_RACES_HPP_

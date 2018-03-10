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

#include "data/races.hpp"

#include "data/level.hpp"
#include "data/plugin.hpp"
#include "data/resource.hpp"
#include "game/globals.hpp"
#include "lang/defines.hpp"

using std::unique_ptr;

namespace antares {

Race race(pn::value_cref x0) {
    if (!x0.is_map()) {
        throw std::runtime_error("must be map");
    }

    path_value x{x0};
    Race       r;
    r.numeric       = required_int(x.get("numeric"));
    r.singular      = required_string(x.get("singular")).copy();
    r.plural        = required_string(x.get("plural")).copy();
    r.military      = required_string(x.get("military")).copy();
    r.homeworld     = required_string(x.get("homeworld")).copy();
    r.apparentColor = required_hue(x.get("apparent_color"));
    r.advantage     = required_fixed(x.get("advantage"));

    path_value ships = x.get("ships");
    if (ships.value().is_null()) {
        r.ships.clear();
    } else if (ships.value().is_map()) {
        for (pn::key_value_cref kv : ships.value().as_map()) {
            r.ships.emplace(kv.key().copy(), required_base(ships.get(kv.key())));
        }
    } else {
        throw std::runtime_error(pn::format("{0}: must be null or map", ships.path()).c_str());
    }

    return r;
}

}  // namespace antares

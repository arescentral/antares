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

std::map<pn::string, Handle<BaseObject>> optional_ships(path_value x) {
    if (x.value().is_null()) {
        return {};
    } else if (x.value().is_map()) {
        std::map<pn::string, Handle<BaseObject>> ships;
        for (pn::key_value_cref kv : x.value().as_map()) {
            ships.emplace(kv.key().copy(), required_base(x.get(kv.key())));
        }
        return ships;
    } else {
        throw std::runtime_error(pn::format("{0}: must be null or map", x.path()).c_str());
    }
}

pn::string required_string_copy(path_value x) { return required_string(x).copy(); }

Race race(path_value x) {
    return required_struct<Race>(
            x, {{"numeric", nullptr},
                {"singular", {&Race::singular, required_string_copy}},
                {"plural", {&Race::plural, required_string_copy}},
                {"military", {&Race::military, required_string_copy}},
                {"homeworld", {&Race::homeworld, required_string_copy}},
                {"apparent_color", {&Race::apparentColor, required_hue}},
                {"illegal_colors", nullptr},
                {"advantage", {&Race::advantage, required_fixed}},
                {"ships", {&Race::ships, optional_ships}}});
}

}  // namespace antares

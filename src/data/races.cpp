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

#include "data/field.hpp"
#include "data/level.hpp"
#include "data/plugin.hpp"
#include "data/resource.hpp"
#include "game/globals.hpp"
#include "lang/defines.hpp"

using std::unique_ptr;

namespace antares {

Race* Race::get(pn::string_view name) { return &plug.races[name.copy()]; }

Race race(path_value x) {
    return required_struct<Race>(
            x, {{"numeric", nullptr},
                {"adjective", &Race::adjective},
                {"plural", &Race::plural},
                {"military", &Race::military},
                {"homeworld", &Race::homeworld},
                {"hue", &Race::hue},
                {"not_hue", &Race::not_hue},
                {"advantage", &Race::advantage}});
}

}  // namespace antares

// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// Copyright (C) 2008-2018 The Antares Authors
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

#ifndef ANTARES_DATA_INITIAL_HPP_
#define ANTARES_DATA_INITIAL_HPP_

#include <pn/string>
#include <sfz/sfz.hpp>

#include "data/handle.hpp"
#include "math/fixed.hpp"
#include "math/geometry.hpp"

namespace antares {

class path_value;

const int32_t kMaxShipCanBuild = 6;

// Might be the name of a BaseObject, or of an entry in a Race’s “ships” list.
struct BuildableObject {
    pn::string name;
};

struct Initial {
    BuildableObject                base;
    sfz::optional<Handle<Admiral>> owner;
    Point                          at;
    sfz::optional<bool>            hide;
    sfz::optional<bool>            flagship;

    struct Target {
        sfz::optional<Handle<const Initial>> initial;
        sfz::optional<bool>                  lock;
    } target;

    struct Override {
        sfz::optional<pn::string> name;
        sfz::optional<pn::string> sprite;
    } override_;

    sfz::optional<Fixed>         earning;
    std::vector<BuildableObject> build;

    static const Initial*            get(int n);
    static Handle<const Initial>     none() { return Handle<const Initial>(-1); }
    static HandleList<const Initial> all();
};

template <typename T>
struct field_reader;
template <>
struct field_reader<Initial> {
    static Initial read(path_value x);
};

}  // namespace antares

#endif  // ANTARES_DATA_LEVEL_HPP_

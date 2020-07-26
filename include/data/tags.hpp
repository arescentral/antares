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

#ifndef ANTARES_DATA_TAGS_HPP_
#define ANTARES_DATA_TAGS_HPP_

#include <stdint.h>

#include <memory>
#include <pn/string>
#include <sfz/sfz.hpp>
#include <vector>

#include "data/enums.hpp"
#include "data/handle.hpp"
#include "data/object-ref.hpp"
#include "data/range.hpp"
#include "drawing/color.hpp"
#include "math/fixed.hpp"
#include "math/geometry.hpp"
#include "math/units.hpp"

namespace antares {

struct Tags {
    Tags()            = default;
    Tags(const Tags&) = delete;
    Tags(Tags&&)      = default;
    Tags& operator=(const Tags&) = delete;

    // Old libstdc++ has an issue that precludes default here.
    // When std::map’s key type is movable but not copyable,
    // the map is move-constructible but not move-assignable.
    // The issue is in libstdc++ 5.4.0 but is fixed by 9.3.0.
    Tags& operator=(Tags&& other) {
        std::swap(tags, other.tags);
        return *this;
    }

    std::map<pn::string, bool> tags;
};

}  // namespace antares

#endif  // ANTARES_DATA_TAGS_HPP_

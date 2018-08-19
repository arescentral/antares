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

#include "data/counter.hpp"

#include <pn/map>
#include <pn/value>

#include "data/field.hpp"

namespace antares {

DEFINE_FIELD_READER(Counter) {
    return required_struct<Counter>(x, {{"player", &Counter::player}, {"which", &Counter::which}});
}

DEFINE_FIELD_READER(sfz::optional<Counter>) {
    return optional_struct<Counter>(x, {{"player", &Counter::player}, {"which", &Counter::which}});
}

DEFINE_FIELD_READER(RelativeCounter) {
    return required_struct<RelativeCounter>(
            x, {{"player", &RelativeCounter::player}, {"which", &RelativeCounter::which}});
}

}  // namespace antares

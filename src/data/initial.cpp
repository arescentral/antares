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

#include "data/initial.hpp"

#include "data/field.hpp"

namespace antares {

FIELD_READER(BuildableObject) { return BuildableObject{read_field<pn::string>(x)}; }

FIELD_READER(std::vector<BuildableObject>) {
    std::vector<BuildableObject> objects =
            optional_array<BuildableObject, read_field<BuildableObject>>(x);
    if (objects.size() > kMaxShipCanBuild) {
        throw std::runtime_error(pn::format(
                                         "{0}has {1} elements, more than max of {2}", x.prefix(),
                                         objects.size(), kMaxShipCanBuild)
                                         .c_str());
    }
    return objects;
}

FIELD_READER(Initial::Override) {
    return optional_struct<Initial::Override>(
                   x, {{"name", &Initial::Override::name}, {"sprite", &Initial::Override::sprite}})
            .value_or(Initial::Override{});
}

FIELD_READER(Initial::Target) {
    return optional_struct<Initial::Target>(
                   x, {{"initial", &Initial::Target::initial}, {"lock", &Initial::Target::lock}})
            .value_or(Initial::Target{});
}

DEFINE_FIELD_READER(Initial) {
    return required_struct<Initial>(
            x, {{"base", &Initial::base},
                {"owner", &Initial::owner},
                {"at", &Initial::at},
                {"earning", &Initial::earning},
                {"hide", &Initial::hide},
                {"flagship", &Initial::flagship},
                {"override", &Initial::override_},
                {"target", &Initial::target},
                {"build", &Initial::build}});
}

}  // namespace antares

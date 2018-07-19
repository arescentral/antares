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

static BuildableObject required_buildable_object(path_value x) {
    return BuildableObject{required_string_copy(x)};
}

static std::vector<BuildableObject> optional_buildable_object_array(path_value x) {
    std::vector<BuildableObject> objects =
            optional_array<BuildableObject, required_buildable_object>(x);
    if (objects.size() > kMaxShipCanBuild) {
        throw std::runtime_error(pn::format(
                                         "{0}has {1} elements, more than max of {2}", x.prefix(),
                                         objects.size(), kMaxShipCanBuild)
                                         .c_str());
    }
    return objects;
}

static Initial::Override optional_override(path_value x) {
    return optional_struct<Initial::Override>(
                   x, {{"name", &Initial::Override::name}, {"sprite", &Initial::Override::sprite}})
            .value_or(Initial::Override{});
}

static Initial::Target optional_target(path_value x) {
    return optional_struct<Initial::Target>(
                   x, {{"initial", {&Initial::Target::initial, optional_initial, Initial::none()}},
                       {"lock", {&Initial::Target::lock, optional_bool, false}}})
            .value_or(Initial::Target{});
}

Initial initial(path_value x) {
    return required_struct<Initial>(
            x, {{"base", {&Initial::base, required_buildable_object}},
                {"owner", {&Initial::owner, optional_admiral, Handle<Admiral>(-1)}},
                {"at", &Initial::at},
                {"earning", {&Initial::earning, optional_fixed, Fixed::zero()}},
                {"hide", {&Initial::hide, optional_bool, false}},
                {"flagship", {&Initial::flagship, optional_bool, false}},
                {"override", {&Initial::override_, optional_override}},
                {"target", {&Initial::target, optional_target}},
                {"build", {&Initial::build, optional_buildable_object_array}}});
}

}  // namespace antares

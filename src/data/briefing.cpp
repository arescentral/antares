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

#include "data/briefing.hpp"

#include "data/field.hpp"
#include "data/initial.hpp"

namespace antares {

Briefing briefing(path_value x) {
    return required_struct<Briefing>(
            x, {
                       {"object", {&Briefing::object, optional_initial, Initial::none()}},
                       {"title", {&Briefing::title, required_string_copy}},
                       {"content", {&Briefing::content, required_string_copy}},
               });
}

}  // namespace antares

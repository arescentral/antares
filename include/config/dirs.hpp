// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// Copyright (C) 2013-2017 The Antares Authors
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

#ifndef ANTARES_CONFIG_DIRS_HPP_
#define ANTARES_CONFIG_DIRS_HPP_

#include <sfz/sfz.hpp>

namespace antares {

extern const char kFactoryScenarioIdentifier[];

struct Directories {
    sfz::String root;

    sfz::String downloads;
    sfz::String registry;
    sfz::String replays;
    sfz::String scenarios;
};

const Directories& dirs();

sfz::String default_application_path();
sfz::String application_path();
void set_application_path(sfz::StringSlice path);

sfz::String scenario_dir(sfz::StringSlice identifier);

}  // namespace antares

#endif  // ANTARES_CONFIG_DIRS_HPP_

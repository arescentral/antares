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

#include <pn/string>

namespace antares {

extern const char kFactoryScenarioIdentifier[];

struct Directories {
    pn::string root;

    pn::string downloads;
    pn::string registry;
    pn::string replays;
    pn::string scenarios;
};

const Directories& dirs();

pn::string_view default_application_path();
pn::string_view application_path();
void            set_application_path(pn::string_view path);

pn::string_view default_factory_scenario_path();
pn::string_view factory_scenario_path();
void            set_factory_scenario_path(pn::string_view path);

pn::string scenario_path();

}  // namespace antares

#endif  // ANTARES_CONFIG_DIRS_HPP_

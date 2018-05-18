// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// Copyright (C) 2017 The Antares Authors
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

#include "config/dirs.hpp"

#include <sfz/sfz.hpp>

#include "config/preferences.hpp"
#include "game/sys.hpp"

namespace antares {

static sfz::optional<pn::string> app_data;
static sfz::optional<pn::string> factory_scenario;
const char kFactoryScenarioIdentifier[] = "4cab7415715aeeacf1486a352267ae82c0efb220";

pn::string_view application_path() {
    if (app_data.has_value()) {
        return *app_data;
    }
    return default_application_path();
}

void set_application_path(pn::string_view path) { app_data.emplace(path.copy()); }

pn::string_view factory_scenario_path() {
    if (factory_scenario.has_value()) {
        return *factory_scenario;
    }
    return default_factory_scenario_path();
}

void set_factory_scenario_path(pn::string_view path) { factory_scenario.emplace(path.copy()); }

pn::string scenario_path() {
    pn::string_view identifier = sys.prefs->scenario_identifier();
    if (identifier == kFactoryScenarioIdentifier) {
        return factory_scenario_path().copy();
    }
    return pn::format("{0}/{1}", dirs().scenarios, identifier);
}

}  // namespace antares

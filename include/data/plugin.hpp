// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// Copyright (C) 2016-2017 The Antares Authors
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

#ifndef ANTARES_DATA_PLUGIN_HPP_
#define ANTARES_DATA_PLUGIN_HPP_

#include <vector>

#include "data/level.hpp"

namespace antares {

struct ScenarioGlobals {
    scenarioInfoType                  meta;
    std::vector<Level>                levels;
    std::vector<Level::InitialObject> initials;
    std::vector<Level::Condition>     conditions;
    std::vector<Level::BriefPoint>    briefings;
    std::vector<BaseObject>           objects;
    std::vector<Action>               actions;
    std::vector<Race>                 races;
};

extern ScenarioGlobals plug;

void PluginInit();

}  // namespace antares

#endif  // ANTARES_DATA_PLUGIN_HPP_

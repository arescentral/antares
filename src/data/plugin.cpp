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

#include "data/plugin.hpp"

#include <algorithm>
#include <pn/file>

#include "config/dirs.hpp"
#include "config/preferences.hpp"
#include "data/base-object.hpp"
#include "data/condition.hpp"
#include "data/field.hpp"
#include "data/initial.hpp"
#include "data/level.hpp"
#include "data/races.hpp"
#include "data/resource.hpp"
#include "game/sys.hpp"
#include "lang/defines.hpp"

using sfz::range;
using std::vector;

namespace antares {

static constexpr int kPluginFormat = 21;

static constexpr const char kSplashPicture[]  = "splash";
static constexpr const char kStarmapPicture[] = "starmap";

ANTARES_GLOBAL ScenarioGlobals plug;

static void read_all_levels() {
    plug.levels.clear();
    plug.chapters.clear();
    for (pn::string_view name : Resource::list_levels()) {
        auto it = plug.levels.emplace(name.copy(), Resource::level(name)).first;
        if (it->second.base.chapter.has_value()) {
            plug.chapters[*it->second.base.chapter] = &it->second;
        }
    }
}

void PluginInit() {
    plug.info = Resource::info();
    try {
        if (plug.info.format != kPluginFormat) {
            throw std::runtime_error(
                    pn::format("unknown plugin format {0}", plug.info.format).c_str());
        }
        plug.splash  = Resource::texture(kSplashPicture);
        plug.starmap = Resource::texture(kStarmapPicture);
    } catch (...) {
        std::throw_with_nested(std::runtime_error("info.pn"));
    }

    read_all_levels();
}

void load_race(const NamedHandle<const Race>& r) {
    if (plug.races.find(r.name().copy()) != plug.races.end()) {
        return;  // already loaded.
    }
    plug.races.emplace(r.name().copy(), Resource::race(r.name()));
}

void load_object(const NamedHandle<const BaseObject>& o) {
    if (plug.objects.find(o.name().copy()) != plug.objects.end()) {
        return;  // already loaded.
    }
    plug.objects.emplace(o.name().copy(), Resource::object(o.name()));
}

}  // namespace antares

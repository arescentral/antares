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

#include "data/base-object.hpp"
#include "data/resource.hpp"
#include "data/string-list.hpp"
#include "lang/defines.hpp"

using sfz::BytesSlice;
using sfz::Exception;
using sfz::StringSlice;
using sfz::format;
using sfz::range;
using std::vector;

namespace antares {

static const int16_t kLevelNameID               = 4600;
static const int16_t kSpaceObjectNameResID      = 5000;
static const int16_t kSpaceObjectShortNameResID = 5001;

static const int16_t kPackedResID = 500;

ANTARES_GLOBAL ScenarioGlobals plug;

template <typename T>
static void read_all(StringSlice name, StringSlice type, StringSlice extension, vector<T>& v) {
    Resource   rsrc(type, extension, kPackedResID);
    BytesSlice in(rsrc.data());
    size_t     count = rsrc.data().size() / T::byte_size;
    v.resize(count);
    for (size_t i = 0; i < count; ++i) {
        read(in, v[i]);
    }
    if (!in.empty()) {
        throw Exception(format("didn't consume all of {0} data", name));
    }
}

void PluginInit() {
    {
        Resource   rsrc("scenario-info", "nlAG", 128);
        BytesSlice in(rsrc.data());
        read(in, plug.meta);
        if (!in.empty()) {
            throw Exception("didn't consume all of scenario file info data");
        }
    }

    read_all("level", "scenarios", "snro", plug.levels);
    read_all("initials", "scenario-initial-objects", "snit", plug.initials);
    read_all("conditions", "scenario-conditions", "sncd", plug.conditions);
    read_all("briefings", "scenario-briefing-points", "snbf", plug.briefings);
    read_all("objects", "objects", "bsob", plug.objects);
    read_all("actions", "object-actions", "obac", plug.actions);
    read_all("races", "races", "race", plug.races);

    StringList level_names(kLevelNameID);
    for (auto& level : plug.levels) {
        level.name.assign(level_names.at(level.levelNameStrNum - 1));
    }
    for (int i : range(plug.levels.size())) {
        while (i != plug.levels[i].levelNameStrNum - 1) {
            using std::swap;
            swap(plug.levels[i], plug.levels[plug.levels[i].levelNameStrNum - 1]);
        }
    }

    StringList object_names(kSpaceObjectNameResID);
    StringList object_short_names(kSpaceObjectShortNameResID);
    for (size_t i = 0; i < plug.objects.size(); ++i) {
        plug.objects[i].name.assign(object_names.at(i));
        plug.objects[i].short_name.assign(object_short_names.at(i));
    }
}

}  // namespace antares

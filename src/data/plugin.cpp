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

#include <glob.h>
#include <algorithm>
#include <pn/file>

#include "config/dirs.hpp"
#include "config/preferences.hpp"
#include "data/base-object.hpp"
#include "data/races.hpp"
#include "data/resource.hpp"
#include "lang/defines.hpp"

using sfz::range;
using std::vector;

namespace antares {

static const int16_t kSpaceObjectNameResID      = 5000;
static const int16_t kSpaceObjectShortNameResID = 5001;

ANTARES_GLOBAL ScenarioGlobals plug;

template <typename T>
static void read_all_binary(pn::string_view name, pn::string_view dir, vector<T>& v) {
    for (int i = 0; true; ++i) {
        pn::string path = pn::format("{0}/{1}.bin", dir, i);
        if (!Resource::exists(path)) {
            break;
        }

        try {
            v.resize(i + 1);
            Resource rsrc = Resource::path(path);

            pn::file in = rsrc.data().open();
            read_from(in, &v[i]);
            in.check();
            if (fgetc(in.c_obj()) != EOF) {
                throw std::runtime_error("expected eof");
            }
        } catch (...) {
            std::throw_with_nested(std::runtime_error(path.c_str()));
        }
    }
}

namespace {

struct ScopedGlob {
    glob_t data;
    ScopedGlob() { memset(&data, sizeof(data), 0); }
    ~ScopedGlob() { globfree(&data); }
};

}  // namespace

static void read_all_levels(vector<Level>& v) {
    ScopedGlob g;
    pn::string dir;
    if (sys.prefs->scenario_identifier() == kFactoryScenarioIdentifier) {
        dir = application_path().copy();
    } else {
        dir = scenario_path();
    }
    glob(pn::format("{0}/levels/*.pn", dir).c_str(), 0, NULL, &g.data);

    v.clear();
    for (int i = 0; i < g.data.gl_pathc; ++i) {
        const pn::string_view full_path = g.data.gl_pathv[i];
        const pn::string_view path      = full_path.substr(dir.size() + 1);

        try {
            pn::value  x;
            pn_error_t e;
            if (!pn::parse(Resource::path(path).data().open(), x, &e)) {
                throw std::runtime_error(
                        pn::format("{0}:{1}: {2}", e.lineno, e.column, pn_strerror(e.code))
                                .c_str());
            }
            v.push_back(level(x));
        } catch (...) {
            std::throw_with_nested(std::runtime_error(path.copy().c_str()));
        }
    }
}

static void read_all_races(vector<Race>& v) {
    for (int i = 0; true; ++i) {
        pn::string path = pn::format("races/{0}.pn", i);
        if (!Resource::exists(path)) {
            break;
        }

        try {
            pn::value  x;
            pn_error_t e;
            if (!pn::parse(Resource::path(path).data().open(), x, &e)) {
                throw std::runtime_error(
                        pn::format("{0}:{1}: {2}", e.lineno, e.column, pn_strerror(e.code))
                                .c_str());
            }
            v.push_back(race(x));
        } catch (...) {
            std::throw_with_nested(std::runtime_error(path.copy().c_str()));
        }
    }
}

void PluginInit() {
    {
        Resource rsrc = Resource::path("info.pn");
        pn::file in   = rsrc.data().open();
        if (!read_from(in, &plug.info)) {
            throw std::runtime_error("error while reading scenario file info data");
        }
        if (fgetc(in.c_obj()) != EOF) {
            throw std::runtime_error("didn't consume all of scenario file info data");
        }
    }

    read_all_levels(plug.levels);
    read_all_binary("objects", "objects", plug.objects);
    read_all_races(plug.races);

    std::sort(plug.levels.begin(), plug.levels.end(), [](const Level& x, const Level& y) {
        return (x.chapter < y.chapter);
    });

    auto object_names       = Resource::strings(kSpaceObjectNameResID);
    auto object_short_names = Resource::strings(kSpaceObjectShortNameResID);
    for (size_t i = 0; i < plug.objects.size(); ++i) {
        plug.objects[i].name       = object_names.at(i).copy();
        plug.objects[i].short_name = object_short_names.at(i).copy();
    }
}

}  // namespace antares

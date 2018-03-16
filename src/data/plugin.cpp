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
#include "game/sys.hpp"
#include "lang/defines.hpp"

using sfz::range;
using std::vector;

namespace antares {

ANTARES_GLOBAL ScenarioGlobals plug;

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
    std::sort(plug.levels.begin(), plug.levels.end(), [](const Level& x, const Level& y) {
        return (x.chapter < y.chapter);
    });
}

void load_race(const NamedHandle<Race>& r) {
    if (plug.races.find(r.name().copy()) != plug.races.end()) {
        return;  // already loaded.
    }

    pn::string path = pn::format("races/{0}.pn", r.name());
    try {
        pn::value  x;
        pn_error_t e;
        if (!pn::parse(Resource::path(path).data().open(), x, &e)) {
            throw std::runtime_error(
                    pn::format("{0}:{1}: {2}", e.lineno, e.column, pn_strerror(e.code)).c_str());
        }
        plug.races.emplace(r.name().copy(), race(path_value{x}));
    } catch (...) {
        std::throw_with_nested(std::runtime_error(path.copy().c_str()));
    }
}

void load_object(const NamedHandle<BaseObject>& o) {
    if (plug.objects.find(o.name().copy()) != plug.objects.end()) {
        return;  // already loaded.
    }

    pn::string path = pn::format("objects/{0}.pn", o.name());
    try {
        pn::value  x;
        pn_error_t e;
        if (!pn::parse(Resource::path(path).data().open(), x, &e)) {
            throw std::runtime_error(
                    pn::format("{0}:{1}: {2}", e.lineno, e.column, pn_strerror(e.code)).c_str());
        }
        plug.objects.emplace(o.name().copy(), base_object(x));
    } catch (...) {
        std::throw_with_nested(std::runtime_error(path.c_str()));
    }
}

void load_object(Handle<BaseObject> o) {
    return load_object(NamedHandle<BaseObject>(pn::dump(o.number(), pn::dump_short)));
}

}  // namespace antares

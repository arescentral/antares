// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// Copyright (C) 2008-2017 The Antares Authors
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

#include "data/scenario-list.hpp"

#include <glob.h>
#include <string.h>
#include <pn/file>
#include <sfz/sfz.hpp>

#include "config/dirs.hpp"
#include "data/field.hpp"
#include "data/level.hpp"

using std::vector;

namespace antares {

namespace {

struct ScopedGlob {
    glob_t data;
    ScopedGlob() { memset(&data, 0, sizeof(data)); }
    ~ScopedGlob() { globfree(&data); }
};

}  // namespace

std::vector<Info> scenario_list() {
    std::vector<Info> scenarios;

    const pn::string factory_info_path = pn::format("{0}/info.pn", application_path());
    try {
        pn::value  x;
        pn_error_t e;
        if (!pn::parse(pn::open(factory_info_path, "r").check(), &x, &e)) {
            throw std::runtime_error(
                    pn::format("{0}:{1}: {2}", e.lineno, e.column, pn_strerror(e.code)).c_str());
        }
        scenarios.emplace_back(info(path_value{x}));
        scenarios.back().identifier.hash = kFactoryScenarioIdentifier;
    } catch (...) {
        // ignore
    }

    ScopedGlob g;
    pn::string str = pn::format("{0}/*/info.pn", dirs().scenarios);
    glob(str.c_str(), 0, NULL, &g.data);

    size_t prefix_len = dirs().scenarios.size() + 1;
    size_t suffix_len = 8;
    for (int i = 0; i < g.data.gl_pathc; ++i) {
        const pn::string path = g.data.gl_pathv[i];
        pn::string_view  identifier =
                path.substr(prefix_len, path.size() - prefix_len - suffix_len);
        if (identifier == kFactoryScenarioIdentifier) {
            continue;
        }

        try {
            sfz::mapped_file file(path);
            pn::file         in = file.data().open();
            pn::value        x;
            pn_error_t       e;
            if (!pn::parse(in, &x, &e)) {
                continue;
            }
            scenarios.emplace_back(info(path_value{x}));
        } catch (...) {
            // ignore
        }
    }

    return scenarios;
}

}  // namespace antares

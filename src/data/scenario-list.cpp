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
#include "data/level.hpp"

using std::vector;

namespace antares {

namespace {

struct ScopedGlob {
    glob_t data;
    ScopedGlob() { memset(&data, sizeof(data), 0); }
    ~ScopedGlob() { globfree(&data); }
};

}  // namespace

ScenarioList::ScenarioList() {
    _scenarios.emplace_back();
    Entry& factory_scenario     = _scenarios.back();
    factory_scenario.identifier = kFactoryScenarioIdentifier;

    const pn::string factory_path = pn::format("{0}/info.pn", factory_scenario_path());
    pn::value        info;
    pn_error_t       err;
    if (sfz::path::isfile(factory_path) && pn::parse(pn::open(factory_path, "r"), info, &err)) {
        pn::map_cref m                = info.as_map();
        factory_scenario.title        = m.get("title").as_string().copy();
        factory_scenario.download_url = m.get("download_url").as_string().copy();
        factory_scenario.author       = m.get("author").as_string().copy();
        factory_scenario.author_url   = m.get("author_url").as_string().copy();
        factory_scenario.version      = m.get("version").as_string().copy();
        factory_scenario.installed    = true;
    } else {
        factory_scenario.title        = "Ares";
        factory_scenario.download_url = "http://www.arescentral.com";
        factory_scenario.author       = "Bigger Planet";
        factory_scenario.author_url   = "http://www.biggerplanet.com";
        factory_scenario.version      = "1.1.1";
        factory_scenario.installed    = false;
    }

    ScopedGlob            g;
    const pn::string_view info_basename("info.pn");
    pn::string            str = pn::format("{0}/*/{1}", dirs().scenarios, info_basename);
    glob(str.c_str(), 0, NULL, &g.data);

    size_t prefix_len = dirs().scenarios.size() + 1;
    size_t suffix_len = info_basename.size() + 1;
    for (int i = 0; i < g.data.gl_pathc; ++i) {
        const pn::string path = g.data.gl_pathv[i];
        pn::string_view  identifier =
                path.substr(prefix_len, path.size() - prefix_len - suffix_len);
        if (identifier == _scenarios[0].identifier) {
            continue;
        }

        sfz::mapped_file file(path);
        pn::file         in = file.data().open();
        pn::value        info;
        if (!pn::parse(in, info, &err)) {
            continue;
        }
        _scenarios.emplace_back();
        Entry& entry       = _scenarios.back();
        entry.identifier   = identifier.copy();
        pn::map_cref m     = info.as_map();
        entry.title        = m.get("title").as_string().copy();
        entry.download_url = m.get("download_url").as_string().copy();
        entry.author       = m.get("author").as_string().copy();
        entry.author_url   = m.get("author_url").as_string().copy();
        entry.version      = m.get("version").as_string().copy();
        entry.installed    = true;
    }
}

size_t ScenarioList::size() const { return _scenarios.size(); }

const ScenarioList::Entry& ScenarioList::at(size_t index) const { return _scenarios.at(index); }

}  // namespace antares

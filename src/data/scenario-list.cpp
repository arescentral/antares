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
#include "data/pn.hpp"

using sfz::BytesSlice;
using sfz::CString;
using sfz::MappedFile;
using sfz::format;
using sfz::read;
using std::vector;

namespace utf8 = sfz::utf8;

namespace antares {

namespace {

struct ScopedGlob {
    glob_t data;
    ScopedGlob() { memset(&data, sizeof(data), 0); }
    ~ScopedGlob() { globfree(&data); }
};

}  // namespace

Version u32_to_version(uint32_t in) {
    using std::swap;
    vector<int> components;
    components.push_back((in & 0xff000000) >> 24);
    components.push_back((in & 0x00ff0000) >> 16);
    if (in & 0x0000ffff) {
        components.push_back((in & 0x0000ff00) >> 8);
    }
    if (in & 0x000000ff) {
        components.push_back(in & 0x000000ff);
    }
    return Version{components};
}

ScenarioList::ScenarioList() {
    _scenarios.emplace_back();
    Entry& factory_scenario     = _scenarios.back();
    factory_scenario.identifier = kFactoryScenarioIdentifier;

    const sfz::String factory_path(format(
            "{0}/scenario-info/128.nlAG", pn2sfz(scenario_dir(kFactoryScenarioIdentifier))));
    if (sfz::path::isfile(factory_path)) {
        MappedFile       file(factory_path);
        BytesSlice       data(file.data());
        scenarioInfoType info;
        read(data, info);
        factory_scenario.title        = info.titleString.copy();
        factory_scenario.download_url = info.downloadURLString.copy();
        factory_scenario.author       = info.authorNameString.copy();
        factory_scenario.author_url   = info.authorURLString.copy();
        factory_scenario.version      = u32_to_version(info.version);
        factory_scenario.installed    = true;
    } else {
        factory_scenario.title        = "Ares";
        factory_scenario.download_url = "http://www.arescentral.com";
        factory_scenario.author       = "Bigger Planet";
        factory_scenario.author_url   = "http://www.biggerplanet.com";
        factory_scenario.version      = u32_to_version(0x01010100);
        factory_scenario.installed    = false;
    }

    ScopedGlob            g;
    const pn::string_view info("scenario-info/128.nlAG");
    pn::string            str = pn::format("{0}/*/{1}", dirs().scenarios, info);
    glob(str.c_str(), 0, NULL, &g.data);

    size_t prefix_len = dirs().scenarios.size() + 1;
    size_t suffix_len = info.size() + 1;
    for (int i = 0; i < g.data.gl_pathc; ++i) {
        const pn::string path = sfz2pn(utf8::decode(g.data.gl_pathv[i]));
        pn::string_view  identifier =
                path.substr(prefix_len, path.size() - prefix_len - suffix_len);
        if (identifier == _scenarios[0].identifier) {
            continue;
        }

        MappedFile       file(pn2sfz(path));
        BytesSlice       data(file.data());
        scenarioInfoType info;
        read(data, info);
        _scenarios.emplace_back();
        Entry& entry       = _scenarios.back();
        entry.identifier   = identifier.copy();
        entry.title        = info.titleString.copy();
        entry.download_url = info.downloadURLString.copy();
        entry.author       = info.authorNameString.copy();
        entry.author_url   = info.authorURLString.copy();
        entry.version      = u32_to_version(info.version);
        entry.installed    = true;
    }
}

size_t ScenarioList::size() const { return _scenarios.size(); }

const ScenarioList::Entry& ScenarioList::at(size_t index) const { return _scenarios.at(index); }

pn::string stringify(const Version& v) {
    pn::string out;
    for (vector<int>::const_iterator begin = v.components.begin(), end = v.components.end();
         begin != end; ++begin) {
        if (begin != v.components.begin()) {
            out += ".";
        }
        out += pn::dump(*begin, pn::dump_short);
    }
    return out;
}

}  // namespace antares

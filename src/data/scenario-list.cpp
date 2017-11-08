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
#include <sfz/sfz.hpp>
#include "config/dirs.hpp"
#include "data/level.hpp"

using sfz::BytesSlice;
using sfz::CString;
using sfz::MappedFile;
using sfz::String;
using sfz::StringSlice;
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
    Entry& factory_scenario = _scenarios.back();
    factory_scenario.identifier.assign(kFactoryScenarioIdentifier);

    const String factory_path(
            format("{0}/scenario-info/128.nlAG", scenario_dir(kFactoryScenarioIdentifier)));
    if (sfz::path::isfile(factory_path)) {
        MappedFile       file(factory_path);
        BytesSlice       data(file.data());
        scenarioInfoType info;
        read(data, info);
        factory_scenario.title.assign(info.titleString);
        factory_scenario.download_url.assign(info.downloadURLString);
        factory_scenario.author.assign(info.authorNameString);
        factory_scenario.author_url.assign(info.authorURLString);
        factory_scenario.version   = u32_to_version(info.version);
        factory_scenario.installed = true;
    } else {
        factory_scenario.title.assign("Ares");
        factory_scenario.download_url.assign("http://www.arescentral.com");
        factory_scenario.author.assign("Bigger Planet");
        factory_scenario.author_url.assign("http://www.biggerplanet.com");
        factory_scenario.version   = u32_to_version(0x01010100);
        factory_scenario.installed = false;
    }

    ScopedGlob        g;
    const StringSlice info("scenario-info/128.nlAG");
    String            str(format("{0}/*/{1}", dirs().scenarios, info));
    CString           c_str(str);
    glob(c_str.data(), 0, NULL, &g.data);

    size_t prefix_len = dirs().scenarios.size() + 1;
    size_t suffix_len = info.size() + 1;
    for (int i = 0; i < g.data.gl_pathc; ++i) {
        const String path(utf8::decode(g.data.gl_pathv[i]));
        StringSlice  identifier = path.slice(prefix_len, path.size() - prefix_len - suffix_len);
        if (identifier == _scenarios[0].identifier) {
            continue;
        }

        MappedFile       file(path);
        BytesSlice       data(file.data());
        scenarioInfoType info;
        read(data, info);
        _scenarios.emplace_back();
        Entry& entry = _scenarios.back();
        entry.identifier.assign(identifier);
        entry.title.assign(info.titleString);
        entry.download_url.assign(info.downloadURLString);
        entry.author.assign(info.authorNameString);
        entry.author_url.assign(info.authorURLString);
        entry.version   = u32_to_version(info.version);
        entry.installed = true;
    }
}

size_t ScenarioList::size() const {
    return _scenarios.size();
}

const ScenarioList::Entry& ScenarioList::at(size_t index) const {
    return _scenarios.at(index);
}

void print_to(sfz::PrintTarget out, const Version& v) {
    for (vector<int>::const_iterator begin = v.components.begin(), end = v.components.end();
         begin != end; ++begin) {
        if (begin != v.components.begin()) {
            print(out, ".");
        }
        print(out, *begin);
    }
}

}  // namespace antares

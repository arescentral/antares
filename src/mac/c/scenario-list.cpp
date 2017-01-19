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

#include "mac/c/scenario-list.h"

#include <sfz/sfz.hpp>

#include "data/scenario-list.hpp"

using sfz::CString;
using sfz::String;
using std::vector;

namespace utf8 = sfz::utf8;

struct AntaresScenarioListEntry {
    AntaresScenarioListEntry(const antares::ScenarioList::Entry& entry)
            : identifier(entry.identifier),
              title(entry.title),
              download_url(entry.download_url),
              author(entry.author),
              author_url(entry.author_url),
              version_string(entry.version),
              version(version_string) {}

    // TODO(sfiera): give CString a move constructor so we don't need to define this.
    AntaresScenarioListEntry(AntaresScenarioListEntry&& other)
            : identifier(String(utf8::decode(other.identifier.data()))),
              title(String(utf8::decode(other.title.data()))),
              download_url(String(utf8::decode(other.download_url.data()))),
              author(String(utf8::decode(other.author.data()))),
              author_url(String(utf8::decode(other.author_url.data()))),
              version_string(other.version_string),
              version(String(utf8::decode(other.version.data()))) {}

    CString identifier;
    CString title;
    CString download_url;
    CString author;
    CString author_url;
    String  version_string;
    CString version;
};

struct AntaresScenarioList {
    antares::ScenarioList            cxx_obj;
    vector<AntaresScenarioListEntry> entries;

    AntaresScenarioList() {
        for (size_t i = 0; i < cxx_obj.size(); ++i) {
            entries.emplace_back(cxx_obj.at(i));
        }
    }
};

namespace antares {

extern "C" AntaresScenarioList* antares_scenario_list_create() {
    return new AntaresScenarioList;
}

extern "C" void antares_scenario_list_destroy(AntaresScenarioList* list) {
    delete list;
}

extern "C" size_t antares_scenario_list_size(AntaresScenarioList* list) {
    return list->entries.size();
}

extern "C" AntaresScenarioListEntry* antares_scenario_list_at(
        AntaresScenarioList* list, size_t index) {
    if (index >= list->entries.size()) {
        return NULL;
    }
    return &list->entries[index];
}

extern "C" const char* antares_scenario_list_entry_identifier(AntaresScenarioListEntry* entry) {
    return entry->identifier.data();
}

extern "C" const char* antares_scenario_list_entry_title(AntaresScenarioListEntry* entry) {
    return entry->title.data();
}

extern "C" const char* antares_scenario_list_entry_download_url(AntaresScenarioListEntry* entry) {
    return entry->download_url.data();
}

extern "C" const char* antares_scenario_list_entry_author(AntaresScenarioListEntry* entry) {
    return entry->author.data();
}

extern "C" const char* antares_scenario_list_entry_author_url(AntaresScenarioListEntry* entry) {
    return entry->author_url.data();
}

extern "C" const char* antares_scenario_list_entry_version(AntaresScenarioListEntry* entry) {
    return entry->version.data();
}

}  // namespace antares

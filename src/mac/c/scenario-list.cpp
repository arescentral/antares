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

#include "data/scenario-list.hpp"

using std::vector;

static sfz::optional<pn::string> copy(const sfz::optional<pn::string>& s) {
    if (s.has_value()) {
        return sfz::make_optional(s->copy());
    }
    return sfz::nullopt;
}

struct AntaresScenarioListEntry {
    AntaresScenarioListEntry(const antares::Info& entry)
            : identifier(entry.identifier.hash.copy()),
              title(entry.title.copy()),
              download_url(copy(entry.download_url)),
              author(entry.author.copy()),
              author_url(copy(entry.author_url)),
              version(entry.version.copy()) {}

    pn::string                identifier;
    pn::string                title;
    sfz::optional<pn::string> download_url;
    pn::string                author;
    sfz::optional<pn::string> author_url;
    pn::string                version;
};

struct AntaresScenarioList {
    vector<AntaresScenarioListEntry> entries;

    AntaresScenarioList() {
        for (const antares::Info& s : antares::scenario_list()) {
            entries.emplace_back(s);
        }
    }
};

namespace antares {

extern "C" AntaresScenarioList* antares_scenario_list_create() { return new AntaresScenarioList; }

extern "C" void antares_scenario_list_destroy(AntaresScenarioList* list) { delete list; }

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
    if (entry->download_url.has_value()) {
        return entry->download_url->data();
    }
    return nullptr;
}

extern "C" const char* antares_scenario_list_entry_author(AntaresScenarioListEntry* entry) {
    return entry->author.data();
}

extern "C" const char* antares_scenario_list_entry_author_url(AntaresScenarioListEntry* entry) {
    if (entry->author_url.has_value()) {
        return entry->author_url->data();
    }
    return nullptr;
}

extern "C" const char* antares_scenario_list_entry_version(AntaresScenarioListEntry* entry) {
    return entry->version.data();
}

}  // namespace antares

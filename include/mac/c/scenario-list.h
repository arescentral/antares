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

#ifndef ANTARES_MAC_C_SCENARIO_LIST_H_
#define ANTARES_MAC_C_SCENARIO_LIST_H_

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct AntaresScenarioList      AntaresScenarioList;
typedef struct AntaresScenarioListEntry AntaresScenarioListEntry;

AntaresScenarioList* antares_scenario_list_create();
void antares_scenario_list_destroy(AntaresScenarioList* list);
size_t antares_scenario_list_size(AntaresScenarioList* list);
AntaresScenarioListEntry* antares_scenario_list_at(AntaresScenarioList* list, size_t index);

const char* antares_scenario_list_entry_identifier(AntaresScenarioListEntry* entry);
const char* antares_scenario_list_entry_title(AntaresScenarioListEntry* entry);
const char* antares_scenario_list_entry_download_url(AntaresScenarioListEntry* entry);
const char* antares_scenario_list_entry_author(AntaresScenarioListEntry* entry);
const char* antares_scenario_list_entry_author_url(AntaresScenarioListEntry* entry);
const char* antares_scenario_list_entry_version(AntaresScenarioListEntry* entry);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // ANTARES_MAC_C_SCENARIO_LIST_H_

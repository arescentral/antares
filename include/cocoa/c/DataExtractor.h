// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// Copyright (C) 2008-2011 Ares Central
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
// License along with this program.  If not, see
// <http://www.gnu.org/licenses/>.

#ifndef ANTARES_COCOA_C_DATA_EXTRACTOR_H_
#define ANTARES_COCOA_C_DATA_EXTRACTOR_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct AntaresDataExtractor AntaresDataExtractor;

AntaresDataExtractor* antares_data_extractor_create(
        const char* downloads_dir, const char* output_dir, const char* scenario);
void antares_data_extractor_destroy(AntaresDataExtractor* extractor);
int antares_data_extractor_set_scenario(AntaresDataExtractor* extractor, const char* scenario);
int antares_data_extractor_set_plugin_file(AntaresDataExtractor* extractor, const char* path);
int antares_data_extractor_current(AntaresDataExtractor* extractor);
void antares_data_extractor_extract(
        AntaresDataExtractor* extractor, void (*callback)(const char*, void*), void* userdata);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // ANTARES_COCOA_C_DATA_EXTRACTOR_H_

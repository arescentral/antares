// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// Copyright (C) 2013-2017 The Antares Authors
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

#include "config/dirs.hpp"

#include <pn/output>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <shlobj.h>
#include <shlwapi.h>

namespace antares {

static pn::string make_default_application_path() {
    char path_buffer[MAX_PATH + 1];
    GetModuleFileNameA(nullptr, path_buffer, MAX_PATH);
    path_buffer[MAX_PATH] = '\0';

    // FIXME: Implement Unicode paths and use PathCchRemoveFileSpec instead
    PathRemoveFileSpecA(path_buffer);

    return pn::format("{0}\\data", path_buffer);
}

pn::string_view default_application_path() {
    static pn::string s = make_default_application_path();
    return s;
}

pn::string_view default_factory_scenario_path() {
    static pn::string s = pn::format("{0}/{1}", dirs().scenarios, kFactoryScenarioIdentifier);
    return s;
}

Directories win_dirs() {
    Directories directories;

#ifdef _MSC_VER
    // mingw doesn't have SHGetKnownFolderPath
    PWSTR path_buffer_wchar = nullptr;
    SHGetKnownFolderPath(FOLDERID_Documents, 0 /*KF_FLAG_DEFAULT*/, nullptr, &path_buffer_wchar);

    // FIXME: Need proper Unicode path support!  Convert this to UTF-8 instead.
    size_t path_length = wcslen(path_buffer_wchar);
    char* path_buffer = new char[path_length + 1];
    for (size_t i = 0; i < path_length; i++) {
        path_buffer[i] = static_cast<char>(path_buffer_wchar[i]);
    }
    path_buffer[path_length] = 0;

    CoTaskMemFree(path_buffer_wchar);
#else
    char path_buffer[MAX_PATH + 1];
    SHGetFolderPathA(nullptr, CSIDL_PERSONAL, nullptr, 0, path_buffer);
    path_buffer[MAX_PATH] = 0;
#endif

    directories.root = path_buffer;

#ifdef _MSC_VER
    delete[] path_buffer;
#endif

    directories.root += "/Antares";

    directories.downloads = pn::format("{0}/Downloads", directories.root);
    directories.registry  = pn::format("{0}/Registry", directories.root);
    directories.replays   = pn::format("{0}/Replays", directories.root);
    directories.scenarios = pn::format("{0}/Scenarios", directories.root);
    return directories;
};

const Directories& dirs() {
    static const Directories dirs = win_dirs();
    return dirs;
}

}  // namespace antares

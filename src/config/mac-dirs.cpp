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

#include <unistd.h>
#include <pn/file>

#include "data/pn.hpp"
#include "mac/core-foundation.hpp"

namespace antares {

pn::string default_application_path() {
    cf::Url    url(CFBundleCopyResourcesDirectoryURL(CFBundleGetMainBundle()));
    cf::String url_string(CFStringCreateCopy(NULL, CFURLGetString(url.c_obj())));
    char       path_buffer[PATH_MAX];
    if (!CFURLGetFileSystemRepresentation(
                url.c_obj(), true, reinterpret_cast<UInt8*>(path_buffer), PATH_MAX)) {
        throw std::runtime_error("couldn't get application_path()");
    }
    return pn::string(path_buffer, strlen(path_buffer));
}

Directories mac_dirs() {
    Directories directories;

    char* home = getenv("HOME");
    if (home && *home) {
        directories.root = home;
    } else {
        char tmp[PATH_MAX] = "/tmp/antares-XXXXXX";
        directories.root   = mkdtemp(tmp);
    }
    directories.root += "/Library/Application Support/Antares";

    directories.downloads = directories.root.copy();
    directories.downloads += "/Downloads";
    directories.registry = directories.root.copy();
    directories.registry += "/Registry";
    directories.replays = directories.root.copy();
    directories.replays += "/Replays";
    directories.scenarios = directories.root.copy();
    directories.scenarios += "/Scenarios";
    return directories;
};

const Directories& dirs() {
    static const Directories dirs = mac_dirs();
    return dirs;
}

pn::string scenario_dir(pn::string_view identifier) {
    return pn::format("{0}/{1}", dirs().scenarios, identifier);
}

}  // namespace antares

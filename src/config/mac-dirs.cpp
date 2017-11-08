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
#include <sfz/sfz.hpp>

#include "mac/core-foundation.hpp"

using sfz::Exception;
using sfz::String;
using sfz::format;

namespace utf8 = sfz::utf8;

namespace antares {

String default_application_path() {
    cf::Url    url(CFBundleCopyResourcesDirectoryURL(CFBundleGetMainBundle()));
    cf::String url_string(CFStringCreateCopy(NULL, CFURLGetString(url.c_obj())));
    char       path_buffer[PATH_MAX];
    if (!CFURLGetFileSystemRepresentation(
                url.c_obj(), true, reinterpret_cast<UInt8*>(path_buffer), PATH_MAX)) {
        throw Exception("couldn't get application_path()");
    }
    return String(utf8::decode(path_buffer));
}

Directories mac_dirs() {
    Directories directories;

    char* home = getenv("HOME");
    if (home && *home) {
        directories.root.assign(utf8::decode(home));
    } else {
        char tmp[PATH_MAX] = "/tmp/antares-XXXXXX";
        directories.root.assign(utf8::decode(mkdtemp(tmp)));
    }
    directories.root.append("/Library/Application Support/Antares");

    directories.downloads.assign(format("{0}/Downloads", directories.root));
    directories.registry.assign(format("{0}/Registry", directories.root));
    directories.replays.assign(format("{0}/Replays", directories.root));
    directories.scenarios.assign(format("{0}/Scenarios", directories.root));
    return directories;
};

const Directories& dirs() {
    static const Directories dirs = mac_dirs();
    return dirs;
}

sfz::String scenario_dir(sfz::StringSlice identifier) {
    return sfz::String(sfz::format("{0}/{1}", dirs().scenarios, identifier));
}

}  // namespace antares

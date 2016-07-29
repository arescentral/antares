// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// Copyright (C) 2015 The Antares Authors
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

#include <sys/param.h>
#include <unistd.h>
#include <sfz/sfz.hpp>
#include <sys/param.h>

using sfz::String;
using sfz::format;

namespace utf8 = sfz::utf8;

namespace antares {

Directories linux_dirs() {
    Directories directories;

    char* home = getenv("HOME");
    if (home && *home) {
        directories.root.assign(utf8::decode(home));
    } else {
        char tmp[PATH_MAX] = "/tmp/antares-XXXXXX";
        directories.root.assign(utf8::decode(mkdtemp(tmp)));
    }
    directories.root.append("/.local/share/antares");

    directories.downloads.assign(format("{0}/downloads", directories.root));
    directories.registry.assign(format("{0}/registry", directories.root));
    directories.replays.assign(format("{0}/replays", directories.root));
    directories.scenarios.assign(format("{0}/scenarios", directories.root));
    return directories;
};

const Directories& dirs() {
    static const Directories dirs = linux_dirs();
    return dirs;
}

}  // namespace antares

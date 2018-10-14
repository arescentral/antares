// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// Copyright (C) 2015-2017 The Antares Authors
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

#include <stdlib.h>
#include <sys/param.h>
#include <unistd.h>
#include <pn/file>
#include "build/defs.hpp"

namespace antares {

pn::string_view default_application_path() {
    static pn::string s = pn::format("{0}/share/games/antares/app", kAntaresPrefix);
    return s;
}

pn::string_view default_factory_scenario_path() {
    static pn::string s = pn::format(
            "{0}/share/games/antares/scenarios/{1}", kAntaresPrefix, kFactoryScenarioIdentifier);
    return s;
}

Directories linux_dirs() {
    Directories directories;

    char* home = getenv("HOME");
    if (home && *home) {
        directories.root = home;
    } else {
        char tmp[PATH_MAX] = "/tmp/antares-XXXXXX";
        directories.root   = mkdtemp(tmp);
    }
    directories.root += "/.local/share/games/antares";

    directories.downloads = directories.root.copy();
    directories.downloads += "/downloads";
    directories.registry = directories.root.copy();
    directories.registry += "/registry";
    directories.replays = directories.root.copy();
    directories.replays += "/replays";
    directories.scenarios = directories.root.copy();
    directories.scenarios += "/scenarios";
    return directories;
};

const Directories& dirs() {
    static const Directories dirs = linux_dirs();
    return dirs;
}

}  // namespace antares

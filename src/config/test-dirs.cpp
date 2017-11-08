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

#include <sys/param.h>
#include <unistd.h>
#include <sfz/sfz.hpp>

namespace antares {

#define STRINGIFY_(x) #x
#define STRINGIFY(x) STRINGIFY_(x)
#define ANTARES_DATA_STRING STRINGIFY(ANTARES_DATA)

sfz::String default_application_path() {
    return sfz::String(ANTARES_DATA_STRING);
}

Directories test_dirs() {
    Directories directories;
    directories.root.assign(application_path());
    directories.downloads.assign(format("{0}/downloads", directories.root));
    directories.registry.assign(format("{0}/registry", directories.root));
    directories.replays.assign(format("{0}/replays", directories.root));
    directories.scenarios.assign(format("{0}/scenarios", directories.root));
    return directories;
};

const Directories& dirs() {
    static const Directories dirs = test_dirs();
    return dirs;
}

sfz::String scenario_dir(sfz::StringSlice identifier) {
    return sfz::String(sfz::format("{0}/{1}", dirs().scenarios, identifier));
}

}  // namespace antares

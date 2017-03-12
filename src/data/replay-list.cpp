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

#include "data/replay-list.hpp"

#include <glob.h>
#include <pn/file>
#include <sfz/sfz.hpp>

#include "config/dirs.hpp"
#include "config/preferences.hpp"
#include "data/pn.hpp"
#include "game/sys.hpp"

using sfz::MappedFile;
using std::vector;

namespace path = sfz::path;

namespace antares {

namespace {

struct ScopedGlob {
    glob_t data;
    ScopedGlob() { memset(&data, sizeof(data), 0); }
    ~ScopedGlob() { globfree(&data); }
};

}  // namespace

ReplayList::ReplayList() {
    ScopedGlob            g;
    const pn::string_view scenario = sys.prefs->scenario_identifier();
    pn::string            str = pn::format("{0}/{1}/replays/*.NLRP", dirs().scenarios, scenario);
    glob(str.c_str(), 0, NULL, &g.data);

    for (int i = 0; i < g.data.gl_pathc; ++i) {
        const pn::string path      = g.data.gl_pathv[i];
        pn::string       basename  = sfz2pn(path::basename(pn2sfz(path)));
        pn::string_view  id_string = basename.substr(0, basename.size() - 5);
        int64_t          id;
        if (pn::strtoll(id_string, &id, nullptr)) {
            _replays.push_back(id);
        }
    }
}

size_t ReplayList::size() const { return _replays.size(); }

int16_t ReplayList::at(size_t index) const { return _replays.at(index); }

}  // namespace antares

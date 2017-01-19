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
#include <sfz/sfz.hpp>
#include "config/dirs.hpp"
#include "config/preferences.hpp"
#include "game/sys.hpp"

using sfz::CString;
using sfz::MappedFile;
using sfz::String;
using sfz::StringSlice;
using sfz::format;
using sfz::read;
using std::vector;

namespace utf8 = sfz::utf8;
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
    ScopedGlob        g;
    const StringSlice scenario = sys.prefs->scenario_identifier();
    String            str(format("{0}/replays/*.NLRP", scenario_dir(scenario)));
    CString           c_str(str);
    glob(c_str.data(), 0, NULL, &g.data);

    for (int i = 0; i < g.data.gl_pathc; ++i) {
        const String path(utf8::decode(g.data.gl_pathv[i]));
        StringSlice  basename  = path::basename(path);
        StringSlice  id_string = basename.slice(0, basename.size() - 5);
        int16_t      id;
        if (string_to_int(id_string, id)) {
            _replays.push_back(id);
        }
    }
}

size_t ReplayList::size() const {
    return _replays.size();
}

int16_t ReplayList::at(size_t index) const {
    return _replays.at(index);
}

}  // namespace antares

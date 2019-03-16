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

#include "config/ledger.hpp"

#include <fcntl.h>
#include <unistd.h>
#include <cmath>
#include <pn/array>
#include <pn/file>
#include <pn/map>
#include <pn/value>
#include <sfz/sfz.hpp>

#include "config/dirs.hpp"
#include "config/preferences.hpp"
#include "game/sys.hpp"
#include "lang/defines.hpp"

using sfz::StringMap;
using sfz::makedirs;
using sfz::range;
using std::unique_ptr;
using std::vector;

namespace path = sfz::path;

namespace antares {

static ANTARES_GLOBAL Ledger* ledger;

Ledger::Ledger() {
    if (antares::ledger) {
        throw std::runtime_error("Ledger is a singleton");
    }
    antares::ledger = this;
}

Ledger::~Ledger() { antares::ledger = NULL; }

Ledger* Ledger::ledger() { return ::antares::ledger; }

NullLedger::NullLedger() : _chapters{1} {}

void NullLedger::unlock_chapter(int chapter) { _chapters.insert(chapter); }

void NullLedger::unlocked_chapters(std::vector<int>* chapters) {
    *chapters = std::vector<int>(_chapters.begin(), _chapters.end());
}

DirectoryLedger::DirectoryLedger() { load(); }

void DirectoryLedger::unlock_chapter(int chapter) {
    _chapters.insert(chapter);
    save();
}

void DirectoryLedger::unlocked_chapters(std::vector<int>* chapters) {
    *chapters = std::vector<int>(_chapters.begin(), _chapters.end());
}

void DirectoryLedger::load() {
    const pn::string_view scenario_id = sys.prefs->scenario_identifier();
    pn::string            path        = pn::format("{0}/{1}.pn", dirs().registry, scenario_id);

    _chapters.clear();
    pn::file file = pn::open(path, "r");
    if (!file) {
        _chapters.insert(1);
        return;
    }

    pn::value x;
    if (!pn::parse(file, &x, nullptr)) {
        throw std::runtime_error("bad ledger");
    }

    pn::map_cref   data     = x.as_map();
    pn::map_cref   unlocked = data.get("unlocked").as_map();
    pn::array_cref chapters = unlocked.get("chapters").as_array();
    for (pn::value_cref chapter : chapters) {
        if (chapter.is_int()) {
            _chapters.insert(chapter.as_int());
        }
    }
}

void DirectoryLedger::save() {
    const pn::string_view scenario_id = sys.prefs->scenario_identifier();
    const pn::string      path        = pn::format("{0}/{1}.pn", dirs().registry, scenario_id);

    pn::array unlocked_chapters;
    for (std::set<int>::const_iterator it = _chapters.begin(); it != _chapters.end(); ++it) {
        unlocked_chapters.push_back(*it);
    }

    makedirs(path::dirname(path), 0755);
    pn::file file = pn::open(path, "w");
    file.dump(pn::map{
            {"unlocked",
             pn::map{
                     {"chapters", std::move(unlocked_chapters)},
             }},
    });
}

}  // namespace antares

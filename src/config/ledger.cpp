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

#include <cmath>
#include <pn/array>
#include <pn/input>
#include <pn/map>
#include <pn/output>
#include <pn/value>
#include <sfz/sfz.hpp>

#include "config/dirs.hpp"
#include "config/preferences.hpp"
#include "data/plugin.hpp"
#include "game/sys.hpp"
#include "lang/defines.hpp"

using sfz::makedirs;
using sfz::range;
using std::unique_ptr;
using std::vector;

namespace path = sfz::path;

namespace antares {

Ledger::Ledger() {
    if (sys.ledger) {
        throw std::runtime_error("Ledger is a singleton");
    }
    sys.ledger = this;
}

Ledger::~Ledger() { sys.ledger = nullptr; }

NullLedger::NullLedger() : _chapters{1} {}

void NullLedger::unlock_chapter(int chapter) { _chapters.insert(chapter); }

void NullLedger::unlocked_chapters(std::vector<int>* chapters) {
    *chapters = std::vector<int>(_chapters.begin(), _chapters.end());
}

DirectoryLedger::DirectoryLedger() { load(); }

void DirectoryLedger::unlock_chapter(int chapter) {
    std::set<int> chapters = load();
    chapters.insert(chapter);
    save(chapters);
}

void DirectoryLedger::unlocked_chapters(std::vector<int>* chapters) {
    std::set<int> set = load();
    *chapters         = std::vector<int>(set.begin(), set.end());
}

std::set<int> DirectoryLedger::load() {
    pn::string path = pn::format("{0}/{1}.pn", dirs().registry, plug.info.identifier.hash);

    std::set<int> chapters;
    pn::input     in{path, pn::text};
    if (!in) {
        chapters.insert(1);
        return chapters;
    }

    pn::value x;
    if (!pn::parse(in, &x, nullptr)) {
        throw std::runtime_error("bad ledger");
    }

    pn::map_cref data     = x.as_map();
    pn::map_cref unlocked = data.get("unlocked").as_map();
    for (pn::value_cref chapter : unlocked.get("chapters").as_array()) {
        if (chapter.is_int()) {
            chapters.insert(chapter.as_int());
        }
    }

    return chapters;
}

void DirectoryLedger::save(const std::set<int> chapters) {
    const pn::string path = pn::format("{0}/{1}.pn", dirs().registry, plug.info.identifier.hash);

    pn::array unlocked_chapters;
    for (std::set<int>::const_iterator it = chapters.begin(); it != chapters.end(); ++it) {
        unlocked_chapters.push_back(*it);
    }

    makedirs(path::dirname(path), 0755);
    pn::output out{path, pn::text};
    out.dump(pn::map{
            {"unlocked",
             pn::map{
                     {"chapters", std::move(unlocked_chapters)},
             }},
    });
}

}  // namespace antares

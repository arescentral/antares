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
#include "data/pn.hpp"
#include "game/sys.hpp"
#include "lang/defines.hpp"

using sfz::Bytes;
using sfz::Exception;
using sfz::MappedFile;
using sfz::Rune;
using sfz::ScopedFd;
using sfz::String;
using sfz::StringMap;
using sfz::StringSlice;
using sfz::format;
using sfz::makedirs;
using sfz::print;
using sfz::range;
using sfz::string_to_int;
using sfz::write;
using std::unique_ptr;
using std::vector;

namespace path = sfz::path;
namespace utf8 = sfz::utf8;

namespace antares {

static ANTARES_GLOBAL Ledger* ledger;

Ledger::Ledger() {
    if (antares::ledger) {
        throw Exception("Ledger is a singleton");
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
    String path(format("{0}/{1}/ledger.pn", pn2sfz(dirs().registry), pn2sfz(scenario_id)));

    _chapters.clear();
    unique_ptr<MappedFile> file;
    try {
        file.reset(new MappedFile(path));
    } catch (Exception& e) {
        _chapters.insert(1);
        return;
    }

    pn::string data = sfz2pn(utf8::decode(file->data()));
    pn::value  x;
    if (!pn::parse(data.open(), x, nullptr)) {
        throw Exception("bad ledger");
    }

    for (pn::value_cref level : x.as_map().get("unlocked-levels").as_array()) {
        if (level.is_int()) {
            _chapters.insert(level.as_int());
        }
    }
}

void DirectoryLedger::save() {
    const pn::string_view scenario_id = sys.prefs->scenario_identifier();
    const String path(format("{0}/{1}/ledger.pn", pn2sfz(dirs().registry), pn2sfz(scenario_id)));

    pn::array unlocked_levels;
    for (std::set<int>::const_iterator it = _chapters.begin(); it != _chapters.end(); ++it) {
        unlocked_levels.push_back(*it);
    }
    String contents(pn2sfz(pn::dump(pn::map{{"unlocked-levels", std::move(unlocked_levels)}})));

    makedirs(path::dirname(path), 0755);
    ScopedFd fd(open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644));
    Bytes    bytes(utf8::encode(contents));
    write(fd, bytes);
}

}  // namespace antares

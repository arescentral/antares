// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// Copyright (C) 2008-2011 Ares Central
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
// License along with this program.  If not, see
// <http://www.gnu.org/licenses/>.

#include "data/resource.hpp"

#include <stdio.h>
#include <sfz/sfz.hpp>
#include "config/preferences.hpp"

using sfz::BytesSlice;
using sfz::Exception;
using sfz::MappedFile;
using sfz::String;
using sfz::StringSlice;
using sfz::format;

namespace path = sfz::path;
namespace utf8 = sfz::utf8;

namespace antares {

namespace {

const char kAres[] = "com.biggerplanet.ares";

}  // namespace

Resource::Resource(const StringSlice& type, const StringSlice& extension, int id) {
    const String resource_path(format("{0}/{1}.{2}", type, id, extension));
    init(resource_path);
}

Resource::Resource(const sfz::PrintItem& resource_path) {
    const String resource_path_string(resource_path);
    init(resource_path_string);
}

Resource::~Resource() { }

BytesSlice Resource::data() const {
    return _file->data();
}

void Resource::init(const sfz::StringSlice& resource_path) {
    const StringSlice scenario_id = Preferences::preferences()->scenario_identifier();
    const String home(utf8::decode(getenv("HOME")));
    const String base(format("{0}/Library/Application Support/Antares/Scenarios", home));
    String p;

    if (scenario_id != kAres) {
        p.assign(format("{0}/{1}/{2}", base, scenario_id, resource_path));
        if (path::isfile(p)) {
            _file.reset(new MappedFile(p));
            return;
        }
    }

    p.assign(format("{0}/{1}/{2}", base, kAres, resource_path));
    if (path::isfile(p)) {
        _file.reset(new MappedFile(p));
        return;
    }

    throw Exception(format("couldn't find resource {0}", quote(resource_path)));
}

}  // namespace antares

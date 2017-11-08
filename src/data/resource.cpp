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

#include "data/resource.hpp"

#include <stdio.h>
#include <sfz/sfz.hpp>
#include "config/dirs.hpp"
#include "config/preferences.hpp"
#include "game/sys.hpp"

using sfz::BytesSlice;
using sfz::Exception;
using sfz::MappedFile;
using sfz::PrintItem;
using sfz::String;
using sfz::StringSlice;
using sfz::format;
using std::unique_ptr;

namespace path = sfz::path;
namespace utf8 = sfz::utf8;

namespace antares {

static unique_ptr<MappedFile> load_first(
        sfz::StringSlice resource_path, const std::initializer_list<PrintItem>& dirs) {
    for (const auto& dir : dirs) {
        String path(sfz::format("{0}/{1}", dir, resource_path));
        if (path::isfile(path)) {
            return unique_ptr<MappedFile>(new MappedFile(path));
        }
    }
    throw Exception(format("couldn't find resource {0}", quote(resource_path)));
}

static unique_ptr<MappedFile> load(sfz::StringSlice resource_path) {
    return load_first(
            resource_path, {
                                   scenario_dir(sys.prefs->scenario_identifier()),
                                   scenario_dir(kFactoryScenarioIdentifier), application_path(),
                           });
}

Resource::Resource(const StringSlice& type, const StringSlice& extension, int id)
        : Resource(format("{0}/{1}.{2}", type, id, extension)) {}

Resource::Resource(const sfz::PrintItem& resource_path) : _file(load(String(resource_path))) {}

Resource::~Resource() {}

BytesSlice Resource::data() const {
    return _file->data();
}

}  // namespace antares

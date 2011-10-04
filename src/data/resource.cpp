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

using sfz::BytesSlice;
using sfz::MappedFile;
using sfz::String;
using sfz::StringSlice;
using sfz::format;

namespace utf8 = sfz::utf8;

namespace antares {

Resource::Resource(const StringSlice& type, const StringSlice& extension, int id) {
    String path(format(
                "{0}/Library/Application Support/Antares/Scenarios/com.biggerplanet.ares"
                "/{1}/{2}.{3}", utf8::decode(getenv("HOME")), type, id, extension));
    _file.reset(new MappedFile(path));
}

Resource::Resource(const sfz::PrintItem& resource_path) {
    String path(format(
                "{0}/Library/Application Support/Antares/Scenarios/com.biggerplanet.ares{1}",
                utf8::decode(getenv("HOME")), resource_path));
    _file.reset(new MappedFile(path));
}

Resource::~Resource() { }

BytesSlice Resource::data() const {
    return _file->data();
}

}  // namespace antares

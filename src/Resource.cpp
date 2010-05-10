// Ares, a tactical space combat game.
// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// 
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

#include "Resource.hpp"

#include <stdio.h>
#include "sfz/Format.hpp"
#include "sfz/MappedFile.hpp"

using sfz::BytesPiece;
using sfz::MappedFile;
using sfz::String;
using sfz::StringPiece;
using sfz::utf8_encoding;

namespace antares {

Resource::Resource(const StringPiece& type, const StringPiece& extension, int id) {
    String path(getenv("HOME"), utf8_encoding());
    path.append("/Library/Application Support/Antares/Scenarios/com.biggerplanet.ares/");
    format(&path, "{0}/{1}.{2}", type, id, extension);
    _file.reset(new MappedFile(path));
}

Resource::~Resource() { }

BytesPiece Resource::data() const {
    return _file->data();
}

}  // namespace antares

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

#include <glob.h>
#include <stdio.h>
#include "sfz/Exception.hpp"
#include "sfz/MappedFile.hpp"

using sfz::BytesPiece;
using sfz::Exception;
using sfz::MappedFile;
using sfz::String;
using sfz::utf8_encoding;

namespace antares {

namespace {

struct FreedGlob : public glob_t {
    ~FreedGlob() {
        globfree(this);
    }
};

void glob_for_resource(uint32_t code, int id, String* out) {
    char fileglob[64];
    char code_chars[5] = {
        code >> 24, code >> 16, code >> 8, code, '\0',
    };
    FreedGlob g;
    g.gl_offs = 0;

    sprintf(fileglob, "data/original/rsrc/%s/%d.%s", code_chars, id, code_chars);
    glob(fileglob, GLOB_DOOFFS, NULL, &g);

    sprintf(fileglob, "data/original/rsrc/%s/%d *.%s", code_chars, id, code_chars);
    glob(fileglob, GLOB_DOOFFS | GLOB_APPEND, NULL, &g);

    if (g.gl_pathc != 1) {
        perror(fileglob);
        throw Exception("{0} {1} not found", code_chars + 0, id);
    }

    String path(g.gl_pathv[0], utf8_encoding());
    out->append(path);
}

}  // namespace

Resource::Resource(uint32_t code, int id) {
    String path;
    glob_for_resource(code, id, &path);
    _file.reset(new MappedFile(path));
}

Resource::~Resource() { }

BytesPiece Resource::data() const {
    return _file->data();
}

}  // namespace antares

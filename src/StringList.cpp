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

#include "StringList.hpp"

#include "rezin/MacRoman.hpp"
#include "sfz/BinaryReader.hpp"
#include "sfz/Exception.hpp"
#include "sfz/Foreach.hpp"
#include "sfz/Formatter.hpp"
#include "Error.hpp"
#include "Resource.hpp"

using rezin::mac_roman_encoding;
using sfz::Bytes;
using sfz::BytesPiece;
using sfz::BytesBinaryReader;
using sfz::Exception;
using sfz::String;
using sfz::StringPiece;
using sfz::quote;

namespace antares {

StringList::~StringList() {
    clear();
}

void StringList::clear() {
    // TODO(sfiera): make exception-safe.
    foreach (it, _strings) {
        delete *it;
    }
    _strings.clear();
}

void StringList::load(int id) {
    clear();
    Resource rsrc('STR#', id);
    BytesBinaryReader bin(rsrc.data().substr(0, 2));
    uint16_t size;
    bin.read(&size);
    BytesPiece data = BytesPiece(rsrc.data()).substr(2);
    for (size_t i = 0; i < size; ++i) {
        uint8_t len = data.at(0);
        data = data.substr(1);
        _strings.push_back(new String(data.substr(0, len), mac_roman_encoding()));
        data = data.substr(len);
    }
}

ssize_t StringList::index_of(const StringPiece& result) const {
    for (size_t i = 0; i < size(); ++i) {
        if (at(i) == result) {
            return i;
        }
    }
    return -1;
}

size_t StringList::size() const {
    return _strings.size();
}

const String& StringList::at(size_t index) const {
    return *_strings.at(index);
}

void string_to_pstring(const String& src, unsigned char* dst) {
    Bytes src_bytes(src, mac_roman_encoding());
    if (src_bytes.size() > 254) {
        throw Exception("{0} is too long to convert to a pstring", quote(src));
    }
    *dst = src_bytes.size();
    memcpy(dst + 1, src_bytes.data(), src_bytes.size());
}

}  // namespace antares

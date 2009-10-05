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

#include <assert.h>

#include "BinaryStream.hpp"
#include "FakeSounds.hpp"
#include "Fakes.hpp"
#include "Resource.hpp"
#include "String.hpp"

void GetIndString(unsigned char* result, int id, int index) {
    if (index <= 0) {
        *result = '\0';
        return;
    }
    Resource rsrc('STR#', id);
    BufferBinaryReader bin(rsrc.data(), rsrc.size());
    uint16_t count;
    bin.read(&count);
    assert(index <= count);

    while (index > 1) {
        uint8_t size;
        bin.read(&size);
        bin.discard(size);
        --index;
    }
    bin.read(result);
    bin.read(result + 1, result[0]);
}

void BlockMove(void* src, void* dst, size_t size) {
    memcpy(dst, src, size);
}

int Munger(TypedHandle<unsigned char> data, int pos, const unsigned char* search, size_t search_len,
        const unsigned char* replace, size_t replace_len) {
    ustring s(search, search_len);
    ustring r(replace, replace_len);
    ustring d(*data, data.count());
    ustring::size_type at = d.find(s, pos);
    if (at != std::string::npos) {
        if (replace_len > search_len) {
            data.resize(data.count() + replace_len - search_len);
        }
        memcpy(*data + at, r.c_str(), r.size());
        memcpy(*data + at + r.size(), d.c_str() + at + s.size(), d.size() - at - s.size());
    }
    return at;
}

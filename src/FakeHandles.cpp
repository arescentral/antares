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

#include "sfz/ReadItem.hpp"
#include "Error.hpp"
#include "FakeSounds.hpp"
#include "Fakes.hpp"
#include "Resource.hpp"

using sfz::BytesPiece;
using sfz::read;

namespace antares {

void GetIndString(unsigned char* result, int id, int index) {
    if (index <= 0) {
        *result = '\0';
        return;
    }
    Resource rsrc("strings", "STR#", id);
    BytesPiece in(rsrc.data());
    uint16_t count;
    read(&in, &count);
    check(index <= count, "out-of-bounds string requested in GetIndString()");

    while (index > 1) {
        uint8_t size;
        read(&in, &size);
        in.shift(size);
        --index;
    }
    read(&in, result);
    read(&in, result + 1, result[0]);
}

}  // namespace antares

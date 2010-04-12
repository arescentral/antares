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

#include "StringHandling.hpp"

#include "sfz/Bytes.hpp"
#include "sfz/String.hpp"

using sfz::BytesPiece;

namespace antares {

void CopyPString(unsigned char *to, const unsigned char *from) {
    *to = *from;
    int l = *to;
    ++to;
    ++from;
    for (int i = 0; i < l; ++i) {
        *(to++) = *(from++);
    }
}

void ConcatenatePString(unsigned char *s1, const unsigned char *s2) {
    unsigned char* dc = s1 + *s1 + 1;
    const unsigned char* sc = s2 + 1;
    for (int i = 0; (i < *s2) && (*s1 < 255); ++i) {
        *dc = *sc;
        ++(*s1);
        ++dc;
        ++sc;
    }
}

BytesPiece PStringBytes(const unsigned char* string) {
    return BytesPiece(string + 1, *string);
}

}  // namespace antares

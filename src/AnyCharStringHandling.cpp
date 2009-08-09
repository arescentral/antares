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

// AnyChar String Handling

#include "AnyChar.hpp"
#include "Casts.hpp"

void CopyAnyCharPString( anyCharType *to, anyCharType *from)

{
    short       i, l;

    *to = *from;
    l = *to;
    to++;
    from++;
    for ( i = 0; i < l; i++)
        *(to++) = *(from++);
}

void InsertAnyCharPStringInPString( anyCharType *to, anyCharType *from, long offset)
{
    anyCharType *dc, *sc;
    long        count, flen;

    if ( offset <= *to)
    {
        if ( (*to + *from) > kAnyCharPStringMaxLen) flen = kAnyCharPStringMaxLen - implicit_cast<long>(*from);
        else flen = *from;

        count = *to - (offset);
        dc = to + offset + 1 + flen + ( count - 1);
        sc = to + offset + 1 + ( count - 1);
        *to += flen;

        // move part that will be replaced
        while ( count > 0)
        {
            *dc = *sc;
            dc--;
            sc--;
            count--;
        }

        dc = to + offset + 1;
        sc = from + 1;
        count = flen; // count = chars over max len

        while ( count > 0)
        {
            *dc = *sc;
            dc++;
            sc++;
            count--;
        }
    }
}

void CutCharsFromAnyCharPString( anyCharType *string, long start, long length)
{
    anyCharType *dc, *sc;
    long        count;

    if ( start <= *string)
    {
        if (( start + length) > *string)
        {
            length = *string - start;
        }
        dc = string + start + 1;
        sc = string + start + length + 1;
        count = *string - length;
        while ( count > 0)
        {
            *dc = *sc;
            dc++;
            sc++;
            count--;
        }
        *string -= length;
    }
}

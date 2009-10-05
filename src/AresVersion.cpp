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

// Ares_Version.c

#pragma mark **INCLUDES**

#include "AresVersion.hpp"

#include "StringHandling.hpp"
#include "StringNumerics.hpp"

namespace antares {

inline bool mChar_is_numeric(char m_c) {
    return (((m_c) >= '0') && ((m_c) <= '9'));
}

inline bool mChar_is_whitespace(char m_c) {
    return (((m_c) == ' ') || ((m_c) == '\r') || ((m_c) == '\r'));
}

inline bool mChar_is_alpha(char m_c) {
    return ((((m_c) >= 'a') && ((m_c) <= 'z')) || (((m_c) >= 'A') && ((m_c) <= 'Z')));
}

inline bool mChar_to_upper(char m_c) {
    return ((((m_c) >= 'a') && ((m_c) <= 'z')) ? ( ((m_c) - 'a') + 'A') : (m_c));
}

aresVersionType AresVersion_Get_FromString(unsigned char* s) {
    aresVersionType     result = 0;
    long                i = 1, v = 0;

    mAssert( s != nil);
    if ( s == nil) return 0;

    // N.x.x.c
    while (( i <= s[0]) && ( mChar_is_whitespace( s[i]))) i++;

    v = 0;
    while (( i <= s[0]) && ( mChar_is_numeric( s[i])))
    {
        v *= 10;
        v += s[i] - '0';
        i++;
    }
    result = v << 24;

    // x.N.x.c
    while (( i <= s[0]) && ( mChar_is_whitespace( s[i]))) i++;
    if (( i > s[0]) || (( s[i] != '.') && ( !mChar_is_numeric( s[i]))))
        return result;

    if ( s[i] == '.') i++;

    while (( i <= s[0]) && ( mChar_is_whitespace( s[i]))) i++;

    v = 0;
    while (( i <= s[0]) && ( mChar_is_numeric( s[i])))
    {
        v *= 10;
        v += s[i] - '0';
        i++;
    }
    result += v << 16;

    // x.x.N.c
    while (( i <= s[0]) && ( mChar_is_whitespace( s[i]))) i++;
    if (( i > s[0]) || (( s[i] != '.') && ( !mChar_is_numeric( s[i]))))
        return result;

    if ( s[i] == '.') i++;

    while (( i <= s[0]) && ( mChar_is_whitespace( s[i]))) i++;

    v = 0;

    while (( i <= s[0]) && ( mChar_is_numeric( s[i])))
    {
        v *= 10;
        v += s[i] - '0';
        i++;
    }
    result += v << 8;

    // x.x.x.C
    while (( i <= s[0]) && ( mChar_is_whitespace( s[i]))) i++;
    if (( i > s[0]) || ( !mChar_is_alpha( s[i]))) return result;
    result += mChar_to_upper( s[i]);
    return result;
}

unsigned char* String_Get_FromAresVersion(unsigned char* s, aresVersionType t) {
    unsigned char   *c = reinterpret_cast<unsigned char*>(&t);
    Str255          numString;

    mAssert( s != nil);
    if ( s == nil) return nil;

    // N.x.x.c
    NumToString( *c, s);

    // n.X.x.c
    c++;
    NumToString( *c, numString);
    pstrcat( s, "\p.");
    pstrcat( s, numString);

    // n.x.X.c
    c++;
    NumToString( *c, numString);
    pstrcat( s, "\p.");
    pstrcat( s, numString);

    // n.x.x.C
    c++;
    if ( *c != 0)
    {
        s[0] = s[0] + 1;
        s[s[0]] = *c;
    }

    return s;
}

}  // namespace antares

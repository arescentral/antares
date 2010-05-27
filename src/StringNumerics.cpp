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

// String Numerics.c

#include "StringNumerics.hpp"

#include "sfz/Bytes.hpp"
#include "sfz/String.hpp"
#include "StringHandling.hpp"

using sfz::BytesPiece;
using sfz::PrintTarget;
using sfz::String;
using sfz::StringPiece;

namespace ascii = sfz::ascii;

namespace antares {

SmallFixed small_fixed(smallFixedType value) {
    return SmallFixed(value);
}

void print_to(PrintTarget out, const SmallFixed& fixed) {
    Str255 s;
    SmallFixedToString(fixed.value, s);
    print_to(out, ascii::decode(PStringBytes(s)));
}

void SmallFixedToString( smallFixedType f, Str255 s)

{
    short               charnum = 0;
    long                l, i, j;
    Str255              tstring;
    smallFixedType      tf;
    double              tfloat;
    
    s[0] = 0;
    if ( f < 0)
    {
        charnum++;
        s[0] = charnum;
        s[charnum] = '-';
        f = -f;
    }
    l = mFixedToLong( f);
    LongToString( l, tstring);
    ConcatenatePString(s, tstring);
    charnum = s[0] + 1;
    s[0] = charnum;
    s[charnum] = '.';
    
    tf = mLongToFixed( l);
    tf = f - tf;
    tfloat = tf;
    tfloat /= 256;
//  tfloat += .0019531;
    
/*  tfloat *= 100;
    
    l = (long)tfloat;
    if ( l < 10)
    {
        charnum++;
        s[0] = charnum;
        s[charnum] = '0';
    }
    
    LongToString( l, tstring);
    ConcatenatePString( (unsigned char *)s, (unsigned char *)tstring);
*/
    for ( i = 0; i < 3; i++)
    {
        tfloat *= 10;
        j = l = (tfloat);
        if ( j > 9) j = 9;
        s[0]++;
        s[s[0]] = '0' + j;
        tfloat -= l;
    }
}

smallFixedType StringToSmallFixed( Str255 s)

{
    short           charnum = 1;
    bool     negative = false, seenDecimal = false;
    double          value = 0, divider = 0, tfloat, sign = 1.0;
    
    while (( charnum <= s[0]) && (( s[charnum] < '0') || ( s[charnum] > '9')) &&
            (( s[charnum] != '-') && ( s[charnum] != '.')))
    {
        charnum++;
    }
    
    if (( s[charnum] == '-') && ( charnum <= s[0]))
    {
        negative = true;
        sign = -1.0;
        charnum++;
        while (( charnum <= s[0]) && ((( s[charnum] < '0') || ( s[charnum] > '9'))
                && ( s[charnum] != '.')))
        {
            charnum++;
        }
    }
    
    while ( charnum <= s[0])
    {
        if (( s[charnum] < '0') || ( s[charnum] > '9'))
        {
            if ( s[charnum] == '.')
            {
                if ( !seenDecimal)
                {
                    seenDecimal = true;
                    divider = 10;
                }
            }
        } else
        {
            if ( !seenDecimal)
            {
                value *= 10;
                value += (s[charnum] - '0');
            } else
            {
                tfloat = s[charnum] - '0';
                tfloat /= divider;
                value += tfloat;
//              value += (float)(s[charnum] - '0') / divider;
                divider *= 10;
            }
        }
        charnum++;
    }
    if ( negative) value = -value;
    return( mFloatToFixed( value + (.0019531 * sign)));
}

void DoubleToString(double d, unsigned char* s, long places) {
    long                l;
    unsigned long   tenFactor = 1000000000, t;  // 2 147 483 647 is max signed long
    
    s[0] = 0;
    if ( d < 0)
    {
        s[1] = '-';
        s[0] = 1;
        d = -d;
    }
    
    l = d;
    while ((tenFactor > static_cast<uint32_t>(l)) && ( tenFactor > 0)) tenFactor /= 10;

    if ( tenFactor == 0)
    {
        s[0]++;
        s[s[0]] = '0';
    } else
    {

        do
        {
            s[0]++;
            t = l / tenFactor;
            s[s[0]] = '0' + t;
            l -= t * tenFactor;
            tenFactor /= 10;
        } while ( tenFactor > 0);
    }
    if ( places > 0)
    {
        s[0]++;
        s[s[0]] = '.';
        while ( places > 0)
        {
            l = d;
            d -= l;
            d *= 10;
            s[0]++;
            l = d;// + .5;
            if ( l >= 10) l = 9;
            s[s[0]] = '0' + l;
            places--;
        }
    }
}

double StringToDouble(unsigned char* s) {
    double  decimalPlace = 10, result = 0;
    long    c = 1;
    
    while (( c <= s[0]) && ( s[c] != '.'))
    {
        result = (result * 10) + (s[c] - '0');
        c++;
    }
    
    if ( s[c] == '.') c++;
    
    while ( c <= s[0])
    {
        result = result + (s[c] - '0') / decimalPlace;
        decimalPlace *= 10;
        c++;
    }
    return result;
}

void NumToHexString( unsigned long l, Str255 s, long bytenum)

{
    unsigned char   *strlen, *c;
    unsigned long   calc;
    
    strlen = s;
    c = strlen + 1;
    *strlen = 0;
    bytenum *= 8;   // 8 bits per byte
    do
    {
        bytenum -= 4;
        calc = l;
        calc >>= bytenum;
        calc &= 0x0000000f;
        (*strlen)++;
        if ( calc < 10) *c = '0' + calc;
        else if ( calc < 16) *c = 'A' + (calc - 10);
        else *c = '#';
        c++;
    } while ( bytenum > 0);
}

void HexStringToNum( Str255 s, unsigned long *l)
{
    long                strlen;
    unsigned char   *c;
    
    c = s;
    strlen = *c;
    *l = 0;
    c++;
    while ( strlen > 0)
    {
        if (( *c >= '0') && ( *c <= '9'))
        {
            *l += *c - '0';
        } else if (( *c >= 'a') && ( *c <= 'f'))
        {
            *l += *c - 'a' + 10;
        } else
        {
            *l += *c - 'A' + 10;
        }
        strlen--;
        c++;
        if ( strlen > 0) *l <<= 4;
    }
}

void UnsignedLongToString(unsigned long l, unsigned char* s) {
    unsigned long   tenFactor = 1000000000, t;  // 4 294 967 295 is max unsigned long
    
//  while ((( l % tenFactor) == 0) && ( tenFactor > 0)) tenFactor /= 10;
    while ((tenFactor > l) && ( tenFactor > 0)) tenFactor /= 10;
    if ( tenFactor == 0)
    {
        s[0] = 1;
        s[1] = '0';
        return;
    }
    
    s[0] = 0;
    do
    {
        s[0]++;
        t = l / tenFactor;
        s[s[0]] = '0' + t;
        l -= t * tenFactor;
        tenFactor /= 10;
    } while ( tenFactor > 0);
}

unsigned long StringToUnsignedLong(unsigned char* s) {
    unsigned long   result = 0;
    long                charNum = 1, strLen = s[0];
    
    while ( charNum <= strLen)
    {
        if ((s[charNum] >= '0') && ( s[charNum] <= '9'))
        {
            result *= 10;
            result += s[charNum] - '0';
        }
        charNum++;
    }
    return( result);
}

unsigned char* LongToString(long l, unsigned char* s) {
    unsigned long   tenFactor = 1000000000, t;  // 2 147 483 647 is max signed long
    
    if ( l < 0)
    {
        s[0] = 1;
        s[1] = '-';
        l = -l;
    } else
    {
        s[0] = 0;
    }

    while ((tenFactor > static_cast<uint32_t>(l)) && ( tenFactor > 0)) tenFactor /= 10;

    if ( tenFactor == 0)
    {
        s[0] = 1;
        s[1] = '0';
        return s;
    }

    do
    {
        s[0]++;
        t = l / tenFactor;
        s[s[0]] = '0' + t;
        l -= t * tenFactor;
        tenFactor /= 10;
    } while ( tenFactor > 0);
    return s;
}

long StringToLong(unsigned char* s) {
    long                result = 0, charNum = 1, strLen = s[0], sign = 1;
    bool             seenSign = false;
    
//  if ( s[1] == '-')
//  {
//      sign = -1;
//      charNum++;
//  }
    
    while ( charNum <= strLen)
    {
        if ((s[charNum] >= '0') && ( s[charNum] <= '9'))
        {
            if ( s[charNum] != 0) seenSign = true;
            result *= 10;
            result += s[charNum] - '0';
        } else if (( s[charNum] == '-') && ( !seenSign))
        {
            sign = -1;
            seenSign = true;
        }
        charNum++;
    }
    result *= sign;
    return( result);
}

}  // namespace antares

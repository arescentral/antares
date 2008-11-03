/*
Ares, a tactical space combat game.
Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

/* String Handling.c */

#ifndef __CONDITIONALMACROS__
#include "ConditionalMacros.h"
#endif // __CONDITIONALMACROS__

#if TARGET_OS_WIN32

    #ifndef __QUICKTIMEVR__
    #include <QuickTimeVR.h>
    #endif

    #ifndef __QTUtilities__
    #include "QTUtilities.h"
    #endif

    #ifndef __QTVRUtilities__
    #include "QTVRUtilities.h"
    #endif

    #include <TextUtils.h>
    #include <Script.h>
    #include <string.h>
#endif // TARGET_OS_WIN32

#include "String Handling.h"

void CopyPString( unsigned char *to, unsigned char *from)

{
    int     i, l;

    *to = *from;
    l = *to;
    to++;
    from++;
    for ( i = 0; i < l; i++)
        *(to++) = *(from++);
}

Boolean ComparePString( unsigned char *s1, unsigned char *s2)

{
    int     len;
    unsigned char   c1, c2;

    if ( *s1 == *s2)
    {
        len = *s1;
        do
        {
            s1++;
            s2++;
            c1 = *s1;
            c2 = *s2;
            if (( c1 >= 'a') && ( c1 <= 'z')) c1 = c1 - 'a' + 'A';
            if (( c2 >= 'a') && ( c2 <= 'z')) c2 = c2 - 'a' + 'A';
            len--;
        } while (( c1 == c2) && ( len > 0));
        return ( c1 == c2);
    } else { return ( FALSE);}
}

int PStringLen( unsigned char *s)

{
    return ((int)*s);
}

void ConcatenatePString( unsigned char *dString, unsigned char *sString)

{
    unsigned char   *dc, *sc;
    int     i;

    dc = dString + (long)*dString + 1L;
    sc = sString + 1L;
    for ( i = 0; (i < *sString) && ( *dString < 255); i++)
    {
        *dc = *sc;
        (*dString)++;
        dc++;
        sc++;
    }
}

void PStringFromCString( unsigned char *pString, unsigned char *cString)

{
    unsigned char   *len;

    len = pString;
    pString++;
    *len = 0;
    while (( *cString != '\0') && ( *len < 255))
    {
        *pString = *cString;
        (*len)++;
        pString++;
        cString++;
    }
}

void ReplacePStringChar( StringPtr s, unsigned char to, unsigned char from)

{
    int     l;
    unsigned char   *c;

    c = (unsigned char *)s;
    l = *c;
    c++;
    while ( l > 0)
    {
        if ( *c == from) *c = to;
        l--;
        c++;
    }
}

// FilterAlphaPString -- replaces any non-numeric chars with ' '
void FilterAlphaPString( StringPtr s)
{
    int     l;
    unsigned char   *c;

    c = (unsigned char *)s;
    l = *c;
    c++;
    while ( l > 0)
    {
        if (( *c < '0') || ( *c > '9')) *c = ' ';
        l--;
        c++;
    }
}

// ChopAlphaPString -- scans a string until it encounters non-numeric char, then sets length to
// that char i.e. "1234Buckle My Shoe" truncates to "1234"
void ChopAlphaPString( StringPtr s)
{
    short   len, newlen = 0;
    unsigned char   *c;

    c = (unsigned char *)s;
    len = *c;
    c++;
    while (( len > 0) && ( (*c >= '0') && ( *c <= '9')))
    {
        newlen++;
        c++;
        len--;
    }
    c = (unsigned char *)s;
    *c = newlen;
}

void UpperCasePString( StringPtr s)
{
    int     l;
    unsigned char   *c;

    c = (unsigned char *)s;
    l = *c;
    c++;
    while ( l > 0)
    {
        if (( *c >= 'a') && ( *c <= 'z')) *c = (*c - 'a') + 'A';
        l--;
        c++;
    }
}



void DrawCString( unsigned char *cString)

{
    Str255  pString;

    PStringFromCString( (unsigned char *)pString, cString);
    DrawString( (StringPtr)pString);
}

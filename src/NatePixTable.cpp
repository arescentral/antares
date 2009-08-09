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

// NATEPIXTABLE.C

// ShapeMachine IV by Nathan Lamont

#include "NatePixTable.hpp"

#include "ColorTranslation.hpp"
#include "ConditionalMacros.h"
#include "Debug.hpp"
#include "Resources.h"
#include "Sound.h"

#define EMPTY_NATE_PIX_SIZE         8L


Handle  CreateNatePixTable( void)

{
    Handle          newTable;
    unsigned long   *size;
    long            *shapenum;

    newTable = NewHandle( 16L);
    if ( newTable != nil)
    {
        HLock( newTable);
        size = reinterpret_cast<unsigned long *>(*newTable);
        *size = 8;
        shapenum = reinterpret_cast<long *>(*newTable + 1);
        *shapenum = 0;
        return ( newTable);
    } else return ( nil);
}

void MoveNatePixTableData( Handle table, long fromWhere, long toWhere)

{
    Size    oldSize, newSize, length;
    char    *from, *to;

    oldSize = GetHandleSize( table);
    newSize = oldSize + (toWhere - fromWhere);
    if ( newSize > oldSize)
    {
        HUnlock( table);
        SetHandleSize( table, newSize);
        MoveHHi( table);
        HLock( table);
    }
    if (( toWhere < newSize) && ( fromWhere < oldSize))
    {
        length = oldSize - fromWhere;
        if ( (toWhere + length) > newSize)
        {
            length = newSize - toWhere;
            SysBeep(20);
        }
        if ( (fromWhere + length) > oldSize)
        {
            length = oldSize - toWhere;
            SysBeep(20);
        }
        from = *table + fromWhere;
        to = *table + toWhere;
        BlockMove( from, to, length);
    }
    if ( newSize < oldSize)
    {
        HUnlock( table);
        SetHandleSize( table, newSize);
        MoveHHi( table);
        HLock( table);
    }
}

unsigned long GetNatePixTableSize( Handle table)

{
    unsigned long   *tablesize;

    tablesize = reinterpret_cast<unsigned long *>(*table);
    return (*tablesize);
}

void SetNatePixTableSize( Handle table, unsigned long newsize)

{
    unsigned long   *tablesize;

    tablesize = reinterpret_cast<unsigned long *>(*table);
    *tablesize = newsize;
}

long GetNatePixTablePixNum( Handle table)

{
    long    *pixnum;

    pixnum = reinterpret_cast<long *>(*table) + 1;
    return (*pixnum);
}

void SetNatePixTablePixNum( Handle table, long newnum)

{
    long    *pixnum;

    pixnum = reinterpret_cast<long *>(*table) + 1;
    *pixnum = newnum;
}

unsigned long GetNatePixTablePixOffset( Handle table, long pixnum)

{
    unsigned long   *pixoffset;

    pixoffset = reinterpret_cast<unsigned long *>(*table) + 2L + pixnum;
    return ( *pixoffset);
}

void SetNatePixTablePixOffset( Handle table, long pixnum, unsigned long newoffset)

{
    unsigned long   *pixoffset;

    pixoffset = reinterpret_cast<unsigned long*>(*table) + 2L + pixnum;
    *pixoffset = newoffset;
}

int GetNatePixTableNatePixWidth( Handle table, long pixnum)

{
    char        *aByte;
    short           *anInt;

    aByte = reinterpret_cast<char*>(*table) + GetNatePixTablePixOffset( table, pixnum);
    anInt = reinterpret_cast<short*>(aByte);
    return *anInt;
}

void SetNatePixTableNatePixWidth( Handle table, long pixnum, int width)

{
    char        *aByte;
    short           *anInt;

    aByte = reinterpret_cast<char*>(*table) + GetNatePixTablePixOffset( table, pixnum);
    anInt = reinterpret_cast<short*>(aByte);
    *anInt = width;
}

int GetNatePixTableNatePixHeight( Handle table, long pixnum)

{
    char        *aByte;
    short           *anInt;

    aByte = reinterpret_cast<char*>(*table) + GetNatePixTablePixOffset( table, pixnum) + 2L;
    anInt = reinterpret_cast<short*>(aByte);
    return *anInt;
}

void SetNatePixTableNatePixHeight( Handle table, long pixnum, int height)

{
    char        *aByte;
    short           *anInt;

    aByte = reinterpret_cast<char*>(*table) + GetNatePixTablePixOffset( table, pixnum) + 2L;
    anInt = reinterpret_cast<short*>(aByte);
    *anInt = height;
}

int GetNatePixTableNatePixHRef( Handle table, long pixnum)

{
    char        *aByte;
    short           *anInt;

    aByte = reinterpret_cast<char*>(*table) + GetNatePixTablePixOffset( table, pixnum) + 4L;
    anInt = reinterpret_cast<short*>(aByte);
    return *anInt;
}

void SetNatePixTableNatePixHRef( Handle table, long pixnum, int href)

{
    char        *aByte;
    short           *anInt;

    aByte = reinterpret_cast<char*>(*table) + GetNatePixTablePixOffset( table, pixnum) + 4L;
    anInt = reinterpret_cast<short*>(aByte);
    *anInt = href;
}

int GetNatePixTableNatePixVRef( Handle table, long pixnum)

{
    char        *aByte;
    short           *anInt;

    aByte = reinterpret_cast<char*>(*table) + GetNatePixTablePixOffset( table, pixnum) + 6L;
    anInt = reinterpret_cast<short*>(aByte);
    return *anInt;
}

void SetNatePixTableNatePixVRef( Handle table, long pixnum, int vref)

{
    char        *aByte;
    short           *anInt;

    aByte = reinterpret_cast<char*>(*table) + GetNatePixTablePixOffset( table, pixnum) + 6L;
    anInt = reinterpret_cast<short*>(aByte);
    *anInt = vref;
}

char *GetNatePixTableNatePixData( Handle table, long pixnum)

{
    char        *aByte;

    aByte = reinterpret_cast<char*>(*table) + GetNatePixTablePixOffset( table, pixnum) + 8L;
    return ( aByte);
}

unsigned char GetNatePixTableNatePixDataPixel( Handle table, long pixnum, int x, int y)

{
    unsigned char       *aByte;

    aByte = reinterpret_cast<unsigned char*>(*table) + GetNatePixTablePixOffset( table, pixnum)
            + 8L + y *
            GetNatePixTableNatePixWidth(table, pixnum) + x;
    return ( *aByte);
}

void SetNatePixTableNatePixDataPixel( Handle table, long pixnum, int x, int y, unsigned char dByte)

{
    unsigned char       *aByte;

    aByte = reinterpret_cast<unsigned char*>(*table) + GetNatePixTablePixOffset( table, pixnum) + 8L + y *
            GetNatePixTableNatePixWidth(table, pixnum) + x;
    *aByte = dByte;
}

Handle GetNatePixTableNatePixDataCopy( Handle table, long pixnum)

{
    long                size, l;
    Handle              copy;
    unsigned char       *sByte, *dByte;

    size = GetNatePixTableNatePixWidth( table, pixnum) *
            GetNatePixTableNatePixHeight( table, pixnum);
    copy = NewHandle( size);
    if ( copy == 0L)
        SysBeep( 20);
    else
    {
        MoveHHi( copy);
        HLock( copy);
        sByte = reinterpret_cast<unsigned char*>(*table) + GetNatePixTablePixOffset( table, pixnum) + 8L;
        dByte = reinterpret_cast<unsigned char *>(*copy);
        for ( l = 0; l < size; l++)
        {
            *dByte = *sByte;
            dByte++;
            sByte++;
        }
    }
    return (copy);
}

// GetNatePixTableNatePixPtr:
//  makes a new natePix structure with a baseAddr pointing into the natePixTable, meaning
//  the original natePixTable cannot be unlocked or disposed.

/*
void GetNatePixTableNatePixPtr( natePix *dPix, Handle table, int pixnum)

{

    if (NewNatePix( dPix, GetNatePixTableNatePixWidth( table, pixnum),
            GetNatePixTableNatePixHeight( table, pixnum), reinterpret_cast<char*>(*table) +
            GetNatePixTablePixOffset( table, pixnum) + 8L) != 0)
        SysBeep (20);
}
*/

/*
void GetNatePixTableNatePixDuplicate( natePix *dPix, Handle table, int pixnum)

{

    if (NewNatePix( dPix, GetNatePixTableNatePixWidth( table, pixnum),
            GetNatePixTableNatePixHeight( table, pixnum), reinterpret_cast<char*>(*table) +
            GetNatePixTablePixOffset( table, pixnum) + 8L) != 0)
        SysBeep (20);
    else
    {
        dPix->pixBase = GetNatePixTableNatePixDataCopy( table, pixnum);
    }
}
*/

unsigned long GetNatePixTableNatePixDataSize( Handle table, long pixnum)

{
    if ( pixnum == GetNatePixTablePixNum(table) - 1)
        return ( GetNatePixTableSize( table) -
                GetNatePixTablePixOffset( table, pixnum));
    else
        return( GetNatePixTablePixOffset( table, pixnum + 1) -
                GetNatePixTablePixOffset( table, pixnum));
}


void InsertNatePix( Handle table, Rect *sRect, int pixnum)

{
    long            newpixsize, l;
    unsigned long   offset;
    int             width;


    width = sRect->right - sRect->left;
    newpixsize = width * (sRect->bottom - sRect->top);
    if ( pixnum == GetNatePixTablePixNum( table))
    {
        MoveNatePixTableData( table, GetNatePixTableSize( table),
                GetNatePixTableSize(table) + EMPTY_NATE_PIX_SIZE + newpixsize);
        for ( l = 0; l < pixnum; l++)
        {
            SetNatePixTablePixOffset( table, l, GetNatePixTablePixOffset(
                    table, l) + 4);
        }
        MoveNatePixTableData( table, 8L + pixnum * 4L, 8L + pixnum * 4L +
                4L);
        SetNatePixTablePixOffset( table, pixnum, GetNatePixTableSize( table) + 4L);
    }
    else
    {
        MoveNatePixTableData( table, GetNatePixTablePixOffset( table,
                pixnum), GetNatePixTablePixOffset( table,pixnum) +
                EMPTY_NATE_PIX_SIZE + newpixsize);
        offset = GetNatePixTablePixOffset(table, pixnum) + 4;
        for ( l = 0; l < pixnum; l++)
        {
            SetNatePixTablePixOffset( table, l, GetNatePixTablePixOffset(
                    table, l) + 4);
        }
        for ( l = pixnum; l < GetNatePixTablePixNum( table); l++)
        {
            SetNatePixTablePixOffset( table, l, GetNatePixTablePixOffset(
                    table, l) + 4 + EMPTY_NATE_PIX_SIZE + newpixsize);
        }
        MoveNatePixTableData( table, 8L + pixnum * 4L, 8L + pixnum * 4L +
                4L);
        SetNatePixTablePixOffset( table, pixnum, offset);
    }
    SetNatePixTableSize( table, GetNatePixTableSize( table) +
            (EMPTY_NATE_PIX_SIZE + newpixsize + 4L));
    SetNatePixTablePixNum( table, GetNatePixTablePixNum( table) + 1L);
    SetNatePixTableNatePixWidth( table, pixnum, width);
    SetNatePixTableNatePixHeight( table, pixnum, (sRect->bottom - sRect->top));
    SetNatePixTableNatePixHRef( table, pixnum, 0);
    SetNatePixTableNatePixVRef( table, pixnum, 0);
/*
    // >>> It's now up to the caller to fill in the pixels!

    CopyPixBlock( (byte *)*(pix->pixBase), (byte *)GetNatePixTableNatePixData( table,
        pixnum), width, (**(pix->pix)).bounds.bottom);
*/
}

void DeleteNatePix( Handle table, int pixnum)

{
    long        l;
    unsigned long   oldpixsize;

    oldpixsize = GetNatePixTableNatePixDataSize( table, pixnum);
    MoveNatePixTableData( table, GetNatePixTablePixOffset( table, pixnum) +
        oldpixsize, GetNatePixTablePixOffset( table, pixnum));
    for ( l = 0; l < pixnum; l++)
    {
        SetNatePixTablePixOffset( table, l, GetNatePixTablePixOffset( table, l) -
                4);
    }
    for ( l = pixnum + 1L; l < GetNatePixTablePixNum( table); l++)
    {
        SetNatePixTablePixOffset( table, l, GetNatePixTablePixOffset( table, l) -
            4 - oldpixsize);
    }
    MoveNatePixTableData( table, 8L + pixnum * 4L + 4L, 8L + pixnum *
        4L);
    SetNatePixTableSize( table, GetNatePixTableSize( table) - oldpixsize -
        4);
    SetNatePixTablePixNum( table, GetNatePixTablePixNum( table) - 1L);
}

// RemapNatePixTableColor:
//  given a NatePixTable, converts the raw pixel data based on custom color table, and translates
//  into device's color table, using Backbone Graphic's GetTranslateIndex().

void RemapNatePixTableColor( Handle table)

{
    long            l;
    unsigned char   *p;
    int             i, j, w, h;

//  WriteDebugLine((char *)"\pRemapSize:");
//  WriteDebugLong( GetNatePixTablePixNum( table));

    for ( l = 0; l < GetNatePixTablePixNum( table); l++)
    {
//      WriteDebugLong( l);
        w = GetNatePixTableNatePixWidth( table, l);
        h = GetNatePixTableNatePixHeight( table, l);
        p = reinterpret_cast<unsigned char *>(GetNatePixTableNatePixData( table, l));
        for ( j = 0; j < h; j++)
        {
            for ( i = 0; i < w; i++)
            {
//              SetNatePixTableNatePixDataPixel( table, l, i, j,
//                  GetTranslateIndex(
//                  (int)GetNatePixTableNatePixDataPixel( table, l, i, j)));
                *p = GetTranslateIndex( *p);
                p++;
            }
        }
    }


}

// ColorizeNatePixTableColor:
//  given a NatePixTable, converts the raw pixel data based on custom color table, and translates
//  into device's color table, but colorizes to color.

void ColorizeNatePixTableColor( Handle table, unsigned char color)

{
    long            l, whiteCount, pixelCount;
    unsigned char   pixel, *p;
    int             i, j, w, h;

    color <<= 4;

    for ( l = 0; l < GetNatePixTablePixNum( table); l++)
    {
        w = GetNatePixTableNatePixWidth( table, l);
        h = GetNatePixTableNatePixHeight( table, l);

        // count the # of pixels, and # of pixels that are white
        whiteCount = pixelCount = 0;
        p = reinterpret_cast<unsigned char *>(GetNatePixTableNatePixData( table, l));
        for ( j = 0; j < h; j++)
        {
            for ( i = 0; i < w; i++)
            {
                pixel = *p; //GetNatePixTableNatePixDataPixel( table, l, i, j);
                if ( pixel != 0)
                {
                    pixelCount++;
                    if ( pixel <= 15) whiteCount++;
                }
                p++;
            }
        }

        if ( whiteCount > ( pixelCount / 3)) whiteCount = 1;
        else whiteCount = 0;
        p = reinterpret_cast<unsigned char *>(GetNatePixTableNatePixData( table, l));
        for ( j = 0; j < h; j++)
        {
            for ( i = 0; i < w; i++)
            {
                pixel = *p;//GetNatePixTableNatePixDataPixel( table, l, i, j);
                if ((pixel != 0) && ((pixel > 15) || ( whiteCount)))
                {
                    pixel &= 0x0f;
                    pixel += color;
                }
                *p = pixel; //SetNatePixTableNatePixDataPixel( table, l, i, j, pixel);
                p++;
            }
        }
    }
}

void RetromapNatePixTableColor( Handle table)

{
    long            l;
    int             i, j, w, h;

//  WriteDebugLine((char *)"\pRetroSize:");
//  WriteDebugLong( GetNatePixTablePixNum( table));

    for ( l = 0; l < GetNatePixTablePixNum( table); l++)
    {
//      WriteDebugLong( l);
        w = GetNatePixTableNatePixWidth( table, l);
        h = GetNatePixTableNatePixHeight( table, l);
        for ( j = 0; j < h; j++)
        {
            for ( i = 0; i < w; i++)
            {
                SetNatePixTableNatePixDataPixel( table, l, i, j, GetRetroIndex(
                    GetNatePixTableNatePixDataPixel( table, l, i, j)));
            }
        }
    }
}


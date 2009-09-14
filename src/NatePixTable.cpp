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

struct natePixEntryType {
    short width;
    short height;
    short hOffset;
    short vOffset;
    unsigned char data[];
};

struct natePixType {
    unsigned long size;
    long pixnum;
    long offsets[];

    natePixEntryType* entryAt(int pixnum) {
        int thisInt = reinterpret_cast<int>(this);
        return reinterpret_cast<natePixEntryType*>(thisInt + offsets[pixnum]);
    }
};

natePixType** CreateNatePixTable( void)
{
    natePixType**   newTable;

    newTable = reinterpret_cast<natePixType**>(NewHandle(16L));
    if ( newTable != nil)
    {
        HLock(reinterpret_cast<Handle>(newTable));
        (*newTable)->size = 8;
        (*newTable)->pixnum = 0;
        return ( newTable);
    } else return ( nil);
}

void MoveNatePixTableData(natePixType** table, long fromWhere, long toWhere)
{
    Size    oldSize, newSize, length;
    char    *from, *to;

    oldSize = GetHandleSize(reinterpret_cast<Handle>(table));
    newSize = oldSize + (toWhere - fromWhere);
    if ( newSize > oldSize)
    {
        HUnlock(reinterpret_cast<Handle>(table));
        SetHandleSize(reinterpret_cast<Handle>(table), newSize);
        MoveHHi(reinterpret_cast<Handle>(table));
        HLock(reinterpret_cast<Handle>(table));
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
        from = reinterpret_cast<char*>(*table) + fromWhere;
        to = reinterpret_cast<char*>(*table) + toWhere;
        BlockMove( from, to, length);
    }
    if ( newSize < oldSize)
    {
        HUnlock(reinterpret_cast<Handle>(table));
        SetHandleSize(reinterpret_cast<Handle>(table), newSize);
        MoveHHi(reinterpret_cast<Handle>(table));
        HLock(reinterpret_cast<Handle>(table));
    }
}

unsigned long GetNatePixTableSize(natePixType** table)
{
    return (*table)->size;
}

void SetNatePixTableSize(natePixType** table, unsigned long newsize)
{
    (*table)->size = newsize;
}

long GetNatePixTablePixNum(natePixType** table)
{
    return (*table)->pixnum;
}

void SetNatePixTablePixNum(natePixType** table, long newnum)
{
    (*table)->pixnum = newnum;
}

unsigned long GetNatePixTablePixOffset(natePixType** table, long pixnum)
{
    return (*table)->offsets[pixnum];
}

void SetNatePixTablePixOffset(natePixType** table, long pixnum, unsigned long newoffset)
{
    (*table)->offsets[pixnum] = newoffset;
}

int GetNatePixTableNatePixWidth(natePixType** table, long pixnum)
{
    return (*table)->entryAt(pixnum)->width;
}

void SetNatePixTableNatePixWidth(natePixType** table, long pixnum, int width)
{
    (*table)->entryAt(pixnum)->width = width;
}

int GetNatePixTableNatePixHeight(natePixType** table, long pixnum)
{
    return (*table)->entryAt(pixnum)->height;
}

void SetNatePixTableNatePixHeight(natePixType** table, long pixnum, int height)
{
    (*table)->entryAt(pixnum)->height = height;
}

int GetNatePixTableNatePixHRef(natePixType** table, long pixnum)
{
    return (*table)->entryAt(pixnum)->hOffset;
}

void SetNatePixTableNatePixHRef(natePixType** table, long pixnum, int href)
{
    (*table)->entryAt(pixnum)->hOffset = href;
}

int GetNatePixTableNatePixVRef(natePixType** table, long pixnum)
{
    return (*table)->entryAt(pixnum)->vOffset;
}

void SetNatePixTableNatePixVRef(natePixType** table, long pixnum, int vref)
{
    (*table)->entryAt(pixnum)->vOffset = vref;
}

unsigned char *GetNatePixTableNatePixData(natePixType** table, long pixnum)
{
    return (*table)->entryAt(pixnum)->data;
}

unsigned char GetNatePixTableNatePixDataPixel(natePixType** table, long pixnum, int x, int y)
{
    natePixEntryType* entry = (*table)->entryAt(pixnum);
    int width = entry->width;

    return entry->data[y * width + x];
}

void SetNatePixTableNatePixDataPixel(natePixType** table, long pixnum, int x, int y, unsigned char dByte)
{
    natePixEntryType* entry = (*table)->entryAt(pixnum);
    int width = entry->width;
    entry->data[y * width + x] = dByte;
}

Handle GetNatePixTableNatePixDataCopy(natePixType** table, long pixnum)
{
    long                size, l;
    Handle              copy;
    unsigned char*      sByte;
    unsigned char*      dByte;

    size = GetNatePixTableNatePixWidth( table, pixnum) *
            GetNatePixTableNatePixHeight( table, pixnum);
    copy = NewHandle(size);
    if (copy == 0L)
        SysBeep( 20);
    else
    {
        MoveHHi(copy);
        HLock(copy);
        sByte = (*table)->entryAt(pixnum)->data;
        dByte = *copy;
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
//  makes a new natePix structure with a baseAddr pointing into the natePixType, meaning
//  the original natePixType cannot be unlocked or disposed.

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

unsigned long GetNatePixTableNatePixDataSize(natePixType** table, long pixnum)

{
    if ( pixnum == GetNatePixTablePixNum(table) - 1)
        return ( GetNatePixTableSize( table) -
                GetNatePixTablePixOffset( table, pixnum));
    else
        return( GetNatePixTablePixOffset( table, pixnum + 1) -
                GetNatePixTablePixOffset( table, pixnum));
}


void InsertNatePix(natePixType** table, Rect *sRect, int pixnum)

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

void DeleteNatePix(natePixType** table, int pixnum)

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

void RemapNatePixTableColor(natePixType** table)

{
    long            l;
    unsigned char*  p;
    int             i, j, w, h;

//  WriteDebugLine((char *)"\pRemapSize:");
//  WriteDebugLong( GetNatePixTablePixNum( table));

    for ( l = 0; l < GetNatePixTablePixNum( table); l++)
    {
//      WriteDebugLong( l);
        w = GetNatePixTableNatePixWidth( table, l);
        h = GetNatePixTableNatePixHeight( table, l);
        p = GetNatePixTableNatePixData( table, l);
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

void ColorizeNatePixTableColor(natePixType** table, unsigned char color)

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
        p = GetNatePixTableNatePixData( table, l);
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
        p = GetNatePixTableNatePixData( table, l);
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

void RetromapNatePixTableColor(natePixType** table)

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


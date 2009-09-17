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

long GetNatePixTablePixNum(natePixType** table)
{
    return (*table)->pixnum;
}

int GetNatePixTableNatePixWidth(natePixType** table, long pixnum)
{
    return (*table)->entryAt(pixnum)->width;
}

int GetNatePixTableNatePixHeight(natePixType** table, long pixnum)
{
    return (*table)->entryAt(pixnum)->height;
}

int GetNatePixTableNatePixHRef(natePixType** table, long pixnum)
{
    return (*table)->entryAt(pixnum)->hOffset;
}

int GetNatePixTableNatePixVRef(natePixType** table, long pixnum)
{
    return (*table)->entryAt(pixnum)->vOffset;
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

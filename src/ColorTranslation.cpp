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

// Color Translation.c

#include "ColorTranslation.hpp"

#include "ConditionalMacros.h"
#include "Debug.hpp"
#include "Error.hpp"
#include "HandleHandling.hpp"

#define kColorTransError        "\pCLTR"

extern  GDHandle        theDevice;

Handle                  gColorTranslateTable = nil;

void ColorTranslatorInit( CTabHandle theClut)

{
    gColorTranslateTable = NewHandle( sizeof( transColorType) * (long)kPaletteSize);
    if ( gColorTranslateTable == nil)
    {
        ShowErrorAny( eQuitErr, kErrorStrID, nil, nil, nil, nil, MEMORY_ERROR, -1, -1, -1, __FILE__, 1);
    }
    else
    {
        /*
        MoveHHi( gColorTranslateTable);
        HLock( gColorTranslateTable);
        */
        mHandleLockAndRegister( gColorTranslateTable, nil, nil, nil, "\pgColorTranslateTable")
    }
    MakeColorTranslatorTable( theClut);
}

void ColorTranslatorCleanup( void)

{
    if ( gColorTranslateTable != nil)
        DisposeHandle( gColorTranslateTable);
}

void MakeColorTranslatorTable( CTabHandle referenceTable)

{
    transColorType      *entry, *retroEntry;
    int                 i, j;
    CTabHandle          deviceTable = nil;
    PixMapHandle        devicePixMap;
    RGBColor            paletteColor, deviceColor;

    devicePixMap = (*theDevice)->gdPMap;
    deviceTable = (**devicePixMap).pmTable;
    entry = (transColorType *)*gColorTranslateTable;
//  referenceTable = GetCTable( kReferenceColorTableID);
    if ( referenceTable == nil)
    {
        ShowErrorAny( eExitToShellErr, kErrorStrID, nil, nil, nil, nil, COLOR_TABLE_ERROR, -1, -1, -1, __FILE__, 2);
    }
    for ( i = 0; i < kPaletteSize; i++)
    {
        entry->trueColor = 0;
        paletteColor = (**referenceTable).ctTable[i].rgb;
        for ( j = 0; j < kPaletteSize; j++)
        {
//          Index2Color( (long)j, &deviceColor);
            deviceColor = (**deviceTable).ctTable[j].rgb;
            if (( deviceColor.red == paletteColor.red) &&
                    (deviceColor.green == paletteColor.green) &&
                    (deviceColor.blue == paletteColor.blue))
            {
                entry->trueColor = (unsigned char)j;
                retroEntry = (transColorType *)*gColorTranslateTable + (long)j;
                retroEntry->retroColor = (unsigned char)i;
            }
        }
        entry++;
    }
//  DisposHandle( (Handle)referenceTable);
}

unsigned char GetRetroIndex( unsigned char which)

{
    transColorType  *entry;

    entry = (transColorType *)*gColorTranslateTable + (long)which;
    return( entry->retroColor);
}

unsigned char GetTranslateIndex( unsigned char which)

{
    transColorType  *entry;

    entry = (transColorType *)*gColorTranslateTable + (long)which;
    return( entry->trueColor);
}

unsigned char GetTranslateColorShade( unsigned char color, unsigned char shade)

{
    transColorType  *entry;

    entry = (transColorType *)*gColorTranslateTable + (long)((16 - shade) + 1 +
            color * 16);
    return( entry->trueColor);
}

void SetTranslateColorShadeFore( unsigned char color, unsigned char shade)

{
    RGBColor        c;
    transColorType  *entry;

    entry = (transColorType *)*gColorTranslateTable + (long)((16 - shade) + 1L +
            (long)color * 16L);
    Index2Color( (long)entry->trueColor, &c);
    RGBForeColor( &c);
}

void GetRGBTranslateColorShade( RGBColor *c, unsigned char color, unsigned char shade)

{
    transColorType  *entry;

    entry = (transColorType *)*gColorTranslateTable + (long)((16 - shade) + 1L +
            (long)color * 16L);
    Index2Color( (long)entry->trueColor, c);
}

void SetTranslateColorFore( unsigned char color)

{
    RGBColor        c;
    transColorType  *entry;

    entry = (transColorType *)*gColorTranslateTable + (long)color;
    Index2Color( (long)entry->trueColor, &c);
    RGBForeColor( &c);
}

void GetRGBTranslateColor( RGBColor *c, unsigned char color)

{
    transColorType  *entry;

    entry = (transColorType *)*gColorTranslateTable + (long)color;
    Index2Color( (long)entry->trueColor, c);
}

void DefaultColors( void)

{
    RGBColor    c;

    c.red = c.blue = c.green = 0;
    RGBForeColor ( &c);
    c.red = c.blue = c.green = 65535;
    RGBBackColor( &c);
}

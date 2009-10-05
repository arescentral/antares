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

#include "ColorTable.hpp"
#include "Debug.hpp"
#include "Error.hpp"

namespace antares {

#define kColorTransError        "\pCLTR"

extern PixMap* gActiveWorld;

TypedHandle<transColorType> gColorTranslateTable;

void ColorTranslatorInit(const ColorTable& theClut) {
    gColorTranslateTable.create(kPaletteSize);
    MakeColorTranslatorTable(theClut);
}

void ColorTranslatorCleanup( void)

{
    if (gColorTranslateTable.get() != nil) {
        gColorTranslateTable.destroy();
    }
}

void MakeColorTranslatorTable(const ColorTable& referenceTable) {
    transColorType      *entry, *retroEntry;
    int                 i, j;
    RGBColor            paletteColor, deviceColor;

    const ColorTable& deviceTable = *gActiveWorld->colors;
    entry = *gColorTranslateTable;
//  referenceTable = GetCTable( kReferenceColorTableID);

    for ( i = 0; i < kPaletteSize; i++)
    {
        entry->trueColor = 0;
        paletteColor = referenceTable.color(i);
        for ( j = 0; j < kPaletteSize; j++)
        {
//          Index2Color( (long)j, &deviceColor);
            deviceColor = deviceTable.color(j);
            if (( deviceColor.red == paletteColor.red) &&
                    (deviceColor.green == paletteColor.green) &&
                    (deviceColor.blue == paletteColor.blue))
            {
                entry->trueColor = j;
                retroEntry = *gColorTranslateTable + implicit_cast<long>(j);
                retroEntry->retroColor = i;
            }
        }
        entry++;
    }
//  DisposHandle( (Handle)referenceTable);
}

unsigned char GetRetroIndex( unsigned char which)

{
    transColorType  *entry;

    entry = *gColorTranslateTable + implicit_cast<long>(which);
    return( entry->retroColor);
}

unsigned char GetTranslateIndex( unsigned char which)

{
    transColorType  *entry;

    entry = *gColorTranslateTable + implicit_cast<long>(which);
    return( entry->trueColor);
}

unsigned char GetTranslateColorShade( unsigned char color, unsigned char shade)

{
    transColorType  *entry;

    entry = *gColorTranslateTable + implicit_cast<long>((16 - shade) + 1 +
            color * 16);
    return( entry->trueColor);
}

void SetTranslateColorShadeFore( unsigned char color, unsigned char shade)

{
    RGBColor        c;
    transColorType  *entry;

    entry = *gColorTranslateTable + implicit_cast<long>((16 - shade) + 1L +
            implicit_cast<long>(color) * 16L);
    Index2Color(entry->trueColor, &c);
    RGBForeColor( &c);
}

void GetRGBTranslateColorShade( RGBColor *c, unsigned char color, unsigned char shade)

{
    transColorType  *entry;

    entry = *gColorTranslateTable + implicit_cast<long>((16 - shade) + 1L +
            implicit_cast<long>(color) * 16L);
    Index2Color( implicit_cast<long>(entry->trueColor), c);
}

void SetTranslateColorFore( unsigned char color)

{
    RGBColor        c;
    transColorType  *entry;

    entry = *gColorTranslateTable + implicit_cast<long>(color);
    Index2Color(entry->trueColor, &c);
    RGBForeColor( &c);
}

void GetRGBTranslateColor( RGBColor *c, unsigned char color)

{
    transColorType  *entry;

    entry = *gColorTranslateTable + implicit_cast<long>(color);
    Index2Color(entry->trueColor, c);
}

void DefaultColors( void)

{
    RGBColor    c;

    c.red = c.blue = c.green = 0;
    RGBForeColor ( &c);
    c.red = c.blue = c.green = 65535;
    RGBBackColor( &c);
}

}  // namespace antares

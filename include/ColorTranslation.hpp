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

#ifndef ANTARES_COLOR_TRANSLATION_HPP_
#define ANTARES_COLOR_TRANSLATION_HPP_

#include "Quickdraw.h"

#include "Casts.hpp"
#include "ColorTable.hpp"

namespace antares {

// Color Translation.h

#define kPaletteSize            256
#define kReferenceColorTableID  256

// EASY COLOR DEFINITIONS

#define RED             15
#define ORANGE          1
#define YELLOW          2
#define BLUE            3
#define GREEN           4
#define PURPLE          5
#define INDIGO          6
#define SALMON          7
#define GOLD            8
#define AQUA            9
#define PINK            10
#define PALE_GREEN      11
#define PALE_PURPLE     12
#define SKY_BLUE        13
#define TAN             14
#define GRAY            0

#define BLACK           0xff
#define WHITE           0x00

#define VERY_LIGHT      16
#define LIGHTER         14
#define LIGHT           12
#define MEDIUM          9
#define DARK            7
#define DARKER          5
#define VERY_DARK       3
#define DARKEST         1

#define COLOR_NUM           16
#define kVisibleShadeNum    15

#define kLighterColor           2
#define kDarkerColor            -2
#define kSlightlyLighterColor   1
#define kSlightlyDarkerColor    -1

struct transColorType {
    unsigned char           trueColor;
    unsigned char           retroColor;
    RgbColor                rgbcolor;
};

extern scoped_array<transColorType> gColorTranslateTable;

inline void mGetTranslateColorShade(
        uint8_t mcolor, uint8_t mshade, uint8_t& mresultColor, transColorType*& mtransColor) {
    mtransColor = gColorTranslateTable.get()
        + implicit_cast<long>((16 - implicit_cast<long>(mshade)) + 1 + implicit_cast<long>(mcolor) * 16);
    mresultColor = mtransColor->trueColor;
}

void ColorTranslatorInit(const ColorTable& colors);
void ColorTranslatorCleanup( void);
void MakeColorTranslatorTable(const ColorTable& colors);
unsigned char GetRetroIndex( unsigned char);
unsigned char GetTranslateIndex( unsigned char);
unsigned char GetTranslateColorShade( unsigned char, unsigned char);
void SetTranslateColorShadeFore( unsigned char, unsigned char);
void GetRGBTranslateColorShade( RgbColor *, unsigned char, unsigned char);
void SetTranslateColorFore( unsigned char);
void GetRGBTranslateColor( RgbColor *, unsigned char);
void DefaultColors( void);

}  // namespace antares

#endif // ANTARES_COLOR_TRANSLATION_HPP_

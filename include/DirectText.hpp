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

#ifndef ANTARES_DIRECT_TEXT_HPP_
#define ANTARES_DIRECT_TEXT_HPP_

// Direct Text.h

#include "sfz/Bytes.hpp"
#include "AnyChar.hpp"
#include "Casts.hpp"
#include "SpriteHandling.hpp"

namespace antares {

#define kCharSpace              0
#define kFirstCharacter         0
#define kLastCharacter          255
#define kCharNum                255
#define kDTextDescriptResType   'nlFD'
#define kDTextFontMapResType    'nlFM'

#define kDirectFontNum          6L

#define kTacticalFontNum        0L
#define kComputerFontNum        1L
#define kButtonFontNum          2L
#define kMessageFontNum         3L
#define kTitleFontNum           4L
#define kButtonSmallFontNum     5L

#define kTacticalFontResID      5000
#define kComputerFontResID      5001
#define kButtonFontResID        5002
#define kMessageFontResID       5003
#define kTitleFontResID         5004
#define kButtonSmallFontResID   5005

struct directTextType {
    directTextType(int32_t id);
    ~directTextType();

    sfz::Bytes charSet;
    int16_t resID;
    int32_t logicalWidth;
    int32_t physicalWidth;
    int32_t height;
    int32_t ascent;
};

int InitDirectText();
void DirectTextCleanup();

void mDirectCharWidth(unsigned char& mwidth, unsigned char mchar);
void mSetDirectFont(long mwhichFont);
int mDirectFontHeight();
int mDirectFontAscent();
void mGetDirectStringDimensions(unsigned char* string, long& width, long& height);
void DrawDirectTextStringClipped(unsigned char*, const RgbColor& color, PixMap *, const Rect&, long, long);

}  // namespace antares

#endif // ANTARES_DIRECT_TEXT_HPP_

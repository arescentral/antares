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

#include "AnyChar.hpp"
#include "Casts.hpp"
#include "Handle.hpp"
#include "SpriteHandling.hpp"

#pragma options align=mac68k

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
    unsigned char**     charSet;
    short       resID;
    ShortBoolean     myHandle;       // different texts can have the same handles; myHandle = TRUE if this guy's in charge of disposing
    long        logicalWidth;
    long        physicalWidth;
    long        height;
    long        ascent;
};

extern long gWhichDirectText;
extern directTextType* gDirectText;
extern TypedHandle<directTextType> gDirectTextData;

inline void mGetDirectStringDimensions(const unsigned char* string, long& width, long& height) {
    height = gDirectText->height << 1;
    width = (implicit_cast<long>(kCharSpace) + implicit_cast<long>(gDirectText->logicalWidth)) * implicit_cast<long>(*(string));   // width * length of string
}

inline void mGetDirectStringDimensions(
        unsigned char* mstring, long& mwidth, long& mheight, long &mstrlen, unsigned char*& mcharptr,
        unsigned char*& mwidptr) {
    mheight = gDirectText->height;
    mwidth = 0;
    mcharptr = mstring;
    mstrlen = implicit_cast<long>(*(mcharptr++));
    while( mstrlen > 0)
    {
        mwidptr = *(gDirectText->charSet)
            + gDirectText->height * gDirectText->physicalWidth * implicit_cast<long>(*mcharptr)
            + implicit_cast<long>(*mcharptr);
        mwidth += implicit_cast<long>(*mwidptr);
        mcharptr++;
        mstrlen--;
    }
}

inline void mDirectCharWidth(unsigned char& mwidth, char mchar, unsigned char*& mwidptr) {
    mwidptr = *(gDirectText->charSet)
        + gDirectText->height * gDirectText->physicalWidth * implicit_cast<long>(mchar)
        + implicit_cast<long>(mchar);
    mwidth = *mwidptr;
}

inline void mSetDirectFont(long mwhichFont) {
    gWhichDirectText = mwhichFont;
    gDirectText = *gDirectTextData + gWhichDirectText;
}

inline int mDirectFontHeight() {
    return gDirectText->height;
}

inline int mDirectFontAscent() {
    return gDirectText->ascent;
}

inline int mDirectFontLogicalWidth() {
    return gDirectText->logicalWidth;
}

int InitDirectText( void);
void DirectTextCleanup( void);
long GetDirectFontNum( short);
short AddDirectFont( directTextType *);

void DrawDirectTextString( char *, unsigned char, PixMap *, long, long);
void DrawDirectTextStringClipped( anyCharType *, unsigned char, PixMap *, longRect *, long, long);

void DrawDirectTextHeightx2( anyCharType *, unsigned char, PixMap *, long, long);
void DrawDirectTextStringClippedx2( anyCharType *, unsigned char, PixMap *, longRect *, long, long);

void ResetDirectTextPtr( Handle);

#pragma options align=reset

#endif // ANTARES_DIRECT_TEXT_HPP_

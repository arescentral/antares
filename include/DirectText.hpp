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

#include "AnyChar.h"
#include "Processor.h"
#include "SpriteHandling.h"

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

#define mGetDirectStringDimensionsx2( string, width, height)\
    (height) = gDirectText->height << 1;\
    (width) = ((long)kCharSpace + (long)gDirectText->logicalWidth) * (long)*(string);   // width * length of string

/*
#define mGetDirectStringDimensions( string, width, height)\
    (height) = gDirectText->height;\
    (width) = ((long)kCharSpace + (long)gDirectText->logicalWidth) * (long)*(string);   // width * length of string
*/

#define mGetDirectStringDimensions( mstring, mwidth, mheight, mstrlen, mcharptr, mwidptr)\
    mheight = gDirectText->height;\
    mwidth = 0;\
    mcharptr = (unsigned char *)(mstring);\
    mstrlen = (long)*(mcharptr++);\
    while( mstrlen > 0)\
    {\
        mwidptr = (unsigned char *)*(gDirectText->charSet) + gDirectText->height * gDirectText->physicalWidth * (long)*mcharptr + (long)*mcharptr;\
        mwidth += (long)*mwidptr;\
        mcharptr++;\
        mstrlen--;\
    }\

#define mDirectCharWidth( mwidth, mchar, mwidptr)\
mwidptr = (unsigned char *)*(gDirectText->charSet) + gDirectText->height * gDirectText->physicalWidth * (long)(mchar) + (long)(mchar);\
mwidth = *mwidptr;

#define mSetDirectFont( mwhichFont) gWhichDirectText = mwhichFont;\
gDirectText = (directTextType *)*gDirectTextData + gWhichDirectText;

#define mDirectFontHeight (gDirectText->height)
#define mDirectFontAscent (gDirectText->ascent)
#define mDirectFontLogicalWidth (gDirectText->logicalWidth)

typedef struct directTextStruct
{
    Handle      charSet;
    short       resID;
    Boolean     myHandle;       // different texts can have the same handles; myHandle = TRUE if this guy's in charge of disposing
    long        logicalWidth;
    long        physicalWidth;
    long        height;
    long        ascent;
} directTextType;

int InitDirectText( void);
void DirectTextCleanup( void);
long GetDirectFontNum( short);
short AddDirectFont( directTextType *);

void DrawDirectTextString( char *, unsigned char, PixMap *, long, long);
void DrawDirectTextStringClipped( anyCharType *, unsigned char, PixMap *, longRect *, long, long);
#ifdef zaka
void asm DrawDirectTextStringClipped( anyCharType *, unsigned char, PixMap *, longRect *, long, long);
#endif

void DrawDirectTextHeightx2( anyCharType *, unsigned char, PixMap *, long, long);
#ifdef powercc
void DrawDirectTextStringClippedx2( anyCharType *, unsigned char, PixMap *, longRect *, long, long);
#else
void asm DrawDirectTextStringClippedx2( anyCharType *, unsigned char, PixMap *, longRect *, long, long);
#endif

void ResetDirectTextPtr( Handle);

#pragma options align=reset

#endif // ANTARES_DIRECT_TEXT_HPP_

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

// Direct Text.c -- Ares Interfaces MUST be opened first --

#include "DirectText.hpp"

#include <QDOffscreen.h>

#include "ConditionalMacros.h"
#include "Debug.hpp"
#include "Error.hpp"
#include "OffscreenGWorld.hpp"
#include "Resources.h"

#define kDirectTextError    "\pTEXT"

#define kRowBytesMask       0x8000

#define kFourBitSize        16

extern  GWorldPtr       gSaveWorld;


directTextType      *gDirectText = nil;
long                gWhichDirectText = 0;
TypedHandle<unsigned long> gFourBitTable;  // for turning 4-bit masks into 8-bit masks on the fly
TypedHandle<directTextType> gDirectTextData;

int InitDirectText( void)

{
    Handle          tData = nil;
    unsigned char   i, *c;
    short           count;
    directTextType  *dtext = nil;

    gDirectTextData.create(kDirectFontNum);
    if (gDirectTextData.get() == nil)
    {
        ShowErrorAny( eQuitErr, kErrorStrID, nil, nil, nil, nil, MEMORY_ERROR, -1, -1, -1, __FILE__, 101);
        return( MEMORY_ERROR);
    }

    dtext = *gDirectTextData;
    for  ( count = 0; count < kDirectFontNum; count++)
    {
        dtext->charSet = nil;
        dtext->resID = 0;
        dtext->myHandle = FALSE;
        dtext->logicalWidth = 0;
        dtext->physicalWidth = 0;
        dtext->height = 0;
        dtext->ascent = 0;
        dtext++;
    }

    // add # 0, kTacticalFontNum
    dtext = *gDirectTextData + kTacticalFontNum;
    tData = GetResource( kDTextDescriptResType, kTacticalFontResID);
    if ( tData == nil)
    {
        ShowErrorAny( eQuitErr, kErrorStrID, nil, nil, nil, nil, kCharSetError, -1, -1, -1, __FILE__, kTacticalFontResID);
        return( RESOURCE_ERROR);
    }
    DetachResource( tData);
    MoveHHi( tData);
    HLock( tData);
    BlockMove( *tData, dtext, sizeof( directTextType));
    DisposeHandle( tData);

    gDirectText = dtext;
    gWhichDirectText = 0;
    AddDirectFont( dtext); // trashes this ptr

    // add # 1, kComputerFontNum
    dtext = *gDirectTextData + kComputerFontNum;
    tData = GetResource( kDTextDescriptResType, kComputerFontResID);
    if ( tData == nil)
    {
        ShowErrorAny( eQuitErr, kErrorStrID, nil, nil, nil, nil, kCharSetError, -1, -1, -1, __FILE__, kComputerFontResID);
        return( RESOURCE_ERROR);
    }
    DetachResource( tData);
    MoveHHi( tData);
    HLock( tData);
    BlockMove( *tData, dtext, sizeof( directTextType));
    DisposeHandle( tData);

    AddDirectFont( dtext);

    // add # 2, kButtonFontNum
    dtext = *gDirectTextData + kButtonFontNum;
    tData = GetResource( kDTextDescriptResType, kButtonFontResID);
    if ( tData == nil)
    {
        ShowErrorAny( eQuitErr, kErrorStrID, nil, nil, nil, nil, kCharSetError, -1, -1, -1, __FILE__, kButtonFontResID);
        return( RESOURCE_ERROR);
    }
    DetachResource( tData);
    MoveHHi( tData);
    HLock( tData);
    BlockMove( *tData, dtext, sizeof( directTextType));
    DisposeHandle( tData);

    AddDirectFont( dtext);

    // add # 3, kMessageFontNum
    dtext = *gDirectTextData + kMessageFontNum;
    tData = GetResource( kDTextDescriptResType, kMessageFontResID);
    if ( tData == nil)
    {
        ShowErrorAny( eQuitErr, kErrorStrID, nil, nil, nil, nil, kCharSetError, -1, -1, -1, __FILE__, kMessageFontResID);
        return( RESOURCE_ERROR);
    }
    DetachResource( tData);
    MoveHHi( tData);
    HLock( tData);
    BlockMove( *tData, dtext, sizeof( directTextType));
    DisposeHandle( tData);

    AddDirectFont( dtext);

    // add # 4, kTitleFontNum
    dtext = *gDirectTextData + kTitleFontNum;
    tData = GetResource( kDTextDescriptResType, kTitleFontResID);
    if ( tData == nil)
    {
        ShowErrorAny( eQuitErr, kErrorStrID, nil, nil, nil, nil, kCharSetError, -1, -1, -1, __FILE__, kTitleFontResID);
        return( RESOURCE_ERROR);
    }
    DetachResource( tData);
    MoveHHi( tData);
    HLock( tData);
    BlockMove( *tData, dtext, sizeof( directTextType));
    DisposeHandle( tData);

    AddDirectFont( dtext);

    // add # 5, kButtonSmallFontNum
    dtext = *gDirectTextData + kButtonSmallFontNum;
    tData = GetResource( kDTextDescriptResType, kButtonSmallFontResID);
    if ( tData == nil)
    {
        ShowErrorAny( eQuitErr, kErrorStrID, nil, nil, nil, nil, kCharSetError, -1, -1, -1, __FILE__, kButtonSmallFontResID);
        return( RESOURCE_ERROR);
    }
    DetachResource( tData);
    MoveHHi( tData);
    HLock( tData);
    BlockMove( *tData, dtext, sizeof( directTextType));
    DisposeHandle( tData);

    AddDirectFont( dtext);

    gFourBitTable.create(kFourBitSize);
    if (gFourBitTable.get() == nil)
    {
        ShowErrorAny( eQuitErr, kErrorStrID, nil, nil, nil, nil, MEMORY_ERROR, -1, -1, -1, __FILE__, 2);
        return( MEMORY_ERROR);
    }

    /*
    MoveHHi( gFourBitTable);
    HLock( gFourBitTable);
    */
    WriteDebugLine("\p4BITREG:");
    TypedHandleClearHack(gFourBitTable);

    c = reinterpret_cast<unsigned char*>(*gFourBitTable);
    for ( i = 0; i < kFourBitSize; i++)
    {

        if ( i & 0x08) *(c++) = 0xff;
        else *(c++) = 0x00;

        if ( i & 0x04) *(c++) = 0xff;
        else *(c++) = 0x00;

        if ( i & 0x02) *(c++) = 0xff;
        else *(c++) = 0x00;

        if ( i & 0x01) *(c++) = 0xff;
        else *(c++) = 0x00;

    //  *(c++) = 0xff; *(c++) = 0xff; *(c++) = 0xff; *(c++) = 0xff;
    }

    return( kNoError);
}

void DirectTextCleanup( void)

{
    directTextType  *dtext;
    long            count;

    dtext = *gDirectTextData;
    for  ( count = 0; count < kDirectFontNum; count++)
    {
        if (( dtext->myHandle) && ( dtext->charSet != nil))
            DisposeHandle(reinterpret_cast<Handle>(dtext->charSet));
        dtext++;
    }
    if (gFourBitTable.get() != nil) {
        gFourBitTable.destroy();
    }
    if (gDirectTextData.get() != nil) {
        gDirectTextData.destroy();
    }
}

// GetDirectFontNum:
//  If a direct font of resource ID resID has been loaded, this returns which font has it.
//  If not, it returns -1

long GetDirectFontNum( short resID)

{
    long            count = 0;
    directTextType  *dtext = nil;

    dtext = *gDirectTextData;
    while (( count < kDirectFontNum) && ( dtext->resID != resID))
    {
        count++;
        dtext++;
    }

    if ( count >= kDirectFontNum) return ( -1);
    else return( count);
}

// AddDirectFont:
//  Given a ptr to a directTextType dtext, finds the char data if it's loaded, or loads it if it's
//  not.  TRASHES THE PTR!!!

short AddDirectFont( directTextType *dtext)

{
    directTextType  *dtextWithTable = nil;
    long            whichTable, keepMyID;

    // we remove our ID when we check for other IDs so we don't find our own ID!
    keepMyID = dtext->resID;
    dtext->resID = 0;
    whichTable = GetDirectFontNum( keepMyID);
    dtext->resID = keepMyID;

    if ( whichTable >= 0)
    {
        dtextWithTable = *gDirectTextData + whichTable;
        dtext->charSet = dtextWithTable->charSet;
        dtext->myHandle = FALSE;
    } else
    {
        dtext->charSet = reinterpret_cast<unsigned char**>(GetResource(kDTextFontMapResType, dtext->resID));
        if ( dtext->charSet == nil)
        {
            ShowErrorAny( eQuitErr, kErrorStrID, nil, nil, nil, nil, kCharSetError, -1, -1, -1, __FILE__, dtext->resID);
            return( RESOURCE_ERROR);
        }
        DetachResource(reinterpret_cast<Handle>(dtext->charSet));
        dtext->myHandle = TRUE;
        WriteDebugLine("\pAddCharSet:");
        WriteDebugLong( dtext->resID);
    }
    return( kNoError);
}

void DrawDirectTextString( char *string, unsigned char color, PixMap *destMap, long portLeft,
                            long portTop)

{
    char            *dchar;
    anyCharType     slen;
    long            rowPlus, charPlus;
    int             i, j, width;
    Point           pen;
    unsigned long   *slong, *dlong, colorlong = 0;

    colorlong = color;
    colorlong |= colorlong << 8;
    colorlong |= colorlong << 8;
    colorlong |= colorlong << 8;
    GetPen( &pen);
    slen  = *string++;
    MoveTo( pen.h + implicit_cast<int>(slen) * (gDirectText->logicalWidth + kCharSpace), pen.v);
    pen.v -= gDirectText->ascent;
    rowPlus = (*destMap).rowBytes & 0x3fff;
    charPlus = implicit_cast<long>(kCharSpace) + implicit_cast<long>(gDirectText->logicalWidth);
    dchar = reinterpret_cast<char *>((*destMap).baseAddr) + implicit_cast<long>(pen.v + portTop) * rowPlus + implicit_cast<long>(pen.h) +
            implicit_cast<long>(portLeft << 2);
    width = gDirectText->physicalWidth >> 2;
    rowPlus >>= 2;
    rowPlus -= implicit_cast<long>(width);
    while ( slen > 0)
    {
        slong = reinterpret_cast<unsigned long *>(*(gDirectText->charSet)) + implicit_cast<long>(gDirectText->height) *
                implicit_cast<long>(width) * implicit_cast<long>(*string++);
        dlong = reinterpret_cast<unsigned long *>(dchar);
        for ( j = 0; j < gDirectText->height; j++)
        {
            for ( i = 0; i < width; i++)
            {
                *dlong = (( *dlong | *slong) ^ *slong) | ( colorlong & *slong);
                dlong++;
                slong++;
            }
            dlong += rowPlus;
        }
        dchar += charPlus;
        slen--;
    }
}

void DrawDirectTextStringClipped( anyCharType *string, unsigned char color, PixMap *destMap,
                longRect *clip, long portLeft, long portTop)

{
    unsigned char   *hchar, *dbyte, *sbyte, *tbyte;
    anyCharType     slen;
    long            rowPlus, charPlus = 0, hpos, leftSkip, bytesToDo, width, rowBytes, topEdge,
                    bottomEdge;
    int             i, j, k;
    Point           pen;
    unsigned long   *slong, *dlong, colorlong = 0;

//  DebugStr( string);
//  *string = 4;
    // set up the long color value
    colorlong = color;
    colorlong |= colorlong << 8;
    colorlong |= colorlong << 8;
    colorlong |= colorlong << 8;

    // move the pen to the resulting location
    GetPen( &pen);
    slen  = *string++;
    pen.v -= gDirectText->ascent;
    hpos = pen.h;

    // set the top edge ie the number of rows to skip
    topEdge = 0;
    if ( pen.v < clip->top) topEdge = clip->top - pen.v;

    // set the bottom edge
    bottomEdge = gDirectText->height;
    if (( pen.v + bottomEdge) >= clip->bottom) bottomEdge -= ( pen.v + bottomEdge) - clip->bottom + 1;

    //set up the initial row values
    rowBytes = rowPlus = (*destMap).rowBytes & 0x3fff;
    rowPlus >>= 2L;
    rowPlus -= implicit_cast<long>(gDirectText->physicalWidth << 1L);

    // set hchar = place holder for start of each char we draw
    hchar = reinterpret_cast<unsigned char *>((*destMap).baseAddr) + implicit_cast<long>(pen.v + portTop + topEdge) * rowBytes +
            hpos + implicit_cast<long>(portLeft << 2L);

    // width = character width in pixels
    width = gDirectText->physicalWidth << 3L;

    // while we still have characters to process
    while ( slen > 0)
    {
        // if this character is visible
        if (( hpos < clip->left) || (( hpos + ( width)) >= clip->right))
        {
            // if this character is clipped
            if ((( hpos + (width)) >= clip->left) || ( hpos < clip->right))
            {
                // leftSkip = # of pixels to skip on left edge
                leftSkip = 0;
                if ( hpos < clip->left) leftSkip = clip->left - hpos;

                // bytesToDo = right edge clip (from 0)
                bytesToDo = width;
                if (( hpos + (width)) >= clip->right)
                    bytesToDo -= hpos + (width) - clip->right;

                // sbyte = source byte
                sbyte = *(gDirectText->charSet) + gDirectText->height *
                        gDirectText->physicalWidth * implicit_cast<long>(*string) + implicit_cast<long>(*string);
                string++;

                // charPlus = width of this character
                charPlus = implicit_cast<long>(*(sbyte++));

                // skip over the clipped top rows
                sbyte += topEdge * gDirectText->physicalWidth;

                // dbyte = destination pixel
                dbyte = hchar;

                // repeat for every unclipped row
                for ( j = topEdge; j < bottomEdge; j++)
                {
                    // k = this h position
                    k = 0;

                    // repeat for every byte of data
                    for ( i = 0; i < gDirectText->physicalWidth; i++)
                    {
                        // really table + ((*sbyte >> 4) << 2) -- look up byte value for bit mask
                        tbyte = reinterpret_cast<unsigned char*>(*gFourBitTable) + implicit_cast<long>(( (*sbyte) >> 2L) & 0x3c);

                        // for each of four bytes for this half of the source byte:
                        // make sure exists & is within left & right bounds
                        // increase h counter (k) and destByte
                        if ( (*(tbyte++)) && ( k >= leftSkip) && ( k < bytesToDo)) *dbyte = color;
                        dbyte++;
                        k++;
                        if ( (*(tbyte++)) && ( k >= leftSkip) && ( k < bytesToDo)) *dbyte = color;
                        dbyte++;
                        k++;
                        if ( (*(tbyte++)) && ( k > leftSkip) && ( k < bytesToDo)) *dbyte = color;
                        dbyte++;
                        k++;
                        if ( (*(tbyte++)) && ( k > leftSkip) && ( k < bytesToDo)) *dbyte = color;
                        dbyte++;
                        k++;

                        tbyte = reinterpret_cast<unsigned char*>(*gFourBitTable) + implicit_cast<long>(( (*(sbyte++)) & 0x0f) << 2L);
                        if ( (*(tbyte++)) && ( k >= leftSkip) && ( k < bytesToDo)) *dbyte = color;
                        dbyte++;
                        k++;
                        if ( (*(tbyte++)) && ( k >= leftSkip) && ( k < bytesToDo)) *dbyte = color;
                        dbyte++;
                        k++;
                        if ( (*(tbyte++)) && ( k >= leftSkip) && ( k < bytesToDo)) *dbyte = color;
                        dbyte++;
                        k++;
                        if ( (*(tbyte++)) && ( k >= leftSkip) && ( k < bytesToDo)) *dbyte = color;
                        dbyte++;
                        k++;
                    }
                    // add row to dest byte
                    dbyte += rowBytes - (width);;
                }
            // else (not on screen) just increase the current character
            } else string++;
        } else // not clipped, draw in long words
        {
            // get source byte
            sbyte = *(gDirectText->charSet) + implicit_cast<long>(gDirectText->height) *
                    implicit_cast<long>(gDirectText->physicalWidth) * implicit_cast<long>(*string) + implicit_cast<long>(*string);
            string++;

            // width of this char in pixels
            charPlus = *(sbyte++);

            // skip clipped rows
            sbyte += topEdge * gDirectText->physicalWidth;

            // dlong = destination byte
            dlong = reinterpret_cast<unsigned long *>(hchar);
            for ( j = topEdge; j < bottomEdge; j++)
            {
                for ( i = 0; i < gDirectText->physicalWidth; i++)
                {
                    // get ptr in bit>byte table
                    slong = *gFourBitTable + implicit_cast<long>( (*sbyte) >> 4L);
                    *dlong = (( *dlong | *slong) ^ *slong) | ( colorlong & *slong);
                    dlong++;

                    // for snd half of word
                    slong = *gFourBitTable + implicit_cast<long>( (*(sbyte++)) & 0x0f);
                    *dlong = (( *dlong | *slong) ^ *slong) | ( colorlong & *slong);
                    dlong++;
                }

                // destByte += rowPlus
                dlong += rowPlus;
            }
        }
        // for every char clipped or no:
        // increase our character pixel starting point by width of this character
        hchar += charPlus;

        // increase our hposition (our position in pixels)
        hpos += charPlus;

        // decrease our length counter
        slen--;
    }
    MoveTo( hpos, pen.v + gDirectText->ascent);
}

void DrawDirectTextHeightx2( anyCharType *string, unsigned char color, PixMap *destMap, long portLeft,
                            long portTop)

{
    char            *dchar;
    anyCharType     slen;
    long            rowPlus, charPlus, rowBytes;
    int             i, j, width;
    Point           pen;
    unsigned long   *slong, *dlong, colorlong = 0;

    colorlong = color;
    colorlong |= colorlong << 8;
    colorlong |= colorlong << 8;
    colorlong |= colorlong << 8;
    GetPen( &pen);
    slen  = *string++;
    MoveTo( pen.h + implicit_cast<int>(slen) * (gDirectText->logicalWidth + kCharSpace), pen.v);
    pen.v -= gDirectText->ascent << 1;
    rowPlus = rowBytes = (*destMap).rowBytes & 0x3fff;
    charPlus = implicit_cast<long>(kCharSpace) + implicit_cast<long>(gDirectText->logicalWidth);
    dchar = reinterpret_cast<char *>((*destMap).baseAddr) + implicit_cast<long>(pen.v + portTop) * rowPlus + implicit_cast<long>(pen.h) +
            implicit_cast<long>(portLeft << 2);
    width = gDirectText->physicalWidth >> 2;
    rowPlus >>= 2;
    rowBytes >>= 2;
    rowPlus -= implicit_cast<long>(width);
    while ( slen > 0)
    {
        slong = reinterpret_cast<unsigned long*>(*(gDirectText->charSet)) + implicit_cast<long>(gDirectText->height) *
                implicit_cast<long>(width) * implicit_cast<long>(*string++);
        dlong = reinterpret_cast<unsigned long *>(dchar);
        for ( j = 0; j < gDirectText->height; j++)
        {
            for ( i = 0; i < width; i++)
            {
                *dlong = (( *dlong | *slong) ^ *slong) | ( colorlong & *slong);
                dlong++;
                slong++;
            }
            dlong += rowPlus + rowBytes;
        }
        dchar += charPlus;
        slen--;
    }
}

void DrawDirectTextStringClippedx2( anyCharType *string, unsigned char color, PixMap *destMap,
                longRect *clip, long portLeft, long portTop)
{
    static_cast<void>(string);
    static_cast<void>(color);
    static_cast<void>(destMap);
    static_cast<void>(clip);
    static_cast<void>(portLeft);
    static_cast<void>(portTop);
/*  unsigned char   *hchar, *dbyte, *sbyte;
    anyCharType     slen;
    long            rowPlus, charPlus, hpos, leftSkip, bytesToDo, width, rowBytes, topEdge,
                    bottomEdge;
    int             i, j;
    Point           pen;
    unsigned long   *slong, *dlong, colorlong = 0, dvalue;

    colorlong = color;
    colorlong |= colorlong << 8;
    colorlong |= colorlong << 8;
    colorlong |= colorlong << 8;
    GetPen( &pen);
    slen  = *string++;
    MoveTo( pen.h + (int)slen * (gDirectText->logicalWidth + kCharSpace), pen.v);
    pen.v -= gDirectText->ascent << 1;
    hpos = pen.h;

    topEdge = 0;
    if ( pen.v < clip->top) topEdge = clip->top - pen.v;
    bottomEdge = gDirectText->height << 1;
    if (( pen.v + bottomEdge) >= clip->bottom) bottomEdge -= ( pen.v + bottomEdge) - clip->bottom + 1;

    rowBytes = rowPlus = (*destMap).rowBytes & 0x3fff;
    charPlus = (long)(kCharSpace) + (long)(gDirectText->logicalWidth);
    hchar = (unsigned char *)(*destMap).baseAddr + (long)(pen.v + portTop + topEdge) * rowPlus +
            hpos + (long)(portLeft << 2);
    width = gDirectText->physicalWidth >> 2;
    rowPlus >>= 2;
    rowPlus -= (long)width;

    while ( slen > 0)
    {
        if (( hpos < clip->left) || (( hpos + gDirectText->physicalWidth) >= clip->right))
        {
            if ((( hpos + gDirectText->physicalWidth - 1) >= clip->left) || ( hpos < clip->right))
            {
                leftSkip = 0;
                if ( hpos < clip->left) leftSkip = clip->left - hpos;
                bytesToDo = 0;
                if (( hpos + gDirectText->physicalWidth) >= clip->right)
                    bytesToDo = hpos + gDirectText->physicalWidth - clip->right + 1;
                bytesToDo = gDirectText->physicalWidth - ( leftSkip + bytesToDo);
                sbyte = (unsigned char *)*(gDirectText->charSet) + gDirectText->height *
                        gDirectText->physicalWidth * (long)*string + (long)*string++;
                charPlus = (long)*(sbyte++);

                sbyte += topEdge * gDirectText->physicalWidth + leftSkip;

                dbyte = hchar + leftSkip;

                for ( j = topEdge; j < bottomEdge; j += 2)
                {
                    for ( i = 0; i < bytesToDo; i++)
                    {
                        if ( *sbyte) *dbyte = color;
                        sbyte++;
                        dbyte++;
                    }
                    dbyte += rowBytes - bytesToDo + rowBytes;
                    sbyte += gDirectText->physicalWidth - bytesToDo;
                }
            } else string++;
        } else
        {
            sbyte = (unsigned char *)*(gDirectText->charSet) + (long)gDirectText->height *
                    (long)gDirectText->physicalWidth * (long)*string + (long)*string++;
            charPlus = (long)*(sbyte++);

            slong = (unsigned long *)sbyte + topEdge * width;

            dlong = (unsigned long *)hchar;
            for ( j = topEdge; j < bottomEdge; j += 2)
            {
                for ( i = 0; i < width; i++)
                {
                    *dlong = (( *dlong | *slong) ^ *slong) | ( colorlong & *slong);
                    dlong++;
                    slong++;
                }
                dlong += rowPlus + (rowBytes >> 2);
            }
        }
        hchar += charPlus;
        hpos += charPlus;
        slen--;
    }
*/}

void ResetDirectTextPtr( Handle directText)

{
#pragma unused( directText)
    WriteDebugLine("\pDText Callback");
    gDirectText = *gDirectTextData + gWhichDirectText;
}

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

#include "DirectText.h"

#include <QDOffscreen.h>

#include "ConditionalMacros.h"
#include "Debug.h"
#include "Error.h"
#include "HandleHandling.h"
#include "OffscreenGWorld.h"
#include "Processor.h"
#include "Resources.h"

#define kDirectTextError    "\pTEXT"

#define kRowBytesMask       0x8000

#define kFourBitSize        16

extern  GWorldPtr       gSaveWorld;


directTextType      *gDirectText = nil;
long                gWhichDirectText = 0;
Handle              gFourBitTable = nil;        // for turning 4-bit masks into 8-bit masks on the fly
Handle              gDirectTextData = nil;

int InitDirectText( void)

{
    Handle          tData = nil;
    unsigned char   i, *c;
    short           count;
    directTextType  *dtext = nil;

    HHMaxMem();
    gDirectTextData = NewHandle( sizeof( directTextType) * kDirectFontNum);
    if ( gDirectTextData == nil)
    {
        ShowErrorAny( eQuitErr, kErrorStrID, nil, nil, nil, nil, MEMORY_ERROR, -1, -1, -1, __FILE__, 101);
        return( MEMORY_ERROR);
    }
    // we can't move this handle since we want to keep ptrs to the character set handles
    MoveHHi( gDirectTextData);
    HLock( gDirectTextData);

    dtext = (directTextType *)*gDirectTextData;
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
    dtext = (directTextType *)*gDirectTextData + kTacticalFontNum;
    tData = HHGetResource( kDTextDescriptResType, kTacticalFontResID);
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
    dtext = (directTextType *)*gDirectTextData + kComputerFontNum;
    tData = HHGetResource( kDTextDescriptResType, kComputerFontResID);
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
    dtext = (directTextType *)*gDirectTextData + kButtonFontNum;
    tData = HHGetResource( kDTextDescriptResType, kButtonFontResID);
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
    dtext = (directTextType *)*gDirectTextData + kMessageFontNum;
    tData = HHGetResource( kDTextDescriptResType, kMessageFontResID);
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
    dtext = (directTextType *)*gDirectTextData + kTitleFontNum;
    tData = HHGetResource( kDTextDescriptResType, kTitleFontResID);
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
    dtext = (directTextType *)*gDirectTextData + kButtonSmallFontNum;
    tData = HHGetResource( kDTextDescriptResType, kButtonSmallFontResID);
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

    gFourBitTable = NewHandle( sizeof( unsigned char) * 4L * (long)kFourBitSize);
    if ( gFourBitTable == nil)
    {
        ShowErrorAny( eQuitErr, kErrorStrID, nil, nil, nil, nil, MEMORY_ERROR, -1, -1, -1, __FILE__, 2);
        return( MEMORY_ERROR);
    }

    /*
    MoveHHi( gFourBitTable);
    HLock( gFourBitTable);
    */
    WriteDebugLine((char *)"\p4BITREG:");
    mHandleLockAndRegister( gFourBitTable, nil, nil, nil, "\pgFourBitTable")

    c = (unsigned char *)*gFourBitTable;
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

    dtext = (directTextType *)*gDirectTextData;
    for  ( count = 0; count < kDirectFontNum; count++)
    {
        if (( dtext->myHandle) && ( dtext->charSet != nil))
            DisposeHandle( dtext->charSet);
        dtext++;
    }
    if ( gFourBitTable != nil)
        DisposeHandle( gFourBitTable);
    if ( gDirectTextData != nil)
        DisposeHandle( gDirectTextData);
}

// GetDirectFontNum:
//  If a direct font of resource ID resID has been loaded, this returns which font has it.
//  If not, it returns -1

long GetDirectFontNum( short resID)

{
    long            count = 0;
    directTextType  *dtext = nil;

    dtext = (directTextType *)*gDirectTextData;
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
        dtextWithTable = (directTextType *)*gDirectTextData + whichTable;
        dtext->charSet = dtextWithTable->charSet;
        dtext->myHandle = FALSE;
    } else
    {
        dtext->charSet = HHGetResource( kDTextFontMapResType, (short)dtext->resID);
        if ( dtext->charSet == nil)
        {
            ShowErrorAny( eQuitErr, kErrorStrID, nil, nil, nil, nil, kCharSetError, -1, -1, -1, __FILE__, dtext->resID);
            return( RESOURCE_ERROR);
        }
        DetachResource( dtext->charSet);
        dtext->myHandle = TRUE;
        WriteDebugLine((char *)"\pAddCharSet:");
        WriteDebugLong( dtext->resID);
        mDataHandleLockAndRegister( dtext->charSet, nil, nil, nil, "\pdtext->charset") // this can move memory, so our ptr's no good
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
    MoveTo( pen.h + (int)slen * (gDirectText->logicalWidth + kCharSpace), pen.v);
    pen.v -= gDirectText->ascent;
    rowPlus = (*destMap).rowBytes & 0x3fff;
    charPlus = (long)(kCharSpace) + (long)(gDirectText->logicalWidth);
    dchar = (char *)(*destMap).baseAddr + (long)(pen.v + portTop) * rowPlus + (long)(pen.h) +
            (long)(portLeft << 2);
    width = gDirectText->physicalWidth >> 2;
    rowPlus >>= 2;
    rowPlus -= (long)width;
    while ( slen > 0)
    {
        slong = (unsigned long *)*(gDirectText->charSet) + (long)gDirectText->height *
                (long)width * (long)*string++;
        dlong = (unsigned long *)dchar;
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
    long            rowPlus, charPlus, hpos, leftSkip, bytesToDo, width, rowBytes, topEdge,
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
    rowPlus -= (long)(gDirectText->physicalWidth << 1L);

    // set hchar = place holder for start of each char we draw
    hchar = (unsigned char *)(*destMap).baseAddr + (long)(pen.v + portTop + topEdge) * rowBytes +
            hpos + (long)(portLeft << 2L);

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
                sbyte = (unsigned char *)*(gDirectText->charSet) + gDirectText->height *
                        gDirectText->physicalWidth * (long)*string + (long)*string;
                string++;

                // charPlus = width of this character
                charPlus = (long)*(sbyte++);

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
                        tbyte = (unsigned char *)*gFourBitTable + (long)(( (*sbyte) >> 2L) & 0x3c);

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

                        tbyte = (unsigned char *)*gFourBitTable + (long)(( (*(sbyte++)) & 0x0f) << 2L);
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
            sbyte = (unsigned char *)*(gDirectText->charSet) + (long)gDirectText->height *
                    (long)gDirectText->physicalWidth * (long)*string + (long)*string;
            string++;

            // width of this char in pixels
            charPlus = (long)*(sbyte++);

            // skip clipped rows
            sbyte += topEdge * gDirectText->physicalWidth;

            // dlong = destination byte
            dlong = (unsigned long *)hchar;
            for ( j = topEdge; j < bottomEdge; j++)
            {
                for ( i = 0; i < gDirectText->physicalWidth; i++)
                {
                    // get ptr in bit>byte table
                    slong = (unsigned long *)*gFourBitTable + (long)( (*sbyte) >> 4L);
                    *dlong = (( *dlong | *slong) ^ *slong) | ( colorlong & *slong);
                    dlong++;

                    // for snd half of word
                    slong = (unsigned long *)*gFourBitTable + (long)( (*(sbyte++)) & 0x0f);
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

#ifdef zaraka
void asm DrawDirectTextStringClipped( anyCharType *string, unsigned char color, PixMap *destMap,
                longRect *clip, long portLeft, long portTop)

{
#ifdef kAllowAssem
    register long   dr3, dr4, dr5, dr6, dr7;
    register long   *ar2, *ar3, *ar4;

    long            rowPlus, charPlus, hpos, leftSkip, bytesToDo, rowBytes, topEdge,
                    bottomEdge;
    long            count;
    Point           pen;
    unsigned long   colorlong;
    long            slen, *hchar;

        fralloc +

        // Get the current pen

        pea             pen;
        _GetPen                                 // GetPen( &pen)

        // get slen, and increment string

        movea.l         string, ar4;                // ar4 = string
        addq.l          #1, string;                 // string++
        move.b          (ar4), dr3;                 // slen = (ar4)
        ext.l           dr3;
        move.l          dr3, slen;

        // Move the real pen to its final position (we're done with it)

        clr.l           dr3;                        // dr3 = 0x00000000
        move.b          (ar4), dr3;                 // dr3 = (ar4) (slen)
        ext.w           dr3;
        move.l          gDirectText->logicalWidth, dr5; // dr5 = physicalWidth
        mulu.w          dr5, dr3;                   // dr3 *= dr5 (physicalWidth)
        move.w          pen.h, dr5;                 // dr5 = pen.h
        add.w           dr5, dr3;                   // dr3 += dr5(pen.h)
        move.w          dr3, -(a7);
        move.w          pen.v, -(a7);
        _MoveTo;                                    // MoveTo( dr3, pen.v)

        // create the four-byte color

        clr.l           dr3;                        // dr3 = 0x00000000
        move.b          color, dr3;                 // dr3 = color
        move.b          dr3, dr7;                   // dr7 = dr3
        lsl.l           #8, dr7;                    // dr7 <<= 8
        or.l            dr3, dr7;                   // dr7 |= dr3
        lsl.l           #8, dr7;                    // dr7 <<= 8
        or.l            dr3, dr7;                   // dr7 |= dr3
        lsl.l           #8, dr7;                    // dr7 <<= 8
        or.l            dr3, dr7;                   // dr7 |= dr3
        move.l          dr7, colorlong;

        // move pen.v to the actual top of where we'll be drawing

        move.l          gDirectText->ascent, dr3;   // dr3 = gDirectText->ascent
        sub.w           dr3, pen.v;                 // pen.v -= dr3

        // set hpos to left edge

        move.w          pen.h, dr3;                 // dr3 = pen.h
        ext.l           dr3;
        move.l          dr3, hpos;                  // hpos = dr3

        // clip the top edge if needed

        clr.l           dr3;                        // dr3 = 0x00000000
        move.w          pen.v, dr5;                 // dr3 = pen.v
        ext.l           dr5;
        movea.l         clip, ar4;                  // ar4 = clip
        cmp.l           struct(longRectStruct.top)(ar4), dr5;   // if dr3 >= (ar4)(clip)->top
        bge             cliptopdone;                // then don't clip

        // clip

        move.l          struct(longRectStruct.top), dr3;    // dr3 = (ar4)(clip)->top
        move.w          pen.v, dr5;                 // dr5 = pen.v
        ext.l           dr5;
        sub.l           dr5, dr3;                   // dr3 -= dr5

    cliptopdone:

        move.l          dr3, topEdge;               // topEdge = dr3

        // clip the bottom edge if needed

        move.l          gDirectText->height, dr3;   // dr3 = gDirectText->height
        move.l          dr3, dr5;                   // dr5 = dr3
        move.w          pen.v, dr4;                 // dr4 = pen.v
        ext.l           dr4;
        add.l           dr4, dr5;                   // dr5 += dr4(pen.v)
        cmp.l           struct(longRectStruct.bottom)(ar4), dr5; // if dr5 < (ar4)(clip)->bottom
        blt             clipbottomdone;             // then don't clip

        // clip

        sub.l           struct(longRectStruct.bottom)(ar4), dr5;    // dr5 (bottomEdge + pen.v) -= (ar4)(clip)->bottom
        add.l           #1, dr5;                    // dr5 += 1
        sub.l           dr5, dr3;                   // dr3(bottomEdge) -= dr5

    clipbottomdone:

        move.l          dr3, bottomEdge;            // bottomEdge = dr3
        movea.l         destMap, ar2;               // ar2 = destMap
        move.w          struct(PixMap.rowBytes)(ar2), dr3;              // dr3 = (ar2)(destMap)->rowBytes
        andi.l          #0x00003fff, dr3;           // dr3 &= 0x3fff (mask out pixMap bit)
        move.l          dr3, rowBytes;              // rowBytes = dr3
        move.l          dr3, rowPlus;               // rowPlus = dr3

        movea.l         (ar2), ar2;                 // ar2 = (ar2)(destMap)->baseAddr
        move.w          pen.v, dr3;                 // dr3 = pen.v
        ext.l           dr3;
        add.l           portTop, dr3;               // dr3 += portTop
        add.l           topEdge, dr3;               // dr3 += topEdge
        move.l          rowBytes, dr5;
        mulu.w          dr5, dr3;                   // dr3 *= rowBytes
        adda.l          dr3, ar2;                   // ar2 (hchar) += dr3
        adda.l          hpos, ar2;                  // ar2 += hpos
        move.l          portLeft, dr3;              // dr3 = portLeft
        lsl.l           #2, dr3;                    // dr3 *= 4
        adda.l          dr3, ar2;                   // ar2 += (portLeft * 4)
        move.l          ar2, hchar;

        move.l          gDirectText->physicalWidth, dr6; // dr6 = gDirectText->physicalWidth
        move.l          dr6, dr3
        asl.l           #3, dr3
        move.l          rowPlus, dr5;               // dr5 = rowPlus
        sub.l           dr3, dr5;

        move.l          bottomEdge, dr3;
        sub.l           topEdge, dr3;
        add.l           #1, dr3;
        move.l          dr3, bottomEdge;

        move.l          topEdge, dr3;
        muls.w          dr6, dr3;
        move.l          dr3, topEdge;

        move.l          gDirectText->height, dr3;
        muls.w          dr3, dr6;

        move.l          slen, dr3;              // dr4 = slen

        bra             stringLoopCheck;

/*
        ar4 = clipRect
        ar2 = hchar
        ar3 =
        ar4 =

        dr3 =
        DRX =
        dr4 = slen
        dr5 = rowPlus
        dr6 = width * height
        dr7 =
*/

        // Prepare for this character

    stringLoopTop:

        move.l      dr3, slen;

        movea.l     clip, ar4;
        move.l      (ar4), dr3;         // dr3 = clip->left
        cmp.l       hpos, dr3;          // if dr3 >= hpos
        bge         drawClippedChar;        // then draw clipped character

        move.l      0x08(ar4), dr3;     // dr3 = clip->right
        move.l      hpos, dr4;          // dr4 = hpos
        add.l       gDirectText->physicalWidth, dr4;
        cmp.l       dr3, dr4;               // if dr4 >= clip->right
        bge         drawClippedChar;        // then draw clipped character

        bra         drawUnclippedChar;

        // character is horiontally clipped; draw it one byte at a time

        // make sure it's not offscreen to the left

    drawClippedChar:

        jmp dontDrawChar;

        move.l      hpos, dr4;          // dr4 = hpos
        add.l       gDirectText->physicalWidth, dr4;
        sub.l       #1, dr4;
        move.l      (ar4), dr3;         // dr3 = clip->left
        cmp.l       dr3, dr4;               // if dr4 (hpos + physicalWidth) < dr3 (clip->left)
        blt         dontDrawChar;       // then char is not at all onscreen

        // make sure it's not offscreen to the right

        move.l      0x08(ar4), dr3;     // dr3 = clip->right/
        cmp.l       hpos, dr3;          // if dr3(clip->right) < hpos
        ble         dontDrawChar;       // then char is not at all on screen

        // prepare to draw the clipped character

        // clip the left edge if needed

        clr.l       dr4;                // dr4 (leftSkip) = 0
        move.l      (ar4), dr3;         // dr3 = clip->left
        cmp.l       hpos, dr3;          // if dr3(clip->left) < hpos
        ble         dontClipLeft;       // then don't clip left

        // clip

        move.l      dr3, dr4;               // dr4 (leftSkip) = dr3(clip->left)
        sub.l       hpos, dr4;          // dr4 -= hpos

    dontClipLeft:

        move.l      dr4, leftSkip;      // leftSkip = dr4

        // clip the right edge if needed

        clr.l       dr4;                    // dr4 (bytesToDo) = 0
        move.l      hpos, dr3;          // dr3 = hpos
        add.l       gDirectText->physicalWidth, dr3;    // dr3 += physicalWidth
        cmp.l       0x08(ar4), dr3;     // if dr3 (hpos + physicalWidth) < (ar4)(clipRect)->right
        blt         dontClipRight;      // then dont clip right

        // clip

        move.l      dr3, dr4;               // dr4(bytesToDo) = dr3(hpos + physicalWidth)
        sub.l       0x08(ar4), dr4;     // dr4(bytesToDo) -= (ar4)(clipRect)->right
        addq.l      #1, dr4;                // dr4 += 1 (exclusive of border)

    dontClipRight:

        move.l      gDirectText->physicalWidth, dr3;    // dr3 = physicalWidth
        sub.l       leftSkip, dr3;      // dr3 -= leftSkip
        sub.l       dr4, dr3;               // dr3 -= dr4(bytesToDo)
        move.l      dr3, bytesToDo;     // bytesToDo = dr3

        // prepare to draw clipped char

        movea.l     gDirectText->charSet, ar3;  // ar3 = charSet
        movea.l     (ar3), ar3;         // ar3 = (ar3) (dereference handle) // ar3 = sbyte
//      move.l      gDirectText->height, dr3;   // dr3 = gDirectText->height
//      muls.w      dr6, dr3;               // dr3(height) *= dr6(width)
        move.l      dr6, dr3;
        movea.l     string, ar4;            // ar4 = string
        add.l       #1, string;         // string++
        clr.l       dr4;                    // dr4 = 0x00000000
        move.b      (ar4), dr4;         // dr4 = (ar4)(previous *string)
        muls.w      dr4, dr3;               // dr3(height * width) *= dr4
        adda.l      dr3, ar3;               // ar3 (slong) += dr3
        adda.l      topEdge, ar3;               // ar3(sbyte) += dr3
        adda.l      leftSkip, ar3;      // ar3(sbyte) += leftSkip

        movea.l     hchar, ar4;             // ar4(dbyte) = ar2(hchar)
        adda.l      leftSkip, ar4;

        move.l      bottomEdge, dr3;        // dr3 = bottomEdge

        bra         clipCharVCheck;

    clipCharVLoop:

        move.l      bytesToDo, dr4;     // dr4(i) = bytesToDo
        bra         clipCharHCheck;     // jump to end of loop

    clipCharHLoop:

        tst.b       (ar3);              // if (ar3)(*sbyte) == 0
        beq         clipCharNoByte;     // don't draw byte

        move.b      color, (ar4);       // else *dbyte = *sbyte

    clipCharNoByte:

        adda.l      #1, ar3;
        adda.l      #1, ar4;

    clipCharHCheck:

        dbra        dr4, clipCharHLoop;

        adda.l      rowBytes, ar4;
        suba.l      bytesToDo, ar4;
        adda.l      rowBytes, ar4;
        adda.l      gDirectText->physicalWidth, ar3;
        suba.l      bytesToDo, ar3;

    clipCharVCheck:

        dbra        dr3, clipCharVLoop;

        bra         stringLoopEnd;

        // character is offscreen; just increment string

    dontDrawChar:

        add.l       #1, string;         // string++
        bra         stringLoopEnd;      // skip to loop check

        // draw a fully horizontally visible character in longwords

    drawUnclippedChar:

        movea.l     gDirectText->charSet, ar3;  // ar3
        movea.l     (ar3), ar3;         // ar3 = (ar3) (dereference handle) // ar3 = slong
        move.l      dr6, dr3;
        movea.l     string, ar4;        // ar4 = string
        clr.l       dr4;                    // dr4 = 0x00000000
        move.b      (ar4), dr4;         // dr4 = (ar4)(previous *string)
        muls.w      dr4, dr3;               // dr3(height * width) *= dr4
        adda.l      dr3, ar3;               // ar3 (slong) += dr3
        move.b      (ar4), dr3;
        ext.l       dr3
        adda.l      dr3, ar3

        add.l       #1, string;         // string++

        move.b      (ar3), dr3;
        ext.l       dr3;
        move.l      dr3, charPlus
        adda.l      #1, ar3;

        adda.l      topEdge, ar3;               // ar3(slong) += dr3
        movea.l     hchar, ar4;             // ar4(dlong) = ar2(hchar)

        move.l      bottomEdge, dr3;        // dr3 = bottomEdge
        add.l       #1, dr3;
        move.l      dr3, count;             // count = dr3

        bra         unclipCharVCheck;   // skip to loop check

    unclipCharVLoop:

        move.l      gDirectText->physicalWidth, dr4;                // dr4 = dr6(width)

        bra         unclipCharHCheck;   // skip to loop check

    unclipCharHLoop:

        movea.l     gFourBitTable, ar2;
        movea.l     (ar2), ar2;
        moveq       #0, dr7;
        move.b      (ar3), dr7;
        asr.b       #2, dr7;
        and.b       #0x3c, dr7;
        adda.l      dr7, ar2;
        move.l      (ar2), dr7;
        move.l      (ar4), dr3;         // dr3 = (ar4)(*dlong)
        or.l        dr7, dr3;               // dr3 |= dr7 (*slong)
        eor.l       dr7, dr3;               // dr3 ^= dr7 (*slong)
        and.l       colorlong, dr7;     // dr7 &= colorlong
        or.l        dr7, dr3;               // dr3 |= dr7 (colorlong & *slong)
        move.l      dr3, (ar4)+;        // (ar4)+(*dlong++) = dr3

        movea.l     gFourBitTable, ar2;
        movea.l     (ar2), ar2;
        moveq       #0, dr7;
        move.b      (ar3)+, dr7;
        and.b       #0x0f, dr7;
        asl.l       #2, dr7;
        adda.l      dr7, ar2;
        move.l      (ar2), dr7;
        move.l      (ar4), dr3;         // dr3 = (ar4)(*dlong)
        or.l        dr7, dr3;               // dr3 |= dr7 (*slong)
        eor.l       dr7, dr3;               // dr3 ^= dr7 (*slong)
        and.l       colorlong, dr7;     // dr7 &= colorlong
        or.l        dr7, dr3;               // dr3 |= dr7 (colorlong & *slong)
        move.l      dr3, (ar4)+;        // (ar4)+(*dlong++) = dr3

    unclipCharHCheck:

        dbra dr4, unclipCharHLoop;      // dr4(i)--; if i >= 0 repeat loop

        adda.l      dr5, ar4;               // ar3(dlong) += rowPlus

    unclipCharVCheck:

        move.l      count, dr3;             // dr3 = count
        subq        #1, dr3;                // dr3 -= 1
        move.l      dr3, count;             // count = dr3
        tst.l       dr3;                // if dr3 >= 0
        bne         unclipCharVLoop;        // then repeat loop


    stringLoopEnd:

        move.l      hchar, ar2;
        adda.l      charPlus, ar2;      // ar2 (hchar) += charPlus
        move.l      ar2, hchar;
        move.l      charPlus, dr3;      // dr3 = charPlus
        add.l       dr3, hpos;          // hpos += charPlus

    stringLoopCheck:

        move.l      slen, dr3;
        dbra        dr3, stringLoopTop; // dr4 (slen)--; if slen >= 0 repeat loop

        frfree;
#endif //kAllowAssem
        rts;
}

#endif

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
    MoveTo( pen.h + (int)slen * (gDirectText->logicalWidth + kCharSpace), pen.v);
    pen.v -= gDirectText->ascent << 1;
    rowPlus = rowBytes = (*destMap).rowBytes & 0x3fff;
    charPlus = (long)(kCharSpace) + (long)(gDirectText->logicalWidth);
    dchar = (char *)(*destMap).baseAddr + (long)(pen.v + portTop) * rowPlus + (long)(pen.h) +
            (long)(portLeft << 2);
    width = gDirectText->physicalWidth >> 2;
    rowPlus >>= 2;
    rowBytes >>= 2;
    rowPlus -= (long)width;
    while ( slen > 0)
    {
        slong = (unsigned long *)*(gDirectText->charSet) + (long)gDirectText->height *
                (long)width * (long)*string++;
        dlong = (unsigned long *)dchar;
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

#ifdef kDontDoLong68KAssem

void DrawDirectTextStringClippedx2( anyCharType *string, unsigned char color, PixMap *destMap,
                longRect *clip, long portLeft, long portTop)

{
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

#else

// NOTE: THIS VERSION ASSUMES TYPE anyCharType IS ONE BYTE IN LENGTH! >> NOT WORLD-READY! <<

void asm DrawDirectTextStringClippedx2( anyCharType *string, unsigned char color, PixMap *destMap,
                longRect *clip, long portLeft, long portTop)

{
#ifdef kAllowAssem
/*  register long   dr3, dr4, dr5, dr6, dr7;
    register long   *ar2, *ar3, *ar4;

    long            rowPlus, charPlus, hpos, leftSkip, bytesToDo, rowBytes, topEdge,
                    bottomEdge;
    long            count;
    Point           pen;
    unsigned long   colorlong;
    long            slen;

        fralloc +

        // Get the current pen

        pea             pen;
        _GetPen                                 // GetPen( &pen)

        // get slen, and increment string

        movea.l         string, ar4;                // ar4 = string
        addq.l          #1, string;             // string++
        move.b          (ar4), dr3;             // slen = (ar4)
        ext.l           dr3;
        move.l          dr3, slen;

        // Move the real pen to its final position (we're done with it)

        clr.l           dr3;                        // dr3 = 0x00000000
        move.b          (ar4), dr3;                 // dr3 = (ar4) (slen)
        ext.w           dr3;
        move.l          struct(directTextStruct.logicalWidth)(gDirectText), dr5; // dr5 = physicalWidth
//      add.l           #kCharSpace, dr5;           // commented out b/c kCharSpace = 0
        mulu.w          dr5, dr3;                   // dr3 *= dr5 (physicalWidth)
        move.w          pen.h, dr5;             // dr5 = pen.h
        add.w           dr5, dr3;                   // dr3 += dr5(pen.h)
        move.w          dr3, -(a7);
        move.w          pen.v, -(a7);
        _MoveTo;                                    // MoveTo( dr3, pen.v)

        // create the four-byte color

        clr.l           dr3;                        // dr3 = 0x00000000
        move.b          color, dr3;             // dr3 = color
        move.b          dr3, dr7;                   // dr7 = dr3
        lsl.l           #8, dr7;                    // dr7 <<= 8
        or.l            dr3, dr7;                   // dr7 |= dr3
        lsl.l           #8, dr7;                    // dr7 <<= 8
        or.l            dr3, dr7;                   // dr7 |= dr3
        lsl.l           #8, dr7;                    // dr7 <<= 8
        or.l            dr3, dr7;                   // dr7 |= dr3
        move.l          dr7, colorlong;

        // move pen.v to the actual top of where we'll be drawing

        move.l          gDirectText->ascent, dr3;   // dr3 = gDirectText->ascent
        add.l           dr3, dr3;                   // dr3 *= 2 (double the height)
        sub.w           dr3, pen.v;             // pen.v -= dr3

        // set hpos to left edge

        move.w          pen.h, dr3;             // dr3 = pen.h
        ext.l           dr3;
        move.l          dr3, hpos;              // hpos = dr3

        // clip the top edge if needed

        clr.l           dr3;                        // dr3 = 0x00000000
        move.w          pen.v, dr5;             // dr3 = pen.v
        ext.l           dr5;
        movea.l         clip, ar4;              // ar4 = clip
        cmp.l           0x04(ar4), dr5;         // if dr3 >= (ar4)(clip)->top
        bge             cliptopdone;                // then don't clip

        // clip

        move.l          0x04(ar4), dr3;         // dr3 = (ar4)(clip)->top
        move.w          pen.v, dr5;             // dr5 = pen.v
        ext.l           dr5;
        sub.l           dr5, dr3;                   // dr3 -= dr5

    cliptopdone:

        move.l          dr3, topEdge;               // topEdge = dr3

        // clip the bottom edge if needed

        move.l          gDirectText->height, dr3;   // dr3 = gDirectText->height
        add.l           dr3, dr3;                   // dr3 *= 2
        move.l          dr3, dr5;                   // dr5 = dr3
        move.w          pen.v, dr4;             // dr4 = pen.v
        ext.l           dr4;
        add.l           dr4, dr5;                   // dr5 += dr4(pen.v)
        cmp.l           0x0c(ar4), dr5;         // if dr5 < (ar4)(clip)->bottom
        blt             clipbottomdone;         // then don't clip

        // clip

        sub.l           0x0c(ar4), dr5;         // dr5 (bottomEdge + pen.v) -= (ar4)(clip)->bottom
        add.l           #1, dr5;                    // dr5 += 1
        sub.l           dr5, dr3;                   // dr3(bottomEdge) -= dr5

    clipbottomdone:

        move.l          dr3, bottomEdge;            // bottomEdge = dr3
        movea.l         destMap, ar2;               // ar2 = destMap
        move.w          0x4(ar2), dr3;              // dr3 = (ar2)(destMap)->rowBytes
        andi.l          #0x00003fff, dr3;           // dr3 &= 0x3fff (mask out pixMap bit)
        move.l          dr3, rowBytes;          // rowBytes = dr3
        move.l          dr3, rowPlus;               // rowPlus = dr3

        move.l          gDirectText->logicalWidth, dr3; // dr3 = gDirectText->physicalWidth
//      add.l           #kCharSpace, dr3;           // dr3 += kCharSpace (always 0)
        move.l          dr3, charPlus;          // charPlus = dr3

        movea.l         (ar2), ar2;             // ar2 = (ar2)(destMap)->baseAddr
        move.w          pen.v, dr3;             // dr3 = pen.v
        ext.l           dr3;
        add.l           portTop, dr3;               // dr3 += portTop
        add.l           topEdge, dr3;               // dr3 += topEdge
        move.l          rowBytes, dr5;
        mulu.w          dr5, dr3;                   // dr3 *= rowBytes
        adda.l          dr3, ar2;                   // ar2 (hchar) += dr3
        adda.l          hpos, ar2;              // ar2 += hpos
        move.l          portLeft, dr3;          // dr3 = portLeft
        lsl.l           #2, dr3;                    // dr3 *= 4
        adda.l          dr3, ar2;                   // ar2 += (portLeft * 4)

        move.l          gDirectText->physicalWidth, dr6; // dr6 = gDirectText->physicalWidth
        move.l          rowPlus, dr5;               // dr5 = rowPlus
        sub.l           dr6, dr5;

        move.l          bottomEdge, dr3;
        sub.l           topEdge, dr3;
        lsr.l           #1, dr3;
        add.l           #1, dr3;
        move.l          dr3, bottomEdge;

        move.l          topEdge, dr3;
        lsr.l           #1, dr3;
        muls.w          dr6, dr3;
        move.l          dr3, topEdge;

        move.l          gDirectText->height, dr3;
        muls.w          dr3, dr6;

//      clr.l           dr4;
        move.l          slen, dr3;              // dr4 = slen
//      ext.l           dr4;

        bra             stringLoopCheck;

/*
        ar4 = clipRect
        ar2 = hchar
        ar3 =
        ar4 =

        dr3 =
        DRX =
        dr4 = slen
        dr5 = rowPlus
        dr6 = width * height
        dr7 =
*/
/*
        // Prepare for this character

    stringLoopTop:

        move.l      dr3, slen;

        movea.l     clip, ar4;
        move.l      (ar4), dr3;         // dr3 = clip->left
        cmp.l       hpos, dr3;          // if dr3 >= hpos
        bge         drawClippedChar;        // then draw clipped character

        move.l      0x08(ar4), dr3;     // dr3 = clip->right
        move.l      hpos, dr4;          // dr4 = hpos
        add.l       gDirectText->physicalWidth, dr4;
        cmp.l       dr3, dr4;               // if dr4 >= clip->right
        bge         drawClippedChar;        // then draw clipped character

        bra         drawUnclippedChar;

        // character is horiontally clipped; draw it one byte at a time

        // make sure it's not offscreen to the left

    drawClippedChar:

        move.l      hpos, dr4;          // dr4 = hpos
        add.l       gDirectText->physicalWidth, dr4;
        sub.l       #1, dr4;
        move.l      (ar4), dr3;         // dr3 = clip->left
        cmp.l       dr3, dr4;               // if dr4 (hpos + physicalWidth) < dr3 (clip->left)
        blt         dontDrawChar;       // then char is not at all onscreen

        // make sure it's not offscreen to the right

        move.l      0x08(ar4), dr3;     // dr3 = clip->right/
        cmp.l       hpos, dr3;          // if dr3(clip->right) < hpos
        ble         dontDrawChar;       // then char is not at all on screen

        // prepare to draw the clipped character

        // clip the left edge if needed

        clr.l       dr4;                // dr4 (leftSkip) = 0
        move.l      (ar4), dr3;         // dr3 = clip->left
        cmp.l       hpos, dr3;          // if dr3(clip->left) < hpos
        ble         dontClipLeft;       // then don't clip left

        // clip

        move.l      dr3, dr4;               // dr4 (leftSkip) = dr3(clip->left)
        sub.l       hpos, dr4;          // dr4 -= hpos

    dontClipLeft:

        move.l      dr4, leftSkip;      // leftSkip = dr4

        // clip the right edge if needed

        clr.l       dr4;                    // dr4 (bytesToDo) = 0
        move.l      hpos, dr3;          // dr3 = hpos
        add.l       gDirectText->physicalWidth, dr3;    // dr3 += physicalWidth
        cmp.l       0x08(ar4), dr3;     // if dr3 (hpos + physicalWidth) < (ar4)(clipRect)->right
        blt         dontClipRight;      // then dont clip right

        // clip

        move.l      dr3, dr4;               // dr4(bytesToDo) = dr3(hpos + physicalWidth)
        sub.l       0x08(ar4), dr4;     // dr4(bytesToDo) -= (ar4)(clipRect)->right
        addq.l      #1, dr4;                // dr4 += 1 (exclusive of border)

    dontClipRight:

        move.l      gDirectText->physicalWidth, dr3;    // dr3 = physicalWidth
        sub.l       leftSkip, dr3;      // dr3 -= leftSkip
        sub.l       dr4, dr3;               // dr3 -= dr4(bytesToDo)
        move.l      dr3, bytesToDo;     // bytesToDo = dr3

        // prepare to draw clipped char

        movea.l     gDirectText->charSet, ar3;  // ar3 = charSet
        movea.l     (ar3), ar3;         // ar3 = (ar3) (dereference handle) // ar3 = sbyte
//      move.l      gDirectText->height, dr3;   // dr3 = gDirectText->height
//      muls.w      dr6, dr3;               // dr3(height) *= dr6(width)
        move.l      dr6, dr3;
        movea.l     string, ar4;            // ar4 = string
        add.l       #1, string;         // string++
        clr.l       dr4;                    // dr4 = 0x00000000
        move.b      (ar4), dr4;         // dr4 = (ar4)(previous *string)
        muls.w      dr4, dr3;               // dr3(height * width) *= dr4
        adda.l      dr3, ar3;               // ar3 (slong) += dr3
        adda.l      topEdge, ar3;               // ar3(sbyte) += dr3
        adda.l      leftSkip, ar3;      // ar3(sbyte) += leftSkip

        movea.l     ar2, ar4;               // ar4(dbyte) = ar2(hchar)
        adda.l      leftSkip, ar4;

        move.l      bottomEdge, dr3;        // dr3 = bottomEdge

        bra         clipCharVCheck;

    clipCharVLoop:

        move.l      bytesToDo, dr4;     // dr4(i) = bytesToDo
        bra         clipCharHCheck;     // jump to end of loop

    clipCharHLoop:

        tst.b       (ar3);              // if (ar3)(*sbyte) == 0
        beq         clipCharNoByte;     // don't draw byte

        move.b      color, (ar4);       // else *dbyte = *sbyte

    clipCharNoByte:

        adda.l      #1, ar3;
        adda.l      #1, ar4;

    clipCharHCheck:

        dbra        dr4, clipCharHLoop;

        adda.l      rowBytes, ar4;
        suba.l      bytesToDo, ar4;
        adda.l      rowBytes, ar4;
        adda.l      gDirectText->physicalWidth, ar3;
        suba.l      bytesToDo, ar3;

    clipCharVCheck:

        dbra        dr3, clipCharVLoop;

        bra         stringLoopEnd;

        // character is offscreen; just increment string

    dontDrawChar:

        add.l       #1, string;         // string++
        bra         stringLoopEnd;      // skip to loop check

        // draw a fully horizontally visible character in longwords

    drawUnclippedChar:

        movea.l     gDirectText->charSet, ar3;  // ar3
        movea.l     (ar3), ar3;         // ar3 = (ar3) (dereference handle) // ar3 = slong
//      move.l      gDirectText->height, dr3;   // dr3 = gDirectText->height
//      muls.w      dr6, dr3;               // dr3(height) *= dr6(width)
        move.l      dr6, dr3;
        movea.l     string, ar4;        // ar4 = string
        add.l       #1, string;         // string++
        clr.l       dr4;                    // dr4 = 0x00000000
        move.b      (ar4), dr4;         // dr4 = (ar4)(previous *string)
        muls.w      dr4, dr3;               // dr3(height * width) *= dr4
        adda.l      dr3, ar3;               // ar3 (slong) += dr3
        adda.l      topEdge, ar3;               // ar3(slong) += dr3

        movea.l     ar2, ar4;               // ar4(dlong) = ar2(hchar)

        move.l      bottomEdge, dr3;        // dr3 = bottomEdge
        add.l       #1, dr3;
        move.l      dr3, count;             // count = dr3

        bra         unclipCharVCheck;   // skip to loop check

    unclipCharVLoop:

        move.l      gDirectText->physicalWidth, dr4;                // dr4 = dr6(width)
        lsr.l       #2, dr4;                // dr4 /= 4 (we're working in four byte chunks)

        bra         unclipCharHCheck;   // skip to loop check

    unclipCharHLoop:

        move.l      (ar4), dr3;         // dr3 = (ar4)(*dlong)
        move.l      (ar3)+, dr7;            // dr7 = (ar3)+(*slong++)
        or.l        dr7, dr3;               // dr3 |= dr7 (*slong)
        eor.l       dr7, dr3;               // dr3 ^= dr7 (*slong)
        and.l       colorlong, dr7;     // dr7 &= colorlong
        or.l        dr7, dr3;               // dr3 |= dr7 (colorlong & *slong)
        move.l      dr3, (ar4)+;        // (ar4)+(*dlong++) = dr3

    unclipCharHCheck:

        dbra dr4, unclipCharHLoop;      // dr4(i)--; if i >= 0 repeat loop

        adda.l      rowBytes, ar4;      // ar4(dlong) += rowbytes
        adda.l      dr5, ar4;               // ar3(dlong) += rowPlus

    unclipCharVCheck:

        move.l      count, dr3;             // dr3 = count
        subq        #1, dr3;                // dr3 -= 1
        move.l      dr3, count;             // count = dr3
        tst.l       dr3;                // if dr3 >= 0
        bne         unclipCharVLoop;        // then repeat loop


    stringLoopEnd:

        adda.l      charPlus, ar2;      // ar2 (hchar) += charPlus
        move.l      charPlus, dr3;      // dr3 = charPlus
        add.l       dr3, hpos;          // hpos += charPlus

    stringLoopCheck:

        move.l      slen, dr3;
        dbra        dr3, stringLoopTop; // dr4 (slen)--; if slen >= 0 repeat loop

        frfree;
        */
#endif //kAllowAssem
        rts;
}

#endif

void ResetDirectTextPtr( Handle directText)

{
#pragma unused( directText)
    WriteDebugLine( (char *)"\pDText Callback");
    gDirectText = (directTextType *)*gDirectTextData + gWhichDirectText;
}

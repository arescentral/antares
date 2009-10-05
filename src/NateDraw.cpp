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

// NateDraw.c : special on- and off-screen drawing -- not to be confused with original "Arcade Proj" Natedraw

#include "NateDraw.hpp"

#include <algorithm>
#include <assert.h>

#include "BinaryStream.hpp"
#include "Error.hpp"
#include "MathMacros.hpp"

// #define kByteLevelTesting

#ifdef kByteLevelTesting
#include "SpriteHandling.hpp" // for test byte debugging kludge
#endif

void longPointType::read(BinaryReader* bin) {
    bin->read(&h);
    bin->read(&v);
}

inline void mHBlitz(unsigned char*& mdbyte, long mrunLen, long mcolor, long& mcount) {
    mcount = mrunLen;
    while ( mcount-- > 0)
    {
        *(mdbyte++) =mcolor;
    }
}

inline void mDrawHorizontalRun(
        unsigned char*& dbyte, long xAdvance, long runLen, long color, long drowPlus, long count) {
    count = runLen;
    while ( count-- > 0)
    {
        *dbyte = color;
        dbyte += xAdvance;
    }
    dbyte += drowPlus;
}

inline void mDrawVerticalRun(
        unsigned char*& dbyte, long xAdvance, long runLen, long color, long drowPlus, long count) {
    count = runLen;
    while ( count-- > 0)
    {
        *dbyte = color;
        dbyte += drowPlus;
    }
    dbyte += xAdvance;
}

inline void mCopyHorizontalRun(
        unsigned char*& dbyte, unsigned char*& sbyte, long xAdvance, long runLen, long drowPlus,
        long srowPlus, long count) {
    count = runLen;
    while ( count-- > 0)
    {
        *dbyte = *sbyte;
        dbyte += xAdvance;
        sbyte += xAdvance;
    }
    dbyte += drowPlus;
    sbyte += srowPlus;
}

inline void mCopyVerticalRun(
        unsigned char*& dbyte, unsigned char*& sbyte, long xAdvance, long runLen, long drowPlus,
        long srowPlus, long count) {
    count = runLen;
    while ( count-- > 0)
    {
        *dbyte = *sbyte;
        dbyte += drowPlus;
        sbyte += srowPlus;
    }
    dbyte += xAdvance;
    sbyte += xAdvance;
}

inline void mDashHorizontalRun(
        unsigned char*& dbyte, long xAdvance, long runLen, long color, long drowPlus, long count,
        long mdashon, long mdashoff, long mdashcount) {
    count = runLen;
    do
    {
        while (( count-- > 0) && ( mdashcount < mdashon))
        {
            *dbyte = color;
            dbyte += xAdvance;
            mdashcount++;
        }
        while (( count-- > 0) && ( mdashcount < mdashoff))
        {
            dbyte += xAdvance;
            mdashcount++;
        }
        if ( mdashcount == mdashoff) mdashcount = 0;
    } while ( count > 0);
    dbyte += drowPlus;
}

inline void mDashVerticalRun(
        unsigned char*& dbyte, long xAdvance, long runLen, long color, long drowPlus, long count,
        long mdashon, long mdashoff, long mdashcount) {
    count = runLen;
    do
    {
        while (( count-- > 0) && ( mdashcount < mdashon))
        {
            *dbyte = color;
            dbyte += drowPlus;
            mdashcount++;
        }
        while (( count-- > 0) && ( mdashcount < mdashon))
        {
            dbyte += drowPlus;
            mdashcount++;
        }
        if ( mdashcount == mdashoff) mdashcount = 0;
    } while ( count > 0);
    dbyte += xAdvance;
}

namespace {

Rect from_origin(const Rect& r) {
    return Rect(0, 0, r.width(), r.height());
}

bool intersects(const Rect& contained, const Rect& container) {
    return ((contained.right > container.left)
            && (contained.left < container.right)
            && (contained.bottom > container.top)
            && (contained.top < container.bottom));
}

void clip_rect(Rect* contained, const Rect& container) {
    contained->left = std::max(contained->left, container.left);
    contained->top = std::max(contained->top, container.top);
    contained->right = std::min(contained->right, container.right);
    contained->bottom = std::min(contained->bottom, container.bottom);
}

}  // namespace

// DrawNateRect: Direct-draws a rectangle
// hoff & voff are the h & v offsets frop the left and top edges of the destination map
// (ie the top & right edges of the destination window)
// CLIPS to destPix->bounds, NOT counting hoff & voff

void DrawNateRect(PixMap* destPix, Rect* destRect, long hoff, long voff, unsigned char color) {
    assert(hoff == 0);
    assert(voff == 0);

    if (!intersects(*destRect, from_origin(destPix->bounds))) {
        destRect->left = destRect->right = destRect->top = destRect->bottom = 0;
        return;
    }
    clip_rect(destRect, from_origin(destPix->bounds));

    int32_t width = destRect->right - destRect->left;
    if (width < 0) {
        return;
    }

    int32_t drowPlus = destPix->rowBytes & 0x3fff;
    unsigned char* bytes = destPix->baseAddr + destRect->top * drowPlus + destRect->left;
    for (int i = destRect->top; i < destRect->bottom; ++i) {
        memset(bytes, color, width);
        bytes += drowPlus;
    }
}

void DrawNateRectVScan( PixMap *destPix, Rect *destRect, long hoff, long voff,
        unsigned char color) {
    assert(hoff == 0);
    assert(voff == 0);

    if (!intersects(*destRect, from_origin(destPix->bounds))) {
        destRect->left = destRect->right = destRect->top = destRect->bottom = 0;
        return;
    }
    clip_rect(destRect, from_origin(destPix->bounds));

    if (destRect->right <= destRect->left) {
        return;
    }

    int32_t drowPlus = destPix->rowBytes & 0x3fff;
    unsigned char* bytes = destPix->baseAddr + destRect->top * drowPlus;
    for (int i = destRect->top; i < destRect->bottom; ++i) {
        for (int j = destRect->left; j < destRect->right; ++j) {
            if ((i ^ j) & 0x1) {
                bytes[j] = color;
            }
        }
        bytes += drowPlus;
    }
}

void DrawNateRectClipped(PixMap* destPix, Rect* destRect, Rect* clipRect, long hoff, long voff,
        unsigned char color) {
    if (!intersects(*destRect, from_origin(destPix->bounds))) {
        destRect->left = destRect->right = destRect->top = destRect->bottom = 0;
        return;
    }
    clip_rect(destRect, *clipRect);

    DrawNateRect(destPix, destRect, hoff, voff, color);
}

// must be square
void DrawNateTriangleUpClipped( PixMap *destPix, Rect *destRect,
    Rect *clipRect, long hoff, long voff, unsigned char color)

{
    long            drowPlus, x, leftEdge, rightPlus, trueWidth, count;
    unsigned char   *dbyte;
    Boolean         clipped = false;

    if (( destRect->right <= 0) || ( destRect->left >= ( destPix->bounds.right - destPix->bounds.left))
        || ( destRect->bottom <= 0) || ( destRect->top >= (destPix->bounds.bottom - destPix->bounds.top)))
    {
        destRect->left = destRect->right = destRect->top = destRect->bottom = 0;
        return;
    }

    drowPlus = destPix->rowBytes & 0x3fff;
    trueWidth = destRect->right - destRect->left;

    if ( destRect->left < clipRect->left)
    {
        destRect->left = clipRect->left;
        clipped = true;
    }
    if ( destRect->right > clipRect->right)
    {
        destRect->right = clipRect->right;
        clipped = true;
    }
    if ( destRect->top < clipRect->top)
    {
        destRect->top = clipRect->top;
        clipped = true;
    }
    if ( destRect->bottom > clipRect->bottom)
    {
        destRect->bottom = clipRect->bottom;
        clipped = true;
    }

    if ( clipped) return;

    if ( trueWidth == 0)
    {
        dbyte = destPix->baseAddr + ((destRect->top + voff) *
            drowPlus) + destRect->left + hoff;
        *dbyte = color;
        return;
    }

    leftEdge = 0;
    x = rightPlus = trueWidth - 1;
    dbyte = destPix->baseAddr + ((destRect->top + voff) *
        drowPlus) + destRect->left + hoff;
    drowPlus -= rightPlus;
    while ( x >= 0)
    {
        *(dbyte++) = color;
        x--;
    }

    if ( rightPlus > 0)
    {
        dbyte += drowPlus - 1;
/*      *dbyte = color;
        dbyte += rightPlus;
        *dbyte = color;
*/
        mHBlitz( dbyte, rightPlus + 1, color, count);
        dbyte--;
    }
    drowPlus += 1;
    rightPlus -= 2;
    dbyte += drowPlus;
    drowPlus += 1;

    while ( rightPlus >= 0)
    {
/*      *dbyte = color;
        dbyte += rightPlus;
        *dbyte = color;
*/
        mHBlitz( dbyte, rightPlus + 1, color, count);
        dbyte--;

        if ( rightPlus > 0)
        {
            dbyte += drowPlus;
/*          *dbyte = color;
            dbyte += rightPlus;
            *dbyte = color;
*/
            mHBlitz( dbyte, rightPlus + 1, color, count);
            dbyte--;
        }

        drowPlus += 1;
        rightPlus -= 2;
        dbyte += drowPlus;
        drowPlus += 1;
    }
}

void DrawNatePlusClipped( PixMap *destPix, Rect *destRect,
    Rect *clipRect, long hoff, long voff, unsigned char color)

{
    long            drowPlus, x, half, trueWidth, count;
    unsigned char   *dbyte;
    Boolean         clipped = false;

    if (( destRect->right <= 0) || ( destRect->left >= ( destPix->bounds.right - destPix->bounds.left))
        || ( destRect->bottom <= 0) || ( destRect->top >= (destPix->bounds.bottom - destPix->bounds.top)))
    {
        destRect->left = destRect->right = destRect->top = destRect->bottom = 0;
        return;
    }

    drowPlus = destPix->rowBytes & 0x3fff;
    trueWidth = destRect->right - destRect->left - 1;

    if ( destRect->left < clipRect->left)
    {
        destRect->left = clipRect->left;
        clipped = true;
    }
    if ( destRect->right > clipRect->right)
    {
        destRect->right = clipRect->right;
        clipped = true;
    }
    if ( destRect->top < clipRect->top)
    {
        destRect->top = clipRect->top;
        clipped = true;
    }
    if ( destRect->bottom > clipRect->bottom)
    {
        destRect->bottom = clipRect->bottom;
        clipped = true;
    }

    if ( clipped) return;

    if ( trueWidth == 0)
    {
        dbyte = destPix->baseAddr + ((destRect->top + voff) *
            drowPlus) + destRect->left + hoff;
        *dbyte = color;
        return;
    }

    if ( !(trueWidth & 0x00000001)) trueWidth--;
    if ( trueWidth < 3)
    {
        half = (trueWidth >> 1) + 1;

        dbyte = destPix->baseAddr + ((destRect->top + voff) *
            drowPlus) + destRect->left + hoff + half;

        x = 1;
        while ( x < half)
        {
            *dbyte = color;
            dbyte += drowPlus;
            x++;
        }
        dbyte -= half - 1;
        x = 0;
        while ( x < trueWidth)
        {
            *(dbyte++) = color;
            x++;
        }
        dbyte += drowPlus - trueWidth + half - 1;
        x = 1;
        while ( x < half)
        {
            *dbyte = color;
            dbyte += drowPlus;
            x++;
        }
    } else
    {
        half = (trueWidth >> 1) + 1;

        dbyte = destPix->baseAddr + ((destRect->top + voff) *
            drowPlus) + destRect->left + hoff + half - 1;

        x = 2;
        while ( x < half)
        {
            mHBlitz( dbyte, 3, color, count);
            dbyte += drowPlus - 3;
            x++;
        }
        dbyte -= half - 2;
        x = 0;
        while ( x < 3)
        {
            mHBlitz( dbyte, trueWidth, color, count);
            dbyte += drowPlus - trueWidth;
            x++;
        }
        dbyte += half - 2;
        x = 2;
        while ( x < half)
        {
            mHBlitz( dbyte, 3, color, count);
            dbyte += drowPlus - 3;
            x++;
        }
    }

}

void DrawNateSquareClipped( PixMap *destPix, Rect *destRect,
    Rect *clipRect, long hoff, long voff, unsigned char color)

{
    long            drowPlus, x, rightPlus, trueWidth;
    unsigned char   *dbyte;
    Boolean         clipped = false;

    if (( destRect->right <= 0) || ( destRect->left >= ( destPix->bounds.right - destPix->bounds.left))
        || ( destRect->bottom <= 0) || ( destRect->top >= (destPix->bounds.bottom - destPix->bounds.top)))
    {
        destRect->left = destRect->right = destRect->top = destRect->bottom = 0;
        return;
    }

    drowPlus = destPix->rowBytes & 0x3fff;
    trueWidth = destRect->right - destRect->left - 1;

    if ( destRect->left < clipRect->left)
    {
        destRect->left = clipRect->left;
        clipped = true;
    }
    if ( destRect->right > clipRect->right)
    {
        destRect->right = clipRect->right;
        clipped = true;
    }
    if ( destRect->top < clipRect->top)
    {
        destRect->top = clipRect->top;
        clipped = true;
    }
    if ( destRect->bottom > clipRect->bottom)
    {
        destRect->bottom = clipRect->bottom;
        clipped = true;
    }

    if ( clipped) return;

    if ( trueWidth == 0)
    {
        dbyte = destPix->baseAddr + ((destRect->top + voff) *
            drowPlus) + destRect->left + hoff;
        *dbyte = color;
        return;
    }

    dbyte = destPix->baseAddr + ((destRect->top + voff) *
        drowPlus) + destRect->left + hoff;

    rightPlus = trueWidth;
    drowPlus -= rightPlus;

    x = 0;
    while ( x <= trueWidth)
    {
        *(dbyte++) = color;
        x++;
    }

    if ( trueWidth == 0) return;

    dbyte += drowPlus - 1;

    if ( trueWidth > 1)
    {
        x = 2;
        while ( x <= trueWidth)
        {
            *dbyte = color;
            dbyte += rightPlus;
            *dbyte = color;
            dbyte += drowPlus;
            x++;
        }
    }

    x = 0;
    while ( x <= trueWidth)
    {
        *(dbyte++) = color;
        x++;
    }
}

void DrawNateDiamondClipped( PixMap *destPix, Rect *destRect,
    Rect *clipRect, long hoff, long voff, unsigned char color)

{
    long            drowPlus, leftEdge, rightPlus, trueWidth, count;
    unsigned char   *dbyte;
    Boolean         clipped = false;

    if (( destRect->right <= 0) || ( destRect->left >= ( destPix->bounds.right - destPix->bounds.left))
        || ( destRect->bottom <= 0) || ( destRect->top >= (destPix->bounds.bottom - destPix->bounds.top)))
    {
        destRect->left = destRect->right = destRect->top = destRect->bottom = 0;
        return;
    }

    drowPlus = destPix->rowBytes & 0x3fff;
    trueWidth = destRect->right - destRect->left - 1;

    if ( destRect->left < clipRect->left)
    {
        destRect->left = clipRect->left;
        clipped = true;
    }
    if ( destRect->right > clipRect->right)
    {
        destRect->right = clipRect->right;
        clipped = true;
    }
    if ( destRect->top < clipRect->top)
    {
        destRect->top = clipRect->top;
        clipped = true;
    }
    if ( destRect->bottom > clipRect->bottom)
    {
        destRect->bottom = clipRect->bottom;
        clipped = true;
    }

    if ( clipped) return;

    if ( trueWidth == 0)
    {
        dbyte = destPix->baseAddr + ((destRect->top + voff) *
            drowPlus) + destRect->left + hoff;
        *dbyte = color;
        return;
    }

    leftEdge = (trueWidth >> 1) + (trueWidth & 1);
    rightPlus = ((trueWidth >> 1) + 1) - leftEdge;

    dbyte = destPix->baseAddr + ((destRect->top + voff) *
        drowPlus) + destRect->left + hoff + leftEdge;
    drowPlus -= rightPlus + 1;
    while ( leftEdge > 0)
    {
/*      *dbyte = color;
        dbyte += rightPlus;
        *dbyte = color;
*/
        mHBlitz( dbyte, rightPlus + 1, color, count);
        dbyte--;

        dbyte += drowPlus;
        drowPlus -= 2;
        rightPlus += 2;
        leftEdge--;
    }
    dbyte++;
    leftEdge = (trueWidth >> 1) + (trueWidth & 1);
    drowPlus += 4;
    rightPlus -= 2;
    if ( trueWidth & 0x1)
    {
        dbyte++;
        drowPlus += 2;
        rightPlus -= 2;
        leftEdge--;
    }
    while ( leftEdge > 0)
    {
/*      *dbyte = color;
        dbyte += rightPlus;
        *dbyte = color;
*/
        mHBlitz( dbyte, rightPlus + 1, color, count);
        dbyte--;

        dbyte += drowPlus;
        drowPlus += 2;
        rightPlus -= 2;
        leftEdge--;
    }
}

void DrawNateVBracket( PixMap *destPix, Rect *destRect, Rect *clipRect, long hoff, long voff,
                        unsigned char color)

{
    long            rowBytes = 0;
    unsigned char   *aByte;

    DrawNateLine ( destPix, clipRect, destRect->left, destRect->top, destRect->right - 1,
                    destRect->top, hoff, voff, color);
    DrawNateLine ( destPix, clipRect, destRect->left, destRect->bottom - 1, destRect->right - 1,
                    destRect->bottom - 1, hoff, voff, color);

    mGetRowBytes( rowBytes, destPix);

    mSetNatePixel( aByte, rowBytes, destRect->left, destRect->top + 1, hoff, voff, destPix, color);
    mSetNatePixel( aByte, rowBytes, destRect->right - 1, destRect->top + 1, hoff, voff, destPix, color);

    mSetNatePixel( aByte, rowBytes, destRect->left, destRect->bottom - 2, hoff, voff, destPix, color);
    mSetNatePixel( aByte, rowBytes, destRect->right - 1, destRect->bottom - 2, hoff, voff, destPix, color);
}

void DrawNateShadedRect( PixMap *destPix, Rect *destRect, Rect *clipRect, long hoff, long voff,
                        unsigned char fillcolor, unsigned char lightcolor, unsigned char darkcolor)

{
    Rect    tRect = *destRect;

    tRect.right--;
    tRect.bottom--;

    DrawNateLine ( destPix, clipRect, tRect.left, tRect.bottom, tRect.left,
                    tRect.top, hoff, voff, lightcolor);
    DrawNateLine ( destPix, clipRect, tRect.left, tRect.top, tRect.right,
                    tRect.top, hoff, voff, lightcolor);

    DrawNateLine ( destPix, clipRect, tRect.right, tRect.top, tRect.right,
                    tRect.bottom, hoff, voff, darkcolor);
    DrawNateLine ( destPix, clipRect, tRect.right, tRect.bottom, tRect.left,
                    tRect.bottom, hoff, voff, darkcolor);
    tRect.left++;
    tRect.top++;

    DrawNateRectClipped( destPix, &tRect, clipRect, hoff, voff, fillcolor);

}

void BiggestRect( Rect  *dRect, Rect *sRect)

{
    if ( sRect->left < dRect->left) dRect->left = sRect->left;
    if ( sRect->right > dRect->right) dRect->right = sRect->right;
    if ( sRect->top < dRect->top) dRect->top = sRect->top;
    if ( sRect->bottom > dRect->bottom) dRect->bottom = sRect->bottom;
}

void LongRectToRect( Rect *sRect, Rect *dRect)

{
    dRect->left = sRect->left;
    dRect->top = sRect->top;
    dRect->right = sRect->right;
    dRect->bottom = sRect->bottom;
}

void RectToLongRect( Rect *sRect, Rect *dRect)

{
    dRect->left = sRect->left;
    dRect->top = sRect->top;
    dRect->right = sRect->right;
    dRect->bottom = sRect->bottom;
}

void SetLongRect( Rect *dRect, long left, long top, long right, long bottom)

{
    dRect->left = left;
    dRect->top = top;
    dRect->right = right;
    dRect->bottom = bottom;
}

//
//           Sunday, November 6, 1994 4:11:09 AM
//           mac.programmer Item
//   From:           Howard Berkey,howard@sirius.com,Internet
//   Subject:        (1 of 2) Re: Need a Fast Line Drawing Algorithm
//   To:             mac.programmer
// Enough people wrote me asking for this that I'm posting it to c.s.m.p. in
// addition to mailing it to Eric.  The specific example is for Borland C++,
// but it gets the point across rather nicely.  I found this in an archive of
// Michael Abrash's columns from DDJ.
//
// I'll post the article here and the source code in the next message.
//
//
// // begin incl. file //
//
// Journal:    Dr. Dobb's Journal  Nov 1992 v17 n11 p171(6)
// -------------------------------------------------------------------------
// Title:     The good, the bad, and the run-sliced. (Bresenham's run-length
//            slice algorithm) (Graphics Programming) (Column)
// Author:    Abrash, Michael
// Attached:  Program: GP-NOV92.ASC  Source code listing.
//            Program: XSHARP21.ZIP  Library for 3-D animation.
//
// Abstract:  Programmers developing graphics applications should never
//            believe that they have developed the fastest possible code;
//            others often have better ideas.  Performance-intensive
//            applications should never have code that performs the same
//            calculation more than once. Programmers have a duty to
//            implement the desired functions while enabling the hardware to
//            work as efficiently as possible.  Bresenham's run-length slice
//            algorithm is a good model for efficient programming.  The
//            approach implemented by the algorithm steps one pixel at a
//            time along the major axis, while an integer term indicating
//            how close the line is to advancing halfway to the next major
//            pixel along the minor axis is maintained.  The algorithm
//            provides an automatic decision-making structure that allows
//            infrequent decisions to be made as quickly as possible.
// -------------------------------------------------------------------------
//
// <EDITED BY NL>
//
// Run-length Slice Fundamentals
//
// First off, I have a confession to make: I'm not sure that the algorithm
// I'll discuss is actually, precisely Bresenham's run-length slice
// algorithm.  It's been a long time since I read about this algorithm; in
// the intervening years, I've misplaced Bresenham's article, and I was
// unable to locate it in time for this column.  (Vermont libraries leave
// something to be desired in the high-tech area.)  As a result, I had to
// derive the algorithm from scratch, which was admittedly more fun than
// reading about it, and also ensured that I understood it inside and out.
// The upshot is that what I discuss may or may not be Bresenham's
// run-length slice algorithm--but it surely is fast.
//
// The place to begin understanding the run-length slice algorithm is the
// standard Bresenham's line-drawing algorithm.  (I discussed the standard
// Bresenham's algorithm at length in the May 1989 issue of the now-defunct
// Programmer's Journal.)  The basis of the standard approach is stepping
// one pixel at a time along the major axis (the longer dimension of the
// line), while maintaining an integer error term that indicates at each
// major-axis step how close the line is to advancing halfway to the next
// pixel along the minor axis.  Figure 1 illustrates standard Bresenham's
// line drawing.  The key point here is that a calculation and a test are
// performed once for each step along the major axis.
//
// The run-length slice algorithm rotates matters 90 degrees, with
// salubrious results.  The basis of the run-length slice algorithm is
// stepping one pixel at a time along the minor axis (the shorter
// dimension), while maintaining an integer error term indicating how close
// the line is to advancing an extra pixel along the major axis, as
// illustrated by Figure 2.
//
// Consider this:  When you're called upon to draw a line with an
// X-dimension of 35 and a Y-dimension of 10, you have a great deal of
// information available, some of which is ignored by standard Bresenham's.
// In particular, because the slope is between 1/3 and 1/4, you know that
// every single run--a run being a set of pixels at the same minor-axis
// coordinate--must be either three or four pixels long.  No other length is
// possible, as shown in Figure 3 (apart from the first and last runs, which
// are special cases that I'll discuss shortly).  Therefore, for this line,
// there's no need to perform an error-term calculation and test for each
// pixel.  Instead, we can just perform one test per run, to see whether the
// run is three or four pixels long, thereby eliminating about 70 percent of
// the calculations in drawing this line.
//
// Take a moment to let the idea behind run-length slice drawing soak in.
// Periodic decisions must be made to control pixel placement.  The key to
// speed is to make those decisions as infrequently and quickly as possible.
//  Of course, it will work to make a decision at each pixel--that's
// standard Bresenham's.  However, most of those per-pixel decisions are
// redundant, and in fact we have enough information before we begin to know
// which are the redundant decisions.  Run-length slice drawing is exactly
// equivalent to standard Bresenham's, but it pares the decision-making
// process down to a minimum.  It's some-what analogous to the difference
// between finding the greatest common divisor of two numbers using Euclid's
// algorithm and finding it by trying every possible divisor.  Both
// approaches produce the desired result, but that which takes maximum
// advantage of the available information and minimizes redundant work is
// preferable.
//
// Run-length Slice Implementation
//
// We know that for any line, a given run will always be one of two possible
// lengths.  How, though, do we know which length to select?  Surprisingly,
// this is easy to determine.  For the following discussion, assume that we
// have a slope of 1/3.5, so that X is the major axis; however, the
// discussion also applies to Y-major lines, with X and Y reversed.
//
// The minimum possible length for any run in an X-major line is
// int(XDelta/YDelta), where XDelta is the X-dimension of the line and
// YDelta is the Y-dimension.  The maximum possible length is
// int(XDelta/YDelta) + 1.  The trick, then, is knowing which of these two
// lengths to select for each run.  To see how we can make this selection,
// refer to Figure 4.  For each one-pixel step along the minor axis (Y, in
// this case), we advance at least three pixels.  The full advance distance
// along X (the major axis) is actually three-plus pixels, because there is
// also a fractional portion to the advance along X for a single-pixel Y
// step.  This fractional advance is the key to deciding when to add an
// extra pixel to a run.  The fraction indicates what portion of an extra
// pixel we advance along X (the major axis) during each run.  If we keep a
// running sum of the fractional parts, we have a measure of how close we
// are to needing an extra pixel; when the fractional sum reaches 1, it's
// time to add an extra pixel to the current run.  Then we can subtract 1
// from the running sum (because we just advanced one pixel), and continue
// on.
//
// Practically speaking, however, we can't work with fractions because
// floating-point arithmetic is slow and fixed-point arithmetic is
// imprecise.  Therefore, we take a cue from standard Bresenham's and scale
// all the error-term calculations up so that we can work with integers.
// The fractional X (major axis) advance per one-pixel Y (minor axis)
// advance is the fractional portion of XDelta/YDelta.  This value is
// exactly equivalent to (XDelta % YDelta)/YDelta.  We'll scale this up by
// multiplying it by YDelta*2, so that the amount by which we adjust the
// error term up for each one-pixel minor-axis advance is (XDelta %
// YDelta)*2.
//
// We'll similarly scale up the one pixel by which we adjust the error term
// down after it turns over, so our downward error-term adjustment is
// YDelta*2.  Therefore, before drawing each run, we'll add (XDelta %
// YDelta)*2 to the error term.  If the error term turns over (reaches one
// full pixel), then we'll lengthen the run by 1, and subtract YDelta*2 from
// the error term.  (All values are multiplied by 2 so that the initial
// error term, which involves a 0.5 term, can be scaled up to an integer, as
// discussed below.)
//
// This is not a complicated process, involving only integer addition and
// subtraction and a single test, and it lends itself to many and varied
// optimizations.  For example, you could break out hardwired optimizations
// for drawing each possible pair of run lengths.  For the aforementioned
// line with a slope of 1/3.5, for example, you could have one routine
// hardwired to blast in a run of three pixels as quickly as possible, and
// another hardwired to blast in a run of four pixels.  These routines would
// ideally have no looping, but rather just a series of instructions
// customized to draw the desired number of pixels at maximum speed.  Each
// routine would know that the only possibilities for the length of the next
// run would be three and four, so they could increment the error term, then
// jump directly to the appropriate one of the two routines depending on
// whether the error term turned over.  Properly implemented, it should be
// possible to reduce the average per-run overhead of line drawing to less
// than one branch, with only two additions and two tests (the number of
// runs must also be counted down), plus a subtraction half the time.  On a
// 486, this amounts to something on the order of 150 nanoseconds of
// overhead per pixel, exclusive of the time required to actually write the
// pixel to display memory.
//
// That's good.
//
// Run-length Slice Details
//
// A couple of run-length slice implementation details yet remain.  First is
// the matter of how error-term turnover is detected.  This is done in much
// the same way as it is with standard Bresenham's: The error term is
// initialized to a value equivalent to -1 pixel and incremented for each
// step; when the error term reaches 0, we've advanced one full pixel along
// the major axis, and it's time to add an extra pixel to the current run.
// This means that we only have to test the sign of the error term after
// advancing it to determine whether or not to add an extra pixel to each
// run.
//
// The second and more difficult detail is balancing the runs so that
// they're centered around the ideal line, and therefore draw the same
// pixels that standard Bresenham's would draw.  If we just drew full-length
// runs from the start, we'd end up with an unbalanced line, as shown in
// Figure 5.  Instead, we have to split the initial pixel plus one full run
// as evenly as possible between the first and last runs of the line, and
// adjust the initial error term appropriately for the initial half-run.
//
// The initial error term is simply one-half of the normal fractional
// advance along the major axis, because the initial step is only one-half
// pixel along the minor axis.  This half-step gets us exactly halfway
// between the initial pixel and the next pixel along the minor axis.  All
// the error-term adjusts are scaled up by two times precisely so that we
// can scale up this halved error term for the initial run by two times, and
// thereby make it an integer.
//
// The other trick here is that if an odd number of pixels are allocated
// between the first and last partial runs, we'll end up with an odd pixel,
// since we are unable to draw a half-pixel.  This odd pixel is accounted
// for by adding half a pixel to the error term.
//
// That's all there is to run-length slice line drawing; the partial first
// and last runs are the only tricky part.  Listing One (page 190) is a
// run-length slice implementation in C.  This is not an optimized
// implementation, nor is it meant to be; this listing is provided so that
// you can see how the run-length slice algorithm works.  Next month, I'll
// move on to an optimized version, but this month's listing will make it
// much easier to grasp the principles of run-length slice drawing, and to
// understand next month's code.
//
// Notwithstanding that it's not optimized, Listing One is reasonably fast.
// If you run Listing Two (page 191), a sample line-drawing program that you
// can use to test-drive Listing One, you may be as surprised as I was at
// how quickly the screen fills with vectors, considering that Listing One
// is entirely in C and has some redundant divides.  Or perhaps you won't be
// surprised, in which case I suggest you check back next month.
//
// Next Time
//
// Next month, I'll switch to assembly language and speed up run-length
// slice lines considerably.  I'll also spend some time discussing the
// limitations of run-length slice drawing, and I'll look at possible
// further optimizations.  After that, perhaps we'll have a look at seed
// fills, or more 3-D animation, or some new 2-D animation topics--or maybe
// something completely different.  Your suggestions are, as always,
// welcome.
//
// --
// :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
// Howard Berkey                                       howard@sirius.com
// "'La-Z-Boy'?  What's the next brand, 'Hopeless Slack-Ass'?" - Frazier
//
// --- Internet Message Header Follows ---
// Path: uupsi!psinntp!rebecca!newserve!ub!news.kei.com!sol.ctr.columbia.edu!howland.reston.ans.net!pipex!uunet!news.cygnus.com!news.zeitgeist.net!slip228.sirius.com!user
// From: howard@sirius.com (Howard Berkey)
// Newsgroups: comp.sys.mac.programmer
// Subject: (1 of 2) Re: Need a Fast Line Drawing Algorithm
// Date: 6 Nov 1994 04:11:09 GMT
// Organization: Weyland-Yutani Thinking Machines Division
// Lines: 558
// Message-ID: <howard-0511942022590001@slip228.sirius.com>
// References: <weasel-0511940219360001@192.0.2.1>
// NNTP-Posting-Host: slip228.sirius.com
//
//
//           Sunday, November 6, 1994 4:13:22 AM
//           mac.programmer Item
//   From:           Howard Berkey,howard@sirius.com,Internet
//   Subject:        (2 of 2) Re: Need a Fast Line Drawing Algorithm
//   To:             mac.programmer
// (Here's the source code continued from the previous message)
//
// // begin incl. message //
//
//
//
// _GRAPHICS PROGRAMMING COLUMN_
// by Michael Abrash
//
// [LISTING ONE]
//
// Run-length slice line drawing implementation for mode 0x13, the VGA's
// 320x200 256-color mode. Not optimized! Tested with Borland C++ 3.0 in
// the small model.

// Draws a line between the specified endpoints in color Color.

void DrawNateLine( PixMap *destPix, Rect *clipRect, long XStart, long YStart, long XEnd,
                    long YEnd, long hoff, long voff, unsigned char Color)
{
    long            Temp, AdjUp, AdjDown, ErrorTerm, XAdvance, XDelta, YDelta, drowPlus;
    long            WholeStep, InitialPixelCount, FinalPixelCount, i, RunLength;
    unsigned char   *dbyte;
    short           cs = mClipCode( XStart, YStart, *clipRect), ce = mClipCode( XEnd, YEnd, *clipRect);

    while ( cs | ce)
    {
        if ( cs & ce) return;
        XDelta = XEnd - XStart;
        YDelta = YEnd - YStart;
        if ( cs)
        {
            if ( cs & 8)
            {
                YStart += YDelta * ( clipRect->left - XStart) / XDelta;
                XStart = clipRect->left;
            } else
            if ( cs & 4)
            {
                YStart += YDelta * ( clipRect->right - 1 - XStart) / XDelta;
                XStart = clipRect->right - 1;
            } else
            if ( cs & 2)
            {
                XStart += XDelta * ( clipRect->top - YStart) / YDelta;
                YStart = clipRect->top;
            } else
            if ( cs & 1)
            {
                XStart += XDelta * ( clipRect->bottom - 1 - YStart) / YDelta;
                YStart = clipRect->bottom - 1;
            }
            cs = mClipCode( XStart, YStart, *clipRect);
        } else if ( ce)
        {
            if ( ce & 8)
            {
                YEnd += YDelta * ( clipRect->left - XEnd) / XDelta;
                XEnd = clipRect->left;
            } else
            if ( ce & 4)
            {
                YEnd += YDelta * ( clipRect->right - 1 - XEnd) / XDelta;
                XEnd = clipRect->right - 1;
            } else
            if ( ce & 2)
            {
                XEnd += XDelta * ( clipRect->top - YEnd) / YDelta;
                YEnd = clipRect->top;
            } else
            if ( ce & 1)
            {
                XEnd += XDelta * ( clipRect->bottom - 1 - YEnd) / YDelta;
                YEnd = clipRect->bottom - 1;
            }
            ce = mClipCode( XEnd, YEnd, *clipRect);
        }
    }

    // We'll always draw top to bottom, to reduce the number of cases we have to
    // handle, and to make lines between the same endpoints draw the same pixels
    if (YStart > YEnd)
    {
        Temp = YStart;
        YStart = YEnd;
        YEnd = Temp;
        Temp = XStart;
        XStart = XEnd;
        XEnd = Temp;
    }


    // Point to the bitmap address first pixel to draw
    drowPlus = destPix->rowBytes & 0x3fff;
    dbyte = destPix->baseAddr + (YStart + voff) * drowPlus +
                XStart + hoff;

    // Figure out whether we're going left or right, and how far we're
    // going horizontally
    if ((XDelta = XEnd - XStart) < 0)
    {
        XAdvance = -1;
        XDelta = -XDelta;
    }
    else
    {
        XAdvance = 1;
    }
    // Figure out how far we're going vertically
    YDelta = YEnd - YStart;

    // Special-case horizontal, vertical, and diagonal lines, for speed
    // and to avoid nasty boundary conditions and division by 0
    if (XDelta == 0)
    {
        // Vertical line
        for (i=0; i<=YDelta; i++)
        {
#ifdef kByteLevelTesting
            TestByte( (char *)dbyte, destPix, "\pVLINE");
#endif
            *dbyte = Color;
            dbyte += drowPlus;
        }
        return;
    }
    if (YDelta == 0)
    {
        // Horizontal line
        for (i=0; i<=XDelta; i++)
        {
#ifdef kByteLevelTesting
            TestByte( (char *)dbyte, destPix, "\pHLINE");
#endif
            *dbyte = Color;
            dbyte += XAdvance;
        }
        return;
    }
    if (XDelta == YDelta)
    {
        // Diagonal line
        for (i=0; i<=XDelta; i++)
        {
#ifdef kByteLevelTesting
            TestByte( (char *)dbyte, destPix, "\pDLINE");
#endif
            *dbyte = Color;
            dbyte += XAdvance + drowPlus;
        }
        return;
    }

    // Determine whether the line is X or Y major, and handle accordingly
    if (XDelta >= YDelta)
    {
        // X major line
        // Minimum # of pixels in a run in this line
        WholeStep = XDelta / YDelta;

        // Error term adjust each time Y steps by 1; used to tell when one
        // extra pixel should be drawn as part of a run, to account for
        // fractional steps along the X axis per 1-pixel steps along Y
        AdjUp = (XDelta % YDelta) << 1;

        // Error term adjust when the error term turns over, used to factor
        // out the X step made at that time
        AdjDown = YDelta * 2;

        // Initial error term; reflects an initial step of 0.5 along the Y
        // axis
        ErrorTerm = (XDelta % YDelta) - (YDelta << 1);

        // The initial and last runs are partial, because Y advances only 0.5
        // for these runs, rather than 1. Divide one full run, plus the
        // initial pixel, between the initial and last runs
        InitialPixelCount = (WholeStep >> 1) + 1;
        FinalPixelCount = InitialPixelCount;

        // If the basic run length is even and there's no fractional
        // advance, we have one pixel that could go to either the initial
        // or last partial run, which we'll arbitrarily allocate to the
        // last run
        if ((AdjUp == 0) && ((WholeStep & 0x01) == 0))
        {
            InitialPixelCount--;
        }
        // If there're an odd number of pixels per run, we have 1 pixel that can't
        // be allocated to either the initial or last partial run, so we'll add 0.5
        // to error term so this pixel will be handled by the normal full-run loop
        if ((WholeStep & 0x01) != 0)
        {
            ErrorTerm += YDelta;
        }
        // Draw the first, partial run of pixels
#ifdef kByteLevelTesting
        TestByte( (char *)dbyte, destPix, "\pHRUN1");
#endif
        mDrawHorizontalRun( dbyte, XAdvance, InitialPixelCount, Color, drowPlus, Temp);

#ifdef kByteLevelTesting
        TestByte( (char *)dbyte, destPix, "\pHRUN2");
#endif

        // Draw all full runs
        for (i=0; i<(YDelta-1); i++)
        {
            RunLength = WholeStep;  // run is at least this long
            // Advance the error term and add an extra pixel if the error
            // term so indicates
            if ((ErrorTerm += AdjUp) > 0)
            {
                RunLength++;
                ErrorTerm -= AdjDown;   // reset the error term
            }
            // Draw this scan line's run
#ifdef kByteLevelTesting
            TestByte( (char *)dbyte, destPix, "\pHRUN3");
#endif
            mDrawHorizontalRun( dbyte, XAdvance, RunLength, Color, drowPlus, Temp);

#ifdef kByteLevelTesting
            TestByte( (char *)dbyte, destPix, "\pHRUN4");
#endif
        }
        // Draw the final run of pixels
#ifdef kByteLevelTesting
        TestByte( (char *)dbyte, destPix, "\pHRUN5");
#endif
        mDrawHorizontalRun( dbyte, XAdvance, FinalPixelCount, Color, drowPlus, Temp);

#ifdef kByteLevelTesting
        TestByte( (char *)dbyte, destPix, "\pHRUN6");
#endif
        return;
    }
    else
    {
        // Y major line

        // Minimum # of pixels in a run in this line
        WholeStep = YDelta / XDelta;

        // Error term adjust each time X steps by 1; used to tell when 1 extra
        // pixel should be drawn as part of a run, to account for
        // fractional steps along the Y axis per 1-pixel steps along X
        AdjUp = (YDelta % XDelta) << 1;

        // Error term adjust when the error term turns over, used to factor
        // out the Y step made at that time
        AdjDown = XDelta << 1;

        // Initial error term; reflects initial step of 0.5 along the X axis
        ErrorTerm = (YDelta % XDelta) - (XDelta << 1);

        // The initial and last runs are partial, because X advances only 0.5
        // for these runs, rather than 1. Divide one full run, plus the
        // initial pixel, between the initial and last runs
        InitialPixelCount = (WholeStep >> 1) + 1;
        FinalPixelCount = InitialPixelCount;

        // If the basic run length is even and there's no fractional advance, we
        // have 1 pixel that could go to either the initial or last partial run,
        // which we'll arbitrarily allocate to the last run
        if ((AdjUp == 0) && ((WholeStep & 0x01) == 0))
        {
            InitialPixelCount--;
        }
        // If there are an odd number of pixels per run, we have one pixel
        // that can't be allocated to either the initial or last partial
        // run, so we'll add 0.5 to the error term so this pixel will be
        // handled by the normal full-run loop
        if ((WholeStep & 0x01) != 0)
        {
            ErrorTerm += XDelta;
        }
        // Draw the first, partial run of pixels
#ifdef kByteLevelTesting
        TestByte( (char *)dbyte, destPix, "\pVRUN1");
#endif
        mDrawVerticalRun( dbyte, XAdvance, InitialPixelCount, Color, drowPlus, Temp);

#ifdef kByteLevelTesting
        TestByte( (char *)dbyte, destPix, "\pVRUN2");
#endif
        // Draw all full runs
        for (i=0; i<(XDelta-1); i++)
        {
            RunLength = WholeStep;  // run is at least this long
            // Advance the error term and add an extra pixel if the error
            // term so indicates
            if ((ErrorTerm += AdjUp) > 0)
            {
                RunLength++;
                ErrorTerm -= AdjDown;   // reset the error term
            }
            // Draw this scan line's run
#ifdef kByteLevelTesting
            TestByte( (char *)dbyte, destPix, "\pVRUN3");
#endif
            mDrawVerticalRun( dbyte, XAdvance, RunLength, Color, drowPlus, Temp);

#ifdef kByteLevelTesting
            TestByte( (char *)dbyte, destPix, "\pVRUN4");
#endif
        }
        // Draw the final run of pixels
#ifdef kByteLevelTesting
        TestByte( (char *)dbyte, destPix, "\pVRUN5");
#endif
        mDrawVerticalRun( dbyte, XAdvance, FinalPixelCount, Color, drowPlus, Temp);

#ifdef kByteLevelTesting
        TestByte( (char *)dbyte, destPix, "\pVRUN6");
#endif
        return;
    }
}

// Copies a line from sourcemap to destmap between the specified endpoints.  hoff and voff are for
// the destPix only (for copying onscreen).

void CopyNateLine( PixMap *sourcePix, PixMap *destPix, Rect *clipRect,
                    long XStart, long YStart, long XEnd, long YEnd, long hoff,
                    long voff)
{
    long            Temp, AdjUp, AdjDown, ErrorTerm, XAdvance, XDelta, YDelta, drowPlus, srowPlus;
    long            WholeStep, InitialPixelCount, FinalPixelCount, i, RunLength;
    unsigned char   *sbyte, *dbyte;
    short           cs = mClipCode( XStart, YStart, *clipRect), ce = mClipCode( XEnd, YEnd, *clipRect);

    while ( cs | ce)
    {
        if ( cs & ce) return;
        XDelta = XEnd - XStart;
        YDelta = YEnd - YStart;
        if ( cs)
        {
            if ( cs & 8)
            {
                YStart += YDelta * ( clipRect->left - XStart) / XDelta;
                XStart = clipRect->left;
            } else
            if ( cs & 4)
            {
                YStart += YDelta * ( clipRect->right - 1 - XStart) / XDelta;
                XStart = clipRect->right - 1;
            } else
            if ( cs & 2)
            {
                XStart += XDelta * ( clipRect->top - YStart) / YDelta;
                YStart = clipRect->top;
            } else
            if ( cs & 1)
            {
                XStart += XDelta * ( clipRect->bottom - 1 - YStart) / YDelta;
                YStart = clipRect->bottom - 1;
            }
            cs = mClipCode( XStart, YStart, *clipRect);
        } else if ( ce)
        {
            if ( ce & 8)
            {
                YEnd += YDelta * ( clipRect->left - XEnd) / XDelta;
                XEnd = clipRect->left;
            } else
            if ( ce & 4)
            {
                YEnd += YDelta * ( clipRect->right - 1 - XEnd) / XDelta;
                XEnd = clipRect->right - 1;
            } else
            if ( ce & 2)
            {
                XEnd += XDelta * ( clipRect->top - YEnd) / YDelta;
                YEnd = clipRect->top;
            } else
            if ( ce & 1)
            {
                XEnd += XDelta * ( clipRect->bottom - 1 - YEnd) / YDelta;
                YEnd = clipRect->bottom - 1;
            }
            ce = mClipCode( XEnd, YEnd, *clipRect);
        }
    }

    // We'll always draw top to bottom, to reduce the number of cases we have to
    // handle, and to make lines between the same endpoints draw the same pixels
    if (YStart > YEnd)
    {
        Temp = YStart;
        YStart = YEnd;
        YEnd = Temp;
        Temp = XStart;
        XStart = XEnd;
        XEnd = Temp;
    }


    // Point to the bitmap address first pixel to draw
    drowPlus = destPix->rowBytes & 0x3fff;
    dbyte = destPix->baseAddr + (YStart + voff) * drowPlus +
                XStart + hoff;

    srowPlus = sourcePix->rowBytes & 0x3fff;
    sbyte = sourcePix->baseAddr + (YStart) * srowPlus + XStart;

    // Figure out whether we're going left or right, and how far we're
    // going horizontally
    if ((XDelta = XEnd - XStart) < 0)
    {
        XAdvance = -1;
        XDelta = -XDelta;
    }
    else
    {
        XAdvance = 1;
    }
    // Figure out how far we're going vertically
    YDelta = YEnd - YStart;

    // Special-case horizontal, vertical, and diagonal lines, for speed
    // and to avoid nasty boundary conditions and division by 0
    if (XDelta == 0)
    {
        // Vertical line
        for (i=0; i<=YDelta; i++)
        {
#ifdef kByteLevelTesting
            TestByte( (char *)dbyte, destPix, "\pVLINE");
#endif
            *dbyte = *sbyte;
            dbyte += drowPlus;
            sbyte += srowPlus;
        }
        return;
    }
    if (YDelta == 0)
    {
        // Horizontal line
        for (i=0; i<=XDelta; i++)
        {
#ifdef kByteLevelTesting
            TestByte( (char *)dbyte, destPix, "\pHLINE");
#endif
            *dbyte = *sbyte;
            dbyte += XAdvance;
            sbyte += XAdvance;
        }
        return;
    }
    if (XDelta == YDelta)
    {
        // Diagonal line
        for (i=0; i<=XDelta; i++)
        {
#ifdef kByteLevelTesting
            TestByte( (char *)dbyte, destPix, "\pDLINE");
#endif
            *dbyte = *sbyte;
            dbyte += XAdvance + drowPlus;
            sbyte += XAdvance + srowPlus;
        }
        return;
    }

    // Determine whether the line is X or Y major, and handle accordingly
    if (XDelta >= YDelta)
    {
        // X major line
        // Minimum # of pixels in a run in this line
        WholeStep = XDelta / YDelta;

        // Error term adjust each time Y steps by 1; used to tell when one
        // extra pixel should be drawn as part of a run, to account for
        // fractional steps along the X axis per 1-pixel steps along Y
        AdjUp = (XDelta % YDelta) << 1;

        // Error term adjust when the error term turns over, used to factor
        // out the X step made at that time
        AdjDown = YDelta * 2;

        // Initial error term; reflects an initial step of 0.5 along the Y
        // axis
        ErrorTerm = (XDelta % YDelta) - (YDelta << 1);

        // The initial and last runs are partial, because Y advances only 0.5
        // for these runs, rather than 1. Divide one full run, plus the
        // initial pixel, between the initial and last runs
        InitialPixelCount = (WholeStep >> 1) + 1;
        FinalPixelCount = InitialPixelCount;

        // If the basic run length is even and there's no fractional
        // advance, we have one pixel that could go to either the initial
        // or last partial run, which we'll arbitrarily allocate to the
        // last run
        if ((AdjUp == 0) && ((WholeStep & 0x01) == 0))
        {
            InitialPixelCount--;
        }
        // If there're an odd number of pixels per run, we have 1 pixel that can't
        // be allocated to either the initial or last partial run, so we'll add 0.5
        // to error term so this pixel will be handled by the normal full-run loop
        if ((WholeStep & 0x01) != 0)
        {
            ErrorTerm += YDelta;
        }
        // Draw the first, partial run of pixels
#ifdef kByteLevelTesting
        TestByte( (char *)dbyte, destPix, "\pHRUN1");
#endif
        mCopyHorizontalRun( dbyte, sbyte, XAdvance, InitialPixelCount, drowPlus, srowPlus, Temp);

#ifdef kByteLevelTesting
        TestByte( (char *)dbyte, destPix, "\pHRUN2");
#endif

        // Draw all full runs
        for (i=0; i<(YDelta-1); i++)
        {
            RunLength = WholeStep;  // run is at least this long
            // Advance the error term and add an extra pixel if the error
            // term so indicates
            if ((ErrorTerm += AdjUp) > 0)
            {
                RunLength++;
                ErrorTerm -= AdjDown;   // reset the error term
            }
            // Draw this scan line's run
#ifdef kByteLevelTesting
            TestByte( (char *)dbyte, destPix, "\pHRUN3");
#endif
            mCopyHorizontalRun( dbyte, sbyte, XAdvance, RunLength, drowPlus, srowPlus, Temp);

#ifdef kByteLevelTesting
            TestByte( (char *)dbyte, destPix, "\pHRUN4");
#endif
        }
        // Draw the final run of pixels
#ifdef kByteLevelTesting
        TestByte( (char *)dbyte, destPix, "\pHRUN5");
#endif
        mCopyHorizontalRun( dbyte, sbyte, XAdvance, FinalPixelCount, drowPlus, srowPlus, Temp);

#ifdef kByteLevelTesting
        TestByte( (char *)dbyte, destPix, "\pHRUN6");
#endif
        return;
    }
    else
    {
        // Y major line

        // Minimum # of pixels in a run in this line
        WholeStep = YDelta / XDelta;

        // Error term adjust each time X steps by 1; used to tell when 1 extra
        // pixel should be drawn as part of a run, to account for
        // fractional steps along the Y axis per 1-pixel steps along X
        AdjUp = (YDelta % XDelta) << 1;

        // Error term adjust when the error term turns over, used to factor
        // out the Y step made at that time
        AdjDown = XDelta << 1;

        // Initial error term; reflects initial step of 0.5 along the X axis
        ErrorTerm = (YDelta % XDelta) - (XDelta << 1);

        // The initial and last runs are partial, because X advances only 0.5
        // for these runs, rather than 1. Divide one full run, plus the
        // initial pixel, between the initial and last runs
        InitialPixelCount = (WholeStep >> 1) + 1;
        FinalPixelCount = InitialPixelCount;

        // If the basic run length is even and there's no fractional advance, we
        // have 1 pixel that could go to either the initial or last partial run,
        // which we'll arbitrarily allocate to the last run
        if ((AdjUp == 0) && ((WholeStep & 0x01) == 0))
        {
            InitialPixelCount--;
        }
        // If there are an odd number of pixels per run, we have one pixel
        // that can't be allocated to either the initial or last partial
        // run, so we'll add 0.5 to the error term so this pixel will be
        // handled by the normal full-run loop
        if ((WholeStep & 0x01) != 0)
        {
            ErrorTerm += XDelta;
        }
        // Draw the first, partial run of pixels
#ifdef kByteLevelTesting
        TestByte( (char *)dbyte, destPix, "\pVRUN1");
#endif
        mCopyVerticalRun( dbyte, sbyte, XAdvance, InitialPixelCount, drowPlus, srowPlus, Temp);

#ifdef kByteLevelTesting
        TestByte( (char *)dbyte, destPix, "\pVRUN2");
#endif
        // Draw all full runs
        for (i=0; i<(XDelta-1); i++)
        {
            RunLength = WholeStep;  // run is at least this long
            // Advance the error term and add an extra pixel if the error
            // term so indicates
            if ((ErrorTerm += AdjUp) > 0)
            {
                RunLength++;
                ErrorTerm -= AdjDown;   // reset the error term
            }
            // Draw this scan line's run
#ifdef kByteLevelTesting
            TestByte( (char *)dbyte, destPix, "\pVRUN3");
#endif
            mCopyVerticalRun( dbyte, sbyte, XAdvance, RunLength, drowPlus, srowPlus, Temp);

#ifdef kByteLevelTesting
            TestByte( (char *)dbyte, destPix, "\pVRUN4");
#endif
        }
        // Draw the final run of pixels
#ifdef kByteLevelTesting
        TestByte( (char *)dbyte, destPix, "\pVRUN5");
#endif
        mCopyVerticalRun( dbyte, sbyte, XAdvance, FinalPixelCount, drowPlus, srowPlus, Temp);

#ifdef kByteLevelTesting
        TestByte( (char *)dbyte, destPix, "\pVRUN6");
#endif
        return;
    }
}

// Draws a horizontal run of pixels, then advances the bitmap pointer to
// the first pixel of the next run.

/*
void DrawHorizontalRun( unsigned char **dbyte, long XAdvance,
    long RunLength, long Color, long drowPlus)
{
    int             i;
    unsigned char   *Workingdbyte = *dbyte;

    for (i=0; i < RunLength; i++)
    {
        *Workingdbyte = Color;
        Workingdbyte += XAdvance;
    }
    // Advance to the next scan line
    Workingdbyte += SCREEN_WIDTH;
    *dbyte = Workingdbyte;
}
*/

// Draws a vertical run of pixels, then advances the bitmap pointer to
// the first pixel of the next run. */

/*
void DrawVerticalRun(char far **dbyte, int XAdvance,
    int RunLength, int Color)
{
    int i;
    char far *Workingdbyte = *dbyte;

    for (i=0; i<RunLength; i++)
    {
        *Workingdbyte = Color;
        Workingdbyte += SCREEN_WIDTH;
    }
// Advance to the next column
    Workingdbyte += XAdvance;
    *dbyte = Workingdbyte;
}
*/

void DashNateLine( PixMap *destPix, Rect *clipRect, long XStart, long YStart, long XEnd,
                    long YEnd, long hoff, long voff, unsigned char Color, unsigned char dashon,
                    unsigned char dashoff, unsigned char dashcount)
{
    long            Temp, AdjUp, AdjDown, ErrorTerm, XAdvance, XDelta, YDelta, drowPlus;
    long            WholeStep, InitialPixelCount, FinalPixelCount, i, RunLength;
    unsigned char   *dbyte;
    short           cs = mClipCode( XStart, YStart, *clipRect), ce = mClipCode( XEnd, YEnd, *clipRect);

    dashoff += dashon;
    dashcount %= dashoff;

    while ( cs | ce)
    {
        if ( cs & ce) return;
        XDelta = XEnd - XStart;
        YDelta = YEnd - YStart;
        if ( cs)
        {
            if ( cs & 8)
            {
                YStart += YDelta * ( clipRect->left - XStart) / XDelta;
                XStart = clipRect->left;
            } else
            if ( cs & 4)
            {
                YStart += YDelta * ( clipRect->right - 1 - XStart) / XDelta;
                XStart = clipRect->right - 1;
            } else
            if ( cs & 2)
            {
                XStart += XDelta * ( clipRect->top - YStart) / YDelta;
                YStart = clipRect->top;
            } else
            if ( cs & 1)
            {
                XStart += XDelta * ( clipRect->bottom - 1 - YStart) / YDelta;
                YStart = clipRect->bottom - 1;
            }
            cs = mClipCode( XStart, YStart, *clipRect);
        } else if ( ce)
        {
            if ( ce & 8)
            {
                YEnd += YDelta * ( clipRect->left - XEnd) / XDelta;
                XEnd = clipRect->left;
            } else
            if ( ce & 4)
            {
                YEnd += YDelta * ( clipRect->right - 1 - XEnd) / XDelta;
                XEnd = clipRect->right - 1;
            } else
            if ( ce & 2)
            {
                XEnd += XDelta * ( clipRect->top - YEnd) / YDelta;
                YEnd = clipRect->top;
            } else
            if ( ce & 1)
            {
                XEnd += XDelta * ( clipRect->bottom - 1 - YEnd) / YDelta;
                YEnd = clipRect->bottom - 1;
            }
            ce = mClipCode( XEnd, YEnd, *clipRect);
        }
    }

   // We'll always draw top to bottom, to reduce the number of cases we have to
   // handle, and to make lines between the same endpoints draw the same pixels
    if (YStart > YEnd)
    {
        Temp = YStart;
        YStart = YEnd;
        YEnd = Temp;
        Temp = XStart;
        XStart = XEnd;
        XEnd = Temp;
    }


    // Point to the bitmap address first pixel to draw
    drowPlus = destPix->rowBytes & 0x3fff;
    dbyte = destPix->baseAddr + (YStart + voff) * drowPlus +
                XStart + hoff;

    // Figure out whether we're going left or right, and how far we're
    // going horizontally
    if ((XDelta = XEnd - XStart) < 0)
    {
        XAdvance = -1;
        XDelta = -XDelta;
    }
    else
    {
        XAdvance = 1;
    }
    // Figure out how far we're going vertically
    YDelta = YEnd - YStart;

    // Special-case horizontal, vertical, and diagonal lines, for speed
    // and to avoid nasty boundary conditions and division by 0
    if (XDelta == 0)
    {
        // Vertical line
/*      for (i=0; i<=YDelta; i++)
        {
            *dbyte = Color;
            dbyte += drowPlus;
        }
*/      i = 0;
        while ( i <= YDelta)
        {
            while (( dashcount < dashon) && ( i <= YDelta))
            {
                *dbyte = Color;
                dbyte += drowPlus;
                i++;
                dashcount++;
            }
            while (( dashcount < dashoff) && ( i <= YDelta))
            {
                dbyte += drowPlus;
                i++;
                dashcount++;
            }
            if ( dashcount == dashoff) dashcount = 0;
        }

        return;
    }
    if (YDelta == 0)
    {
        // Horizontal line
/*      for (i=0; i<=XDelta; i++)
        {
            *dbyte = Color;
            dbyte += XAdvance;
        }
*/      i = 0;
        while ( i <= XDelta)
        {
            while (( dashcount < dashon) && ( i <= XDelta))
            {
                *dbyte = Color;
                dbyte += XAdvance;
                i++;
                dashcount++;
            }
            while (( dashcount < dashoff) && ( i <= XDelta))
            {
                dbyte += XAdvance;
                i++;
                dashcount++;
            }
            if ( dashcount == dashoff) dashcount = 0;
        }
        return;
    }
    if (XDelta == YDelta)
    {
        // Diagonal line
/*      for (i=0; i<=XDelta; i++)
        {
            *dbyte = Color;
            dbyte += XAdvance + drowPlus;
        }
*/      i = 0;
        while ( i <= XDelta)
        {
            while (( dashcount < dashon) && ( i <= XDelta))
            {
                *dbyte = Color;
                dbyte += XAdvance + drowPlus;
                i++;
                dashcount++;
            }
            while (( dashcount < dashoff) && ( i <= XDelta))
            {
                dbyte += XAdvance + drowPlus;
                i++;
                dashcount++;
            }
            if ( dashcount == dashoff) dashcount = 0;
        }
        return;
    }

    // Determine whether the line is X or Y major, and handle accordingly
    if (XDelta >= YDelta)
    {
        // X major line
        // Minimum # of pixels in a run in this line
        WholeStep = XDelta / YDelta;

        // Error term adjust each time Y steps by 1; used to tell when one
        // extra pixel should be drawn as part of a run, to account for
        // fractional steps along the X axis per 1-pixel steps along Y
        AdjUp = (XDelta % YDelta) << 1;

        // Error term adjust when the error term turns over, used to factor
        // out the X step made at that time
        AdjDown = YDelta * 2;

        // Initial error term; reflects an initial step of 0.5 along the Y
        // axis
        ErrorTerm = (XDelta % YDelta) - (YDelta << 1);

        // The initial and last runs are partial, because Y advances only 0.5
        // for these runs, rather than 1. Divide one full run, plus the
        // initial pixel, between the initial and last runs
        InitialPixelCount = (WholeStep >> 1) + 1;
        FinalPixelCount = InitialPixelCount;

        // If the basic run length is even and there's no fractional
        // advance, we have one pixel that could go to either the initial
        // or last partial run, which we'll arbitrarily allocate to the
        // last run
        if ((AdjUp == 0) && ((WholeStep & 0x01) == 0))
        {
            InitialPixelCount--;
        }
        // If there're an odd number of pixels per run, we have 1 pixel that can't
        // be allocated to either the initial or last partial run, so we'll add 0.5
        // to error term so this pixel will be handled by the normal full-run loop
        if ((WholeStep & 0x01) != 0)
        {
            ErrorTerm += YDelta;
        }
        // Draw the first, partial run of pixels
        mDashHorizontalRun( dbyte, XAdvance, InitialPixelCount, Color, drowPlus, Temp, dashon, dashoff, dashcount);


        // Draw all full runs
        for (i=0; i<(YDelta-1); i++)
        {
            RunLength = WholeStep;  // run is at least this long
            // Advance the error term and add an extra pixel if the error
            // term so indicates
            if ((ErrorTerm += AdjUp) > 0)
            {
                RunLength++;
                ErrorTerm -= AdjDown;   // reset the error term
            }
            // Draw this scan line's run
            mDashHorizontalRun( dbyte, XAdvance, RunLength, Color, drowPlus, Temp, dashon, dashoff, dashcount);

        }
        // Draw the final run of pixels
        mDashHorizontalRun( dbyte, XAdvance, FinalPixelCount, Color, drowPlus, Temp, dashon, dashoff, dashcount);

        return;
    }
    else
    {
        // Y major line

        // Minimum # of pixels in a run in this line
        WholeStep = YDelta / XDelta;

        // Error term adjust each time X steps by 1; used to tell when 1 extra
        // pixel should be drawn as part of a run, to account for
        // fractional steps along the Y axis per 1-pixel steps along X
        AdjUp = (YDelta % XDelta) << 1;

        // Error term adjust when the error term turns over, used to factor
        // out the Y step made at that time
        AdjDown = XDelta << 1;

        // Initial error term; reflects initial step of 0.5 along the X axis
        ErrorTerm = (YDelta % XDelta) - (XDelta << 1);

        // The initial and last runs are partial, because X advances only 0.5
        // for these runs, rather than 1. Divide one full run, plus the
        // initial pixel, between the initial and last runs
        InitialPixelCount = (WholeStep >> 1) + 1;
        FinalPixelCount = InitialPixelCount;

        // If the basic run length is even and there's no fractional advance, we
        // have 1 pixel that could go to either the initial or last partial run,
        // which we'll arbitrarily allocate to the last run
        if ((AdjUp == 0) && ((WholeStep & 0x01) == 0))
        {
            InitialPixelCount--;
        }
        // If there are an odd number of pixels per run, we have one pixel
        // that can't be allocated to either the initial or last partial
        // run, so we'll add 0.5 to the error term so this pixel will be
        // handled by the normal full-run loop
        if ((WholeStep & 0x01) != 0)
        {
            ErrorTerm += XDelta;
        }
        // Draw the first, partial run of pixels
        mDashVerticalRun( dbyte, XAdvance, InitialPixelCount, Color, drowPlus, Temp, dashon, dashoff, dashcount);

        // Draw all full runs
        for (i=0; i<(XDelta-1); i++)
        {
            RunLength = WholeStep;  // run is at least this long
            // Advance the error term and add an extra pixel if the error
            // term so indicates
            if ((ErrorTerm += AdjUp) > 0)
            {
                RunLength++;
                ErrorTerm -= AdjDown;   // reset the error term
            }
            // Draw this scan line's run
            mDashVerticalRun( dbyte, XAdvance, RunLength, Color, drowPlus, Temp, dashon, dashoff, dashcount);

        }
        // Draw the final run of pixels
        mDashVerticalRun( dbyte, XAdvance, FinalPixelCount, Color, drowPlus, Temp, dashon, dashoff, dashcount);

        return;
    }
}

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

#ifndef ANTARES_NATE_DRAW_HPP_
#define ANTARES_NATE_DRAW_HPP_

// Natedraw.h

#include <algorithm>
#include <Base.h>
#include <Quickdraw.h>

class BinaryReader;

inline void mGetRowBytes(long& rBytes, PixMap* pix) {
    rBytes = pix->rowBytes & 0x3fff;
}

inline void mSetNatePixel(uint8_t*& mdestByte, long mrowBytes, long mx, long my, long mxoff, long myoff, PixMap* mdestPix, uint8_t mcolor) {
    mdestByte = mdestPix->baseAddr + (my + myoff) * mrowBytes + mx + mxoff;
    *mdestByte = mcolor;
}

inline void mGetNatePixel(uint8_t*& destByte, long rowBytes, long x, long y, long xoff, long yoff, PixMap* destPix) {
    destByte = destPix->baseAddr + (y + yoff) * rowBytes + x + xoff;
}

inline void mBiggestRect(Rect& mdrect, const Rect& morect) {
    mdrect.left = std::min(mdrect.left, morect.left);
    mdrect.top = std::min(mdrect.top, morect.top);
    mdrect.right = std::max(mdrect.right, morect.right);
    mdrect.bottom = std::max(mdrect.bottom, morect.bottom);
}

inline void mCopyAnyRect(Rect& mdrect, const Rect& msrect) {
    mdrect.left = msrect.left;
    mdrect.top = msrect.top;
    mdrect.right = msrect.right;
    mdrect.bottom = msrect.bottom;
}

typedef unsigned long   coordType;

struct coordPointType
{
    coordType   h;
    coordType   v;
};

struct longPointType {
    int32_t     h;
    int32_t     v;

    void read(BinaryReader* bin);
};

void DrawNateRect( PixMap *, Rect *, long, long, unsigned char);
void DrawNateRectVScan( PixMap *, Rect *, long, long, unsigned char);
void DrawNateRectClipped( PixMap *, Rect *, Rect *, long, long, unsigned char);

void DrawNateTriangleUpClipped( PixMap *, Rect *, Rect *, long, long, unsigned char);
void DrawNatePlusClipped( PixMap *, Rect *, Rect *, long, long, unsigned char);
void DrawNateSquareClipped( PixMap *, Rect *, Rect *, long, long, unsigned char);
void DrawNateDiamondClipped( PixMap *, Rect *, Rect *, long, long, unsigned char);
void DrawNateVBracket( PixMap *, Rect *, Rect *, long, long, unsigned char);
void DrawNateShadedRect( PixMap *, Rect *, Rect *, long, long, unsigned char, unsigned char,
                    unsigned char);
void BiggestRect( Rect  *, Rect *);
void LongRectToRect( Rect *, Rect *);
void RectToLongRect( Rect *, Rect *);
void SetLongRect( Rect *, long, long, long, long);
void DrawNateLine( PixMap *, Rect *, long, long, long,
                    long, long, long, unsigned char);
void CopyNateLine( PixMap *, PixMap *, Rect *, long, long, long, long , long, long);
void DashNateLine( PixMap *, Rect *, long, long, long,
                    long, long, long, unsigned char, unsigned char, unsigned char, unsigned char);

#endif // ANTARES_NATE_DRAW_HPP_

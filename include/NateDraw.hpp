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

namespace antares {

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

struct coordPointType {
    uint32_t    h;
    uint32_t    v;
};

void DrawNateRect( PixMap *, Rect *, long, long, unsigned char);
void DrawNateRectVScan( PixMap *, Rect *, long, long, unsigned char);
void DrawNateRectClipped( PixMap *, Rect *, const Rect&, long, long, unsigned char);

void DrawNateTriangleUpClipped( PixMap *, Rect *, const Rect&, long, long, unsigned char);
void DrawNatePlusClipped( PixMap *, Rect *, const Rect&, long, long, unsigned char);
void DrawNateSquareClipped( PixMap *, Rect *, const Rect&, long, long, unsigned char);
void DrawNateDiamondClipped( PixMap *, Rect *, const Rect&, long, long, unsigned char);
void DrawNateVBracket(PixMap *, const Rect&, const Rect&, long, long, unsigned char);
void DrawNateShadedRect( PixMap *, Rect *, const Rect&, long, long, unsigned char, unsigned char,
                    unsigned char);
void BiggestRect(Rect*, const Rect&);
void DrawNateLine(PixMap *, const Rect&, long, long, long,
                    long, long, long, unsigned char);
void CopyNateLine( PixMap *, PixMap *, const Rect&, long, long, long, long , long, long);
void DashNateLine(PixMap *, const Rect&, long, long, long,
                    long, long, long, unsigned char, unsigned char, unsigned char, unsigned char);

}  // namespace antares

#endif // ANTARES_NATE_DRAW_HPP_

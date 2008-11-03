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

#pragma options align=mac68k

#define mGetRowBytes( rBytes, msourcePix) rBytes = (msourcePix)->rowBytes & 0x3fff;

#define mSetNatePixel( mdestByte, mrowBytes, mx, my, mxoff, myoff, mdestPix, mcolor) \
mdestByte = (unsigned char *)(mdestPix)->baseAddr + (long)((my) + (myoff)) * (mrowBytes) + (long)(mx) + (long)(mxoff);\
*mdestByte = (mcolor);

#define mGetNatePixel( destByte, rowBytes, x, y, xoff, yoff, destPix) \
destByte = (unsigned char *)(destPix)->baseAddr + (long)((y) + (yoff)) * (rowBytes) + (long)(x) + (long)(xoff);

#define mBiggestRect( mdrect, morect) \
if ( (morect).left < (mdrect).left) (mdrect).left = (morect).left;\
if ( (morect).top < (mdrect).top) (mdrect).top = (morect).top;\
if ( (morect).right > (mdrect).right) (mdrect).right = (morect).right;\
if ( (morect).bottom > (mdrect).bottom) (mdrect).bottom = (morect).bottom;

#define mCopyAnyRect( mdrect, msrect)\
(mdrect).left = (msrect).left;\
(mdrect).top = (msrect).top;\
(mdrect).right = (msrect).right;\
(mdrect).bottom = (msrect).bottom;

typedef unsigned long   coordType;

typedef struct
{
    coordType   h;
    coordType   v;
}   coordPointType;

typedef struct
{
    long        h;
    long        v;
} longPointType;

typedef struct longRectStruct {
    long        left;
    long        top;
    long        right;
    long        bottom;
    } longRect;

void DrawNateRect( PixMap *, longRect *, long, long, unsigned char);
void DrawNateRectVScan( PixMap *, longRect *, long, long, unsigned char);
void DrawNateRectClipped( PixMap *, longRect *, longRect *, long, long, unsigned char);
void DrawNateRectVScanClipped( PixMap *, longRect *, longRect *, long, long, unsigned char);

void DrawNateTriangleUpClipped( PixMap *, longRect *, longRect *, long, long, unsigned char);
void DrawNatePlusClipped( PixMap *, longRect *, longRect *, long, long, unsigned char);
void DrawNateSquareClipped( PixMap *, longRect *, longRect *, long, long, unsigned char);
void DrawNateDiamondClipped( PixMap *, longRect *, longRect *, long, long, unsigned char);
void DrawNateVBracket( PixMap *, longRect *, longRect *, long, long, unsigned char);
void DrawNateShadedRect( PixMap *, longRect *, longRect *, long, long, unsigned char, unsigned char,
                    unsigned char);
void BiggestRect( Rect  *, Rect *);
void LongRectToRect( longRect *, Rect *);
void RectToLongRect( Rect *, longRect *);
void SetLongRect( longRect *, long, long, long, long);
void DrawNateLine( PixMap *, longRect *, long, long, long,
                    long, long, long, unsigned char);
void CopyNateLine( PixMap *, PixMap *, longRect *, long, long, long, long , long, long);
void DashNateLine( PixMap *, longRect *, long, long, long,
                    long, long, long, unsigned char, unsigned char, unsigned char, unsigned char);

#pragma options align=reset

#endif // ANTARES_NATE_DRAW_HPP_

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

#ifndef ANTARES_NATE_PIX_HPP_
#define ANTARES_NATE_PIX_HPP_

/* NATEPIX.H */

typedef unsigned char   colorByte;
typedef unsigned char   byte;

#pragma options align=mac68k

typedef struct {
    PixMapHandle    pix;
    Handle          pixBase;
    Point           center;
    } natePix;

typedef struct {
    BitMap          bit;
    Handle          bitBase;
    Point           center;
    } nateBit;

#define kNatePix

/* typedef of BitMap (I-144):

typedef struct {
    baseAddr        Ptr;    (should be set to *bitBlock)
    rowBytes        int;    (holds correct row width in bytes - no mask)
    bounds          Rect;   (should be same as PixMap's)
    } BitMap;

    typedef of PixMap (V-53-54):

typedef *PixMapPtr  PixMapHandle;
typedef *PixMap     PixMapPtr;
typedef struct {
    baseAddr        Ptr;    (should be set to *pixBlock)
    rowBytes        int;    (must be ORed with ROW_BYTES_MASK for PixMap)
    bounds          Rect;   (should be same is BitMap's)
    pmVersion       int;    ( = 0)
    packType        int;    ( = 0)
    packSize        int;    ( = 0)
    hRes            Fixed;  ( = 72)
    vRes            Fixed;  ( = 72)
    pixelType       int;    ( = 0)
    pixelSize       int;    ( = 8)
    cmpCount        int;    ( = 1)
    cmpSize         int;    ( = 8)
    planeBytes      long;   ( = 0)
    pmTable         CTabHandle; (first dispose the pmTable which is auto-
                                matically allocated to a new PixMap, then
                                assign its pmTable = GetCTable( CTABLE_ID))
    pmReserved      long;   ( = 0)

*/


#pragma options align=reset

#endif // ANTARES_NATE_PIX_HPP_

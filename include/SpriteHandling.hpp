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

#ifndef ANTARES_SPRITE_HANDLING_HPP_
#define ANTARES_SPRITE_HANDLING_HPP_

// SpriteHandler.h

#include <Base.h>

#include "NateDraw.hpp"

#define kNoSpriteTable  -1

#define ROW_BYTES_MASK  0x8000
#define kPixResType             'SMIV'

#define BLOCK_TYPE      0
#define MASK_TYPE       1
#define COMP_PIX_TYPE   2
#define RUN_LENGTH_TYPE 3
#define OLD_SCALE_TYPE  4
#define ASM_SCALE_TYPE  5
#define ASM_LENGTH_TYPE 6
#define EXPERIMENT_TYPE 7

#define kSpriteTableColorShift  implicit_cast<short>(11)
#define kSpriteTableColorIDMask 0x7800  // bits 11-14
// this makes the max legal sprite id 2047

#define SCALE_SCALE     4096//256
#define MIN_SCALE       256//16
#define MAX_SCALE       32768//2048
#define MAX_SCALE_PIX   32      // the maximum size a single scaled pixel can be
                                // (should be 32)

#define kOneSixteenthScale      256
#define kOneEighthScale         512
#define kOneQuarterScale        1024//64
#define kThreeQuarterScale      3072//192
#define kOneHalfScale           2048//128
#define kOneAndAQuarterScale    5120//320
#define kOneAndAHalfScale       6144//384
#define kTimesTwoScale          8192//512

#define SHIFT_SCALE             12//8
#define ASM_SHIFT_SCALE_1       8
#define ASM_SHIFT_SCALE_2       4

#define kNoTinyColor            0xff

#define kNoSpriteLayer          0
#define kFirstSpriteLayer       1
#define kMiddleSpriteLayer      2
#define kLastSpriteLayer        3

#define MAX_PIX_SIZE    480

#define kMaxPixTableEntry   60
#define kNoSprite               -1

enum spriteStyleType {
    spriteNormal = 0,
    spriteStatic = 1,
    spriteColor = 2
};

struct spritePix {
    Point       center;
    int         width;
    int         height;
    int         type;
    Handle      data;
    };

//typedef  spritePix;

struct natePixType;
struct spriteType {
    Point           where;
    natePixType**   table;
    short           resID;
    int             whichShape;
    long            scale;
    spriteStyleType style;
    unsigned char   styleColor;
    short           styleData;
    long            tinySize;
    short           whichLayer;
    unsigned char   tinyColor;
    Boolean         killMe;

    Rect        thisRect;
    Rect        lastRect;
    };


struct pixTableType {
    natePixType**   resource;
    int         resID;
    Boolean     keepMe;
    };

struct natePixType;

void SpriteHandlingInit( void);
void CleanupSpriteHandling( void);
void ResetAllSprites( void);
void CreateSpritePixFromPixMap( spritePix *, int type, PixMapHandle, Rect *);
void RunLengthSpritePixInPixMap( spritePix *, Point, PixMapHandle);
void OptScaleSpritePixInPixMap( spritePix *, Point, long, longRect *, longRect *, PixMapHandle);
void StaticScaleSpritePixInPixMap( spritePix *, Point, long, longRect *, longRect *,
    PixMapHandle, short);
void ColorScaleSpritePixInPixMap( spritePix *, Point, long, longRect *, longRect *,
    PixMapHandle, short, unsigned char, unsigned char);
void OutlineScaleSpritePixInPixMap( spritePix *sprite, Point where, long scale, longRect *dRect,
        longRect *clipRect, PixMapHandle pixMap, unsigned char colorOut,
        unsigned char colorIn);
void ResetAllPixTables( void);
void SetAllPixTablesNoKeep( void);
void KeepPixTable( short);
void RemoveAllUnusedPixTables( void);
natePixType** AddPixTable( short);
natePixType** GetPixTable( short);
spriteType *AddSprite( Point, natePixType**, short, short, long, long, short, unsigned char, long *);
void RemoveSprite( spriteType *);
void EraseSpriteTable( void);
void DrawSpriteTableInOffWorld( longRect *);
void GetOldSpritePixData( spriteType *, spritePix *);
void ShowSpriteTable( void);
void CullSprites( void);
void  PixMapTest( spritePix *, Point, long, longRect *, longRect *, PixMapHandle);
void TestByte(unsigned char*, PixMap*, unsigned char*);
void ResolveScaleMapData( Handle);
int Randomize( int);

#endif // ANTARES_SPRITE_HANDLING_HPP_

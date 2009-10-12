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

// SpriteHandling

#include "SpriteHandling.hpp"

#include "AresGlobalType.hpp"
#include "ColorTranslation.hpp"
#include "Debug.hpp"
#include "DirectText.hpp"
#include "Error.hpp"
#include "MathMacros.hpp"
#include "NateDraw.hpp"
#include "NatePixTable.hpp"
#include "OffscreenGWorld.hpp"
#include "Randomize.hpp" // for static table
#include "Resources.h"
#include "Rotation.hpp"
#include "StringNumerics.hpp"

namespace antares {

#define kMaxSpriteNum           500//300

#define kDefaultBoxSize         3

#define kSpriteHandleError      "\pSPHD"

#define kSpriteResFileName      "\p:Ares Data Folder:Ares Sprites"

#define kSpriteBlipThreshhold   kOneQuarterScale
//#define   kBlipSpriteID           1000                // table we need for blips
//#define   kBlipRotDiv             45                  // divide real angle by this to get blip
//#define   kBlipShapeNum           9                   // each color blip has this many shapes
//#define   kBlipRotOffset          1                   // add this to color * blip shape num for 1st rot shape

#define kMinVolatilePixTable    1                   // sound 0 is always there; 1+ is volatile

//#define   kDrawOverride
//#define   kByteLevelTesting

// assembly struct offsets

#define kSpriteTypeWidth        0x0004
#define kSpriteTypeHeight       0x0008
#define kLongRectTop            0x0004
#define kLongRectRight          0x0008
#define kLongRectBottom         0x000c
#define kPixMapRowBytes         0x0004

#define kSolidSquareBlip        0x00000000
#define kTriangeUpBlip          0x00000010
#define kDiamondBlip            0x00000020
#define kPlusBlip               0x00000030
#define kFramedSquareBlip       0x00000040

#define kBlipSizeMask           0x0000000f
#define kBlipTypeMask           0x000000f0

#define kStaticTableSize        2000

extern WindowPtr        gTheWindow;
extern long             gNatePortLeft, gNatePortTop;

long                    *gScaleHMap = nil, *gScaleVMap = nil, gAbsoluteScale = MIN_SCALE;
pixTableType            gPixTable[ kMaxPixTableEntry];
scoped_array<spriteType> gSpriteTable;
scoped_array<long> gBothScaleMaps;
scoped_array<unsigned char> gStaticTable;
short                   gSpriteFileRefID = 0;

extern PixMap* gActiveWorld;
extern PixMap* gOffWorld;
extern PixMap* gSaveWorld;

bool PixelInSprite_IsOutside( spritePix *sprite, long x, long y, long *hmap,
long *vamp);

void SpriteHandlingInit() {
    int             i, j;
    unsigned char   *staticValue = nil;

    gBothScaleMaps.reset(new long[MAX_PIX_SIZE * 2]);
    gScaleHMap = gBothScaleMaps.get();
    gScaleVMap = gBothScaleMaps.get() + MAX_PIX_SIZE;

    ResetAllPixTables();

    gSpriteTable.reset(new spriteType[kMaxSpriteNum]);
    ResetAllSprites();

    gStaticTable.reset(new unsigned char[kStaticTableSize * 2]);
    staticValue = gStaticTable.get();
    for (i = 0; i < (kStaticTableSize * 2); i++) {
        j = Randomize( 256);
        *staticValue = j;
        staticValue++;
    }
}

void ResetAllSprites( void)

{
    spriteType  *aSprite;
    short       i;

    aSprite = gSpriteTable.get();
    for ( i = 0; i < kMaxSpriteNum; i++)
    {
        aSprite->resID = -1;
        aSprite->killMe = false;
        aSprite->thisRect.left = aSprite->thisRect.top = aSprite->thisRect.right =
                aSprite->thisRect.bottom = aSprite->lastRect.left = aSprite->lastRect.top =
                aSprite->lastRect.right = aSprite->lastRect.bottom = 0;
        aSprite->whichLayer = kNoSpriteLayer;
        aSprite->style = spriteNormal;
        aSprite->styleColor = 0x00;
        aSprite->styleData = 0;
        aSprite++;
    }

}

void ResetAllPixTables( void)

{
    short   i;

    for ( i = 0; i < kMaxPixTableEntry; i++)
    {
        if (gPixTable[i].resource.get() != nil) {
            gPixTable[i].resource.destroy();
        }
        gPixTable[i].keepMe = false;
        gPixTable[i].resID = -1;
    }
}

void CleanupSpriteHandling( void)

{
    int i;

    gBothScaleMaps.reset();
//  CloseResFile( gSpriteFileRefID);
    for ( i = 0; i < kMaxPixTableEntry; i++)
    {
        if (gPixTable[i].resource.get() != nil) {
//          mHandleDisposeAndDeregister( gPixTable[i].resource);
            gPixTable[i].resource.destroy();
        }
    }
    gSpriteTable.reset();
}

void SetAllPixTablesNoKeep( void)

{
    short   i;

    for ( i = kMinVolatilePixTable; i < kMaxPixTableEntry; i++)
        gPixTable[i].keepMe = false;
}

void KeepPixTable( short resID)

{
    short   i = 0;

    while (( gPixTable[i].resID != resID) && ( i < kMaxPixTableEntry)) i++;
    if ( i != kMaxPixTableEntry) gPixTable[i].keepMe = true;
}

void RemoveAllUnusedPixTables( void)

{
    short   i;

    for ( i = kMinVolatilePixTable; i < kMaxPixTableEntry; i++)
    {
        if ((gPixTable[i].keepMe == false) && (gPixTable[i].resource.get() != nil)) {
            gPixTable[i].resource.destroy();
            gPixTable[i].keepMe = false;
            gPixTable[i].resID = -1;
        }
    }

}

TypedHandle<natePixType> AddPixTable( short resID)
{
    short           i = 0, realResID = resID;
    short           color = 0;

    mWriteDebugString("\pADDPIX < HANDLE");
    while ((!((gPixTable[i].resource.get() != nil) && (gPixTable[i].resID == resID)))
            && (i < kMaxPixTableEntry)) {
        i++;
    }
    if ( i == kMaxPixTableEntry)
    {
        i = 0;
        while ((gPixTable[i].resource.get() != nil) && ( i < kMaxPixTableEntry)) {
            i++;
        }
        if ( i == kMaxPixTableEntry)
        {
//          Debugger();
            ShowErrorAny( eExitToShellErr, kErrorStrID, nil, nil, nil, nil, kNoMoreSpriteTablesError, -1, -1, -1, __FILE__, 111);
        }

        if ( realResID & kSpriteTableColorIDMask)
        {
            realResID &= ~kSpriteTableColorIDMask;
            color = ( resID & kSpriteTableColorIDMask) >> kSpriteTableColorShift;
            mWriteDebugString("\pAdd COLORIZED Pix");
            WriteDebugLong( realResID);
            WriteDebugLong( resID);
            WriteDebugLong( color);
        }

        gPixTable[i].resource.load_resource(kPixResType, realResID);

        if (gPixTable[i].resource.get() == nil) {
//          Debugger();
            ShowErrorAny( eContinueOnlyErr, kErrorStrID, nil, nil, nil, nil, kLoadSpriteError, -1, -1, -1, __FILE__, resID);
            return TypedHandle<natePixType>();
        }

        /*
        MoveHHi( gPixTable[i].resource);
        HLock( gPixTable[i].resource);
        */

//      WriteDebugLine((char *)"\pADDPIX");
//      WriteDebugLong( resID);

        if ( color == 0)
        {
            RemapNatePixTableColor( gPixTable[i].resource);
        } else
        {
            ColorizeNatePixTableColor( gPixTable[i].resource, color);
        }

        gPixTable[i].resID = resID;
        return( gPixTable[i].resource);
    } return( GetPixTable( resID));
}

TypedHandle<natePixType> GetPixTable( short resID)
{
    short       i = 0;

//  mWriteDebugString("\pGETpix < HANDLE");
    while (( gPixTable[i].resID != resID) && ( i < kMaxPixTableEntry)) i++;
    if (i == kMaxPixTableEntry) {
        return TypedHandle<natePixType>();
    }
    return ( gPixTable[i].resource);
}

spriteType *AddSprite( Point where, TypedHandle<natePixType> table, short resID, short whichShape, long scale, long size,
                    short layer, unsigned char color, long *whichSprite)

{
    int         i = 0;
    spriteType  *aSprite;

    aSprite = gSpriteTable.get();
    while ((aSprite->table.get() != nil) && ( i < kMaxSpriteNum)) {
        i++;
        aSprite++;
    }
    if ( i == kMaxSpriteNum)
    {
        *whichSprite = kNoSprite;
        DebugStr("\pNo Free Sprites!");
        return ( nil);
    } else *whichSprite = i;

    aSprite->where = where;
    aSprite->table = table;
    aSprite->resID = resID;
    aSprite->whichShape = whichShape;
    aSprite->scale = scale;
    aSprite->thisRect.left = aSprite->thisRect.top = 0;
    aSprite->thisRect.right = aSprite->thisRect.bottom = -1;
    aSprite->whichLayer = layer;
    aSprite->tinySize = size;
    aSprite->tinyColor = color;
    aSprite->killMe = false;
    aSprite->style = spriteNormal;
    aSprite->styleColor = 0x00;
    aSprite->styleData = 0;

    return ( aSprite);
}
void RemoveSprite( spriteType *aSprite)

{
    aSprite->killMe = false;
    aSprite->table = TypedHandle<natePixType>();
    aSprite->resID = -1;
}

// DrawSpriteInPixMap:
//  WARNING: DOES NOT CLIP.  WILL CRASH IF DESTINATION RECT IS NOT CONTAINED IN
//  DESTINATION PIX MAP.

void RunLengthSpritePixInPixMap( spritePix *sprite, Point where, PixMap* pixMap)

{
    int     width, height, runlen, pixlen, *sword;
    unsigned char *source, *dest;
    long    rowBytes, sRowPlus, dRowPlus, *slong, *dlong;

    rowBytes = 0x0000ffff & (pixMap->rowBytes ^ ROW_BYTES_MASK);
    dest = pixMap->baseAddr + where.v * rowBytes + where.h;
    sword = reinterpret_cast<int*>(*(sprite->data));
    sRowPlus = 0;
    dRowPlus = rowBytes - sprite->width;
    width = sprite->width;
    height = sprite->height - 1;

    startrow:
        sword++;
    addnil:
        dest += *sword;
        sword++;
        if ( *sword == 0)
            goto endrow;
        runlen = *sword;
        sword++;
        if ( runlen < 4)
            goto pixbyte;
        pixlen = runlen >> 2;
        runlen %= 4;
        pixlen--;
        dlong = reinterpret_cast<long*>(dest);
        slong = reinterpret_cast<long*>(sword);
    longloop:
        *dlong++ = *slong++;
        if ( --pixlen >= 0)
            goto longloop;
        dest = reinterpret_cast<unsigned char*>(dlong);
        sword = reinterpret_cast<int*>(slong);
        if ( runlen == 0)
            goto addnil;

    pixbyte:
        runlen--;
        source = reinterpret_cast<unsigned char*>(sword);
    byteloop:
        *dest++ = *source++;
        if ( --runlen >= 0)
            goto byteloop;
        sword = reinterpret_cast<int*>(source);
        goto addnil;

    endrow:
        dest += dRowPlus;
        sword++;
        if ( --height >= 0)
            goto startrow;
}


void OptScaleSpritePixInPixMap( spritePix *sprite, Point where, long scale, Rect *dRect,
        Rect *clipRect, PixMap* pixMap)
{
    long        mapWidth, mapHeight, x, y, i, h, v, d, last;
    long        shapeRowPlus, destRowPlus, rowbytes, *hmap, *vmap, *hmapoffset, *lhend, scaleCalc;
    unsigned char *destByte, *shapeByte, *hend, *vend, *chunkByte;
    Rect    mapRect, sourceRect;
    bool     clipped = false;

    scaleCalc = (sprite->width * scale);
    scaleCalc >>= SHIFT_SCALE;
    mapWidth = scaleCalc;
    scaleCalc = (sprite->height * scale);
    scaleCalc >>= SHIFT_SCALE;
    mapHeight = scaleCalc;

    if ((( where.h + mapWidth) > clipRect->left) && (( where.h - mapWidth) <
            clipRect->right) && (( where.v + mapHeight) > clipRect->top) &&
            (( where.v - mapHeight < clipRect->bottom)) && ( mapHeight > 0) && ( mapWidth > 0))
    {
        scaleCalc = sprite->center.h * scale;
        scaleCalc >>= SHIFT_SCALE;
        dRect->left = where.h - scaleCalc;
        scaleCalc = sprite->center.v * scale;
        scaleCalc >>= SHIFT_SCALE;
        dRect->top = where.v - scaleCalc;

        mapRect.left = mapRect.top = 0;
        mapRect.right = mapWidth;
        mapRect.bottom = mapHeight;
        dRect->right = dRect->left + mapWidth;
        dRect->bottom = dRect->top + mapHeight;

        sourceRect.left = sourceRect.top = 0;
        sourceRect.right = sprite->width;
        sourceRect.bottom = sprite->height;

        if ( dRect->left < clipRect->left)
        {
            mapRect.left += clipRect->left - dRect->left;
            dRect->left = clipRect->left;
            clipped = true;
        }
        if ( dRect->right > clipRect->right)
        {
            mapRect.right -= dRect->right - clipRect->right;// + 1;
            dRect->right = clipRect->right;// - 1;
            clipped = true;
        }
        if ( dRect->top < clipRect->top)
        {
            mapRect.top += clipRect->top - dRect->top;
            dRect->top = clipRect->top;
            clipped = true;
        }
        if ( dRect->bottom > clipRect->bottom)
        {
            mapRect.bottom -= dRect->bottom - clipRect->bottom;// + 1;
            dRect->bottom = clipRect->bottom;// - 1;
            clipped = true;
        }

        if (( (dRect->left + 1) < clipRect->right) && ( dRect->right > clipRect->left) &&
                ( dRect->top < clipRect->bottom) && ( dRect->bottom > clipRect->top))
        {
            if ( scale <= SCALE_SCALE)
            {

                h = sprite->width - 1;
                v = ( mapWidth - 1) << 1;
                d = v - h;
                h = v - ( h << 1);
                x = y = i = last = 0;
                hmap = gScaleHMap;
                if ( v == 0) v = 1;

                while (( x < sprite->width) || ( y < mapWidth))
                {
                    x++;
                    i++;
                    if ( d > 0)
                    {
                        *hmap = ( x - (i >> 1L)) - last;
                        last += *hmap;
                        i = 0;
                        y++;
                        hmap++;
                        d += h;
                    } else d += v;
                }
                *hmap = sprite->width - last;

                h = sprite->height - 1;
                v = ( mapHeight - 1) << 1;
                d = v - h;
                h = v - ( h << 1);
                x = y = i = last = 0;
                vmap = gScaleVMap;
                if ( v == 0) v = 1;

                while (( x < sprite->height) || ( y < mapHeight))
                {
                    x++;
                    i++;
                    if ( d > 0)
                    {
                        *vmap = ( x - ( i >> 1L)) - last;
                        last += *vmap;
                        i = 0;
                        y++;
                        vmap++;
                        d += h;
                    } else d += v;
                }

                *vmap = sprite->height - last;

                if ( clipped)
                {
                    sourceRect.left = 0;
                    hmap = gScaleHMap;
                    d = mapRect.left;
                    while ( d > 0) { sourceRect.left += *hmap++; d--;}

                    sourceRect.right = sourceRect.left;
                    d = mapRect.right - mapRect.left + 1;
                    while ( d > 0) { sourceRect.right += *hmap++; d--;}

                    sourceRect.top = 0;
                    vmap = gScaleVMap;
                    d = mapRect.top;
                    while ( d > 0) { sourceRect.top += *vmap++; d--;}

                    sourceRect.bottom = sourceRect.top;
                    d = mapRect.bottom - mapRect.top + 1;
                    while ( d > 0) { sourceRect.bottom += *vmap++; d--;}


                } // otherwise sourceRect is set

                scaleCalc = (dRect->right - dRect->left);

                rowbytes = pixMap->rowBytes;
                rowbytes &= 0x0000ffff;
                rowbytes |= 0x00008000;
                rowbytes ^= 0x00008000;

                destRowPlus = rowbytes - scaleCalc;
                shapeRowPlus = sprite->width - (sourceRect.right - sourceRect.left);                                              //KLUDGE ALERT
                destByte = pixMap->baseAddr + dRect->top * rowbytes + dRect->left;
                shapeByte = sprite->data + sourceRect.top * sprite->width + sourceRect.left;

                vmap = gScaleVMap + mapRect.top;
                hmapoffset = gScaleHMap + mapRect.left;
                vend = destByte + rowbytes * (dRect->bottom - dRect->top);
                y = dRect->bottom - dRect->top;

                shapeRowPlus += *(hmapoffset + scaleCalc);
                mapWidth = sprite->width;
                chunkByte = pixMap->baseAddr + (pixMap->bounds.bottom) * rowbytes;

                do
                {
                    hmap = hmapoffset;
                    hend = destByte + scaleCalc;

                    do
                    {
                        if ( *shapeByte)
                            *destByte = *shapeByte;

                        shapeByte += *hmap++;
                        destByte++;

//                      // debugging
//                      if ( x > clipRect->right)
//                      {
//                          WriteDebugLine( (char *)"\pX:");
//                          WriteDebugLong( hend - destByte);
//                      }
//                      x++;

                    } while ( destByte < hend);
                    destByte += destRowPlus;

                    // debugging
//                  y++;
//                  if ( y > clipRect->bottom)
//                  {
//                      WriteDebugLine( (char *)"\pY:");
//                      WriteDebugLong( y);
//                  }

                    shapeByte += (*vmap++ - 1) * mapWidth + shapeRowPlus;
                } while ( destByte < vend);
//              } while ( --y > 0);
            } else if ( scale <= MAX_SCALE)
            {
                h = mapWidth - 1;
                v = ( sprite->width - 1) << 1;
                d = v - h;
                h = v - ( h << 1);
                x = y = i = last = 0;
                hmap = gScaleHMap;
                vmap = hmapoffset = nil;
                while (( x < mapWidth - 1) || ( y < sprite->width - 1))
                {
                    x++;
                    i++;
                    if ( d > 0)
                    {
                        *hmap = i;

                        i = 0;
                        y++;
                        hmap++;
                        d += h;
                    } else d += v;
                }
                *hmap = i + 1;

                h = mapHeight - 1;
                v = ( sprite->height - 1) << 1;
                d = v - h;
                h = v - ( h << 1);
                x = y = i = last = 0;
                vmap = gScaleVMap;
                hmapoffset = hmap = nil;
                while (( x < mapHeight - 1) || ( y < sprite->height - 1))
                {
                    x++;
                    i++;
                    if ( d > 0)
                    {
                        *vmap = i;

                        i = 0;
                        y++;
                        vmap++;
                        d += h;
                    } else d += v;
                }
                *vmap = i + 1;

                if ( clipped)
                {
                    sourceRect.left = h = 0;
                    hmap = gScaleHMap;
                    while ( ( h + *hmap) < mapRect.left) { h += *hmap++; sourceRect.left++;}
                    x = *hmap;
                    *hmap -= mapRect.left - h;
                    h += x - *hmap;
                    sourceRect.right = sourceRect.left;
                    while ( ( h + *hmap) < mapRect.right) { h += *hmap++; sourceRect.right++;}
                    *hmap = mapRect.right - h;
                    sourceRect.right++;

                    sourceRect.top = h = 0;
                    vmap = gScaleVMap;
                    while ( ( h + *vmap) < mapRect.top) { h += *vmap++; sourceRect.top++;}
                    x = *vmap;
                    *vmap -= mapRect.top - h;
                    h += x - *vmap;
                    sourceRect.bottom = sourceRect.top;
                    while ( ( h + *vmap) < mapRect.bottom) { h += *vmap++; sourceRect.bottom++;}
                    *vmap = mapRect.bottom - h;
                    if ( sourceRect.bottom < sprite->height) sourceRect.bottom++;
                } // otherwise sourceRect is already set

                scaleCalc = (dRect->right - dRect->left);
                rowbytes = 0x0000ffff & (pixMap->rowBytes ^ ROW_BYTES_MASK);
                destRowPlus = rowbytes - scaleCalc;
                destByte = pixMap->baseAddr + dRect->top * rowbytes + dRect->left;
                shapeByte = sprite->data + sourceRect.top * sprite->width + sourceRect.left;

                vmap = gScaleVMap + sourceRect.top;
                hmapoffset = gScaleHMap + sourceRect.left;
                shapeRowPlus = sprite->width;
                mapWidth = sprite->width;
                vend = sprite->data + sourceRect.bottom * sprite->width + sourceRect.left;
                lhend = gScaleHMap + sourceRect.right;

                while ( shapeByte < vend)
                {
                    v = *vmap;
                    while ( v > 0)
                    {
                        hmap = hmapoffset;
                        chunkByte = shapeByte;
                        while ( hmap < lhend)
                        {
                            if (( *chunkByte) && ( *hmap))
                            {
                                for ( h = *hmap; h > 0; h--)
                                    *destByte++ = *chunkByte;
                            } else destByte += *hmap;
                            hmap++;
                            chunkByte++;
                        }
                        destByte += destRowPlus;
                        v--;
                    }
                    vmap++;
                    shapeByte += shapeRowPlus;
                }

            } else dRect->left = dRect->right = dRect->top = dRect->bottom = 0;
        } else dRect->left = dRect->right = dRect->top = dRect->bottom = 0;
    } else dRect->left = dRect->right = dRect->top = dRect->bottom = 0;
}

void StaticScaleSpritePixInPixMap( spritePix *sprite, Point where, long scale, Rect *dRect,
        Rect *clipRect, PixMap* pixMap, short staticValue)

{
    long        mapWidth, mapHeight, x, y, i, h, v, d, last;
    long        shapeRowPlus, destRowPlus, rowbytes, *hmap, *vmap, *hmapoffset, *lhend, scaleCalc;
    unsigned char *destByte, *shapeByte, *hend, *vend, *chunkByte;
    unsigned char   *staticByte;
    Rect    mapRect, sourceRect;
    bool     clipped = false;

    scaleCalc = (sprite->width * scale);
    scaleCalc >>= SHIFT_SCALE;
    mapWidth = scaleCalc;
    scaleCalc = (sprite->height * scale);
    scaleCalc >>= SHIFT_SCALE;
    mapHeight = scaleCalc;

    if ((( where.h + mapWidth) > clipRect->left) && (( where.h - mapWidth) <
            clipRect->right) && (( where.v + mapHeight) > clipRect->top) &&
            (( where.v - mapHeight < clipRect->bottom)) && ( mapHeight > 0) && ( mapWidth > 0))
    {
        scaleCalc = sprite->center.h * scale;
        scaleCalc >>= SHIFT_SCALE;
        dRect->left = where.h - scaleCalc;
        scaleCalc = sprite->center.v * scale;
        scaleCalc >>= SHIFT_SCALE;
        dRect->top = where.v - scaleCalc;

        mapRect.left = mapRect.top = 0;
        mapRect.right = mapWidth;
        mapRect.bottom = mapHeight;
        dRect->right = dRect->left + mapWidth;
        dRect->bottom = dRect->top + mapHeight;

        sourceRect.left = sourceRect.top = 0;
        sourceRect.right = sprite->width;
        sourceRect.bottom = sprite->height;

        if ( dRect->left < clipRect->left)
        {
            mapRect.left += clipRect->left - dRect->left;
            dRect->left = clipRect->left;
            clipped = true;
        }
        if ( dRect->right > clipRect->right)
        {
            mapRect.right -= dRect->right - clipRect->right;// + 1;
            dRect->right = clipRect->right;// - 1;
            clipped = true;
        }
        if ( dRect->top < clipRect->top)
        {
            mapRect.top += clipRect->top - dRect->top;
            dRect->top = clipRect->top;
            clipped = true;
        }
        if ( dRect->bottom > clipRect->bottom)
        {
            mapRect.bottom -= dRect->bottom - clipRect->bottom;// + 1;
            dRect->bottom = clipRect->bottom;// - 1;
            clipped = true;
        }

        if (( (dRect->left + 1) < clipRect->right) && ( dRect->right > clipRect->left) &&
                ( dRect->top < clipRect->bottom) && ( dRect->bottom > clipRect->top))
        {
            staticByte = gStaticTable.get() + staticValue;
            if ( scale <= SCALE_SCALE)
            {

                h = sprite->width - 1;
                v = ( mapWidth - 1) << 1;
                d = v - h;
                h = v - ( h << 1);
                x = y = i = last = 0;
                hmap = gScaleHMap;
                if ( v == 0) v = 1;

                while (( x < sprite->width) || ( y < mapWidth))
                {
                    x++;
                    i++;
                    if ( d > 0)
                    {
                        *hmap = ( x - (i >> 1L)) - last;
                        last += *hmap;
                        i = 0;
                        y++;
                        hmap++;
                        d += h;
                    } else d += v;
                }
                *hmap = sprite->width - last;

                h = sprite->height - 1;
                v = ( mapHeight - 1) << 1;
                d = v - h;
                h = v - ( h << 1);
                x = y = i = last = 0;
                vmap = gScaleVMap;
                if ( v == 0) v = 1;

                while (( x < sprite->height) || ( y < mapHeight))
                {
                    x++;
                    i++;
                    if ( d > 0)
                    {
                        *vmap = ( x - ( i >> 1L)) - last;
                        last += *vmap;
                        i = 0;
                        y++;
                        vmap++;
                        d += h;
                    } else d += v;
                }

                *vmap = sprite->height - last;

                if ( clipped)
                {
                    sourceRect.left = 0;
                    hmap = gScaleHMap;
                    d = mapRect.left;
                    while ( d > 0) { sourceRect.left += *hmap++; d--;}

                    sourceRect.right = sourceRect.left;
                    d = mapRect.right - mapRect.left + 1;
                    while ( d > 0) { sourceRect.right += *hmap++; d--;}

                    sourceRect.top = 0;
                    vmap = gScaleVMap;
                    d = mapRect.top;
                    while ( d > 0) { sourceRect.top += *vmap++; d--;}

                    sourceRect.bottom = sourceRect.top;
                    d = mapRect.bottom - mapRect.top + 1;
                    while ( d > 0) { sourceRect.bottom += *vmap++; d--;}


                } // otherwise sourceRect is set

                scaleCalc = (dRect->right - dRect->left);

                rowbytes = pixMap->rowBytes;
                rowbytes &= 0x0000ffff;
                rowbytes |= 0x00008000;
                rowbytes ^= 0x00008000;

                destRowPlus = rowbytes - scaleCalc;
                shapeRowPlus = sprite->width - (sourceRect.right - sourceRect.left);                                              //KLUDGE ALERT
                destByte = pixMap->baseAddr + dRect->top * rowbytes + dRect->left;
                shapeByte = sprite->data + sourceRect.top * sprite->width + sourceRect.left;

                vmap = gScaleVMap + mapRect.top;
                hmapoffset = gScaleHMap + mapRect.left;
                vend = destByte + rowbytes * (dRect->bottom - dRect->top);
                y = dRect->bottom - dRect->top;

                shapeRowPlus += *(hmapoffset + scaleCalc);
                mapWidth = sprite->width;
                chunkByte = pixMap->baseAddr + (pixMap->bounds.bottom) * rowbytes;

                do
                {
                    hmap = hmapoffset;
                    hend = destByte + scaleCalc;
                    if ( (staticValue + scaleCalc) > ( kStaticTableSize))
                    {
                        staticValue += scaleCalc - kStaticTableSize;
                        staticByte = gStaticTable.get() + staticValue;
                    } else staticValue += scaleCalc;

                    do
                    {
                        if ( *shapeByte)
                            *destByte = *staticByte;

                        shapeByte += *hmap++;
                        destByte++;
                        staticByte++;


                    } while ( destByte < hend);
                    destByte += destRowPlus;


                    shapeByte += (*vmap++ - 1) * mapWidth + shapeRowPlus;
                } while ( destByte < vend);
            } else if ( scale <= MAX_SCALE)
            {
                h = mapWidth - 1;
                v = ( sprite->width - 1) << 1;
                d = v - h;
                h = v - ( h << 1);
                x = y = i = last = 0;
                hmap = gScaleHMap;
                vmap = hmapoffset = nil;
                while (( x < mapWidth - 1) || ( y < sprite->width - 1))
                {
                    x++;
                    i++;
                    if ( d > 0)
                    {
                        *hmap = i;

                        i = 0;
                        y++;
                        hmap++;
                        d += h;
                    } else d += v;
                }
                *hmap = i + 1;

                h = mapHeight - 1;
                v = ( sprite->height - 1) << 1;
                d = v - h;
                h = v - ( h << 1);
                x = y = i = last = 0;
                vmap = gScaleVMap;
                hmapoffset = hmap = nil;
                while (( x < mapHeight - 1) || ( y < sprite->height - 1))
                {
                    x++;
                    i++;
                    if ( d > 0)
                    {
                        *vmap = i;

                        i = 0;
                        y++;
                        vmap++;
                        d += h;
                    } else d += v;
                }
                *vmap = i + 1;

                if ( clipped)
                {
                    sourceRect.left = h = 0;
                    hmap = gScaleHMap;
                    while ( ( h + *hmap) < mapRect.left) { h += *hmap++; sourceRect.left++;}
                    x = *hmap;
                    *hmap -= mapRect.left - h;
                    h += x - *hmap;
                    sourceRect.right = sourceRect.left;
                    while ( ( h + *hmap) < mapRect.right) { h += *hmap++; sourceRect.right++;}
                    *hmap = mapRect.right - h;
                    sourceRect.right++;

                    sourceRect.top = h = 0;
                    vmap = gScaleVMap;
                    while ( ( h + *vmap) < mapRect.top) { h += *vmap++; sourceRect.top++;}
                    x = *vmap;
                    *vmap -= mapRect.top - h;
                    h += x - *vmap;
                    sourceRect.bottom = sourceRect.top;
                    while ( ( h + *vmap) < mapRect.bottom) { h += *vmap++; sourceRect.bottom++;}
                    *vmap = mapRect.bottom - h;
                    if ( sourceRect.bottom < sprite->height) sourceRect.bottom++;
                } // otherwise sourceRect is already set

                scaleCalc = (dRect->right - dRect->left);
                rowbytes = 0x0000ffff & (pixMap->rowBytes ^ ROW_BYTES_MASK);
                destRowPlus = rowbytes - scaleCalc;
                destByte = pixMap->baseAddr + dRect->top * rowbytes + dRect->left;
                shapeByte = sprite->data + sourceRect.top * sprite->width + sourceRect.left;

                vmap = gScaleVMap + sourceRect.top;
                hmapoffset = gScaleHMap + sourceRect.left;
                shapeRowPlus = sprite->width;
                mapWidth = sprite->width;
                vend = sprite->data + sourceRect.bottom * sprite->width + sourceRect.left;
                lhend = gScaleHMap + sourceRect.right;

                while ( shapeByte < vend)
                {
                    v = *vmap;
                    while ( v > 0)
                    {
                        hmap = hmapoffset;
                        chunkByte = shapeByte;
                        if ( (staticValue + mapWidth) > ( kStaticTableSize))
                        {
                            staticValue += mapWidth - kStaticTableSize;
                            staticByte = gStaticTable.get() + staticValue;
                        } else staticValue += mapWidth;
                        while ( hmap < lhend)
                        {
                            if (( *chunkByte) && ( *hmap))
                            {
                                for ( h = *hmap; h > 0; h--)
                                    *destByte++ = *staticByte;
                            } else destByte += *hmap;
                            hmap++;
                            chunkByte++;
                            staticByte++;
                        }
                        destByte += destRowPlus;
                        v--;
                    }
                    vmap++;
                    shapeByte += shapeRowPlus;
                }

            } else dRect->left = dRect->right = dRect->top = dRect->bottom = 0;
        } else dRect->left = dRect->right = dRect->top = dRect->bottom = 0;
    } else dRect->left = dRect->right = dRect->top = dRect->bottom = 0;
}

void ColorScaleSpritePixInPixMap( spritePix *sprite, Point where, long scale, Rect *dRect,
        Rect *clipRect, PixMap* pixMap, short staticValue, unsigned char color,
        unsigned char colorAmount)

{
    long        mapWidth, mapHeight, x, y, i, h, v, d, last;
    long        shapeRowPlus, destRowPlus, rowbytes, *hmap, *vmap, *hmapoffset, *lhend, scaleCalc;
    unsigned char *destByte, *shapeByte, *hend, *vend, *chunkByte;
    unsigned char   *staticByte;
    Rect    mapRect, sourceRect;
    bool     clipped = false;

    scaleCalc = (sprite->width * scale);
    scaleCalc >>= SHIFT_SCALE;
    mapWidth = scaleCalc;
    scaleCalc = (sprite->height * scale);
    scaleCalc >>= SHIFT_SCALE;
    mapHeight = scaleCalc;

    if ((( where.h + mapWidth) > clipRect->left) && (( where.h - mapWidth) <
            clipRect->right) && (( where.v + mapHeight) > clipRect->top) &&
            (( where.v - mapHeight < clipRect->bottom)) && ( mapHeight > 0) && ( mapWidth > 0))
    {
        scaleCalc = sprite->center.h * scale;
        scaleCalc >>= SHIFT_SCALE;
        dRect->left = where.h - scaleCalc;
        scaleCalc = sprite->center.v * scale;
        scaleCalc >>= SHIFT_SCALE;
        dRect->top = where.v - scaleCalc;

        mapRect.left = mapRect.top = 0;
        mapRect.right = mapWidth;
        mapRect.bottom = mapHeight;
        dRect->right = dRect->left + mapWidth;
        dRect->bottom = dRect->top + mapHeight;

        sourceRect.left = sourceRect.top = 0;
        sourceRect.right = sprite->width;
        sourceRect.bottom = sprite->height;

        if ( dRect->left < clipRect->left)
        {
            mapRect.left += clipRect->left - dRect->left;
            dRect->left = clipRect->left;
            clipped = true;
        }
        if ( dRect->right > clipRect->right)
        {
            mapRect.right -= dRect->right - clipRect->right;// + 1;
            dRect->right = clipRect->right;// - 1;
            clipped = true;
        }
        if ( dRect->top < clipRect->top)
        {
            mapRect.top += clipRect->top - dRect->top;
            dRect->top = clipRect->top;
            clipped = true;
        }
        if ( dRect->bottom > clipRect->bottom)
        {
            mapRect.bottom -= dRect->bottom - clipRect->bottom;// + 1;
            dRect->bottom = clipRect->bottom;// - 1;
            clipped = true;
        }

        if (( (dRect->left + 1) < clipRect->right) && ( dRect->right > clipRect->left) &&
                ( dRect->top < clipRect->bottom) && ( dRect->bottom > clipRect->top))
        {
            staticByte = gStaticTable.get() + staticValue;
            if ( scale <= SCALE_SCALE)
            {

                h = sprite->width - 1;
                v = ( mapWidth - 1) << 1;
                d = v - h;
                h = v - ( h << 1);
                x = y = i = last = 0;
                hmap = gScaleHMap;
                if ( v == 0) v = 1;

                while (( x < sprite->width) || ( y < mapWidth))
                {
                    x++;
                    i++;
                    if ( d > 0)
                    {
                        *hmap = ( x - (i >> 1L)) - last;
                        last += *hmap;
                        i = 0;
                        y++;
                        hmap++;
                        d += h;
                    } else d += v;
                }
                *hmap = sprite->width - last;

                h = sprite->height - 1;
                v = ( mapHeight - 1) << 1;
                d = v - h;
                h = v - ( h << 1);
                x = y = i = last = 0;
                vmap = gScaleVMap;
                if ( v == 0) v = 1;

                while (( x < sprite->height) || ( y < mapHeight))
                {
                    x++;
                    i++;
                    if ( d > 0)
                    {
                        *vmap = ( x - ( i >> 1L)) - last;
                        last += *vmap;
                        i = 0;
                        y++;
                        vmap++;
                        d += h;
                    } else d += v;
                }

                *vmap = sprite->height - last;

                if ( clipped)
                {
                    sourceRect.left = 0;
                    hmap = gScaleHMap;
                    d = mapRect.left;
                    while ( d > 0) { sourceRect.left += *hmap++; d--;}

                    sourceRect.right = sourceRect.left;
                    d = mapRect.right - mapRect.left + 1;
                    while ( d > 0) { sourceRect.right += *hmap++; d--;}

                    sourceRect.top = 0;
                    vmap = gScaleVMap;
                    d = mapRect.top;
                    while ( d > 0) { sourceRect.top += *vmap++; d--;}

                    sourceRect.bottom = sourceRect.top;
                    d = mapRect.bottom - mapRect.top + 1;
                    while ( d > 0) { sourceRect.bottom += *vmap++; d--;}


                } // otherwise sourceRect is set

                scaleCalc = (dRect->right - dRect->left);

                rowbytes = pixMap->rowBytes;
                rowbytes &= 0x0000ffff;
                rowbytes |= 0x00008000;
                rowbytes ^= 0x00008000;

                destRowPlus = rowbytes - scaleCalc;
                shapeRowPlus = sprite->width - (sourceRect.right - sourceRect.left);                                              //KLUDGE ALERT
                destByte = pixMap->baseAddr + dRect->top * rowbytes + dRect->left;
                shapeByte = sprite->data + sourceRect.top * sprite->width + sourceRect.left;

                vmap = gScaleVMap + mapRect.top;
                hmapoffset = gScaleHMap + mapRect.left;
                vend = destByte + rowbytes * (dRect->bottom - dRect->top);
                y = dRect->bottom - dRect->top;

                shapeRowPlus += *(hmapoffset + scaleCalc);
                mapWidth = sprite->width;
                chunkByte = pixMap->baseAddr + (pixMap->bounds.bottom) * rowbytes;

                if ( color != 0xff)
                {
                    do
                    {
                        hmap = hmapoffset;
                        hend = destByte + scaleCalc;
                        if ( (staticValue + scaleCalc) > ( kStaticTableSize))
                        {
                            staticValue += scaleCalc - kStaticTableSize;
                            staticByte = gStaticTable.get() + staticValue;
                        } else staticValue += scaleCalc;

                        do
                        {
                            if ( *shapeByte)
                            {
                                if ( *staticByte > colorAmount)
                                    *destByte = *shapeByte;
                                else *destByte = color;
                            }

                            shapeByte += *hmap++;
                            destByte++;
                            staticByte++;


                        } while ( destByte < hend);
                        destByte += destRowPlus;


                        shapeByte += (*vmap++ - 1) * mapWidth + shapeRowPlus;
                    } while ( destByte < vend);
                } else // black is a special case--we don't want to draw black color
                {
                    do
                    {
                        hmap = hmapoffset;
                        hend = destByte + scaleCalc;
                        if ( (staticValue + scaleCalc) > ( kStaticTableSize))
                        {
                            staticValue += scaleCalc - kStaticTableSize;
                            staticByte = gStaticTable.get() + staticValue;
                        } else staticValue += scaleCalc;

                        do
                        {
                            if ( *shapeByte)
                            {
                                if ( *staticByte > colorAmount)
                                    *destByte = *shapeByte;
                            }

                            shapeByte += *hmap++;
                            destByte++;
                            staticByte++;


                        } while ( destByte < hend);
                        destByte += destRowPlus;


                        shapeByte += (*vmap++ - 1) * mapWidth + shapeRowPlus;
                    } while ( destByte < vend);
                }
            } else if ( scale <= MAX_SCALE)
            {
                h = mapWidth - 1;
                v = ( sprite->width - 1) << 1;
                d = v - h;
                h = v - ( h << 1);
                x = y = i = last = 0;
                hmap = gScaleHMap;
                vmap = hmapoffset = nil;
                while (( x < mapWidth - 1) || ( y < sprite->width - 1))
                {
                    x++;
                    i++;
                    if ( d > 0)
                    {
                        *hmap = i;

                        i = 0;
                        y++;
                        hmap++;
                        d += h;
                    } else d += v;
                }
                *hmap = i + 1;

                h = mapHeight - 1;
                v = ( sprite->height - 1) << 1;
                d = v - h;
                h = v - ( h << 1);
                x = y = i = last = 0;
                vmap = gScaleVMap;
                hmapoffset = hmap = nil;
                while (( x < mapHeight - 1) || ( y < sprite->height - 1))
                {
                    x++;
                    i++;
                    if ( d > 0)
                    {
                        *vmap = i;

                        i = 0;
                        y++;
                        vmap++;
                        d += h;
                    } else d += v;
                }
                *vmap = i + 1;

                if ( clipped)
                {
                    sourceRect.left = h = 0;
                    hmap = gScaleHMap;
                    while ( ( h + *hmap) < mapRect.left) { h += *hmap++; sourceRect.left++;}
                    x = *hmap;
                    *hmap -= mapRect.left - h;
                    h += x - *hmap;
                    sourceRect.right = sourceRect.left;
                    while ( ( h + *hmap) < mapRect.right) { h += *hmap++; sourceRect.right++;}
                    *hmap = mapRect.right - h;
                    sourceRect.right++;

                    sourceRect.top = h = 0;
                    vmap = gScaleVMap;
                    while ( ( h + *vmap) < mapRect.top) { h += *vmap++; sourceRect.top++;}
                    x = *vmap;
                    *vmap -= mapRect.top - h;
                    h += x - *vmap;
                    sourceRect.bottom = sourceRect.top;
                    while ( ( h + *vmap) < mapRect.bottom) { h += *vmap++; sourceRect.bottom++;}
                    *vmap = mapRect.bottom - h;
                    if ( sourceRect.bottom < sprite->height) sourceRect.bottom++;
                } // otherwise sourceRect is already set

                scaleCalc = (dRect->right - dRect->left);
                rowbytes = 0x0000ffff & (pixMap->rowBytes ^ ROW_BYTES_MASK);
                destRowPlus = rowbytes - scaleCalc;
                destByte = pixMap->baseAddr + dRect->top * rowbytes + dRect->left;
                shapeByte = sprite->data + sourceRect.top * sprite->width + sourceRect.left;

                vmap = gScaleVMap + sourceRect.top;
                hmapoffset = gScaleHMap + sourceRect.left;
                shapeRowPlus = sprite->width;
                mapWidth = sprite->width;
                vend = sprite->data + sourceRect.bottom * sprite->width + sourceRect.left;
                lhend = gScaleHMap + sourceRect.right;

                while ( shapeByte < vend)
                {
                    v = *vmap;
                    while ( v > 0)
                    {
                        hmap = hmapoffset;
                        chunkByte = shapeByte;
                        if ( (staticValue + mapWidth) > ( kStaticTableSize))
                        {
                            staticValue += mapWidth - kStaticTableSize;
                            staticByte = gStaticTable.get() + staticValue;
                        } else staticValue += mapWidth;
                        while ( hmap < lhend)
                        {
                            if (( *chunkByte) && ( *hmap))
                            {
                                if ( *staticByte > colorAmount)
                                {
                                    for ( h = *hmap; h > 0; h--)
                                        *destByte++ = *chunkByte;
                                } else
                                {
                                    for ( h = *hmap; h > 0; h--)
                                        *destByte++ = color;
                                }
                            } else destByte += *hmap;
                            hmap++;
                            chunkByte++;
                            staticByte++;
                        }
                        destByte += destRowPlus;
                        v--;
                    }
                    vmap++;
                    shapeByte += shapeRowPlus;
                }

            } else dRect->left = dRect->right = dRect->top = dRect->bottom = 0;
        } else dRect->left = dRect->right = dRect->top = dRect->bottom = 0;
    } else dRect->left = dRect->right = dRect->top = dRect->bottom = 0;
}

// a hack; not fast

void OutlineScaleSpritePixInPixMap( spritePix *sprite, Point where, long scale, Rect *dRect,
        Rect *clipRect, PixMap* pixMap, unsigned char colorOut,
        unsigned char colorIn)

{
    long        mapWidth, mapHeight, x, y, i, h, v, d, last, sourceX, sourceY;
    long        shapeRowPlus, destRowPlus, rowbytes, *hmap, *vmap, *hmapoffset, *lhend, scaleCalc;
    unsigned char *destByte, *shapeByte, *hend, *vend, *chunkByte;
    Rect    mapRect, sourceRect;
    bool     clipped = false;

    scaleCalc = (sprite->width * scale);
    scaleCalc >>= SHIFT_SCALE;
    mapWidth = scaleCalc;
    scaleCalc = (sprite->height * scale);
    scaleCalc >>= SHIFT_SCALE;
    mapHeight = scaleCalc;

    if ((( where.h + mapWidth) > clipRect->left) && (( where.h - mapWidth) <
            clipRect->right) && (( where.v + mapHeight) > clipRect->top) &&
            (( where.v - mapHeight < clipRect->bottom)) && ( mapHeight > 0) && ( mapWidth > 0))
    {
        scaleCalc = sprite->center.h * scale;
        scaleCalc >>= SHIFT_SCALE;
        dRect->left = where.h - scaleCalc;
        scaleCalc = sprite->center.v * scale;
        scaleCalc >>= SHIFT_SCALE;
        dRect->top = where.v - scaleCalc;

        mapRect.left = mapRect.top = 0;
        mapRect.right = mapWidth;
        mapRect.bottom = mapHeight;
        dRect->right = dRect->left + mapWidth;
        dRect->bottom = dRect->top + mapHeight;

        sourceRect.left = sourceRect.top = 0;
        sourceRect.right = sprite->width;
        sourceRect.bottom = sprite->height;

        if ( dRect->left < clipRect->left)
        {
            mapRect.left += clipRect->left - dRect->left;
            dRect->left = clipRect->left;
            clipped = true;
        }
        if ( dRect->right > clipRect->right)
        {
            mapRect.right -= dRect->right - clipRect->right;// + 1;
            dRect->right = clipRect->right;// - 1;
            clipped = true;
        }
        if ( dRect->top < clipRect->top)
        {
            mapRect.top += clipRect->top - dRect->top;
            dRect->top = clipRect->top;
            clipped = true;
        }
        if ( dRect->bottom > clipRect->bottom)
        {
            mapRect.bottom -= dRect->bottom - clipRect->bottom;// + 1;
            dRect->bottom = clipRect->bottom;// - 1;
            clipped = true;
        }

        if (( (dRect->left + 1) < clipRect->right) && ( dRect->right > clipRect->left) &&
                ( dRect->top < clipRect->bottom) && ( dRect->bottom > clipRect->top))
        {
            if ( scale <= SCALE_SCALE)
            {

                h = sprite->width - 1;
                v = ( mapWidth - 1) << 1;
                d = v - h;
                h = v - ( h << 1);
                x = y = i = last = 0;
                hmap = gScaleHMap;
                if ( v == 0) v = 1;

                while (( x < sprite->width) || ( y < mapWidth))
                {
                    x++;
                    i++;
                    if ( d > 0)
                    {
                        *hmap = ( x - (i >> 1L)) - last;
                        last += *hmap;
                        i = 0;
                        y++;
                        hmap++;
                        d += h;
                    } else d += v;
                }
                *hmap = sprite->width - last;

                h = sprite->height - 1;
                v = ( mapHeight - 1) << 1;
                d = v - h;
                h = v - ( h << 1);
                x = y = i = last = 0;
                vmap = gScaleVMap;
                if ( v == 0) v = 1;

                while (( x < sprite->height) || ( y < mapHeight))
                {
                    x++;
                    i++;
                    if ( d > 0)
                    {
                        *vmap = ( x - ( i >> 1L)) - last;
                        last += *vmap;
                        i = 0;
                        y++;
                        vmap++;
                        d += h;
                    } else d += v;
                }

                *vmap = sprite->height - last;

                if ( clipped)
                {
                    sourceRect.left = 0;
                    hmap = gScaleHMap;
                    d = mapRect.left;
                    while ( d > 0) { sourceRect.left += *hmap++; d--;}

                    sourceRect.right = sourceRect.left;
                    d = mapRect.right - mapRect.left + 1;
                    while ( d > 0) { sourceRect.right += *hmap++; d--;}

                    sourceRect.top = 0;
                    vmap = gScaleVMap;
                    d = mapRect.top;
                    while ( d > 0) { sourceRect.top += *vmap++; d--;}

                    sourceRect.bottom = sourceRect.top;
                    d = mapRect.bottom - mapRect.top + 1;
                    while ( d > 0) { sourceRect.bottom += *vmap++; d--;}


                } // otherwise sourceRect is set

                scaleCalc = (dRect->right - dRect->left);

                rowbytes = pixMap->rowBytes;
                rowbytes &= 0x0000ffff;
                rowbytes |= 0x00008000;
                rowbytes ^= 0x00008000;

                destRowPlus = rowbytes - scaleCalc;
                shapeRowPlus = sprite->width - (sourceRect.right - sourceRect.left);                                              //KLUDGE ALERT
                destByte = pixMap->baseAddr + dRect->top * rowbytes + dRect->left;
                shapeByte = sprite->data + sourceRect.top * sprite->width + sourceRect.left;

                vmap = gScaleVMap + mapRect.top;
                hmapoffset = gScaleHMap + mapRect.left;
                vend = destByte + rowbytes * (dRect->bottom - dRect->top);
                y = dRect->bottom - dRect->top;

                shapeRowPlus += *(hmapoffset + scaleCalc);
                mapWidth = sprite->width;
                chunkByte = pixMap->baseAddr + (pixMap->bounds.bottom) * rowbytes;

                sourceY = sourceRect.top;
                do
                {
                    sourceX = sourceRect.left;
                    hmap = hmapoffset;
                    hend = destByte + scaleCalc;

                    do
                    {
                        if ( *shapeByte)
                        {
                            if ( PixelInSprite_IsOutside( sprite, sourceX, sourceY,
                                hmap, vmap))
                                *destByte = colorOut;// *shapeByte;
                            else
                                *destByte = colorIn;
                        }

//                      // debugging
                        sourceX += *hmap;
                        shapeByte += *hmap++;
                        destByte++;

//                      // debugging
//                      if ( x > clipRect->right)
//                      {
//                          WriteDebugLine( (char *)"\pX:");
//                          WriteDebugLong( hend - destByte);
//                      }
//                      x++;

                    } while ( destByte < hend);
                    destByte += destRowPlus;

                    // debugging
//                  y++;
//                  if ( y > clipRect->bottom)
//                  {
//                      WriteDebugLine( (char *)"\pY:");
//                      WriteDebugLong( y);
//                  }
                    sourceY += (*vmap);
                    shapeByte += (*vmap++ - 1) * mapWidth + shapeRowPlus;
                } while ( destByte < vend);
//              } while ( --y > 0);
            } else if ( scale <= MAX_SCALE)
            {
                h = mapWidth - 1;
                v = ( sprite->width - 1) << 1;
                d = v - h;
                h = v - ( h << 1);
                x = y = i = last = 0;
                hmap = gScaleHMap;
                vmap = hmapoffset = nil;
                while (( x < mapWidth - 1) || ( y < sprite->width - 1))
                {
                    x++;
                    i++;
                    if ( d > 0)
                    {
                        *hmap = i;

                        i = 0;
                        y++;
                        hmap++;
                        d += h;
                    } else d += v;
                }
                *hmap = i + 1;

                h = mapHeight - 1;
                v = ( sprite->height - 1) << 1;
                d = v - h;
                h = v - ( h << 1);
                x = y = i = last = 0;
                vmap = gScaleVMap;
                hmapoffset = hmap = nil;
                while (( x < mapHeight - 1) || ( y < sprite->height - 1))
                {
                    x++;
                    i++;
                    if ( d > 0)
                    {
                        *vmap = i;

                        i = 0;
                        y++;
                        vmap++;
                        d += h;
                    } else d += v;
                }
                *vmap = i + 1;

                if ( clipped)
                {
                    sourceRect.left = h = 0;
                    hmap = gScaleHMap;
                    while ( ( h + *hmap) < mapRect.left) { h += *hmap++; sourceRect.left++;}
                    x = *hmap;
                    *hmap -= mapRect.left - h;
                    h += x - *hmap;
                    sourceRect.right = sourceRect.left;
                    while ( ( h + *hmap) < mapRect.right) { h += *hmap++; sourceRect.right++;}
                    *hmap = mapRect.right - h;
                    sourceRect.right++;

                    sourceRect.top = h = 0;
                    vmap = gScaleVMap;
                    while ( ( h + *vmap) < mapRect.top) { h += *vmap++; sourceRect.top++;}
                    x = *vmap;
                    *vmap -= mapRect.top - h;
                    h += x - *vmap;
                    sourceRect.bottom = sourceRect.top;
                    while ( ( h + *vmap) < mapRect.bottom) { h += *vmap++; sourceRect.bottom++;}
                    *vmap = mapRect.bottom - h;
                    if ( sourceRect.bottom < sprite->height) sourceRect.bottom++;
                } // otherwise sourceRect is already set

                scaleCalc = (dRect->right - dRect->left);
                rowbytes = 0x0000ffff & (pixMap->rowBytes ^ ROW_BYTES_MASK);
                destRowPlus = rowbytes - scaleCalc;
                destByte = pixMap->baseAddr + dRect->top * rowbytes + dRect->left;
                shapeByte = sprite->data + sourceRect.top * sprite->width + sourceRect.left;

                vmap = gScaleVMap + sourceRect.top;
                hmapoffset = gScaleHMap + sourceRect.left;
                shapeRowPlus = sprite->width;
                mapWidth = sprite->width;
                vend = sprite->data + sourceRect.bottom * sprite->width + sourceRect.left;
                lhend = gScaleHMap + sourceRect.right;

                while ( shapeByte < vend)
                {
                    v = *vmap;
                    while ( v > 0)
                    {
                        hmap = hmapoffset;
                        chunkByte = shapeByte;
                        while ( hmap < lhend)
                        {
                            if (( *chunkByte) && ( *hmap))
                            {
                                for ( h = *hmap; h > 0; h--)
                                    *destByte++ = *chunkByte;
                            } else destByte += *hmap;
                            hmap++;
                            chunkByte++;
                        }
                        destByte += destRowPlus;
                        v--;
                    }
                    vmap++;
                    shapeByte += shapeRowPlus;
                }

            } else dRect->left = dRect->right = dRect->top = dRect->bottom = 0;
        } else dRect->left = dRect->right = dRect->top = dRect->bottom = 0;
    } else dRect->left = dRect->right = dRect->top = dRect->bottom = 0;
}

bool PixelInSprite_IsOutside( spritePix *sprite, long x, long y,
    long *hmap, long *vmap)
{
    unsigned char* pixel;
    long    rowPlus = sprite->width, i, j, *hmapStart = hmap;

    if ( x == 0) return true;
    if ( x >= ( sprite->width - *hmap)) return true;
    if ( y == 0) return true;
    if ( y >= ( sprite->height - *vmap)) return true;

    vmap--;
    hmapStart--;
    rowPlus -= ( *hmapStart + ( *(hmapStart + 1)) + ( *(hmapStart + 2)));
    pixel = sprite->data + ((y - *vmap) * (sprite->width)) +
                (x - *hmapStart);
    for ( j = y - 1; j <= ( y + 1); j++)
    {
        hmap = hmapStart;
        for ( i = x - 1; i <= ( x + 1); i++)
        {
            if ((( j != y) || ( x != i)) && ( !(*pixel))) return true;
            pixel += *hmap++;
        }
        pixel += ((*vmap++ - 1) * sprite->width) + rowPlus;
    }
    return false;
}

void EraseSpriteTable( void)

{
    long                i;
    spriteType          *aSprite;

    aSprite = gSpriteTable.get();
    for ( i = 0; i < kMaxSpriteNum; i++)
    {
        if (aSprite->table.get() != nil) {
        #ifndef kDrawOverride
            if ( aSprite->thisRect.left < aSprite->thisRect.right)
            {
                ChunkErasePixMap( gOffWorld, &(aSprite->thisRect));
            }
        #endif
            if ( aSprite->killMe)
                aSprite->lastRect = aSprite->thisRect;
        }
//      CopySaveWorldToOffWorld( &(gSprite[i].thisRect));
        aSprite++;
    }
}


void DrawSpriteTableInOffWorld( Rect *clipRect)

{
    long            i, trueScale, layer, tinySize;
    Rect        sRect;
    spritePix       aSpritePix;
    TypedHandle<natePixType> pixTable;
    int             whichShape;
    spriteType      *aSprite;

    aSprite = gSpriteTable.get();

//  WriteDebugLong( gAbsoluteScale);
    if ( gAbsoluteScale >= kSpriteBlipThreshhold)
    {
        for ( layer = kFirstSpriteLayer; layer <= kLastSpriteLayer; layer++)
        {
            aSprite = gSpriteTable.get();
            for ( i = 0; i < kMaxSpriteNum; i++)
            {
                if ((aSprite->table.get() != nil) && ( !aSprite->killMe) && ( aSprite->whichLayer == layer))
                {

                    pixTable = aSprite->table;
                    whichShape = aSprite->whichShape;

    //      if (( whichShape < 0) || ( whichShape >= GetNatePixTablePixNum( pixTable)))
    //          WriteDebugLong( whichShape);

                    aSpritePix.data = GetNatePixTableNatePixData( pixTable, aSprite->whichShape);
                    aSpritePix.center.h = GetNatePixTableNatePixHRef( pixTable, whichShape);
                    aSpritePix.center.v = GetNatePixTableNatePixVRef( pixTable, whichShape);
                    aSpritePix.width = GetNatePixTableNatePixWidth( pixTable, whichShape);
                    aSpritePix.height = GetNatePixTableNatePixHeight( pixTable, whichShape);

                    trueScale = aSprite->scale * gAbsoluteScale;
                    trueScale >>= SHIFT_SCALE;

                #ifndef kDrawOverride
                    switch( aSprite->style)
                    {
                        case spriteNormal:
                            OptScaleSpritePixInPixMap( &aSpritePix, aSprite->where, trueScale,
                                &sRect, clipRect, gOffWorld);
                            break;

                        case spriteColor:
                            ColorScaleSpritePixInPixMap( &aSpritePix, aSprite->where, trueScale,
                                    &sRect, clipRect, gOffWorld, Randomize( kStaticTableSize),
                                    aSprite->styleColor, aSprite->styleData);
                            break;

                        case spriteStatic:
                            StaticScaleSpritePixInPixMap( &aSpritePix, aSprite->where, trueScale,
                                    &sRect, clipRect, gOffWorld, Randomize( kStaticTableSize));
                            break;
                    }
//                  sRect.top = sRect.left = sRect.bottom = sRect.right = 0;
                    mCopyAnyRect( aSprite->thisRect, sRect);
    //              LongRectToRect( &sRect, &(aSprite->thisRect));
                #endif
                }
                aSprite++;
            }
        }
    } else
    {
        for ( layer = kFirstSpriteLayer; layer <= kLastSpriteLayer; layer++)
        {
            aSprite = gSpriteTable.get();
            for ( i = 0; i < kMaxSpriteNum; i++)
            {
                tinySize = aSprite->tinySize & kBlipSizeMask;
                if (( aSprite->table.get() != nil) && ( !aSprite->killMe) &&
                    ( aSprite->tinyColor != kNoTinyColor) &&
                    ( tinySize)
                    && ( aSprite->whichLayer == layer))
                {
/*                  sRect.left = aSprite->where.h - (aSprite->tinySize >> 1L);
                    sRect.right = sRect.left + aSprite->tinySize;
                    sRect.top = aSprite->where.v - (aSprite->tinySize >> 1L);
                    sRect.bottom = sRect.top + aSprite->tinySize;
*/
                    sRect.left = aSprite->where.h - tinySize;
                    sRect.right = aSprite->where.h + tinySize;
                    sRect.top = aSprite->where.v - tinySize;
                    sRect.bottom = aSprite->where.v + tinySize;
//                  WriteDebugLong( aSprite->tinySize);
                #ifndef kDrawOverride
                    switch( aSprite->tinySize & kBlipTypeMask)
                    {
                        case kTriangeUpBlip:
                            DrawNateTriangleUpClipped( gOffWorld, &sRect, *clipRect, 0, 0, aSprite->tinyColor);
                            break;

                        case kSolidSquareBlip:
                            DrawNateRectClipped( gOffWorld, &sRect, *clipRect, 0, 0, aSprite->tinyColor);
                            break;

                        case kPlusBlip:
                            DrawNatePlusClipped( gOffWorld, &sRect, *clipRect, 0, 0, aSprite->tinyColor);
                            break;

                        case kDiamondBlip:
                            DrawNateDiamondClipped( gOffWorld, &sRect, *clipRect, 0, 0, aSprite->tinyColor);
                            break;

                        case kFramedSquareBlip:
                            DrawNateRectClipped( gOffWorld, &sRect, *clipRect, 0, 0, aSprite->tinyColor);
                            break;

                        default:
                            sRect.top = sRect.left = sRect.bottom = sRect.right = 0;
//                          NumToString( aSprite->resID, hack);
//                          DebugStr( hack);
                            break;
                    }

                    mCopyAnyRect( aSprite->thisRect, sRect);
                #endif
                }

                aSprite++;
            }
        }
    }
}

void GetOldSpritePixData( spriteType *sourceSprite, spritePix *oldData)

{
    short               whichShape;
    TypedHandle<natePixType> pixTable;

    if (sourceSprite->table.get() != nil) {
        pixTable = sourceSprite->table;
        whichShape = sourceSprite->whichShape;

//      WriteDebugLong( pixTable);
        if ( whichShape >= GetNatePixTablePixNum( pixTable))
        {
            Str255  resIDString, shapeNumString;

            NumToString( sourceSprite->resID, resIDString);
            NumToString( whichShape, shapeNumString);

            ShowErrorAny( eQuitErr, kErrorStrID, nil, shapeNumString, nil, resIDString,
                SPRITE_DATA_ERROR, -1, 78, -1, __FILE__, sourceSprite->resID);
//          Debugger();
            return;
        }
        oldData->data = GetNatePixTableNatePixData( pixTable, sourceSprite->whichShape);
        oldData->center.h = GetNatePixTableNatePixHRef( pixTable, whichShape);
        oldData->center.v = GetNatePixTableNatePixVRef( pixTable, whichShape);
        oldData->width = GetNatePixTableNatePixWidth( pixTable, whichShape);
        oldData->height = GetNatePixTableNatePixHeight( pixTable, whichShape);
    }
}

void ShowSpriteTable( void)

{
    Rect            tRect;
    long            i;
    spriteType      *aSprite;

    aSprite = gSpriteTable.get();
    for ( i = 0; i < kMaxSpriteNum; i++)
    {
        if (aSprite->table.get() != nil) {
            // if thisRect is null
            if (( aSprite->thisRect.right <= aSprite->thisRect.left) ||
                ( aSprite->thisRect.bottom <= aSprite->thisRect.top))
            {
                // and lastRect isn't then
                if ( aSprite->lastRect.right > aSprite->lastRect.left)
                {
                    // show lastRect
                    ChunkCopyPixMapToScreenPixMap( gOffWorld, aSprite->lastRect, gActiveWorld);
                }
            // else if lastRect is null (we now know this rect isn't)
            } else if (( aSprite->lastRect.right <= aSprite->lastRect.left) ||
                ( aSprite->lastRect.bottom <= aSprite->lastRect.top))
            {
                // then show thisRect

                ChunkCopyPixMapToScreenPixMap( gOffWorld, aSprite->thisRect,
                        gActiveWorld);

            // else if the rects don't intersect
            } else if ( ( aSprite->lastRect.right < ( aSprite->thisRect.left - 32)) ||
                        ( aSprite->lastRect.left > ( aSprite->thisRect.right + 32)) ||
                        ( aSprite->lastRect.bottom < ( aSprite->thisRect.top - 32)) ||
                        ( aSprite->lastRect.top > ( aSprite->thisRect.bottom + 32)))
            {
                // then draw them individually


                ChunkCopyPixMapToScreenPixMap( gOffWorld, aSprite->lastRect, gActiveWorld);
                ChunkCopyPixMapToScreenPixMap( gOffWorld, aSprite->thisRect, gActiveWorld);

            // else the rects do intersect (and we know are both non-null)
            } else
            {
                tRect = aSprite->thisRect;
                mBiggestRect( tRect, aSprite->lastRect);

                ChunkCopyPixMapToScreenPixMap(gOffWorld, tRect, gActiveWorld);

            }
            aSprite->lastRect = aSprite->thisRect;
            if ( aSprite->killMe)
                RemoveSprite( aSprite);
        }
        aSprite++;
    }
}

// CullSprites: if you're keeping track of sprites, but not showing them, use this to remove
// dead sprites. (Implemented for Asteroid level, where game is run to populate scenario with Asteroids
// before the player actually starts.

void CullSprites( void)
{
    long            i;
    spriteType      *aSprite;

    aSprite = gSpriteTable.get();
    for ( i = 0; i < kMaxSpriteNum; i++)
    {
        if (aSprite->table.get() != nil) {
            aSprite->lastRect = aSprite->thisRect;
            if ( aSprite->killMe)
                RemoveSprite( aSprite);
        }
        aSprite++;
    }
}

void TestByte(unsigned char *dbyte, PixMap *pixMap, unsigned char* name) {
    long            rowbytes, rowplus;
    unsigned char* lbyte;

    rowbytes = 0x0000ffff & ((pixMap->rowBytes | ROW_BYTES_MASK) ^ ROW_BYTES_MASK);
    rowplus = (pixMap->bounds.bottom - pixMap->bounds.top + 1) * rowbytes;
    lbyte = pixMap->baseAddr + rowplus;
    if (( dbyte < pixMap->baseAddr) || ( dbyte >= lbyte))
    {
        DebugStr( name);
        WriteDebugLine(name);
        WriteDebugLine("\p<<BAD>>");
    }
}

}  // namespace antares

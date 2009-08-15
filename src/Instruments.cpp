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

// Instruments.c
// REQUIRES THAT ARES INTERFACES DATA FILE HAS BEEN OPENED!

#include "Instruments.hpp"

//#include <math routines.h>
#include <QDOffscreen.h>

#include "Admiral.hpp"
#include "AresGlobalType.hpp"
#include "ColorTranslation.hpp"
#include "ConditionalMacros.h"
#include "Debug.hpp"
#include "Error.hpp"
#include "GXMath.h"
#include "HandleHandling.hpp"
#include "MathMacros.hpp"
#include "Minicomputer.hpp"
#include "NateDraw.hpp"
#include "OffscreenGWorld.hpp"
#include "PlayerShip.hpp"
#include "Resources.h"
#include "Rotation.hpp"
#include "ScreenLabel.hpp"
//#include "SoundFX.hpp"
#include "SpaceObject.hpp"
#include "SpriteCursor.hpp"
#include "ToolUtils.h"
#include "UniverseUnit.hpp"

#define kRadarBlipNum       50L
#define kRadarColor         GREEN
#define kSectorColor        SKY_BLUE

#define kRadarLeft          6
#define kRadarTop           6
#define kRadarRight         116
#define kRadarBottom        116
#define kRadarCenter        55
#define kRadarColorSteps    14

#define kScaleListNum       64L
#define kScaleListShift     6L
#define kInstLeftPictID     501
#define kInstRightPictID    512
#define kSiteDistance       200
#define kSiteSize           16
#define kSiteCoordNum       6L
#define kSiteCoordOffset    (kMaxSectorLine << 2L) // offset in SectorLineData to site coords

#define kCursorCoordNum     2L
#define kCursorCoordOffset  ((kMaxSectorLine << 2L) + (kSiteCoordNum << 1L))
#define kCursorBoundsSize   16

#define kCenterScaleSize    983040L//61440L         // = 240 * SCALE_SCALE

#define kInstrumentError "\pINST"

#define kBarIndicatorHeight 98
#define kBarIndicatorWidth  9//13//15
#define kBarIndicatorLeft   6//613

#define kShieldBar          0
#define kEnergyBar          1
#define kBatteryBar         2
#define kFineMoneyBar       3
#define kGrossMoneyBar      4

#define kMaxSectorLine          32L
#define kMinGraphicSectorSize   90

#define kGrossMoneyLeft         11//12//620
#define kGrossMoneyTop          48//49
#define kGrossMoneyWidth        14
#define kGrossMoneyHeight       42
#define kGrossMoneyHBuffer      2
#define kGrossMoneyVBuffer      4
#define kGrossMoneyBarWidth     10
#define kGrossMoneyBarHeight    5
#define kGrossMoneyBarNum       7
#define kGrossMoneyBarValue     5120000//20000
#define kGrossMoneyColor        YELLOW

#define kFineMoneyLeft          25//27//635
#define kFineMoneyTop           48//42
#define kFineMoneyHBuffer       1
#define kFineMoneyVBuffer       1
#define kFineMoneyBarWidth      2//3
#define kFineMoneyBarHeight     4
#define kFineMoneyBarNum        100
#define kFineMoneyBarMod        5120000//20000
#define kFineMoneyBarValue      51200//200
#define kFineMoneyColor         PALE_GREEN
#define kFineMoneyNeedColor     ORANGE
#define kFineMoneyUseColor      SKY_BLUE

#define kMaxMoneyValue          35840000 // = kGrossMoneyBarValue * 7

#define kMiniBuildTimeLeft      ( gAresGlobal->gRightPanelLeftEdge + 10)
#define kMiniBuildTimeRight     ( gAresGlobal->gRightPanelLeftEdge + 22)

#define kMiniBuildTimeTop       8
#define kMiniBuildTimeBottom    37

#define kMinimumAutoScale   2 //128 //kOneQuarterScale
#define kBlipThreshhold     kOneQuarterScale            // should be the same as kSpriteBlipThreshhold
#define kSuperSmallScale    2

#define kMouseSleepTime     60

inline void mWideASR8(UnsignedWide& mwide) {
    (mwide).lo >>= 8;
    (mwide).lo |= (mwide).hi << 24;
    (mwide).hi >>= 8;
}

#define kSectorLineBrightness   DARKER

extern  CWindowPtr      gTheWindow; // hack to copy bar indicators to offworld
extern aresGlobalType   *gAresGlobal;
extern spaceObjectType**    gSpaceObjectData;
extern Handle           gColorTranslateTable, gRotTable;//, gAresGlobal->gAdmiralData;
extern spaceObjectType  *gScrollStarObject;
extern PixMapHandle     thePixMapHandle;
extern long             gNatePortLeft, gNatePortTop,
                        /*gAresGlobal->gGameTime, gAresGlobal->gGameStartTime,
                        gAresGlobal->gClosestObject,
                        gAresGlobal->gFarthestObject,*/
                        gAbsoluteScale, /*gAresGlobal->gZoomMode,
                        gAresGlobal->gPlayerAdmiralNumber,*/
                        WORLD_WIDTH, WORLD_HEIGHT, CLIP_LEFT, CLIP_TOP, CLIP_RIGHT,
                        CLIP_BOTTOM/*, gAresGlobal->gCenterScaleV*/;
extern coordPointType   gGlobalCorner;

extern GWorldPtr        gOffWorld, gRealWorld, gSaveWorld;

//Handle                    gAresGlobal->gRadarBlipData = nil, gAresGlobal->gScaleList = nil,
//                      gAresGlobal->gSectorLineData = nil;
/*long                  gAresGlobal->gRadarCount = 0, gAresGlobal->gRadarSpeed = 30,
                        gAresGlobal->gRadarRange = kRadarSize * 50,
                        gAresGlobal->gWhichScaleNum = 0,
                        gAresGlobal->gLastScale = SCALE_SCALE,
                        gAresGlobal->gInstrumentTop = 0,
                        gAresGlobal->gRightPanelLeftEdge = 608;*/
coordPointType          gLastGlobalCorner;
//barIndicatorType      gAresGlobal->gBarIndicator[ kBarIndicatorNum];
//short                 gAresGlobal->gMouseActive = kMouseOff; // 0 = off now, 1 = turning off, 2 = on

int InstrumentInit( void)

{

    gAresGlobal->gInstrumentTop = (WORLD_HEIGHT / 2) - ( kPanelHeight / 2);
    gAresGlobal->gRightPanelLeftEdge = WORLD_WIDTH - kRightPanelWidth;

    gAresGlobal->gRadarBlipData = NewHandle( sizeof( longPointType) * kRadarBlipNum);
    if ( gAresGlobal->gRadarBlipData == nil)
    {
        ShowErrorAny( eQuitErr, kErrorStrID, nil, nil, nil, nil, MEMORY_ERROR, -1, -1, -1, __FILE__, 1);
        return ( MEMORY_ERROR);
    }
    /*
    MoveHHi( gAresGlobal->gRadarBlipData);
    HLock( gAresGlobal->gRadarBlipData);
    */
    mHandleLockAndRegister( gAresGlobal->gRadarBlipData, nil, nil, nil, "\pgAresGlobal->gRadarBlipData");

    gAresGlobal->gScaleList = NewHandle( sizeof( long) * kScaleListNum);
    if ( gAresGlobal->gScaleList == nil)
    {
        ShowErrorAny( eQuitErr, kErrorStrID, nil, nil, nil, nil, MEMORY_ERROR, -1, -1, -1, __FILE__, 2);
        return ( MEMORY_ERROR);
    }
    /*
    MoveHHi( gAresGlobal->gScaleList);
    HLock( gAresGlobal->gScaleList);
    */
    mHandleLockAndRegister( gAresGlobal->gScaleList, nil, nil, nil, "\pgAresGlobal->gScaleList");

    gAresGlobal->gSectorLineData = NewHandle( sizeof( long) * kMaxSectorLine * 4L + sizeof( long) *
                    kSiteCoordNum * 2L + sizeof( long) * kCursorCoordNum * 2L);
    if ( gAresGlobal->gSectorLineData == nil)
    {
        ShowErrorAny( eQuitErr, kErrorStrID, nil, nil, nil, nil, MEMORY_ERROR, -1, -1, -1, __FILE__, 3);
        return ( MEMORY_ERROR);
    }
    /*
    MoveHHi( gAresGlobal->gSectorLineData);
    HLock( gAresGlobal->gSectorLineData);
    */
    mHandleLockAndRegister( gAresGlobal->gSectorLineData, nil, nil, nil, "\pgAresGlobal->gSectorLineData");

    ResetInstruments();

    return( MiniScreenInit());
}

void InstrumentCleanup( void)

{
    if ( gAresGlobal->gRadarBlipData != nil) DisposeHandle( gAresGlobal->gRadarBlipData);
    MiniScreenCleanup();
}

void ResetInstruments( void)

{
    long            *l, i;
    longPointType   *lp;

    gAresGlobal->gRadarCount = 0;
    gAresGlobal->gRadarSpeed = 30;
    gAresGlobal->gRadarRange = kRadarSize * 50;
    gAresGlobal->gLastScale = gAbsoluteScale = SCALE_SCALE;
    gAresGlobal->gWhichScaleNum = 0;
    gLastGlobalCorner.h = gLastGlobalCorner.v = 0;
    gAresGlobal->gMouseActive = kMouseOff;
    l = reinterpret_cast<long *>(*gAresGlobal->gScaleList);
    for ( i = 0; i < kScaleListNum; i++)
    {
        *l = SCALE_SCALE;
        l++;
    }

    for ( i = 0; i < kBarIndicatorNum; i++)
    {
        gAresGlobal->gBarIndicator[i].lastValue = gAresGlobal->gBarIndicator[i].thisValue = -1;
    }
    // the shield bar
    gAresGlobal->gBarIndicator[kShieldBar].top = 359 + gAresGlobal->gInstrumentTop;
    gAresGlobal->gBarIndicator[kShieldBar].color = SKY_BLUE;

    gAresGlobal->gBarIndicator[kEnergyBar].top = 231 + gAresGlobal->gInstrumentTop;
    gAresGlobal->gBarIndicator[kEnergyBar].color = GOLD;

    gAresGlobal->gBarIndicator[kBatteryBar].top = 103 + gAresGlobal->gInstrumentTop;
    gAresGlobal->gBarIndicator[kBatteryBar].color = SALMON;

    lp = reinterpret_cast<longPointType *>(*gAresGlobal->gRadarBlipData);
    for ( i = 0; i < kRadarBlipNum; i++)
    {
        lp->h = -1;
        lp++;
    }
    ResetSectorLines();
}

void ResetSectorLines( void)

{
    long    *l, count;

    l = reinterpret_cast<long*>(*gAresGlobal->gSectorLineData);

    for ( count = 0; count < kMaxSectorLine; count++)
    {
        *l = -1;
        l++;
        *l = -1;
        l++;
        *l = -1;
        l++;
        *l = -1;
        l++;
    }
}

void UpdateRadar( long unitsDone)

{
    longRect        lRect;
    spaceObjectType *anObject;
    baseObjectType  *baseObject;
    long            dx, oCount, rcount, x, y, *scaleval;
    const long      rrange = gAresGlobal->gRadarRange >> 1L;
    longPointType   *lp;
    unsigned char   color, color2, *dByte;
    unsigned long   bestScale, rootCorrect, distance, difference, dcalc;
    transColorType  *transColor;
    PixMapHandle    offPixBase;
    admiralType     *admiral;
    Rect            tRect;
    Boolean         doDraw;
    UnsignedWide    tempWide, hugeDistance;

    if ( gScrollStarObject != nil)
    {
        if ( gScrollStarObject->offlineTime <= 0) doDraw = true;
        else
        {
            if ( Randomize( gScrollStarObject->offlineTime) < 5) doDraw = true;
            else doDraw = false;
        }
    } else doDraw = false;

    if ( unitsDone < 0) unitsDone = 0;
    gAresGlobal->gRadarCount -= unitsDone;
    if ( gAresGlobal->gMouseActive > kMouseTurningOff)
        gAresGlobal->gMouseActive += unitsDone;

    if ( gAresGlobal->gRadarCount <= 0)
    {
        mGetTranslateColorShade( kRadarColor, VERY_DARK, color, transColor);
    } else
    {
        mGetTranslateColorShade( kRadarColor, (( kRadarColorSteps * gAresGlobal->gRadarCount) / gAresGlobal->gRadarSpeed) + 1, color, transColor);
    }

    mGetRowBytes( oCount, *thePixMapHandle);

    if ( doDraw)
    {
        lp = reinterpret_cast<longPointType*>(*gAresGlobal->gRadarBlipData);
        for ( rcount = 0; rcount < kRadarBlipNum; rcount++)
        {
            if ( lp->h >= 0)
            {
                mSetNatePixel( dByte, oCount, lp->h, lp->v, gNatePortLeft << 2L,
                        gNatePortTop, *thePixMapHandle, color);
            }
            lp++;
        }
    }

    if (( gScrollStarObject != nil) && ( gScrollStarObject->active) )
    {
        if (( gAresGlobal->gRadarCount <= 0) && ( doDraw))
        {
            offPixBase = GetGWorldPixMap( gOffWorld);
            DrawInOffWorld();

            MacSetRect( &tRect, kRadarLeft + 1, kRadarTop + 1 + gAresGlobal->gInstrumentTop, kRadarRight - 1,
                    kRadarBottom - 1 + gAresGlobal->gInstrumentTop);
            RectToLongRect( &tRect, &lRect);
            mGetTranslateColorShade( kRadarColor, DARKEST, color, transColor);
            DrawNateRect( *offPixBase, &lRect, 0, 0, color);
            SetTranslateColorShadeFore( kRadarColor, VERY_LIGHT);
            MacFrameRect( &tRect);

            dx = gScrollStarObject->location.h - gGlobalCorner.h;
            dx = dx * kRadarSize;
            dx /= gAresGlobal->gRadarRange;

            lRect.left = kRadarLeft + kRadarCenter - dx;
            if ( lRect.left <= tRect.left) lRect.left = tRect.left + 1;
            lRect.right = kRadarLeft + kRadarCenter + dx;
            if ( lRect.right >= tRect.right) lRect.right = tRect.right - 1;
            lRect.top = kRadarTop + kRadarCenter - dx + gAresGlobal->gInstrumentTop;
            if ( lRect.top <= tRect.top) lRect.top = tRect.top + 1;
            lRect.bottom = kRadarTop + kRadarCenter + dx + gAresGlobal->gInstrumentTop;
            if ( lRect.bottom >= tRect.bottom) lRect.bottom = tRect.bottom - 1;
            mGetTranslateColorShade( kRadarColor, VERY_DARK, color, transColor);
            DrawNateRect( *offPixBase, &lRect, 0, 0, color);
            NormalizeColors();
            DrawInRealWorld();
            ChunkCopyPixMapToScreenPixMap( *offPixBase, &tRect, *thePixMapHandle);

            lp = reinterpret_cast<longPointType*>(*gAresGlobal->gRadarBlipData);
            for ( rcount = 0; rcount < kRadarBlipNum; rcount++)
            {
                lp->h = -1;
                lp++;
            }
            rcount = 0;

            lp = reinterpret_cast<longPointType*>(*gAresGlobal->gRadarBlipData);
            gAresGlobal->gRadarCount = gAresGlobal->gRadarSpeed;
            anObject = *gSpaceObjectData;

            for ( oCount = 0; oCount < kMaxSpaceObject; oCount++)
            {
                if (( anObject->active) && (anObject != gScrollStarObject))
                {
                    x = ( anObject->location.h - gScrollStarObject->location.h);
                    y = ( anObject->location.v - gScrollStarObject->location.v);
                    if ( rcount < kRadarBlipNum)
                    {
                        if (( x >= -( rrange)) && ( x < ( rrange)))
                        {
                            if (( y >= -( rrange)) && ( y < ( rrange)))
                            {
                                x = ( x * kRadarSize) / gAresGlobal->gRadarRange + kRadarCenter + kRadarLeft;
                                y = ( y * kRadarSize) / gAresGlobal->gRadarRange + kRadarCenter + kRadarTop
                                        + gAresGlobal->gInstrumentTop;
                                lp->h = x;
                                lp->v = y;
                                rcount++;
                                lp++;
                            }
                        }
                    }
                }
                anObject++;
            }
        } else if (!doDraw)
        {
            MacSetRect( &tRect, kRadarLeft + 1, kRadarTop + 1 + gAresGlobal->gInstrumentTop, kRadarRight - 1,
                    kRadarBottom - 1 + gAresGlobal->gInstrumentTop);
            RectToLongRect( &tRect, &lRect);
            mGetTranslateColorShade( kRadarColor, DARKEST, color, transColor);
            DrawNateRect( *thePixMapHandle, &lRect, gNatePortLeft << 2L, gNatePortTop, color);
        }

        switch ( gAresGlobal->gZoomMode)
        {
            case kNearestFoeZoom:
            case kNearestAnythingZoom:
                anObject = *gSpaceObjectData + gAresGlobal->gClosestObject;
                bestScale = MIN_SCALE;
                /*
                dx = ( (long)anObject->location.h - (long)gScrollStarObject->location.h);
                dy = ( (long)anObject->location.v - (long)gScrollStarObject->location.v);
                if (( ABS(dx) < kMaximumRelevantDistance) && ( ABS( dy) < kMaximumRelevantDistance))
                {
                    bestScale = dx * dx + dy * dy;
                    bestScale = lsqrt( bestScale);
                    if ( bestScale == 0) bestScale = 1;
                    bestScale = gAresGlobal->gCenterScaleV / bestScale;
                    if ( bestScale < SCALE_SCALE) bestScale = ( bestScale >> 2L) + ( bestScale >> 1L);
                    if ( bestScale > SCALE_SCALE) bestScale = SCALE_SCALE;
                    if ( bestScale < kMinimumAutoScale) bestScale = kMinimumAutoScale;
                }*/
                hugeDistance = anObject->distanceFromPlayer;
                if ( hugeDistance.lo == 0) // if this is true, then we haven't calced its distance
                {
                    difference = ABS( implicit_cast<long>(gScrollStarObject->location.h) - implicit_cast<long>(anObject->location.h));
                    dcalc = difference;
                    difference =  ABS( implicit_cast<long>(gScrollStarObject->location.v) - implicit_cast<long>(anObject->location.v));
                    distance = difference;

                    if (( dcalc > kMaximumRelevantDistance) ||
                        ( distance > kMaximumRelevantDistance))
                    {
                        tempWide.hi = 0;
                        tempWide.lo = dcalc;    // must be positive
                        MyWideMul( tempWide.lo, tempWide.lo, &hugeDistance);
                        tempWide.lo = distance;
                        MyWideMul( tempWide.lo, tempWide.lo, &tempWide);
                        MyWideAdd( &hugeDistance, &tempWide);
                    } else
                    {
                        hugeDistance.hi = 0;
                        hugeDistance.lo = distance * distance + dcalc * dcalc;
                    }
                }
                if ( hugeDistance.hi == 0) bestScale = lsqrt( hugeDistance.lo);
                else
                {
                    rootCorrect = 0;
                    do
                    {
                        rootCorrect += 4;
                        mWideASR8( hugeDistance);
                    } while ( hugeDistance.hi);
                    bestScale = lsqrt( hugeDistance.lo);
                    bestScale <<= rootCorrect;
                }
                if ( bestScale == 0) bestScale = 1;
                bestScale = gAresGlobal->gCenterScaleV / bestScale;
                if ( bestScale < SCALE_SCALE) bestScale = ( bestScale >> 2L) + ( bestScale >> 1L);
                if ( bestScale > SCALE_SCALE) bestScale = SCALE_SCALE;
                if ( bestScale < kMinimumAutoScale) bestScale = kMinimumAutoScale;
                break;

            case kActualSizeZoom:
                bestScale = SCALE_SCALE;
                break;

            case kEighthSizeZoom:
                bestScale = kOneEighthScale;
                break;

            case kQuarterSizeZoom:
                bestScale = kOneQuarterScale;
                break;

            case kHalfSizeZoom:
                //bestScale = kOneQuarterScale;
                bestScale = kOneHalfScale;
                break;

            case kTimesTwoZoom:
                bestScale = kTimesTwoScale;
                break;

            case kSmallestZoom:
//              bestScale = kSuperSmallScale;
                anObject = *gSpaceObjectData + gAresGlobal->gFarthestObject;
                bestScale = MIN_SCALE;
                /*
                dx = ( (long)anObject->location.h - (long)gScrollStarObject->location.h);
                dy = ( (long)anObject->location.v - (long)gScrollStarObject->location.v);
                if (( ABS(dx) < kMaximumRelevantDistance) && ( ABS( dy) < kMaximumRelevantDistance))
                {
                    bestScale = dx * dx + dy * dy;
                    bestScale = lsqrt( bestScale);
                    if ( bestScale == 0) bestScale = 1;
                    bestScale = gAresGlobal->gCenterScaleV / bestScale;
                    if ( bestScale < SCALE_SCALE) bestScale = ( bestScale >> 2L) + ( bestScale >> 1L);
                    if ( bestScale > SCALE_SCALE) bestScale = SCALE_SCALE;
                    if ( bestScale < kMinimumAutoScale) bestScale = kMinimumAutoScale;
                }*/
                tempWide = anObject->distanceFromPlayer;
                if ( tempWide.hi == 0) bestScale = lsqrt( tempWide.lo);
                else
                {
                    rootCorrect = 0;
                    do
                    {
                        rootCorrect += 4;
                        mWideASR8( tempWide);
                    } while ( tempWide.hi);
                    bestScale = lsqrt( tempWide.lo);
                    bestScale <<= rootCorrect;
                }
                if ( bestScale == 0) bestScale = 1;
                bestScale = gAresGlobal->gCenterScaleV / bestScale;
                if ( bestScale < SCALE_SCALE) bestScale = ( bestScale >> 2L) + ( bestScale >> 1L);
                if ( bestScale > SCALE_SCALE) bestScale = SCALE_SCALE;
                if ( bestScale < kMinimumAutoScale) bestScale = kMinimumAutoScale;
                break;
        }

        for ( x = 0; x < unitsDone; x++)
        {
            scaleval = reinterpret_cast<long *>(*gAresGlobal->gScaleList) + gAresGlobal->gWhichScaleNum;
            *scaleval = bestScale;
            gAresGlobal->gWhichScaleNum++;
            if ( gAresGlobal->gWhichScaleNum == kScaleListNum) gAresGlobal->gWhichScaleNum = 0;
        }

        scaleval = reinterpret_cast<long *>(*gAresGlobal->gScaleList);
        rcount = 0;
        for ( oCount = 0; oCount < kScaleListNum; oCount++)
        {
            rcount += *scaleval++;
        }
        rcount >>= kScaleListShift;

        gAbsoluteScale = rcount;
//      gAbsoluteScale = SCALE_SCALE;

        baseObject = gScrollStarObject->baseType;
        UpdateBarIndicator( kShieldBar, gScrollStarObject->health, baseObject->health, thePixMapHandle);
        UpdateBarIndicator( kEnergyBar, gScrollStarObject->energy, baseObject->energy, thePixMapHandle);
        UpdateBarIndicator( kBatteryBar, gScrollStarObject->battery, baseObject->energy * 5, thePixMapHandle);

        // SHOW ME THE MONEY
        admiral = mGetAdmiralPtr( gAresGlobal->gPlayerAdmiralNumber);
        y = x = admiral->cash;
        if ( x < 0) x = 0;
        if ( x >= kMaxMoneyValue) x = kMaxMoneyValue - 1;
        gAresGlobal->gBarIndicator[kFineMoneyBar].thisValue = x % kFineMoneyBarMod;
        gAresGlobal->gBarIndicator[kFineMoneyBar].thisValue /= kFineMoneyBarValue;
        x = MiniComputerGetPriceOfCurrentSelection();
        x /= kFineMoneyBarValue;

        if (( gAresGlobal->gBarIndicator[kFineMoneyBar].thisValue !=
            gAresGlobal->gBarIndicator[kFineMoneyBar].lastValue) ||
            ( x != gAresGlobal->gLastSelectedBuildPrice))
        {
            lRect.left = kFineMoneyLeft + kFineMoneyHBuffer + gAresGlobal->gRightPanelLeftEdge;
            lRect.right = lRect.left + kFineMoneyBarWidth;

            // if selected build price is greater than the money we have
                if ( gAresGlobal->gBarIndicator[kFineMoneyBar].thisValue < x)
                {
                    // draw the money we have
                    mGetTranslateColorShade( kFineMoneyColor, VERY_LIGHT, color, transColor);
                    mGetTranslateColorShade( kFineMoneyColor, LIGHT, color2, transColor);

                    for ( rcount = 0;
                        rcount < gAresGlobal->gBarIndicator[kFineMoneyBar].thisValue;
                        rcount++)
                    {
                        lRect.top = kFineMoneyTop + gAresGlobal->gInstrumentTop +
                                kFineMoneyVBuffer + rcount * kFineMoneyBarHeight;
                        lRect.bottom = lRect.top + kFineMoneyBarHeight - 1;
                        if ( rcount % 5)
                            DrawNateRect( *thePixMapHandle, &lRect,
                                gNatePortLeft << 2L, gNatePortTop, color2);
                        else
                            DrawNateRect( *thePixMapHandle, &lRect,
                                gNatePortLeft << 2L, gNatePortTop, color);

                    }

                    // draw the money we need
                    mGetTranslateColorShade( kFineMoneyNeedColor, MEDIUM, color, transColor);
                    mGetTranslateColorShade( kFineMoneyNeedColor, DARK, color2, transColor);

                    for ( rcount = gAresGlobal->gBarIndicator[kFineMoneyBar].thisValue;
                        rcount < x;
                        rcount++)
                    {
                        lRect.top = kFineMoneyTop + gAresGlobal->gInstrumentTop +
                                kFineMoneyVBuffer + rcount * kFineMoneyBarHeight;
                        lRect.bottom = lRect.top + kFineMoneyBarHeight - 1;
                        if ( rcount % 5)
                            DrawNateRect( *thePixMapHandle, &lRect,
                                gNatePortLeft << 2L, gNatePortTop, color2);
                        else
                            DrawNateRect( *thePixMapHandle, &lRect,
                                gNatePortLeft << 2L, gNatePortTop, color);

                    }

                    gAresGlobal->gBarIndicator[kFineMoneyBar].thisValue = x;
                } else
                {
                    // draw the money we'll have left
                    mGetTranslateColorShade( kFineMoneyColor, VERY_LIGHT, color, transColor);
                    mGetTranslateColorShade( kFineMoneyColor, LIGHT, color2, transColor);

                    for ( rcount = 0;
                        rcount < ( gAresGlobal->gBarIndicator[kFineMoneyBar].thisValue - x);
                        rcount++)
                    {
                        lRect.top = kFineMoneyTop + gAresGlobal->gInstrumentTop +
                                kFineMoneyVBuffer + rcount * kFineMoneyBarHeight;
                        lRect.bottom = lRect.top + kFineMoneyBarHeight - 1;
                        if ( rcount % 5)
                            DrawNateRect( *thePixMapHandle, &lRect,
                                gNatePortLeft << 2L, gNatePortTop, color2);
                        else
                            DrawNateRect( *thePixMapHandle, &lRect,
                                gNatePortLeft << 2L, gNatePortTop, color);

                    }

                    // draw the money we'll have left
                    mGetTranslateColorShade( kFineMoneyUseColor, VERY_LIGHT, color, transColor);
                    mGetTranslateColorShade( kFineMoneyUseColor, LIGHT, color2, transColor);

                    for ( rcount = (gAresGlobal->gBarIndicator[kFineMoneyBar].thisValue -x);
                        rcount < gAresGlobal->gBarIndicator[kFineMoneyBar].thisValue;
                        rcount++)
                    {
                        lRect.top = kFineMoneyTop + gAresGlobal->gInstrumentTop +
                                kFineMoneyVBuffer + rcount * kFineMoneyBarHeight;
                        lRect.bottom = lRect.top + kFineMoneyBarHeight - 1;
                        if ( rcount % 5)
                            DrawNateRect( *thePixMapHandle, &lRect,
                                gNatePortLeft << 2L, gNatePortTop, color2);
                        else
                            DrawNateRect( *thePixMapHandle, &lRect,
                                gNatePortLeft << 2L, gNatePortTop, color);

                    }

                }

                mGetTranslateColorShade( kFineMoneyColor, VERY_DARK, color, transColor);

                for ( rcount = gAresGlobal->gBarIndicator[kFineMoneyBar].thisValue;
                    rcount < kFineMoneyBarNum; rcount++)
                {
                    lRect.top = kFineMoneyTop + gAresGlobal->gInstrumentTop +
                        kFineMoneyVBuffer + rcount * kFineMoneyBarHeight;
                    lRect.bottom = lRect.top + kFineMoneyBarHeight - 1;
                    DrawNateRect( *thePixMapHandle, &lRect, gNatePortLeft << 2L,
                        gNatePortTop, color);
                }
        }

        gAresGlobal->gLastSelectedBuildPrice = x;

        gAresGlobal->gBarIndicator[kGrossMoneyBar].thisValue = y;
        gAresGlobal->gBarIndicator[kGrossMoneyBar].thisValue /= kGrossMoneyBarValue;
        if ( gAresGlobal->gBarIndicator[kGrossMoneyBar].thisValue != gAresGlobal->gBarIndicator[kGrossMoneyBar].lastValue)
        {
            lRect.left = kGrossMoneyLeft + kGrossMoneyHBuffer + gAresGlobal->gRightPanelLeftEdge;
            lRect.right = lRect.left + kGrossMoneyBarWidth;
            mGetTranslateColorShade( kGrossMoneyColor, VERY_LIGHT, color, transColor);

            for ( rcount = 0; rcount < gAresGlobal->gBarIndicator[kGrossMoneyBar].thisValue; rcount++)
            {
                lRect.top = kGrossMoneyTop + gAresGlobal->gInstrumentTop + kGrossMoneyVBuffer + rcount *
                            kGrossMoneyBarHeight;
                lRect.bottom = lRect.top + kGrossMoneyBarHeight - 1;
                DrawNateRect( *thePixMapHandle, &lRect, gNatePortLeft << 2L, gNatePortTop, color);
            }
            mGetTranslateColorShade( kGrossMoneyColor, VERY_DARK, color, transColor);

            for ( rcount = gAresGlobal->gBarIndicator[kGrossMoneyBar].thisValue; rcount < kGrossMoneyBarNum; rcount++)
            {
                lRect.top = kGrossMoneyTop + gAresGlobal->gInstrumentTop + kGrossMoneyVBuffer + rcount *
                            kGrossMoneyBarHeight;
                lRect.bottom = lRect.top + kGrossMoneyBarHeight - 1;
                DrawNateRect( *thePixMapHandle, &lRect, gNatePortLeft << 2L, gNatePortTop, color);
            }
        }
    }
}

void DrawInstrumentPanel( WindowPtr whatPort)

{
    PicHandle       pict;
    Rect            tRect;

//  pict = GetPicture( kBackgroundPictID);
    gAresGlobal->gZoomMode = kNearestFoeZoom;
    DrawInSaveWorld();
    MacSetRect( &tRect, 0, 0, WORLD_WIDTH, WORLD_HEIGHT);
    SetTranslateColorFore( BLACK);
    PaintRect( &tRect);
    CopySaveWorldToOffWorld( &tRect);
    DrawInRealWorld();

    pict = GetPicture(kInstLeftPictID);
    if ( pict == nil)
    {
        ShowErrorAny( eContinueOnlyErr, kErrorStrID, nil, nil, nil, nil, OFFSCREEN_GRAPHICS_ERROR, -1, -1, -1, __FILE__, 2);
    } else
    {
        DrawInSaveWorld();
        tRect = (**pict).picFrame;
        tRect.left = 0;
        tRect.right = (**pict).picFrame.right - (**pict).picFrame.left;
        tRect.top = (WORLD_HEIGHT / 2) - ( (**pict).picFrame.bottom - (**pict).picFrame.top) / 2;
        tRect.bottom = tRect.top + (**pict).picFrame.bottom - (**pict).picFrame.top;
        DrawPicture( pict, &tRect);
        CopySaveWorldToOffWorld( &tRect);
        KillPicture(pict);
        DrawInRealWorld();
    }

    pict = GetPicture(kInstRightPictID);
    if ( pict == nil)
    {
        ShowErrorAny( eContinueOnlyErr, kErrorStrID, nil, nil, nil, nil, OFFSCREEN_GRAPHICS_ERROR, -1, -1, -1, __FILE__, 3);
    } else
    {
        DrawInSaveWorld();
        tRect = (**pict).picFrame;
        tRect.left = WORLD_WIDTH - ((**pict).picFrame.right - (**pict).picFrame.left);
        tRect.right = tRect.left + (**pict).picFrame.right - (**pict).picFrame.left;
        tRect.top = (WORLD_HEIGHT / 2) - ( (**pict).picFrame.bottom - (**pict).picFrame.top) / 2;
        tRect.bottom = tRect.top + (**pict).picFrame.bottom - (**pict).picFrame.top;
        DrawPicture( pict, &tRect);
        CopySaveWorldToOffWorld( &tRect);
        KillPicture(pict);
        DrawInRealWorld();
    }

    MacSetRect( &tRect, 0, 0, WORLD_WIDTH, WORLD_HEIGHT);
    CopyOffWorldToRealWorld( whatPort, &tRect);
    MakeMiniScreenFromIndString( 1);
    DrawMiniScreen();
    ResetInstruments();
    ClearMiniObjectData();
    ShowWholeMiniScreen();
    UpdatePlayerAmmo( -1, -1, -1);
    UpdateRadar(100);

}


void EraseSite( void)

{
    long            *l, *olp;
    short           sx, sy, sa, sb, sc, sd;
    PixMapHandle    offPixBase;
    longRect        clipRect;

    offPixBase = GetGWorldPixMap( gOffWorld);
    clipRect.left = CLIP_LEFT;
    clipRect.right = CLIP_RIGHT;
    clipRect.top = CLIP_TOP;
    clipRect.bottom = CLIP_BOTTOM;

    l = reinterpret_cast<long*>(*gAresGlobal->gSectorLineData) + kSiteCoordOffset;
    olp = l + kSiteCoordNum;
    sx = *(olp++) = *(l++);
    sy = *(olp++) = *(l++);
    sa = *(olp++) = *(l++);
    sb = *(olp++) = *(l++);
    sc = *(olp++) = *(l++);
    sd = *(olp++) = *(l++);
    DrawNateLine( *offPixBase, &clipRect, sa,
                sb, sx, sy, 0,
                0, BLACK);

    DrawNateLine( *offPixBase, &clipRect, sc,
                sd, sx, sy, 0,
                0, BLACK);
    DrawNateLine( *offPixBase, &clipRect, sc,
                sd, sa, sb, 0,
                0, BLACK);

    // do the cursor, too

    l = reinterpret_cast<long*>(*gAresGlobal->gSectorLineData) + kCursorCoordOffset;
    olp = l + kCursorCoordNum;
    sx = *(olp++) = *(l++);
    sy = *(olp++) = *(l++);

    if ( gAresGlobal->gMouseActive)
    {
        DrawNateLine( *offPixBase, &clipRect, sx,
                    clipRect.top, sx, (sy - kCursorBoundsSize), 0,
                    0, BLACK);
        DrawNateLine( *offPixBase, &clipRect, sx,
                    (sy + kCursorBoundsSize), sx, clipRect.bottom - 1, 0,
                    0, BLACK);
        DrawNateLine( *offPixBase, &clipRect, clipRect.left,
                    sy, (sx - kCursorBoundsSize), sy, 0,
                    0, BLACK);
        DrawNateLine( *offPixBase, &clipRect, (sx + kCursorBoundsSize),
                    sy, clipRect.right - 1, sy, 0,
                    0, BLACK);
    }
}

void EraseSectorLines( void)

{
    long            *l, count, ol;
    PixMapHandle    offPixBase;
    longRect        clipRect;

    offPixBase = GetGWorldPixMap( gOffWorld);
    clipRect.left = CLIP_LEFT;
    clipRect.right = CLIP_RIGHT;
    clipRect.top = CLIP_TOP;
    clipRect.bottom = CLIP_BOTTOM;

    l = reinterpret_cast<long*>(*gAresGlobal->gSectorLineData);
    count = 0;

    while (( *l != -1) && ( count < kMaxSectorLine))
    {
        DrawNateLine( *offPixBase, &clipRect, *l, CLIP_TOP, *l, CLIP_BOTTOM, 0, 0, BLACK);
        ol = *l;
        *l = -1;
        l++;
        *l = ol;
        l++;
        count++;
    }

    l = reinterpret_cast<long*>(*gAresGlobal->gSectorLineData) + (kMaxSectorLine << 1L);
    count = 0;

    while (( *l != -1) && ( count < kMaxSectorLine))
    {
        DrawNateLine( *offPixBase, &clipRect, CLIP_LEFT, *l, CLIP_RIGHT, *l, 0, 0, BLACK);
        ol = *l;
        *l = -1;
        l++;
        *l = ol;
        l++;
        count++;
    }
}

void DrawSite( void)

{
    long            *l, count;
    short           sx, sy, sa, sb, sc, sd;
    smallFixedType  fa, fb, fc;
    transColorType  *transColor;
    unsigned char   color;
    PixMapHandle    offPixBase;
    longRect        clipRect;
    Point           cursorCoord;
    Boolean         doDraw;

    if ( gScrollStarObject != nil)
    {
        if ( gScrollStarObject->offlineTime <= 0) doDraw = true;
        else
        {
            if ( Randomize( gScrollStarObject->offlineTime) < 5) doDraw = true;
            else doDraw = false;
        }
    } else doDraw = false;

    offPixBase = GetGWorldPixMap( gOffWorld);
    clipRect.left = CLIP_LEFT;
    clipRect.right = CLIP_RIGHT;
    clipRect.top = CLIP_TOP;
    clipRect.bottom = CLIP_BOTTOM;

    if (( gScrollStarObject != nil) && ( gScrollStarObject->active) && ( gScrollStarObject->sprite != nil))
    {
        mGetRotPoint( fa, fb, gScrollStarObject->direction);

        fc = mLongToFixed( -kSiteDistance);
        fa = mMultiplyFixed( fc, fa);
        fb = mMultiplyFixed( fc, fb);

        sx = mFixedToLong( fa);
        sy = mFixedToLong( fb);
        sx += gScrollStarObject->sprite->where.h;
        sy += gScrollStarObject->sprite->where.v;

        count = gScrollStarObject->direction;
        mAddAngle( count, 30);
        mGetRotPoint( fa, fb, count);
        fc = mLongToFixed( kSiteSize);
        fa = mMultiplyFixed( fc, fa);
        fb = mMultiplyFixed( fc, fb);

        sa = mFixedToLong( fa);
        sb = mFixedToLong( fb);

        mGetTranslateColorShade( PALE_GREEN, MEDIUM, color, transColor);
        if ( doDraw)
        {
            DrawNateLine( *offPixBase, &clipRect, (sx + sa),
                    (sy + sb), sx, sy, 0,
                    0, color);
        }

        count = gScrollStarObject->direction;
        mAddAngle( count, -30);
        mGetRotPoint( fa, fb, count);
        fc = mLongToFixed( kSiteSize);
        fa = mMultiplyFixed( fc, fa);
        fb = mMultiplyFixed( fc, fb);

        sc = mFixedToLong( fa);
        sd = mFixedToLong( fb);

        if ( doDraw)
        {
            DrawNateLine( *offPixBase, &clipRect, (sx + sc),
                    (sy + sd), sx, sy, 0,
                    0, color);
            mGetTranslateColorShade( PALE_GREEN, DARKER+kSlightlyDarkerColor, color, transColor);
            DrawNateLine( *offPixBase, &clipRect, (sx + sc),
                    (sy + sd), (sx + sa), (sy + sb), 0,
                    0, color);
        }

        l = reinterpret_cast<long*>(*gAresGlobal->gSectorLineData) + kSiteCoordOffset;
        *(l++) = sx;
        *(l++) = sy;
        *(l++) = (sx + sa);
        *(l++) = (sy + sb);
        *(l++) = (sx + sc);
        *(l++) = (sy + sd);
    }

    // do the cursor, too
    GetMouse( &cursorCoord);
    MoveSpriteCursor( cursorCoord);
    HideSpriteCursor( true);
    if ( cursorCoord.h < (CLIP_LEFT /*+ kCursorBoundsSize*/))
    {
//      cursorCoord.h = (CLIP_LEFT + kCursorBoundsSize);
        ShowSpriteCursor( true);
    } else if ( cursorCoord.h > ( CLIP_RIGHT - kCursorBoundsSize - 1))
    {
        cursorCoord.h = CLIP_RIGHT - kCursorBoundsSize - 1;
//      ShowSpriteCursor( true);
    }
    if ( cursorCoord.v < ( CLIP_TOP + kCursorBoundsSize))
    {
        cursorCoord.v = CLIP_TOP + kCursorBoundsSize;
//      ShowSpriteCursor( true);
    } else if ( cursorCoord.v > ( /*CLIP_BOTTOM*/gAresGlobal->gTrueClipBottom - kCursorBoundsSize - 1))
    {
        cursorCoord.v = /*CLIP_BOTTOM*/gAresGlobal->gTrueClipBottom - kCursorBoundsSize - 1;
//      ShowSpriteCursor( true);
    }

    l = reinterpret_cast<long*>(*gAresGlobal->gSectorLineData) + kCursorCoordOffset;
    sx = *(l++) = cursorCoord.h;
    sy = *(l++) = cursorCoord.v;

    sa = *(l++);
    sb = *(l++);

    if ((( sa != sx) || ( sb != sy)) && ( !SpriteCursorVisible())) gAresGlobal->gMouseActive = kMouseActive;
    else if ( gAresGlobal->gMouseActive > kMouseSleepTime) gAresGlobal->gMouseActive = kMouseTurningOff;

    if ( gAresGlobal->gMouseActive > kMouseTurningOff)
    {
        mGetTranslateColorShade( SKY_BLUE, MEDIUM, color, transColor);
        DrawNateLine( *offPixBase, &clipRect, sx,
                    clipRect.top, sx, (sy - kCursorBoundsSize), 0,
                    0, color);
        DrawNateLine( *offPixBase, &clipRect, sx,
                    (sy + kCursorBoundsSize), sx, clipRect.bottom - 1, 0,
                    0, color);
        DrawNateLine( *offPixBase, &clipRect, clipRect.left,
                    sy, (sx - kCursorBoundsSize), sy, 0,
                    0, color);
        DrawNateLine( *offPixBase, &clipRect, (sx + kCursorBoundsSize),
                    sy, clipRect.right - 1, sy, 0,
                    0, color);
    }
}

void DrawSectorLines( void)

{
    long            *l, dashon, dashoff, dashcount;
    unsigned long   size, level, x, h, division;
    PixMapHandle    offPixBase;
    longRect        clipRect;
    unsigned char   color;
    transColorType  *transColor;
    Boolean         doDraw;

    if ( gScrollStarObject != nil)
    {
        if ( gScrollStarObject->offlineTime <= 0) doDraw = true;
        else
        {
            if ( Randomize( gScrollStarObject->offlineTime) < 5) doDraw = true;
            else doDraw = false;
        }
    } else doDraw = false;

    offPixBase = GetGWorldPixMap( gOffWorld);
    clipRect.left = CLIP_LEFT;
    clipRect.right = CLIP_RIGHT;
    clipRect.top = CLIP_TOP;
    clipRect.bottom = CLIP_BOTTOM;

    size = kSubSectorSize >> 2L;
    level = 1;
    do
    {
        level <<= 1L;
        size <<= 2L;
        h = size;
        h *= gAbsoluteScale;
        h >>= SHIFT_SCALE;
    } while ( h < kMinGraphicSectorSize);
    level >>= 1L;
    level *= level;

    x = gGlobalCorner.h;
    x &= size - 1;
    x = size - x;

    division = gGlobalCorner.h + x;
    division >>= kSubSectorShift;
    division &= 0x0000000f;

    x *= gAbsoluteScale;
    x >>= SHIFT_SCALE;
    x += CLIP_LEFT;

    l = reinterpret_cast<long*>(*gAresGlobal->gSectorLineData);
    if ( doDraw)
    {
        while (( x < CLIP_RIGHT) && ( h > 0))
        {
            if ( !division)
            {
                mGetTranslateColorShade( GREEN, kSectorLineBrightness, color, transColor);
                dashon = 56;
                dashoff = 8;
            } else if ( !(division & 0x3))
            {
                mGetTranslateColorShade( SKY_BLUE, kSectorLineBrightness, color, transColor);
                dashon = 32;
                dashoff = 32;
            } else
            {
                mGetTranslateColorShade( BLUE, kSectorLineBrightness, color, transColor);
                dashon = 8;
                dashoff = 56;
            }

            dashon = (( dashon * gAbsoluteScale) >> SHIFT_SCALE);
            dashoff = (( dashoff * gAbsoluteScale) >> SHIFT_SCALE);
            dashon = dashoff = 0;
            if (( dashon > 0) && ( dashoff > 0))
            {
//              dashcount = ((gGlobalCorner.v * gAbsoluteScale) >> SHIFT_SCALE) % 128;
                dashcount = gGlobalCorner.v % ( 64);
                dashcount = (( dashcount * gAbsoluteScale) >> SHIFT_SCALE);
                DashNateLine( *offPixBase, &clipRect, x, CLIP_TOP, x, CLIP_BOTTOM, 0,
                    0, color, dashon, dashoff, dashcount);
            } else
            {
                DrawNateLine( *offPixBase, &clipRect, x, CLIP_TOP, x, CLIP_BOTTOM, 0,
                    0, color);
            }
            *l = x;
            l += 2;
            division += level;
            division &= 0x0000000f;
            x += h;
        }
    } else
    {
        while (( x < CLIP_RIGHT) && ( h > 0))
        {
            if ( !division)
            {
                mGetTranslateColorShade( RED, kSectorLineBrightness, color, transColor);
            } else if ( !(division & 0x3))
            {
                mGetTranslateColorShade( YELLOW, kSectorLineBrightness, color, transColor);
            } else
            {
                mGetTranslateColorShade( BLUE, kSectorLineBrightness, color, transColor);
            }

            *l = x;
            l += 2;
            division += level;
            division &= 0x0000000f;
            x += h;
        }
    }

    x = gGlobalCorner.v;
    x &= size - 1;
    x = size - x;

    division = gGlobalCorner.v + x;
    division >>= kSubSectorShift;
    division &= 0x0000000f;

    x *= gAbsoluteScale;
    x >>= SHIFT_SCALE;
    x += CLIP_TOP;

    l = reinterpret_cast<long*>(*gAresGlobal->gSectorLineData) + (kMaxSectorLine << 1L);

    if ( doDraw)
    {
        while (( x < CLIP_BOTTOM) && ( h > 0))
        {
            if ( !division)
            {
                mGetTranslateColorShade( GREEN, kSectorLineBrightness, color, transColor);
                dashon = 56;
                dashoff = 8;
            } else if ( !(division & 0x3))
            {
                mGetTranslateColorShade( SKY_BLUE, kSectorLineBrightness, color, transColor);
                dashon = 32;
                dashoff = 32;
            } else
            {
                mGetTranslateColorShade( BLUE, kSectorLineBrightness, color, transColor);
                dashon = 8;
                dashoff = 56;
            }

            dashon = (( dashon * gAbsoluteScale) >> SHIFT_SCALE);
            dashoff = (( dashoff * gAbsoluteScale) >> SHIFT_SCALE);
            dashon = dashoff = 0;
            if (( dashon > 0) && ( dashoff > 0))
            {
                dashcount = gGlobalCorner.v % ( 64);
                dashcount = (( dashcount * gAbsoluteScale) >> SHIFT_SCALE);
                DashNateLine( *offPixBase, &clipRect, CLIP_LEFT, x, CLIP_RIGHT, x, 0,
                    0, color, dashon, dashoff, dashcount);
            } else
            {
                DrawNateLine( *offPixBase, &clipRect, CLIP_LEFT, x, CLIP_RIGHT, x, 0,
                    0, color);
            }
            *l = x;
            l += 2;

            division += level;
            division &= 0x0000000f;

            x += h;
        }
    } else
    {
        while (( x < CLIP_BOTTOM) && ( h > 0))
        {
            if ( !division)
            {
                mGetTranslateColorShade( RED, kSectorLineBrightness, color, transColor);
            } else if ( !(division & 0x3))
            {
                mGetTranslateColorShade( YELLOW, kSectorLineBrightness, color, transColor);
            } else
            {
                mGetTranslateColorShade( BLUE, kSectorLineBrightness, color, transColor);
            }

            *l = x;
            l += 2;

            division += level;
            division &= 0x0000000f;

            x += h;
        }
    }

    if ((( gAresGlobal->gLastScale < kBlipThreshhold) && ( gAbsoluteScale >= kBlipThreshhold)) ||
        (( gAresGlobal->gLastScale >= kBlipThreshhold) && ( gAbsoluteScale < kBlipThreshhold)))
        PlayVolumeSound(  kComputerBeep4, kMediumVolume, kMediumPersistence, kLowPrioritySound);

    gAresGlobal->gLastScale = gAbsoluteScale;
    gLastGlobalCorner = gGlobalCorner;
}


void ShowSite( void)

{
    short           sx, sy, sa, sb, sc, sd;
    PixMapHandle    offPixBase;
    longRect        clipRect;
    long            *l;

    offPixBase = GetGWorldPixMap( gOffWorld);
    clipRect.left = CLIP_LEFT;
    clipRect.right = CLIP_RIGHT;
    clipRect.top = CLIP_TOP;
    clipRect.bottom = CLIP_BOTTOM;

    l = reinterpret_cast<long*>(*gAresGlobal->gSectorLineData) + kSiteCoordOffset;
    sx = *(l++);
    sy = *(l++);
    sa = *(l++);
    sb = *(l++);
    sc = *(l++);
    sd = *(l++);

    CopyNateLine( *offPixBase, *thePixMapHandle, &clipRect, sa,
                sb, sx, sy,
                gNatePortLeft << 2L, gNatePortTop);
    CopyNateLine( *offPixBase, *thePixMapHandle, &clipRect, sc,
                sd, sx, sy,
                gNatePortLeft << 2L, gNatePortTop);
    CopyNateLine( *offPixBase, *thePixMapHandle, &clipRect, sc,
                sd, sa, sb,
                gNatePortLeft << 2L, gNatePortTop);

    sx = *(l++);
    sy = *(l++);
    sa = *(l++);
    sb = *(l++);
    sc = *(l++);
    sd = *(l++);
    CopyNateLine( *offPixBase, *thePixMapHandle, &clipRect, sa,
                sb, sx, sy,
                gNatePortLeft << 2L, gNatePortTop);
    CopyNateLine( *offPixBase, *thePixMapHandle, &clipRect, sc,
                sd, sx, sy,
                gNatePortLeft << 2L, gNatePortTop);
    CopyNateLine( *offPixBase, *thePixMapHandle, &clipRect, sc,
                sd, sa, sb,
                gNatePortLeft << 2L, gNatePortTop);

    l = reinterpret_cast<long*>(*gAresGlobal->gSectorLineData) + kCursorCoordOffset;
    sx = *(l++);
    sy = *(l++);

    if ( gAresGlobal->gMouseActive > kMouseTurningOff)
    {
        CopyNateLine( *offPixBase, *thePixMapHandle, &clipRect, sx,
                    clipRect.top, sx, clipRect.bottom - 1,
                    gNatePortLeft << 2L, gNatePortTop);
        CopyNateLine( *offPixBase, *thePixMapHandle, &clipRect, clipRect.left,
                    sy, clipRect.right - 1, sy,
                    gNatePortLeft << 2L, gNatePortTop);
    }

    sx = *(l++);
    sy = *(l++);

    if ( gAresGlobal->gMouseActive)
    {
        CopyNateLine( *offPixBase, *thePixMapHandle, &clipRect, sx,
                    clipRect.top, sx, (sy - kCursorBoundsSize),
                    gNatePortLeft << 2L, gNatePortTop);
        CopyNateLine( *offPixBase, *thePixMapHandle, &clipRect, sx,
                    (sy + kCursorBoundsSize), sx, clipRect.bottom - 1,
                    gNatePortLeft << 2L, gNatePortTop);
        CopyNateLine( *offPixBase, *thePixMapHandle, &clipRect, clipRect.left,
                    sy, (sx - kCursorBoundsSize), sy,
                    gNatePortLeft << 2L, gNatePortTop);
        CopyNateLine( *offPixBase, *thePixMapHandle, &clipRect, (sx + kCursorBoundsSize),
                    sy, clipRect.right - 1, sy,
                    gNatePortLeft << 2L, gNatePortTop);
        if ( gAresGlobal->gMouseActive == kMouseTurningOff) gAresGlobal->gMouseActive = kMouseOff;
    }
}

void ShowSectorLines( void)

{
    long            *l, count;
    PixMapHandle    offPixBase;
    longRect        clipRect;

    offPixBase = GetGWorldPixMap( gOffWorld);
    clipRect.left = CLIP_LEFT;
    clipRect.right = CLIP_RIGHT;
    clipRect.top = CLIP_TOP;
    clipRect.bottom = CLIP_BOTTOM;

    l = reinterpret_cast<long*>(*gAresGlobal->gSectorLineData);
    count = 0;

    while ( count < kMaxSectorLine)
    {
        if ( *l != -1)
            CopyNateLine( *offPixBase, *thePixMapHandle, &clipRect, *l, CLIP_TOP, *l, CLIP_BOTTOM,
                gNatePortLeft << 2L, gNatePortTop);
        l++;
        if ( *l != -1)
            CopyNateLine( *offPixBase, *thePixMapHandle, &clipRect, *l, CLIP_TOP, *l, CLIP_BOTTOM,
                gNatePortLeft << 2L, gNatePortTop);
        l++;
        count++;
    }

    l = reinterpret_cast<long*>(*gAresGlobal->gSectorLineData) + (kMaxSectorLine << 1L);
    count = 0;

    while ( count < kMaxSectorLine)
    {
        if ( *l != -1)
            CopyNateLine( *offPixBase, *thePixMapHandle, &clipRect, CLIP_LEFT, *l, CLIP_RIGHT, *l,
                gNatePortLeft << 2L, gNatePortTop);
        l++;
        if ( *l != -1)
            CopyNateLine( *offPixBase, *thePixMapHandle, &clipRect, CLIP_LEFT, *l, CLIP_RIGHT, *l,
                gNatePortLeft << 2L, gNatePortTop);
        l++;
        count++;
    }


}

void InstrumentsHandleClick( void)

{
    Point   where;
    long    *l;

    l = reinterpret_cast<long*>(*gAresGlobal->gSectorLineData) + kCursorCoordOffset;
    where.h = *(l++);
    where.v = *(l++);

    PlayerShipHandleClick( where);
    MiniComputerHandleClick( where);
    if (!SpriteCursorVisible()) gAresGlobal->gMouseActive = kMouseActive;
}

void InstrumentsHandleDoubleClick( void)

{
    Point   where;
    long    *l;

    l = reinterpret_cast<long*>(*gAresGlobal->gSectorLineData) + kCursorCoordOffset;
    where.h = *(l++);
    where.v = *(l++);

    PlayerShipHandleClick( where);
    MiniComputerHandleDoubleClick( where);
    if (!SpriteCursorVisible()) gAresGlobal->gMouseActive = kMouseActive;
}

void InstrumentsHandleMouseUp( void)

{
    Point   where;
    long    *l;

    l = reinterpret_cast<long*>(*gAresGlobal->gSectorLineData) + kCursorCoordOffset;
    where.h = *(l++);
    where.v = *(l++);

    MiniComputerHandleMouseUp( where);
}

void InstrumentsHandleMouseStillDown( void)

{
    Point   where;
    long    *l;

    l = reinterpret_cast<long*>(*gAresGlobal->gSectorLineData) + kCursorCoordOffset;
    where.h = *(l++);
    where.v = *(l++);

    MiniComputerHandleMouseStillDown( where);
}

void DrawArbitrarySectorLines( coordPointType *corner, long scale, long minSectorSize, Rect *bounds,
                                PixMapHandle pixBase, long portLeft, long portTop)

{
    unsigned long   size, level, x, h, division;
    longRect        clipRect;
    unsigned char   color;
    transColorType  *transColor;

    clipRect.left = bounds->left;
    clipRect.right = bounds->right;
    clipRect.top = bounds->top;
    clipRect.bottom = bounds->bottom;

    size = kSubSectorSize >> 2L;
    level = 1;
    do
    {
        level <<= 1L;
        size <<= 2L;
        h = size;
        h *= scale;
        h >>= SHIFT_SCALE;
    } while ( h < minSectorSize);
    level >>= 1L;
    level *= level;

    x = corner->h;
    x &= size - 1;
    x = size - x;

    division = corner->h + x;
    division >>= kSubSectorShift;
    division &= 0x0000000f;

    x *= scale;
    x >>= SHIFT_SCALE;
    x += bounds->left;

    while (( x < bounds->right) && ( h > 0))
    {
        if ( !division)
        {
            mGetTranslateColorShade( GREEN, DARKER, color, transColor);
        } else if ( !(division & 0x3))
        {
            mGetTranslateColorShade( SKY_BLUE, DARKER, color, transColor);
        } else
        {
            mGetTranslateColorShade( BLUE, DARKER, color, transColor);
        }

        DrawNateLine( *pixBase, &clipRect, x, bounds->top, x, bounds->bottom,
                portLeft << 2, portTop, color);
        division += level;
        division &= 0x0000000f;
        x += h;
    }

    x = corner->v;
    x &= size - 1;
    x = size - x;

    division = corner->v + x;
    division >>= kSubSectorShift;
    division &= 0x0000000f;

    x *= scale;
    x >>= SHIFT_SCALE;
    x += bounds->top;

    while (( x < bounds->bottom) && ( h > 0))
    {
        if ( !division)
        {
            mGetTranslateColorShade( GREEN, DARKER, color, transColor);
        } else if ( !(division & 0x3))
        {
            mGetTranslateColorShade( SKY_BLUE, DARKER, color, transColor);
        } else
        {
            mGetTranslateColorShade( BLUE, DARKER, color, transColor);
        }

        DrawNateLine( *pixBase, &clipRect, bounds->left, x, bounds->right, x,
                portLeft << 2, portTop, color);

        division += level;
        division &= 0x0000000f;

        x += h;
    }
}

void GetArbitrarySingleSectorBounds( coordPointType *corner, coordPointType *location,
                                long scale, long minSectorSize, Rect *bounds, Rect *destRect)

{
    unsigned long   size, level, x, h, division, scaledLoc;
    longRect        clipRect;

    clipRect.left = bounds->left;
    clipRect.right = bounds->right;
    clipRect.top = bounds->top;
    clipRect.bottom = bounds->bottom;

    destRect->left = bounds->left;
    destRect->right = bounds->right;
    destRect->top = bounds->top;
    destRect->bottom = bounds->bottom;

    size = kSubSectorSize >> 2L;
    level = 1;
    do
    {
        level <<= 1L;
        size <<= 2L;
        h = size;
        h *= scale;
        h >>= SHIFT_SCALE;
    } while ( h < minSectorSize);
    level >>= 1L;
    level *= level;

    x = corner->h;
    x &= size - 1;
    x = size - x;

    division = corner->h + x;
    division >>= kSubSectorShift;
    division &= 0x0000000f;

    x *= scale;
    x >>= SHIFT_SCALE;
    x += bounds->left;

    scaledLoc = location->h - corner->h;
    scaledLoc *= scale;
    scaledLoc >>= SHIFT_SCALE;
    scaledLoc += bounds->left;

    while (( x < bounds->right) && ( h > 0))
    {
        division += level;
        division &= 0x0000000f;

        if (( x < scaledLoc) && ( x > destRect->left)) destRect->left = x;
        if (( x > scaledLoc) && ( x < destRect->right)) destRect->right = x;

        x += h;
    }

    x = corner->v;
    x &= size - 1;
    x = size - x;

    division = corner->v + x;
    division >>= kSubSectorShift;
    division &= 0x0000000f;

    x *= scale;
    x >>= SHIFT_SCALE;
    x += bounds->top;

    scaledLoc = location->v - corner->v;
    scaledLoc *= scale;
    scaledLoc >>= SHIFT_SCALE;
    scaledLoc += bounds->top;

    while (( x < bounds->bottom) && ( h > 0))
    {
        division += level;
        division &= 0x0000000f;

        if (( x < scaledLoc) && ( x > destRect->top)) destRect->top = x;
        if (( x > scaledLoc) && ( x < destRect->bottom)) destRect->bottom = x;

        x += h;
    }
}


void UpdateBarIndicator( short which, long value, long max, PixMapHandle pixMap)

{
    long            graphicValue;
    longRect        tRect, clipRect;
    transColorType  *transColor;
    unsigned char   color, lightColor, darkColor;
    Rect            rrect;

    if ( value > max) value = max;

    if ( value != gAresGlobal->gBarIndicator[ which].thisValue)
    {
        gAresGlobal->gBarIndicator[ which].lastValue = gAresGlobal->gBarIndicator[ which].thisValue;
        if ( max > 0)
        {
            graphicValue = kBarIndicatorHeight * value;
            graphicValue /= max;
            if ( graphicValue < 0) graphicValue = 0;
            if ( graphicValue > kBarIndicatorHeight) graphicValue = kBarIndicatorHeight;
        } else graphicValue = 0;


        tRect.left = kBarIndicatorLeft + gAresGlobal->gRightPanelLeftEdge;
        tRect.right =  tRect.left + kBarIndicatorWidth;
        tRect.top = gAresGlobal->gBarIndicator[ which].top,
        tRect.bottom = tRect.top + kBarIndicatorHeight - graphicValue;
        clipRect.left = gAresGlobal->gRightPanelLeftEdge;
        clipRect.top = gAresGlobal->gInstrumentTop;
        clipRect.right = clipRect.left + kRightPanelWidth;
        clipRect.bottom = clipRect.top + kPanelHeight;

        mGetTranslateColorShade( gAresGlobal->gBarIndicator[which].color, DARK, color, transColor);
        mGetTranslateColorShade( gAresGlobal->gBarIndicator[which].color, MEDIUM, lightColor, transColor);
        mGetTranslateColorShade( gAresGlobal->gBarIndicator[which].color, DARKER, darkColor, transColor);
//      DrawNateRect( *pixMap, &tRect, gNatePortLeft << 2, gNatePortTop, color);
        DrawNateShadedRect( *pixMap, &tRect, &clipRect, gNatePortLeft << 2L, gNatePortTop, color, lightColor, darkColor);

        if ( graphicValue > 0)
        {
            tRect.top = tRect.bottom;//tRect.bottom - graphicValue;
            tRect.bottom = gAresGlobal->gBarIndicator[which].top + kBarIndicatorHeight;
            mGetTranslateColorShade( gAresGlobal->gBarIndicator[which].color, LIGHTER, color, transColor);
            mGetTranslateColorShade( gAresGlobal->gBarIndicator[which].color, VERY_LIGHT, lightColor, transColor);
            mGetTranslateColorShade( gAresGlobal->gBarIndicator[which].color, MEDIUM, darkColor, transColor);
    //      DrawNateRect( *pixMap, &tRect, gNatePortLeft << 2, gNatePortTop, color);
            DrawNateShadedRect( *pixMap, &tRect, &clipRect, gNatePortLeft << 2L, gNatePortTop, color, lightColor, darkColor);
        }
        SetRect( &rrect, tRect.left, gAresGlobal->gBarIndicator[ which].top,
            tRect.right, gAresGlobal->gBarIndicator[ which].top + kBarIndicatorHeight);
        CopyRealWorldToOffWorld( gTheWindow, &rrect);

        gAresGlobal->gBarIndicator[ which].thisValue = value;
    }
}

void DrawBuildTimeBar( long value)

{
    longRect        tRect, clipRect;
    transColorType  *transColor;
    unsigned char   color;

    if ( value < 0) value = 0;
    value = kMiniBuildTimeHeight - value;

    tRect.left = clipRect.left = kMiniBuildTimeLeft;
    tRect.top = clipRect.top = kMiniBuildTimeTop + gAresGlobal->gInstrumentTop;
    tRect.bottom = clipRect.bottom = kMiniBuildTimeBottom + gAresGlobal->gInstrumentTop;
    tRect.right = clipRect.right = kMiniBuildTimeRight;

    mGetTranslateColorShade( PALE_PURPLE, MEDIUM, color, transColor);
    DrawNateVBracket( *thePixMapHandle, &tRect, &clipRect, gNatePortLeft << 2L, gNatePortTop,
                     color);

    tRect.left = kMiniBuildTimeLeft + 2;
    tRect.top = kMiniBuildTimeTop + 2 + value + gAresGlobal->gInstrumentTop;
    tRect.bottom = kMiniBuildTimeBottom - 2 + gAresGlobal->gInstrumentTop;
    tRect.right = kMiniBuildTimeRight - 2;

    if ( value > 0)
    {
        mGetTranslateColorShade( PALE_PURPLE, LIGHT, color, transColor);

        DrawNateRect( *thePixMapHandle, &tRect, gNatePortLeft << 2L, gNatePortTop, color);

        tRect.top = kMiniBuildTimeTop + 2 + gAresGlobal->gInstrumentTop;
    } else value = 0;

    tRect.bottom = kMiniBuildTimeTop + 2 + value + gAresGlobal->gInstrumentTop;

    mGetTranslateColorShade( PALE_PURPLE, DARK, color, transColor);

    DrawNateRect( *thePixMapHandle, &tRect, gNatePortLeft << 2L, gNatePortTop, color);
}

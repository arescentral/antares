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

#include "Admiral.hpp"
#include "AresGlobalType.hpp"
#include "ColorTranslation.hpp"
#include "Debug.hpp"
#include "Error.hpp"
#include "MathMacros.hpp"
#include "Minicomputer.hpp"
#include "NateDraw.hpp"
#include "OffscreenGWorld.hpp"
#include "Picture.hpp"
#include "PlayerShip.hpp"
#include "Rotation.hpp"
#include "ScreenLabel.hpp"
//#include "SoundFX.hpp"
#include "SpaceObject.hpp"
#include "SpriteCursor.hpp"
#include "UniverseUnit.hpp"

namespace antares {

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

#define kMiniBuildTimeLeft      ( globals()->gRightPanelLeftEdge + 10)
#define kMiniBuildTimeRight     ( globals()->gRightPanelLeftEdge + 22)

#define kMiniBuildTimeTop       8
#define kMiniBuildTimeBottom    37

#define kMinimumAutoScale   2 //128 //kOneQuarterScale
#define kBlipThreshhold     kOneQuarterScale            // should be the same as kSpriteBlipThreshhold
#define kSuperSmallScale    2

#define kMouseSleepTime     60

#define kSectorLineBrightness   DARKER

extern scoped_array<spaceObjectType> gSpaceObjectData;
extern spaceObjectType  *gScrollStarObject;
extern int32_t          gNatePortLeft, gNatePortTop, gAbsoluteScale,
                        WORLD_WIDTH, WORLD_HEIGHT, CLIP_LEFT, CLIP_TOP, CLIP_RIGHT, CLIP_BOTTOM;
extern coordPointType   gGlobalCorner;

extern PixMap*          gActiveWorld;
extern PixMap*          gOffWorld;

coordPointType          gLastGlobalCorner;

int InstrumentInit() {
    globals()->gInstrumentTop = (WORLD_HEIGHT / 2) - ( kPanelHeight / 2);
    globals()->gRightPanelLeftEdge = WORLD_WIDTH - kRightPanelWidth;

    globals()->gRadarBlipData.reset(new Point[kRadarBlipNum]);
    globals()->gScaleList.reset(new int32_t[kScaleListNum]);
    globals()->gSectorLineData.reset(new int32_t[kMaxSectorLine * 4 + kSiteCoordNum * 2 + kCursorCoordNum * 2]);
    ResetInstruments();

    return MiniScreenInit();
}

void InstrumentCleanup() {
    globals()->gRadarBlipData.reset();
    MiniScreenCleanup();
}

void ResetInstruments() {
    int32_t         *l, i;
    Point           *lp;

    globals()->gRadarCount = 0;
    globals()->gRadarSpeed = 30;
    globals()->gRadarRange = kRadarSize * 50;
    globals()->gLastScale = gAbsoluteScale = SCALE_SCALE;
    globals()->gWhichScaleNum = 0;
    gLastGlobalCorner.h = gLastGlobalCorner.v = 0;
    globals()->gMouseActive = kMouseOff;
    l = globals()->gScaleList.get();
    for (i = 0; i < kScaleListNum; i++) {
        *l = SCALE_SCALE;
        l++;
    }

    for ( i = 0; i < kBarIndicatorNum; i++)
    {
        globals()->gBarIndicator[i].lastValue = globals()->gBarIndicator[i].thisValue = -1;
    }
    // the shield bar
    globals()->gBarIndicator[kShieldBar].top = 359 + globals()->gInstrumentTop;
    globals()->gBarIndicator[kShieldBar].color = SKY_BLUE;

    globals()->gBarIndicator[kEnergyBar].top = 231 + globals()->gInstrumentTop;
    globals()->gBarIndicator[kEnergyBar].color = GOLD;

    globals()->gBarIndicator[kBatteryBar].top = 103 + globals()->gInstrumentTop;
    globals()->gBarIndicator[kBatteryBar].color = SALMON;

    lp = globals()->gRadarBlipData.get();
    for ( i = 0; i < kRadarBlipNum; i++)
    {
        lp->h = -1;
        lp++;
    }
    ResetSectorLines();
}

void ResetSectorLines() {
    int32_t *l, count;

    l = globals()->gSectorLineData.get();

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

void UpdateRadar(int32_t unitsDone) {
    Rect        lRect;
    spaceObjectType *anObject;
    baseObjectType  *baseObject;
    long            oCount;
    int32_t         dx, rcount, x, y, *scaleval;
    const int32_t   rrange = globals()->gRadarRange >> 1L;
    Point           *lp;
    unsigned char   color, color2;
    uint32_t        bestScale = MIN_SCALE, rootCorrect, distance, difference, dcalc;
    transColorType  *transColor;
    admiralType     *admiral;
    Rect            tRect;
    bool         doDraw;
    uint64_t        tempWide, hugeDistance;

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
    globals()->gRadarCount -= unitsDone;
    if ( globals()->gMouseActive > kMouseTurningOff)
        globals()->gMouseActive += unitsDone;

    if ( globals()->gRadarCount <= 0)
    {
        mGetTranslateColorShade( kRadarColor, VERY_DARK, color, transColor);
    } else
    {
        mGetTranslateColorShade( kRadarColor, (( kRadarColorSteps * globals()->gRadarCount) / globals()->gRadarSpeed) + 1, color, transColor);
    }

    if ( doDraw)
    {
        lp = globals()->gRadarBlipData.get();
        for ( rcount = 0; rcount < kRadarBlipNum; rcount++)
        {
            if ( lp->h >= 0)
            {
                gActiveWorld->set(lp->h, lp->v, color);
            }
            lp++;
        }
    }

    if (( gScrollStarObject != nil) && ( gScrollStarObject->active) )
    {
        if (( globals()->gRadarCount <= 0) && ( doDraw))
        {
            DrawInOffWorld();

            tRect = Rect(kRadarLeft + 1, kRadarTop + 1 + globals()->gInstrumentTop, kRadarRight - 1,
                    kRadarBottom - 1 + globals()->gInstrumentTop);
            lRect = tRect;
            mGetTranslateColorShade( kRadarColor, DARKEST, color, transColor);
            DrawNateRect(gOffWorld, &lRect, 0, 0, color);
            SetTranslateColorShadeFore( kRadarColor, VERY_LIGHT);
            MacFrameRect(tRect);

            dx = gScrollStarObject->location.h - gGlobalCorner.h;
            dx = dx * kRadarSize;
            dx /= globals()->gRadarRange;

            lRect.left = kRadarLeft + kRadarCenter - dx;
            if ( lRect.left <= tRect.left) lRect.left = tRect.left + 1;
            lRect.right = kRadarLeft + kRadarCenter + dx;
            if ( lRect.right >= tRect.right) lRect.right = tRect.right - 1;
            lRect.top = kRadarTop + kRadarCenter - dx + globals()->gInstrumentTop;
            if ( lRect.top <= tRect.top) lRect.top = tRect.top + 1;
            lRect.bottom = kRadarTop + kRadarCenter + dx + globals()->gInstrumentTop;
            if ( lRect.bottom >= tRect.bottom) lRect.bottom = tRect.bottom - 1;
            mGetTranslateColorShade( kRadarColor, VERY_DARK, color, transColor);
            DrawNateRect(gOffWorld, &lRect, 0, 0, color);
            NormalizeColors();
            DrawInRealWorld();
            ChunkCopyPixMapToScreenPixMap(gOffWorld, tRect, gActiveWorld);

            lp = globals()->gRadarBlipData.get();
            for ( rcount = 0; rcount < kRadarBlipNum; rcount++)
            {
                lp->h = -1;
                lp++;
            }
            rcount = 0;

            lp = globals()->gRadarBlipData.get();
            globals()->gRadarCount = globals()->gRadarSpeed;
            anObject = gSpaceObjectData.get();

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
                                x = ( x * kRadarSize) / globals()->gRadarRange + kRadarCenter + kRadarLeft;
                                y = ( y * kRadarSize) / globals()->gRadarRange + kRadarCenter + kRadarTop
                                        + globals()->gInstrumentTop;
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
            tRect = Rect(kRadarLeft + 1, kRadarTop + 1 + globals()->gInstrumentTop, kRadarRight - 1,
                    kRadarBottom - 1 + globals()->gInstrumentTop);
            lRect = tRect;
            mGetTranslateColorShade( kRadarColor, DARKEST, color, transColor);
            DrawNateRect( gActiveWorld, &lRect, gNatePortLeft << 2L, gNatePortTop, color);
        }

        switch ( globals()->gZoomMode)
        {
            case kNearestFoeZoom:
            case kNearestAnythingZoom:
                anObject = gSpaceObjectData.get() + globals()->gClosestObject;
                bestScale = MIN_SCALE;
                /*
                dx = ( (long)anObject->location.h - (long)gScrollStarObject->location.h);
                dy = ( (long)anObject->location.v - (long)gScrollStarObject->location.v);
                if (( ABS(dx) < kMaximumRelevantDistance) && ( ABS( dy) < kMaximumRelevantDistance))
                {
                    bestScale = dx * dx + dy * dy;
                    bestScale = lsqrt( bestScale);
                    if ( bestScale == 0) bestScale = 1;
                    bestScale = globals()->gCenterScaleV / bestScale;
                    if ( bestScale < SCALE_SCALE) bestScale = ( bestScale >> 2L) + ( bestScale >> 1L);
                    if ( bestScale > SCALE_SCALE) bestScale = SCALE_SCALE;
                    if ( bestScale < kMinimumAutoScale) bestScale = kMinimumAutoScale;
                }*/
                hugeDistance = anObject->distanceFromPlayer;
                if (hugeDistance == 0) // if this is true, then we haven't calced its distance
                {
                    difference = ABS<int32_t>(gScrollStarObject->location.h - anObject->location.h);
                    dcalc = difference;
                    difference =  ABS<int32_t>(gScrollStarObject->location.v - anObject->location.v);
                    distance = difference;

                    if (( dcalc > kMaximumRelevantDistance) ||
                        ( distance > kMaximumRelevantDistance))
                    {
                        tempWide = dcalc;    // must be positive
                        MyWideMul( tempWide, tempWide, &hugeDistance);
                        tempWide = distance;
                        MyWideMul( tempWide, tempWide, &tempWide);
                        hugeDistance += tempWide;
                    } else
                    {
                        hugeDistance = distance * distance + dcalc * dcalc;
                    }
                }
                if ((hugeDistance & 0xFFFFFFFF00000000ull) == 0) {
                    bestScale = lsqrt(hugeDistance);
                } else {
                    rootCorrect = 0;
                    do {
                        rootCorrect += 4;
                        hugeDistance >>= 8;
                    } while (hugeDistance & 0xFFFFFFFF00000000ull);
                    bestScale = lsqrt(hugeDistance);
                    bestScale <<= rootCorrect;
                }
                if ( bestScale == 0) bestScale = 1;
                bestScale = globals()->gCenterScaleV / bestScale;
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
                anObject = gSpaceObjectData.get() + globals()->gFarthestObject;
                bestScale = MIN_SCALE;
                /*
                dx = ( (long)anObject->location.h - (long)gScrollStarObject->location.h);
                dy = ( (long)anObject->location.v - (long)gScrollStarObject->location.v);
                if (( ABS(dx) < kMaximumRelevantDistance) && ( ABS( dy) < kMaximumRelevantDistance))
                {
                    bestScale = dx * dx + dy * dy;
                    bestScale = lsqrt( bestScale);
                    if ( bestScale == 0) bestScale = 1;
                    bestScale = globals()->gCenterScaleV / bestScale;
                    if ( bestScale < SCALE_SCALE) bestScale = ( bestScale >> 2L) + ( bestScale >> 1L);
                    if ( bestScale > SCALE_SCALE) bestScale = SCALE_SCALE;
                    if ( bestScale < kMinimumAutoScale) bestScale = kMinimumAutoScale;
                }*/
                tempWide = anObject->distanceFromPlayer;
                if ((tempWide & 0xFFFFFFFF00000000ull) == 0) {
                    bestScale = lsqrt(tempWide);
                } else {
                    rootCorrect = 0;
                    do {
                        rootCorrect += 4;
                        tempWide >>= 8;
                    } while (tempWide & 0xFFFFFFFF00000000ull);
                    bestScale = lsqrt(tempWide);
                    bestScale <<= rootCorrect;
                }
                if ( bestScale == 0) bestScale = 1;
                bestScale = globals()->gCenterScaleV / bestScale;
                if ( bestScale < SCALE_SCALE) bestScale = ( bestScale >> 2L) + ( bestScale >> 1L);
                if ( bestScale > SCALE_SCALE) bestScale = SCALE_SCALE;
                if ( bestScale < kMinimumAutoScale) bestScale = kMinimumAutoScale;
                break;
        }

        for ( x = 0; x < unitsDone; x++)
        {
            scaleval = globals()->gScaleList.get() + globals()->gWhichScaleNum;
            *scaleval = bestScale;
            globals()->gWhichScaleNum++;
            if ( globals()->gWhichScaleNum == kScaleListNum) globals()->gWhichScaleNum = 0;
        }

        scaleval = globals()->gScaleList.get();
        rcount = 0;
        for ( oCount = 0; oCount < kScaleListNum; oCount++)
        {
            rcount += *scaleval++;
        }
        rcount >>= kScaleListShift;

        gAbsoluteScale = rcount;
//      gAbsoluteScale = SCALE_SCALE;

        baseObject = gScrollStarObject->baseType;
        UpdateBarIndicator( kShieldBar, gScrollStarObject->health, baseObject->health, gActiveWorld);
        UpdateBarIndicator( kEnergyBar, gScrollStarObject->energy, baseObject->energy, gActiveWorld);
        UpdateBarIndicator( kBatteryBar, gScrollStarObject->battery, baseObject->energy * 5, gActiveWorld);

        // SHOW ME THE MONEY
        admiral = mGetAdmiralPtr( globals()->gPlayerAdmiralNumber);
        y = x = admiral->cash;
        if ( x < 0) x = 0;
        if ( x >= kMaxMoneyValue) x = kMaxMoneyValue - 1;
        globals()->gBarIndicator[kFineMoneyBar].thisValue = x % kFineMoneyBarMod;
        globals()->gBarIndicator[kFineMoneyBar].thisValue /= kFineMoneyBarValue;
        x = MiniComputerGetPriceOfCurrentSelection();
        x /= kFineMoneyBarValue;

        if (( globals()->gBarIndicator[kFineMoneyBar].thisValue !=
            globals()->gBarIndicator[kFineMoneyBar].lastValue) ||
            ( x != globals()->gLastSelectedBuildPrice))
        {
            lRect.left = kFineMoneyLeft + kFineMoneyHBuffer + globals()->gRightPanelLeftEdge;
            lRect.right = lRect.left + kFineMoneyBarWidth;

            // if selected build price is greater than the money we have
                if ( globals()->gBarIndicator[kFineMoneyBar].thisValue < x)
                {
                    // draw the money we have
                    mGetTranslateColorShade( kFineMoneyColor, VERY_LIGHT, color, transColor);
                    mGetTranslateColorShade( kFineMoneyColor, LIGHT, color2, transColor);

                    for ( rcount = 0;
                        rcount < globals()->gBarIndicator[kFineMoneyBar].thisValue;
                        rcount++)
                    {
                        lRect.top = kFineMoneyTop + globals()->gInstrumentTop +
                                kFineMoneyVBuffer + rcount * kFineMoneyBarHeight;
                        lRect.bottom = lRect.top + kFineMoneyBarHeight - 1;
                        if ( rcount % 5)
                            DrawNateRect( gActiveWorld, &lRect,
                                gNatePortLeft << 2L, gNatePortTop, color2);
                        else
                            DrawNateRect( gActiveWorld, &lRect,
                                gNatePortLeft << 2L, gNatePortTop, color);

                    }

                    // draw the money we need
                    mGetTranslateColorShade( kFineMoneyNeedColor, MEDIUM, color, transColor);
                    mGetTranslateColorShade( kFineMoneyNeedColor, DARK, color2, transColor);

                    for ( rcount = globals()->gBarIndicator[kFineMoneyBar].thisValue;
                        rcount < x;
                        rcount++)
                    {
                        lRect.top = kFineMoneyTop + globals()->gInstrumentTop +
                                kFineMoneyVBuffer + rcount * kFineMoneyBarHeight;
                        lRect.bottom = lRect.top + kFineMoneyBarHeight - 1;
                        if ( rcount % 5)
                            DrawNateRect( gActiveWorld, &lRect,
                                gNatePortLeft << 2L, gNatePortTop, color2);
                        else
                            DrawNateRect( gActiveWorld, &lRect,
                                gNatePortLeft << 2L, gNatePortTop, color);

                    }

                    globals()->gBarIndicator[kFineMoneyBar].thisValue = x;
                } else
                {
                    // draw the money we'll have left
                    mGetTranslateColorShade( kFineMoneyColor, VERY_LIGHT, color, transColor);
                    mGetTranslateColorShade( kFineMoneyColor, LIGHT, color2, transColor);

                    for ( rcount = 0;
                        rcount < ( globals()->gBarIndicator[kFineMoneyBar].thisValue - x);
                        rcount++)
                    {
                        lRect.top = kFineMoneyTop + globals()->gInstrumentTop +
                                kFineMoneyVBuffer + rcount * kFineMoneyBarHeight;
                        lRect.bottom = lRect.top + kFineMoneyBarHeight - 1;
                        if ( rcount % 5)
                            DrawNateRect( gActiveWorld, &lRect,
                                gNatePortLeft << 2L, gNatePortTop, color2);
                        else
                            DrawNateRect( gActiveWorld, &lRect,
                                gNatePortLeft << 2L, gNatePortTop, color);

                    }

                    // draw the money we'll have left
                    mGetTranslateColorShade( kFineMoneyUseColor, VERY_LIGHT, color, transColor);
                    mGetTranslateColorShade( kFineMoneyUseColor, LIGHT, color2, transColor);

                    for ( rcount = (globals()->gBarIndicator[kFineMoneyBar].thisValue -x);
                        rcount < globals()->gBarIndicator[kFineMoneyBar].thisValue;
                        rcount++)
                    {
                        lRect.top = kFineMoneyTop + globals()->gInstrumentTop +
                                kFineMoneyVBuffer + rcount * kFineMoneyBarHeight;
                        lRect.bottom = lRect.top + kFineMoneyBarHeight - 1;
                        if ( rcount % 5)
                            DrawNateRect( gActiveWorld, &lRect,
                                gNatePortLeft << 2L, gNatePortTop, color2);
                        else
                            DrawNateRect( gActiveWorld, &lRect,
                                gNatePortLeft << 2L, gNatePortTop, color);

                    }

                }

                mGetTranslateColorShade( kFineMoneyColor, VERY_DARK, color, transColor);

                for ( rcount = globals()->gBarIndicator[kFineMoneyBar].thisValue;
                    rcount < kFineMoneyBarNum; rcount++)
                {
                    lRect.top = kFineMoneyTop + globals()->gInstrumentTop +
                        kFineMoneyVBuffer + rcount * kFineMoneyBarHeight;
                    lRect.bottom = lRect.top + kFineMoneyBarHeight - 1;
                    DrawNateRect( gActiveWorld, &lRect, gNatePortLeft << 2L,
                        gNatePortTop, color);
                }
        }

        globals()->gLastSelectedBuildPrice = x;

        globals()->gBarIndicator[kGrossMoneyBar].thisValue = y;
        globals()->gBarIndicator[kGrossMoneyBar].thisValue /= kGrossMoneyBarValue;
        if ( globals()->gBarIndicator[kGrossMoneyBar].thisValue != globals()->gBarIndicator[kGrossMoneyBar].lastValue)
        {
            lRect.left = kGrossMoneyLeft + kGrossMoneyHBuffer + globals()->gRightPanelLeftEdge;
            lRect.right = lRect.left + kGrossMoneyBarWidth;
            mGetTranslateColorShade( kGrossMoneyColor, VERY_LIGHT, color, transColor);

            for ( rcount = 0; rcount < globals()->gBarIndicator[kGrossMoneyBar].thisValue; rcount++)
            {
                lRect.top = kGrossMoneyTop + globals()->gInstrumentTop + kGrossMoneyVBuffer + rcount *
                            kGrossMoneyBarHeight;
                lRect.bottom = lRect.top + kGrossMoneyBarHeight - 1;
                DrawNateRect( gActiveWorld, &lRect, gNatePortLeft << 2L, gNatePortTop, color);
            }
            mGetTranslateColorShade( kGrossMoneyColor, VERY_DARK, color, transColor);

            for ( rcount = globals()->gBarIndicator[kGrossMoneyBar].thisValue; rcount < kGrossMoneyBarNum; rcount++)
            {
                lRect.top = kGrossMoneyTop + globals()->gInstrumentTop + kGrossMoneyVBuffer + rcount *
                            kGrossMoneyBarHeight;
                lRect.bottom = lRect.top + kGrossMoneyBarHeight - 1;
                DrawNateRect( gActiveWorld, &lRect, gNatePortLeft << 2L, gNatePortTop, color);
            }
        }
    }
}

void DrawInstrumentPanel() {
    scoped_ptr<Picture> pict;
    Rect            tRect;

//  pict = GetPicture( kBackgroundPictID);
    globals()->gZoomMode = kNearestFoeZoom;
    DrawInSaveWorld();
    tRect = Rect(0, 0, WORLD_WIDTH, WORLD_HEIGHT);
    SetTranslateColorFore( BLACK);
    gActiveWorld->view(tRect).fill(BLACK);
    CopySaveWorldToOffWorld(tRect);
    DrawInRealWorld();

    pict.reset(new Picture(kInstLeftPictID));
    if (pict.get() == nil) {
        ShowErrorAny( eContinueOnlyErr, kErrorStrID, nil, nil, nil, nil, OFFSCREEN_GRAPHICS_ERROR, -1, -1, -1, __FILE__, 2);
    } else {
        DrawInSaveWorld();
        tRect = pict->bounds();
        tRect.left = 0;
        tRect.right = pict->bounds().right - pict->bounds().left;
        tRect.top = (WORLD_HEIGHT / 2) - (pict->bounds().bottom - pict->bounds().top) / 2;
        tRect.bottom = tRect.top + pict->bounds().bottom - pict->bounds().top;
        CopyBits(pict.get(), gActiveWorld, pict->bounds(), tRect);
        CopySaveWorldToOffWorld(tRect);
        DrawInRealWorld();
    }

    pict.reset(new Picture(kInstRightPictID));
    if (pict.get() == nil) {
        ShowErrorAny( eContinueOnlyErr, kErrorStrID, nil, nil, nil, nil, OFFSCREEN_GRAPHICS_ERROR, -1, -1, -1, __FILE__, 3);
    } else {
        DrawInSaveWorld();
        tRect = pict->bounds();
        tRect.left = WORLD_WIDTH - (pict->bounds().right - pict->bounds().left);
        tRect.right = tRect.left + pict->bounds().right - pict->bounds().left;
        tRect.top = (WORLD_HEIGHT / 2) - (pict->bounds().bottom - pict->bounds().top) / 2;
        tRect.bottom = tRect.top + pict->bounds().bottom - pict->bounds().top;
        CopyBits(pict.get(), gActiveWorld, pict->bounds(), tRect);
        CopySaveWorldToOffWorld(tRect);
        DrawInRealWorld();
    }
    pict.reset();

    tRect = Rect(0, 0, WORLD_WIDTH, WORLD_HEIGHT);
    CopyOffWorldToRealWorld(tRect);
    MakeMiniScreenFromIndString( 1);
    DrawMiniScreen();
    ResetInstruments();
    ClearMiniObjectData();
    ShowWholeMiniScreen();
    UpdatePlayerAmmo( -1, -1, -1);
    UpdateRadar(100);

}


void EraseSite() {
    int32_t         *l, *olp;
    int16_t         sx, sy, sa, sb, sc, sd;
    Rect        clipRect;

    clipRect.left = CLIP_LEFT;
    clipRect.right = CLIP_RIGHT;
    clipRect.top = CLIP_TOP;
    clipRect.bottom = CLIP_BOTTOM;

    l = globals()->gSectorLineData.get() + kSiteCoordOffset;
    olp = l + kSiteCoordNum;
    sx = *(olp++) = *(l++);
    sy = *(olp++) = *(l++);
    sa = *(olp++) = *(l++);
    sb = *(olp++) = *(l++);
    sc = *(olp++) = *(l++);
    sd = *(olp++) = *(l++);
    DrawNateLine(gOffWorld, clipRect, sa,
                sb, sx, sy, 0,
                0, BLACK);

    DrawNateLine(gOffWorld, clipRect, sc,
                sd, sx, sy, 0,
                0, BLACK);
    DrawNateLine(gOffWorld, clipRect, sc,
                sd, sa, sb, 0,
                0, BLACK);

    // do the cursor, too

    l = globals()->gSectorLineData.get() + kCursorCoordOffset;
    olp = l + kCursorCoordNum;
    sx = *(olp++) = *(l++);
    sy = *(olp++) = *(l++);

    if ( globals()->gMouseActive)
    {
        DrawNateLine(gOffWorld, clipRect, sx,
                    clipRect.top, sx, (sy - kCursorBoundsSize), 0,
                    0, BLACK);
        DrawNateLine(gOffWorld, clipRect, sx,
                    (sy + kCursorBoundsSize), sx, clipRect.bottom - 1, 0,
                    0, BLACK);
        DrawNateLine(gOffWorld, clipRect, clipRect.left,
                    sy, (sx - kCursorBoundsSize), sy, 0,
                    0, BLACK);
        DrawNateLine(gOffWorld, clipRect, (sx + kCursorBoundsSize),
                    sy, clipRect.right - 1, sy, 0,
                    0, BLACK);
    }
}

void EraseSectorLines() {
    int32_t         *l, count, ol;
    Rect        clipRect;

    clipRect.left = CLIP_LEFT;
    clipRect.right = CLIP_RIGHT;
    clipRect.top = CLIP_TOP;
    clipRect.bottom = CLIP_BOTTOM;

    l = globals()->gSectorLineData.get();
    count = 0;

    while (( *l != -1) && ( count < kMaxSectorLine))
    {
        DrawNateLine(gOffWorld, clipRect, *l, CLIP_TOP, *l, CLIP_BOTTOM, 0, 0, BLACK);
        ol = *l;
        *l = -1;
        l++;
        *l = ol;
        l++;
        count++;
    }

    l = globals()->gSectorLineData.get() + (kMaxSectorLine << 1L);
    count = 0;

    while (( *l != -1) && ( count < kMaxSectorLine))
    {
        DrawNateLine(gOffWorld, clipRect, CLIP_LEFT, *l, CLIP_RIGHT, *l, 0, 0, BLACK);
        ol = *l;
        *l = -1;
        l++;
        *l = ol;
        l++;
        count++;
    }
}

void DrawSite() {
    int32_t         *l, count;
    int16_t         sx, sy, sa, sb, sc, sd;
    smallFixedType  fa, fb, fc;
    transColorType  *transColor;
    unsigned char   color;
    Rect        clipRect;
    Point           cursorCoord;
    bool         doDraw;

    if ( gScrollStarObject != nil)
    {
        if ( gScrollStarObject->offlineTime <= 0) doDraw = true;
        else
        {
            if ( Randomize( gScrollStarObject->offlineTime) < 5) doDraw = true;
            else doDraw = false;
        }
    } else doDraw = false;

    clipRect.left = CLIP_LEFT;
    clipRect.right = CLIP_RIGHT;
    clipRect.top = CLIP_TOP;
    clipRect.bottom = CLIP_BOTTOM;

    if (( gScrollStarObject != nil) && ( gScrollStarObject->active) && ( gScrollStarObject->sprite != nil))
    {
        GetRotPoint(&fa, &fb, gScrollStarObject->direction);

        fc = mLongToFixed( -kSiteDistance);
        fa = mMultiplyFixed( fc, fa);
        fb = mMultiplyFixed( fc, fb);

        sx = mFixedToLong( fa);
        sy = mFixedToLong( fb);
        sx += gScrollStarObject->sprite->where.h;
        sy += gScrollStarObject->sprite->where.v;

        count = gScrollStarObject->direction;
        mAddAngle( count, 30);
        GetRotPoint(&fa, &fb, count);
        fc = mLongToFixed( kSiteSize);
        fa = mMultiplyFixed( fc, fa);
        fb = mMultiplyFixed( fc, fb);

        sa = mFixedToLong( fa);
        sb = mFixedToLong( fb);

        mGetTranslateColorShade( PALE_GREEN, MEDIUM, color, transColor);
        if ( doDraw)
        {
            DrawNateLine(gOffWorld, clipRect, (sx + sa),
                    (sy + sb), sx, sy, 0,
                    0, color);
        }

        count = gScrollStarObject->direction;
        mAddAngle( count, -30);
        GetRotPoint(&fa, &fb, count);
        fc = mLongToFixed( kSiteSize);
        fa = mMultiplyFixed( fc, fa);
        fb = mMultiplyFixed( fc, fb);

        sc = mFixedToLong( fa);
        sd = mFixedToLong( fb);

        if ( doDraw)
        {
            DrawNateLine(gOffWorld, clipRect, (sx + sc),
                    (sy + sd), sx, sy, 0,
                    0, color);
            mGetTranslateColorShade( PALE_GREEN, DARKER+kSlightlyDarkerColor, color, transColor);
            DrawNateLine(gOffWorld, clipRect, (sx + sc),
                    (sy + sd), (sx + sa), (sy + sb), 0,
                    0, color);
        }

        l = globals()->gSectorLineData.get() + kSiteCoordOffset;
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
    } else if ( cursorCoord.v > ( /*CLIP_BOTTOM*/globals()->gTrueClipBottom - kCursorBoundsSize - 1))
    {
        cursorCoord.v = /*CLIP_BOTTOM*/globals()->gTrueClipBottom - kCursorBoundsSize - 1;
//      ShowSpriteCursor( true);
    }

    l = globals()->gSectorLineData.get() + kCursorCoordOffset;
    sx = *(l++) = cursorCoord.h;
    sy = *(l++) = cursorCoord.v;

    sa = *(l++);
    sb = *(l++);

    if ((( sa != sx) || ( sb != sy)) && ( !SpriteCursorVisible())) globals()->gMouseActive = kMouseActive;
    else if ( globals()->gMouseActive > kMouseSleepTime) globals()->gMouseActive = kMouseTurningOff;

    if ( globals()->gMouseActive > kMouseTurningOff)
    {
        mGetTranslateColorShade( SKY_BLUE, MEDIUM, color, transColor);
        DrawNateLine(gOffWorld, clipRect, sx,
                    clipRect.top, sx, (sy - kCursorBoundsSize), 0,
                    0, color);
        DrawNateLine(gOffWorld, clipRect, sx,
                    (sy + kCursorBoundsSize), sx, clipRect.bottom - 1, 0,
                    0, color);
        DrawNateLine(gOffWorld, clipRect, clipRect.left,
                    sy, (sx - kCursorBoundsSize), sy, 0,
                    0, color);
        DrawNateLine(gOffWorld, clipRect, (sx + kCursorBoundsSize),
                    sy, clipRect.right - 1, sy, 0,
                    0, color);
    }
}

void DrawSectorLines() {
    int32_t         *l;
    uint32_t        size, level, x, h, division;
    Rect        clipRect;
    unsigned char   color;
    transColorType  *transColor;
    bool         doDraw;

    if ( gScrollStarObject != nil)
    {
        if ( gScrollStarObject->offlineTime <= 0) doDraw = true;
        else
        {
            if ( Randomize( gScrollStarObject->offlineTime) < 5) doDraw = true;
            else doDraw = false;
        }
    } else doDraw = false;

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

    l = globals()->gSectorLineData.get();
    if ( doDraw)
    {
        while ((x < implicit_cast<uint32_t>(CLIP_RIGHT)) && (h > 0)) {
            if (!division) {
                mGetTranslateColorShade( GREEN, kSectorLineBrightness, color, transColor);
            } else if ( !(division & 0x3)) {
                mGetTranslateColorShade( SKY_BLUE, kSectorLineBrightness, color, transColor);
            } else {
                mGetTranslateColorShade( BLUE, kSectorLineBrightness, color, transColor);
            }

            DrawNateLine(gOffWorld, clipRect, x, CLIP_TOP, x, CLIP_BOTTOM, 0, 0, color);
            *l = x;
            l += 2;
            division += level;
            division &= 0x0000000f;
            x += h;
        }
    } else
    {
        while ((x < implicit_cast<uint32_t>(CLIP_RIGHT)) && (h > 0)) {
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

    l = globals()->gSectorLineData.get() + (kMaxSectorLine << 1L);

    if ( doDraw)
    {
        while ((x < implicit_cast<uint32_t>(CLIP_BOTTOM)) && (h > 0)) {
            if (!division) {
                mGetTranslateColorShade( GREEN, kSectorLineBrightness, color, transColor);
            } else if ( !(division & 0x3)) {
                mGetTranslateColorShade( SKY_BLUE, kSectorLineBrightness, color, transColor);
            } else {
                mGetTranslateColorShade( BLUE, kSectorLineBrightness, color, transColor);
            }

            DrawNateLine(gOffWorld, clipRect, CLIP_LEFT, x, CLIP_RIGHT, x, 0, 0, color);
            *l = x;
            l += 2;

            division += level;
            division &= 0x0000000f;

            x += h;
        }
    } else
    {
        while ((x < implicit_cast<uint32_t>(CLIP_BOTTOM)) && (h > 0)) {
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

    if ((( globals()->gLastScale < kBlipThreshhold) && ( gAbsoluteScale >= kBlipThreshhold)) ||
        (( globals()->gLastScale >= kBlipThreshhold) && ( gAbsoluteScale < kBlipThreshhold)))
        PlayVolumeSound(  kComputerBeep4, kMediumVolume, kMediumPersistence, kLowPrioritySound);

    globals()->gLastScale = gAbsoluteScale;
    gLastGlobalCorner = gGlobalCorner;
}


void ShowSite() {
    int16_t         sx, sy, sa, sb, sc, sd;
    Rect        clipRect;
    int32_t         *l;

    clipRect.left = CLIP_LEFT;
    clipRect.right = CLIP_RIGHT;
    clipRect.top = CLIP_TOP;
    clipRect.bottom = CLIP_BOTTOM;

    l = globals()->gSectorLineData.get() + kSiteCoordOffset;
    sx = *(l++);
    sy = *(l++);
    sa = *(l++);
    sb = *(l++);
    sc = *(l++);
    sd = *(l++);

    CopyNateLine(gOffWorld, gActiveWorld, clipRect, sa,
                sb, sx, sy,
                gNatePortLeft << 2L, gNatePortTop);
    CopyNateLine(gOffWorld, gActiveWorld, clipRect, sc,
                sd, sx, sy,
                gNatePortLeft << 2L, gNatePortTop);
    CopyNateLine(gOffWorld, gActiveWorld, clipRect, sc,
                sd, sa, sb,
                gNatePortLeft << 2L, gNatePortTop);

    sx = *(l++);
    sy = *(l++);
    sa = *(l++);
    sb = *(l++);
    sc = *(l++);
    sd = *(l++);
    CopyNateLine(gOffWorld, gActiveWorld, clipRect, sa,
                sb, sx, sy,
                gNatePortLeft << 2L, gNatePortTop);
    CopyNateLine(gOffWorld, gActiveWorld, clipRect, sc,
                sd, sx, sy,
                gNatePortLeft << 2L, gNatePortTop);
    CopyNateLine(gOffWorld, gActiveWorld, clipRect, sc,
                sd, sa, sb,
                gNatePortLeft << 2L, gNatePortTop);

    l = globals()->gSectorLineData.get() + kCursorCoordOffset;
    sx = *(l++);
    sy = *(l++);

    if ( globals()->gMouseActive > kMouseTurningOff)
    {
        CopyNateLine(gOffWorld, gActiveWorld, clipRect, sx,
                    clipRect.top, sx, clipRect.bottom - 1,
                    gNatePortLeft << 2L, gNatePortTop);
        CopyNateLine(gOffWorld, gActiveWorld, clipRect, clipRect.left,
                    sy, clipRect.right - 1, sy,
                    gNatePortLeft << 2L, gNatePortTop);
    }

    sx = *(l++);
    sy = *(l++);

    if ( globals()->gMouseActive)
    {
        CopyNateLine(gOffWorld, gActiveWorld, clipRect, sx,
                    clipRect.top, sx, (sy - kCursorBoundsSize),
                    gNatePortLeft << 2L, gNatePortTop);
        CopyNateLine(gOffWorld, gActiveWorld, clipRect, sx,
                    (sy + kCursorBoundsSize), sx, clipRect.bottom - 1,
                    gNatePortLeft << 2L, gNatePortTop);
        CopyNateLine(gOffWorld, gActiveWorld, clipRect, clipRect.left,
                    sy, (sx - kCursorBoundsSize), sy,
                    gNatePortLeft << 2L, gNatePortTop);
        CopyNateLine(gOffWorld, gActiveWorld, clipRect, (sx + kCursorBoundsSize),
                    sy, clipRect.right - 1, sy,
                    gNatePortLeft << 2L, gNatePortTop);
        if ( globals()->gMouseActive == kMouseTurningOff) globals()->gMouseActive = kMouseOff;
    }
}

void ShowSectorLines() {
    int32_t         *l, count;
    Rect        clipRect;

    clipRect.left = CLIP_LEFT;
    clipRect.right = CLIP_RIGHT;
    clipRect.top = CLIP_TOP;
    clipRect.bottom = CLIP_BOTTOM;

    l = globals()->gSectorLineData.get();
    count = 0;

    while ( count < kMaxSectorLine)
    {
        if ( *l != -1)
            CopyNateLine(gOffWorld, gActiveWorld, clipRect, *l, CLIP_TOP, *l, CLIP_BOTTOM,
                gNatePortLeft << 2L, gNatePortTop);
        l++;
        if ( *l != -1)
            CopyNateLine(gOffWorld, gActiveWorld, clipRect, *l, CLIP_TOP, *l, CLIP_BOTTOM,
                gNatePortLeft << 2L, gNatePortTop);
        l++;
        count++;
    }

    l = globals()->gSectorLineData.get() + (kMaxSectorLine << 1L);
    count = 0;

    while ( count < kMaxSectorLine)
    {
        if ( *l != -1)
            CopyNateLine(gOffWorld, gActiveWorld, clipRect, CLIP_LEFT, *l, CLIP_RIGHT, *l,
                gNatePortLeft << 2L, gNatePortTop);
        l++;
        if ( *l != -1)
            CopyNateLine(gOffWorld, gActiveWorld, clipRect, CLIP_LEFT, *l, CLIP_RIGHT, *l,
                gNatePortLeft << 2L, gNatePortTop);
        l++;
        count++;
    }


}

void InstrumentsHandleClick() {
    Point   where;
    int32_t *l;

    l = globals()->gSectorLineData.get() + kCursorCoordOffset;
    where.h = *(l++);
    where.v = *(l++);

    PlayerShipHandleClick( where);
    MiniComputerHandleClick( where);
    if (!SpriteCursorVisible()) globals()->gMouseActive = kMouseActive;
}

void InstrumentsHandleDoubleClick() {
    Point   where;
    int32_t *l;

    l = globals()->gSectorLineData.get() + kCursorCoordOffset;
    where.h = *(l++);
    where.v = *(l++);

    PlayerShipHandleClick( where);
    MiniComputerHandleDoubleClick( where);
    if (!SpriteCursorVisible()) globals()->gMouseActive = kMouseActive;
}

void InstrumentsHandleMouseUp() {
    Point   where;
    int32_t *l;

    l = globals()->gSectorLineData.get() + kCursorCoordOffset;
    where.h = *(l++);
    where.v = *(l++);

    MiniComputerHandleMouseUp( where);
}

void InstrumentsHandleMouseStillDown() {
    Point   where;
    int32_t *l;

    l = globals()->gSectorLineData.get() + kCursorCoordOffset;
    where.h = *(l++);
    where.v = *(l++);

    MiniComputerHandleMouseStillDown( where);
}

void DrawArbitrarySectorLines(coordPointType *corner, int32_t scale, int32_t minSectorSize,
        Rect *bounds, PixMap* pixBase, int32_t portLeft, int32_t portTop) {
    uint32_t        size, level, x, h, division;
    Rect        clipRect;
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
    } while (h < implicit_cast<uint32_t>(minSectorSize));
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

    while ((x < implicit_cast<uint32_t>(bounds->right)) && (h > 0)) {
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

        DrawNateLine( pixBase, clipRect, x, bounds->top, x, bounds->bottom,
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

    while ((x < implicit_cast<uint32_t>(bounds->bottom)) && (h > 0)) {
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

        DrawNateLine( pixBase, clipRect, bounds->left, x, bounds->right, x,
                portLeft << 2, portTop, color);

        division += level;
        division &= 0x0000000f;

        x += h;
    }
}

void GetArbitrarySingleSectorBounds(coordPointType *corner, coordPointType *location,
        int32_t scale, int32_t minSectorSize, Rect *bounds, Rect *destRect) {
    uint32_t    size, level, x, h, division, scaledLoc;
    Rect        clipRect;

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
    } while (h < implicit_cast<uint32_t>(minSectorSize));
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

    while ((x < implicit_cast<uint32_t>(bounds->right)) && (h > 0)) {
        division += level;
        division &= 0x0000000f;

        if ((x < scaledLoc) && (x > implicit_cast<uint32_t>(destRect->left))) {
            destRect->left = x;
        }
        if ((x > scaledLoc) && (x < implicit_cast<uint32_t>(destRect->right))) {
            destRect->right = x;
        }

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

    while ((x < implicit_cast<uint32_t>(bounds->bottom)) && (h > 0)) {
        division += level;
        division &= 0x0000000f;

        if ((x < scaledLoc) && (x > implicit_cast<uint32_t>(destRect->top))) {
            destRect->top = x;
        }
        if ((x > scaledLoc) && (x < implicit_cast<uint32_t>(destRect->bottom))) {
            destRect->bottom = x;
        }

        x += h;
    }
}

void UpdateBarIndicator(int16_t which, int32_t value, int32_t max, PixMap* pixMap) {
    int32_t         graphicValue;
    Rect        tRect, clipRect;
    transColorType  *transColor;
    unsigned char   color, lightColor, darkColor;
    Rect            rrect;

    if ( value > max) value = max;

    if ( value != globals()->gBarIndicator[ which].thisValue)
    {
        globals()->gBarIndicator[ which].lastValue = globals()->gBarIndicator[ which].thisValue;
        if ( max > 0)
        {
            graphicValue = kBarIndicatorHeight * value;
            graphicValue /= max;
            if ( graphicValue < 0) graphicValue = 0;
            if ( graphicValue > kBarIndicatorHeight) graphicValue = kBarIndicatorHeight;
        } else graphicValue = 0;


        tRect.left = kBarIndicatorLeft + globals()->gRightPanelLeftEdge;
        tRect.right =  tRect.left + kBarIndicatorWidth;
        tRect.top = globals()->gBarIndicator[ which].top,
        tRect.bottom = tRect.top + kBarIndicatorHeight - graphicValue;
        clipRect.left = globals()->gRightPanelLeftEdge;
        clipRect.top = globals()->gInstrumentTop;
        clipRect.right = clipRect.left + kRightPanelWidth;
        clipRect.bottom = clipRect.top + kPanelHeight;

        mGetTranslateColorShade( globals()->gBarIndicator[which].color, DARK, color, transColor);
        mGetTranslateColorShade( globals()->gBarIndicator[which].color, MEDIUM, lightColor, transColor);
        mGetTranslateColorShade( globals()->gBarIndicator[which].color, DARKER, darkColor, transColor);
//      DrawNateRect( *pixMap, &tRect, gNatePortLeft << 2, gNatePortTop, color);
        DrawNateShadedRect(pixMap, &tRect, clipRect, gNatePortLeft << 2L, gNatePortTop, color, lightColor, darkColor);

        if ( graphicValue > 0)
        {
            tRect.top = tRect.bottom;//tRect.bottom - graphicValue;
            tRect.bottom = globals()->gBarIndicator[which].top + kBarIndicatorHeight;
            mGetTranslateColorShade( globals()->gBarIndicator[which].color, LIGHTER, color, transColor);
            mGetTranslateColorShade( globals()->gBarIndicator[which].color, VERY_LIGHT, lightColor, transColor);
            mGetTranslateColorShade( globals()->gBarIndicator[which].color, MEDIUM, darkColor, transColor);
    //      DrawNateRect( *pixMap, &tRect, gNatePortLeft << 2, gNatePortTop, color);
            DrawNateShadedRect(pixMap, &tRect, clipRect, gNatePortLeft << 2L, gNatePortTop, color, lightColor, darkColor);
        }
        rrect = Rect(tRect.left, globals()->gBarIndicator[ which].top,
            tRect.right, globals()->gBarIndicator[ which].top + kBarIndicatorHeight);
        CopyRealWorldToOffWorld(rrect);

        globals()->gBarIndicator[ which].thisValue = value;
    }
}

void DrawBuildTimeBar(int32_t value) {
    Rect        tRect, clipRect;
    transColorType  *transColor;
    unsigned char   color;

    if ( value < 0) value = 0;
    value = kMiniBuildTimeHeight - value;

    tRect.left = clipRect.left = kMiniBuildTimeLeft;
    tRect.top = clipRect.top = kMiniBuildTimeTop + globals()->gInstrumentTop;
    tRect.bottom = clipRect.bottom = kMiniBuildTimeBottom + globals()->gInstrumentTop;
    tRect.right = clipRect.right = kMiniBuildTimeRight;

    mGetTranslateColorShade( PALE_PURPLE, MEDIUM, color, transColor);
    DrawNateVBracket( gActiveWorld, tRect, clipRect, gNatePortLeft << 2L, gNatePortTop,
                     color);

    tRect.left = kMiniBuildTimeLeft + 2;
    tRect.top = kMiniBuildTimeTop + 2 + value + globals()->gInstrumentTop;
    tRect.bottom = kMiniBuildTimeBottom - 2 + globals()->gInstrumentTop;
    tRect.right = kMiniBuildTimeRight - 2;

    if ( value > 0)
    {
        mGetTranslateColorShade( PALE_PURPLE, LIGHT, color, transColor);

        DrawNateRect( gActiveWorld, &tRect, gNatePortLeft << 2L, gNatePortTop, color);

        tRect.top = kMiniBuildTimeTop + 2 + globals()->gInstrumentTop;
    } else value = 0;

    tRect.bottom = kMiniBuildTimeTop + 2 + value + globals()->gInstrumentTop;

    mGetTranslateColorShade( PALE_PURPLE, DARK, color, transColor);

    DrawNateRect( gActiveWorld, &tRect, gNatePortLeft << 2L, gNatePortTop, color);
}

}  // namespace antares

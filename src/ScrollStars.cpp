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

// Scroll Stars.c *** also includes drawing for BEAMS, since they involve direct-screen lines

#include "ScrollStars.hpp"

#include <QDOffscreen.h>

#include "AresGlobalType.hpp"
#include "ColorTranslation.hpp"
#include "ConditionalMacros.h"
#include "Debug.hpp"
#include "Error.hpp"
#include "MathMacros.hpp"
#include "Motion.hpp"
#include "NateDraw.hpp"
#include "OffscreenGWorld.hpp"
#include "SpaceObject.hpp"
#include "Randomize.hpp"
#include "Resources.h"

#ifdef powerc
#define kScrollStarNum          125
#define kSparkStarNum           125
#define kAllStarNum             250         // scrollStarNum + sparkStarNum
#else
#define kScrollStarNum          25
#define kSparkStarNum           25
#define kAllStarNum             50              // scrollStarNum + sparkStarNum
#endif
#define kSparkStarOffset        kScrollStarNum

#define kMinimumStarSpeed       1
#define kMaximumStarSpeed       3
#define kStarSpeedSpread        ( kMaximumStarSpeed - kMinimumStarSpeed + 1)

#define kSlowStarFraction       0x00000080  // this is 1/2 so could be made to asr 1 ( >> 1)
#define kMediumStarFraction     0x000000c0  // this is .75 could be made to 1 - asr 2 ( 1 - ( >> 2))
#define kFastStarFraction       0x00000100  // this is 1 so could be ignored

#define kStarColor              GRAY

#define k3DStarInitialCo        .004        // as small as it gets
#define kMax3DStarAge           4096L
#define k3DStarAgeToShadeShift  8L

#define kScrollStarError        "\pSTAR"

#define kUseScrollStar

extern PixMapHandle     thePixMapHandle;
extern long             gNatePortLeft, gNatePortTop, gAbsoluteScale,
                        CLIP_LEFT, CLIP_TOP, CLIP_RIGHT, CLIP_BOTTOM,
                        gPlayScreenWidth, gPlayScreenHeight, gRootObjectNumber;
extern TypedHandle<spaceObjectType> gSpaceObjectData;
extern coordPointType   gGlobalCorner;
extern GWorldPtr        gOffWorld;
extern spaceObjectType* gRootObject;

spaceObjectType *gScrollStarObject = nil;   // this object is also used for the radar center

void CorrectScrollStarObject( Handle);

inline StarSpeed RandomStarSpeed() {
    return static_cast<StarSpeed>(Randomize(kStarSpeedSpread) + kMinimumStarSpeed);
}

int InitScrollStars() {
    scrollStarType  *star;
    short           i;

    globals()->gScrollStarData.create(kAllStarNum);
    star = *globals()->gScrollStarData;
    for (i = 0; i < kAllStarNum; i++) {
        star->speed = kNoStar;
        star++;
    }
    globals()->gLastClipBottom = CLIP_BOTTOM;

    return kNoError;
}

void CleanupScrollStars( void)

{
#ifdef kUseScrollStar
    if (globals()->gScrollStarData.get() != nil) {
        globals()->gScrollStarData.destroy();
    }
#endif
}

void ResetScrollStars ( long which)

{
    short           i;
    scrollStarType  *star;
    spaceObjectType *centerObject = *gSpaceObjectData + which;

    gScrollStarObject = centerObject;
    globals()->gScrollStarNumber = which;

#ifdef kUseScrollStar
    if ( gScrollStarObject != nil)
    {
        star = *globals()->gScrollStarData;
        for ( i = 0; i < kScrollStarNum; i++)
        {
            star->location.h = Randomize( gPlayScreenWidth) + CLIP_LEFT;
            star->location.v = Randomize( gPlayScreenHeight) + CLIP_TOP;
            star->motionFraction.h = star->motionFraction.v = 0;

            star->speed = RandomStarSpeed();
            star->age = 0;

            star++;
        }
        for ( i = kSparkStarOffset; i < kAllStarNum; i++)
        {
//          star->speed = kNoStar;
            star->age = 0;
            star++;
        }
    }
#endif
}

void MakeNewSparks( long sparkNum, long sparkSpeed, smallFixedType maxVelocity,
                    unsigned char color, longPointType *location)

{
    long            i, whichSpark = kSparkStarOffset;
    scrollStarType  *spark = *globals()->gScrollStarData + kSparkStarOffset;

    maxVelocity *= gAbsoluteScale;
    maxVelocity >>= SHIFT_SCALE;

    for ( i = 0; i < sparkNum; i++)
    {
        while (( whichSpark < kAllStarNum) && ( spark->speed != kNoStar))
        {
            whichSpark++;
            spark++;
        }

        if ( whichSpark < kAllStarNum)
        {
            spark->velocity.h = Randomize( maxVelocity << 2L)
                                - maxVelocity;
            spark->velocity.v = Randomize( maxVelocity << 2L)
                                - maxVelocity;
            spark->oldLocation.h = spark->location.h = location->h;
            spark->oldLocation.v = spark->location.v = location->v;
            spark->motionFraction.h = spark->motionFraction.v = 0;
            spark->age = kMaxSparkAge;
            spark->speed = static_cast<StarSpeed>(sparkSpeed);
            spark->color = color;
        }
    }
}

// PrepareToMoveScrollStars:
//  We need to save the stars' last position since we may move them several times before they
//  are redrawn; the old positions have to be erased right after the new ones are drawn.

void PrepareToMoveScrollStars( void)

{
#ifdef kUseScrollStar
    short           i;
    scrollStarType  *star;

    star = *globals()->gScrollStarData;
    for ( i = 0; i < kAllStarNum; i++)
    {
        star->oldOldLocation.h = star->oldLocation.h;
        star->oldOldLocation.v = star->oldLocation.v;
        star->oldLocation.h = star->location.h;
        star->oldLocation.v = star->location.v;

        star++;
    }
#endif
}

void MoveScrollStars( const long byUnits)

{
#ifdef kUseScrollStar
    short           i;
    scrollStarType  *star;
    fixedPointType  slowVelocity, mediumVelocity, fastVelocity, *velocity = NULL;
    long            h, v;

    if ( gScrollStarObject == nil) return;
    if ( !gScrollStarObject->active) return;

    slowVelocity.h = ( mMultiplyFixed(gScrollStarObject->velocity.h, kSlowStarFraction))
                         * byUnits;
    slowVelocity.h *= gAbsoluteScale;
    slowVelocity.h /= SCALE_SCALE;

    slowVelocity.v = ( mMultiplyFixed(gScrollStarObject->velocity.v, kSlowStarFraction))
                         * byUnits;
    slowVelocity.v *= gAbsoluteScale;
    slowVelocity.v /= SCALE_SCALE;

    mediumVelocity.h = ( mMultiplyFixed(gScrollStarObject->velocity.h, kMediumStarFraction))
                         * byUnits;
    mediumVelocity.h *= gAbsoluteScale;
    mediumVelocity.h /= SCALE_SCALE;
    mediumVelocity.v = ( mMultiplyFixed(gScrollStarObject->velocity.v, kMediumStarFraction))
                         * byUnits;
    mediumVelocity.v *= gAbsoluteScale;
    mediumVelocity.v /= SCALE_SCALE;

    fastVelocity.h = ( mMultiplyFixed(gScrollStarObject->velocity.h, kFastStarFraction))
                         * byUnits;
    fastVelocity.h *= gAbsoluteScale;
    fastVelocity.h /= SCALE_SCALE;
    fastVelocity.v = ( mMultiplyFixed(gScrollStarObject->velocity.v, kFastStarFraction))
                         * byUnits;
    fastVelocity.v *= gAbsoluteScale;
    fastVelocity.v /= SCALE_SCALE;

    star = *globals()->gScrollStarData;
    for ( i = 0; i < kScrollStarNum; i++)
    {
        if ( star->speed != kNoStar)
        {
            switch ( star->speed)
            {
                case kSlowStarSpeed:
                    velocity = &slowVelocity;
                    break;
                case kMediumStarSpeed:
                    velocity = &mediumVelocity;
                    break;
                case kFastStarSpeed:
                    velocity = &fastVelocity;
                    break;
                case kNoStar:
                    exit(1);
                    break;
            }

            star->motionFraction.h += velocity->h;
            star->motionFraction.v += velocity->v;

            if ( star->motionFraction.h >= 0)
                h = ( star->motionFraction.h + kFixedPlusPointFive) >> kFixedBitShiftNumber;
            else
                h = (( star->motionFraction.h - kFixedPlusPointFive) >> kFixedBitShiftNumber) + 1;
            star->location.h += h;
            star->motionFraction.h -= mLongToFixed(h);

            if ( star->motionFraction.v >= 0)
                v = ( star->motionFraction.v + kFixedPlusPointFive) >> kFixedBitShiftNumber;
            else
                v = (( star->motionFraction.v - kFixedPlusPointFive) >> kFixedBitShiftNumber) + 1;
            star->location.v += v;
            star->motionFraction.v -= mLongToFixed(v);

            if (( star->location.h < CLIP_LEFT) && ( star->oldLocation.h < CLIP_LEFT))
            {
                star->location.h += gPlayScreenWidth - 1;
                star->location.v = Randomize( gPlayScreenHeight) + CLIP_TOP;
                star->motionFraction.h = star->motionFraction.v = 0;
                star->speed = RandomStarSpeed();
                star->age = 0;
            } else if (( star->location.h >= CLIP_RIGHT) && ( star->oldLocation.h >= CLIP_RIGHT))
            {
                star->location.h -= gPlayScreenWidth;
                star->location.v = Randomize( gPlayScreenHeight) + CLIP_TOP;
                star->motionFraction.h = star->motionFraction.v = 0;
                star->speed = RandomStarSpeed();
                star->age = 0;
            } else if (( star->location.v < CLIP_TOP) && ( star->oldLocation.v < CLIP_TOP))
            {
                star->location.h = Randomize( gPlayScreenWidth) + CLIP_LEFT;
                star->location.v += gPlayScreenHeight - 1;
                star->motionFraction.h = star->motionFraction.v = 0;
                star->speed = RandomStarSpeed();
                star->age = 0;
            } else if (( star->location.v >= globals()->gTrueClipBottom) && ( star->oldLocation.v >= globals()->gTrueClipBottom))
            {
                star->location.h = Randomize( gPlayScreenWidth) + CLIP_LEFT;
                star->location.v -= gPlayScreenHeight;
                star->motionFraction.h = star->motionFraction.v = 0;
                star->speed = RandomStarSpeed();
                star->age = 0;
            }

            if ( (globals()->gWarpStars) && ( star->age == 0))
            {
                switch ( star->speed)
                {
                    case kSlowStarSpeed:
                        velocity = &slowVelocity;
                        break;
                    case kMediumStarSpeed:
                        velocity = &mediumVelocity;
                        break;
                    case kFastStarSpeed:
                        velocity = &fastVelocity;
                        break;
                    case kNoStar:
                        exit(1);
                        break;
                }
                if ( velocity->h >= 0)
                    star->location.h -= (velocity->h >> kFixedBitShiftNumber);
                else
                    star->location.h -= (velocity->h >> kFixedBitShiftNumber) + 1;
                if ( velocity->v >= 0)
                    star->location.v -= (velocity->v >> kFixedBitShiftNumber);
                else
                    star->location.v -= (velocity->v >> kFixedBitShiftNumber) + 1;
            }
        }

        star++;
    }

    for ( i = kSparkStarOffset; i < kAllStarNum; i++)
    {
        if ( star->speed != kNoStar)
        {
            star->age -= star->speed * byUnits;

            star->motionFraction.h += star->velocity.h * byUnits + slowVelocity.h;
            star->motionFraction.v += star->velocity.v * byUnits + slowVelocity.v;

            if ( star->motionFraction.h >= 0)
                h = ( star->motionFraction.h + kFixedPlusPointFive) >> kFixedBitShiftNumber;
            else
                h = (( star->motionFraction.h - kFixedPlusPointFive) >> kFixedBitShiftNumber) + 1;
            star->location.h += h;
            star->motionFraction.h -= mLongToFixed(h);

            if ( star->motionFraction.v >= 0)
                v = ( star->motionFraction.v + kFixedPlusPointFive) >> kFixedBitShiftNumber;
            else
                v = (( star->motionFraction.v - kFixedPlusPointFive) >> kFixedBitShiftNumber) + 1;
            star->location.v += v;
            star->motionFraction.v -= mLongToFixed(v);
        }
        star++;
    }
#endif
}

void DrawScrollStars( Boolean warp)

{
#ifdef kUseScrollStar
    short           i;
    scrollStarType  *star;
    unsigned char   slowColor, mediumColor, fastColor, *color, *dByte;
    long            rowBytes;
    transColorType  *transColor;
    Rect        bounds, lastBounds;
    PixMapHandle    offMap;

#pragma unused( warp)

    offMap = GetGWorldPixMap( gOffWorld);

/*  slowColor = GetTranslateColorShade( kStarColor, MEDIUM);
    mediumColor = GetTranslateColorShade( kStarColor, LIGHT);   //kStarColor
    fastColor = GetTranslateColorShade( kStarColor, LIGHTER);
*/
    mGetTranslateColorShade( kStarColor, MEDIUM, slowColor, transColor);
    mGetTranslateColorShade( kStarColor, LIGHT, mediumColor, transColor);
    mGetTranslateColorShade( kStarColor, LIGHTER, fastColor, transColor);

    mGetRowBytes( rowBytes, *offMap);
    bounds.left = lastBounds.left = CLIP_LEFT;
    bounds.top = lastBounds.top = CLIP_TOP;
    bounds.bottom = CLIP_BOTTOM;
    bounds.right = lastBounds.right = CLIP_RIGHT;
    lastBounds.bottom = globals()->gLastClipBottom;

    star = *globals()->gScrollStarData;

    if (( gScrollStarObject->presenceState != kWarpInPresence) &&
        ( gScrollStarObject->presenceState != kWarpOutPresence) &&
        ( gScrollStarObject->presenceState != kWarpingPresence))
    {
        if ( !globals()->gWarpStars) // we're not warping in any way
        {
            for ( i = 0; i < kScrollStarNum; i++)
            {
                if ( star->speed != kNoStar)
                {
                    color = &slowColor;
                    if ( star->speed == kMediumStarSpeed) color = &mediumColor;
                    else if ( star->speed == kFastStarSpeed) color = &fastColor;

                    if (( star->location.h >= CLIP_LEFT) && ( star->location.v >= CLIP_TOP)
                        && ( star->location.h < CLIP_RIGHT) && ( star->location.v < CLIP_BOTTOM))
                    {
                        mSetNatePixel( dByte, rowBytes, star->location.h, star->location.v, 0,
                                0, *offMap, *color);
            #ifdef kByteLevelTesting
                        TestByte( (char *)dByte, *offMap, "\pDRAWSTAR");
            #endif
                    }
                    if ((( star->location.h != star->oldLocation.h) ||
                    ( star->location.v != star->oldLocation.v)) &&
                    (( star->oldLocation.h >= CLIP_LEFT) && ( star->oldLocation.v >= CLIP_TOP)
                        && ( star->oldLocation.h < CLIP_RIGHT) && ( star->oldLocation.v < CLIP_BOTTOM)))
                    {
                        mSetNatePixel ( dByte, rowBytes, star->oldLocation.h, star->oldLocation.v,
                            0, 0, *offMap, 0xff);

        #ifdef kByteLevelTesting
                        TestByte( (char *)dByte, *offMap, "\pERASTAR");
        #endif
                    }
                }
                star++;
            }
        } else // we were warping but now are not; erase warped stars
        {
//          globals()->gWarpStars = FALSE;
            for ( i = 0; i < kScrollStarNum; i++)
            {
                if ( star->speed != kNoStar)
                {
                    if ( star->age > 0)
                    {
                        if ( star->age > 1)
                        {
                            DrawNateLine( *offMap, &lastBounds, star->oldOldLocation.h,
                                        star->oldOldLocation.v,
                                        star->oldLocation.h,
                                        star->oldLocation.v,
                                        0,
                                        0, 0xff);
                        }/* else star->age = 2;*/
                    } else
                    {
                        DrawNateLine( *offMap, &lastBounds, star->oldOldLocation.h,
                                    star->oldOldLocation.v,
                                    star->oldLocation.h,
                                    star->oldLocation.v,
                                    0,
                                    0, 0xff);
                        /*star->age = 1;*/
                    }
                }
                star++;
            }
        }
    } else // we're warping now
    {
//      globals()->gWarpStars = TRUE;

        for ( i = 0; i < kScrollStarNum; i++)
        {
            if ( star->speed != kNoStar)
            {
                color = &slowColor;
                if ( star->speed == kMediumStarSpeed) color = &mediumColor;
                else if ( star->speed == kFastStarSpeed) color = &fastColor;
                if ( star->age > 0)
                {
                    if ( star->age > 1)
                    {
                        DrawNateLine( *offMap, &lastBounds, star->oldOldLocation.h,
                                    star->oldOldLocation.v,
                                    star->oldLocation.h,
                                    star->oldLocation.v,
                                    0,
                                    0, 0xff);
                    }/* else star->age = 2;*/
                    DrawNateLine( *offMap, &bounds, star->oldLocation.h,
                                star->oldLocation.v,
                                star->location.h,
                                star->location.v,
                                0,
                                0, *color);
                } else
                {
                    DrawNateLine( *offMap, &lastBounds, star->oldOldLocation.h,
                                star->oldOldLocation.v,
                                star->oldLocation.h,
                                star->oldLocation.v,
                                0,
                                0, 0xff);
                    /*star->age = 1;*/
                }
            }
            star++;
        }
    }

    for ( i = kSparkStarOffset; i < kAllStarNum; i++)
    {
        if ( star->speed != kNoStar)
        {
            if ( star->age > 0)
            {
                if ( !((star->location.h < CLIP_LEFT) || ( star->location.h >= CLIP_RIGHT) ||
                    ( star->location.v < CLIP_TOP) ||  ( star->location.v >= CLIP_BOTTOM)))
                {
                    mGetTranslateColorShade( star->color, (star->age >> kSparkAgeToShadeShift) + 1, slowColor, transColor);
                    mSetNatePixel( dByte, rowBytes, star->location.h, star->location.v, 0,
                        0, *offMap, slowColor);
                }
            } else
            {
//              star->speed = kNoStar;
            }

            if ( !((star->oldLocation.h < CLIP_LEFT) || ( star->oldLocation.h >= CLIP_RIGHT) ||
                ( star->oldLocation.v < CLIP_TOP) ||  ( star->oldLocation.v >= CLIP_BOTTOM)))
            {
                mSetNatePixel ( dByte, rowBytes, star->oldLocation.h, star->oldLocation.v,
                    0, 0, *offMap, 0xff);
            }

        }
        star++;
    }

#endif
}

void ShowScrollStars( Boolean warp)

{
#ifdef kUseScrollStar
    short           i;
    scrollStarType  *star;
    unsigned char   slowColor, mediumColor, fastColor, *color, *dByte, *sByte;
    long            srowBytes, drowBytes;
    transColorType  *transColor;
    Rect        bounds, lastBounds;
    PixMapHandle    sourceMap;

#pragma unused( warp)

    sourceMap = GetGWorldPixMap( gOffWorld);

/*  slowColor = GetTranslateColorShade( kStarColor, MEDIUM);
    mediumColor = GetTranslateColorShade( kStarColor, LIGHT);   //kStarColor
    fastColor = GetTranslateColorShade( kStarColor, LIGHTER);
*/
    mGetTranslateColorShade( kStarColor, MEDIUM, slowColor, transColor);
    mGetTranslateColorShade( kStarColor, LIGHT, mediumColor, transColor);
    mGetTranslateColorShade( kStarColor, LIGHTER, fastColor, transColor);

    mGetRowBytes( srowBytes, *sourceMap);
    mGetRowBytes( drowBytes, *thePixMapHandle);

    bounds.left = lastBounds.left = CLIP_LEFT;
    bounds.top = lastBounds.top = CLIP_TOP;
    bounds.bottom = CLIP_BOTTOM;
    bounds.right = lastBounds.right = CLIP_RIGHT;
    lastBounds.bottom = globals()->gLastClipBottom;

    star = *globals()->gScrollStarData;

    if (( gScrollStarObject->presenceState != kWarpInPresence) &&
        ( gScrollStarObject->presenceState != kWarpOutPresence) &&
        ( gScrollStarObject->presenceState != kWarpingPresence))
    {
        if ( !globals()->gWarpStars) // we're not warping in any way
        {
            for ( i = 0; i < kScrollStarNum; i++)
            {
                if ( star->speed != kNoStar)
                {
                    color = &slowColor;
                    if ( star->speed == kMediumStarSpeed) color = &mediumColor;
                    else if ( star->speed == kFastStarSpeed) color = &fastColor;

                    if (( star->location.h >= CLIP_LEFT) && ( star->location.v >= CLIP_TOP)
                        && ( star->location.h < CLIP_RIGHT) && ( star->location.v < CLIP_BOTTOM))
                    {
                        mGetNatePixel( dByte, srowBytes, star->location.h, star->location.v, 0,
                            0, *sourceMap);
                        mSetNatePixel( sByte, drowBytes, star->location.h, star->location.v, gNatePortLeft << 2,
                                gNatePortTop, *thePixMapHandle, *dByte);
            #ifdef kByteLevelTesting
                        TestByte( (char *)dByte, *thePixMapHandle, "\pDRAWSTAR");
            #endif
                    }
                    if ((( star->location.h != star->oldLocation.h) ||
                    ( star->location.v != star->oldLocation.v)) &&
                    (( star->oldLocation.h >= CLIP_LEFT) && ( star->oldLocation.v >= CLIP_TOP)
                        && ( star->oldLocation.h < CLIP_RIGHT) && ( star->oldLocation.v < CLIP_BOTTOM)))
                    {
                        mGetNatePixel( dByte, srowBytes, star->oldLocation.h, star->oldLocation.v, 0,
                            0, *sourceMap);
                        mSetNatePixel( sByte, drowBytes, star->oldLocation.h, star->oldLocation.v, gNatePortLeft << 2,
                                gNatePortTop, *thePixMapHandle, *dByte);

        #ifdef kByteLevelTesting
                        TestByte( (char *)dByte, *thePixMapHandle, "\pERASTAR");
        #endif
                    }
                }
                star++;
            }
        } else // we were warping but now are not; erase warped stars
        {
            globals()->gWarpStars = FALSE;
            for ( i = 0; i < kScrollStarNum; i++)
            {
                if ( star->speed != kNoStar)
                {
                    if ( star->age > 0)
                    {
                        if ( star->age > 1)
                        {
/*                          DrawNateLine( *thePixMapHandle, &bounds, star->oldOldLocation.h,
                                        star->oldOldLocation.v,
                                        star->oldLocation.h,
                                        star->oldLocation.v,
                                        gNatePortLeft << 2,
                                        gNatePortTop, 0xff);
*/
                            CopyNateLine( *sourceMap, *thePixMapHandle, &lastBounds, star->oldOldLocation.h,
                                    star->oldOldLocation.v, star->oldLocation.h, star->oldLocation.v,
                                    gNatePortLeft << 2, gNatePortTop);
                        } else star->age = 2;
                    } else
                    {
/*                      DrawNateLine( *thePixMapHandle, &bounds, star->oldOldLocation.h,
                                    star->oldOldLocation.v,
                                    star->oldLocation.h,
                                    star->oldLocation.v,
                                    gNatePortLeft << 2,
                                    gNatePortTop, 0xff);
*/
                        CopyNateLine( *sourceMap, *thePixMapHandle, &lastBounds, star->oldOldLocation.h,
                                star->oldOldLocation.v, star->oldLocation.h, star->oldLocation.v,
                                gNatePortLeft << 2, gNatePortTop);
                        star->age = 1;
                    }
                }
                star++;
            }
        }
    } else // we're warping now
    {
        globals()->gWarpStars = TRUE;

        for ( i = 0; i < kScrollStarNum; i++)
        {
            if ( star->speed != kNoStar)
            {
                color = &slowColor;
                if ( star->speed == kMediumStarSpeed) color = &mediumColor;
                else if ( star->speed == kFastStarSpeed) color = &fastColor;
                if ( star->age > 0)
                {
                    if ( star->age > 1)
                    {
/*                      DrawNateLine( *thePixMapHandle, &bounds, star->oldOldLocation.h,
                                    star->oldOldLocation.v,
                                    star->oldLocation.h,
                                    star->oldLocation.v,
                                    gNatePortLeft << 2,
                                    gNatePortTop, 0xff);
*/
                        CopyNateLine( *sourceMap, *thePixMapHandle, &lastBounds, star->oldOldLocation.h,
                                star->oldOldLocation.v, star->oldLocation.h, star->oldLocation.v,
                                gNatePortLeft << 2, gNatePortTop);
                    } else star->age = 2;
/*                  DrawNateLine( *thePixMapHandle, &bounds, star->oldLocation.h,
                                star->oldLocation.v,
                                star->location.h,
                                star->location.v,
                                gNatePortLeft << 2,
                                gNatePortTop, *color);
*/
                    CopyNateLine( *sourceMap, *thePixMapHandle, &bounds, star->oldLocation.h,
                            star->oldLocation.v, star->location.h, star->location.v,
                            gNatePortLeft << 2, gNatePortTop);
                } else
                {
/*                  DrawNateLine( *thePixMapHandle, &bounds, star->oldOldLocation.h,
                                star->oldOldLocation.v,
                                star->oldLocation.h,
                                star->oldLocation.v,
                                gNatePortLeft << 2,
                                gNatePortTop, 0xff);
*/
                    CopyNateLine( *sourceMap, *thePixMapHandle, &lastBounds, star->oldOldLocation.h,
                            star->oldOldLocation.v, star->oldLocation.h, star->oldLocation.v,
                            gNatePortLeft << 2, gNatePortTop);
                    star->age = 1;
                }
            }
            star++;
        }
    }

    for ( i = kSparkStarOffset; i < kAllStarNum; i++)
    {
        if ( star->speed != kNoStar)
        {
            if ( star->age > 0)
            {
                if ( !((star->location.h < CLIP_LEFT) || ( star->location.h >= CLIP_RIGHT) ||
                    ( star->location.v < CLIP_TOP) ||  ( star->location.v >= CLIP_BOTTOM)))
                {
/*                  mGetTranslateColorShade( star->color, star->age >> kSparkAgeToShadeShift, slowColor, transColor);
                    mSetNatePixel( dByte, rowBytes, star->location.h, star->location.v, gNatePortLeft << 2,
                        gNatePortTop, *thePixMapHandle, slowColor)
*/
                    mGetNatePixel( dByte, srowBytes, star->location.h, star->location.v, 0,
                        0, *sourceMap);
                    mSetNatePixel( sByte, drowBytes, star->location.h, star->location.v, gNatePortLeft << 2,
                            gNatePortTop, *thePixMapHandle, *dByte);
                }
            } else
            {
                star->speed = kNoStar;
            }

            if ( !((star->oldLocation.h < CLIP_LEFT) || ( star->oldLocation.h >= CLIP_RIGHT) ||
                ( star->oldLocation.v < CLIP_TOP) ||  ( star->oldLocation.v >= CLIP_BOTTOM)))
            {
/*              mSetNatePixel ( dByte, rowBytes, star->oldLocation.h, star->oldLocation.v,
                    gNatePortLeft << 2, gNatePortTop, *thePixMapHandle, 0xff)
*/
                mGetNatePixel( dByte, srowBytes, star->oldLocation.h, star->oldLocation.v, 0,
                    0, *sourceMap);
                mSetNatePixel( sByte, drowBytes, star->oldLocation.h, star->oldLocation.v, gNatePortLeft << 2,
                        gNatePortTop, *thePixMapHandle, *dByte);
            }

        }
        star++;
    }

    globals()->gLastClipBottom = CLIP_BOTTOM;
#endif
}

// DontShowScrollStars:
//  does everything except actually show the stars; this is for when you want update everything
//  without copying to the screen, as is the case with QuickDraw only, when it copies the
//  whole screen by itself.

void DontShowScrollStars( void)

{
#ifdef kUseScrollStar
    short           i;
    scrollStarType  *star;
    Rect        bounds, lastBounds;

    bounds.left = lastBounds.left = CLIP_LEFT;
    bounds.top = lastBounds.top = CLIP_TOP;
    bounds.bottom = CLIP_BOTTOM;
    bounds.right = lastBounds.right = CLIP_RIGHT;
    lastBounds.bottom = globals()->gLastClipBottom;

    star = *globals()->gScrollStarData;

    if (( gScrollStarObject->presenceState != kWarpInPresence) &&
        ( gScrollStarObject->presenceState != kWarpOutPresence) &&
        ( gScrollStarObject->presenceState != kWarpingPresence))
    {
        if ( !globals()->gWarpStars) // we're not warping in any way
        {
            for ( i = 0; i < kScrollStarNum; i++)
            {
                if ( star->speed != kNoStar)
                {
                    if (( star->location.h >= CLIP_LEFT) && ( star->location.v >= CLIP_TOP)
                        && ( star->location.h < CLIP_RIGHT) && ( star->location.v < CLIP_BOTTOM))
                    {
                    }
                    if ((( star->location.h != star->oldLocation.h) ||
                    ( star->location.v != star->oldLocation.v)) &&
                    (( star->oldLocation.h >= CLIP_LEFT) && ( star->oldLocation.v >= CLIP_TOP)
                        && ( star->oldLocation.h < CLIP_RIGHT) && ( star->oldLocation.v < CLIP_BOTTOM)))
                    {
                    }
                }
                star++;
            }
        } else // we were warping but now are not; erase warped stars
        {
            globals()->gWarpStars = FALSE;
            for ( i = 0; i < kScrollStarNum; i++)
            {
                if ( star->speed != kNoStar)
                {
                    if ( star->age > 0)
                    {
                        if ( star->age > 1)
                        {
                        } else star->age = 2;
                    } else
                    {
                        star->age = 1;
                    }
                }
                star++;
            }
        }
    } else // we're warping now
    {
        globals()->gWarpStars = TRUE;

        for ( i = 0; i < kScrollStarNum; i++)
        {
            if ( star->speed != kNoStar)
            {
                if ( star->age > 0)
                {
                    if ( star->age > 1)
                    {
                    } else star->age = 2;
                } else
                {
                    star->age = 1;
                }
            }
            star++;
        }
    }

    for ( i = kSparkStarOffset; i < kAllStarNum; i++)
    {
        if ( star->speed != kNoStar)
        {
            if ( star->age > 0)
            {
                if ( !((star->location.h < CLIP_LEFT) || ( star->location.h >= CLIP_RIGHT) ||
                    ( star->location.v < CLIP_TOP) ||  ( star->location.v >= CLIP_BOTTOM)))
                {
                }
            } else
            {
                star->speed = kNoStar;
            }

            if ( !((star->oldLocation.h < CLIP_LEFT) || ( star->oldLocation.h >= CLIP_RIGHT) ||
                ( star->oldLocation.v < CLIP_TOP) ||  ( star->oldLocation.v >= CLIP_BOTTOM)))
            {
            }

        }
        star++;
    }

    globals()->gLastClipBottom = CLIP_BOTTOM;
#endif
}

#ifdef kUseYeOldeBeams
void DrawAllBeams( void)

{
    spaceObjectType *anObject, *bObject;
    baseObjectType  *baseObject;
    short           i;
    Rect        bounds;
    long            h;

    bounds.left = CLIP_LEFT;
    bounds.right = CLIP_RIGHT;
    bounds.top = CLIP_TOP;
    bounds.bottom = CLIP_BOTTOM;

    anObject = *gSpaceObjectData;

    for ( i = 0; i < kMaxSpaceObject; i++)
    {
        if (( anObject->attributes & kIsBeam) && ( anObject->active))//(( anObject->active) || ( anObject->frame.beam.killMe)))
        {
                baseObject = anObject->baseType;

//              anObject->frame.beam.thisLocation.right = anObject->frame.beam.thisLocation.left;
//              anObject->frame.beam.thisLocation.bottom = anObject->frame.beam.thisLocation.top;

                h = ( anObject->frame.beam.lastGlobalLocation.h - gGlobalCorner.h) * gAbsoluteScale;
                h >>= SHIFT_SCALE;
                anObject->frame.beam.thisLocation.right = h + CLIP_LEFT;
                h = (anObject->frame.beam.lastGlobalLocation.v - gGlobalCorner.v) * gAbsoluteScale;
                h >>= SHIFT_SCALE; /*+ CLIP_TOP*/;
                anObject->frame.beam.thisLocation.bottom = h;

                h = ( anObject->location.h - gGlobalCorner.h) * gAbsoluteScale;
                h >>= SHIFT_SCALE;
                anObject->frame.beam.thisLocation.left = h + CLIP_LEFT;
                h = (anObject->location.v - gGlobalCorner.v) * gAbsoluteScale;
                h >>= SHIFT_SCALE; /*+ CLIP_TOP*/;
                anObject->frame.beam.thisLocation.top = h;

                if (( anObject->frame.beam.lastGlobalLocation.h == anObject->location.h) &&
                    ( anObject->frame.beam.lastGlobalLocation.v == anObject->location.v))
                {
                    anObject->frame.beam.thisLocation.right = anObject->frame.beam.thisLocation.left;
                    anObject->frame.beam.thisLocation.bottom = anObject->frame.beam.thisLocation.top;
                }
                if ((baseObject->frame.beam.color != 0) &&
                    ( !anObject->frame.beam.killMe) && ( anObject->active != kObjectToBeFreed))
                {
                    DrawNateLine( *thePixMapHandle, &bounds, anObject->frame.beam.thisLocation.left,
                                anObject->frame.beam.thisLocation.top,
                                anObject->frame.beam.thisLocation.right,
                                anObject->frame.beam.thisLocation.bottom,
                                gNatePortLeft << 2, gNatePortTop, baseObject->frame.beam.color);
                } else
                {
/*                  anObject->active = kObjectAvailable;
                    anObject->nextNearObject = anObject->nextFarObject = nil;
                    if ( anObject->previousObject != nil)
                    {
                        bObject = (spaceObjectType *)anObject->previousObject;
                        bObject->nextObject = anObject->nextObject;
                        bObject->nextObjectNumber = anObject->nextObjectNumber;
                    }
                    if ( anObject->nextObject != nil)
                    {
                        bObject = (spaceObjectType *)anObject->nextObject;
                        bObject->previousObject = anObject->previousObject;
                        bObject->previousObjectNumber = anObject->previousObjectNumber;
                    }
                    if ( gRootObject == anObject)
                    {
                        gRootObject = (spaceObjectType *)anObject->nextObject;
                        gRootObjectNumber = anObject->nextObjectNumber;
                    }
                    anObject->nextObject = nil;
                    anObject->nextObjectNumber = -1;
                    anObject->previousObject = nil;
                    anObject->previousObjectNumber = -1;
*/              }
                if ( baseObject->frame.beam.color != 0)
                {
                    DrawNateLine( *thePixMapHandle, &bounds, anObject->frame.beam.lastLocation.left,
                                anObject->frame.beam.lastLocation.top,
                                anObject->frame.beam.lastLocation.right,
                                anObject->frame.beam.lastLocation.bottom,
                                gNatePortLeft << 2, gNatePortTop,
                                BLACK);
                }
                anObject->frame.beam.lastLocation = anObject->frame.beam.thisLocation;


        }
        anObject++;
    }
}
#endif

void Reset3DStars( Point center, Rect *bounds)

{
    short           i;
    scrollStarType  *star;
    smallFixedType  f;

    star = *globals()->gScrollStarData;
    for ( i = 0; i < kAllStarNum; i++)
    {
        star->oldOldLocation.h = star->oldLocation.h = star->location.h =
            Randomize( bounds->right - bounds->left) + bounds->left;
        star->oldOldLocation.v = star->oldLocation.v = star->location.v =
            Randomize( bounds->bottom - bounds->top) + bounds->top;
        star->motionFraction.h = star->motionFraction.v = 0;

        star->speed = kMediumStarSpeed;

        f = mFloatToFixed( k3DStarInitialCo);
        star->velocity.h = mLongToFixed( star->location.h - center.h);
        star->velocity.h = mMultiplyFixed( f, star->velocity.h);
        star->velocity.v = mLongToFixed( star->location.v - center.v);
        star->velocity.v = mMultiplyFixed( f, star->velocity.v);

        star->velocity.h =  star->location.h - center.h;
        star->velocity.v =  star->location.v - center.v;
        star->age = Randomize( kMax3DStarAge);

        star++;
    }
}

void Move3DStars( Point center, long byUnits, Rect *bounds)

{
    short           i;
    scrollStarType  *star;
    smallFixedType  f;
    long            h, v, l = byUnits * 32;

    star = *globals()->gScrollStarData;
    for ( i = 0; i < kAllStarNum; i++)
    {
        if ( star->speed != kNoStar)
        {
            star->age += l;

//          f = mFloatToFixed( 1.2);
//          star->velocity.h = mMultiplyFixed( star->velocity.h, f);
//          star->velocity.v = mMultiplyFixed( star->velocity.v, f);

            f = star->velocity.h * byUnits;
            f = mMultiplyFixed( star->velocity.h, star->age);
//          f >>= 3L;
            star->motionFraction.h += f; //star->velocity.h * byUnits;
            f = star->velocity.v * byUnits;
            f = mMultiplyFixed( star->velocity.v, star->age);
//          f >>= 3L;
            star->motionFraction.v += f; //star->velocity.v * byUnits;

            if ( star->motionFraction.h >= 0)
                h = ( star->motionFraction.h + kFixedPlusPointFive) >> kFixedBitShiftNumber;
            else
                h = (( star->motionFraction.h - kFixedPlusPointFive) >> kFixedBitShiftNumber) + 1;
            star->location.h += h;
            star->motionFraction.h -= mLongToFixed(h);

            if ( star->motionFraction.v >= 0)
                v = ( star->motionFraction.v + kFixedPlusPointFive) >> kFixedBitShiftNumber;
            else
                v = (( star->motionFraction.v - kFixedPlusPointFive) >> kFixedBitShiftNumber) + 1;
            star->location.v += v;
            star->motionFraction.v -= mLongToFixed(v);

            if ( (!((star->location.h < bounds->left) || ( star->location.h >= bounds->right) ||
                ( star->location.v < bounds->top) ||  ( star->location.v >= bounds->bottom))) ||
                (!((star->oldLocation.h < bounds->left) || ( star->oldLocation.h >= bounds->right) ||
                ( star->oldLocation.v < bounds->top) ||  ( star->oldLocation.v >= bounds->bottom))))
            {
            } else
            {
                star->location.h = Randomize( bounds->right - bounds->left) + bounds->left;
                star->location.v = Randomize( bounds->bottom - bounds->top) + bounds->top;
                star->motionFraction.h = star->motionFraction.v = 0;

                star->speed = kMediumStarSpeed;
                star->velocity.h = mLongToFixed( star->location.h - center.h);
                f = mFloatToFixed( k3DStarInitialCo);
                star->velocity.h = mMultiplyFixed( f, star->velocity.h);
                star->velocity.v = mLongToFixed( star->location.v - center.v);
                f = mFloatToFixed( k3DStarInitialCo);
                star->velocity.v = mMultiplyFixed( f, star->velocity.v);
                star->age = 0;
                star->velocity.h =  star->location.h - center.h;
                star->velocity.v =  star->location.v - center.v;
            }
        }
        star++;
    }
}

void Draw3DStars( Boolean warp, Rect *bounds, PixMapHandle destMap)

{
    short           i;
    scrollStarType  *star;
    unsigned char   *dByte, color, shade;
    long            rowBytes, clipAge;
    transColorType  *transColor;

    mGetRowBytes( rowBytes, *destMap);
    star = *globals()->gScrollStarData;

    for ( i = 0; i < kAllStarNum; i++)
    {
        if ( star->speed != kNoStar)
        {
            if (( !warp))
            {
                if ( !((star->oldLocation.h < bounds->left) || ( star->oldLocation.h >= bounds->right) ||
                    ( star->oldLocation.v < bounds->top) ||  ( star->oldLocation.v >= bounds->bottom)))
                {
                    mSetNatePixel ( dByte, rowBytes, star->oldLocation.h, star->oldLocation.v,
                        0, 0, *destMap, 0xff);
                }
            } else
            {
                if ( star->speed != 1)
                {
                    DrawNateLine( *destMap, bounds, star->oldOldLocation.h,
                                star->oldOldLocation.v,
                                star->oldLocation.h,
                                star->oldLocation.v,
                                0, 0, 0xff);
                }
            }

        }
        star++;
    }

    star = *globals()->gScrollStarData;

    for ( i = 0; i < kAllStarNum; i++)
    {
        if ( star->speed != kNoStar)
        {
            if ( star->age > 0)
            {
                clipAge = star->age;
                if ( clipAge >= kMax3DStarAge)
                    clipAge = kMax3DStarAge - 1;
                shade =  clipAge >> k3DStarAgeToShadeShift;
                if ( shade < 1) color = 0xff;
                else if ( shade > 15) color = 0x00;
                else
                {
                    mGetTranslateColorShade( GRAY, shade, color, transColor);
                }
                if ( !warp)
                {
                    if ( !((star->location.h < bounds->left) || ( star->location.h >= bounds->right) ||
                        ( star->location.v < bounds->top) ||  ( star->location.v >= bounds->bottom)))
                    {
                        mSetNatePixel( dByte, rowBytes, star->location.h, star->location.v, 0,
                            0, *destMap, color);
                    }
                } else
                {
                    DrawNateLine( *destMap, bounds, star->location.h,
                                star->location.v,
                                star->oldLocation.h,
                                star->oldLocation.v,
                                0, 0, color);
                }
            } else
            {
//              star->speed = kNoStar;
            }

        }
        star++;
    }
}

void Show3DStars( Boolean warp, Rect *bounds, PixMapHandle sourceMap)

{
    short           i;
    scrollStarType  *star;
    unsigned char   *dByte, *sByte;
    long            srowBytes, drowBytes;

    mGetRowBytes( srowBytes, *sourceMap);
    mGetRowBytes( drowBytes, *thePixMapHandle);
    star = *globals()->gScrollStarData;

    for ( i = 0; i < kAllStarNum; i++)
    {
        if ( star->speed != kNoStar)
        {
            if ( star->age > 0)
            {
                if ( !warp)
                {
                    if ( !((star->location.h < bounds->left) || ( star->location.h >= bounds->right) ||
                        ( star->location.v < bounds->top) ||  ( star->location.v >= bounds->bottom)))
                    {
                        mGetNatePixel( dByte, srowBytes, star->location.h, star->location.v, 0,
                            0, *sourceMap);
                        mSetNatePixel( sByte, drowBytes, star->location.h, star->location.v, gNatePortLeft << 2,
                                gNatePortTop, *thePixMapHandle, *dByte);
                    }
                } else
                {
                    CopyNateLine( *sourceMap, *thePixMapHandle, bounds, star->location.h,
                            star->location.v, star->oldLocation.h, star->oldLocation.v,
                            gNatePortLeft << 2, gNatePortTop);
                }
            } else
            {
//              star->speed = kNoStar;
            }
            if (( !warp))
            {
                if ( !((star->oldLocation.h < bounds->left) || ( star->oldLocation.h >= bounds->right) ||
                    ( star->oldLocation.v < bounds->top) ||  ( star->oldLocation.v >= bounds->bottom)))
                {
                    mGetNatePixel( dByte, srowBytes, star->oldLocation.h, star->oldLocation.v, 0,
                        0, *sourceMap);
                    mSetNatePixel( sByte, drowBytes, star->oldLocation.h, star->oldLocation.v, gNatePortLeft << 2,
                            gNatePortTop, *thePixMapHandle, *dByte);
                    star->speed = kNoStar;
                }
            } else
            {
                if ( star->speed != 1)
                {
                    CopyNateLine( *sourceMap, *thePixMapHandle, bounds, star->oldOldLocation.h,
                            star->oldOldLocation.v, star->oldLocation.h, star->oldLocation.v,
                            gNatePortLeft << 2, gNatePortTop);
                }
                if ( star->speed == 2)
                {
                    star->speed = kSlowStarSpeed;
                } else if ( star->speed == 1)
                {
                    star->speed = kNoStar;
                }
            }

        }
        star++;
    }
}

void CorrectScrollStarObject( Handle data)
{
#pragma unused( data)

    if ( globals()->gScrollStarNumber >= 0)
        gScrollStarObject = *gSpaceObjectData +
            globals()->gScrollStarNumber;
    else
        gScrollStarObject = nil;
}

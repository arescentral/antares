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

#include "AresGlobalType.hpp"
#include "ColorTranslation.hpp"
#include "Debug.hpp"
#include "Error.hpp"
#include "MathMacros.hpp"
#include "Motion.hpp"
#include "NateDraw.hpp"
#include "OffscreenGWorld.hpp"
#include "SpaceObject.hpp"
#include "Randomize.hpp"

using sfz::scoped_array;

namespace antares {

const int kScrollStarNum = 125;
const int kSparkStarNum = 125;
const int kAllStarNum = kScrollStarNum + kSparkStarNum;
const int kSparkStarOffset = kScrollStarNum;

const int kMinimumStarSpeed = 1;
const int kMaximumStarSpeed = 3;
const int kStarSpeedSpread = (kMaximumStarSpeed - kMinimumStarSpeed + 1);

const Fixed kSlowStarFraction = 0x00000080;  // this is 1/2 so could be made to asr 1 ( >> 1)
const Fixed kMediumStarFraction = 0x000000c0;  // this is .75 could be made to 1 - asr 2 ( 1 - ( >> 2))
const Fixed kFastStarFraction = 0x00000100;  // this is 1 so could be ignored

const uint8_t kStarColor = GRAY;

extern long             gNatePortLeft, gNatePortTop, gAbsoluteScale,
                        CLIP_LEFT, CLIP_TOP, CLIP_RIGHT, CLIP_BOTTOM,
                        gPlayScreenWidth, gPlayScreenHeight;
extern scoped_array<spaceObjectType> gSpaceObjectData;
extern PixMap*          gActiveWorld;
extern PixMap*          gOffWorld;

spaceObjectType *gScrollStarObject = nil;   // this object is also used for the radar center

inline StarSpeed RandomStarSpeed() {
    return static_cast<StarSpeed>(Randomize(kStarSpeedSpread) + kMinimumStarSpeed);
}

int InitScrollStars() {
    scrollStarType  *star;
    short           i;

    globals()->gScrollStarData.reset(new scrollStarType[kAllStarNum]);
    star = globals()->gScrollStarData.get();
    for (i = 0; i < kAllStarNum; i++) {
        star->speed = kNoStar;
        star++;
    }
    globals()->gLastClipBottom = CLIP_BOTTOM;

    return kNoError;
}

void ResetScrollStars(long which) {
    short           i;
    scrollStarType  *star;
    spaceObjectType *centerObject = gSpaceObjectData.get() + which;

    gScrollStarObject = centerObject;
    globals()->gScrollStarNumber = which;

    if ( gScrollStarObject != nil)
    {
        star = globals()->gScrollStarData.get();
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
}

void MakeNewSparks(
        long sparkNum, long sparkSpeed, smallFixedType maxVelocity, unsigned char color,
        Point* location) {
    long            i, whichSpark = kSparkStarOffset;
    scrollStarType  *spark = globals()->gScrollStarData.get() + kSparkStarOffset;

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

void PrepareToMoveScrollStars() {
    short           i;
    scrollStarType  *star;

    star = globals()->gScrollStarData.get();
    for ( i = 0; i < kAllStarNum; i++)
    {
        star->oldOldLocation.h = star->oldLocation.h;
        star->oldOldLocation.v = star->oldLocation.v;
        star->oldLocation.h = star->location.h;
        star->oldLocation.v = star->location.v;

        star++;
    }
}

void MoveScrollStars(const long byUnits) {
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

    star = globals()->gScrollStarData.get();
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
                    fail("invalid value of star->speed.");
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
                        fail("invalid value of star->speed.");
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
}

void DrawScrollStars(bool warp) {
    short           i;
    scrollStarType  *star;
    RgbColor        slowColor, mediumColor, fastColor, *color;
    Rect        bounds, lastBounds;

#pragma unused( warp)

    GetRGBTranslateColorShade(&slowColor, kStarColor, MEDIUM);
    GetRGBTranslateColorShade(&mediumColor, kStarColor, LIGHT);
    GetRGBTranslateColorShade(&fastColor, kStarColor, LIGHTER);

    bounds.left = lastBounds.left = CLIP_LEFT;
    bounds.top = lastBounds.top = CLIP_TOP;
    bounds.bottom = CLIP_BOTTOM;
    bounds.right = lastBounds.right = CLIP_RIGHT;
    lastBounds.bottom = globals()->gLastClipBottom;

    star = globals()->gScrollStarData.get();

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
                        gOffWorld->set(star->location.h, star->location.v, *color);
                    }
                    if ((( star->location.h != star->oldLocation.h) ||
                    ( star->location.v != star->oldLocation.v)) &&
                    (( star->oldLocation.h >= CLIP_LEFT) && ( star->oldLocation.v >= CLIP_TOP)
                        && ( star->oldLocation.h < CLIP_RIGHT) && ( star->oldLocation.v < CLIP_BOTTOM)))
                    {
                        gOffWorld->set(star->oldLocation.h, star->oldLocation.v, RgbColor::kBlack);
                    }
                }
                star++;
            }
        } else // we were warping but now are not; erase warped stars
        {
//          globals()->gWarpStars = false;
            for ( i = 0; i < kScrollStarNum; i++)
            {
                if ( star->speed != kNoStar)
                {
                    if ( star->age > 0)
                    {
                        if ( star->age > 1)
                        {
                            DrawNateLine(gOffWorld, lastBounds, star->oldOldLocation.h,
                                        star->oldOldLocation.v,
                                        star->oldLocation.h,
                                        star->oldLocation.v,
                                        0,
                                        0, RgbColor::kBlack);
                        }/* else star->age = 2;*/
                    } else
                    {
                        DrawNateLine(gOffWorld, lastBounds, star->oldOldLocation.h,
                                    star->oldOldLocation.v,
                                    star->oldLocation.h,
                                    star->oldLocation.v,
                                    0,
                                    0, RgbColor::kBlack);
                        /*star->age = 1;*/
                    }
                }
                star++;
            }
        }
    } else // we're warping now
    {
//      globals()->gWarpStars = true;

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
                        DrawNateLine(gOffWorld, lastBounds, star->oldOldLocation.h,
                                    star->oldOldLocation.v,
                                    star->oldLocation.h,
                                    star->oldLocation.v,
                                    0,
                                    0, RgbColor::kBlack);
                    }/* else star->age = 2;*/
                    DrawNateLine(gOffWorld, bounds, star->oldLocation.h,
                                star->oldLocation.v,
                                star->location.h,
                                star->location.v,
                                0,
                                0, *color);
                } else
                {
                    DrawNateLine(gOffWorld, lastBounds, star->oldOldLocation.h,
                                star->oldOldLocation.v,
                                star->oldLocation.h,
                                star->oldLocation.v,
                                0,
                                0, RgbColor::kBlack);
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
                    GetRGBTranslateColorShade(&slowColor, star->color, (star->age >> kSparkAgeToShadeShift) + 1);
                    gOffWorld->set(star->location.h, star->location.v, slowColor);
                }
            } else
            {
//              star->speed = kNoStar;
            }

            if ( !((star->oldLocation.h < CLIP_LEFT) || ( star->oldLocation.h >= CLIP_RIGHT) ||
                ( star->oldLocation.v < CLIP_TOP) ||  ( star->oldLocation.v >= CLIP_BOTTOM)))
            {
                gOffWorld->set(star->oldLocation.h, star->oldLocation.v, RgbColor::kBlack);
            }

        }
        star++;
    }
}

void ShowScrollStars(bool warp) {
    short           i;
    scrollStarType  *star;
    unsigned char   slowColor, mediumColor, fastColor, *color;
    transColorType  *transColor;
    Rect        bounds, lastBounds;

#pragma unused( warp)

    mGetTranslateColorShade( kStarColor, MEDIUM, slowColor, transColor);
    mGetTranslateColorShade( kStarColor, LIGHT, mediumColor, transColor);
    mGetTranslateColorShade( kStarColor, LIGHTER, fastColor, transColor);

    bounds.left = lastBounds.left = CLIP_LEFT;
    bounds.top = lastBounds.top = CLIP_TOP;
    bounds.bottom = CLIP_BOTTOM;
    bounds.right = lastBounds.right = CLIP_RIGHT;
    lastBounds.bottom = globals()->gLastClipBottom;

    star = globals()->gScrollStarData.get();

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
                        const RgbColor& c = gOffWorld->get(star->location.h, star->location.v);
                        gActiveWorld->set(star->location.h, star->location.v, c);
                    }
                    if ((( star->location.h != star->oldLocation.h) ||
                    ( star->location.v != star->oldLocation.v)) &&
                    (( star->oldLocation.h >= CLIP_LEFT) && ( star->oldLocation.v >= CLIP_TOP)
                        && ( star->oldLocation.h < CLIP_RIGHT) && ( star->oldLocation.v < CLIP_BOTTOM)))
                    {
                        const RgbColor& c = gOffWorld->get(
                                star->oldLocation.h, star->oldLocation.v);
                        gActiveWorld->set(star->oldLocation.h, star->oldLocation.v, c);
                    }
                }
                star++;
            }
        } else // we were warping but now are not; erase warped stars
        {
            globals()->gWarpStars = false;
            for ( i = 0; i < kScrollStarNum; i++)
            {
                if ( star->speed != kNoStar)
                {
                    if ( star->age > 0)
                    {
                        if ( star->age > 1)
                        {
                            CopyNateLine( gOffWorld, gActiveWorld, lastBounds, star->oldOldLocation.h,
                                    star->oldOldLocation.v, star->oldLocation.h, star->oldLocation.v,
                                    gNatePortLeft << 2, gNatePortTop);
                        } else star->age = 2;
                    } else
                    {
                        CopyNateLine( gOffWorld, gActiveWorld, lastBounds, star->oldOldLocation.h,
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
        globals()->gWarpStars = true;

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
                        CopyNateLine( gOffWorld, gActiveWorld, lastBounds, star->oldOldLocation.h,
                                star->oldOldLocation.v, star->oldLocation.h, star->oldLocation.v,
                                gNatePortLeft << 2, gNatePortTop);
                    } else star->age = 2;
                    CopyNateLine( gOffWorld, gActiveWorld, bounds, star->oldLocation.h,
                            star->oldLocation.v, star->location.h, star->location.v,
                            gNatePortLeft << 2, gNatePortTop);
                } else
                {
                    CopyNateLine( gOffWorld, gActiveWorld, lastBounds, star->oldOldLocation.h,
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
                    const RgbColor& c = gOffWorld->get(star->location.h, star->location.v);
                    gActiveWorld->set(star->location.h, star->location.v, c);
                }
            } else
            {
                star->speed = kNoStar;
            }

            if ( !((star->oldLocation.h < CLIP_LEFT) || ( star->oldLocation.h >= CLIP_RIGHT) ||
                ( star->oldLocation.v < CLIP_TOP) ||  ( star->oldLocation.v >= CLIP_BOTTOM)))
            {
                const RgbColor& c = gOffWorld->get(star->oldLocation.h, star->oldLocation.v);
                gActiveWorld->set(star->oldLocation.h, star->oldLocation.v, c);
            }

        }
        star++;
    }

    globals()->gLastClipBottom = CLIP_BOTTOM;
}

}  // namespace antares

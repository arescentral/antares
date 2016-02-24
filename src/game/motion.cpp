// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// Copyright (C) 2008-2012 The Antares Authors
//
// This file is part of Antares, a tactical space combat game.
//
// Antares is free software: you can redistribute it and/or modify it
// under the terms of the Lesser GNU General Public License as published
// by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Antares is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with Antares.  If not, see http://www.gnu.org/licenses/

#include "game/motion.hpp"

#include <sfz/sfz.hpp>

#include "data/space-object.hpp"
#include "drawing/color.hpp"
#include "drawing/pix-table.hpp"
#include "drawing/sprite-handling.hpp"
#include "game/action.hpp"
#include "game/admiral.hpp"
#include "game/globals.hpp"
#include "game/non-player-ship.hpp"
#include "game/player-ship.hpp"
#include "game/space-object.hpp"
#include "lang/defines.hpp"
#include "math/macros.hpp"
#include "math/random.hpp"
#include "math/rotation.hpp"
#include "math/special.hpp"
#include "math/units.hpp"
#include "sound/fx.hpp"

using sfz::Exception;
using std::unique_ptr;

namespace antares {

const int32_t kProximitySuperSize           = 16;   // number of cUnits in cSuperUnits
const int32_t kProximityGridDataLength      = kProximitySuperSize * kProximitySuperSize;
const int32_t kProximityUnitAndModulo       = kProximitySuperSize - 1;  // & a int32_t with this and get modulo kCollisionSuperSize
const int32_t kProximityWidthMultiply       = 4;    // for speed = * kCollisionSuperSize

const int32_t kCollisionUnitBitShift        = 7;    // >> 7 = / 128
const int32_t kCollisionSuperUnitBitShift   = 11;   // >> 11 = / 2048
const int32_t kCollisionSuperExtraShift     = kCollisionSuperUnitBitShift - kCollisionUnitBitShift;

const int32_t kDistanceUnitBitShift         = 11;   // >> 14L = / 2048
const int32_t kDistanceSuperUnitBitShift    = 15;   // >> 18L = / 262144
const int32_t kDistanceSuperExtraShift      = kDistanceSuperUnitBitShift - kDistanceUnitBitShift;
const int32_t kDistanceUnitExtraShift       = 0;    // speed from kCollisionSuperUnitBitShift to kDistanceUnitBitShift

const int32_t kNoDir = -1;

const int32_t kConsiderDistanceAttributes = (
        kCanCollide | kCanBeHit | kIsDestination | kCanThink | kConsiderDistance | kCanBeEvaded |
        kIsHumanControlled | kIsRemote);

const uint32_t kThinkiverseTopLeft       = (kUniversalCenter - (2 * 65534)); // universe for thinking or owned objects
const uint32_t kThinkiverseBottomRight   = (kUniversalCenter + (2 * 65534));

// kAdjacentUnits encodes the following set of locations relative to the
// center:
//
//     # # #
//     # 0 1
//     2 3 4
//
// The point of this is, if we iterate through a grid such as
// gProximityGrid, and at each cell, check the cell at each of these
// relative locations, we will make a pairwise comparison between all
// adjacent cells exactly once.
//
// InitMotion() turns the relative locations to absolute indices, and
// keeps that information in unitsToCheck[k].adjacentUnit.  If the
// relative location would be outside the 16x16 grid of gProximityGrid,
// then superOffset points into the adjacent grid, which is also the
// same grid in a way I have yet to comprehend.
const static Point kAdjacentUnits[] = {{0, 0}, {1, 0}, {-1, 1}, {0, 1}, {1, 1}};

ANTARES_GLOBAL coordPointType          gGlobalCorner;
static ANTARES_GLOBAL unique_ptr<proximityUnitType[]> gProximityGrid;

// for the macro mRanged, time is assumed to be a int32_t game ticks, velocity a fixed, result int32_t, scratch fixed
static inline void mRange(int32_t& result, int32_t time, Fixed velocity, Fixed& scratch) {
    scratch = mLongToFixed( time);
    scratch = mMultiplyFixed (scratch, velocity);
    result = mFixedToLong( scratch);
}

Size center_scale() {
    return {
        (play_screen.width() / 2) * SCALE_SCALE,
        (play_screen.height() / 2) * SCALE_SCALE,
    };
}

void InitMotion() {
    gProximityGrid.reset(new proximityUnitType[kProximityGridDataLength]);

    // initialize the proximityGrid & set up the needed lookups (see Notebook 2 p.34)
    for (int y = 0; y < kProximitySuperSize; y++) {
        for (int x = 0; x < kProximitySuperSize; x++) {
            proximityUnitType* p = &gProximityGrid[(y << kProximityWidthMultiply) + x];
            p->nearObject = p->farObject = SpaceObject::none();
            for (int i = 0; i < kUnitsToCheckNumber; i++) {
                int32_t ux = x;
                int32_t uy = y;
                int32_t sx = 0, sy = 0;

                ux += kAdjacentUnits[i].h;
                if (ux < 0) {
                    ux += kProximitySuperSize;
                    sx--;
                } else if (ux >= kProximitySuperSize) {
                    ux -= kProximitySuperSize;
                    sx++;
                }

                uy += kAdjacentUnits[i].v;
                if (uy < 0) {
                    uy += kProximitySuperSize;
                    sy--;
                } else if (uy >= kProximitySuperSize) {
                    uy -= kProximitySuperSize;
                    sy++;
                }
                p->unitsToCheck[i].adjacentUnit = (uy << kProximityWidthMultiply) + ux;
                p->unitsToCheck[i].superOffset.h = sx;
                p->unitsToCheck[i].superOffset.v = sy;
            }
        }
    }
}

void ResetMotionGlobals() {
    gGlobalCorner.h = gGlobalCorner.v = 0;
    g.closest = Handle<SpaceObject>(0);
    g.farthest = Handle<SpaceObject>(0);

    for (int i = 0; i < kProximityGridDataLength; i++) {
        gProximityGrid[i].nearObject = gProximityGrid[i].farObject = SpaceObject::none();
    }
}

void MotionCleanup() {
    gProximityGrid.reset();
}

static void move(Handle<SpaceObject> anObject) {
    if ((anObject->maxVelocity == 0) && !(anObject->attributes & kCanTurn)) {
        return;
    }

    if (anObject->attributes & kCanTurn) {
        anObject->turnFraction += anObject->turnVelocity;

        int32_t h;
        if (anObject->turnFraction >= 0) {
            h = more_evil_fixed_to_long(anObject->turnFraction + mFloatToFixed(0.5));
        } else {
            h = more_evil_fixed_to_long(anObject->turnFraction - mFloatToFixed(0.5)) + 1;
        }
        anObject->direction += h;
        anObject->turnFraction -= mLongToFixed(h);

        while (anObject->direction >= ROT_POS) {
            anObject->direction -= ROT_POS;
        }
        while (anObject->direction < 0) {
            anObject->direction += ROT_POS;
        }
    }

    if (anObject->thrust != 0) {
        Fixed fa, fb, useThrust;
        if (anObject->thrust > 0) {
            // get the goal dh & dv
            GetRotPoint(&fa, &fb, anObject->direction);

            // multiply by max velocity
            if (anObject->presenceState == kWarpingPresence) {
                fa = mMultiplyFixed(fa, anObject->presence.warping);
                fb = mMultiplyFixed(fb, anObject->presence.warping);
            } else if (anObject->presenceState == kWarpOutPresence) {
                fa = mMultiplyFixed(fa, anObject->presence.warp_out);
                fb = mMultiplyFixed(fb, anObject->presence.warp_out);
            } else {
                fa = mMultiplyFixed(anObject->maxVelocity, fa);
                fb = mMultiplyFixed(anObject->maxVelocity, fb);
            }

            // the difference between our actual vector and our goal vector is our new vector
            fa = fa - anObject->velocity.h;
            fb = fb - anObject->velocity.v;
            useThrust = anObject->thrust;
        } else {
            fa = -anObject->velocity.h;
            fb = -anObject->velocity.v;
            useThrust = -anObject->thrust;
        }

        // get the angle of our new vector
        int16_t angle = ratio_to_angle(fa, fb);

        // get the maxthrust of new vector
        Fixed fh, fv;
        GetRotPoint(&fh, &fv, angle);

        fh = mMultiplyFixed(useThrust, fh);
        fv = mMultiplyFixed(useThrust, fv);

        // if our new vector excedes our max thrust, it must be limited
        if (fh < 0) {
            if (fa < fh) {
                fa = fh;
            }
        } else {
            if (fa > fh) {
                fa = fh;
            }
        }

        if (fv < 0) {
            if (fb < fv) {
                fb = fv;
            }
        } else {
            if (fb > fv) {
                fb = fv;
            }
        }

        anObject->velocity.h += fa;
        anObject->velocity.v += fb;
    }

    anObject->motionFraction.h += anObject->velocity.h;
    anObject->motionFraction.v += anObject->velocity.v;

    int32_t h;
    if (anObject->motionFraction.h >= 0) {
        h = more_evil_fixed_to_long(anObject->motionFraction.h + mFloatToFixed(0.5));
    } else {
        h = more_evil_fixed_to_long(anObject->motionFraction.h - mFloatToFixed(0.5)) + 1;
    }
    anObject->location.h -= h;
    anObject->motionFraction.h -= mLongToFixed(h);

    int32_t v;
    if (anObject->motionFraction.v >= 0) {
        v = more_evil_fixed_to_long(anObject->motionFraction.v + mFloatToFixed(0.5));
    } else {
        v = more_evil_fixed_to_long(anObject->motionFraction.v - mFloatToFixed(0.5)) + 1;
    }
    anObject->location.v -= v;
    anObject->motionFraction.v -= mLongToFixed(v);
}

static void bounce(Handle<SpaceObject> anObject) {
    // check to see if it's out of bounds
    if (!(anObject->attributes & kDoesBounce)) {
        if ((anObject->location.h < kThinkiverseTopLeft) ||
            (anObject->location.v < kThinkiverseTopLeft) ||
            (anObject->location.h > kThinkiverseBottomRight) ||
            (anObject->location.v > kThinkiverseBottomRight)) {
            anObject->active = kObjectToBeFreed;
        }
    } else {
        if (anObject->location.h < kThinkiverseTopLeft) {
            anObject->location.h = kThinkiverseTopLeft;
            anObject->velocity.h = -anObject->velocity.h;
        } else if (anObject->location.h > kThinkiverseBottomRight) {
            anObject->location.h = kThinkiverseBottomRight;
            anObject->velocity.h = -anObject->velocity.h;
        }
        if (anObject->location.v < kThinkiverseTopLeft) {
            anObject->location.v = kThinkiverseTopLeft;
            anObject->velocity.v = -anObject->velocity.v;
        } else if (anObject->location.v > kThinkiverseBottomRight) {
            anObject->location.v = kThinkiverseBottomRight;
            anObject->velocity.v = -anObject->velocity.v;
        }
    }
}

static void animate(Handle<SpaceObject> anObject) {
    auto& base_anim = anObject->base->frame.animation;
    if (base_anim.frameSpeed == 0) {
        return;
    }

    auto& space_anim = anObject->frame.animation;
    space_anim.thisShape += space_anim.frameDirection * space_anim.frameSpeed;
    if (anObject->attributes & kAnimationCycle) {
        int shape_num = (base_anim.lastShape - base_anim.firstShape) + 1;
        while (space_anim.thisShape > base_anim.lastShape) {
            space_anim.thisShape -= shape_num;
        }
        while (space_anim.thisShape < base_anim.firstShape) {
            space_anim.thisShape += shape_num;
        }
    } else if ((space_anim.thisShape > base_anim.lastShape) ||
            (space_anim.thisShape < base_anim.firstShape)) {
        anObject->active = kObjectToBeFreed;
        space_anim.thisShape = base_anim.lastShape;
    }
}

static void move_beam(Handle<SpaceObject> anObject) {
    if (!anObject->frame.beam.get()) {
        throw Exception("Unexpected error: a beam appears to be missing.");
    }
    auto& beam = *anObject->frame.beam;

    beam.objectLocation = anObject->location;
    if ((beam.beamKind == eStaticObjectToObjectKind) ||
            (beam.beamKind == eBoltObjectToObjectKind)) {
        if (beam.toObject.get()) {
            auto target = beam.toObject;
            if (target->active && (target->id == beam.toObjectID)) {
                anObject->location = beam.objectLocation = target->location;
            } else {
                anObject->active = kObjectToBeFreed;
            }
        }

        if (beam.fromObject.get()) {
            auto target = beam.fromObject;
            if (target->active && (target->id == beam.fromObjectID)) {
                beam.lastGlobalLocation = beam.lastApparentLocation = target->location;
            } else {
                anObject->active = kObjectToBeFreed;
            }
        }
    } else if ((beam.beamKind == eStaticObjectToRelativeCoordKind) ||
            (beam.beamKind == eBoltObjectToRelativeCoordKind)) {
        if (beam.fromObject.get()) {
            auto target = beam.fromObject;
            if (target->active && (target->id == beam.fromObjectID)) {
                beam.lastGlobalLocation = beam.lastApparentLocation = target->location;
                anObject->location.h = beam.objectLocation.h =
                    target->location.h + beam.toRelativeCoord.h;
                anObject->location.v = beam.objectLocation.v =
                    target->location.v + beam.toRelativeCoord.v;
            } else {
                anObject->active = kObjectToBeFreed;
            }
        }
    }
}

static void update_static(Handle<SpaceObject> anObject, int unitsToDo) {
    auto& sprite = *anObject->sprite;
    if (anObject->hitState != 0) {
        anObject->hitState -= unitsToDo << 2L;
        if (anObject->hitState <= 0) {
            anObject->hitState = 0;
            sprite.style = spriteNormal;
            sprite.styleData = 0;
        } else {
            // we know it has sprite
            sprite.style = spriteColor;
            sprite.styleColor = GetRGBTranslateColor(anObject->shieldColor);
            sprite.styleData = anObject->hitState;
        }
    } else {
        if (anObject->cloakState > 0) {
            if (anObject->cloakState < kCloakOnStateMax) {
                anObject->runTimeFlags |= kIsCloaked;
                anObject->cloakState += unitsToDo << 2L;
                if (anObject->cloakState > kCloakOnStateMax) {
                    anObject->cloakState = kCloakOnStateMax;
                }
            }
            sprite.style = spriteColor;
            sprite.styleColor = RgbColor::kClear;
            sprite.styleData = anObject->cloakState;
            if (anObject->owner == g.admiral) {
                sprite.styleData -= sprite.styleData >> 2;
            }
        } else if (anObject->cloakState < 0) {
            anObject->cloakState += unitsToDo << 2L;
            if (anObject->cloakState >= 0) {
                anObject->runTimeFlags &= ~kIsCloaked;
                anObject->cloakState = 0;
                sprite.style = spriteNormal;
            } else {
                sprite.style = spriteColor;
                sprite.styleColor = RgbColor::kClear;
                sprite.styleData = -anObject->cloakState;
                if (anObject->owner == g.admiral) {
                    sprite.styleData -= sprite.styleData >> 2;
                }
            }
        }
    }
}

void MoveSpaceObjects(const int32_t unitsToDo) {
    if (unitsToDo == 0) {
        return;
    }

    for (int32_t jl = 0; jl < unitsToDo; jl++) {
        for (Handle<SpaceObject> anObject = g.root; anObject.get(); anObject = anObject->nextObject) {
            if (anObject->active != kObjectInUse) {
                continue;
            }

            move(anObject);
            bounce(anObject);
            if (anObject->attributes & kIsSelfAnimated) {
                animate(anObject);
            } else if (anObject->attributes & kIsBeam) {
                move_beam(anObject);
            }
        }
    }

    if (g.ship->active) {
        gGlobalCorner.h = g.ship->location.h - (center_scale().width / gAbsoluteScale);
        gGlobalCorner.v = g.ship->location.v - (center_scale().height / gAbsoluteScale);
    }

// !!!!!!!!
// nothing below can effect any object actions (expire actions get executed)
// (but they can effect objects thinking)
// !!!!!!!!
    for (Handle<SpaceObject> anObject = g.root; anObject.get(); anObject = anObject->nextObject) {
        if (anObject->active != kObjectInUse) {
            continue;
        } else if ((anObject->attributes & kIsBeam) || !anObject->sprite.get()) {
            continue;
        }
        auto& sprite = *anObject->sprite;

        int32_t h = (anObject->location.h - gGlobalCorner.h) * gAbsoluteScale;
        h >>= SHIFT_SCALE;
        if ((h > -kSpriteMaxSize) && (h < kSpriteMaxSize)) {
            sprite.where.h = h + viewport.left;
        } else {
            sprite.where.h = -kSpriteMaxSize;
        }

        int32_t v = (anObject->location.v - gGlobalCorner.v) * gAbsoluteScale;
        v >>= SHIFT_SCALE;
        if ((v > -kSpriteMaxSize) && (v < kSpriteMaxSize)) {
            sprite.where.v = v;
        } else {
            sprite.where.v = -kSpriteMaxSize;
        }

        update_static(anObject, unitsToDo);

        auto baseObject = anObject->base;
        if (anObject->attributes & kIsSelfAnimated) {
            if (baseObject->frame.animation.frameSpeed != 0) {
                sprite.whichShape = more_evil_fixed_to_long(anObject->frame.animation.thisShape);
            }
        } else if (anObject->attributes & kShapeFromDirection) {
            int16_t angle = anObject->direction;
            mAddAngle(angle, baseObject->frame.rotation.rotRes >> 1);
            sprite.whichShape = angle / baseObject->frame.rotation.rotRes;
        }
    }
}

static void age_object(const Handle<SpaceObject>& o) {
    if (o->age >= 0) {
        o->age -= 3;
        if (o->age < 0) {
            if (!(o->baseType->expireDontDie)) {
                o->active = kObjectToBeFreed;
            }

            exec(o->baseType->expire, o, SpaceObject::none(), NULL);
        }
    }
}

static void activate_object(const Handle<SpaceObject>& o) {
    if (o->periodicTime > 0) {
        o->periodicTime--;
        if (o->periodicTime <= 0) {
            exec(o->baseType->activate, o, SpaceObject::none(), NULL);
            o->periodicTime =
                o->baseType->activatePeriod
                + o->randomSeed.next(o->baseType->activatePeriodRange);
        }
    }
}

static void calc_misc() {
    // set up player info so we can find closest ship (for scaling)
    uint64_t farthestDist = 0;
    uint64_t closestDist = 0x7fffffffffffffffull;
    g.closest = g.farthest = Handle<SpaceObject>(0);

    // reset the collision grid
    for (int32_t i = 0; i < kProximityGridDataLength; i++) {
        gProximityGrid[i].nearObject = gProximityGrid[i].farObject = SpaceObject::none();
    }

    for (auto aObject = g.root; aObject.get(); aObject = aObject->nextObject) {
        if (!aObject->active) {
            if (g.ship.get() && g.ship->active) {
                aObject->distanceFromPlayer = 0x7fffffffffffffffull;
            }
        }
    }

    for (auto aObject = g.root; aObject.get(); aObject = aObject->nextObject) {
        if (!aObject->active) {
            continue;
        }

        age_object(aObject);
        if (!aObject->active) {
            continue;
        }

        activate_object(aObject);
        if (!aObject->active) {
            continue;
        }

        // Mark closest and farthest object relative to player, for zooming.
        if (g.ship.get() && g.ship->active) {
            if (aObject->attributes & kAppearOnRadar) {
                uint64_t hdiff = ABS<int>(g.ship->location.h - aObject->location.h);
                uint64_t vdiff = ABS<int>(g.ship->location.v - aObject->location.v);
                uint64_t dist = (vdiff * vdiff) + (hdiff * hdiff);
                aObject->distanceFromPlayer = dist;
                if ((dist < closestDist) && (aObject != g.ship)) {
                    if (!((globals()->gZoomMode == kNearestFoeZoom) &&
                          (aObject->owner == g.ship->owner))) {
                        closestDist = dist;
                        g.closest = aObject;
                    }
                }
                if (dist > farthestDist) {
                    farthestDist = dist;
                    g.farthest = aObject;
                }
            }
        }

        if (aObject->attributes & kConsiderDistanceAttributes) {
            aObject->localFriendStrength = aObject->baseType->offenseValue;
            aObject->localFoeStrength = 0;
            aObject->closestObject = SpaceObject::none();
            aObject->closestDistance = kMaximumRelevantDistanceSquared;
            aObject->absoluteBounds.right = aObject->absoluteBounds.left = 0;

            const auto& loc = aObject->location;
            {
                int32_t x1 = (loc.h >> kCollisionUnitBitShift) & kProximityUnitAndModulo;
                int32_t x2 = loc.h >> kCollisionSuperUnitBitShift;
                int32_t y1 = (loc.v >> kCollisionUnitBitShift) & kProximityUnitAndModulo;
                int32_t y2 = loc.v >> kCollisionSuperUnitBitShift;
                auto& proximityObject = gProximityGrid[(y1 << kProximityWidthMultiply) + x1];
                aObject->nextNearObject = proximityObject.nearObject;
                proximityObject.nearObject = aObject;
                aObject->collisionGrid = {x2, y2};
            }

            {
                int32_t x3 = (loc.h >> kDistanceUnitBitShift) & kProximityUnitAndModulo;
                int32_t x4 = loc.h >> kDistanceSuperUnitBitShift;
                int32_t y3 = (loc.v >> kDistanceUnitBitShift) & kProximityUnitAndModulo;
                int32_t y4 = loc.v >> kDistanceSuperUnitBitShift;
                auto& proximityObject = gProximityGrid[(y3 << kProximityWidthMultiply) + x3];
                aObject->nextFarObject = proximityObject.farObject;
                proximityObject.farObject = aObject;
                aObject->distanceGrid = {x4, y4};
            }

            if (!(aObject->attributes & kIsDestination)) {
                aObject->seenByPlayerFlags = 0x80000000;
            }
            aObject->runTimeFlags &= ~kIsHidden;

            if (aObject->sprite.get()) {
                aObject->sprite->tinySize = aObject->tinySize;
            }
        }
    }
}

// Collision uses inclusive rect bounds for historical reasons.
static bool inclusive_intersect(Rect x, Rect y) {
    ++x.right;
    ++x.bottom;;
    ++y.right;
    ++y.bottom;
    return x.intersects(y);
}

static int mClipCode(int x, int y, const Rect& bounds) {
    return ((x < bounds.left) << 3)
        | ((x >= bounds.right) << 2)
        | ((y < bounds.top) << 1)
        | (y >= bounds.bottom);
}

static bool beam_intersects(const Handle<SpaceObject>& beam, const Handle<SpaceObject>& target) {
    if (beam->active == kObjectToBeFreed) {
        return false;
    }

    Point start(beam->location.h, beam->location.v);
    Point end(beam->frame.beam->lastGlobalLocation.h, beam->frame.beam->lastGlobalLocation.v);

    //
    // Determine if the line segment defined by `{start, end}` passes
    // through the rect `target->absoluteBounds`.
    //
    // Imagine dividing space up into nine areas based on the rect:
    //
    //    1010 | 0010 | 0110
    //   ------########------
    //    1000 # 0000 # 0100
    //   ------########------
    //    1001 | 0001 | 0101
    //
    // The binary numbers above are the clip code, which we compute for
    // both `start` and `end`. There are a few cases we need to worry
    // about:
    //
    //   1.  Either `start_clip` or `end_clip` is zero. In this case,
    //       one endpoint lies within the rect, so the line definitely
    //       definitely intersects it, and we run the collision.
    //
    //   2.  `start_clip & end_clip` is non-zero. In this case, both the
    //       start and end points exceed some dimension of the rect;
    //       they are both above, below, left, or right of it. In this
    //       case, the line doesn't intersect, and we bail.
    //
    //   3.  `start_clip & end_clip` is zero, but neither of them is
    //       itself zero. This has two sub-cases, and we now have to
    //       figure out which it is.
    //
    // In this illustration, both A and B have clip code 1010, and both
    // C and D have clip code 0101. In order to figure out if AC and BD
    // intersect the rect, we walk forward the start points.
    //
    //     B A.|      |
    //      \  E.     |
    //       \ | ˙.   |
    //        \|   ˙. |
    //         F     ˙.
    //         |\     |˙.
    //   ------##H#####--G.-----
    //         #  \   #    ˙.
    //   ------####\###------˙.-
    //         |    \ |        C
    //         |     \|
    //         |      \
    //         |      |\
    //         |      | D
    //
    // First, we move them forward horizontally, to E or F. These have
    // clip code 0010, so we've made progress, but don't yet have an
    // answer. Then we walk them forward vertically, to G and H. G has
    // clip code 0100, so both C and G are right of the rect; we bail. H
    // has clip code 0000; it's in the rect, and we run the collision.
    //
    int16_t end_clip = mClipCode(end.h, end.v, target->absoluteBounds);
    if (!end_clip) {
        return true;
    }

    while (true) {
        int16_t start_clip = mClipCode(start.h, start.v, target->absoluteBounds);
        if (!start_clip) {
            return true;
        } else if (start_clip & end_clip) {
            return false;
        }

        int32_t xd = end.h - start.h;
        int32_t yd = end.v - start.v;
        if (start_clip & 8) {
            start.v += yd * (target->absoluteBounds.left - start.h) / xd;
            start.h = target->absoluteBounds.left;
        } else if (start_clip & 4) {
            start.v += yd * (target->absoluteBounds.right - 1 - start.h) / xd;
            start.h = target->absoluteBounds.right - 1;
        } else if (start_clip & 2) {
            start.h += xd * (target->absoluteBounds.top - start.v) / yd;
            start.v = target->absoluteBounds.top;
        } else if (start_clip & 1) {
            start.h += xd * (target->absoluteBounds.bottom - 1 - start.v) / yd;
            start.v = target->absoluteBounds.bottom - 1;
        }
    }
}

// Set absoluteBounds on all objects.
static void calc_bounds() {
    for (auto o = g.root; o.get(); o = o->nextObject) {
        if ((o->absoluteBounds.left >= o->absoluteBounds.right) && o->sprite.get()) {
            const NatePixTable::Frame& frame
                = o->sprite->table->at(o->sprite->whichShape);

            Size size = {
                (frame.width() * o->naturalScale) >> SHIFT_SCALE,
                (frame.height() * o->naturalScale) >> SHIFT_SCALE,
            };
            Point corner = {
                -((frame.center().h * o->naturalScale) >> SHIFT_SCALE),
                -((frame.center().v * o->naturalScale) >> SHIFT_SCALE),
            };

            o->absoluteBounds.left = o->location.h + corner.h;
            o->absoluteBounds.right = o->absoluteBounds.left + size.width;
            o->absoluteBounds.top = o->location.v + corner.v;
            o->absoluteBounds.bottom = o->absoluteBounds.top + size.height;
        }
    }
}

// Call HitObject() and CorrectPhysicalSpace() for all colliding pairs of objects.
static void calc_impacts() {
    for (int32_t i = 0; i < kProximityGridDataLength; i++) {
        const auto& cell = gProximityGrid[i];
        for (auto aObject = cell.nearObject; aObject.get(); aObject = aObject->nextNearObject) {
            for (int32_t k = 0; k < kUnitsToCheckNumber; k++) {
                Handle<SpaceObject> bObject = aObject->nextNearObject;
                Point super = aObject->collisionGrid;
                if (k > 0) {
                    const auto& adj = cell.unitsToCheck[k];
                    bObject = gProximityGrid[adj.adjacentUnit].nearObject;
                    super.offset(adj.superOffset.h, adj.superOffset.v);
                }

                if ((super.h < 0) || (super.v < 0)) {
                    continue;
                }

                for (; bObject.get(); bObject = bObject->nextNearObject) {
                    // this'll be true even ONLY if BOTH objects are not non-physical dest object
                    if (!((bObject->attributes | aObject->attributes) & kCanCollide)
                            || !((bObject->attributes | aObject->attributes) & kCanBeHit)
                            || (bObject->collisionGrid != super)) {
                        continue;
                    }

                    if (aObject->owner == bObject->owner) {
                        continue;
                    }

                    if (aObject->attributes & bObject->attributes & kIsBeam) {
                        // no reason beams can't intersect, but the
                        // code we have now won't handle it.
                        continue;
                    } else if (aObject->attributes & kIsBeam) {
                        if (beam_intersects(aObject, bObject)) {
                            HitObject(bObject, aObject);
                        }
                        continue;
                    } else if (bObject->attributes & kIsBeam) {
                        if (beam_intersects(bObject, aObject)) {
                            HitObject(aObject, bObject);
                        }
                        continue;
                    }

                    if (inclusive_intersect(aObject->absoluteBounds, bObject->absoluteBounds)) {
                        HitObject(aObject, bObject);
                        HitObject(bObject, aObject);
                        CorrectPhysicalSpace(aObject, bObject);
                    }
                }
            }
        }
    }
}

// Sets the following properties on objects:
//   * closestObject
//   * closestDistance
//   * localFriendStrength
//   * localFoeStrength
// Also sets seenByPlayerFlags and kIsHidden based on object proximity.
static void calc_locality() {
    for (int32_t i = 0; i < kProximityGridDataLength; i++) {
        const auto& cell = gProximityGrid[i];
        for (auto aObject = cell.farObject; aObject.get(); aObject = aObject->nextFarObject) {
            for (int32_t k = 0; k < kUnitsToCheckNumber; k++) {
                Handle<SpaceObject> bObject = aObject->nextFarObject;
                Point super = aObject->distanceGrid;
                if (k > 0) {
                    const auto& adj = cell.unitsToCheck[k];
                    bObject = gProximityGrid[adj.adjacentUnit].farObject;
                    super.offset(adj.superOffset.h, adj.superOffset.v);
                }
                if ((super.h < 0) || (super.v < 0)) {
                    continue;
                }

                for (; bObject.get(); bObject = bObject->nextFarObject) {
                    if (bObject->distanceGrid != super) {
                        continue;
                    }
                    if ((bObject->owner != aObject->owner)
                            && ((bObject->attributes & kCanThink)
                                || (bObject->attributes & kRemoteOrHuman)
                                || (bObject->attributes & kHated))
                            && ((aObject->attributes & kCanThink)
                                || (aObject->attributes & kRemoteOrHuman)
                                || (aObject->attributes & kHated))) {
                        uint32_t x_dist = ABS<int>(bObject->location.h - aObject->location.h);
                        uint32_t y_dist = ABS<int>(bObject->location.v - aObject->location.v);
                        uint32_t dist;
                        if ((x_dist > kMaximumRelevantDistance)
                                || (y_dist > kMaximumRelevantDistance)) {
                            dist = kMaximumRelevantDistanceSquared;
                        } else {
                            dist = (y_dist * y_dist) + (x_dist * x_dist);
                        }

                        if (dist < kMaximumRelevantDistanceSquared) {
                            aObject->seenByPlayerFlags |= bObject->myPlayerFlag;
                            bObject->seenByPlayerFlags |= aObject->myPlayerFlag;

                            if (bObject->attributes & kHideEffect) {
                                aObject->runTimeFlags |= kIsHidden;
                            }

                            if (aObject->attributes & kHideEffect) {
                                bObject->runTimeFlags |= kIsHidden;
                            }
                        }

                        if (aObject->engages(*bObject)) {
                            if ((dist < aObject->closestDistance) && (bObject->attributes & kPotentialTarget)) {
                                aObject->closestDistance = dist;
                                aObject->closestObject = bObject;
                            }
                        }

                        if (bObject->engages(*aObject)) {
                            if ((dist < bObject->closestDistance) && (aObject->attributes & kPotentialTarget)) {
                                bObject->closestDistance = dist;
                                bObject->closestObject = aObject;
                            }
                        }

                        bObject->localFoeStrength += aObject->localFriendStrength;
                        bObject->localFriendStrength += aObject->localFoeStrength;
                    } else if (k == 0) {
                        if (aObject->owner != bObject->owner) {
                            bObject->localFoeStrength += aObject->localFriendStrength;
                            bObject->localFriendStrength += aObject->localFoeStrength;
                        } else {
                            bObject->localFoeStrength += aObject->localFoeStrength;
                            bObject->localFriendStrength += aObject->localFriendStrength;
                        }
                    }
                }
            }
        }
    }
}

static void calc_visibility() {
    // here, it doesn't matter in what order we step through the table
    const uint32_t seen_by_me = 1ul << g.admiral.number();

    for (auto o: SpaceObject::all()) {
        if (o->active == kObjectToBeFreed) {
            o->free();
        } else if (o->active) {
            if ((o->attributes & kConsiderDistanceAttributes)
                    && (!(o->attributes & kIsDestination))) {
                if (o->runTimeFlags & kIsCloaked) {
                    o->seenByPlayerFlags = 0;
                } else if (!(o->runTimeFlags & kIsHidden)) {
                    o->seenByPlayerFlags = 0xffffffff;
                }
                o->seenByPlayerFlags |= o->myPlayerFlag;
                if (!(o->seenByPlayerFlags & seen_by_me)
                        && o->sprite.get()) {
                    o->sprite->tinySize = 0;
                }
            }
        }
    }
}

static void update_last_beam_locations() {
    for (auto o: SpaceObject::all()) {
        if (o->active == kObjectInUse) {
            if (o->attributes & kIsBeam) {
                o->frame.beam->lastGlobalLocation = o->location;
            }
        }
    }
}

void CollideSpaceObjects() {
    calc_misc();
    calc_bounds();
    calc_impacts();
    calc_locality();
    calc_visibility();
    update_last_beam_locations();
}

// CorrectPhysicalSpace-- takes 2 objects that are colliding and moves them back 1
//  bresenham-style step at a time to their previous locations or until they don't
//  collide.  For keeping objects which occupy space from occupying the
//  same space.

void CorrectPhysicalSpace(Handle<SpaceObject> aObject, Handle<SpaceObject> bObject) {
    if (!(bObject->attributes & aObject->attributes & kOccupiesSpace)) {
        return;  // no need; at least one object doesn't actually occupy space.
    } else if (bObject->owner == aObject->owner) {
        return;  // the collision changed the owner of one object, e.g. a flagpod.
    }

    int32_t    ah, av, ad, bh, bv, bd, adir = kNoDir, bdir = kNoDir,
            h, v;
    fixedPointType  tvel;
    Fixed           force, totalMass, tfix;
    int16_t         angle;

    // calculate the new velocities
    force = ( bObject->velocity.h - aObject->velocity.h);
    force = mMultiplyFixed( force, force);
    totalMass = ( bObject->velocity.v - aObject->velocity.v);
    totalMass = mMultiplyFixed( totalMass, totalMass);
    force += totalMass;
    force = lsqrt( force);  // tvel = force
    ah = bObject->location.h - aObject->location.h;
    av = bObject->location.v - aObject->location.v;

    angle = ratio_to_angle(ah, av);
    totalMass = aObject->baseType->mass + bObject->baseType->mass;  // svel = total mass
    tfix = aObject->baseType->mass;
    tfix = mMultiplyFixed( tfix, force);
    if ( totalMass == 0) tfix = -1;
    else
    {
        tfix = mDivideFixed( tfix, totalMass);
    }
    tfix += aObject->maxVelocity >> 1;
    GetRotPoint(&tvel.h, &tvel.v, angle);
    tvel.h = mMultiplyFixed( tfix, tvel.h);
    tvel.v = mMultiplyFixed( tfix, tvel.v);
//  tvel.h = mMultiplyFixed( aObject->baseType->maxVelocity, tvel.h);
//  tvel.v = mMultiplyFixed( aObject->baseType->maxVelocity, tvel.v);
    aObject->velocity.v = tvel.v;
    aObject->velocity.h = tvel.h;

    mAddAngle( angle, 180);
    tfix = bObject->baseType->mass;
    tfix = mMultiplyFixed( tfix, force);
    if ( totalMass == 0) tfix = -1;
    else
    {
        tfix = mDivideFixed( tfix, totalMass);
    }
    tfix += bObject->maxVelocity >> 1;
    GetRotPoint(&tvel.h, &tvel.v, angle);
    tvel.h = mMultiplyFixed( tfix, tvel.h);
    tvel.v = mMultiplyFixed( tfix, tvel.v);
//  tvel.h = mMultiplyFixed( bObject->baseType->maxVelocity, tvel.h);
//  tvel.v = mMultiplyFixed( bObject->baseType->maxVelocity, tvel.v);
    bObject->velocity.v = tvel.v;
    bObject->velocity.h = tvel.h;

    ah = aObject->location.h - aObject->absoluteBounds.left;
    ad = aObject->absoluteBounds.right - aObject->location.h;
    av = aObject->location.v - aObject->absoluteBounds.top;
    adir = aObject->absoluteBounds.bottom - aObject->location.v;

    bh = bObject->location.h - bObject->absoluteBounds.left;
    bd = bObject->absoluteBounds.right - bObject->location.h;
    bv = bObject->location.v - bObject->absoluteBounds.top;
    bdir = bObject->absoluteBounds.bottom - bObject->location.v;

    if ( (aObject->velocity.h || aObject->velocity.v || bObject->velocity.h ||
        bObject->velocity.v))
    {
        while ((!(( aObject->absoluteBounds.right   <   bObject->absoluteBounds.left) ||
                (   aObject->absoluteBounds.left    >   bObject->absoluteBounds.right) ||
                (   aObject->absoluteBounds.bottom  <   bObject->absoluteBounds.top) ||
                (   aObject->absoluteBounds.top     >   bObject->absoluteBounds.bottom))))
        {
            aObject->motionFraction.h += aObject->velocity.h;
            aObject->motionFraction.v += aObject->velocity.v;

            if ( aObject->motionFraction.h >= 0)
                h = more_evil_fixed_to_long(aObject->motionFraction.h + mFloatToFixed(0.5));
            else
                h = more_evil_fixed_to_long(aObject->motionFraction.h - mFloatToFixed(0.5)) + 1;
            aObject->location.h -= h;
            aObject->motionFraction.h -= mLongToFixed(h);

            if ( aObject->motionFraction.v >= 0)
                v = more_evil_fixed_to_long(aObject->motionFraction.v + mFloatToFixed(0.5));
            else
                v = more_evil_fixed_to_long(aObject->motionFraction.v - mFloatToFixed(0.5)) + 1;
            aObject->location.v -= v;
            aObject->motionFraction.v -= mLongToFixed(v);

            bObject->motionFraction.h += bObject->velocity.h;
            bObject->motionFraction.v += bObject->velocity.v;

            if ( bObject->motionFraction.h >= 0)
                h = more_evil_fixed_to_long(bObject->motionFraction.h + mFloatToFixed(0.5));
            else
                h = more_evil_fixed_to_long(bObject->motionFraction.h - mFloatToFixed(0.5)) + 1;
            bObject->location.h -= h;
            bObject->motionFraction.h -= mLongToFixed(h);

            if ( bObject->motionFraction.v >= 0)
                v = more_evil_fixed_to_long(bObject->motionFraction.v + mFloatToFixed(0.5));
            else
                v = more_evil_fixed_to_long(bObject->motionFraction.v - mFloatToFixed(0.5)) + 1;
            bObject->location.v -= v;
            bObject->motionFraction.v -= mLongToFixed(v);

            aObject->absoluteBounds.left = aObject->location.h - ah;
            aObject->absoluteBounds.right = aObject->location.h + ad;
            aObject->absoluteBounds.top = aObject->location.v - av;
            aObject->absoluteBounds.bottom = aObject->location.v + adir;

            bObject->absoluteBounds.left = bObject->location.h - bh;
            bObject->absoluteBounds.right = bObject->location.h + bd;
            bObject->absoluteBounds.top = bObject->location.v - bv;
            bObject->absoluteBounds.bottom = bObject->location.v + bdir;
        }
    }
}

}  // namespace antares

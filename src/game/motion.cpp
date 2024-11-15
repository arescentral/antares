// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// Copyright (C) 2008-2017 The Antares Authors
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

#include "data/base-object.hpp"
#include "drawing/color.hpp"
#include "drawing/pix-table.hpp"
#include "drawing/sprite-handling.hpp"
#include "game/action.hpp"
#include "game/admiral.hpp"
#include "game/globals.hpp"
#include "game/non-player-ship.hpp"
#include "game/player-ship.hpp"
#include "game/space-object.hpp"
#include "game/vector.hpp"
#include "lang/defines.hpp"
#include "math/macros.hpp"
#include "math/random.hpp"
#include "math/rotation.hpp"
#include "math/special.hpp"
#include "math/units.hpp"
#include "sound/fx.hpp"

using std::unique_ptr;

namespace antares {

enum {
    PROXIMITY_GRID_WIDTH = 16,
    PROXIMITY_GRID_AREA  = 256,  // WIDTH * WIDTH
    PROXIMITY_GRID_MASK  = 0xf,
    PROXIMITY_GRID_SHIFT = 4,
};

const int32_t kConsiderDistanceAttributes =
        (kCanCollide | kCanBeHit | kIsDestination | kCanThink | kConsiderDistance | kCanBeEvaded |
         kIsPlayerShip);

const int32_t kThinkiverseCenter = 0x40000000;
const int32_t kThinkiverseRadius = SECTOR_MAX / 2;
const Rect    kThinkiverse{
        // universe for thinking or owned objects
        kThinkiverseCenter - kThinkiverseRadius, kThinkiverseCenter - kThinkiverseRadius,
        kThinkiverseCenter + kThinkiverseRadius, kThinkiverseCenter + kThinkiverseRadius};

// kAdjacentUnits encodes the following set of locations relative to the
// center:
//
//     # # #
//     # 0 1
//     2 3 4
//
// The point of this is, if we iterate through a grid such as
// {near,far}_objects, and at each cell, check the cell at each of these
// relative locations, we will make a pairwise comparison between all
// adjacent cells exactly once.
//
// make_adjacent_cells turns the relative locations to absolute indices,
// and keeps that information in kAdjacentCells[k].  If the relative
// location would be outside the 16x16 grid of near_object, then
// super_offset gets added to the object in question’s super location.
// An object is only really in a cell if the super location matches too.
const static Point kAdjacentUnits[] = {{0, 0}, {1, 0}, {-1, 1}, {0, 1}, {1, 1}};

struct AdjacentCells {
    struct AdjacentCell {
        uint8_t index_offset;  // the normal adjacent unit
        Point   super_offset;  // the offset of the super unit (for wrap-around)
    };

    static const int size  = 5;
    using AdjacentCellList = AdjacentCell[AdjacentCells::size];

    AdjacentCellList at[PROXIMITY_GRID_AREA];
};

static int proximity_index(int32_t x, int32_t y) { return (y << PROXIMITY_GRID_SHIFT) + x; }

static AdjacentCells make_adjacent_cells() {
    // initialize the proximityGrid & set up the needed lookups (see Notebook 2 p.34)
    AdjacentCells a;
    for (int y = 0; y < PROXIMITY_GRID_WIDTH; y++) {
        for (int x = 0; x < PROXIMITY_GRID_WIDTH; x++) {
            int   i     = proximity_index(x, y);
            auto* cells = a.at[i];
            for (int i = 0; i < AdjacentCells::size; i++) {
                auto*   cell         = &cells[i];
                int32_t ux           = x;
                int32_t uy           = y;
                cell->super_offset.h = cell->super_offset.v = 0;

                ux += kAdjacentUnits[i].h;
                if (ux < 0) {
                    ux += PROXIMITY_GRID_WIDTH;
                    cell->super_offset.h = -1;
                } else if (ux >= PROXIMITY_GRID_WIDTH) {
                    ux -= PROXIMITY_GRID_WIDTH;
                    cell->super_offset.h = +1;
                }

                uy += kAdjacentUnits[i].v;
                if (uy < 0) {
                    uy += PROXIMITY_GRID_WIDTH;
                    cell->super_offset.v = -1;
                } else if (uy >= PROXIMITY_GRID_WIDTH) {
                    uy -= PROXIMITY_GRID_WIDTH;
                    cell->super_offset.v = +1;
                }

                cells[i].index_offset = proximity_index(ux, uy);
            }
        }
    }
    return a;
}

static const AdjacentCells kAdjacentCells = make_adjacent_cells();

ANTARES_GLOBAL ScaledScreen scaled_screen;

static void correct_physical_space(SpaceObject* a, SpaceObject* b);

Point scale_to_viewport(Point p) {
    return Point{scale_by(p.h - scaled_screen.bounds.left, scaled_screen.scale) + viewport().left,
                 scale_by(p.v - scaled_screen.bounds.top, scaled_screen.scale) + viewport().top};
}

void ResetMotionGlobals() {
    scaled_screen.bounds = Rect{};
    scaled_screen.scale  = SCALE_SCALE;
    g.closest            = Handle<SpaceObject>(0);
    g.farthest           = Handle<SpaceObject>(0);
}

static void move_object(SpaceObject* o) {
    if ((o->maxVelocity == Fixed::zero()) && !(o->attributes & kCanTurn)) {
        return;
    }

    if (o->attributes & kCanTurn) {
        o->turnFraction += o->turnVelocity;

        int32_t h;
        if (o->turnFraction >= Fixed::zero()) {
            h = more_evil_fixed_to_long(o->turnFraction + Fixed::from_float(0.5));
        } else {
            h = more_evil_fixed_to_long(o->turnFraction - Fixed::from_float(0.5)) + 1;
        }
        o->direction += h;
        o->turnFraction -= Fixed::from_long(h);

        while (o->direction >= ROT_POS) {
            o->direction -= ROT_POS;
        }
        while (o->direction < 0) {
            o->direction += ROT_POS;
        }
    }

    if (o->thrust != Fixed::zero()) {
        Fixed fa, fb, useThrust;
        if (o->thrust > Fixed::zero()) {
            // get the goal dh & dv
            GetRotPoint(&fa, &fb, o->direction);

            // multiply by max velocity
            if (o->presenceState == kWarpingPresence) {
                fa = (fa * o->presence.warping);
                fb = (fb * o->presence.warping);
            } else if (o->presenceState == kWarpOutPresence) {
                fa = (fa * o->presence.warp_out);
                fb = (fb * o->presence.warp_out);
            } else {
                fa = (o->maxVelocity * fa);
                fb = (o->maxVelocity * fb);
            }

            // the difference between our actual vector and our goal vector is our new vector
            fa        = fa - o->velocity.h;
            fb        = fb - o->velocity.v;
            useThrust = o->thrust;
        } else {
            fa        = -o->velocity.h;
            fb        = -o->velocity.v;
            useThrust = -o->thrust;
        }

        // get the angle of our new vector
        int16_t angle = ratio_to_angle(fa, fb);

        // get the maxthrust of new vector
        Fixed fh, fv;
        GetRotPoint(&fh, &fv, angle);

        fh = (useThrust * fh);
        fv = (useThrust * fv);

        // if our new vector excedes our max thrust, it must be limited
        if (fh < Fixed::zero()) {
            if (fa < fh) {
                fa = fh;
            }
        } else {
            if (fa > fh) {
                fa = fh;
            }
        }

        if (fv < Fixed::zero()) {
            if (fb < fv) {
                fb = fv;
            }
        } else {
            if (fb > fv) {
                fb = fv;
            }
        }

        o->velocity.h += fa;
        o->velocity.v += fb;
    }

    o->motionFraction.h += o->velocity.h;
    o->motionFraction.v += o->velocity.v;

    int32_t h;
    if (o->motionFraction.h >= Fixed::zero()) {
        h = more_evil_fixed_to_long(o->motionFraction.h + Fixed::from_float(0.5));
    } else {
        h = more_evil_fixed_to_long(o->motionFraction.h - Fixed::from_float(0.5)) + 1;
    }
    o->location.h -= h;
    o->motionFraction.h -= Fixed::from_long(h);

    int32_t v;
    if (o->motionFraction.v >= Fixed::zero()) {
        v = more_evil_fixed_to_long(o->motionFraction.v + Fixed::from_float(0.5));
    } else {
        v = more_evil_fixed_to_long(o->motionFraction.v - Fixed::from_float(0.5)) + 1;
    }
    o->location.v -= v;
    o->motionFraction.v -= Fixed::from_long(v);
}

static void bounce_object(SpaceObject* o) {
    if (!(o->attributes & kDoesBounce)) {
        if (!kThinkiverse.contains(o->location)) {
            o->active = kObjectToBeFreed;
        }
        return;
    }

    if (o->location.h < kThinkiverse.left) {
        o->location.h = kThinkiverse.left;
        o->velocity.h = -o->velocity.h;
    } else if (o->location.h >= kThinkiverse.right) {
        o->location.h = kThinkiverse.right - 1;
        o->velocity.h = -o->velocity.h;
    }
    if (o->location.v < kThinkiverse.top) {
        o->location.v = kThinkiverse.top;
        o->velocity.v = -o->velocity.v;
    } else if (o->location.v >= kThinkiverse.bottom) {
        o->location.v = kThinkiverse.bottom - 1;
        o->velocity.v = -o->velocity.v;
    }
}

static void animate_object(SpaceObject* o) {
    auto& base_anim = o->base->animation;
    if (base_anim->speed == Fixed::zero()) {
        return;
    }

    auto& space_anim = o->frame.animation;
    space_anim.thisShape += static_cast<int>(space_anim.direction) * space_anim.speed;
    if (o->attributes & kAnimationCycle) {
        Fixed shape_num = base_anim->frames.range();
        while (space_anim.thisShape >= base_anim->frames.end) {
            space_anim.thisShape -= shape_num;
        }
        while (space_anim.thisShape < base_anim->frames.begin) {
            space_anim.thisShape += shape_num;
        }
    } else if (
            (space_anim.thisShape >= base_anim->frames.end) ||
            (space_anim.thisShape < base_anim->frames.begin)) {
        o->active            = kObjectToBeFreed;
        space_anim.thisShape = base_anim->frames.end - Fixed::from_val(1);
    }
}

static void move_vector(SpaceObject* o) {
    if (!o->frame.vector.get()) {
        throw std::runtime_error("Unexpected error: a vector appears to be missing.");
    }
    auto& vector = *o->frame.vector;

    vector.objectLocation = o->location;
    if (!vector.is_ray) {
        return;
    } else if (!vector.to_coord) {
        if (vector.toObject.get()) {
            auto target = vector.toObject;
            if (target->active && (target->id == vector.toObjectID)) {
                o->location = vector.objectLocation = target->location;
            } else {
                o->active = kObjectToBeFreed;
            }
        }

        if (vector.fromObject.get()) {
            auto target = vector.fromObject;
            if (target->active && (target->id == vector.fromObjectID)) {
                vector.lastGlobalLocation = vector.lastApparentLocation = target->location;
            } else {
                o->active = kObjectToBeFreed;
            }
        }
    } else if (vector.to_coord) {
        if (vector.fromObject.get()) {
            auto target = vector.fromObject;
            if (target->active && (target->id == vector.fromObjectID)) {
                vector.lastGlobalLocation = vector.lastApparentLocation = target->location;
                o->location.h                                           = vector.objectLocation.h =
                        target->location.h + vector.toRelativeCoord.h;
                o->location.v = vector.objectLocation.v =
                        target->location.v + vector.toRelativeCoord.v;
            } else {
                o->active = kObjectToBeFreed;
            }
        }
    }
}

static void update_static(SpaceObject* o, ticks unitsToDo) {
    auto& sprite = *o->sprite;
    if (o->hitState != 0) {
        o->hitState -= unitsToDo.count() << 2L;
        if (o->hitState <= 0) {
            o->hitState      = 0;
            sprite.style     = spriteNormal;
            sprite.styleData = 0;
        } else {
            // we know it has sprite
            sprite.style      = spriteColor;
            sprite.styleColor = o->shieldColor.value_or(RgbColor::clear());
            sprite.styleData  = o->hitState;
        }
    } else {
        if (o->cloakState > 0) {
            if (o->cloakState < kCloakOnStateMax) {
                o->runTimeFlags |= kIsCloaked;
                o->cloakState += unitsToDo.count() << 2L;
                if (o->cloakState > kCloakOnStateMax) {
                    o->cloakState = kCloakOnStateMax;
                }
            }
            sprite.style      = spriteColor;
            sprite.styleColor = RgbColor::clear();
            sprite.styleData  = o->cloakState;
            if (o->owner == g.admiral) {
                sprite.styleData -= sprite.styleData >> 2;
            }
        } else if (o->cloakState < 0) {
            o->cloakState += unitsToDo.count() << 2L;
            if (o->cloakState >= 0) {
                o->runTimeFlags &= ~kIsCloaked;
                o->cloakState = 0;
                sprite.style  = spriteNormal;
            } else {
                sprite.style      = spriteColor;
                sprite.styleColor = RgbColor::clear();
                sprite.styleData  = -o->cloakState;
                if (o->owner == g.admiral) {
                    sprite.styleData -= sprite.styleData >> 2;
                }
            }
        }
    }
}

void MoveSpaceObjects(const ticks unitsToDo) {
    if (unitsToDo == ticks(0)) {
        return;
    }

    for (ticks jl = ticks(0); jl < unitsToDo; jl++) {
        SpaceObject* o = nullptr;
        for (Handle<SpaceObject> o_handle = g.root; (o = o_handle.get());
             o_handle                     = o->nextObject) {
            if (o->active != kObjectInUse) {
                continue;
            }

            move_object(o);
            bounce_object(o);
            if (o->attributes & kIsSelfAnimated) {
                animate_object(o);
            } else if (o->attributes & kIsVector) {
                move_vector(o);
            }
        }
    }

    if (g.ship.get() && g.ship->active) {
        Size scale{((play_screen().width() / 2) * SCALE_SCALE) / gAbsoluteScale,
                   ((play_screen().height() / 2) * SCALE_SCALE) / gAbsoluteScale};

        scaled_screen.scale  = gAbsoluteScale;
        scaled_screen.bounds = Rect{
                g.ship->location.h - scale.width,
                g.ship->location.v - scale.height,
                g.ship->location.h + scale.width,
                g.ship->location.v + scale.height,
        };
    }

    // !!!!!!!!
    // nothing below can effect any object actions (expire actions get executed)
    // (but they can effect objects thinking)
    // !!!!!!!!
    SpaceObject* o        = nullptr;
    const Rect   viewport = antares::viewport();
    const Rect   sprite_bounds{
            viewport.left - kSpriteMaxSize, viewport.top - kSpriteMaxSize,
            viewport.right + kSpriteMaxSize, viewport.bottom + kSpriteMaxSize};
    for (Handle<SpaceObject> o_handle = g.root; (o = o_handle.get()); o_handle = o->nextObject) {
        if (o->active != kObjectInUse) {
            continue;
        } else if ((o->attributes & kIsVector) || !o->sprite.get()) {
            continue;
        }
        auto& sprite = *o->sprite;

        sprite.where = scale_to_viewport(o->location);
        if (!sprite_bounds.contains(sprite.where)) {
            sprite.where = Point{-kSpriteMaxSize, -kSpriteMaxSize};
        }

        update_static(o, unitsToDo);

        auto baseObject = o->base;
        if (o->attributes & kIsSelfAnimated) {
            if (baseObject->animation->speed != Fixed::zero()) {
                sprite.whichShape = more_evil_fixed_to_long(o->frame.animation.thisShape);
            }
        } else if (o->attributes & kShapeFromDirection) {
            int16_t angle = o->direction;
            mAddAngle(angle, rotation_resolution(*baseObject) >> 1);
            sprite.whichShape = angle / rotation_resolution(*baseObject);
        }
    }
}

static void age_object(const Handle<SpaceObject>& o) {
    if (o->expires) {
        o->expire_after -= kMajorTick;
        if (o->expire_after < ticks(0)) {
            if (o->base->expire.die) {
                o->active = kObjectToBeFreed;
            }

            exec(o->base->expire.action, o, SpaceObject::none(), {0, 0});
        }
    }
}

static void activate_object(const Handle<SpaceObject>& o) {
    if (o->periodicTime > ticks(0)) {
        o->periodicTime--;
        if (o->periodicTime <= ticks(0)) {
            exec(o->base->activate.action, o, SpaceObject::none(), {0, 0});
            o->periodicTime = o->base->activate.period->begin +
                              o->randomSeed.next(o->base->activate.period->range());
        }
    }
}

static void calc_misc(
        Handle<SpaceObject> near_objects[PROXIMITY_GRID_AREA],
        Handle<SpaceObject> far_objects[PROXIMITY_GRID_AREA]) {
    // set up player info so we can find closest ship (for scaling)
    uint64_t farthestDist = 0;
    uint64_t closestDist  = 0x7fffffffffffffffull;
    g.closest = g.farthest = Handle<SpaceObject>(0);

    // reset the collision grid
    for (int32_t i = 0; i < PROXIMITY_GRID_AREA; i++) {
        near_objects[i] = far_objects[i] = SpaceObject::none();
    }

    SpaceObject* o = nullptr;
    for (auto o_handle = g.root; (o = o_handle.get()); o_handle = o->nextObject) {
        if (!o->active) {
            if (g.ship.get() && g.ship->active) {
                o->distanceFromPlayer = 0x7fffffffffffffffull;
            }
        }
    }

    for (auto o_handle = g.root; (o = o_handle.get()); o_handle = o->nextObject) {
        if (!o->active) {
            continue;
        }

        age_object(o_handle);
        if (!o->active) {
            continue;
        }

        activate_object(o_handle);
        if (!o->active) {
            continue;
        }

        // Mark closest and farthest object relative to player, for zooming.
        if (g.ship.get() && g.ship->active) {
            if (o->attributes & kAppearOnRadar) {
                uint64_t hdiff        = ABS<int>(g.ship->location.h - o->location.h);
                uint64_t vdiff        = ABS<int>(g.ship->location.v - o->location.v);
                uint64_t dist         = (vdiff * vdiff) + (hdiff * hdiff);
                o->distanceFromPlayer = dist;
                if ((dist < closestDist) && (o_handle != g.ship)) {
                    if (!((g.zoom == Zoom::FOE) && (o->owner == g.ship->owner))) {
                        closestDist = dist;
                        g.closest   = o_handle;
                    }
                }
                if (dist > farthestDist) {
                    farthestDist = dist;
                    g.farthest   = o_handle;
                }
            }
        }

        if (o->attributes & kConsiderDistanceAttributes) {
            o->localFriendStrength  = o->base->ai.escort.power;
            o->localFoeStrength     = Fixed::zero();
            o->closestObject        = SpaceObject::none();
            o->closestDistance      = kMaximumRelevantDistanceSquared;
            o->absoluteBounds.right = o->absoluteBounds.left = 0;

            const auto& loc = o->location;
            {
                auto* near_object = &near_objects[proximity_index(
                        (loc.h / SUBSECTOR) & PROXIMITY_GRID_MASK,
                        (loc.v / SUBSECTOR) & PROXIMITY_GRID_MASK)];
                o->nextNearObject = *near_object;
                *near_object      = o_handle;

                o->collisionGrid = {loc.h / SECTOR_MEDIUM, loc.v / SECTOR_MEDIUM};
            }

            {
                auto* far_object = &far_objects[proximity_index(
                        (loc.h / SECTOR_MEDIUM) & PROXIMITY_GRID_MASK,
                        (loc.v / SECTOR_MEDIUM) & PROXIMITY_GRID_MASK)];
                o->nextFarObject = *far_object;
                *far_object      = o_handle;

                o->distanceGrid = {loc.h / SECTOR_HUGE, loc.v / SECTOR_HUGE};
            }

            if (!(o->attributes & kIsDestination)) {
                o->seenByPlayerFlags = 0x80000000;
            }
            o->runTimeFlags &= ~kIsHidden;

            if (o->sprite.get()) {
                o->sprite->icon =
                        o->icon.value_or(BaseObject::Icon{BaseObject::Icon::Shape::SQUARE, 0});
            }
        }
    }
}

// Collision uses inclusive rect bounds for historical reasons.
static bool inclusive_intersect(Rect x, Rect y) {
    ++x.right;
    ++x.bottom;
    ++y.right;
    ++y.bottom;
    return x.intersects(y);
}

static int mClipCode(int x, int y, const Rect& bounds) {
    return ((x < bounds.left) << 3) | ((x >= bounds.right) << 2) | ((y < bounds.top) << 1) |
           (y >= bounds.bottom);
}

static bool vector_intersects(const SpaceObject& vector, const SpaceObject& target) {
    if (vector.active == kObjectToBeFreed) {
        return false;
    }

    Point start(vector.location.h, vector.location.v);
    Point end(
            vector.frame.vector->lastGlobalLocation.h, vector.frame.vector->lastGlobalLocation.v);

    //
    // Determine if the line segment defined by `{start, end}` passes
    // through the rect `target.absoluteBounds`.
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
    int16_t end_clip = mClipCode(end.h, end.v, target.absoluteBounds);
    if (!end_clip) {
        return true;
    }

    while (true) {
        int16_t start_clip = mClipCode(start.h, start.v, target.absoluteBounds);
        if (!start_clip) {
            return true;
        } else if (start_clip & end_clip) {
            return false;
        }

        int32_t xd = end.h - start.h;
        int32_t yd = end.v - start.v;
        if (start_clip & 8) {
            start.v += yd * (target.absoluteBounds.left - start.h) / xd;
            start.h = target.absoluteBounds.left;
        } else if (start_clip & 4) {
            start.v += yd * (target.absoluteBounds.right - 1 - start.h) / xd;
            start.h = target.absoluteBounds.right - 1;
        } else if (start_clip & 2) {
            start.h += xd * (target.absoluteBounds.top - start.v) / yd;
            start.v = target.absoluteBounds.top;
        } else if (start_clip & 1) {
            start.h += xd * (target.absoluteBounds.bottom - 1 - start.v) / yd;
            start.v = target.absoluteBounds.bottom - 1;
        }
    }
}

// Set absoluteBounds on all objects.
static void calc_bounds() {
    SpaceObject* o = nullptr;
    for (auto o_handle = g.root; (o = o_handle.get()); o_handle = o->nextObject) {
        if ((o->absoluteBounds.left >= o->absoluteBounds.right) && o->sprite.get()) {
            const NatePixTable::Frame& frame = o->sprite->table->at(o->sprite->whichShape);
            o->absoluteBounds = scale_sprite_rect(frame, o->location, o->naturalScale);
        }
    }
}

static bool can_hit(const SpaceObject& a, const SpaceObject& b) {
    return (a.attributes & kCanCollide) && (b.attributes & kCanBeHit);
}

// Call HitObject() and CorrectPhysicalSpace() for all colliding pairs of objects.
static void calc_impacts(Handle<SpaceObject> near_objects[PROXIMITY_GRID_AREA]) {
    for (int32_t i = 0; i < PROXIMITY_GRID_AREA; i++) {
        const auto*  cells = kAdjacentCells.at[i];
        SpaceObject* a     = nullptr;
        for (auto a_handle = near_objects[i]; (a = a_handle.get()); a_handle = a->nextNearObject) {
            for (int32_t k = 0; k < AdjacentCells::size; k++) {
                Handle<SpaceObject> b_handle = a->nextNearObject;
                Point               super    = a->collisionGrid;
                if (k > 0) {
                    const auto& adj = cells[k];
                    b_handle        = near_objects[adj.index_offset];
                    super.offset(adj.super_offset.h, adj.super_offset.v);
                }

                SpaceObject* b = nullptr;
                for (; (b = b_handle.get()); b_handle = b->nextNearObject) {
                    if ((!can_hit(*a, *b) &&
                         !can_hit(*b, *a)) ||           // neither object can hit the other
                        (b->collisionGrid != super) ||  // not near enough
                        (a->owner == b->owner)) {       // same owner
                        continue;
                    }

                    if (a->attributes & b->attributes & kIsVector) {
                        // no reason vectors can't intersect, but the
                        // code we have now won't handle it.
                        continue;
                    } else if (a->attributes & kIsVector) {
                        if (vector_intersects(*a, *b)) {
                            HitObject(b_handle, a_handle);
                        }
                        continue;
                    } else if (b->attributes & kIsVector) {
                        if (vector_intersects(*b, *a)) {
                            HitObject(a_handle, b_handle);
                        }
                        continue;
                    }

                    if (inclusive_intersect(a->absoluteBounds, b->absoluteBounds)) {
                        HitObject(a_handle, b_handle);
                        HitObject(b_handle, a_handle);
                        correct_physical_space(a, b);
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
static void calc_locality(Handle<SpaceObject> far_objects[PROXIMITY_GRID_AREA]) {
    for (int32_t i = 0; i < PROXIMITY_GRID_AREA; i++) {
        const auto*  cells = kAdjacentCells.at[i];
        SpaceObject* a     = nullptr;
        for (auto a_handle = far_objects[i]; (a = a_handle.get()); a_handle = a->nextFarObject) {
            for (int32_t k = 0; k < AdjacentCells::size; k++) {
                Handle<SpaceObject> b_handle = a->nextFarObject;
                Point               super    = a->distanceGrid;
                if (k > 0) {
                    const auto& adj = cells[k];
                    b_handle        = far_objects[adj.index_offset];
                    super.offset(adj.super_offset.h, adj.super_offset.v);
                }

                SpaceObject* b = nullptr;
                for (; (b = b_handle.get()); b_handle = b->nextFarObject) {
                    if (b->distanceGrid != super) {
                        continue;
                    }
                    if ((b->owner != a->owner) &&
                        ((b->attributes & kCanThink) || (b->attributes & kRemoteOrHuman) ||
                         (b->attributes & kHated)) &&
                        ((a->attributes & kCanThink) || (a->attributes & kRemoteOrHuman) ||
                         (a->attributes & kHated))) {
                        uint32_t x_dist = ABS<int>(b->location.h - a->location.h);
                        uint32_t y_dist = ABS<int>(b->location.v - a->location.v);
                        uint32_t dist;
                        if ((x_dist > kMaximumRelevantDistance) ||
                            (y_dist > kMaximumRelevantDistance)) {
                            dist = kMaximumRelevantDistanceSquared;
                        } else {
                            dist = (y_dist * y_dist) + (x_dist * x_dist);
                        }

                        if (dist < kMaximumRelevantDistanceSquared) {
                            a->seenByPlayerFlags |= b->myPlayerFlag;
                            b->seenByPlayerFlags |= a->myPlayerFlag;

                            if (b->attributes & kHideEffect) {
                                a->runTimeFlags |= kIsHidden;
                            }

                            if (a->attributes & kHideEffect) {
                                b->runTimeFlags |= kIsHidden;
                            }
                        }

                        if (a->engages(*b)) {
                            if ((dist < a->closestDistance) &&
                                (b->attributes & kPotentialTarget)) {
                                a->closestDistance = dist;
                                a->closestObject   = b_handle;
                            }
                        }

                        if (b->engages(*a)) {
                            if ((dist < b->closestDistance) &&
                                (a->attributes & kPotentialTarget)) {
                                b->closestDistance = dist;
                                b->closestObject   = a_handle;
                            }
                        }

                        b->localFoeStrength += a->localFriendStrength;
                        b->localFriendStrength += a->localFoeStrength;
                    } else if (k == 0) {
                        if (a->owner != b->owner) {
                            b->localFoeStrength += a->localFriendStrength;
                            b->localFriendStrength += a->localFoeStrength;
                        } else {
                            b->localFoeStrength += a->localFoeStrength;
                            b->localFriendStrength += a->localFriendStrength;
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

    for (auto o_handle : SpaceObject::all()) {
        SpaceObject* o = o_handle.get();
        if (o->active == kObjectToBeFreed) {
            o->free();
        } else if (o->active) {
            if ((o->attributes & kConsiderDistanceAttributes) &&
                (!(o->attributes & kIsDestination))) {
                if (o->runTimeFlags & kIsCloaked) {
                    o->seenByPlayerFlags = 0;
                } else if (!(o->runTimeFlags & kIsHidden)) {
                    o->seenByPlayerFlags = 0xffffffff;
                }
                o->seenByPlayerFlags |= o->myPlayerFlag;
                if (!(o->seenByPlayerFlags & seen_by_me) && o->sprite.get()) {
                    o->sprite->icon.size = 0;
                }
            }
        }
    }
}

static void update_last_vector_locations() {
    for (auto o : SpaceObject::all()) {
        if (o->active == kObjectInUse) {
            if (o->attributes & kIsVector) {
                o->frame.vector->lastGlobalLocation = o->location;
            }
        }
    }
}

void CollideSpaceObjects() {
    Handle<SpaceObject> near_objects[PROXIMITY_GRID_AREA];
    Handle<SpaceObject> far_objects[PROXIMITY_GRID_AREA];

    calc_misc(near_objects, far_objects);
    calc_bounds();
    calc_impacts(near_objects);
    calc_locality(far_objects);
    calc_visibility();
    update_last_vector_locations();
}

static void adjust_velocity(SpaceObject* o, int16_t angle, Fixed totalMass, Fixed force) {
    Fixed tfix = (o->base->mass * force);
    if (totalMass == Fixed::zero()) {
        tfix = kFixedNone;
    } else {
        tfix /= totalMass;
    }
    tfix += o->maxVelocity >> 1;
    fixedPointType tvel;
    GetRotPoint(&tvel.h, &tvel.v, angle);
    tvel.h        = (tfix * tvel.h);
    tvel.v        = (tfix * tvel.v);
    o->velocity.v = tvel.v;
    o->velocity.h = tvel.h;
}

static void push(SpaceObject* o) {
    o->motionFraction.h += o->velocity.h;
    o->motionFraction.v += o->velocity.v;

    int32_t h;
    if (o->motionFraction.h >= Fixed::zero()) {
        h = more_evil_fixed_to_long(o->motionFraction.h + Fixed::from_float(0.5));
    } else {
        h = more_evil_fixed_to_long(o->motionFraction.h - Fixed::from_float(0.5)) + 1;
    }
    o->location.h -= h;
    o->motionFraction.h -= Fixed::from_long(h);

    int32_t v;
    if (o->motionFraction.v >= Fixed::zero()) {
        v = more_evil_fixed_to_long(o->motionFraction.v + Fixed::from_float(0.5));
    } else {
        v = more_evil_fixed_to_long(o->motionFraction.v - Fixed::from_float(0.5)) + 1;
    }
    o->location.v -= v;
    o->motionFraction.v -= Fixed::from_long(v);

    o->absoluteBounds.offset(-h, -v);
}

// CorrectPhysicalSpace-- takes 2 objects that are colliding and moves them back 1
//  bresenham-style step at a time to their previous locations or until they don't
//  collide.  For keeping objects which occupy space from occupying the
//  same space.

static void correct_physical_space(SpaceObject* a, SpaceObject* b) {
    if (!(b->attributes & a->attributes & kOccupiesSpace)) {
        return;  // no need; at least one object doesn't actually occupy space.
    } else if (b->owner == a->owner) {
        return;  // the collision changed the owner of one object, e.g. a flagpod.
    }

    // calculate the new velocities
    const Fixed   dvx   = b->velocity.h - a->velocity.h;
    const Fixed   dvy   = b->velocity.v - a->velocity.v;
    const Fixed   force = lsqrt((dvx * dvx) + (dvy * dvy));
    const int32_t ah    = b->location.h - a->location.h;
    const int32_t av    = b->location.v - a->location.v;

    const Fixed totalMass = a->base->mass + b->base->mass;
    int16_t     angle     = ratio_to_angle(ah, av);
    adjust_velocity(a, angle, totalMass, force);
    mAddAngle(angle, 180);
    adjust_velocity(b, angle, totalMass, force);

    if ((a->velocity.h == Fixed::zero()) && (a->velocity.v == Fixed::zero()) &&
        (b->velocity.h == Fixed::zero()) && (b->velocity.v == Fixed::zero())) {
        return;
    }

    while (
            !((a->absoluteBounds.right < b->absoluteBounds.left) ||
              (a->absoluteBounds.left > b->absoluteBounds.right) ||
              (a->absoluteBounds.bottom < b->absoluteBounds.top) ||
              (a->absoluteBounds.top > b->absoluteBounds.bottom))) {
        push(a);
        push(b);
    }
}

}  // namespace antares

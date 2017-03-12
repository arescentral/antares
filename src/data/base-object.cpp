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

// Space Object Handling >> MUST BE INITED _AFTER_ SCENARIOMAKER << (uses Ares Scenarios file)

#include "data/base-object.hpp"

namespace antares {

static const uint32_t kPeriodicActionTimeMask   = 0xff000000;
static const uint32_t kPeriodicActionRangeMask  = 0x00ff0000;
static const uint32_t kPeriodicActionNotMask    = 0x0000ffff;
static const int32_t  kPeriodicActionTimeShift  = 24;
static const int32_t  kPeriodicActionRangeShift = 16;

static const uint32_t kDestroyActionNotMask     = 0x7fffffff;
static const uint32_t kDestroyActionDontDieFlag = 0x80000000;

static bool read_destroy(pn::file_view in, BaseObject* object) {
    int32_t start, count;
    if (!in.read(&start, &count)) {
        return false;
    }
    object->destroyDontDie = count & kDestroyActionDontDieFlag;
    count &= kDestroyActionNotMask;
    auto end        = (start >= 0) ? (start + count) : start;
    object->destroy = {start, end};
    return true;
}

static bool read_expire(pn::file_view in, BaseObject* object) {
    int32_t start, count;
    if (!in.read(&start, &count)) {
        return false;
    }
    object->expireDontDie = count & kDestroyActionDontDieFlag;
    count &= kDestroyActionNotMask;
    auto end       = (start >= 0) ? (start + count) : start;
    object->expire = {start, end};
    return true;
}

static bool read_create(pn::file_view in, BaseObject* object) {
    int32_t start, count;
    if (!in.read(&start, &count)) {
        return false;
    }
    auto end       = (start >= 0) ? (start + count) : start;
    object->create = {start, end};
    return true;
}

static bool read_collide(pn::file_view in, BaseObject* object) {
    int32_t start, count;
    if (!in.read(&start, &count)) {
        return false;
    }
    auto end        = (start >= 0) ? (start + count) : start;
    object->collide = {start, end};
    return true;
}

static bool read_activate(pn::file_view in, BaseObject* object) {
    int32_t start, count;
    if (!in.read(&start, &count)) {
        return false;
    }
    object->activatePeriod = ticks((count & kPeriodicActionTimeMask) >> kPeriodicActionTimeShift);
    object->activatePeriodRange =
            ticks((count & kPeriodicActionRangeMask) >> kPeriodicActionRangeShift);
    count &= kPeriodicActionNotMask;
    auto end         = (start >= 0) ? (start + count) : start;
    object->activate = {start, end};
    return true;
}

static bool read_arrive(pn::file_view in, BaseObject* object) {
    int32_t start, count;
    if (!in.read(&start, &count)) {
        return false;
    }
    auto end       = (start >= 0) ? (start + count) : start;
    object->arrive = {start, end};
    return true;
}

static bool read_weapons(pn::file_view in, BaseObject* object) {
    int32_t pulse, beam, special;
    if (!in.read(
                &pulse, &beam, &special, &object->pulse.positionNum, &object->beam.positionNum,
                &object->special.positionNum)) {
        return false;
    }
    object->pulse.base   = Handle<BaseObject>(pulse);
    object->beam.base    = Handle<BaseObject>(beam);
    object->special.base = Handle<BaseObject>(special);

    for (auto weapon : {&object->pulse, &object->beam, &object->special}) {
        for (size_t i = 0; i < kMaxWeaponPosition; ++i) {
            if (!read_from(in, &weapon->position[i])) {
                return false;
            }
        }
    }
    return true;
}

bool read_from(pn::file_view in, BaseObject* object) {
    uint8_t section[32];

    if (!in.read(&object->attributes)) {
        return false;
    }
    if (object->attributes & kIsSelfAnimated) {
        object->attributes &= ~(kShapeFromDirection | kIsVector);
    } else if (object->attributes & kShapeFromDirection) {
        object->attributes &= ~kIsVector;
    }

    int32_t initial_age, age_range;
    if (!(in.read(&object->baseClass, &object->baseRace, &object->price) &&
          read_from(in, &object->offenseValue) && in.read(&object->destinationClass) &&
          read_from(in, &object->maxVelocity) && read_from(in, &object->warpSpeed) &&
          in.read(&object->warpOutDistance) && read_from(in, &object->initialVelocity) &&
          read_from(in, &object->initialVelocityRange) && read_from(in, &object->mass) &&
          read_from(in, &object->maxThrust) &&
          in.read(&object->health, &object->damage, &object->energy, &initial_age, &age_range))) {
        return false;
    }

    object->initialAge = ticks(initial_age);
    if (age_range >= 0) {
        object->initialAgeRange = ticks(age_range);
    } else {
        object->initialAgeRange = ticks(0);
    }

    if (object->attributes & kNeutralDeath) {
        object->occupy_count = age_range;
    } else {
        object->occupy_count = -1;
    }

    uint8_t unused1;
    if (!in.read(
                &object->naturalScale, &object->pixLayer, &object->pixResID, &object->tinySize,
                &object->shieldColor, &unused1, &object->initialDirection,
                &object->initialDirectionRange)) {
        return false;
    }

    if ((object->shieldColor != 0xFF) && (object->shieldColor != 0)) {
        object->shieldColor = GetTranslateColorShade(object->shieldColor, 15);
    }

    if (!(read_weapons(in, object) && read_from(in, &object->friendDefecit) &&
          read_from(in, &object->dangerThreshold) &&
          in.read(&object->specialDirection, &object->arriveActionDistance) &&
          read_destroy(in, object) && read_expire(in, object) && read_create(in, object) &&
          read_collide(in, object) && read_activate(in, object) && read_arrive(in, object) &&
          (fread(section, 1, 32, in.c_obj()) == 32) && in.read(&object->buildFlags) &&
          in.read(&object->orderFlags))) {
        return false;
    }

    object->levelKeyTag  = (object->buildFlags & kLevelKeyTag) >> kLevelKeyTagShift;
    object->engageKeyTag = (object->buildFlags & kEngageKeyTag) >> kEngageKeyTagShift;
    object->orderKeyTag  = (object->orderFlags & kOrderKeyTag) >> kOrderKeyTagShift;

    uint32_t build_time, unused2;
    uint16_t unused3;
    if (!(read_from(in, &object->buildRatio) &&
          in.read(&build_time, &object->skillNum, &object->skillDen, &object->skillNumAdj,
                  &object->skillDenAdj, &object->pictPortraitResID, &unused2, &unused3,
                  &object->internalFlags))) {
        return false;
    }
    object->buildTime = 3 * ticks(build_time / 10);

    pn::file sub = pn::data_view{section, 32}.open();
    if (object->attributes & kShapeFromDirection) {
        read_from(sub, &object->frame.rotation);
    } else if (object->attributes & kIsSelfAnimated) {
        read_from(sub, &object->frame.animation);
    } else if (object->attributes & kIsVector) {
        read_from(sub, &object->frame.vector);
        if (object->frame.vector.color > 16) {
            object->frame.vector.color = object->frame.vector.color;
        } else {
            object->frame.vector.color = 0;
        }
    } else {
        read_from(sub, &object->frame.weapon);
    }
    return true;
}

bool read_from(pn::file_view in, objectFrameType::Rotation* rotation) {
    return in.read(&rotation->shapeOffset, &rotation->rotRes) &&
           read_from(in, &rotation->maxTurnRate) && read_from(in, &rotation->turnAcceleration);
}

bool read_from(pn::file_view in, objectFrameType::Animation* animation) {
    int32_t first_shape, last_shape, frame_shape, frame_shape_range;
    if (!(in.read(&first_shape, &last_shape, &animation->frameDirection,
                  &animation->frameDirectionRange) &&
          read_from(in, &animation->frameSpeed) && read_from(in, &animation->frameSpeedRange) &&
          in.read(&frame_shape, &frame_shape_range))) {
        return false;
    }
    animation->firstShape      = Fixed::from_long(first_shape);
    animation->lastShape       = Fixed::from_long(last_shape);
    animation->frameShape      = Fixed::from_long(frame_shape);
    animation->frameShapeRange = Fixed::from_long(frame_shape_range);
    return true;
}

bool read_from(pn::file_view in, objectFrameType::Vector* vector) {
    return in.read(&vector->color, &vector->kind, &vector->accuracy, &vector->range);
}

bool read_from(pn::file_view in, objectFrameType::Weapon* weapon) {
    int32_t fire_time;
    if (!(in.read(&weapon->usage, &weapon->energyCost, &fire_time, &weapon->ammo,
                  &weapon->range) &&
          read_from(in, &weapon->inverseSpeed) && in.read(&weapon->restockCost))) {
        return false;
    }
    weapon->fireTime = ticks(fire_time);
    return true;
}

}  // namespace antares

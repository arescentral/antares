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

#include <sfz/sfz.hpp>

using sfz::BytesSlice;
using sfz::ReadSource;
using sfz::read;

namespace antares {

static const uint32_t kPeriodicActionTimeMask   = 0xff000000;
static const uint32_t kPeriodicActionRangeMask  = 0x00ff0000;
static const uint32_t kPeriodicActionNotMask    = 0x0000ffff;
static const int32_t  kPeriodicActionTimeShift  = 24;
static const int32_t  kPeriodicActionRangeShift = 16;

static const uint32_t kDestroyActionNotMask     = 0x7fffffff;
static const uint32_t kDestroyActionDontDieFlag = 0x80000000;

static void read_destroy(ReadSource in, BaseObject& object) {
    auto start            = read<int32_t>(in);
    auto count            = read<int32_t>(in);
    object.destroyDontDie = count & kDestroyActionDontDieFlag;
    count &= kDestroyActionNotMask;
    auto end       = (start >= 0) ? (start + count) : start;
    object.destroy = {start, end};
}

static void read_expire(ReadSource in, BaseObject& object) {
    auto start           = read<int32_t>(in);
    auto count           = read<int32_t>(in);
    object.expireDontDie = count & kDestroyActionDontDieFlag;
    count &= kDestroyActionNotMask;
    auto end      = (start >= 0) ? (start + count) : start;
    object.expire = {start, end};
}

static void read_create(ReadSource in, BaseObject& object) {
    auto start    = read<int32_t>(in);
    auto count    = read<int32_t>(in);
    auto end      = (start >= 0) ? (start + count) : start;
    object.create = {start, end};
}

static void read_collide(ReadSource in, BaseObject& object) {
    auto start     = read<int32_t>(in);
    auto count     = read<int32_t>(in);
    auto end       = (start >= 0) ? (start + count) : start;
    object.collide = {start, end};
}

static void read_activate(ReadSource in, BaseObject& object) {
    auto start            = read<int32_t>(in);
    auto count            = read<int32_t>(in);
    object.activatePeriod = ticks((count & kPeriodicActionTimeMask) >> kPeriodicActionTimeShift);
    object.activatePeriodRange =
            ticks((count & kPeriodicActionRangeMask) >> kPeriodicActionRangeShift);
    count &= kPeriodicActionNotMask;
    auto end        = (start >= 0) ? (start + count) : start;
    object.activate = {start, end};
}

static void read_arrive(ReadSource in, BaseObject& object) {
    auto start    = read<int32_t>(in);
    auto count    = read<int32_t>(in);
    auto end      = (start >= 0) ? (start + count) : start;
    object.arrive = {start, end};
}

void read_from(ReadSource in, BaseObject& object) {
    uint8_t section[32];

    read(in, object.attributes);
    if (object.attributes & kIsSelfAnimated) {
        object.attributes &= ~(kShapeFromDirection | kIsVector);
    } else if (object.attributes & kShapeFromDirection) {
        object.attributes &= ~kIsVector;
    }

    read(in, object.baseClass);
    read(in, object.baseRace);
    read(in, object.price);

    read(in, object.offenseValue);
    read(in, object.destinationClass);

    read(in, object.maxVelocity);
    read(in, object.warpSpeed);
    read(in, object.warpOutDistance);

    read(in, object.initialVelocity);
    read(in, object.initialVelocityRange);

    read(in, object.mass);
    read(in, object.maxThrust);

    read(in, object.health);
    read(in, object.damage);
    read(in, object.energy);

    object.initialAge = ticks(read<int32_t>(in));

    int32_t age_range = read<int32_t>(in);
    if (age_range >= 0) {
        object.initialAgeRange = ticks(age_range);
    } else {
        object.initialAgeRange = ticks(0);
    }

    if (object.attributes & kNeutralDeath) {
        object.occupy_count = age_range;
    } else {
        object.occupy_count = -1;
    }

    read(in, object.naturalScale);

    read(in, object.pixLayer);
    read(in, object.pixResID);
    read(in, object.tinySize);
    read(in, object.shieldColor);
    if ((object.shieldColor != 0xFF) && (object.shieldColor != 0)) {
        object.shieldColor = GetTranslateColorShade(object.shieldColor, 15);
    }
    in.shift(1);

    read(in, object.initialDirection);
    read(in, object.initialDirectionRange);

    object.pulse.base   = Handle<BaseObject>(read<int32_t>(in));
    object.beam.base    = Handle<BaseObject>(read<int32_t>(in));
    object.special.base = Handle<BaseObject>(read<int32_t>(in));

    read(in, object.pulse.positionNum);
    read(in, object.beam.positionNum);
    read(in, object.special.positionNum);

    read(in, object.pulse.position, kMaxWeaponPosition);
    read(in, object.beam.position, kMaxWeaponPosition);
    read(in, object.special.position, kMaxWeaponPosition);

    read(in, object.friendDefecit);
    read(in, object.dangerThreshold);
    read(in, object.specialDirection);

    read(in, object.arriveActionDistance);

    read_destroy(in, object);
    read_expire(in, object);
    read_create(in, object);
    read_collide(in, object);
    read_activate(in, object);
    read_arrive(in, object);

    read(in, section, 32);

    read(in, object.buildFlags);
    read(in, object.orderFlags);

    object.levelKeyTag  = (object.buildFlags & kLevelKeyTag) >> kLevelKeyTagShift;
    object.engageKeyTag = (object.buildFlags & kEngageKeyTag) >> kEngageKeyTagShift;
    object.orderKeyTag  = (object.orderFlags & kOrderKeyTag) >> kOrderKeyTagShift;

    read(in, object.buildRatio);
    object.buildTime = 3 * ticks(read<uint32_t>(in) / 10);
    read(in, object.skillNum);
    read(in, object.skillDen);
    read(in, object.skillNumAdj);
    read(in, object.skillDenAdj);
    read(in, object.pictPortraitResID);
    in.shift(6);
    read(in, object.internalFlags);

    BytesSlice sub(BytesSlice(section, 32));
    if (object.attributes & kShapeFromDirection) {
        read(sub, object.frame.rotation);
    } else if (object.attributes & kIsSelfAnimated) {
        read(sub, object.frame.animation);
    } else if (object.attributes & kIsVector) {
        read(sub, object.frame.vector);
        if (object.frame.vector.color > 16) {
            object.frame.vector.color = GetTranslateIndex(object.frame.vector.color);
        } else {
            object.frame.vector.color = 0;
        }
    } else {
        read(sub, object.frame.weapon);
    }
}

void read_from(ReadSource in, objectFrameType::Rotation& rotation) {
    read(in, rotation.shapeOffset);
    read(in, rotation.rotRes);
    read(in, rotation.maxTurnRate);
    read(in, rotation.turnAcceleration);
}

void read_from(ReadSource in, objectFrameType::Animation& animation) {
    animation.firstShape = Fixed::from_long(read<int32_t>(in));
    animation.lastShape  = Fixed::from_long(read<int32_t>(in));
    read(in, animation.frameDirection);
    read(in, animation.frameDirectionRange);
    read(in, animation.frameSpeed);
    read(in, animation.frameSpeedRange);
    animation.frameShape      = Fixed::from_long(read<int32_t>(in));
    animation.frameShapeRange = Fixed::from_long(read<int32_t>(in));
}

void read_from(ReadSource in, objectFrameType::Vector& vector) {
    read(in, vector.color);
    read(in, vector.kind);
    read(in, vector.accuracy);
    read(in, vector.range);
}

void read_from(ReadSource in, objectFrameType::Weapon& weapon) {
    read(in, weapon.usage);
    read(in, weapon.energyCost);
    weapon.fireTime = ticks(read<int32_t>(in));
    read(in, weapon.ammo);
    read(in, weapon.range);
    read(in, weapon.inverseSpeed);
    read(in, weapon.restockCost);
}

}  // namespace antares

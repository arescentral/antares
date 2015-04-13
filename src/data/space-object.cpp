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

// Space Object Handling >> MUST BE INITED _AFTER_ SCENARIOMAKER << (uses Ares Scenarios file)

#include "data/space-object.hpp"

#include <sfz/sfz.hpp>

using sfz::BytesSlice;
using sfz::ReadSource;
using sfz::read;

namespace antares {

static const uint32_t kPeriodicActionTimeMask  = 0xff000000;
static const uint32_t kPeriodicActionRangeMask = 0x00ff0000;
static const uint32_t kPeriodicActionNotMask   = 0x0000ffff;
static const int32_t kPeriodicActionTimeShift  = 24;
static const int32_t kPeriodicActionRangeShift = 16;

static const uint32_t kDestroyActionNotMask        = 0x7fffffff;
static const uint32_t kDestroyActionDontDieFlag    = 0x80000000;

void read_from(ReadSource in, baseObjectType& object) {
    uint8_t section[32];

    read(in, object.attributes);
    if ((object.attributes & kIsSelfAnimated) && (object.attributes & kShapeFromDirection)) {
        object.attributes ^= kShapeFromDirection;
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

    read(in, object.initialAge);
    read(in, object.initialAgeRange);

    read(in, object.naturalScale);

    read(in, object.pixLayer);
    read(in, object.pixResID);
    read(in, object.tinySize);
    read(in, object.shieldColor);
    in.shift(1);

    read(in, object.initialDirection);
    read(in, object.initialDirectionRange);

    read(in, object.pulse.base);
    read(in, object.beam.base);
    read(in, object.special.base);

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

    read(in, object.destroy);
    read(in, object.expire);
    read(in, object.create);
    read(in, object.collide);
    read(in, object.activate);
    read(in, object.arrive);

    object.destroyDontDie = object.destroy.count & kDestroyActionDontDieFlag;
    object.destroy.count &= kDestroyActionNotMask;
    object.expireDontDie = object.expire.count & kDestroyActionDontDieFlag;
    object.expire.count &= kDestroyActionNotMask;
    object.activatePeriod = (object.activate.count & kPeriodicActionTimeMask) >> kPeriodicActionTimeShift;
    object.activatePeriodRange = (object.activate.count & kPeriodicActionRangeMask) >> kPeriodicActionRangeShift;
    object.activate.count &= kPeriodicActionNotMask;

    read(in, section, 32);

    read(in, object.buildFlags);
    read(in, object.orderFlags);

    object.levelKeyTag = (object.buildFlags & kLevelKeyTag) >> kLevelKeyTagShift;
    object.engageKeyTag = (object.buildFlags & kEngageKeyTag) >> kEngageKeyTagShift;
    object.orderKeyTag = (object.orderFlags & kOrderKeyTag) >> kOrderKeyTagShift;

    read(in, object.buildRatio);
    read(in, object.buildTime);
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
    } else if (object.attributes & kIsBeam) {
        read(sub, object.frame.beam);
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
    read(in, animation.firstShape);
    read(in, animation.lastShape);
    read(in, animation.frameDirection);
    read(in, animation.frameDirectionRange);
    read(in, animation.frameSpeed);
    read(in, animation.frameSpeedRange);
    read(in, animation.frameShape);
    read(in, animation.frameShapeRange);
}

void read_from(ReadSource in, objectFrameType::Beam& beam) {
    read(in, beam.color);
    read(in, beam.kind);
    read(in, beam.accuracy);
    read(in, beam.range);
}

void read_from(ReadSource in, objectFrameType::Weapon& weapon) {
    read(in, weapon.usage);
    read(in, weapon.energyCost);
    read(in, weapon.fireTime);
    read(in, weapon.ammo);
    read(in, weapon.range);
    read(in, weapon.inverseSpeed);
    read(in, weapon.restockCost);
}

}  // namespace antares

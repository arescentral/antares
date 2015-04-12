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

void read_from(ReadSource in, objectActionType& action) {
    uint8_t section[24];

    read(in, action.verb);
    read(in, action.reflexive);
    read(in, action.inclusiveFilter);
    read(in, action.exclusiveFilter);
    if (action.exclusiveFilter == 0xffffffff) {
        action.levelKeyTag = (action.inclusiveFilter & kLevelKeyTag) >> kLevelKeyTagShift;
    } else {
        action.levelKeyTag = 0;
    }
    read(in, action.owner);
    read(in, action.delay);
    read(in, action.initialSubjectOverride);
    read(in, action.initialDirectOverride);
    in.shift(4);
    read(in, section, 24);

    BytesSlice sub(BytesSlice(section, 24));
    switch (action.verb) {
      case kNoAction:
      case kSetDestination:
      case kActivateSpecial:
      case kActivatePulse:
      case kActivateBeam:
      case kNilTarget:
        break;

      case kCreateObject:
      case kCreateObjectSetDest:
        read(sub, action.argument.createObject);
        break;

      case kPlaySound:
        read(sub, action.argument.playSound);
        break;

      case kAlter:
        read(sub, action.argument.alterObject);
        break;

      case kMakeSparks:
        read(sub, action.argument.makeSparks);
        break;

      case kReleaseEnergy:
        read(sub, action.argument.releaseEnergy);
        break;

      case kLandAt:
        read(sub, action.argument.landAt);
        break;

      case kEnterWarp:
        read(sub, action.argument.enterWarp);
        break;

      case kDisplayMessage:
        read(sub, action.argument.displayMessage);
        break;

      case kChangeScore:
        read(sub, action.argument.changeScore);
        break;

      case kDeclareWinner:
        read(sub, action.argument.declareWinner);
        break;

      case kDie:
        read(sub, action.argument.killObject);
        break;

      case kColorFlash:
        read(sub, action.argument.colorFlash);
        break;

      case kDisableKeys:
      case kEnableKeys:
        read(sub, action.argument.keys);
        break;

      case kSetZoom:
        read(sub, action.argument.zoom);
        break;

      case kComputerSelect:
        read(sub, action.argument.computerSelect);
        break;

      case kAssumeInitialObject:
        read(sub, action.argument.assumeInitial);
        break;
    }
}

void read_from(ReadSource in, argumentType::CreateObject& argument) {
    read(in, argument.whichBaseType);
    read(in, argument.howManyMinimum);
    read(in, argument.howManyRange);
    read(in, argument.velocityRelative);
    read(in, argument.directionRelative);
    read(in, argument.randomDistance);
}

void read_from(ReadSource in, argumentType::PlaySound& argument) {
    read(in, argument.priority);
    in.shift(1);
    read(in, argument.persistence);
    read(in, argument.absolute);
    in.shift(1);
    read(in, argument.volumeMinimum);
    read(in, argument.volumeRange);
    read(in, argument.idMinimum);
    read(in, argument.idRange);
}

void read_from(ReadSource in, argumentType::AlterObject& argument) {
    read(in, argument.alterType);
    read(in, argument.relative);
    read(in, argument.minimum);
    read(in, argument.range);
}

void read_from(ReadSource in, argumentType::MakeSparks& argument) {
    read(in, argument.howMany);
    read(in, argument.speed);
    read(in, argument.velocityRange);
    read(in, argument.color);
}

void read_from(ReadSource in, argumentType::ReleaseEnergy& argument) {
    read(in, argument.percent);
}

void read_from(ReadSource in, argumentType::LandAt& argument) {
    read(in, argument.landingSpeed);
}

void read_from(ReadSource in, argumentType::EnterWarp& argument) {
    read(in, argument.warpSpeed);
}

void read_from(ReadSource in, argumentType::DisplayMessage& argument) {
    read(in, argument.resID);
    read(in, argument.pageNum);
}

void read_from(ReadSource in, argumentType::ChangeScore& argument) {
    read(in, argument.whichPlayer);
    read(in, argument.whichScore);
    read(in, argument.amount);
}

void read_from(ReadSource in, argumentType::DeclareWinner& argument) {
    read(in, argument.whichPlayer);
    read(in, argument.nextLevel);
    read(in, argument.textID);
}

void read_from(ReadSource in, argumentType::KillObject& argument) {
    read(in, argument.dieType);
}

void read_from(ReadSource in, argumentType::ColorFlash& argument) {
    read(in, argument.length);
    read(in, argument.color);
    read(in, argument.shade);
}

void read_from(ReadSource in, argumentType::Keys& argument) {
    read(in, argument.keyMask);
}

void read_from(ReadSource in, argumentType::Zoom& argument) {
    read(in, argument.zoomLevel);
}

void read_from(ReadSource in, argumentType::ComputerSelect& argument) {
    read(in, argument.screenNumber);
    read(in, argument.lineNumber);
}

void read_from(ReadSource in, argumentType::AssumeInitial& argument) {
    read(in, argument.whichInitialObject);
}

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

    read(in, object.destroyAction);
    read(in, object.destroyActionNum);
    read(in, object.expireAction);
    read(in, object.expireActionNum);
    read(in, object.createAction);
    read(in, object.createActionNum);
    read(in, object.collideAction);
    read(in, object.collideActionNum);
    read(in, object.activateAction);
    read(in, object.activateActionNum);
    read(in, object.arriveAction);
    read(in, object.arriveActionNum);

    object.destroyDontDie = object.destroyActionNum & kDestroyActionDontDieFlag;
    object.destroyActionNum &= kDestroyActionNotMask;
    object.expireDontDie = object.expireActionNum & kDestroyActionDontDieFlag;
    object.expireActionNum &= kDestroyActionNotMask;
    object.activatePeriod = (object.activateActionNum & kPeriodicActionTimeMask) >> kPeriodicActionTimeShift;
    object.activatePeriodRange = (object.activateActionNum & kPeriodicActionRangeMask) >> kPeriodicActionRangeShift;
    object.activateActionNum &= kPeriodicActionNotMask;

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

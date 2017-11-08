// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// Copyright (C) 2015-2017 The Antares Authors
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

#include "data/action.hpp"

#include <sfz/sfz.hpp>

#include "data/base-object.hpp"

using sfz::BytesSlice;
using sfz::ReadSource;
using sfz::read;

namespace antares {

void read_from(ReadSource in, Action& action) {
    uint8_t section[24];

    action.verb = read<uint8_t>(in) << 8;

    read(in, action.reflexive);
    read(in, action.inclusiveFilter);
    read(in, action.exclusiveFilter);
    if (action.exclusiveFilter == 0xffffffff) {
        action.levelKeyTag = (action.inclusiveFilter & kLevelKeyTag) >> kLevelKeyTagShift;
    } else {
        action.levelKeyTag = 0;
    }
    read(in, action.owner);
    action.delay = ticks(read<uint32_t>(in));
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
        case kNilTarget: break;

        case kCreateObject:
        case kCreateObjectSetDest: read(sub, action.argument.createObject); break;

        case kPlaySound: read(sub, action.argument.playSound); break;

        case kAlter:
            action.verb |= read<uint8_t>(sub);
            switch (action.verb) {
                case kAlterDamage: read(sub, action.argument.alterDamage); break;
                case kAlterEnergy: read(sub, action.argument.alterEnergy); break;
                case kAlterHidden: read(sub, action.argument.alterHidden); break;
                case kAlterSpin: read(sub, action.argument.alterSpin); break;
                case kAlterOffline: read(sub, action.argument.alterOffline); break;
                case kAlterVelocity: read(sub, action.argument.alterVelocity); break;
                case kAlterMaxVelocity: read(sub, action.argument.alterMaxVelocity); break;
                case kAlterThrust: read(sub, action.argument.alterThrust); break;
                case kAlterBaseType: read(sub, action.argument.alterBaseType); break;
                case kAlterOwner: read(sub, action.argument.alterOwner); break;
                case kAlterConditionTrueYet:
                    read(sub, action.argument.alterConditionTrueYet);
                    break;
                case kAlterOccupation: read(sub, action.argument.alterOccupation); break;
                case kAlterAbsoluteCash: read(sub, action.argument.alterAbsoluteCash); break;
                case kAlterAge: read(sub, action.argument.alterAge); break;
                case kAlterLocation: read(sub, action.argument.alterLocation); break;
                case kAlterAbsoluteLocation:
                    read(sub, action.argument.alterAbsoluteLocation);
                    break;
                case kAlterWeapon1:
                case kAlterWeapon2:
                case kAlterSpecial: read(sub, action.argument.alterWeapon); break;
            }
            break;

        case kMakeSparks: read(sub, action.argument.makeSparks); break;

        case kReleaseEnergy: read(sub, action.argument.releaseEnergy); break;

        case kLandAt: read(sub, action.argument.landAt); break;

        case kEnterWarp: read(sub, action.argument.enterWarp); break;

        case kDisplayMessage: read(sub, action.argument.displayMessage); break;

        case kChangeScore: read(sub, action.argument.changeScore); break;

        case kDeclareWinner: read(sub, action.argument.declareWinner); break;

        case kDie: read(sub, action.argument.killObject); break;

        case kColorFlash: read(sub, action.argument.colorFlash); break;

        case kDisableKeys:
        case kEnableKeys: read(sub, action.argument.keys); break;

        case kSetZoom: read(sub, action.argument.zoom); break;

        case kComputerSelect: read(sub, action.argument.computerSelect); break;

        case kAssumeInitialObject: read(sub, action.argument.assumeInitial); break;
    }
}

void read_from(ReadSource in, argumentType::CreateObject& argument) {
    argument.whichBaseType = Handle<BaseObject>(read<int32_t>(in));
    read(in, argument.howManyMinimum);
    read(in, argument.howManyRange);
    read(in, argument.velocityRelative);
    read(in, argument.directionRelative);
    read(in, argument.randomDistance);
}

void read_from(ReadSource in, argumentType::PlaySound& argument) {
    read(in, argument.priority);
    in.shift(1);
    argument.persistence = ticks(read<int32_t>(in));
    read(in, argument.absolute);
    in.shift(1);
    read(in, argument.volumeMinimum);
    read(in, argument.volumeRange);
    read(in, argument.idMinimum);
    read(in, argument.idRange);
}

void read_from(ReadSource in, argumentType::AlterHidden& argument) {
    in.shift(1);
    read(in, argument.first);
    read(in, argument.count_minus_1);
}

void read_from(ReadSource in, argumentType::AlterConditionTrueYet& argument) {
    argument.true_yet = read<uint8_t>(in);
    read(in, argument.first);
    read(in, argument.count_minus_1);
}

void read_from(ReadSource in, argumentType::AlterSimple& argument) {
    in.shift(1);
    read(in, argument.amount);
}

void read_from(ReadSource in, argumentType::AlterWeapon& argument) {
    in.shift(1);
    argument.base = Handle<BaseObject>(read<int32_t>(in));
}

void read_from(ReadSource in, argumentType::AlterFixedRange& argument) {
    in.shift(1);
    read(in, argument.minimum);
    read(in, argument.range);
}

void read_from(ReadSource in, argumentType::AlterAge& argument) {
    argument.relative = read<uint8_t>(in);
    argument.minimum  = ticks(read<int32_t>(in));
    argument.range    = ticks(read<int32_t>(in));
}

void read_from(ReadSource in, argumentType::AlterThrust& argument) {
    argument.relative = read<uint8_t>(in);
    read(in, argument.minimum);
    read(in, argument.range);
}

void read_from(ReadSource in, argumentType::AlterMaxVelocity& argument) {
    in.shift(1);
    read(in, argument.amount);
}

void read_from(ReadSource in, argumentType::AlterOwner& argument) {
    read(in, argument.relative);
    argument.admiral = Handle<Admiral>(read<uint32_t>(in));
}

void read_from(ReadSource in, argumentType::AlterCash& argument) {
    read(in, argument.relative);
    read(in, argument.amount);
    argument.admiral = Handle<Admiral>(read<uint32_t>(in));
}

void read_from(ReadSource in, argumentType::AlterVelocity& argument) {
    read(in, argument.relative);
    read(in, argument.amount);
}

void read_from(ReadSource in, argumentType::AlterBaseType& argument) {
    argument.keep_ammo = read<uint8_t>(in);
    argument.base      = Handle<BaseObject>(read<int32_t>(in));
}

void read_from(ReadSource in, argumentType::AlterLocation& argument) {
    argument.relative = read<uint8_t>(in);
    read(in, argument.by);
}

void read_from(ReadSource in, argumentType::AlterAbsoluteLocation& argument) {
    argument.relative = read<uint8_t>(in);
    read(in, argument.at);
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
    argument.whichPlayer = Handle<Admiral>(read<int32_t>(in));
    read(in, argument.whichScore);
    read(in, argument.amount);
}

void read_from(ReadSource in, argumentType::DeclareWinner& argument) {
    argument.whichPlayer = Handle<Admiral>(read<int32_t>(in));
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

}  // namespace antares

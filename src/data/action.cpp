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
#include "data/resource.hpp"

namespace antares {

namespace {

bool read_from(pn::file_view in, argumentType::CreateObject* argument) {
    int32_t base_type;
    if (!in.read(
                &base_type, &argument->howManyMinimum, &argument->howManyRange,
                &argument->velocityRelative, &argument->directionRelative,
                &argument->randomDistance)) {
        return false;
    }
    argument->whichBaseType = Handle<BaseObject>(base_type);
    return true;
}

bool read_from(pn::file_view in, argumentType::PlaySound* argument) {
    uint8_t unused1, unused2;
    int32_t persistence;
    if (!in.read(
                &argument->priority, &unused1, &persistence, &argument->absolute, &unused2,
                &argument->volumeMinimum, &argument->volumeRange, &argument->idMinimum,
                &argument->idRange)) {
        return false;
    }
    argument->persistence = ticks(persistence);
    return true;
}

bool read_from(pn::file_view in, argumentType::AlterSimple* argument) {
    uint8_t unused;
    return in.read(&unused, &argument->amount);
}

bool read_from(pn::file_view in, argumentType::AlterWeapon* argument) {
    uint8_t unused;
    int32_t base;
    if (!in.read(&unused, &base)) {
        return false;
    }
    argument->base = Handle<BaseObject>(base);
    return true;
}

bool read_from(pn::file_view in, argumentType::AlterAbsoluteLocation* argument) {
    return in.read(&argument->relative) && read_from(in, &argument->at);
}

bool read_from(pn::file_view in, argumentType::AlterHidden* argument) {
    uint8_t unused;
    return in.read(&unused, &argument->first, &argument->count_minus_1);
}

bool read_from(pn::file_view in, argumentType::AlterFixedRange* argument) {
    uint8_t unused;
    return in.read(&unused) && read_from(in, &argument->minimum) &&
           read_from(in, &argument->range);
}

bool read_from(pn::file_view in, argumentType::AlterVelocity* argument) {
    return in.read(&argument->relative) && read_from(in, &argument->amount);
}

bool read_from(pn::file_view in, argumentType::AlterMaxVelocity* argument) {
    uint8_t unused;
    return in.read(&unused) && read_from(in, &argument->amount);
}

bool read_from(pn::file_view in, argumentType::AlterThrust* argument) {
    return in.read(&argument->relative) && read_from(in, &argument->minimum) &&
           read_from(in, &argument->range);
}

bool read_from(pn::file_view in, argumentType::AlterBaseType* argument) {
    int32_t base;
    if (!in.read(&argument->keep_ammo, &base)) {
        return false;
    }
    argument->base = Handle<BaseObject>(base);
    return true;
}

bool read_from(pn::file_view in, argumentType::AlterOwner* argument) {
    uint32_t admiral;
    if (!in.read(&argument->relative, &admiral)) {
        return false;
    }
    argument->admiral = Handle<Admiral>(admiral);
    return true;
}

bool read_from(pn::file_view in, argumentType::AlterConditionTrueYet* argument) {
    return in.read(&argument->true_yet, &argument->first, &argument->count_minus_1);
}

bool read_from(pn::file_view in, argumentType::AlterCash* argument) {
    uint32_t admiral;
    if (!(in.read(&argument->relative) && read_from(in, &argument->amount) && in.read(&admiral))) {
        return false;
    }
    argument->admiral = Handle<Admiral>(admiral);
    return true;
}

bool read_from(pn::file_view in, argumentType::AlterAge* argument) {
    int32_t minimum, range;
    if (!in.read(&argument->relative, &minimum, &range)) {
        return false;
    }
    argument->minimum = ticks(minimum);
    argument->range   = ticks(range);
    return true;
}

bool read_from(pn::file_view in, argumentType::AlterLocation* argument) {
    return in.read(&argument->relative, &argument->by);
}

bool read_from(pn::file_view in, argumentType::MakeSparks* argument) {
    return in.read(&argument->howMany, &argument->speed) &&
           read_from(in, &argument->velocityRange) && in.read(&argument->color);
}

bool read_from(pn::file_view in, argumentType::ReleaseEnergy* argument) {
    return read_from(in, &argument->percent);
}

bool read_from(pn::file_view in, argumentType::LandAt* argument) {
    return in.read(&argument->landingSpeed);
}

bool read_from(pn::file_view in, argumentType::EnterWarp* argument) {
    return read_from(in, &argument->warpSpeed);
}

bool read_from(pn::file_view in, argumentType::DisplayMessage* argument) {
    return in.read(&argument->resID, &argument->pageNum);
}

bool read_from(pn::file_view in, argumentType::ChangeScore* argument) {
    int32_t admiral;
    if (!in.read(&admiral, &argument->whichScore, &argument->amount)) {
        return false;
    }
    argument->whichPlayer = Handle<Admiral>(admiral);
    return true;
}

bool read_from(pn::file_view in, argumentType::DeclareWinner* argument) {
    int32_t admiral;
    int32_t text_id;
    if (!in.read(&admiral, &argument->nextLevel, &text_id)) {
        return false;
    }
    argument->whichPlayer = Handle<Admiral>(admiral);
    argument->text        = Resource::text(text_id).string().copy();
    return true;
}

bool read_from(pn::file_view in, argumentType::KillObject* argument) {
    return in.read(&argument->dieType);
}

bool read_from(pn::file_view in, argumentType::ColorFlash* argument) {
    return in.read(&argument->length, &argument->color, &argument->shade);
}

bool read_from(pn::file_view in, argumentType::Keys* argument) {
    return in.read(&argument->keyMask);
}

bool read_from(pn::file_view in, argumentType::Zoom* argument) {
    return in.read(&argument->zoomLevel);
}

bool read_from(pn::file_view in, argumentType::ComputerSelect* argument) {
    return in.read(&argument->screenNumber, &argument->lineNumber);
}

bool read_from(pn::file_view in, argumentType::AssumeInitial* argument) {
    return in.read(&argument->whichInitialObject);
}

}  // namespace

bool read_from(pn::file_view in, Action* action) {
    uint8_t section[24];

    uint8_t verb;
    if (!in.read(&verb)) {
        return false;
    }
    action->verb = verb << 8;

    if (!in.read(&action->reflexive, &action->inclusiveFilter, &action->exclusiveFilter)) {
        return false;
    }
    if (action->exclusiveFilter == 0xffffffff) {
        action->levelKeyTag = (action->inclusiveFilter & kLevelKeyTag) >> kLevelKeyTagShift;
    } else {
        action->levelKeyTag = 0;
    }
    uint32_t delay;
    if (!in.read(
                &action->owner, &delay, &action->initialSubjectOverride,
                &action->initialDirectOverride)) {
        return false;
    }
    action->delay = ticks(delay);
    char ignore[4];
    if (fread(ignore, 1, 4, in.c_obj()) < 4) {
        return false;
    }
    if (fread(section, 1, 24, in.c_obj()) < 24) {
        return false;
    }

    pn::file sub = pn::data_view{section, 24}.open();
    switch (action->verb) {
        case kNoAction:
        case kSetDestination:
        case kActivateSpecial:
        case kActivatePulse:
        case kActivateBeam:
        case kNilTarget: return true;

        case kCreateObject:
        case kCreateObjectSetDest: return read_from(sub, &action->argument.createObject);

        case kPlaySound: return read_from(sub, &action->argument.playSound);

        case kAlter: {
            uint8_t alter;
            if (!sub.read(&alter)) {
                return false;
            }
            action->verb |= alter;
            switch (action->verb) {
                case kAlterDamage: return read_from(sub, &action->argument.alterDamage);
                case kAlterEnergy: return read_from(sub, &action->argument.alterEnergy);
                case kAlterHidden: return read_from(sub, &action->argument.alterHidden);
                case kAlterSpin: return read_from(sub, &action->argument.alterSpin);
                case kAlterOffline: return read_from(sub, &action->argument.alterOffline);
                case kAlterVelocity: return read_from(sub, &action->argument.alterVelocity);
                case kAlterMaxVelocity: return read_from(sub, &action->argument.alterMaxVelocity);
                case kAlterThrust: return read_from(sub, &action->argument.alterThrust);
                case kAlterBaseType: return read_from(sub, &action->argument.alterBaseType);
                case kAlterOwner: return read_from(sub, &action->argument.alterOwner);
                case kAlterConditionTrueYet:
                    return read_from(sub, &action->argument.alterConditionTrueYet);
                case kAlterOccupation: return read_from(sub, &action->argument.alterOccupation);
                case kAlterAbsoluteCash:
                    return read_from(sub, &action->argument.alterAbsoluteCash);
                case kAlterAge: return read_from(sub, &action->argument.alterAge);
                case kAlterLocation: return read_from(sub, &action->argument.alterLocation);
                case kAlterAbsoluteLocation:
                    return read_from(sub, &action->argument.alterAbsoluteLocation);
                case kAlterWeapon1:
                case kAlterWeapon2:
                case kAlterSpecial: return read_from(sub, &action->argument.alterWeapon);
                default: return true;
            }
        }

        case kMakeSparks: return read_from(sub, &action->argument.makeSparks);

        case kReleaseEnergy: return read_from(sub, &action->argument.releaseEnergy);

        case kLandAt: return read_from(sub, &action->argument.landAt);

        case kEnterWarp: return read_from(sub, &action->argument.enterWarp);

        case kDisplayMessage: return read_from(sub, &action->argument.displayMessage);

        case kChangeScore: return read_from(sub, &action->argument.changeScore);

        case kDeclareWinner: return read_from(sub, &action->argument.declareWinner);

        case kDie: return read_from(sub, &action->argument.killObject);

        case kColorFlash: return read_from(sub, &action->argument.colorFlash);

        case kDisableKeys:
        case kEnableKeys: return read_from(sub, &action->argument.keys);

        case kSetZoom: return read_from(sub, &action->argument.zoom);

        case kComputerSelect: return read_from(sub, &action->argument.computerSelect);

        case kAssumeInitialObject: return read_from(sub, &action->argument.assumeInitial);

        default: return true;
    }
}

}  // namespace antares

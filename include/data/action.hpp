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

#ifndef ANTARES_DATA_ACTION_HPP_
#define ANTARES_DATA_ACTION_HPP_

#include <stdint.h>
#include <sfz/sfz.hpp>

#include "data/handle.hpp"
#include "math/fixed.hpp"
#include "math/geometry.hpp"
#include "math/units.hpp"

namespace antares {

class SpaceObject;

enum objectVerbIDEnum {
    kNoAction            = 0 << 8,
    kCreateObject        = 1 << 8,
    kPlaySound           = 2 << 8,
    kAlter               = 3 << 8,
    kMakeSparks          = 4 << 8,
    kReleaseEnergy       = 5 << 8,
    kLandAt              = 6 << 8,
    kEnterWarp           = 7 << 8,
    kDisplayMessage      = 8 << 8,
    kChangeScore         = 9 << 8,
    kDeclareWinner       = 10 << 8,
    kDie                 = 11 << 8,
    kSetDestination      = 12 << 8,
    kActivateSpecial     = 13 << 8,
    kActivatePulse       = 14 << 8,
    kActivateBeam        = 15 << 8,
    kColorFlash          = 16 << 8,
    kCreateObjectSetDest = 17 << 8,  // creates an object with the same destination as anObject's
                                     // (either subject or direct)
    kNilTarget           = 18 << 8,
    kDisableKeys         = 19 << 8,
    kEnableKeys          = 20 << 8,
    kSetZoom             = 21 << 8,
    kComputerSelect      = 22 << 8,  // selects a line & screen of the minicomputer
    kAssumeInitialObject = 23 << 8,  // assumes the identity of an intial object; for tutorial
};

enum alterVerbIDType {
    kAlterDamage      = kAlter | 0,
    kAlterVelocity    = kAlter | 1,
    kAlterThrust      = kAlter | 2,
    kAlterMaxThrust   = kAlter | 3,
    kAlterMaxVelocity = kAlter | 4,
    kAlterMaxTurnRate = kAlter | 5,
    kAlterLocation    = kAlter | 6,
    kAlterScale       = kAlter | 7,
    kAlterWeapon1     = kAlter | 8,
    kAlterWeapon2     = kAlter | 9,
    kAlterSpecial     = kAlter | 10,
    kAlterEnergy      = kAlter | 11,
    kAlterOwner       = kAlter | 12,
    kAlterHidden      = kAlter | 13,
    kAlterCloak       = kAlter | 14,
    kAlterOffline     = kAlter | 15,
    kAlterSpin        = kAlter | 16,
    kAlterBaseType    = kAlter | 17,
    kAlterConditionTrueYet =
            kAlter | 18,  // relative = state, min = which condition basically force to recheck
    kAlterOccupation = kAlter | 19,  // for special neutral death objects
    kAlterAbsoluteCash =
            kAlter |
            20,  // relative: true = cash to object : false = range = admiral who gets cash
    kAlterAge              = kAlter | 21,
    kAlterAttributes       = kAlter | 22,
    kAlterLevelKeyTag      = kAlter | 23,
    kAlterOrderKeyTag      = kAlter | 24,
    kAlterEngageKeyTag     = kAlter | 25,
    kAlterAbsoluteLocation = kAlter | 26,
};

enum dieVerbIDEnum { kDieNone = 0, kDieExpire = 1, kDieDestroy = 2 };
typedef uint8_t dieVerbIDType;

//
// Action:
//  Defines any action that an object can take.  Conditions that can cause an action to execute
//  are:
//  destroy, expire, create, collide, activate, or message.
//

union argumentType {
    argumentType() {}

    // createObject: make another type of object appear
    struct CreateObject {
        Handle<BaseObject> whichBaseType;      // what type
        int32_t            howManyMinimum;     // # to make min
        int32_t            howManyRange;       // # to make range
        uint8_t            velocityRelative;   // is velocity relative to creator?
        uint8_t            directionRelative;  // determines initial heading
        int32_t randomDistance;  // if not 0, then object will be created in random direction from
                                 // 0 to this away
    };
    CreateObject createObject;

    // playSound: play a sound effect
    struct PlaySound {
        uint8_t priority;
        ticks   persistence;
        uint8_t absolute;  // not distanced
        int32_t volumeMinimum;
        int32_t volumeRange;
        int32_t idMinimum;
        int32_t idRange;
    };
    PlaySound playSound;

    struct AlterSimple {
        int32_t amount;
    };
    struct AlterSimple alterDamage;
    struct AlterSimple alterEnergy;
    struct AlterSimple alterOccupation;

    struct AlterWeapon {
        Handle<BaseObject> base;
    } alterWeapon;

    struct AlterFixedRange {
        Fixed minimum, range;
    };
    AlterFixedRange alterSpin;
    AlterFixedRange alterOffline;

    struct AlterAge {
        bool  relative;
        ticks minimum, range;
    } alterAge;

    struct AlterThrust {
        bool  relative;
        Fixed minimum, range;
    } alterThrust;

    struct AlterMaxVelocity {
        Fixed amount;
    } alterMaxVelocity;

    struct AlterOwner {
        bool            relative;
        Fixed           amount;
        Handle<Admiral> admiral;
    } alterOwner;

    struct AlterCash {
        bool            relative;
        Fixed           amount;
        Handle<Admiral> admiral;
    } alterAbsoluteCash;

    struct AlterVelocity {
        bool  relative;
        Fixed amount;
    } alterVelocity;

    struct AlterBaseType {
        bool               keep_ammo;
        Handle<BaseObject> base;
    } alterBaseType;

    struct AlterLocation {
        bool    relative;
        int32_t by;
    } alterLocation;

    struct AlterAbsoluteLocation {
        bool  relative;
        Point at;
    } alterAbsoluteLocation;

    struct AlterHidden {
        int32_t first;
        int32_t count_minus_1;
    } alterHidden;

    struct AlterConditionTrueYet {
        bool    true_yet;
        int32_t first;
        int32_t count_minus_1;
    } alterConditionTrueYet;

    // makeSpark
    struct MakeSparks {
        int32_t howMany;
        int32_t speed;
        Fixed   velocityRange;
        uint8_t color;
    };
    MakeSparks makeSparks;

    // release energy
    struct ReleaseEnergy {
        Fixed percent;
    };
    ReleaseEnergy releaseEnergy;

    // land at
    struct LandAt {
        int32_t landingSpeed;
    };
    LandAt landAt;

    // enter warp
    struct EnterWarp {
        Fixed warpSpeed;
    };
    EnterWarp enterWarp;

    // Display message
    struct DisplayMessage {
        int16_t resID;
        int16_t pageNum;
    };
    DisplayMessage displayMessage;

    // Change score
    struct ChangeScore {
        Handle<Admiral> whichPlayer;  // in scenario's terms; -1 = owner of executor of action
        int32_t         whichScore;   // each player can have many "scores"
        int32_t         amount;
    };
    ChangeScore changeScore;

    // Declare winner
    struct DeclareWinner {
        Handle<Admiral> whichPlayer;  // in scenario's terms; -1 = owner of executor of action
        int32_t         nextLevel;    // -1 = none
        int32_t         textID;       // id of "debriefing" text
    };
    DeclareWinner declareWinner;

    // killObject: cause object to expire
    struct KillObject {
        dieVerbIDType dieType;
    };
    KillObject killObject;

    // colorFlash: flash whole screen to a color
    struct ColorFlash {
        int32_t length;  // length of color flash
        uint8_t color;   // color of flash
        uint8_t shade;   // brightness of flash
    };
    ColorFlash colorFlash;

    // keys: disable or enable keys/ for tutorial
    struct Keys {
        uint32_t keyMask;
    };
    Keys keys;

    // zoomLevel; manually set zoom level
    struct Zoom {
        int32_t zoomLevel;
    };
    Zoom zoom;

    struct ComputerSelect {
        int32_t screenNumber;
        int32_t lineNumber;
    };
    ComputerSelect computerSelect;

    struct AssumeInitial {
        int32_t whichInitialObject;
    };
    AssumeInitial assumeInitial;
};

void read_from(sfz::ReadSource in, argumentType::CreateObject& argument);
void read_from(sfz::ReadSource in, argumentType::PlaySound& argument);
void read_from(sfz::ReadSource in, argumentType::AlterSimple& argument);
void read_from(sfz::ReadSource in, argumentType::AlterWeapon& argument);
void read_from(sfz::ReadSource in, argumentType::MakeSparks& argument);
void read_from(sfz::ReadSource in, argumentType::ReleaseEnergy& argument);
void read_from(sfz::ReadSource in, argumentType::LandAt& argument);
void read_from(sfz::ReadSource in, argumentType::EnterWarp& argument);
void read_from(sfz::ReadSource in, argumentType::DisplayMessage& argument);
void read_from(sfz::ReadSource in, argumentType::ChangeScore& argument);
void read_from(sfz::ReadSource in, argumentType::DeclareWinner& argument);
void read_from(sfz::ReadSource in, argumentType::KillObject& argument);
void read_from(sfz::ReadSource in, argumentType::ColorFlash& argument);
void read_from(sfz::ReadSource in, argumentType::Keys& argument);
void read_from(sfz::ReadSource in, argumentType::Zoom& argument);
void read_from(sfz::ReadSource in, argumentType::ComputerSelect& argument);
void read_from(sfz::ReadSource in, argumentType::AssumeInitial& argument);

struct Action {
    static Action* get(int number);

    uint16_t verb;

    uint8_t  reflexive;        // does it apply to object executing verb?
    uint32_t inclusiveFilter;  // if it has ALL these attributes, OK -- for non-reflective verbs
    uint32_t exclusiveFilter;  // don't execute if it has ANY of these
    uint8_t  levelKeyTag;
    int16_t  owner;  // 0 no matter, 1 same owner, -1 different owner
    ticks    delay;
    //  uint32_t                    reserved1;
    int16_t      initialSubjectOverride;
    int16_t      initialDirectOverride;
    uint32_t     reserved2;
    argumentType argument;

    static const size_t byte_size = 48;
};
void read_from(sfz::ReadSource in, Action& action);

}  // namespace antares

#endif  // ANTARES_DATA_ACTION_HPP_

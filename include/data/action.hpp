// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// Copyright (C) 2015 The Antares Authors
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
#include "math/units.hpp"
#include "math/geometry.hpp"

namespace antares {

struct SpaceObject;

enum objectVerbIDEnum {
    kNoAction               = 0,
    kCreateObject           = 1,
    kPlaySound              = 2,
    kAlter                  = 3,
    kMakeSparks             = 4,
    kReleaseEnergy          = 5,
    kLandAt                 = 6,
    kEnterWarp              = 7,
    kDisplayMessage         = 8,
    kChangeScore            = 9,
    kDeclareWinner          = 10,
    kDie                    = 11,
    kSetDestination         = 12,
    kActivateSpecial        = 13,
    kActivatePulse          = 14,
    kActivateBeam           = 15,
    kColorFlash             = 16,
    kCreateObjectSetDest    = 17,  // creates an object with the same destination as anObject's
                                   // (either subject or direct)
    kNilTarget              = 18,
    kDisableKeys            = 19,
    kEnableKeys             = 20,
    kSetZoom                = 21,
    kComputerSelect         = 22,  // selects a line & screen of the minicomputer
    kAssumeInitialObject    = 23,  // assumes the identity of an intial object; for tutorial
};
typedef uint8_t objectVerbIDType;

enum alterVerbIDType {
    kAlterDamage            = 0,
    kAlterVelocity          = 1,
    kAlterThrust            = 2,
    kAlterMaxThrust         = 3,
    kAlterMaxVelocity       = 4,
    kAlterMaxTurnRate       = 5,
    kAlterLocation          = 6,
    kAlterScale             = 7,
    kAlterWeapon1           = 8,
    kAlterWeapon2           = 9,
    kAlterSpecial           = 10,
    kAlterEnergy            = 11,
    kAlterOwner             = 12,
    kAlterHidden            = 13,
    kAlterCloak             = 14,
    kAlterOffline           = 15,
    kAlterSpin              = 16,
    kAlterBaseType          = 17,
    kAlterConditionTrueYet  = 18,  // relative = state, min = which condition basically force to recheck
    kAlterOccupation        = 19,  // for special neutral death objects
    kAlterAbsoluteCash      = 20,  // relative: true = cash to object : false = range = admiral who gets cash
    kAlterAge               = 21,
    kAlterAttributes        = 22,
    kAlterLevelKeyTag       = 23,
    kAlterOrderKeyTag       = 24,
    kAlterEngageKeyTag      = 25,
    kAlterAbsoluteLocation  = 26,
};

enum dieVerbIDEnum {
    kDieNone = 0,
    kDieExpire = 1,
    kDieDestroy = 2
};
typedef uint8_t dieVerbIDType;

//
// Action:
//  Defines any action that an object can take.  Conditions that can cause an action to execute are:
//  destroy, expire, create, collide, activate, or message.
//

union argumentType {
    argumentType() { }

    // createObject: make another type of object appear
    struct CreateObject {
        Handle<BaseObject>      whichBaseType;      // what type
        int32_t                 howManyMinimum;     // # to make min
        int32_t                 howManyRange;       // # to make range
        uint8_t                 velocityRelative;   // is velocity relative to creator?
        uint8_t                 directionRelative;  // determines initial heading
        int32_t                 randomDistance;     // if not 0, then object will be created in random direction from 0 to this away
    };
    CreateObject createObject;

    // playSound: play a sound effect
    struct PlaySound {
        uint8_t                 priority;
        ticks                   persistence;
        uint8_t                 absolute;           // not distanced
        int32_t                 volumeMinimum;
        int32_t                 volumeRange;
        int32_t                 idMinimum;
        int32_t                 idRange;
    };
    PlaySound playSound;

    // alterObject: change some attribute of an object
    struct AlterObject {
        uint8_t                 alterType;
        uint8_t                 relative;
        int32_t                 minimum;
        int32_t                 range;
    };
    AlterObject alterObject;

    // makeSpark
    struct MakeSparks {
        int32_t                 howMany;
        int32_t                 speed;
        Fixed                   velocityRange;
        uint8_t                 color;
    };
    MakeSparks makeSparks;

    // release energy
    struct ReleaseEnergy {
        Fixed                   percent;
    };
    ReleaseEnergy releaseEnergy;

    // land at
    struct LandAt {
        int32_t                 landingSpeed;
    };
    LandAt landAt;

    // enter warp
    struct EnterWarp {
        Fixed                   warpSpeed;
    };
    EnterWarp enterWarp;

    // Display message
    struct DisplayMessage {
        int16_t                 resID;
        int16_t                 pageNum;
    };
    DisplayMessage displayMessage;

    // Change score
    struct ChangeScore {
        Handle<Admiral>         whichPlayer;    // in scenario's terms; -1 = owner of executor of action
        int32_t                 whichScore;     // each player can have many "scores"
        int32_t                 amount;
    };
    ChangeScore changeScore;

    // Declare winner
    struct DeclareWinner {
        Handle<Admiral>         whichPlayer;    // in scenario's terms; -1 = owner of executor of action
        int32_t                 nextLevel;      // -1 = none
        int32_t                 textID;         // id of "debriefing" text
    };
    DeclareWinner declareWinner;

    // killObject: cause object to expire
    struct KillObject {
        dieVerbIDType           dieType;
    };
    KillObject killObject;

    // colorFlash: flash whole screen to a color
    struct ColorFlash {
        int32_t                 length;         // length of color flash
        uint8_t                 color;          // color of flash
        uint8_t                 shade;          // brightness of flash
    };
    ColorFlash colorFlash;

    // keys: disable or enable keys/ for tutorial
    struct Keys {
        uint32_t                keyMask;
    };
    Keys keys;

    // zoomLevel; manually set zoom level
    struct Zoom {
        int32_t                 zoomLevel;
    };
    Zoom zoom;

    struct ComputerSelect {
        int32_t                 screenNumber;
        int32_t                 lineNumber;
    };
    ComputerSelect computerSelect;

    struct AssumeInitial {
        int32_t                 whichInitialObject;
    };
    AssumeInitial assumeInitial;
};
void read_from(sfz::ReadSource in, argumentType::CreateObject& argument);
void read_from(sfz::ReadSource in, argumentType::PlaySound& argument);
void read_from(sfz::ReadSource in, argumentType::AlterObject& argument);
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

    objectVerbIDType            verb;                   // what is this verb?
    uint8_t                     reflexive;              // does it apply to object executing verb?
    uint32_t                    inclusiveFilter;        // if it has ALL these attributes, OK -- for non-reflective verbs
    uint32_t                    exclusiveFilter;        // don't execute if it has ANY of these
    uint8_t                     levelKeyTag;
    int16_t                     owner;                  // 0 no matter, 1 same owner, -1 different owner
    ticks                       delay;
//  uint32_t                    reserved1;
    int16_t                     initialSubjectOverride;
    int16_t                     initialDirectOverride;
    uint32_t                    reserved2;
    argumentType                argument;

    static const size_t byte_size = 48;
};
void read_from(sfz::ReadSource in, Action& action);

}  // namespace antares

#endif // ANTARES_DATA_ACTION_HPP_

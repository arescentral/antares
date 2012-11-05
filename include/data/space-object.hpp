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

#ifndef ANTARES_DATA_SPACE_OBJECT_HPP_
#define ANTARES_DATA_SPACE_OBJECT_HPP_

#include "drawing/color.hpp"
#include "drawing/sprite-handling.hpp"
#include "game/beam.hpp"
#include "game/globals.hpp"
#include "math/fixed.hpp"
#include "sound/fx.hpp"

namespace antares {

const int32_t kMaxSpaceObject           = 250;

const int32_t kTimeToCheckHome          = 900;

const int32_t kEnergyPodAmount          = 500;  // average (calced) of 500 energy units/pod

const int32_t kWarpAcceleration         = 1;  // how fast we warp in & out

const int32_t kNoWeapon                 = -1;
const int32_t kMaxWeaponPosition        = 3;

const int32_t kNoShip                   = -1;
const int32_t kNoDestinationCoord       = 0;
const int32_t kNoDestinationObject      = -1;
const int32_t kNoOwner                  = -1;

const int16_t kObjectInUse              = 1;
const int16_t kObjectToBeFreed          = 2;
const int16_t kObjectAvailable          = 0;

const int32_t kNoClass                  = -1;

// any class > 10000 = direct map to base object - 10000
const int32_t kLiteralClass             = 10000;

const int32_t kHitStateMax              = 128;
const int32_t kCloakOnStateMax          = 254;
const int32_t kCloakOffStateMax         = -252;

const int16_t kSpaceObjectNameResID         = 5000;
const int16_t kSpaceObjectShortNameResID    = 5001;

const int32_t kEngageRange              = 1048576;  // range at which to engage closest ship
                                                    // about 2 subsectors (512 * 2)^2

enum {
    kCanTurn                = 0x00000001,  // we have to worry about its rotation velocity
    kCanBeEngaged           = 0x00000002,  // if it's worth going after
    kHasDirectionGoal       = 0x00000004,  // we must turn it towards its goal
    kIsRemote               = 0x00000008,  // is controlled by remote computer
    kIsHumanControlled      = 0x00000010,  // human is controlling it
    kIsBeam                 = 0x00000020,  // a vector shot, no sprite
    kDoesBounce             = 0x00000040,  // when it hits the edge, it bounces FORMER: can't move, so don't try
    kIsSelfAnimated         = 0x00000080,  // cycles through animation frames
    kShapeFromDirection     = 0x00000100,  // its appearence is based on its direction
    kIsPlayerShip           = 0x00000200,  // this is the ship we focus on
    kCanBeDestination       = 0x00000400,  // can be selected as a place to go to
    kCanEngage              = 0x00000800,  // can go into engage mode
    kCanEvade               = 0x00001000,  // can go into evade mode
    kCanAcceptMessages      = 0x00002000,  // can accept a message from player
    kCanAcceptBuild         = 0x00004000,  // can accept a build message
    kCanAcceptDestination   = 0x00008000,  // can accept destination message
    kAutoTarget             = 0x00010000,  // for creating a material weapon, born at parent's target angle
    kAnimationCycle         = 0x00020000,  // repeating animation
    kCanCollide             = 0x00040000,  // can collide with another object
    kCanBeHit               = 0x00080000,  // can be on the receving end of a collision
    kIsDestination          = 0x00100000,  // is a point of interest to which ships can be sent
    kHideEffect             = 0x00200000,  // hides other objects around it
    kReleaseEnergyOnDeath   = 0x00400000,  // when destroyed, remaining energy released
    kHated                  = 0x00800000,  // when you don't own it, if it's hated, you shoot it
    kOccupiesSpace          = 0x01000000,  // 2 objects cannot occupy same physical space
    kStaticDestination      = 0x02000000,  // destination cannot be altered
    kCanBeEvaded            = 0x04000000,  // it can be veered away from
    kNeutralDeath           = 0x08000000,  // if true, object becomes neutral when "destroyed" but doesn't die
    kIsGuided               = 0x10000000,  // doesn't really think; can't accept orders; if not yours, it is feared
    kAppearOnRadar          = 0x20000000,  // shows up on radar
    kBit31                  = 0x40000000,
    kOnAutoPilot            = 0x80000000,  // if human controlled, this temporarily gives the computer control

    kCanThink               = kCanEngage | kCanEvade | kCanAcceptDestination,  // not just "dumb"
    kConsiderDistance       = kCanThink | kCanBeDestination,
    kPotentialTarget        = kCanBeEngaged | kCanBeEvaded,
    kRemoteOrHuman          = kIsPlayerShip,

    // for baseObjectTypes only: these bits have different functions in
    // baseObjectTypes than they do in normal spaceObjectTypes.  However, they
    // should never be turned on permanently!
    kHaveCheckedMedia       = 0x00000008,  // we've checked its sounds & sprites

    // for initial objects only
    // kInitiallyExists should NOT be carried over to real objects!
    kInitiallyHidden        = 0x00000020,  // does it exist at first, or is it turned on later?
    kFixedRace              = 0x00000010,  // don't change this object even if owner's race is different
    kInitialAttributesMask  = kFixedRace | kInitiallyHidden,
};

enum {
    kUseForTransportation   = 0x00000001,  // use when we're going to our destination
    kUseForAttacking        = 0x00000002,  // use when we've got a target our sites
    kUseForDefense          = 0x00000004,  // use when we're running/evading
};

enum {
    kUncapturedBaseExists       = 0x00000001,
    kSufficientEscortsExist     = 0x00000002,
    kThisBaseNeedsProtection    = 0x00000004,
    kFriendUpTrend              = 0x00000008,
    kFriendDownTrend            = 0x00000010,
    kFoeUpTrend                 = 0x00000020,
    kFoeDownTrend               = 0x00000040,
    kMatchingFoeExists          = 0x00000080,  // unowned object with same level-key exists
    kBuildFlagBit9              = 0x00000100,
    kBuildFlagBit10             = 0x00000200,
    kBuildFlagBit11             = 0x00000400,
    kBuildFlagBit12             = 0x00000800,
    kBuildFlagBit13             = 0x00001000,
    kBuildFlagBit14             = 0x00002000,
    kBuildFlagBit15             = 0x00004000,
    kBuildFlagBit16             = 0x00008000,
    kBuildFlagBit17             = 0x00010000,
    kBuildFlagBit18             = 0x00020000,
    kBuildFlagBit19             = 0x00040000,
    kBuildFlagBit20             = 0x00080000,
    kBuildFlagBit21             = 0x00100000,
    kBuildFlagBit22             = 0x00200000,
    kOnlyEngagedBy              = 0x00400000,
    kCanOnlyEngage              = 0x00800000,
    kEngageKeyTag1              = 0x01000000,
    kEngageKeyTag2              = 0x02000000,
    kEngageKeyTag3              = 0x04000000,
    kEngageKeyTag4              = 0x08000000,
    kLevelKeyTag1               = 0x10000000,
    kLevelKeyTag2               = 0x20000000,
    kLevelKeyTag3               = 0x40000000,
    kLevelKeyTag4               = 0x80000000,
};

//
// Well, this is a hack. If a verb's exclusive filter == 0xffffffff
// then we treat the high four bits of the inclusive filter like
// a special tag, matching to the high four bits of an baseObject's
// build flag.
//
const uint32_t kLevelKeyTagMask      = 0xf0000000;
const uint32_t kEngageKeyTagMask     = 0x0f000000;
const int32_t kEngageKeyTagShift   = 4;

enum {
    kStrongerThanTarget         = 0x00000001,
    kTargetIsBase               = 0x00000002,
    kTargetIsNotBase            = 0x00000004,
    kTargetIsLocal              = 0x00000008,
    kTargetIsRemote             = 0x00000010,
    kOnlyEscortNotBase          = 0x00000020,
    kTargetIsFriend             = 0x00000040,
    kTargetIsFoe                = 0x00000080,
    kOrderFlagBit9              = 0x00000100,
    kOrderFlagBit10             = 0x00000200,
    kOrderFlagBit11             = 0x00000400,
    kOrderFlagBit12             = 0x00000800,
    kOrderFlagBit13             = 0x00001000,
    kOrderFlagBit14             = 0x00002000,
    kOrderFlagBit15             = 0x00004000,
    kOrderFlagBit16             = 0x00008000,
    kOrderFlagBit17             = 0x00010000,
    kOrderFlagBit18             = 0x00020000,
    kHardMatchingFriend         = 0x00040000,
    kHardMatchingFoe            = 0x00080000,
    kHardFriendlyEscortOnly     = 0x00100000,
    kHardNoFriendlyEscort       = 0x00200000,
    kHardTargetIsRemote         = 0x00400000,
    kHardTargetIsLocal          = 0x00800000,
    kHardTargetIsFoe            = 0x01000000,
    kHardTargetIsFriend         = 0x02000000,
    kHardTargetIsNotBase        = 0x04000000,
    kHardTargetIsBase           = 0x08000000,
    kOrderKeyTag1               = 0x10000000,
    kOrderKeyTag2               = 0x20000000,
    kOrderKeyTag3               = 0x40000000,
    kOrderKeyTag4               = 0x80000000,
};

// RUNTIME FLAG BITS
enum {
    kHasArrived     = 0x00000001,
    kTargetLocked   = 0x00000002,  // if some foe has locked on, you will be visible
    kIsCloaked      = 0x00000004,  // if you are near a naturally shielding object
    kIsHidden       = 0x00000008,  // overrides natural shielding
    kIsTarget       = 0x00000010,  // preserve target lock in case you become invisible
};

const uint32_t kPresenceDataHiWordMask = 0xffff0000;
const uint32_t kPresenceDataLoWordMask = 0x0000ffff;
const int32_t kPresenceDataHiWordShift = 16;

const uint32_t kPeriodicActionTimeMask  = 0xff000000;
const uint32_t kPeriodicActionRangeMask = 0x00ff0000;
const uint32_t kPeriodicActionNotMask   = 0x0000ffff;
const int32_t kPeriodicActionTimeShift  = 24;
const int32_t kPeriodicActionRangeShift = 16;

const uint32_t kDestroyActionNotMask        = 0x7fffffff;
const uint32_t kDestroyActionDontDieFlag    = 0x80000000;

struct spaceObjectType;
typedef spaceObjectType *spaceObjectTypePtr;

enum objectVerbIDEnum {
    kNoAction = 0,
    kCreateObject = 1,
    kPlaySound = 2,
    kAlter = 3,
    kMakeSparks = 4,
    kReleaseEnergy = 5,
    kLandAt = 6,
    kEnterWarp = 7,
    kDisplayMessage = 8,
    kChangeScore = 9,
    kDeclareWinner = 10,
    kDie = 11,
    kSetDestination = 12,
    kActivateSpecial = 13,
    kActivatePulse = 14,
    kActivateBeam = 15,
    kColorFlash = 16,
    kCreateObjectSetDest = 17,      // creates an object with the same destination as anObject's (either subject or direct)
    kNilTarget = 18,
    kDisableKeys = 19,
    kEnableKeys = 20,
    kSetZoom = 21,
    kComputerSelect = 22,           // selects a line & screen of the minicomputer
    kAssumeInitialObject = 23       // assumes the identity of an intial object; for tutorial
};
typedef uint8_t objectVerbIDType;

enum alterVerbIDType {
    kAlterDamage = 0,
    kAlterVelocity = 1,
    kAlterThrust = 2,
    kAlterMaxThrust = 3,
    kAlterMaxVelocity = 4,
    kAlterMaxTurnRate = 5,
    kAlterLocation = 6,
    kAlterScale = 7,
    kAlterWeapon1 = 8,
    kAlterWeapon2 = 9,
    kAlterSpecial = 10,
    kAlterEnergy = 11,
    kAlterOwner = 12,
    kAlterHidden = 13,
    kAlterCloak = 14,
    kAlterOffline = 15,
    kAlterSpin = 16,
    kAlterBaseType = 17,
    kAlterConditionTrueYet = 18,    // relative = state, min = which condition basically force to recheck
    kAlterOccupation = 19,          // for special neutral death objects
    kAlterAbsoluteCash = 20,        // relative: true = cash to object : false = range = admiral who gets cash
    kAlterAge = 21,
    kAlterAttributes = 22,
    kAlterLevelKeyTag = 23,
    kAlterOrderKeyTag = 24,
    kAlterEngageKeyTag = 25,
    kAlterAbsoluteLocation = 26
};

enum dieVerbIDEnum {
    kDieNone = 0,
    kDieExpire = 1,
    kDieDestroy = 2
};
typedef uint8_t dieVerbIDType;

enum kPresenceStateType {
    kNormalPresence = 0,
    kLandingPresence = 1,
    kTakeoffPresence = 2,
    kWarpInPresence = 3,
    kWarpingPresence = 4,
    kWarpOutPresence = 5
};

//
// objectActionType:
//  Defines any action that an object can take.  Conditions that can cause an action to execute are:
//  destroy, expire, create, collide, activate, or message.
//

union argumentType {

    // createObject: make another type of object appear
    struct CreateObject {
        int32_t                 whichBaseType;      // what type
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
        int32_t                 persistence;
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
        int32_t                 whichPlayer;    // in scenario's terms; -1 = owner of executor of action
        int32_t                 whichScore;     // each player can have many "scores"
        int32_t                 amount;
    };
    ChangeScore changeScore;

    // Declare winner
    struct DeclareWinner {
        int32_t                 whichPlayer;    // in scenario's terms; -1 = owner of executor of action
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

struct objectActionType {
    objectVerbIDType            verb;                   // what is this verb?
    uint8_t                     reflexive;              // does it apply to object executing verb?
    uint32_t                    inclusiveFilter;        // if it has ALL these attributes, OK -- for non-reflective verbs
    uint32_t                    exclusiveFilter;        // don't execute if it has ANY of these
    int16_t                     owner;                  // 0 no matter, 1 same owner, -1 different owner
    uint32_t                    delay;
//  unsigned long               reserved1;
    int16_t                     initialSubjectOverride;
    int16_t                     initialDirectOverride;
    uint32_t                    reserved2;
    argumentType                argument;

    static const size_t byte_size = 48;
};
void read_from(sfz::ReadSource in, objectActionType& action);

union objectFrameType {
    // rotation: for objects whose shapes depend on their direction
    struct Rotation {
        int32_t                 shapeOffset;        // offset for 1st shape
        int32_t                 rotRes;             // ROT_POS / rotRes = # of discrete shapes
        Fixed                   maxTurnRate;        // max rate at which object can turn
        Fixed                   turnAcceleration;   // rate at which object reaches maxTurnRate
    };
    Rotation rotation;

    // animation: objects whose appearence does not depend on direction
    struct Animation {
        int32_t                 firstShape;         // first shape in range
        int32_t                 lastShape;          // last shape (inclusive)

        int32_t                 frameDirection;     // direction (either -1, 0, or 1)
        int32_t                 frameDirectionRange;    // either 0, 1, or 2

        int32_t                 frameSpeed;         // speed at which object animates
        int32_t                 frameSpeedRange;    // random addition to speed

        int32_t                 frameShape;         // starting shape #
        int32_t                 frameShapeRange;    // random addition to starting shape #
    };
    Animation animation;

    // beam: have no associated sprite
    struct Beam {
        uint8_t                 color;              // color of beam
        beamKindType            kind;
        int32_t                 accuracy;           // for non-normal beams, how accurate
        int32_t                 range;
    };
    Beam beam;

    // weapon: weapon objects have no physical form, and can only be activated
    struct Weapon {
        uint32_t                usage;              // when is this used?
        int32_t                 energyCost;         // cost to fire
        int32_t                 fireTime;           // time between shots
        int32_t                 ammo;               // initial ammo
        int32_t                 range;              // range (= age * max velocity)
        int32_t                 inverseSpeed;       // for AI = 1/max velocity
        int32_t                 restockCost;        // energy to make new ammo
    };
    Weapon weapon;
};
void read_from(sfz::ReadSource in, objectFrameType::Rotation& rotation);
void read_from(sfz::ReadSource in, objectFrameType::Animation& animation);
void read_from(sfz::ReadSource in, objectFrameType::Beam& beam);
void read_from(sfz::ReadSource in, objectFrameType::Weapon& weapon);

struct baseObjectType {
    uint32_t                attributes;                 // initial attributes (see flags)
    int32_t                 baseClass;
    int32_t                 baseRace;
    int32_t                 price;

    Fixed                   offenseValue;
//  Fixed                   defenseValue;
    int32_t                 destinationClass;           // for computer

    Fixed                   maxVelocity;                // maximum speed
    Fixed                   warpSpeed;                  // multiplier of speed at warp (0 if cannot)
    uint32_t                warpOutDistance;                // distance at which to come out of warp

    Fixed                   initialVelocity;            // initial minimum velocity (usually relative)
    Fixed                   initialVelocityRange;       // random addition to initial velocity

    Fixed                   mass;                       // how quickly thrust acheives max
    Fixed                   maxThrust;                  // maximum amount of thrust

    int32_t                 health;                     // starting health
    int32_t                 damage;                     // damage caused by impact
    int32_t                 energy;                     // starting energy for material objects

    int32_t                 initialAge;                 // starting minimum age
    int32_t                 initialAgeRange;            // random addition to starting age --
                                                        // for neutral death objects =
                                                        // size of occupying force (HACK)

    int32_t                 naturalScale;               // natural scale relative to %100

    int16_t                 pixLayer;                   // 0 = no layer 1->3 = back to front
    int16_t                 pixResID;                   // resID of SMIV
    int32_t                 tinySize;                   // size of representation on radar (0 = 1 pixel)
    uint8_t                 shieldColor;                // color on radar (0 = don't put on radar)

    int32_t                 initialDirection;           // initial direction (usually relative)
    int32_t                 initialDirectionRange;      // random addition to initial direction

    int32_t                 pulse;                      // pulse weapon baseObject #(kNoWeapon = none)
    int32_t                 beam;                       // beam weapon baseObject #
    int32_t                 special;                    // special weapon baseObject #

    int32_t                 pulsePositionNum;           // # of places from which pulse can fire
    int32_t                 beamPositionNum;            // # of places from which beam can fire
    int32_t                 specialPositionNum;         // # of places from which special can fire

    fixedPointType          pulsePosition[kMaxWeaponPosition];  // relative positions (unrotated) of fire points
    fixedPointType          beamPosition[kMaxWeaponPosition];
    fixedPointType          specialPosition[kMaxWeaponPosition];

    Fixed                   friendDefecit;
    Fixed                   dangerThreshold;
//  long                    pulseDirection;             // direction relative to shooter
//  long                    beamDirection;              // direction relative to shooter
    int32_t                 specialDirection;           // direction relative to shooter

    int32_t                 arriveActionDistance;               // distance^2 at which arrive action is triggered on dest

    int32_t                 destroyAction;  // what happens when object is destroyed
    int32_t                 destroyActionNum;
    int32_t                 expireAction;       // what happens when object expires
    int32_t                 expireActionNum;
    int32_t                 createAction;       // what happens when object is 1st made
    int32_t                 createActionNum;
    int32_t                 collideAction;  // what happens when object collides
    int32_t                 collideActionNum;
    int32_t                 activateAction; // what happens when object is activated
    int32_t                 activateActionNum;
    int32_t                 arriveAction;   // what happens when object arrives at destination
    int32_t                 arriveActionNum;

    objectFrameType         frame;

    uint32_t            buildFlags;
    uint32_t            orderFlags;
    Fixed               buildRatio;
    uint32_t            buildTime;
//  long                reserved1;
    uint8_t             skillNum;
    uint8_t             skillDen;
    uint8_t             skillNumAdj;
    uint8_t             skillDenAdj;
    int16_t             pictPortraitResID;
    int16_t             reserved2;
    int32_t             reserved3;
    int32_t             internalFlags;

    static const int byte_size = 318;
};
void read_from(sfz::ReadSource in, baseObjectType& object);

enum dutyType {
    eNoDuty =           0,
    eEscortDuty =       1,
    eGuardDuty =        2,
    eAssaultDuty =      3,
    eHostileBaseDuty =  4
};

//typedef beamTypeStruct;


struct spaceObjectType {
    unsigned long           attributes;
    baseObjectType          *baseType;
    long                    whichBaseObject;
    long                    entryNumber;            // major hack?

    unsigned long           keysDown;

    long                    tinySize;
    RgbColor                tinyColor;

    long                    direction;
    long                    directionGoal;
    Fixed                   turnVelocity;
    Fixed                   turnFraction;

    long                    offlineTime;

    coordPointType          location;
    coordPointType          lastLocation;
    long                    lastDir;
    spaceObjectTypePtr      collideObject;
    Point                   collisionGrid;
    spaceObjectTypePtr      nextNearObject;
    Point                   distanceGrid;
    spaceObjectTypePtr      nextFarObject;
    spaceObjectTypePtr      previousObject;
    long                    previousObjectNumber;
    spaceObjectTypePtr      nextObject;
    long                    nextObjectNumber;

    long                    runTimeFlags;       // distance from origin to destination
    coordPointType          destinationLocation;// coords of our destination ( or kNoDestination)
    long                    destinationObject;  // which object?  or kNoDestinationObject -- or, if we're a dest, our corresponding destBalance for AI
    spaceObjectTypePtr      destObjectPtr;      // ptr to destination object
    long                    destObjectDest;     // # of our destination's destination in case it dies
    long                    destObjectID;       // ID of our dest object
    long                    destObjectDestID;   // id of our dest's destination

    Fixed                   localFriendStrength;
    Fixed                   localFoeStrength;
    Fixed                   escortStrength;
    Fixed                   remoteFriendStrength;
    Fixed                   remoteFoeStrength;

    Fixed                   bestConsideredTargetValue;
    Fixed                   currentTargetValue;
    long                    bestConsideredTargetNumber;

    long                    timeFromOrigin;     // time it's been since we left
    fixedPointType          idealLocationCalc;  // calced when we got origin
    coordPointType          originLocation;     // coords of our origin

    fixedPointType          motionFraction;
    fixedPointType          velocity;
    Fixed                   thrust;
    Fixed                   maxVelocity;
    Point                   scaledCornerOffset;
    Point                   scaledSize;
    Rect                absoluteBounds;
    int32_t                 randomSeed;

    union
    {
//      struct
//      {
            long                directionGoal;
            Fixed               turnVelocity;
            Fixed               turnFraction;
//      } rotation;
        struct
        {
            long                thisShape;
            Fixed               frameFraction;
            long                frameDirection;
            Fixed               frameSpeed;
        } animation;
        struct
        {
            int32_t             whichBeam;
            beamType            *beam;
        } beam;
    } frame;

    long                    health;
    long                    energy;
    long                    battery;
    long                    owner;
    long                    age;
    long                    naturalScale;
    long                    id;
    short                   rechargeTime;
    short                   pulseCharge;
    short                   beamCharge;
    short                   specialCharge;
    short                   active;

    long                    warpEnergyCollected;

    short                   layer;
    spriteType              *sprite;
    long                    whichSprite;

    uint64_t                distanceFromPlayer;
    unsigned long           closestDistance;
    long                    closestObject;
    long                    targetObjectNumber;
    long                    targetObjectID;
    long                    targetAngle;
    long                    lastTarget;
    long                    lastTargetDistance;
    long                    longestWeaponRange;
    long                    shortestWeaponRange;
    long                    engageRange;            // either longestWeaponRange or kEngageRange

    kPresenceStateType      presenceState;
    long                    presenceData;

    long                    hitState;
    long                    cloakState;
    dutyType                duty;
    int                     pixResID;

    baseObjectType          *pulseBase;
    long                    pulseType;
    long                    pulseTime;
    long                    pulseAmmo;
    long                    pulsePosition;

    baseObjectType          *beamBase;
    long                    beamType;
    long                    beamTime;
    long                    beamAmmo;
    long                    beamPosition;

    baseObjectType          *specialBase;
    long                    specialType;
    long                    specialTime;
    long                    specialAmmo;
    long                    specialPosition;

    long                    periodicTime;
    long                    whichLabel;

    unsigned long           myPlayerFlag;
    unsigned long           seenByPlayerFlags;
    unsigned long           hostileTowardsFlags;

    unsigned char           shieldColor;
    unsigned char           originalColor;
};

}  // namespace antares

#endif // ANTARES_DATA_SPACE_OBJECT_HPP_

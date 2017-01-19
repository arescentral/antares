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

#ifndef ANTARES_DATA_BASE_OBJECT_HPP_
#define ANTARES_DATA_BASE_OBJECT_HPP_

#include "data/action.hpp"
#include "drawing/color.hpp"
#include "math/fixed.hpp"
#include "math/random.hpp"

namespace antares {

const int32_t kMaxSpaceObject = 250;

const ticks kTimeToCheckHome = secs(15);

const int32_t kEnergyPodAmount = 500;  // average (calced) of 500 energy units/pod

const int32_t kWarpAcceleration = 1;  // how fast we warp in & out

const int32_t kNoWeapon          = -1;
const int32_t kMaxWeaponPosition = 3;

const int32_t kNoShip              = -1;
const int32_t kNoDestinationCoord  = 0;
const int32_t kNoDestinationObject = -1;
const int32_t kNoOwner             = -1;

const int16_t kObjectInUse     = 1;
const int16_t kObjectToBeFreed = 2;
const int16_t kObjectAvailable = 0;

const int32_t kNoClass = -1;

// any class > 10000 = direct map to base object - 10000
const int32_t kLiteralClass = 10000;

const int32_t kHitStateMax      = 128;
const int32_t kCloakOnStateMax  = 254;
const int32_t kCloakOffStateMax = -252;

const int32_t kEngageRange = 1048576;  // range at which to engage closest ship
                                       // about 2 subsectors (512 * 2)^2

enum {
    kCanTurn           = 0x00000001,  // we have to worry about its rotation velocity
    kCanBeEngaged      = 0x00000002,  // if it's worth going after
    kHasDirectionGoal  = 0x00000004,  // we must turn it towards its goal
    kIsRemote          = 0x00000008,  // is controlled by remote computer
    kIsHumanControlled = 0x00000010,  // human is controlling it
    kIsVector          = 0x00000020,  // a vector shot, no sprite
    kDoesBounce =
            0x00000040,  // when it hits the edge, it bounces FORMER: can't move, so don't try
    kIsSelfAnimated       = 0x00000080,  // cycles through animation frames
    kShapeFromDirection   = 0x00000100,  // its appearence is based on its direction
    kIsPlayerShip         = 0x00000200,  // this is the ship we focus on
    kCanBeDestination     = 0x00000400,  // can be selected as a place to go to
    kCanEngage            = 0x00000800,  // can go into engage mode
    kCanEvade             = 0x00001000,  // can go into evade mode
    kCanAcceptMessages    = 0x00002000,  // can accept a message from player
    kCanAcceptBuild       = 0x00004000,  // can accept a build message
    kCanAcceptDestination = 0x00008000,  // can accept destination message
    kAutoTarget     = 0x00010000,  // for creating a material weapon, born at parent's target angle
    kAnimationCycle = 0x00020000,  // repeating animation
    kCanCollide     = 0x00040000,  // can collide with another object
    kCanBeHit       = 0x00080000,  // can be on the receving end of a collision
    kIsDestination  = 0x00100000,  // is a point of interest to which ships can be sent
    kHideEffect     = 0x00200000,  // hides other objects around it
    kReleaseEnergyOnDeath = 0x00400000,  // when destroyed, remaining energy released
    kHated                = 0x00800000,  // when you don't own it, if it's hated, you shoot it
    kOccupiesSpace        = 0x01000000,  // 2 objects cannot occupy same physical space
    kStaticDestination    = 0x02000000,  // destination cannot be altered
    kCanBeEvaded          = 0x04000000,  // it can be veered away from
    kNeutralDeath =
            0x08000000,  // if true, object becomes neutral when "destroyed" but doesn't die
    kIsGuided =
            0x10000000,  // doesn't really think; can't accept orders; if not yours, it is feared
    kAppearOnRadar = 0x20000000,  // shows up on radar
    kBit31         = 0x40000000,
    kOnAutoPilot = 0x80000000,  // if human controlled, this temporarily gives the computer control

    kCanThink         = kCanEngage | kCanEvade | kCanAcceptDestination,  // not just "dumb"
    kConsiderDistance = kCanThink | kCanBeDestination,
    kPotentialTarget  = kCanBeEngaged | kCanBeEvaded,
    kRemoteOrHuman    = kIsPlayerShip,

    // for initial objects only
    // kInitiallyExists should NOT be carried over to real objects!
    kInitiallyHidden = 0x00000020,  // does it exist at first, or is it turned on later?
    kFixedRace       = 0x00000010,  // don't change this object even if owner's race is different
    kInitialAttributesMask = kFixedRace | kInitiallyHidden,
};

enum {
    kUseForTransportation = 0x00000001,  // use when we're going to our destination
    kUseForAttacking      = 0x00000002,  // use when we've got a target our sites
    kUseForDefense        = 0x00000004,  // use when we're running/evading
};

enum {
    kUncapturedBaseExists    = 0x00000001,
    kSufficientEscortsExist  = 0x00000002,
    kThisBaseNeedsProtection = 0x00000004,
    kFriendUpTrend           = 0x00000008,
    kFriendDownTrend         = 0x00000010,
    kFoeUpTrend              = 0x00000020,
    kFoeDownTrend            = 0x00000040,
    kMatchingFoeExists       = 0x00000080,  // unowned object with same level-key exists
    kBuildFlagBit9           = 0x00000100,
    kBuildFlagBit10          = 0x00000200,
    kBuildFlagBit11          = 0x00000400,
    kBuildFlagBit12          = 0x00000800,
    kBuildFlagBit13          = 0x00001000,
    kBuildFlagBit14          = 0x00002000,
    kBuildFlagBit15          = 0x00004000,
    kBuildFlagBit16          = 0x00008000,
    kBuildFlagBit17          = 0x00010000,
    kBuildFlagBit18          = 0x00020000,
    kBuildFlagBit19          = 0x00040000,
    kBuildFlagBit20          = 0x00080000,
    kBuildFlagBit21          = 0x00100000,
    kBuildFlagBit22          = 0x00200000,
    kOnlyEngagedBy           = 0x00400000,
    kCanOnlyEngage           = 0x00800000,

    kEngageKeyTag      = 0x0f000000,
    kEngageKeyTagShift = 24,
    kLevelKeyTag       = 0xf0000000,
    kLevelKeyTagShift  = 28,
};

//
// Well, this is a hack. If a verb's exclusive filter == 0xffffffff
// then we treat the high four bits of the inclusive filter like
// a special tag, matching to the high four bits of an baseObject's
// build flag.
//

enum {
    kStrongerThanTarget     = 0x00000001,
    kTargetIsBase           = 0x00000002,
    kTargetIsNotBase        = 0x00000004,
    kTargetIsLocal          = 0x00000008,
    kTargetIsRemote         = 0x00000010,
    kOnlyEscortNotBase      = 0x00000020,
    kTargetIsFriend         = 0x00000040,
    kTargetIsFoe            = 0x00000080,
    kOrderFlagBit9          = 0x00000100,
    kOrderFlagBit10         = 0x00000200,
    kOrderFlagBit11         = 0x00000400,
    kOrderFlagBit12         = 0x00000800,
    kOrderFlagBit13         = 0x00001000,
    kOrderFlagBit14         = 0x00002000,
    kOrderFlagBit15         = 0x00004000,
    kOrderFlagBit16         = 0x00008000,
    kOrderFlagBit17         = 0x00010000,
    kOrderFlagBit18         = 0x00020000,
    kHardMatchingFriend     = 0x00040000,
    kHardMatchingFoe        = 0x00080000,
    kHardFriendlyEscortOnly = 0x00100000,
    kHardNoFriendlyEscort   = 0x00200000,
    kHardTargetIsRemote     = 0x00400000,
    kHardTargetIsLocal      = 0x00800000,
    kHardTargetIsFoe        = 0x01000000,
    kHardTargetIsFriend     = 0x02000000,
    kHardTargetIsNotBase    = 0x04000000,
    kHardTargetIsBase       = 0x08000000,

    kOrderKeyTag      = 0xf0000000,
    kOrderKeyTagShift = 28,
};

// RUNTIME FLAG BITS
enum {
    kHasArrived   = 0x00000001,
    kTargetLocked = 0x00000002,  // if some foe has locked on, you will be visible
    kIsCloaked    = 0x00000004,  // if you are near a naturally shielding object
    kIsHidden     = 0x00000008,  // overrides natural shielding
    kIsTarget     = 0x00000010,  // preserve target lock in case you become invisible
};

const uint32_t kPresenceDataHiWordMask  = 0xffff0000;
const uint32_t kPresenceDataLoWordMask  = 0x0000ffff;
const int32_t  kPresenceDataHiWordShift = 16;

enum kPresenceStateType {
    kNormalPresence  = 0,
    kLandingPresence = 1,
    kWarpInPresence  = 3,
    kWarpingPresence = 4,
    kWarpOutPresence = 5
};

union objectFrameType {
    // rotation: for objects whose shapes depend on their direction
    struct Rotation {
        int32_t shapeOffset;       // offset for 1st shape
        int32_t rotRes;            // ROT_POS / rotRes = # of discrete shapes
        Fixed   maxTurnRate;       // max rate at which object can turn
        Fixed   turnAcceleration;  // rate at which object reaches maxTurnRate
    };
    Rotation rotation;

    // animation: objects whose appearence does not depend on direction
    struct Animation {
        Fixed firstShape;  // first shape in range
        Fixed lastShape;   // last shape (inclusive)

        int32_t frameDirection;       // direction (either -1, 0, or 1)
        int32_t frameDirectionRange;  // either 0, 1, or 2

        Fixed frameSpeed;       // speed at which object animates
        Fixed frameSpeedRange;  // random addition to speed

        Fixed frameShape;       // starting shape #
        Fixed frameShapeRange;  // random addition to starting shape #
    };
    Animation animation;

    // vector: have no associated sprite
    struct Vector {
        uint8_t color;  // color of line
        uint8_t kind;
        int32_t accuracy;  // for non-normal vector objects, how accurate
        int32_t range;
    };
    Vector vector;

    // weapon: weapon objects have no physical form, and can only be activated
    struct Weapon {
        uint32_t usage;         // when is this used?
        int32_t  energyCost;    // cost to fire
        ticks    fireTime;      // time between shots
        int32_t  ammo;          // initial ammo
        int32_t  range;         // range (= age * max velocity)
        Fixed    inverseSpeed;  // for AI = 1/max velocity
        int32_t  restockCost;   // energy to make new ammo
    };
    Weapon weapon;
};
void read_from(sfz::ReadSource in, objectFrameType::Rotation& rotation);
void read_from(sfz::ReadSource in, objectFrameType::Animation& animation);
void read_from(sfz::ReadSource in, objectFrameType::Vector& vector);
void read_from(sfz::ReadSource in, objectFrameType::Weapon& weapon);

class BaseObject {
  public:
    static BaseObject* get(int number);
    static Handle<BaseObject>     none() { return Handle<BaseObject>(-1); }
    static HandleList<BaseObject> all();

    sfz::String name;
    sfz::String short_name;

    uint32_t attributes;  // initial attributes (see flags)
    int32_t  baseClass;
    int32_t  baseRace;
    int32_t  price;

    Fixed offenseValue;
    //  Fixed                   defenseValue;
    int32_t destinationClass;  // for computer

    Fixed    maxVelocity;      // maximum speed
    Fixed    warpSpeed;        // multiplier of speed at warp (0 if cannot)
    uint32_t warpOutDistance;  // distance at which to come out of warp

    Fixed initialVelocity;       // initial minimum velocity (usually relative)
    Fixed initialVelocityRange;  // random addition to initial velocity

    Fixed mass;       // how quickly thrust acheives max
    Fixed maxThrust;  // maximum amount of thrust

    int32_t health;  // starting health
    int32_t damage;  // damage caused by impact
    int32_t energy;  // starting energy for material objects

    ticks initialAge;       // starting minimum age
    ticks initialAgeRange;  // random addition to starting age

    int32_t occupy_count;  // size of occupying force

    int32_t naturalScale;  // natural scale relative to %100

    int16_t pixLayer;     // 0 = no layer 1->3 = back to front
    int16_t pixResID;     // resID of SMIV
    int32_t tinySize;     // size of representation on radar (0 = 1 pixel)
    uint8_t shieldColor;  // color on radar (0 = don't put on radar)

    int32_t initialDirection;       // initial direction (usually relative)
    int32_t initialDirectionRange;  // random addition to initial direction

    struct Weapon {
        Handle<BaseObject> base;
        int32_t            positionNum;  // # of places from which weapon can fire
        fixedPointType
                position[kMaxWeaponPosition];  // relative positions (unrotated) of fire points
    };
    Weapon pulse;
    Weapon beam;
    Weapon special;

    Fixed   friendDefecit;
    Fixed   dangerThreshold;
    int32_t specialDirection;  // direction relative to shooter

    int32_t arriveActionDistance;  // distance^2 at which arrive action is triggered on dest

    HandleList<Action> destroy;
    HandleList<Action> expire;
    HandleList<Action> create;
    HandleList<Action> collide;
    HandleList<Action> activate;
    HandleList<Action> arrive;

    bool  destroyDontDie;
    bool  expireDontDie;
    ticks activatePeriod;
    ticks activatePeriodRange;

    objectFrameType frame;

    uint32_t buildFlags;
    uint32_t orderFlags;
    uint8_t  levelKeyTag;
    uint8_t  engageKeyTag;
    uint8_t  orderKeyTag;
    Fixed    buildRatio;
    ticks    buildTime;
    //  int32_t             reserved1;
    uint8_t  skillNum;
    uint8_t  skillDen;
    uint8_t  skillNumAdj;
    uint8_t  skillDenAdj;
    int16_t  pictPortraitResID;
    int16_t  reserved2;
    int32_t  reserved3;
    uint32_t internalFlags;

    static const int byte_size = 318;
};
void read_from(sfz::ReadSource in, BaseObject& object);

}  // namespace antares

#endif  // ANTARES_DATA_BASE_OBJECT_HPP_

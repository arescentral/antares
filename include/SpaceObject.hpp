// Ares, a tactical space combat game.
// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// 
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

#ifndef ANTARES_SPACE_OBJECT_HPP_
#define ANTARES_SPACE_OBJECT_HPP_

// Space Object.h

#include "AresGlobalType.hpp"
#include "MathSpecial.hpp"
#include "NateDraw.hpp"
#include "Scenario.hpp"
#include "SoundFX.hpp"
#include "SpriteHandling.hpp"

class BinaryReader;

#define kMaxSpaceObject     250
#define kMaxBaseObject      (globals()->maxBaseObject)//300
#define kMaxObjectAction    (globals()->maxObjectAction)//700//500//360

#define kTimeToCheckHome        900

// these objects should always be manually loaded
#define kEnergyPodBaseNum           28          // base object #28 always must be energy pod
#define kWarpInBaseNum              32
#define kWarpOutBaseNum             33
#define kCastawayBaseNum            22

#define kEnergyPodAmount            500         // average (calced) of 500 energy units/pod

#define kWarpAcceleration           1           // how fast we warp in & out

//#define   kMaxWeapon                  1
#define kNoWeapon                   -1
#define kMaxWeaponPosition          3

#define kPulseWeapon                0
#define kBeamWeapon                 1
#define kSpecialDevice              2

#define kNoAge                      -1
#define kNoShip                     -1
#define kNoDestinationCoord         0
#define kNoDestinationObject        -1
#define kNoOwner                    -1

#define kObjectInUse                1
#define kObjectToBeFreed            2
#define kObjectAvailable            0

/*
#define kMaxAnyAction               4
#define kMaxDestroyAction           4
#define kMaxExpireAction            2
#define kMaxCreateAction            4
#define kMaxCollideAction           3
#define kMaxActivateAction          2
#define kMaxArriveAction            2
*/

#define kNoRace                     -1
#define kIshimanRace                100
#define kCantharanRace              200
#define kPirateRace                 300

#define kNoClass                    -1
#define kSmallFighterClass          100
#define kCruiserClass               200
#define kGunshipClass               300
#define kEscortClass                400
#define kTransportClass             800
#define kTransportSLClass           801 // sub-light transport

// any class > 10000 = direct map to base object - 10000
#define kLiteralClass               10000

#define kHitStateMax                128
#define kCloakOnStateMax            254
#define kCloakOffStateMax           -252

#define kSpaceObjectNameResID       5000
#define kSpaceObjectShortNameResID  5001

#define kEngageRange            1048576L    // range at which to engage closest ship
                                            // about 2 subsectors (512 * 2)^2

#define kCanTurn                    0x00000001  // we have to worry about its rotation velocity
#define kCanBeEngaged               0x00000002  // if it's worth going after
#define kHasDirectionGoal           0x00000004  // we must turn it towards its goal
#define kIsRemote                   0x00000008  // is controlled by remote computer
#define kIsHumanControlled          0x00000010  // human is controlling it
#define kIsBeam                     0x00000020  // a vector shot, no sprite
#define kDoesBounce                 0x00000040  // when it hits the edge, it bounces FORMER: can't move, so don't try
#define kIsSelfAnimated             0x00000080  // cycles through animation frames
#define kShapeFromDirection         0x00000100  // its appearence is based on its direction
#define kIsPlayerShip               0x00000200  // this is the ship we focus on
#define kCanBeDestination           0x00000400  // can be selected as a place to go to
#define kCanEngage                  0x00000800  // can go into engage mode
#define kCanEvade                   0x00001000  // can go into evade mode
#define kCanAcceptMessages          0x00002000  // can accept a message from player
#define kCanAcceptBuild             0x00004000  // can accept a build message
#define kCanAcceptDestination       0x00008000  // can accept destination message
#define kAutoTarget                 0x00010000  // for creating a material weapon, born at parent's target angle
#define kAnimationCycle             0x00020000  // repeating animation
#define kCanCollide                 0x00040000  // can collide with another object
#define kCanBeHit                   0x00080000  // can be on the receving end of a collision
#define kIsDestination              0x00100000  // is a point of interest to which ships can be sent
#define kHideEffect                 0x00200000  // hides other objects around it
#define kReleaseEnergyOnDeath       0x00400000  // when destroyed, remaining energy released
#define kHated                      0x00800000  // when you don't own it, if it's hated, you shoot it
#define kOccupiesSpace              0x01000000  // 2 objects cannot occupy same physical space
#define kStaticDestination          0x02000000  // destination cannot be altered
#define kCanBeEvaded                0x04000000  // it can be veered away from
#define kNeutralDeath               0x08000000  // if true, object becomes neutral when "destroyed" but doesn't die
#define kIsGuided                   0x10000000  // doesn't really think; can't accept orders; if not yours, it is feared
#define kAppearOnRadar              0x20000000  // shows up on radar
#define kBit31                      0x40000000
#define kOnAutoPilot                0x80000000  // if human controlled, this temporarily gives the computer control

#define kCanThink                   (kCanEngage | kCanEvade | kCanAcceptDestination)    // not just "dumb"
#define kConsiderDistance           (kCanThink | kCanBeDestination)
#define kPotentialTarget            (kCanBeEngaged | kCanBeEvaded)
#define kRemoteOrHuman              (kIsPlayerShip)//(kIsHumanControlled | kIsRemote)

//
// for baseObjectTypes only: these bits have different functions in
// baseObjectTypes than they do in normal spaceObjectTypes.  However, they
// should never be turned on permanently!
//

#define kHaveCheckedMedia           0x00000008  // we've checked its sounds & sprites

// <END OF BASEOBJECTTYPE SPECIAL BITS>

// for initial objects only
// kInitiallyExists should NOT be carried over to real objects!
#define kInitiallyHidden            0x00000020  // does it exist at first, or is it turned on later?
#define kFixedRace                  0x00000010  // don't change this object even if owner's race is different
#define kInitialAttributesMask      (kFixedRace | kInitiallyHidden)

// <END OF INITAL OBJECT SPECIAL BITS

#define kUseForTransportation       0x00000001  // use when we're going to our destination
#define kUseForAttacking            0x00000002  // use when we've got a target our sites
#define kUseForDefense              0x00000004  // use when we're running/evading

#define kUncapturedBaseExists           0x00000001
#define kSufficientEscortsExist         0x00000002
#define kThisBaseNeedsProtection        0x00000004
#define kFriendUpTrend                  0x00000008
#define kFriendDownTrend                0x00000010
#define kFoeUpTrend                     0x00000020
#define kFoeDownTrend                   0x00000040
#define kMatchingFoeExists              0x00000080 // unowned object with same level-key exists
#define kBuildFlagBit9                  0x00000100
#define kBuildFlagBit10                 0x00000200
#define kBuildFlagBit11                 0x00000400
#define kBuildFlagBit12                 0x00000800
#define kBuildFlagBit13                 0x00001000
#define kBuildFlagBit14                 0x00002000
#define kBuildFlagBit15                 0x00004000
#define kBuildFlagBit16                 0x00008000
#define kBuildFlagBit17                 0x00010000
#define kBuildFlagBit18                 0x00020000
#define kBuildFlagBit19                 0x00040000
#define kBuildFlagBit20                 0x00080000
#define kBuildFlagBit21                 0x00100000
#define kBuildFlagBit22                 0x00200000
#define kOnlyEngagedBy                  0x00400000
#define kCanOnlyEngage                  0x00800000
#define kEngageKeyTag1                  0x01000000
#define kEngageKeyTag2                  0x02000000
#define kEngageKeyTag3                  0x04000000
#define kEngageKeyTag4                  0x08000000
#define kLevelKeyTag1                   0x10000000
#define kLevelKeyTag2                   0x20000000
#define kLevelKeyTag3                   0x40000000
#define kLevelKeyTag4                   0x80000000
//
// Well, this is a hack. If a verb's exclusive filter == 0xffffffff
// then we treat the high four bits of the inclusive filter like
// a special tag, matching to the high four bits of an baseObject's
// build flag.
//
#define kLevelKeyTagMask                0xf0000000
#define kEngageKeyTagMask               0x0f000000
#define kEngageKeyTagShift              4ul

#define kStrongerThanTarget             0x00000001
#define kTargetIsBase                   0x00000002
#define kTargetIsNotBase                0x00000004
#define kTargetIsLocal                  0x00000008
#define kTargetIsRemote                 0x00000010
#define kOnlyEscortNotBase              0x00000020
#define kTargetIsFriend                 0x00000040
#define kTargetIsFoe                    0x00000080
#define kOrderFlagBit9                  0x00000100
#define kOrderFlagBit10                 0x00000200
#define kOrderFlagBit11                 0x00000400
#define kOrderFlagBit12                 0x00000800
#define kOrderFlagBit13                 0x00001000
#define kOrderFlagBit14                 0x00002000
#define kOrderFlagBit15                 0x00004000
#define kOrderFlagBit16                 0x00008000
#define kOrderFlagBit17                 0x00010000
#define kOrderFlagBit18                 0x00020000
#define kHardMatchingFriend             0x00040000
#define kHardMatchingFoe                0x00080000
#define kHardFriendlyEscortOnly         0x00100000
#define kHardNoFriendlyEscort           0x00200000
#define kHardTargetIsRemote             0x00400000
#define kHardTargetIsLocal              0x00800000
#define kHardTargetIsFoe                0x01000000
#define kHardTargetIsFriend             0x02000000
#define kHardTargetIsNotBase            0x04000000
#define kHardTargetIsBase               0x08000000
#define kOrderKeyTag1                   0x10000000
#define kOrderKeyTag2                   0x20000000
#define kOrderKeyTag3                   0x40000000
#define kOrderKeyTag4                   0x80000000

// RUNTIME FLAG BITS
#define kHasArrived                     0x00000001
#define kTargetLocked                   0x00000002  // if some foe has locked on, you will be visible
#define kIsCloaked                      0x00000004  // if you are near a naturally shielding object
#define kIsHidden                       0x00000008  // overrides natural shielding
#define kIsTarget                       0x00000010  // preserve target lock in case you become invisible

// end of runtime flag bits

#define kObjectNameLength           17
#define kObjectShortNameLength      8

#define kPresenceDataHiWordMask         0xffff0000
#define kPresenceDataLoWordMask         0x0000ffff
#define kPresenceDataHiWordShift        16l

#define kBoltPointNum                   10

#define kPeriodicActionTimeMask     0xff000000
#define kPeriodicActionTimeShift    24l
#define kPeriodicActionRangeMask    0x00ff0000
#define kPeriodicActionRangeShift   16l
#define kPeriodicActionNotMask      0x0000ffff

#define kDestroyActionNotMask       0x7fffffff
#define kDestroyActionDontDieFlag   0x80000000

//typedef struct spaceObjectType *spaceObjectTypePtr;

struct spaceObjectType;
typedef spaceObjectType *spaceObjectTypePtr;
//typedef unsigned long *spaceObjectTypePtr;

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

        void read(BinaryReader* bin);
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

        void read(BinaryReader* bin);
    };
    PlaySound playSound;

    // alterObject: change some attribute of an object
    struct AlterObject {
        uint8_t                 alterType;
        uint8_t                 relative;
        int32_t                 minimum;
        int32_t                 range;

        void read(BinaryReader* bin);
    };
    AlterObject alterObject;

    // makeSpark
    struct MakeSparks {
        int32_t                 howMany;
        int32_t                 speed;
        smallFixedType          velocityRange;
        uint8_t                 color;

        void read(BinaryReader* bin);
    };
    MakeSparks makeSparks;

    // release energy
    struct ReleaseEnergy {
        smallFixedType          percent;

        void read(BinaryReader* bin);
    };
    ReleaseEnergy releaseEnergy;

    // land at
    struct LandAt {
        int32_t                 landingSpeed;

        void read(BinaryReader* bin);
    };
    LandAt landAt;

    // enter warp
    struct EnterWarp {
        smallFixedType          warpSpeed;

        void read(BinaryReader* bin);
    };
    EnterWarp enterWarp;

    // Display message
    struct DisplayMessage {
        int16_t                 resID;
        int16_t                 pageNum;

        void read(BinaryReader* bin);
    };
    DisplayMessage displayMessage;

    // Change score
    struct ChangeScore {
        int32_t                 whichPlayer;    // in scenario's terms; -1 = owner of executor of action
        int32_t                 whichScore;     // each player can have many "scores"
        int32_t                 amount;

        void read(BinaryReader* bin);
    };
    ChangeScore changeScore;

    // Declare winner
    struct DeclareWinner {
        int32_t                 whichPlayer;    // in scenario's terms; -1 = owner of executor of action
        int32_t                 nextLevel;      // -1 = none
        int32_t                 textID;         // id of "debriefing" text

        void read(BinaryReader* bin);
    };
    DeclareWinner declareWinner;

    // killObject: cause object to expire
    struct KillObject {
        dieVerbIDType           dieType;

        void read(BinaryReader* bin);
    };
    KillObject killObject;

    // colorFlash: flash whole screen to a color
    struct ColorFlash {
        int32_t                 length;         // length of color flash
        uint8_t                 color;          // color of flash
        uint8_t                 shade;          // brightness of flash

        void read(BinaryReader* bin);
    };
    ColorFlash colorFlash;

    // keys: disable or enable keys/ for tutorial
    struct Keys {
        uint32_t                keyMask;

        void read(BinaryReader* bin);
    };
    Keys keys;

    // zoomLevel; manually set zoom level
    struct Zoom {
        int32_t                 zoomLevel;

        void read(BinaryReader* bin);
    };
    Zoom zoom;

    struct ComputerSelect {
        int32_t                 screenNumber;
        int32_t                 lineNumber;

        void read(BinaryReader* bin);
    };
    ComputerSelect computerSelect;

    struct AssumeInitial {
        int32_t                 whichInitialObject;

        void read(BinaryReader* bin);
    };
    AssumeInitial assumeInitial;
};

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

    size_t load_data(const char* data, size_t len);
};

typedef uint8_t beamKindType;
enum beamKindEnum {
    eKineticBeamKind =                  0,  // has velocity, moves
    eStaticObjectToObjectKind =         1,  // static line connects 2 objects
    eStaticObjectToRelativeCoordKind =  2,  // static line goes from object to coord
    eBoltObjectToObjectKind =           3,  // lightning bolt, connects 2 objects
    eBoltObjectToRelativeCoordKind =    4   // lightning bolt, from object to coord
};

struct beamType {
    beamKindType        beamKind;
    Rect                thisLocation;
    Rect                lastLocation;
    coordPointType      lastGlobalLocation;
    coordPointType      objectLocation;
    coordPointType      lastApparentLocation;
    coordPointType      endLocation;
    unsigned char       color;
    bool             killMe;
    bool             active;
    long                fromObjectNumber;
    long                fromObjectID;
    spaceObjectTypePtr  fromObject;
    long                toObjectNumber;
    long                toObjectID;
    spaceObjectTypePtr  toObject;
    Point               toRelativeCoord;
    unsigned long       boltRandomSeed;
    unsigned long       lastBoldRandomSeed;
    long                boltCycleTime;
    long                boltState;
    long                accuracy;
    long                range;
    Point               thisBoltPoint[kBoltPointNum];
    Point               lastBoltPoint[kBoltPointNum];
};

union objectFrameType {
    // rotation: for objects whose shapes depend on their direction
    struct Rotation {
        int32_t                 shapeOffset;        // offset for 1st shape
        int32_t                 rotRes;             // ROT_POS / rotRes = # of discrete shapes
        smallFixedType          maxTurnRate;        // max rate at which object can turn
        smallFixedType          turnAcceleration;   // rate at which object reaches maxTurnRate

        void read(BinaryReader* bin);
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

        void read(BinaryReader* bin);
    };
    Animation animation;

    // beam: have no associated sprite
    struct Beam {
        uint8_t                 color;              // color of beam
        beamKindType            kind;
        int32_t                 accuracy;           // for non-normal beams, how accurate
        int32_t                 range;

        void read(BinaryReader* bin);
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

        void read(BinaryReader* bin);
    };
    Weapon weapon;
};

struct baseObjectType {
    uint32_t                attributes;                 // initial attributes (see flags)
    int32_t                 baseClass;
    int32_t                 baseRace;
    int32_t                 price;

    smallFixedType          offenseValue;
//  smallFixedType          defenseValue;
    int32_t                 destinationClass;           // for computer

    smallFixedType          maxVelocity;                // maximum speed
    smallFixedType          warpSpeed;                  // multiplier of speed at warp (0 if cannot)
    uint32_t                warpOutDistance;                // distance at which to come out of warp

    smallFixedType          initialVelocity;            // initial minimum velocity (usually relative)
    smallFixedType          initialVelocityRange;       // random addition to initial velocity

    smallFixedType          mass;                       // how quickly thrust acheives max
    smallFixedType          maxThrust;                  // maximum amount of thrust

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

    smallFixedType          friendDefecit;
    smallFixedType          dangerThreshold;
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
    smallFixedType      buildRatio;
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

    size_t load_data(const char* data, size_t len);
};

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
    unsigned char           tinyColor;

    long                    direction;
    long                    directionGoal;
    smallFixedType          turnVelocity;
    smallFixedType          turnFraction;

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

//  smallFixedType          balance[kScenarioPlayerNum];
    smallFixedType          localFriendStrength;
    smallFixedType          localFoeStrength;
    smallFixedType          escortStrength;
    smallFixedType          remoteFriendStrength;
    smallFixedType          remoteFoeStrength;

    smallFixedType          bestConsideredTargetValue;
    smallFixedType          currentTargetValue;
    long                    bestConsideredTargetNumber;

    long                    timeFromOrigin;     // time it's been since we left
    fixedPointType          idealLocationCalc;  // calced when we got origin
    coordPointType          originLocation;     // coords of our origin

    fixedPointType          motionFraction;
    fixedPointType          velocity;
    smallFixedType          thrust;
    smallFixedType          maxVelocity;
    Point                   scaledCornerOffset;
    Point                   scaledSize;
    Rect                absoluteBounds;
    int32_t                 randomSeed;

    union
    {
//      struct
//      {
            long                directionGoal;
            smallFixedType      turnVelocity;
            smallFixedType      turnFraction;
//      } rotation;
        struct
        {
            long                thisShape;
            smallFixedType      frameFraction;
            long                frameDirection;
            smallFixedType      frameSpeed;
        } animation;
        struct
        {
            long                whichBeam;
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

extern TypedHandle<baseObjectType> gBaseObjectData;

inline baseObjectType* mGetBaseObjectPtr(long whichObject) {
    return *gBaseObjectData + whichObject;
}

inline void mGetBaseObjectFromClassRace(
        baseObjectType*& mbaseObject, long& mcount, int mbaseClass, int mbaseRace) {
    mcount = 0;
    if ( mbaseClass >= kLiteralClass)
    {
        mcount = mbaseClass - kLiteralClass;
        mbaseObject = mGetBaseObjectPtr(mcount);
    }
    else
    {
        mbaseObject = mGetBaseObjectPtr( 0);
        while (( mcount < kMaxBaseObject) && (( mbaseObject->baseClass != mbaseClass) || ( mbaseObject->baseRace != mbaseRace)))
        {
            mcount++;
            mbaseObject++;
        }
        if ( mcount >= kMaxBaseObject) mbaseObject = nil;
    }
}

#endif // ANTARES_SPACE_OBJECT_HPP_

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
#include <pn/string>
#include <memory>
#include <vector>

#include "data/handle.hpp"
#include "math/fixed.hpp"
#include "math/geometry.hpp"
#include "math/units.hpp"

namespace antares {

class SpaceObject;
struct Level_Initial;

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

// TODO(sfiera): use std::variant<> when it’s available.
struct argumentType {
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
        int16_t                 resID;
        std::vector<pn::string> pages;
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
        pn::string      text;         // "debriefing" text
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

struct ActionBase;

class Action {
  public:
                operator bool() const { return _base != nullptr; }
    ActionBase* operator->() const { return _base.get(); }
    ActionBase& operator*() const { return *_base; }

    template <typename T>
    T* init() {
        T* out;
        _base = std::unique_ptr<ActionBase>(out = new T);
        return out;
    }

  private:
    std::unique_ptr<ActionBase> _base;
};

struct ActionBase {
    uint16_t verb;

    bool     reflexive;        // does it apply to object executing verb?
    uint32_t inclusiveFilter;  // if it has ALL these attributes, OK -- for non-reflective verbs
    uint32_t exclusiveFilter;  // don't execute if it has ANY of these
    uint8_t  levelKeyTag;
    int16_t  owner;  // 0 no matter, 1 same owner, -1 different owner
    ticks    delay;
    //  uint32_t                    reserved1;
    Handle<Level_Initial> initialSubjectOverride;
    Handle<Level_Initial> initialDirectOverride;
    uint32_t              reserved2;
    argumentType          argument;

    virtual ~ActionBase() = default;
    virtual void apply(
            Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
            Point* offset) = 0;

    static const size_t byte_size = 48;
};
bool read_from(pn::file_view in, Action* action);

std::vector<Action> read_actions(int begin, int end);

struct NoAction : public ActionBase {
    virtual void apply(
            Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
            Point* offset);
};

struct CreateObjectAction : public ActionBase {
    virtual void apply(
            Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
            Point* offset);
};

struct PlaySoundAction : public ActionBase {
    virtual void apply(
            Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
            Point* offset);
};

struct AlterAction : public ActionBase {
    virtual void apply(
            Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
            Point* offset);
};

struct MakeSparksAction : public ActionBase {
    virtual void apply(
            Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
            Point* offset);
};

struct ReleaseEnergyAction : public ActionBase {
    virtual void apply(
            Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
            Point* offset);
};

struct LandAtAction : public ActionBase {
    virtual void apply(
            Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
            Point* offset);
};

struct EnterWarpAction : public ActionBase {
    virtual void apply(
            Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
            Point* offset);
};

struct DisplayMessageAction : public ActionBase {
    virtual void apply(
            Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
            Point* offset);
};

struct ChangeScoreAction : public ActionBase {
    virtual void apply(
            Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
            Point* offset);
};

struct DeclareWinnerAction : public ActionBase {
    virtual void apply(
            Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
            Point* offset);
};

struct DieAction : public ActionBase {
    virtual void apply(
            Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
            Point* offset);
};

struct SetDestinationAction : public ActionBase {
    virtual void apply(
            Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
            Point* offset);
};

struct ActivateSpecialAction : public ActionBase {
    virtual void apply(
            Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
            Point* offset);
};

struct ActivatePulseAction : public ActionBase {
    virtual void apply(
            Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
            Point* offset);
};

struct ActivateBeamAction : public ActionBase {
    virtual void apply(
            Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
            Point* offset);
};

struct ColorFlashAction : public ActionBase {
    virtual void apply(
            Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
            Point* offset);
};

struct NilTargetAction : public ActionBase {
    virtual void apply(
            Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
            Point* offset);
};

struct DisableKeysAction : public ActionBase {
    virtual void apply(
            Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
            Point* offset);
};

struct EnableKeysAction : public ActionBase {
    virtual void apply(
            Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
            Point* offset);
};

struct SetZoomAction : public ActionBase {
    virtual void apply(
            Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
            Point* offset);
};

struct ComputerSelectAction : public ActionBase {
    virtual void apply(
            Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
            Point* offset);
};

struct AssumeInitialObjectAction : public ActionBase {
    virtual void apply(
            Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
            Point* offset);
};

struct AlterDamageAction : public ActionBase {
    virtual void apply(
            Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
            Point* offset);
};

struct AlterVelocityAction : public ActionBase {
    virtual void apply(
            Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
            Point* offset);
};

struct AlterThrustAction : public ActionBase {
    virtual void apply(
            Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
            Point* offset);
};

struct AlterMaxThrustAction : public ActionBase {
    virtual void apply(
            Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
            Point* offset);
};

struct AlterMaxVelocityAction : public ActionBase {
    virtual void apply(
            Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
            Point* offset);
};

struct AlterMaxTurnRateAction : public ActionBase {
    virtual void apply(
            Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
            Point* offset);
};

struct AlterLocationAction : public ActionBase {
    virtual void apply(
            Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
            Point* offset);
};

struct AlterScaleAction : public ActionBase {
    virtual void apply(
            Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
            Point* offset);
};

struct AlterWeapon1Action : public ActionBase {
    virtual void apply(
            Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
            Point* offset);
};

struct AlterWeapon2Action : public ActionBase {
    virtual void apply(
            Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
            Point* offset);
};

struct AlterSpecialAction : public ActionBase {
    virtual void apply(
            Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
            Point* offset);
};

struct AlterEnergyAction : public ActionBase {
    virtual void apply(
            Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
            Point* offset);
};

struct AlterOwnerAction : public ActionBase {
    virtual void apply(
            Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
            Point* offset);
};

struct AlterHiddenAction : public ActionBase {
    virtual void apply(
            Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
            Point* offset);
};

struct AlterCloakAction : public ActionBase {
    virtual void apply(
            Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
            Point* offset);
};

struct AlterOfflineAction : public ActionBase {
    virtual void apply(
            Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
            Point* offset);
};

struct AlterSpinAction : public ActionBase {
    virtual void apply(
            Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
            Point* offset);
};

struct AlterBaseTypeAction : public ActionBase {
    virtual void apply(
            Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
            Point* offset);
};

struct AlterConditionTrueYetAction : public ActionBase {
    virtual void apply(
            Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
            Point* offset);
};

struct AlterOccupationAction : public ActionBase {
    virtual void apply(
            Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
            Point* offset);
};

struct AlterAbsoluteCashAction : public ActionBase {
    virtual void apply(
            Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
            Point* offset);
};

struct AlterAgeAction : public ActionBase {
    virtual void apply(
            Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
            Point* offset);
};

struct AlterAttributesAction : public ActionBase {
    virtual void apply(
            Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
            Point* offset);
};

struct AlterLevelKeyTagAction : public ActionBase {
    virtual void apply(
            Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
            Point* offset);
};

struct AlterOrderKeyTagAction : public ActionBase {
    virtual void apply(
            Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
            Point* offset);
};

struct AlterEngageKeyTagAction : public ActionBase {
    virtual void apply(
            Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
            Point* offset);
};

struct AlterAbsoluteLocationAction : public ActionBase {
    virtual void apply(
            Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
            Point* offset);
};

}  // namespace antares

#endif  // ANTARES_DATA_ACTION_HPP_

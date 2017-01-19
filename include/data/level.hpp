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

#ifndef ANTARES_DATA_LEVEL_HPP_
#define ANTARES_DATA_LEVEL_HPP_

#include <sfz/sfz.hpp>

#include "data/action.hpp"
#include "data/handle.hpp"
#include "math/fixed.hpp"
#include "math/geometry.hpp"
#include "math/units.hpp"

namespace antares {

struct LevelName;
class BaseObject;

const size_t kMaxPlayerNum = 4;

const int32_t kMaxTypeBaseCanBuild = 12;
const int32_t kMaxShipCanBuild     = 6;

enum {
    kSingleHumanPlayer  = 0,
    kNetworkHumanPlayer = 1,
    kComputerPlayer     = 2,
};

const int16_t kLevelBriefMask  = 0x00ff;
const int16_t kLevelAngleMask  = 0xff00;
const int32_t kLevelAngleShift = 8;

const int32_t kLevelNoOwner = -1;

// condition flags
const int32_t kTrueOnlyOnce  = 0x00000001;
const int32_t kInitiallyTrue = 0x00000002;
const int32_t kHasBeenTrue   = 0x00000004;

struct scenarioInfoType {
    Handle<BaseObject> warpInFlareID;
    Handle<BaseObject> warpOutFlareID;
    Handle<BaseObject> playerBodyID;
    Handle<BaseObject> energyBlobID;
    sfz::String        downloadURLString;
    sfz::String        titleString;
    sfz::String        authorNameString;
    sfz::String        authorURLString;
    uint32_t           version;
};
void read_from(sfz::ReadSource in, scenarioInfoType& scenario_info);

enum conditionType {
    kNoCondition                      = 0,
    kLocationCondition                = 1,
    kCounterCondition                 = 2,
    kProximityCondition               = 3,
    kOwnerCondition                   = 4,
    kDestructionCondition             = 5,
    kAgeCondition                     = 6,
    kTimeCondition                    = 7,
    kRandomCondition                  = 8,
    kHalfHealthCondition              = 9,
    kIsAuxiliaryObject                = 10,
    kIsTargetObject                   = 11,
    kCounterGreaterCondition          = 12,
    kCounterNotCondition              = 13,
    kDistanceGreaterCondition         = 14,
    kVelocityLessThanEqualToCondition = 15,
    kNoShipsLeftCondition             = 16,
    kCurrentMessageCondition          = 17,  // use location.h for res id, .v for page
    kCurrentComputerCondition         = 18,  // use location.h for screen #, .v for line #
    kZoomLevelCondition               = 19,
    kAutopilotCondition               = 20,
    kNotAutopilotCondition            = 21,
    kObjectIsBeingBuilt               = 22,  // for tutorial; is base building something?
    kDirectIsSubjectTarget            = 23,
    kSubjectIsPlayerCondition         = 24
};

typedef uint8_t briefingPointKindType;
enum briefingPointKindEnum {
    kNoPointKind           = 0,
    kBriefObjectKind       = 1,
    kBriefAbsoluteKind     = 2,
    kBriefFreestandingKind = 3
};

struct Level {
    struct InitialObject;
    struct Condition;
    struct BriefPoint;

    struct Player {
        int16_t playerType;
        int16_t playerRace;
        int16_t nameResID;
        int16_t nameStrNum;
        Fixed   earningPower;
        int16_t netRaceFlags;
        int16_t reserved1;
    };

    sfz::String name;
    int16_t     netRaceFlags;
    int16_t     playerNum;
    Player      player[kMaxPlayerNum];
    int16_t     scoreStringResID;
    int16_t     initialFirst;
    int16_t     prologueID;
    int16_t     initialNum;
    int16_t     songID;
    int16_t     conditionFirst;
    int16_t     epilogueID;
    int16_t     conditionNum;
    int16_t     starMapH;
    int16_t     briefPointFirst;
    int16_t     starMapV;
    int16_t     briefPointNum;  // use kLevelBriefMask
    game_ticks  parTime;
    int16_t     parKills;
    int16_t     levelNameStrNum;
    Fixed       parKillRatio;
    int16_t     parLosses;
    secs        startTime;
    bool        is_training;

    static const size_t byte_size = 124;

    static Level* get(int n);
    static Handle<Level> none() { return Handle<Level>(-1); }

    InitialObject* initial(size_t at) const;
    Condition* condition(size_t at) const;

    BriefPoint* brief_point(size_t at) const;
    size_t brief_point_size() const;

    int32_t angle() const;
    Point   star_map_point() const;
    int32_t chapter_number() const;

    int32_t prologue_id() const;
    int32_t epilogue_id() const;
};
void read_from(sfz::ReadSource in, Level& level);
void read_from(sfz::ReadSource in, Level::Player& level_player);

struct Level::InitialObject {
    Handle<BaseObject>  type;
    Handle<Admiral>     owner;
    Handle<SpaceObject> realObject;
    int32_t             realObjectID;
    Point               location;
    Fixed               earning;
    int32_t             distanceRange;
    int32_t             rotationMinimum;
    int32_t             rotationRange;
    int32_t             spriteIDOverride;  // <- ADDED 9/30
    int32_t             canBuild[kMaxTypeBaseCanBuild];
    int32_t             initialDestination;  // <- ADDED 9/27
    int32_t             nameResID;
    int32_t             nameStrNum;
    uint32_t            attributes;

    static const size_t byte_size = 108;
};
void read_from(sfz::ReadSource in, Level::InitialObject& level_initial);

struct Level::Condition {
    struct CounterArgument {
        Handle<Admiral> whichPlayer;
        int32_t         whichCounter;
        int32_t         amount;
    };

    uint8_t condition;
    struct {
        // Really a union
        Point           location;
        CounterArgument counter;
        int32_t         longValue;
        Fixed           fixedValue;
        ticks           timeValue;
        uint32_t        unsignedLongValue;
    } conditionArgument;
    int32_t            subjectObject;  // initial object #
    int32_t            directObject;   // initial object #
    HandleList<Action> action;
    uint32_t           flags;
    int32_t            direction;

    static const size_t byte_size = 38;

    bool active() const;
    bool is_true() const;
    bool true_yet() const;
    void set_true_yet(bool state);
};
void read_from(sfz::ReadSource in, Level::Condition& level_condition);
void read_from(sfz::ReadSource in, Level::Condition::CounterArgument& counter_argument);

//
// We need to know:
// type of tour point: object, absolute, or free-standing
// either level object # & visible --or-- location ((int32_t & bool) or longPoint)
// range (longPoint)
// title ID, # (int16_t, int16_t)
// content ID, # (int16_t, int16_t)
//

struct Level::BriefPoint {
    struct ObjectBrief {
        int32_t objectNum;
        uint8_t objectVisible;  // bool
    };
    struct AbsoluteBrief {
        Point location;
    };

    briefingPointKindType briefPointKind;
    struct {
        // Really a union
        ObjectBrief   objectBriefType;
        AbsoluteBrief absoluteBriefType;
    } briefPointData;
    Point   range;
    int16_t titleResID;
    int16_t titleNum;
    int16_t contentResID;

    static const size_t byte_size = 24;
};
void read_from(sfz::ReadSource in, Level::BriefPoint::ObjectBrief& object_brief);
void read_from(sfz::ReadSource in, Level::BriefPoint::AbsoluteBrief& absolute_brief);
void read_from(sfz::ReadSource in, Level::BriefPoint& brief_point);

struct Race {
    int32_t  id;
    uint8_t  apparentColor;
    uint32_t illegalColors;
    int32_t  advantage;

    static const size_t byte_size = 14;
};
void read_from(sfz::ReadSource in, Race& race);

}  // namespace antares

#endif  // ANTARES_DATA_LEVEL_HPP_

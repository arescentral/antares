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

#include <pn/string>
#include <vector>

#include "data/action.hpp"
#include "data/handle.hpp"
#include "math/fixed.hpp"
#include "math/geometry.hpp"
#include "math/units.hpp"
#include "video/driver.hpp"

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

struct ScenarioInfo {
    Handle<BaseObject> warpInFlareID;
    Handle<BaseObject> warpOutFlareID;
    Handle<BaseObject> playerBodyID;
    Handle<BaseObject> energyBlobID;
    pn::string         downloadURLString;
    pn::string         titleString;
    pn::string         authorNameString;
    pn::string         authorURLString;
    pn::string         intro_text;
    pn::string         about_text;

    Texture publisher_screen;
    Texture ego_screen;
    Texture splash_screen;
    Texture starmap;

    pn::string version;
};
bool read_from(pn::file_view in, ScenarioInfo* scenario_info);

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

struct Level {
    struct Initial;

    struct Briefing;

    struct Condition;
    struct ConditionBase;
    struct AutopilotCondition;
    struct BuildingCondition;
    struct ComputerCondition;
    struct CounterCondition;
    struct DestroyedCondition;
    struct DistanceCondition;
    struct FalseCondition;
    struct HealthCondition;
    struct MessageCondition;
    struct OrderedCondition;
    struct OwnerCondition;
    struct ShipsCondition;
    struct SpeedCondition;
    struct SubjectCondition;
    struct TimeCondition;
    struct ZoomCondition;

    enum Type {
        DEMO,
        SOLO,
        NET,
    };
    Type type = DEMO;

    struct Player {
        int16_t    playerType;
        int16_t    playerRace;
        pn::string name;
        Fixed      earningPower;
        int16_t    netRaceFlags;
    };

    pn::string              name;
    int16_t                 netRaceFlags;
    int16_t                 playerNum;
    Player                  player[kMaxPlayerNum];
    std::vector<pn::string> score_strings;
    int16_t                 songID;
    int16_t                 starMapH;
    int16_t                 starMapV;
    game_ticks              parTime;
    int16_t                 parKills;
    int16_t                 levelNameStrNum;
    Fixed                   parKillRatio;
    int16_t                 parLosses;
    secs                    startTime;
    bool                    is_training;
    int32_t                 angle;

    std::vector<Initial>   initials;
    std::vector<Condition> conditions;
    std::vector<Briefing>  briefings;

    pn::string prologue;           // SOLO
    pn::string epilogue;           // SOLO
    pn::string own_no_ships_text;  // SOLO, NET
    pn::string foe_no_ships_text;  // NET
    pn::string description;        // NET

    static const size_t byte_size = 124;

    static Level*        get(int n);
    static Handle<Level> none() { return Handle<Level>(-1); }

    Point   star_map_point() const;
    int32_t chapter_number() const;
};
bool read_from(pn::file_view in, Level* level);
bool read_from(pn::file_view in, Level::Player* level_player);

struct Level::Initial {
    Handle<BaseObject> base;
    Handle<Admiral>    owner;
    Point              at;
    Fixed              earning;

    pn::string name_override;
    int32_t    sprite_override;

    int32_t build[kMaxTypeBaseCanBuild];
    int32_t target;

    class Attributes {
      public:
        constexpr Attributes(uint32_t value = 0) : _value(value) {}

        bool     initially_hidden() const { return _value & (1 << 5); }
        bool     fixed_race() const { return _value & (1 << 4); }
        bool     is_player_ship() const { return _value & (1 << 9); }
        uint32_t space_object_attributes() const { return _value & ~((1 << 4) | (1 << 5)); }

      private:
        uint32_t _value;
    };
    Attributes attributes;

    // Transient information during level.
    // TODO(sfiera): remove.
    Handle<SpaceObject> realObject;
    int32_t             realObjectID;

    static const size_t byte_size = 108;
};
bool                        read_from(pn::file_view in, Level::Initial* level_initial);
std::vector<Level::Initial> read_initials(int begin, int end);

struct Level::ConditionBase {
    enum class Op { EQ, NE, LT, GT, LE, GE };

    Op                  op                = Op::EQ;
    int32_t             subject           = -1;  // initial object #
    int32_t             object            = -1;  // initial object #
    bool                initially_enabled = true;
    bool                persistent        = false;
    std::vector<Action> action;

    // Transient information during level.
    // TODO(sfiera): remove.
    bool enabled = true;

    static const size_t byte_size = 38;

    ConditionBase()              = default;
    virtual ~ConditionBase()     = default;
    virtual bool is_true() const = 0;

    ConditionBase(const ConditionBase&) = delete;
    ConditionBase& operator=(const ConditionBase&) = delete;
};
bool                          read_from(pn::file_view in, Level::Condition* level_condition);
std::vector<Level::Condition> read_conditions(int begin, int end);

// Ops: EQ, NE
// Compares local player’s autopilot state (on = true; off = false) to `value`.
//
// Warning: not net-safe.
struct Level::AutopilotCondition : Level::ConditionBase {
    bool         value;
    virtual bool is_true() const;
};

// Ops: EQ, NE
// Precondition: local player has a build object.
// Compares local player’s build object state (building = true; not building = false) to `value`.
//
// Warning: not net-safe.
struct Level::BuildingCondition : Level::ConditionBase {
    bool         value;
    virtual bool is_true() const;
};

// Ops: EQ, NE
// Compares local player’s (screen, line), or just screen if line < 0.
//
// Warning: not net-safe.
struct Level::ComputerCondition : Level::ConditionBase {
    int32_t      screen;
    int32_t      line;
    virtual bool is_true() const;
};

// Ops: EQ, NE, LT, GT, LE, GE
// Compares given counter of given admiral to `value`.
struct Level::CounterCondition : Level::ConditionBase {
    Handle<Admiral> player;
    int32_t         counter;
    int32_t         value;
    virtual bool    is_true() const;
};

// Ops: EQ, NE
// Compares state of given initial (destroyed = true; alive = false) to `value`.
//
// Note: the initial object referenced here can be (and usually is) different from either `subject`
// or `object`.
// Note: an initially-hidden object that has not yet been unhidden is considered “destroyed”
struct Level::DestroyedCondition : Level::ConditionBase {
    int32_t      initial;
    bool         value;
    virtual bool is_true() const;
};

// Ops: EQ, NE, LT, GT, LE, GE
// Precondition: `subject` and `object` exist; `subject` and `object` are not “extremely” distant.
// Compares distance between `subject` and `object` to `value`.
//
// TODO(sfiera): provide a definition of “distance” in this context, and especially what
// “extremely” distant means.
struct Level::DistanceCondition : Level::ConditionBase {
    uint32_t     value;
    virtual bool is_true() const;
};

// Always false.
struct Level::FalseCondition : Level::ConditionBase {
    virtual bool is_true() const;
};

// Ops: EQ, NE, LT, GT, LE, GE
// Compares health fraction of `subject` (e.g. 0.5 for half health) to `value`.
//
// Note: an initially-hidden object that has not yet been unhidden is considered “destroyed”; i.e.
// its health fraction is 0.0.
struct Level::HealthCondition : Level::ConditionBase {
    double       value;
    virtual bool is_true() const;
};

// Ops: EQ, NE
// Compares (start, page) of local player’s current message to (start, page).
//
// Warning: not net-safe.
struct Level::MessageCondition : Level::ConditionBase {
    int32_t      start;
    int32_t      page;
    virtual bool is_true() const;
};

// Ops: EQ, NE
// Precondition: `subject` and `object` exist.
// Compares target of `subject` to `object`.
struct Level::OrderedCondition : Level::ConditionBase {
    virtual bool is_true() const;
};

// Ops: EQ, NE
// Precondition: `subject` exists.
// Compares owner of `subject` to `player`.
struct Level::OwnerCondition : Level::ConditionBase {
    Handle<Admiral> player;
    virtual bool    is_true() const;
};

// Ops: EQ, NE, LT, GT, LE, GE
// Compares ship count of `player` to `value`.
struct Level::ShipsCondition : Level::ConditionBase {
    Handle<Admiral> player;
    int32_t         value;
    virtual bool    is_true() const;
};

// Ops: EQ, NE, LT, GT, LE, GE
// Precondition: `subject` exists.
// Compares speed of `subject` to `value`.
struct Level::SpeedCondition : Level::ConditionBase {
    Fixed        value;
    virtual bool is_true() const;
};

// Ops: EQ, NE, LT, GT, LE, GE
// Precondition: `subject` exists.
// Compares `subject` to the control, target, or flagship of the local player, per `value`.
//
// Warning: not net-safe.
struct Level::SubjectCondition : Level::ConditionBase {
    enum class Value { CONTROL, TARGET, PLAYER };
    Value        value;
    virtual bool is_true() const;
};

// Ops: EQ, NE, LT, GT, LE, GE
// Compares `subject` to the control, target, or flagship of the local player, per `value`.
//
// Note: On a level that specifies a `start_time`, the setup time counts for only 1/3 as much as
// time after the
//
// TODO(sfiera): provide a way to specify game time “normally”
struct Level::TimeCondition : Level::ConditionBase {
    ticks        value;
    virtual bool is_true() const;
};

// Ops: EQ, NE, LT, GT, LE, GE
// Compares zoom level of the local player to `value`.
//
// Warning: not net-safe.
struct Level::ZoomCondition : Level::ConditionBase {
    int32_t      value;
    virtual bool is_true() const;
};

struct Level::Condition {
  public:
                   operator bool() const { return _base != nullptr; }
    ConditionBase* operator->() const { return _base.get(); }
    ConditionBase& operator*() const { return *_base.get(); }

    bool active() { return _base->persistent || _base->enabled; }

    template <typename T>
    T* init() {
        T* out;
        _base = std::unique_ptr<ConditionBase>(out = new T);
        return out;
    }

  private:
    std::unique_ptr<ConditionBase> _base;
};

//
// We need to know:
// type of tour point: object, absolute, or free-standing
// either level object # & visible --or-- location ((int32_t & bool) or longPoint)
// range (longPoint)
// title ID, # (int16_t, int16_t)
// content ID, # (int16_t, int16_t)
//

struct Level::Briefing {
    int32_t    object;   // Index into g.level->initials, or <0 for freestanding.
    pn::string title;    // Plain text, used for title bar.
    pn::string content;  // Styled text, used for body.

    static const size_t byte_size = 24;
};
bool                         read_from(pn::file_view in, Level::Briefing* brief_point);
std::vector<Level::Briefing> read_briefings(int begin, int end);

struct Race {
    int32_t  id;
    uint8_t  apparentColor;
    uint32_t illegalColors;
    int32_t  advantage;

    static const size_t byte_size = 14;
};
bool read_from(pn::file_view in, Race* race);

}  // namespace antares

#endif  // ANTARES_DATA_LEVEL_HPP_

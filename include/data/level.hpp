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
#include "data/base-object.hpp"
#include "data/enums.hpp"
#include "data/handle.hpp"
#include "math/fixed.hpp"
#include "math/geometry.hpp"
#include "math/units.hpp"
#include "video/driver.hpp"

namespace antares {

struct LevelName;
class BaseObject;
struct Level_Briefing;
struct Level_Condition;
struct Level_Initial;
struct Race;

const size_t kMaxPlayerNum = 4;

const int32_t kMaxShipCanBuild = 6;

const int16_t kLevelBriefMask  = 0x00ff;
const int16_t kLevelAngleMask  = 0xff00;
const int32_t kLevelAngleShift = 8;

const int32_t kLevelNoOwner = -1;

// condition flags
const int32_t kTrueOnlyOnce  = 0x00000001;
const int32_t kInitiallyTrue = 0x00000002;
const int32_t kHasBeenTrue   = 0x00000004;

struct ScenarioInfo {
    NamedHandle<const BaseObject> warpInFlareID;
    NamedHandle<const BaseObject> warpOutFlareID;
    NamedHandle<const BaseObject> playerBodyID;
    NamedHandle<const BaseObject> energyBlobID;
    pn::string                    downloadURLString;
    pn::string                    titleString;
    pn::string                    authorNameString;
    pn::string                    authorURLString;
    pn::string                    intro_text;
    pn::string                    about_text;

    Texture publisher_screen;
    Texture ego_screen;
    Texture splash_screen;
    Texture starmap;

    pn::string version;
};
ScenarioInfo scenario_info(pn::value_cref x0);

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
    using Initial = Level_Initial;

    using Briefing = Level_Briefing;

    using Condition = Level_Condition;
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

    LevelType type = LevelType::DEMO;

    struct Player {
        PlayerType              playerType = PlayerType::CPU;
        NamedHandle<const Race> playerRace;
        pn::string              name;
        Fixed                   earningPower = Fixed::zero();
        int16_t                 netRaceFlags = 0;
        Hue                     hue          = Hue::GRAY;
    };

    int                     chapter = -1;
    pn::string              name;
    std::vector<Player>     players;
    std::vector<pn::string> score_strings;
    int16_t                 songID      = -1;
    Point                   starMap     = {-1, -1};
    game_ticks              parTime     = game_ticks{};
    int16_t                 parKills    = 0;
    int16_t                 parLosses   = 0;
    secs                    startTime   = secs(0);
    bool                    is_training = false;
    int32_t                 angle       = 0;

    std::vector<Initial>                    initials;
    std::vector<std::unique_ptr<Condition>> conditions;
    std::vector<Briefing>                   briefings;

    pn::string prologue;           // SOLO
    pn::string epilogue;           // SOLO
    pn::string own_no_ships_text;  // SOLO, NET
    pn::string foe_no_ships_text;  // NET
    pn::string description;        // NET

    static const size_t byte_size = 124;

    static const Level* get(int n);
    static const Level* get(pn::string_view n);

    Point   star_map_point() const;
    int32_t chapter_number() const;
};
Level level(pn::value_cref x);

// Might be the name of a BaseObject, or of an entry in a Race’s “ships” list.
struct BuildableObject {
    pn::string name;
};

struct Level_Initial {
    BuildableObject base;
    Handle<Admiral> owner;
    Point           at;
    bool            hide     = false;
    bool            flagship = false;

    struct Target {
        Handle<const Level::Initial> initial;
        bool                         lock = false;
    } target;

    struct Override {
        sfz::optional<pn::string> name;
        sfz::optional<pn::string> sprite;
    } override_;

    Fixed                        earning = Fixed::zero();
    std::vector<BuildableObject> build;

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

    static const Level::Initial*            get(int n);
    static Handle<const Level::Initial>     none() { return Handle<const Level::Initial>(-1); }
    static HandleList<const Level::Initial> all();

    static const size_t byte_size = 108;
};

struct Level_Condition {
    ConditionOp                                op                = ConditionOp::EQ;
    bool                                       initially_enabled = true;
    bool                                       persistent        = false;
    Handle<const Level::Initial>               subject;
    Handle<const Level::Initial>               object;
    std::vector<std::unique_ptr<const Action>> action;

    static const size_t byte_size = 38;

    static const Level::Condition*            get(int n);
    static HandleList<const Level::Condition> all();

    Level_Condition()            = default;
    virtual ~Level_Condition()   = default;
    virtual bool is_true() const = 0;

    Level_Condition(const Level_Condition&) = delete;
    Level_Condition& operator=(const Level_Condition&) = delete;
};

// Ops: EQ, NE
// Compares local player’s autopilot state (on = true; off = false) to `value`.
//
// Warning: not net-safe.
struct Level::AutopilotCondition : Level::Condition {
    bool         value;
    virtual bool is_true() const;
};

// Ops: EQ, NE
// Precondition: local player has a build object.
// Compares local player’s build object state (building = true; not building = false) to `value`.
//
// Warning: not net-safe.
struct Level::BuildingCondition : Level::Condition {
    bool         value;
    virtual bool is_true() const;
};

// Ops: EQ, NE
// Compares local player’s (screen, line), or just screen if line < 0.
//
// Warning: not net-safe.
struct Level::ComputerCondition : Level::Condition {
    Screen       screen;
    int32_t      line;
    virtual bool is_true() const;
};

// Ops: EQ, NE, LT, GT, LE, GE
// Compares given counter of given admiral to `value`.
struct Level::CounterCondition : Level::Condition {
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
struct Level::DestroyedCondition : Level::Condition {
    Handle<const Level::Initial> initial;
    bool                         value;
    virtual bool                 is_true() const;
};

// Ops: EQ, NE, LT, GT, LE, GE
// Precondition: `subject` and `object` exist; `subject` and `object` are not “extremely” distant.
// Compares distance between `subject` and `object` to `value`.
//
// TODO(sfiera): provide a definition of “distance” in this context, and especially what
// “extremely” distant means.
struct Level::DistanceCondition : Level::Condition {
    uint32_t     value;
    virtual bool is_true() const;
};

// Always false.
struct Level::FalseCondition : Level::Condition {
    virtual bool is_true() const;
};

// Ops: EQ, NE, LT, GT, LE, GE
// Compares health fraction of `subject` (e.g. 0.5 for half health) to `value`.
//
// Note: an initially-hidden object that has not yet been unhidden is considered “destroyed”; i.e.
// its health fraction is 0.0.
struct Level::HealthCondition : Level::Condition {
    double       value;
    virtual bool is_true() const;
};

// Ops: EQ, NE
// Compares (id, page) of local player’s current message to (id, page).
//
// Warning: not net-safe.
struct Level::MessageCondition : Level::Condition {
    int32_t      id;
    int32_t      page;
    virtual bool is_true() const;
};

// Ops: EQ, NE
// Precondition: `subject` and `object` exist.
// Compares target of `subject` to `object`.
struct Level::OrderedCondition : Level::Condition {
    virtual bool is_true() const;
};

// Ops: EQ, NE
// Precondition: `subject` exists.
// Compares owner of `subject` to `player`.
struct Level::OwnerCondition : Level::Condition {
    Handle<Admiral> player;
    virtual bool    is_true() const;
};

// Ops: EQ, NE, LT, GT, LE, GE
// Compares ship count of `player` to `value`.
struct Level::ShipsCondition : Level::Condition {
    Handle<Admiral> player;
    int32_t         value;
    virtual bool    is_true() const;
};

// Ops: EQ, NE, LT, GT, LE, GE
// Precondition: `subject` exists.
// Compares speed of `subject` to `value`.
struct Level::SpeedCondition : Level::Condition {
    Fixed        value;
    virtual bool is_true() const;
};

// Ops: EQ, NE, LT, GT, LE, GE
// Precondition: `subject` exists.
// Compares `subject` to the control, target, or flagship of the local player, per `value`.
//
// Warning: not net-safe.
struct Level::SubjectCondition : Level::Condition {
    SubjectValue value;
    virtual bool is_true() const;
};

// Ops: EQ, NE, LT, GT, LE, GE
// Compares `subject` to the control, target, or flagship of the local player, per `value`.
//
// Note: On a level that specifies a `start_time`, the setup time counts for only 1/3 as much as
// time after the
//
// TODO(sfiera): provide a way to specify game time “normally”
struct Level::TimeCondition : Level::Condition {
    ticks        value;
    virtual bool is_true() const;
};

// Ops: EQ, NE, LT, GT, LE, GE
// Compares zoom level of the local player to `value`.
//
// Warning: not net-safe.
struct Level::ZoomCondition : Level::Condition {
    Zoom         value;
    virtual bool is_true() const;
};

//
// We need to know:
// type of tour point: object, absolute, or free-standing
// either level object # & visible --or-- location ((int32_t & bool) or longPoint)
// range (longPoint)
// title ID, # (int16_t, int16_t)
// content ID, # (int16_t, int16_t)
//

struct Level_Briefing {
    Handle<const Level::Initial> object;   // Object to focus on, or none for freestanding.
    pn::string                   title;    // Plain text, used for title bar.
    pn::string                   content;  // Styled text, used for body.

    static const size_t byte_size = 24;
};

}  // namespace antares

#endif  // ANTARES_DATA_LEVEL_HPP_

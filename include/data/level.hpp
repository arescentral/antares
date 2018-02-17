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
    struct Condition;
    struct Briefing;

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

struct Level::Condition {
    struct CounterArgument {
        Handle<Admiral> whichPlayer;
        int32_t         whichCounter;
        int32_t         amount;
    };

    conditionType condition;
    struct {
        // Really a union
        Point           location;
        CounterArgument counter;
        int32_t         longValue;
        Fixed           fixedValue;
        ticks           timeValue;
        uint32_t        unsignedLongValue;
    } conditionArgument;

    int32_t             subject           = -1;  // initial object #
    int32_t             object            = -1;  // initial object #
    bool                initially_enabled = true;
    bool                persistent        = false;
    std::vector<Action> action;

    // Transient information during level.
    // TODO(sfiera): remove.
    bool enabled = true;

    static const size_t byte_size = 38;

    bool active() const;
    bool is_true() const;
};
bool read_from(pn::file_view in, Level::Condition* level_condition);
bool read_from(pn::file_view in, Level::Condition::CounterArgument* counter_argument);
std::vector<Level::Condition> read_conditions(int begin, int end);

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

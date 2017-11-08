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

#include "data/level.hpp"

#include <sfz/sfz.hpp>

#include "data/plugin.hpp"

using sfz::BytesSlice;
using sfz::ReadSource;
using sfz::String;
using sfz::read;
namespace macroman = sfz::macroman;

namespace antares {

static const int16_t kLevel_StartTimeMask  = 0x7fff;
static const int16_t kLevel_IsTraining_Bit = 0x8000;

namespace {

void read_pstr(ReadSource in, String& out) {
    uint8_t bytes[256];
    read(in, bytes, 256);
    BytesSlice encoded(bytes + 1, bytes[0]);
    out.assign(macroman::decode(encoded));
}

}  // namespace

Level* Level::get(int n) {
    return &plug.levels[n];
}

void read_from(ReadSource in, scenarioInfoType& scenario_info) {
    scenario_info.warpInFlareID  = Handle<BaseObject>(read<int32_t>(in));
    scenario_info.warpOutFlareID = Handle<BaseObject>(read<int32_t>(in));
    scenario_info.playerBodyID   = Handle<BaseObject>(read<int32_t>(in));
    scenario_info.energyBlobID   = Handle<BaseObject>(read<int32_t>(in));
    read_pstr(in, scenario_info.downloadURLString);
    read_pstr(in, scenario_info.titleString);
    read_pstr(in, scenario_info.authorNameString);
    read_pstr(in, scenario_info.authorURLString);
    read(in, scenario_info.version);
    in.shift(12);
}

void read_from(ReadSource in, Level& level) {
    read(in, level.netRaceFlags);
    read(in, level.playerNum);
    read(in, level.player, kMaxPlayerNum);
    read(in, level.scoreStringResID);
    read(in, level.initialFirst);
    read(in, level.prologueID);
    read(in, level.initialNum);
    read(in, level.songID);
    read(in, level.conditionFirst);
    read(in, level.epilogueID);
    read(in, level.conditionNum);
    read(in, level.starMapH);
    read(in, level.briefPointFirst);
    read(in, level.starMapV);
    read(in, level.briefPointNum);
    level.parTime = game_ticks(secs(read<int16_t>(in)));
    in.shift(2);
    read(in, level.parKills);
    read(in, level.levelNameStrNum);
    read(in, level.parKillRatio);
    read(in, level.parLosses);
    int16_t start_time = read<int16_t>(in);
    level.startTime    = secs(start_time & kLevel_StartTimeMask);
    level.is_training  = start_time & kLevel_IsTraining_Bit;
}

void read_from(ReadSource in, Level::Player& level_player) {
    read(in, level_player.playerType);
    read(in, level_player.playerRace);
    read(in, level_player.nameResID);
    read(in, level_player.nameStrNum);
    in.shift(4);
    read(in, level_player.earningPower);
    read(in, level_player.netRaceFlags);
    in.shift(2);
}

static void read_action(sfz::ReadSource in, Level::Condition& condition) {
    auto start       = read<int32_t>(in);
    auto count       = read<int32_t>(in);
    auto end         = (start >= 0) ? (start + count) : start;
    condition.action = {start, end};
}

void read_from(ReadSource in, Level::Condition& level_condition) {
    uint8_t section[12];

    read(in, level_condition.condition);
    in.shift(1);
    read(in, section, 12);
    read(in, level_condition.subjectObject);
    read(in, level_condition.directObject);
    read_action(in, level_condition);
    read(in, level_condition.flags);
    read(in, level_condition.direction);

    BytesSlice sub(section, 12);
    switch (level_condition.condition) {
        case kCounterCondition:
        case kCounterGreaterCondition:
        case kCounterNotCondition: read(sub, level_condition.conditionArgument.counter); break;

        case kDestructionCondition:
        case kOwnerCondition:
        case kNoShipsLeftCondition:
        case kZoomLevelCondition: read(sub, level_condition.conditionArgument.longValue); break;

        case kVelocityLessThanEqualToCondition:
            read(sub, level_condition.conditionArgument.fixedValue);

        case kTimeCondition:
            level_condition.conditionArgument.timeValue = ticks(read<int32_t>(sub));
            break;

        case kProximityCondition:
        case kDistanceGreaterCondition:
            read(sub, level_condition.conditionArgument.unsignedLongValue);
            break;

        case kCurrentMessageCondition:
        case kCurrentComputerCondition:
            read(sub, level_condition.conditionArgument.location);
            break;
    }
}

void read_from(ReadSource in, Level::Condition::CounterArgument& counter_argument) {
    counter_argument.whichPlayer = Handle<Admiral>(read<int32_t>(in));
    read(in, counter_argument.whichCounter);
    read(in, counter_argument.amount);
}

void read_from(ReadSource in, Level::BriefPoint& brief_point) {
    uint8_t section[8];

    read(in, brief_point.briefPointKind);
    in.shift(1);
    read(in, section, 8);
    read(in, brief_point.range);
    read(in, brief_point.titleResID);
    read(in, brief_point.titleNum);
    read(in, brief_point.contentResID);

    BytesSlice sub(section, 8);
    switch (brief_point.briefPointKind) {
        case kNoPointKind:
        case kBriefFreestandingKind: break;

        case kBriefObjectKind: read(sub, brief_point.briefPointData.objectBriefType); break;

        case kBriefAbsoluteKind: read(sub, brief_point.briefPointData.absoluteBriefType); break;
    }
}

void read_from(ReadSource in, Level::BriefPoint::ObjectBrief& object_brief) {
    read(in, object_brief.objectNum);
    read(in, object_brief.objectVisible);
}

void read_from(ReadSource in, Level::BriefPoint::AbsoluteBrief& absolute_brief) {
    read(in, absolute_brief.location);
}

void read_from(ReadSource in, Level::InitialObject& level_initial) {
    level_initial.type  = Handle<BaseObject>(read<int32_t>(in));
    level_initial.owner = Handle<Admiral>(read<int32_t>(in));
    in.shift(4);
    level_initial.realObject = Handle<SpaceObject>(-1);
    read(in, level_initial.realObjectID);
    read(in, level_initial.location);
    read(in, level_initial.earning);
    read(in, level_initial.distanceRange);
    read(in, level_initial.rotationMinimum);
    read(in, level_initial.rotationRange);
    read(in, level_initial.spriteIDOverride);
    read(in, level_initial.canBuild, kMaxTypeBaseCanBuild);
    read(in, level_initial.initialDestination);
    read(in, level_initial.nameResID);
    read(in, level_initial.nameStrNum);
    read(in, level_initial.attributes);
}

}  // namespace antares

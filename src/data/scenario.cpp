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

#include "data/scenario.hpp"

#include <sfz/sfz.hpp>

using sfz::BytesSlice;
using sfz::ReadSource;
using sfz::String;
using sfz::read;
namespace macroman = sfz::macroman;

namespace antares {

namespace {

void read_pstr(ReadSource in, String& out) {
    uint8_t bytes[256];
    read(in, bytes, 256);
    BytesSlice encoded(bytes + 1, bytes[0]);
    out.assign(macroman::decode(encoded));
}

}  // namespace

void read_from(ReadSource in, scenarioInfoType& scenario_info) {
    scenario_info.warpInFlareID = Handle<BaseObject>(read<int32_t>(in));
    scenario_info.warpOutFlareID = Handle<BaseObject>(read<int32_t>(in));
    scenario_info.playerBodyID = Handle<BaseObject>(read<int32_t>(in));
    scenario_info.energyBlobID = Handle<BaseObject>(read<int32_t>(in));
    read_pstr(in, scenario_info.downloadURLString);
    read_pstr(in, scenario_info.titleString);
    read_pstr(in, scenario_info.authorNameString);
    read_pstr(in, scenario_info.authorURLString);
    read(in, scenario_info.version);
    read(in, scenario_info.requiresAresVersion);
    read(in, scenario_info.flags);
    read(in, scenario_info.checkSum);
}

void read_from(ReadSource in, Scenario& scenario) {
    read(in, scenario.netRaceFlags);
    read(in, scenario.playerNum);
    read(in, scenario.player, kMaxPlayerNum);
    read(in, scenario.scoreStringResID);
    read(in, scenario.initialFirst);
    read(in, scenario.prologueID);
    read(in, scenario.initialNum);
    read(in, scenario.songID);
    read(in, scenario.conditionFirst);
    read(in, scenario.epilogueID);
    read(in, scenario.conditionNum);
    read(in, scenario.starMapH);
    read(in, scenario.briefPointFirst);
    read(in, scenario.starMapV);
    read(in, scenario.briefPointNum);
    scenario.parTime = secs(read<int16_t>(in));
    in.shift(2);
    read(in, scenario.parKills);
    read(in, scenario.levelNameStrNum);
    read(in, scenario.parKillRatio);
    read(in, scenario.parLosses);
    read(in, scenario.startTime);
}

void read_from(ReadSource in, Scenario::Player& scenario_player) {
    read(in, scenario_player.playerType);
    read(in, scenario_player.playerRace);
    read(in, scenario_player.nameResID);
    read(in, scenario_player.nameStrNum);
    in.shift(4);
    read(in, scenario_player.earningPower);
    read(in, scenario_player.netRaceFlags);
    in.shift(2);
}

static void read_action(sfz::ReadSource in, Scenario::Condition& condition) {
    auto start = read<int32_t>(in);
    auto count = read<int32_t>(in);
    auto end = (start >= 0) ? (start + count) : start;
    condition.action = {start, end};
}

void read_from(ReadSource in, Scenario::Condition& scenario_condition) {
    uint8_t section[12];

    read(in, scenario_condition.condition);
    in.shift(1);
    read(in, section, 12);
    read(in, scenario_condition.subjectObject);
    read(in, scenario_condition.directObject);
    read_action(in, scenario_condition);
    read(in, scenario_condition.flags);
    read(in, scenario_condition.direction);

    BytesSlice sub(section, 12);
    switch (scenario_condition.condition) {
      case kCounterCondition:
      case kCounterGreaterCondition:
      case kCounterNotCondition:
        read(sub, scenario_condition.conditionArgument.counter);
        break;

      case kDestructionCondition:
      case kOwnerCondition:
      case kTimeCondition:
      case kVelocityLessThanEqualToCondition:
      case kNoShipsLeftCondition:
      case kZoomLevelCondition:
        read(sub, scenario_condition.conditionArgument.longValue);
        break;

      case kProximityCondition:
      case kDistanceGreaterCondition:
        read(sub, scenario_condition.conditionArgument.unsignedLongValue);
        break;

      case kCurrentMessageCondition:
      case kCurrentComputerCondition:
        read(sub, scenario_condition.conditionArgument.location);
        break;
    }
}

void read_from(ReadSource in, Scenario::Condition::CounterArgument& counter_argument) {
    counter_argument.whichPlayer = Handle<Admiral>(read<int32_t>(in));
    read(in, counter_argument.whichCounter);
    read(in, counter_argument.amount);
}

void read_from(ReadSource in, Scenario::BriefPoint& brief_point) {
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
      case kBriefFreestandingKind:
        break;

      case kBriefObjectKind:
        read(sub, brief_point.briefPointData.objectBriefType);
        break;

      case kBriefAbsoluteKind:
        read(sub, brief_point.briefPointData.absoluteBriefType);
        break;
    }
}

void read_from(ReadSource in, Scenario::BriefPoint::ObjectBrief& object_brief) {
    read(in, object_brief.objectNum);
    read(in, object_brief.objectVisible);
}

void read_from(ReadSource in, Scenario::BriefPoint::AbsoluteBrief& absolute_brief) {
    read(in, absolute_brief.location);
}

void read_from(ReadSource in, Scenario::InitialObject& scenario_initial) {
    scenario_initial.type = Handle<BaseObject>(read<int32_t>(in));
    scenario_initial.owner = Handle<Admiral>(read<int32_t>(in));
    in.shift(4);
    scenario_initial.realObject = Handle<SpaceObject>(-1);
    read(in, scenario_initial.realObjectID);
    read(in, scenario_initial.location);
    read(in, scenario_initial.earning);
    read(in, scenario_initial.distanceRange);
    read(in, scenario_initial.rotationMinimum);
    read(in, scenario_initial.rotationRange);
    read(in, scenario_initial.spriteIDOverride);
    read(in, scenario_initial.canBuild, kMaxTypeBaseCanBuild);
    read(in, scenario_initial.initialDestination);
    read(in, scenario_initial.nameResID);
    read(in, scenario_initial.nameStrNum);
    read(in, scenario_initial.attributes);
}

}  // namespace antares

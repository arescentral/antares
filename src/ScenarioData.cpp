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

#include "ScenarioData.hpp"

#include <sfz/sfz.hpp>
#include "Scenario.hpp"

using sfz::BytesPiece;
using sfz::ReadSource;
using sfz::read;

namespace antares {

void read_from(ReadSource in, scenarioInfoType* scenario_info) {
    read(in, &scenario_info->warpInFlareID);
    read(in, &scenario_info->warpOutFlareID);
    read(in, &scenario_info->playerBodyID);
    read(in, &scenario_info->energyBlobID);
    read(in, scenario_info->downloadURLString, 256);
    read(in, scenario_info->titleString, 256);
    read(in, scenario_info->authorNameString, 256);
    read(in, scenario_info->authorURLString, 256);
    read(in, &scenario_info->version);
    read(in, &scenario_info->requiresAresVersion);
    read(in, &scenario_info->flags);
    read(in, &scenario_info->checkSum);
}

void read_from(ReadSource in, scenarioType* scenario) {
    read(in, &scenario->netRaceFlags);
    read(in, &scenario->playerNum);
    read(in, scenario->player, kScenarioPlayerNum);
    read(in, &scenario->scoreStringResID);
    read(in, &scenario->initialFirst);
    read(in, &scenario->prologueID);
    read(in, &scenario->initialNum);
    read(in, &scenario->songID);
    read(in, &scenario->conditionFirst);
    read(in, &scenario->epilogueID);
    read(in, &scenario->conditionNum);
    read(in, &scenario->starMapH);
    read(in, &scenario->briefPointFirst);
    read(in, &scenario->starMapV);
    read(in, &scenario->briefPointNum);
    read(in, &scenario->parTime);
    in.shift(2);
    read(in, &scenario->parKills);
    read(in, &scenario->levelNameStrNum);
    read(in, &scenario->parKillRatio);
    read(in, &scenario->parLosses);
    read(in, &scenario->startTime);
}

void read_from(ReadSource in, scenarioPlayerType* scenario_player) {
    read(in, &scenario_player->playerType);
    read(in, &scenario_player->playerRace);
    read(in, &scenario_player->nameResID);
    read(in, &scenario_player->nameStrNum);
    read(in, &scenario_player->admiralNumber);
    read(in, &scenario_player->earningPower);
    read(in, &scenario_player->netRaceFlags);
    in.shift(2);
}

void read_from(ReadSource in, scenarioConditionType* scenario_condition) {
    uint8_t section[12];

    read(in, &scenario_condition->condition);
    in.shift(1);
    read(in, section, 12);
    read(in, &scenario_condition->subjectObject);
    read(in, &scenario_condition->directObject);
    read(in, &scenario_condition->startVerb);
    read(in, &scenario_condition->verbNum);
    read(in, &scenario_condition->flags);
    read(in, &scenario_condition->direction);

    BytesPiece sub(section, 12);
    switch (scenario_condition->condition) {
      case kCounterCondition:
      case kCounterGreaterCondition:
      case kCounterNotCondition:
        read(&sub, &scenario_condition->conditionArgument.counter);
        break;

      case kDestructionCondition:
      case kOwnerCondition:
      case kTimeCondition:
      case kVelocityLessThanEqualToCondition:
      case kNoShipsLeftCondition:
      case kZoomLevelCondition:
        read(&sub, &scenario_condition->conditionArgument.longValue);
        break;

      case kProximityCondition:
      case kDistanceGreaterCondition:
        read(&sub, &scenario_condition->conditionArgument.unsignedLongValue);
        break;

      case kCurrentMessageCondition:
      case kCurrentComputerCondition:
        read(&sub, &scenario_condition->conditionArgument.location);
        break;
    }
}

void read_from(ReadSource in, counterArgumentType* counter_argument) {
    read(in, &counter_argument->whichPlayer);
    read(in, &counter_argument->whichCounter);
    read(in, &counter_argument->amount);
}

void read_from(ReadSource in, briefPointType* brief_point) {
    uint8_t section[8];

    read(in, &brief_point->briefPointKind);
    in.shift(1);
    read(in, section, 8);
    read(in, &brief_point->range);
    read(in, &brief_point->titleResID);
    read(in, &brief_point->titleNum);
    read(in, &brief_point->contentResID);

    BytesPiece sub(section, 8);
    switch (brief_point->briefPointKind) {
      case kNoPointKind:
      case kBriefFreestandingKind:
        break;

      case kBriefObjectKind:
        read(&sub, &brief_point->briefPointData.objectBriefType);
        break;

      case kBriefAbsoluteKind:
        read(&sub, &brief_point->briefPointData.absoluteBriefType);
        break;
    }
}

void read_from(ReadSource in, briefPointType::ObjectBrief* object_brief) {
    read(in, &object_brief->objectNum);
    read(in, &object_brief->objectVisible);
}

void read_from(ReadSource in, briefPointType::AbsoluteBrief* absolute_brief) {
    read(in, &absolute_brief->location);
}

void read_from(ReadSource in, scenarioInitialType* scenario_initial) {
    read(in, &scenario_initial->type);
    read(in, &scenario_initial->owner);
    read(in, &scenario_initial->realObjectNumber);
    read(in, &scenario_initial->realObjectID);
    read(in, &scenario_initial->location);
    read(in, &scenario_initial->earning);
    read(in, &scenario_initial->distanceRange);
    read(in, &scenario_initial->rotationMinimum);
    read(in, &scenario_initial->rotationRange);
    read(in, &scenario_initial->spriteIDOverride);
    read(in, scenario_initial->canBuild, kMaxTypeBaseCanBuild);
    read(in, &scenario_initial->initialDestination);
    read(in, &scenario_initial->nameResID);
    read(in, &scenario_initial->nameStrNum);
    read(in, &scenario_initial->attributes);
}

}  // namespace antares

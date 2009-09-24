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

#include "BinaryStream.hpp"
#include "Scenario.hpp"

size_t scenarioInfoType::load_data(const char* data, size_t len) {
    BufferBinaryReader bin(data, len);

    bin.read(&warpInFlareID);
    bin.read(&warpOutFlareID);
    bin.read(&playerBodyID);
    bin.read(&energyBlobID);
    bin.read(downloadURLString, 256);
    bin.read(titleString, 256);
    bin.read(authorNameString, 256);
    bin.read(authorURLString, 256);
    bin.read(&version);
    bin.read(&requiresAresVersion);
    bin.read(&flags);
    bin.read(&checkSum);

    return bin.bytes_read();
}

size_t scenarioType::load_data(const char* data, size_t len) {
    BufferBinaryReader bin(data, len);

    bin.read(&netRaceFlags);
    bin.read(&playerNum);
    bin.read(player, kScenarioPlayerNum);
    bin.read(&scoreStringResID);
    bin.read(&initialFirst);
    bin.read(&prologueID);
    bin.read(&initialNum);
    bin.read(&songID);
    bin.read(&conditionFirst);
    bin.read(&epilogueID);
    bin.read(&conditionNum);
    bin.read(&starMapH);
    bin.read(&briefPointFirst);
    bin.read(&starMapV);
    bin.read(&briefPointNum);
    bin.read(&parTime);
    bin.read(&movieNameStrNum);
    bin.read(&parKills);
    bin.read(&levelNameStrNum);
    bin.read(&parKillRatio);
    bin.read(&parLosses);
    bin.read(&startTime);

    return bin.bytes_read();
}

void scenarioPlayerType::read(BinaryReader* bin) {
    bin->read(&playerType);
    bin->read(&playerRace);
    bin->read(&nameResID);
    bin->read(&nameStrNum);
    bin->read(&admiralNumber);
    bin->read(&earningPower);
    bin->read(&netRaceFlags);
    bin->discard(2);
}

size_t scenarioConditionType::load_data(const char* data, size_t len) {
    BufferBinaryReader bin(data, len);
    char section[12];

    bin.read(&condition);
    bin.discard(1);
    bin.read(section, 12);
    bin.read(&subjectObject);
    bin.read(&directObject);
    bin.read(&startVerb);
    bin.read(&verbNum);
    bin.read(&flags);
    bin.read(&direction);

    BufferBinaryReader sub(section, 12);
    switch (condition) {
      case kCounterCondition:
      case kCounterGreaterCondition:
      case kCounterNotCondition:
        sub.read(&conditionArgument.counter);
        break;

      case kDestructionCondition:
      case kOwnerCondition:
      case kTimeCondition:
      case kVelocityLessThanEqualToCondition:
      case kNoShipsLeftCondition:
      case kZoomLevelCondition:
        sub.read(&conditionArgument.longValue);
        break;

      case kProximityCondition:
      case kDistanceGreaterCondition:
        sub.read(&conditionArgument.unsignedLongValue);
        break;

      case kCurrentMessageCondition:
      case kCurrentComputerCondition:
        sub.read(&conditionArgument.location);
        break;
    }

    return bin.bytes_read();
}

void counterArgumentType::read(BinaryReader* bin) {
    bin->read(&whichPlayer);
    bin->read(&whichCounter);
    bin->read(&amount);
}

size_t briefPointType::load_data(const char* data, size_t len) {
    BufferBinaryReader bin(data, len);

    char section[8];

    bin.read(&briefPointKind);
    bin.discard(1);
    bin.read(section, 8);
    bin.read(&range);
    bin.read(&titleResID);
    bin.read(&titleNum);
    bin.read(&contentResID);

    BufferBinaryReader sub(section, 8);
    switch (briefPointKind) {
      case kNoPointKind:
      case kBriefFreestandingKind:
        break;

      case kBriefObjectKind:
        sub.read(&briefPointData.objectBriefType);
        break;

      case kBriefAbsoluteKind:
        sub.read(&briefPointData.absoluteBriefType);
        break;
    }

    return bin.bytes_read();
}

void briefPointType::ObjectBrief::read(BinaryReader* bin) {
    bin->read(&objectNum);
    bin->read(&objectVisible);
}

void briefPointType::AbsoluteBrief::read(BinaryReader* bin) {
    bin->read(&location);
}

size_t scenarioInitialType::load_data(const char* data, size_t len) {
    BufferBinaryReader bin(data, len);

    bin.read(&type);
    bin.read(&owner);
    bin.read(&realObjectNumber);
    bin.read(&realObjectID);
    bin.read(&location);
    bin.read(&earning);
    bin.read(&distanceRange);
    bin.read(&rotationMinimum);
    bin.read(&rotationRange);
    bin.read(&spriteIDOverride);
    bin.read(canBuild, kMaxTypeBaseCanBuild);
    bin.read(&initialDestination);
    bin.read(&nameResID);
    bin.read(&nameStrNum);
    bin.read(&attributes);

    return bin.bytes_read();
}

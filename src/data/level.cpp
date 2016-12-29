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
#include "data/pn.hpp"

using sfz::ReadSource;
using sfz::read;
namespace macroman = sfz::macroman;

namespace antares {

static const int16_t kLevel_StartTimeMask  = 0x7fff;
static const int16_t kLevel_IsTraining_Bit = 0x8000;

namespace {

bool read_pstr(pn::file_view in, pn::string* out) {
    uint8_t bytes[256];
    if (fread(bytes, 1, 256, in.c_obj()) < 256) {
        return false;
    }
    pn::data_view encoded{bytes + 1, bytes[0]};
    *out = sfz2pn(macroman::decode({encoded.data(), static_cast<size_t>(encoded.size())}));
    return true;
}

}  // namespace

Level* Level::get(int n) { return &plug.levels[n]; }

bool read_from(pn::file_view in, scenarioInfoType* scenario_info) {
    int32_t warp_in, warp_out, body, energy;
    uint8_t unused[12];
    if (!(in.read(&warp_in, &warp_out, &body, &energy) &&
          read_pstr(in, &scenario_info->downloadURLString) &&
          read_pstr(in, &scenario_info->titleString) &&
          read_pstr(in, &scenario_info->authorNameString) &&
          read_pstr(in, &scenario_info->authorURLString) && in.read(&scenario_info->version) &&
          (fread(unused, 1, 12, in.c_obj()) == 12))) {
        return false;
    }

    scenario_info->warpInFlareID  = Handle<BaseObject>(warp_in);
    scenario_info->warpOutFlareID = Handle<BaseObject>(warp_out);
    scenario_info->playerBodyID   = Handle<BaseObject>(body);
    scenario_info->energyBlobID   = Handle<BaseObject>(energy);
    return true;
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

static bool read_action(pn::file_view in, Level::Condition* condition) {
    int32_t start, count;
    if (!in.read(&start, &count)) {
        return false;
    }
    auto end          = (start >= 0) ? (start + count) : start;
    condition->action = {start, end};
    return true;
}

bool read_from(pn::file_view in, Level::Condition* level_condition) {
    uint8_t section[12];

    uint8_t unused;
    if (!(in.read(&level_condition->condition, &unused) &&
          (fread(section, 1, 12, in.c_obj()) == 12) &&
          in.read(&level_condition->subjectObject, &level_condition->directObject) &&
          read_action(in, level_condition) &&
          in.read(&level_condition->flags, &level_condition->direction))) {
        return false;
    }

    pn::file sub = pn::data_view{section, 12}.open();
    switch (level_condition->condition) {
        case kCounterCondition:
        case kCounterGreaterCondition:
        case kCounterNotCondition:
            return read_from(sub, &level_condition->conditionArgument.counter);

        case kDestructionCondition:
        case kOwnerCondition:
        case kNoShipsLeftCondition:
        case kZoomLevelCondition: return sub.read(&level_condition->conditionArgument.longValue);

        case kVelocityLessThanEqualToCondition:
            return read_from(sub, &level_condition->conditionArgument.fixedValue);

        case kTimeCondition: {
            int32_t time;
            if (!sub.read(&time)) {
                return false;
            }
            level_condition->conditionArgument.timeValue = ticks(time);
            return true;
        }

        case kProximityCondition:
        case kDistanceGreaterCondition:
            return sub.read(&level_condition->conditionArgument.unsignedLongValue);

        case kCurrentMessageCondition:
        case kCurrentComputerCondition:
            return read_from(sub, &level_condition->conditionArgument.location);

        default: return true;
    }
}

bool read_from(pn::file_view in, Level::Condition::CounterArgument* counter_argument) {
    int32_t admiral;
    if (!in.read(&admiral, &counter_argument->whichCounter, &counter_argument->amount)) {
        return false;
    }
    counter_argument->whichPlayer = Handle<Admiral>(admiral);
    return true;
}

bool read_from(pn::file_view in, Level::BriefPoint* brief_point) {
    uint8_t unused;
    uint8_t section[8];
    if (!(in.read(&brief_point->briefPointKind, &unused) &&
          (fread(section, 1, 8, in.c_obj()) == 8) && read_from(in, &brief_point->range) &&
          in.read(&brief_point->titleResID, &brief_point->titleNum, &brief_point->contentResID))) {
        return false;
    }

    pn::file sub = pn::data_view{section, 8}.open();
    switch (brief_point->briefPointKind) {
        case kNoPointKind:
        case kBriefFreestandingKind: return true;

        case kBriefObjectKind: return read_from(sub, &brief_point->briefPointData.objectBriefType);

        case kBriefAbsoluteKind:
            return read_from(sub, &brief_point->briefPointData.absoluteBriefType);

        default: return false;
    }
}

bool read_from(pn::file_view in, Level::BriefPoint::ObjectBrief* object_brief) {
    return in.read(&object_brief->objectNum, &object_brief->objectVisible);
}

bool read_from(pn::file_view in, Level::BriefPoint::AbsoluteBrief* absolute_brief) {
    return read_from(in, &absolute_brief->location);
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

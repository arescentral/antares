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
    *out = macroman::decode(encoded);
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

bool read_from(pn::file_view in, Level* level) {
    if (!in.read(&level->netRaceFlags, &level->playerNum)) {
        return false;
    }
    for (size_t i = 0; i < kMaxPlayerNum; ++i) {
        if (!read_from(in, &level->player[i])) {
            return false;
        }
    }
    int16_t par_time, start_time, unused;
    if (!(in.read(&level->scoreStringResID, &level->initialFirst, &level->prologueID,
                  &level->initialNum, &level->songID, &level->conditionFirst, &level->epilogueID,
                  &level->conditionNum, &level->starMapH, &level->briefPointFirst,
                  &level->starMapV, &level->briefPointNum, &par_time, &unused, &level->parKills,
                  &level->levelNameStrNum) &&
          read_from(in, &level->parKillRatio) && in.read(&level->parLosses, &start_time))) {
        return false;
    }
    level->parTime     = game_ticks(secs(par_time));
    level->startTime   = secs(start_time & kLevel_StartTimeMask);
    level->is_training = start_time & kLevel_IsTraining_Bit;
    return true;
}

bool read_from(pn::file_view in, Level::Player* level_player) {
    uint32_t unused1;
    uint16_t unused2;
    return in.read(&level_player->playerType, &level_player->playerRace, &level_player->nameResID,
                   &level_player->nameStrNum, &unused1) &&
           read_from(in, &level_player->earningPower) &&
           in.read(&level_player->netRaceFlags, &unused2);
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

bool read_from(pn::file_view in, Level::InitialObject* level_initial) {
    int32_t type, owner;
    uint8_t unused[4];
    if (!(in.read(&type, &owner) && (fread(unused, 1, 4, in.c_obj()) == 4) &&
          in.read(&level_initial->realObjectID) && read_from(in, &level_initial->location) &&
          read_from(in, &level_initial->earning) &&
          in.read(&level_initial->distanceRange, &level_initial->rotationMinimum,
                  &level_initial->rotationRange, &level_initial->spriteIDOverride))) {
        return false;
    }
    for (size_t i = 0; i < kMaxTypeBaseCanBuild; ++i) {
        if (!in.read(&level_initial->canBuild[i])) {
            return false;
        }
    }
    if (!in.read(
                &level_initial->initialDestination, &level_initial->nameResID,
                &level_initial->nameStrNum, &level_initial->attributes)) {
        return false;
    }
    level_initial->realObject = Handle<SpaceObject>(-1);
    level_initial->type       = Handle<BaseObject>(type);
    level_initial->owner      = Handle<Admiral>(owner);
    return true;
}

}  // namespace antares

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
#include "data/resource.hpp"

namespace macroman = sfz::macroman;

namespace antares {

static const int16_t kLevel_StartTimeMask  = 0x7fff;
static const int16_t kLevel_IsTraining_Bit = 0x8000;

const int16_t kLevelOwnNoShipTextID = 10000;
const int16_t kLevelFoeNoShipTextID = 10050;

Level* Level::get(int n) { return &plug.levels[n]; }

bool read_from(pn::file_view in, ScenarioInfo* info) {
    pn::value  x;
    pn_error_t error;
    if (!pn::parse(in, x, &error)) {
        return false;
    }
    pn::map_cref m = x.as_map();
    for (pn::string_view field : {"title", "download_url", "author", "author_url", "version"}) {
        if (m.get(field).as_string().empty()) {
            return false;
        }
    }
    for (pn::string_view field :
         {"warp_in_flare", "warp_out_flare", "player_body", "energy_blob"}) {
        if (!m.has(field) || !m.get(field).is_int()) {
            return false;
        }
    }

    info->titleString       = m.get("title").as_string().copy();
    info->downloadURLString = m.get("download_url").as_string().copy();
    info->authorNameString  = m.get("author").as_string().copy();
    info->authorURLString   = m.get("author_url").as_string().copy();
    info->version           = m.get("version").as_string().copy();
    info->warpInFlareID     = Handle<BaseObject>(m.get("warp_in_flare").as_int());
    info->warpOutFlareID    = Handle<BaseObject>(m.get("warp_out_flare").as_int());
    info->playerBodyID      = Handle<BaseObject>(m.get("player_body").as_int());
    info->energyBlobID      = Handle<BaseObject>(m.get("energy_blob").as_int());

    info->intro_text = Resource::text(5600);
    info->about_text = Resource::text(6500);

    info->publisher_screen = nullptr;  // Don’t have permission to show ASW logo.
    info->ego_screen       = Resource::texture(2001);
    info->splash_screen    = Resource::texture(502);
    info->starmap          = Resource::texture(8000);

    return true;
}

bool read_from(pn::file_view in, Level* level) {
    if (!in.read(&level->netRaceFlags, &level->playerNum)) {
        return false;
    }

    level->type = Level::DEMO;
    for (size_t i = 0; i < kMaxPlayerNum; ++i) {
        if (!read_from(in, &level->player[i])) {
            return false;
        }
        if (level->player[i].playerType == kSingleHumanPlayer) {
            level->type = Level::SOLO;
        } else if (level->player[i].playerType == kNetworkHumanPlayer) {
            level->type = Level::NET;
        }
    }

    int16_t par_time, start_time, unused;
    int16_t score_string_id, prologue_id, epilogue_id;
    int16_t initial_first, initial_num;
    int16_t condition_first, condition_num;
    int16_t briefing_first, briefing_num;
    if (!(in.read(&score_string_id, &initial_first, &prologue_id, &initial_num, &level->songID,
                  &condition_first, &epilogue_id, &condition_num, &level->starMapH,
                  &briefing_first, &level->starMapV, &briefing_num, &par_time, &unused,
                  &level->parKills, &level->levelNameStrNum) &&
          read_from(in, &level->parKillRatio) && in.read(&level->parLosses, &start_time))) {
        return false;
    }
    if (score_string_id > 0) {
        level->score_strings = Resource::strings(score_string_id);
    }
    if (briefing_num & kLevelAngleMask) {
        level->angle = (((briefing_num & kLevelAngleMask) >> kLevelAngleShift) - 1) * 2;
        briefing_num &= ~kLevelAngleMask;
    } else {
        level->angle = -1;
    }

    level->initials   = read_initials(initial_first, initial_first + initial_num);
    level->conditions = read_conditions(condition_first, condition_first + condition_num);
    level->briefings  = read_briefings(briefing_first, briefing_first + briefing_num);

    switch (level->type) {
        case Level::DEMO: break;
        case Level::SOLO:
            try {
                level->own_no_ships_text =
                        Resource::text(kLevelOwnNoShipTextID + level->levelNameStrNum);
            } catch (std::runtime_error& e) {
                level->own_no_ships_text.clear();
            }
            if (prologue_id > 0) {
                level->prologue = Resource::text(prologue_id);
            }
            if (epilogue_id > 0) {
                level->epilogue = Resource::text(epilogue_id);
            }
            break;
        case Level::NET:
            level->own_no_ships_text =
                    Resource::text(kLevelOwnNoShipTextID + level->levelNameStrNum);
            level->foe_no_ships_text =
                    Resource::text(kLevelFoeNoShipTextID + level->levelNameStrNum);
            if (prologue_id > 0) {
                level->description = Resource::text(prologue_id);
            }
            break;
    }

    level->parTime     = game_ticks(secs(par_time));
    level->startTime   = secs(start_time & kLevel_StartTimeMask);
    level->is_training = start_time & kLevel_IsTraining_Bit;
    return true;
}

bool read_from(pn::file_view in, Level::Player* level_player) {
    uint32_t unused1;
    int16_t  name_id, name_index;
    uint16_t unused2;
    if (!(in.read(&level_player->playerType, &level_player->playerRace, &name_id, &name_index,
                  &unused1) &&
          read_from(in, &level_player->earningPower) &&
          in.read(&level_player->netRaceFlags, &unused2))) {
        return false;
    }
    if ((name_id > 0) && (name_index > 0)) {
        level_player->name = Resource::strings(name_id).at(name_index - 1).copy();
    }
    return true;
}

static bool read_action(pn::file_view in, Level::Condition* condition) {
    int32_t start, count;
    if (!in.read(&start, &count)) {
        return false;
    }
    auto end          = (start >= 0) ? (start + count) : start;
    condition->action = read_actions(start, end);
    return true;
}

bool read_from(pn::file_view in, Level::Condition* level_condition) {
    uint8_t section[12];

    uint32_t flags;
    uint8_t  condition;
    if (!(in.read(&condition, pn::pad(1)) && (fread(section, 1, 12, in.c_obj()) == 12) &&
          in.read(&level_condition->subject, &level_condition->object) &&
          read_action(in, level_condition) && in.read(&flags, pn::pad(4)))) {
        return false;
    }
    level_condition->condition         = static_cast<conditionType>(condition);
    level_condition->persistent        = !(flags & kTrueOnlyOnce);
    level_condition->initially_enabled = !(flags & kInitiallyTrue);

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

std::vector<Level::Condition> read_conditions(int begin, int end) {
    if (end <= begin) {
        return std::vector<Level::Condition>{};
    }
    Resource r = Resource::path("scenario-conditions.bin");

    pn::data_view d = r.data();
    if ((begin < 0) || ((d.size() / Level::Condition::byte_size) < end)) {
        throw std::runtime_error(pn::format(
                                         "condition range {{{0}, {1}}} outside {{0, {2}}}", begin,
                                         end, d.size() / Level::Condition::byte_size)
                                         .c_str());
    }

    int      count = end - begin;
    pn::file f = d.slice(Level::Condition::byte_size * begin, Level::Condition::byte_size * count)
                         .open();
    std::vector<Level::Condition> conditions;
    conditions.resize(count);
    for (Level::Condition& a : conditions) {
        read_from(f, &a);
    }
    return conditions;
}

bool read_from(pn::file_view in, Level::Condition::CounterArgument* counter_argument) {
    int32_t admiral;
    if (!in.read(&admiral, &counter_argument->whichCounter, &counter_argument->amount)) {
        return false;
    }
    counter_argument->whichPlayer = Handle<Admiral>(admiral);
    return true;
}

bool read_from(pn::file_view in, Level::Briefing* brief_point) {
    uint8_t kind;
    int16_t title_id, title_index, content_id;
    if (!in.read(
                &kind, pn::pad(1), &brief_point->object, pn::pad(12), &title_id, &title_index,
                &content_id)) {
        return false;
    }

    if (kind != 1) {
        brief_point->object = -1;
    }

    try {
        brief_point->title = Resource::strings(title_id).at(title_index - 1).copy();
    } catch (std::exception& e) {
        brief_point->title = "";
    }
    try {
        brief_point->content = Resource::text(content_id);
    } catch (std::exception& e) {
        brief_point->content = "";
    }
    return true;
}

std::vector<Level::Briefing> read_briefings(int begin, int end) {
    if (end <= begin) {
        return std::vector<Level::Briefing>{};
    }
    Resource r = Resource::path("scenario-briefings.bin");

    pn::data_view d = r.data();
    if ((begin < 0) || ((d.size() / Level::Briefing::byte_size) < end)) {
        throw std::runtime_error(pn::format(
                                         "briefing range {{{0}, {1}}} outside {{0, {2}}}", begin,
                                         end, d.size() / Level::Briefing::byte_size)
                                         .c_str());
    }

    int      count = end - begin;
    pn::file f =
            d.slice(Level::Briefing::byte_size * begin, Level::Briefing::byte_size * count).open();
    std::vector<Level::Briefing> briefings;
    briefings.resize(count);
    for (Level::Briefing& a : briefings) {
        read_from(f, &a);
    }
    return briefings;
}

bool read_from(pn::file_view in, Level::Initial* level_initial) {
    int32_t type, owner;
    uint8_t unused[4];
    if (!(in.read(&type, &owner) && (fread(unused, 1, 4, in.c_obj()) == 4) &&
          in.read(&level_initial->realObjectID) && read_from(in, &level_initial->at) &&
          read_from(in, &level_initial->earning) &&
          in.read(pn::pad(12), &level_initial->sprite_override))) {
        return false;
    }
    for (size_t i = 0; i < kMaxTypeBaseCanBuild; ++i) {
        if (!in.read(&level_initial->build[i])) {
            return false;
        }
    }
    int32_t  name_id, name_index;
    uint32_t attributes;
    if (!in.read(&level_initial->target, &name_id, &name_index, &attributes)) {
        return false;
    }
    level_initial->realObject = Handle<SpaceObject>(-1);
    level_initial->base       = Handle<BaseObject>(type);
    level_initial->owner      = Handle<Admiral>(owner);
    level_initial->attributes = Level::Initial::Attributes(attributes);
    if (name_id > 0) {
        level_initial->name_override = Resource::strings(name_id).at(name_index - 1).copy();
    } else {
        level_initial->name_override = "";
    }
    return true;
}

std::vector<Level::Initial> read_initials(int begin, int end) {
    if (end <= begin) {
        return std::vector<Level::Initial>{};
    }
    Resource r = Resource::path("scenario-initials.bin");

    pn::data_view d = r.data();
    if ((begin < 0) || ((d.size() / Level::Initial::byte_size) < end)) {
        throw std::runtime_error(pn::format(
                                         "initial range {{{0}, {1}}} outside {{0, {2}}}", begin,
                                         end, d.size() / Level::Initial::byte_size)
                                         .c_str());
    }

    int      count = end - begin;
    pn::file f =
            d.slice(Level::Initial::byte_size * begin, Level::Initial::byte_size * count).open();
    std::vector<Level::Initial> initials;
    initials.resize(count);
    for (Level::Initial& a : initials) {
        read_from(f, &a);
    }
    return initials;
}

}  // namespace antares

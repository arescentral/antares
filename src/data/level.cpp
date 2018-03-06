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

#include "data/field.hpp"
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
    for (pn::string_view field :
         {"title", "download_url", "author", "author_url", "version", "splash", "starmap"}) {
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

    info->intro_text = m.get("intro").as_string().copy();
    info->about_text = m.get("about").as_string().copy();

    info->publisher_screen = nullptr;  // Don’t have permission to show ASW logo.
    info->ego_screen       = Resource::texture("pictures/credit");
    info->splash_screen    = Resource::texture(m.get("splash").as_string());
    info->starmap          = Resource::texture(m.get("starmap").as_string());

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

static std::unique_ptr<Level::Condition> autopilot_condition(path_value x) {
    std::unique_ptr<Level::AutopilotCondition> c(new Level::AutopilotCondition);
    c->value = required_bool(x.get("value"));
    return std::move(c);
}

static std::unique_ptr<Level::Condition> building_condition(path_value x) {
    std::unique_ptr<Level::BuildingCondition> c(new Level::BuildingCondition);
    c->value = required_bool(x.get("value"));
    return std::move(c);
}

static std::unique_ptr<Level::Condition> computer_condition(path_value x) {
    std::unique_ptr<Level::ComputerCondition> c(new Level::ComputerCondition);
    c->screen = required_screen(x.get("screen"));
    c->line   = optional_int(x.get("line")).value_or(-1);
    return std::move(c);
}

static std::unique_ptr<Level::Condition> counter_condition(path_value x) {
    std::unique_ptr<Level::CounterCondition> c(new Level::CounterCondition);
    c->player  = required_admiral(x.get("player"));
    c->counter = required_int(x.get("counter"));
    c->value   = required_int(x.get("value"));
    return std::move(c);
}

static std::unique_ptr<Level::Condition> destroyed_condition(path_value x) {
    std::unique_ptr<Level::DestroyedCondition> c(new Level::DestroyedCondition);
    c->initial = required_initial(x.get("initial"));
    c->value   = required_bool(x.get("value"));
    return std::move(c);
}

static std::unique_ptr<Level::Condition> distance_condition(path_value x) {
    std::unique_ptr<Level::DistanceCondition> c(new Level::DistanceCondition);
    c->value = required_int(x.get("value"));
    return std::move(c);
}

static std::unique_ptr<Level::Condition> health_condition(path_value x) {
    std::unique_ptr<Level::HealthCondition> c(new Level::HealthCondition);
    c->value = required_double(x.get("value"));
    return std::move(c);
}

static std::unique_ptr<Level::Condition> message_condition(path_value x) {
    std::unique_ptr<Level::MessageCondition> c(new Level::MessageCondition);
    c->id   = required_int(x.get("id"));
    c->page = required_int(x.get("page"));
    return std::move(c);
}

static std::unique_ptr<Level::Condition> ordered_condition(path_value x) {
    return std::unique_ptr<Level::OrderedCondition>(new Level::OrderedCondition);
}

static std::unique_ptr<Level::Condition> owner_condition(path_value x) {
    std::unique_ptr<Level::OwnerCondition> c(new Level::OwnerCondition);
    c->player = required_admiral(x.get("player"));
    return std::move(c);
}

static std::unique_ptr<Level::Condition> ships_condition(path_value x) {
    std::unique_ptr<Level::ShipsCondition> c(new Level::ShipsCondition);
    c->player = required_admiral(x.get("player"));
    c->value  = required_int(x.get("value"));
    return std::move(c);
}

static std::unique_ptr<Level::Condition> speed_condition(path_value x) {
    std::unique_ptr<Level::SpeedCondition> c(new Level::SpeedCondition);
    c->value = required_fixed(x.get("value"));
    return std::move(c);
}

static std::unique_ptr<Level::Condition> subject_condition(path_value x) {
    std::unique_ptr<Level::SubjectCondition> c(new Level::SubjectCondition);
    c->value = required_subject_value(x.get("value"));
    return std::move(c);
}

static std::unique_ptr<Level::Condition> time_condition(path_value x) {
    std::unique_ptr<Level::TimeCondition> c(new Level::TimeCondition);
    c->value = required_ticks(x.get("value"));
    return std::move(c);
}

static std::unique_ptr<Level::Condition> zoom_condition(path_value x) {
    std::unique_ptr<Level::ZoomCondition> c(new Level::ZoomCondition);
    c->value = required_zoom(x.get("value"));
    return std::move(c);
}

static std::unique_ptr<Level::Condition> condition(pn::value_cref x0) {
    path_value x{x0};

    pn::string_view                   type = required_string(x.get("type"));
    std::unique_ptr<Level::Condition> c;
    if (type == "autopilot") {
        c = autopilot_condition(x);
    } else if (type == "building") {
        c = building_condition(x);
    } else if (type == "computer") {
        c = computer_condition(x);
    } else if (type == "counter") {
        c = counter_condition(x);
    } else if (type == "destroyed") {
        c = destroyed_condition(x);
    } else if (type == "distance") {
        c = distance_condition(x);
    } else if (type == "false") {
        return std::unique_ptr<Level::Condition>(new Level::FalseCondition);
    } else if (type == "health") {
        c = health_condition(x);
    } else if (type == "message") {
        c = message_condition(x);
    } else if (type == "ordered") {
        c = ordered_condition(x);
    } else if (type == "owner") {
        c = owner_condition(x);
    } else if (type == "ships") {
        c = ships_condition(x);
    } else if (type == "speed") {
        c = speed_condition(x);
    } else if (type == "subject") {
        c = subject_condition(x);
    } else if (type == "time") {
        c = time_condition(x);
    } else if (type == "zoom") {
        c = zoom_condition(x);
    } else {
        throw std::runtime_error(pn::format("unknown type: {0}", type).c_str());
    }

    c->op                = required_condition_op(x.get("op"));
    c->persistent        = optional_bool(x.get("persistent")).value_or(false);
    c->initially_enabled = !optional_bool(x.get("initially_disabled")).value_or(false);
    c->subject           = optional_initial(x.get("subject")).value_or(Level::Initial::none());
    c->object            = optional_initial(x.get("object")).value_or(Level::Initial::none());
    c->action            = required_action_array(x.get("action"));

    return c;
}

std::vector<std::unique_ptr<Level::Condition>> read_conditions(int begin, int end) {
    if (end <= begin) {
        return std::vector<std::unique_ptr<Level::Condition>>{};
    }

    std::vector<std::unique_ptr<Level::Condition>> conditions;
    conditions.resize(end - begin);
    for (int i : sfz::range(begin, end)) {
        pn::string path = pn::format("conditions/{0}.pn", i);
        try {
            Resource   r = Resource::path(path);
            pn::value  x;
            pn_error_t e;
            if (!pn::parse(r.data().open(), x, &e)) {
                throw std::runtime_error(
                        pn::format("{1}:{2}: {3}", e.lineno, e.column, pn_strerror(e.code))
                                .c_str());
            }
            conditions[i - begin] = condition(x);
        } catch (...) {
            std::throw_with_nested(std::runtime_error(path.c_str()));
        }
    }
    return conditions;
}

static Level::Briefing briefing(pn::value_cref x0) {
    if (!x0.is_map()) {
        throw std::runtime_error("must be map");
    }
    path_value x{x0};

    Level::Briefing b;
    b.object  = optional_initial(x.get("object")).value_or(Level::Initial::none());
    b.title   = required_string(x.get("title")).copy();
    b.content = required_string(x.get("content")).copy();
    return b;
}

std::vector<Level::Briefing> read_briefings(int begin, int end) {
    if (end <= begin) {
        return std::vector<Level::Briefing>{};
    }

    std::vector<Level::Briefing> briefings;
    briefings.resize(end - begin);
    for (int i : sfz::range(begin, end)) {
        pn::string path = pn::format("briefings/{0}.pn", i);
        try {
            Resource   r = Resource::path(path);
            pn::value  x;
            pn_error_t e;
            if (!pn::parse(r.data().open(), x, &e)) {
                throw std::runtime_error(
                        pn::format("{1}:{2}: {3}", e.lineno, e.column, pn_strerror(e.code))
                                .c_str());
            }
            briefings[i - begin] = briefing(x);
        } catch (...) {
            std::throw_with_nested(std::runtime_error(path.c_str()));
        }
    }
    return briefings;
}

bool read_from(pn::file_view in, Level::Initial* level_initial) {
    int32_t type, owner;
    if (!(in.read(&type, &owner, pn::pad(8)) && read_from(in, &level_initial->at) &&
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

    std::vector<Level::Initial> initials;
    initials.resize(end - begin);
    for (int i : sfz::range(begin, end)) {
        Resource r = Resource::path(pn::format("initials/{0}.bin", i));
        read_from(r.data().open(), &initials[i - begin]);
    }
    return initials;
}

}  // namespace antares

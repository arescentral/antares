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

bool read_from(pn::file_view in, Level::Condition* condition) {
    uint32_t flags;
    uint8_t  type;
    int32_t  subject, object;
    int32_t  action_start, action_count;
    pn::data section;
    section.resize(12);

    if (!in.read(
                &type, pn::pad(1), &section, &subject, &object, &action_start, &action_count,
                &flags, pn::pad(4))) {
        return false;
    }

    pn::file sub = section.open();
    switch (static_cast<conditionType>(type)) {
        case kNoCondition:
        case kLocationCondition:
        case kAgeCondition:
        case kRandomCondition: condition->init<Level::FalseCondition>(); break;

        case kHalfHealthCondition:
            condition->init<Level::HealthCondition>()->value = 0.5;
            (*condition)->op                                 = Level::ConditionBase::Op::LE;
            break;

        case kIsAuxiliaryObject:
            condition->init<Level::SubjectCondition>()->value =
                    Level::SubjectCondition::Value::CONTROL;
            (*condition)->op = Level::ConditionBase::Op::EQ;
            break;

        case kIsTargetObject:
            condition->init<Level::SubjectCondition>()->value =
                    Level::SubjectCondition::Value::TARGET;
            (*condition)->op = Level::ConditionBase::Op::EQ;
            break;

        case kObjectIsBeingBuilt:
            condition->init<Level::BuildingCondition>()->value = true;
            (*condition)->op                                   = Level::ConditionBase::Op::EQ;
            break;

        case kDirectIsSubjectTarget:
            condition->init<Level::OrderedCondition>();
            (*condition)->op = Level::ConditionBase::Op::EQ;
            break;

        case kSubjectIsPlayerCondition:
            condition->init<Level::SubjectCondition>()->value =
                    Level::SubjectCondition::Value::PLAYER;
            (*condition)->op = Level::ConditionBase::Op::EQ;
            break;

        case kAutopilotCondition:
            condition->init<Level::AutopilotCondition>()->value = true;
            (*condition)->op                                    = Level::ConditionBase::Op::EQ;
            break;

        case kNotAutopilotCondition:
            condition->init<Level::AutopilotCondition>()->value = false;
            (*condition)->op                                    = Level::ConditionBase::Op::EQ;
            break;

        case kCounterCondition: {
            auto*   counter = condition->init<Level::CounterCondition>();
            int32_t admiral;
            if (!sub.read(&admiral, &counter->counter, &counter->value)) {
                return false;
            }
            counter->player = Handle<Admiral>(admiral);
            counter->op     = Level::ConditionBase::Op::EQ;
            break;
        }

        case kCounterGreaterCondition: {
            auto*   counter = condition->init<Level::CounterCondition>();
            int32_t admiral;
            if (!sub.read(&admiral, &counter->counter, &counter->value)) {
                return false;
            }
            counter->player = Handle<Admiral>(admiral);
            counter->op     = Level::ConditionBase::Op::GE;
            break;
        }

        case kCounterNotCondition: {
            auto*   counter = condition->init<Level::CounterCondition>();
            int32_t admiral;
            if (!sub.read(&admiral, &counter->counter, &counter->value)) {
                return false;
            }
            counter->player = Handle<Admiral>(admiral);
            counter->op     = Level::ConditionBase::Op::NE;
            break;
        }

        case kDestructionCondition: {
            int32_t initial;
            if (!sub.read(&initial)) {
                return false;
            }
            auto* destroyed    = condition->init<Level::DestroyedCondition>();
            destroyed->initial = Handle<Level::Initial>(initial);
            destroyed->value   = true;
            destroyed->op      = Level::ConditionBase::Op::EQ;
            break;
        }

        case kOwnerCondition: {
            int32_t player;
            if (!sub.read(&player)) {
                return false;
            }
            condition->init<Level::OwnerCondition>()->player = Handle<Admiral>(player);
            (*condition)->op                                 = Level::ConditionBase::Op::EQ;
            break;
        }

        case kNoShipsLeftCondition: {
            int32_t player;
            if (!sub.read(&player)) {
                return false;
            }
            auto* ships   = condition->init<Level::ShipsCondition>();
            ships->player = Handle<Admiral>(player);
            ships->value  = 0;
            ships->op     = Level::ConditionBase::Op::LE;
            break;
        }

        case kZoomLevelCondition: {
            int32_t value;
            if (!sub.read(&value)) {
                return false;
            }
            condition->init<Level::ZoomCondition>()->value = static_cast<Zoom>(value);
            (*condition)->op                               = Level::ConditionBase::Op::EQ;
            break;
        }

        case kVelocityLessThanEqualToCondition:
            if (!read_from(sub, &condition->init<Level::SpeedCondition>()->value)) {
                return false;
            }
            (*condition)->op = Level::ConditionBase::Op::LT;
            break;

        case kTimeCondition: {
            int32_t time;
            if (!sub.read(&time)) {
                return false;
            }
            condition->init<Level::TimeCondition>()->value = ticks(time);
            (*condition)->op                               = Level::ConditionBase::Op::GE;
            break;
        }

        case kProximityCondition:
            if (!sub.read(&condition->init<Level::DistanceCondition>()->value)) {
                return false;
            }
            (*condition)->op = Level::ConditionBase::Op::LT;
            break;

        case kDistanceGreaterCondition:
            if (!sub.read(&condition->init<Level::DistanceCondition>()->value)) {
                return false;
            }
            (*condition)->op = Level::ConditionBase::Op::GE;
            break;

        case kCurrentMessageCondition: {
            auto* message = condition->init<Level::MessageCondition>();
            if (!sub.read(&message->start, &message->page)) {
                return false;
            }
            (*condition)->op = Level::ConditionBase::Op::EQ;
            break;
        }

        case kCurrentComputerCondition: {
            auto* computer = condition->init<Level::ComputerCondition>();
            if (!sub.read(&computer->screen, &computer->line)) {
                return false;
            }
            (*condition)->op = Level::ConditionBase::Op::EQ;
            break;
        }
    }

    if (*condition) {
        (*condition)->subject           = Handle<Level::Initial>(subject);
        (*condition)->object            = Handle<Level::Initial>(object);
        (*condition)->action            = read_actions(action_start, action_start + action_count);
        (*condition)->persistent        = !(flags & kTrueOnlyOnce);
        (*condition)->initially_enabled = !(flags & kInitiallyTrue);
    }
    return true;
}

std::vector<Level::Condition> read_conditions(int begin, int end) {
    if (end <= begin) {
        return std::vector<Level::Condition>{};
    }

    std::vector<Level::Condition> conditions;
    conditions.resize(end - begin);
    for (int i : sfz::range(begin, end)) {
        Resource r = Resource::path(pn::format("conditions/{0}.bin", i));
        read_from(r.data().open(), &conditions[i - begin]);
    }
    return conditions;
}

bool read_from(pn::file_view in, Level::Briefing* brief_point) {
    uint8_t kind;
    int16_t title_id, title_index, content_id;
    int32_t object;
    if (!in.read(&kind, pn::pad(1), &object, pn::pad(12), &title_id, &title_index, &content_id)) {
        return false;
    }

    if (kind == 1) {
        brief_point->object = Handle<Level::Initial>(object);
    } else {
        brief_point->object = Level::Initial::none();
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

    std::vector<Level::Briefing> briefings;
    briefings.resize(end - begin);
    for (int i : sfz::range(begin, end)) {
        Resource r = Resource::path(pn::format("briefings/{0}.bin", i));
        read_from(r.data().open(), &briefings[i - begin]);
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

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
#include "data/races.hpp"
#include "data/resource.hpp"

namespace macroman = sfz::macroman;

namespace antares {

static std::vector<Level::Initial>                    optional_initial_array(path_value x);
static std::vector<std::unique_ptr<Level::Condition>> optional_condition_array(path_value x);
static std::vector<Level::Briefing>                   optional_briefing_array(path_value x);

const Level* Level::get(int number) { return plug.chapters[number]; }

const Level* Level::get(pn::string_view name) {
    auto it = plug.levels.find(name.copy());
    if (it == plug.levels.end()) {
        return nullptr;
    } else {
        return &it->second;
    }
}

static sfz::optional<int32_t> optional_int32(path_value x) {
    auto i = optional_int(x, {-0x80000000ll, 0x80000000ll});
    return (i.has_value()) ? sfz::make_optional<int32_t>(*i) : sfz::nullopt;
}

static bool valid_sha1(pn::string_view s) {
    if (s.size() != 40) {
        return false;
    }
    for (pn::rune r : s) {
        if (!(((pn::rune{'0'} <= r) && (r <= pn::rune{'9'})) ||
              ((pn::rune{'a'} <= r) && (r <= pn::rune{'f'})))) {
            return false;
        }
    }
    return true;
}

static sfz::optional<pn::string_view> optional_identifier(path_value x) {
    auto id = optional_string(x);
    if (id.has_value() && !valid_sha1(*id)) {
        throw std::runtime_error(
                pn::format("{0}invalid identifier (must be lowercase sha1 digest)", x.prefix())
                        .c_str());
    }
    return id;
}

ScenarioInfo scenario_info(pn::value_cref x0) {
    path_value x{x0};
    return required_struct<ScenarioInfo>(
            x, {{"title", {&ScenarioInfo::titleString, required_string_copy}},
                {"identifier", {&ScenarioInfo::identifier, optional_identifier, ""}},
                {"format", {&ScenarioInfo::format, required_int}},
                {"download_url", {&ScenarioInfo::downloadURLString, required_string_copy}},
                {"author", {&ScenarioInfo::authorNameString, required_string_copy}},
                {"author_url", {&ScenarioInfo::authorURLString, required_string_copy}},
                {"version", {&ScenarioInfo::version, required_string_copy}},
                {"warp_in_flare", {&ScenarioInfo::warpInFlareID, required_base}},
                {"warp_out_flare", {&ScenarioInfo::warpOutFlareID, required_base}},
                {"player_body", {&ScenarioInfo::playerBodyID, required_base}},
                {"energy_blob", {&ScenarioInfo::energyBlobID, required_base}},
                {"intro", {&ScenarioInfo::intro, optional_string_copy}},
                {"about", {&ScenarioInfo::about, optional_string_copy}},
                {"splash", {&ScenarioInfo::splash_screen, required_string_copy}},
                {"starmap", {&ScenarioInfo::starmap, required_string_copy}}});
}

static Level::Player required_player(path_value x, LevelType level_type) {
    switch (level_type) {
        case LevelType::DEMO:
            return required_struct<Level::Player>(
                    x, {{"name", {&Level::Player::name, required_string_copy}},
                        {"race", {&Level::Player::playerRace, required_race}},
                        {"earning_power",
                         {&Level::Player::earningPower, optional_fixed, Fixed::zero()}}});
        case LevelType::SOLO:
            return required_struct<Level::Player>(
                    x, {{"type", {&Level::Player::playerType, required_player_type}},
                        {"name", {&Level::Player::name, required_string_copy}},
                        {"race", {&Level::Player::playerRace, required_race}},
                        {"hue", {&Level::Player::hue, optional_hue, Hue::GRAY}},
                        {"earning_power",
                         {&Level::Player::earningPower, optional_fixed, Fixed::zero()}}});
        case LevelType::NET:
            return required_struct<Level::Player>(
                    x, {{"type", {&Level::Player::playerType, required_player_type}},
                        {"earning_power",
                         {&Level::Player::earningPower, optional_fixed, Fixed::zero()}},
                        {"races", nullptr}});  // TODO(sfiera): populate field
    }
}

template <LevelType level_type>
static std::vector<Level::Player> required_player_array(path_value x) {
    if (x.value().is_array()) {
        std::vector<Level::Player> p;
        for (int i = 0; i < x.value().as_array().size(); ++i) {
            p.push_back(required_player(x.get(i), level_type));
        }
        return p;
    } else {
        throw std::runtime_error(pn::format("{0}: must be array", x.path()).c_str());
    }
}

static game_ticks required_game_ticks(path_value x) { return game_ticks{required_ticks(x)}; }

static Level::Par optional_par(path_value x) {
    return optional_struct<Level::Par>(
                   x,
                   {
                           {"time", {&Level::Par::time, required_game_ticks}},
                           {"kills", {&Level::Par::kills, required_int}},
                           {"losses", {&Level::Par::losses, required_int}},
                   })
            .value_or(Level::Par{game_ticks{ticks{0}}, 0, 0});
}

/*
        struct Counter {
            int  player = 0;
            int  which  = 0;
            bool fixed  = false;
        };
        */

static sfz::optional<Level::StatusLine::Counter> optional_status_line_counter(path_value x) {
    return optional_struct<Level::StatusLine::Counter>(
            x, {
                       {"player", {&Level::StatusLine::Counter::player, required_int}},
                       {"which", {&Level::StatusLine::Counter::which, required_int}},
                       {"fixed", {&Level::StatusLine::Counter::fixed, optional_bool, false}},
               });
};

static Level::StatusLine required_status_line(path_value x) {
    return required_struct<Level::StatusLine>(
            x, {
                       {"text", {&Level::StatusLine::text, optional_string_copy}},
                       {"prefix", {&Level::StatusLine::prefix, optional_string_copy}},

                       {"condition", {&Level::StatusLine::condition, optional_int}},
                       {"true", {&Level::StatusLine::true_, optional_string_copy}},
                       {"false", {&Level::StatusLine::false_, optional_string_copy}},

                       {"minuend", {&Level::StatusLine::minuend, optional_fixed}},
                       {"counter", {&Level::StatusLine::counter, optional_status_line_counter}},

                       {"suffix", {&Level::StatusLine::suffix, optional_string_copy}},
                       {"underline", {&Level::StatusLine::underline, optional_bool, false}},
               });
};

static std::vector<Level::StatusLine> optional_status_line_array(path_value x) {
    if (x.value().is_null()) {
        return {};
    } else if (x.value().is_array()) {
        pn::array_cref                 a = x.value().as_array();
        std::vector<Level::StatusLine> result;
        for (int i = 0; i < a.size(); ++i) {
            result.emplace_back(required_status_line(x.get(i)));
        }
        return result;
    } else {
        throw std::runtime_error(pn::format("{0}: must be null or array", x.path()).c_str());
    }
}

// clang-format off
#define COMMON_LEVEL_FIELDS                                                                      \
            {"type", {&Level::type, required_level_type}},                                       \
            {"chapter", {&Level::chapter, optional_int}},                                        \
            {"title", {&Level::name, required_string_copy}},                                     \
            {"initials", {&Level::initials, optional_initial_array}},                            \
            {"conditions", {&Level::conditions, optional_condition_array}},                      \
            {"briefings", {&Level::briefings, optional_briefing_array}},                         \
            {"starmap", {&Level::starmap, optional_rect}},                                       \
            {"song", {&Level::song, optional_string_copy}},                                      \
            {"status", {&Level::status, optional_status_line_array}},                            \
            {"start_time", {&Level::startTime, optional_secs, secs(0)}},                         \
            {"skip", {&Level::skip, optional_level}},                                            \
            {"angle", {&Level::angle, optional_int32, -1}},                                      \
            {"par", {&Level::par, optional_par}}
// clang-format on

Level demo_level(path_value x) {
    return required_struct<Level>(
            x, {
                       COMMON_LEVEL_FIELDS,
                       {"players", {&Level::players, required_player_array<LevelType::DEMO>}},
               });
}

Level solo_level(path_value x) {
    return required_struct<Level>(
            x, {
                       COMMON_LEVEL_FIELDS,
                       {"players", {&Level::players, required_player_array<LevelType::SOLO>}},
                       {"no_ships", {&Level::own_no_ships_text, optional_string, ""}},
                       {"prologue", {&Level::prologue, optional_string, ""}},
                       {"epilogue", {&Level::epilogue, optional_string, ""}},
               });
}

Level net_level(path_value x) {
    return required_struct<Level>(
            x, {
                       COMMON_LEVEL_FIELDS,
                       {"players", {&Level::players, required_player_array<LevelType::NET>}},
                       {"own_no_ships", {&Level::own_no_ships_text, optional_string, ""}},
                       {"foe_no_ships", {&Level::foe_no_ships_text, optional_string, ""}},
                       {"description", {&Level::description, optional_string, ""}},
               });
}

Level level(pn::value_cref x0) {
    path_value x{x0};
    if (!x.value().is_map()) {
        throw std::runtime_error("must be map");
    }
    switch (required_level_type(x.get("type"))) {
        case LevelType::DEMO: return demo_level(x);
        case LevelType::SOLO: return solo_level(x);
        case LevelType::NET: return net_level(x);
    }
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
    c->duration          = required_ticks(x.get("duration"));
    c->legacy_start_time = optional_bool(x.get("legacy_start_time")).value_or(false);
    return std::move(c);
}

static std::unique_ptr<Level::Condition> zoom_condition(path_value x) {
    std::unique_ptr<Level::ZoomCondition> c(new Level::ZoomCondition);
    c->value = required_zoom(x.get("value"));
    return std::move(c);
}

static std::unique_ptr<Level::Condition> condition(path_value x) {
    if (!x.value().is_map()) {
        throw std::runtime_error(pn::format("{0}: must be map", x.path()).c_str());
    }

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

static std::vector<std::unique_ptr<Level::Condition>> optional_condition_array(path_value x) {
    if (x.value().is_null()) {
        return {};
    } else if (x.value().is_array()) {
        std::vector<std::unique_ptr<Level::Condition>> v;
        for (int i = 0; i < x.value().as_array().size(); ++i) {
            v.push_back(condition(x.get(i)));
        }
        return v;
    } else {
        throw std::runtime_error(pn::format("{0}: must be null or array", x.path()).c_str());
    }
}

static Level::Briefing briefing(path_value x) {
    return required_struct<Level::Briefing>(
            x, {
                       {"object",
                        {&Level::Briefing::object, optional_initial, Level::Initial::none()}},
                       {"title", {&Level::Briefing::title, required_string_copy}},
                       {"content", {&Level::Briefing::content, required_string_copy}},
               });
}

static std::vector<Level::Briefing> optional_briefing_array(path_value x) {
    if (x.value().is_null()) {
        return {};
    } else if (x.value().is_array()) {
        std::vector<Level::Briefing> v;
        for (int i = 0; i < x.value().as_array().size(); ++i) {
            v.push_back(briefing(x.get(i)));
        }
        return v;
    } else {
        throw std::runtime_error(pn::format("{0}: must be null or array", x.path()).c_str());
    }
}

static BuildableObject required_buildable_object(path_value x) {
    return BuildableObject{required_string_copy(x)};
}

std::vector<BuildableObject> optional_buildable_object_array(path_value x) {
    if (x.value().is_null()) {
        return {};
    } else if (x.value().is_array()) {
        pn::array_cref a = x.value().as_array();
        if (a.size() > kMaxShipCanBuild) {
            throw std::runtime_error(pn::format(
                                             "{0}has {1} elements, more than max of {2}",
                                             x.prefix(), a.size(), kMaxShipCanBuild)
                                             .c_str());
        }
        std::vector<BuildableObject> result;
        for (int i = 0; i < a.size(); ++i) {
            result.emplace_back(required_buildable_object(x.get(i)));
        }
        return result;
    } else {
        throw std::runtime_error(pn::format("{0}: must be null or array", x.path()).c_str());
    }
}

Level::Initial::Override optional_override(path_value x) {
    return optional_struct<Level::Initial::Override>(
                   x, {{"name", {&Level::Initial::Override::name, optional_string_copy}},
                       {"sprite", {&Level::Initial::Override::sprite, optional_string_copy}}})
            .value_or(Level::Initial::Override{});
}

Level::Initial::Target optional_target(path_value x) {
    return optional_struct<Level::Initial::Target>(
                   x,
                   {{"initial",
                     {&Level::Initial::Target::initial, optional_initial, Level::Initial::none()}},
                    {"lock", {&Level::Initial::Target::lock, optional_bool, false}}})
            .value_or(Level::Initial::Target{});
}

static Level::Initial initial(path_value x) {
    return required_struct<Level::Initial>(
            x, {{"base", {&Level::Initial::base, required_buildable_object}},
                {"owner", {&Level::Initial::owner, optional_admiral, Handle<Admiral>(-1)}},
                {"at", {&Level::Initial::at, required_point}},
                {"earning", {&Level::Initial::earning, optional_fixed, Fixed::zero()}},
                {"hide", {&Level::Initial::hide, optional_bool, false}},
                {"flagship", {&Level::Initial::flagship, optional_bool, false}},
                {"override", {&Level::Initial::override_, optional_override}},
                {"target", {&Level::Initial::target, optional_target}},
                {"build", {&Level::Initial::build, optional_buildable_object_array}}});
}

static std::vector<Level::Initial> optional_initial_array(path_value x) {
    if (x.value().is_null()) {
        return {};
    } else if (x.value().is_array()) {
        std::vector<Level::Initial> v;
        for (int i = 0; i < x.value().as_array().size(); ++i) {
            v.push_back(initial(x.get(i)));
        }
        return v;
    } else {
        throw std::runtime_error(pn::format("{0}: must be null or array", x.path()).c_str());
    }
}

}  // namespace antares

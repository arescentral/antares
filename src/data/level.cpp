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

#include "data/briefing.hpp"
#include "data/condition.hpp"
#include "data/field.hpp"
#include "data/initial.hpp"
#include "data/plugin.hpp"
#include "data/races.hpp"
#include "data/resource.hpp"

namespace macroman = sfz::macroman;

namespace antares {

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

template <LevelType T>
static Level::Player required_player(path_value x);

template <>
Level::Player required_player<LevelType::DEMO>(path_value x) {
    return required_struct<Level::Player>(
            x, {{"name", {&Level::Player::name, required_string_copy}},
                {"race", {&Level::Player::playerRace, required_race}},
                {"earning_power", {&Level::Player::earningPower, optional_fixed, Fixed::zero()}}});
}

template <>
Level::Player required_player<LevelType::SOLO>(path_value x) {
    return required_struct<Level::Player>(
            x, {{"type", {&Level::Player::playerType, required_player_type}},
                {"name", {&Level::Player::name, required_string_copy}},
                {"race", {&Level::Player::playerRace, required_race}},
                {"hue", {&Level::Player::hue, optional_hue, Hue::GRAY}},
                {"earning_power", {&Level::Player::earningPower, optional_fixed, Fixed::zero()}}});
}

template <>
Level::Player required_player<LevelType::NET>(path_value x) {
    return required_struct<Level::Player>(
            x, {{"type", {&Level::Player::playerType, required_player_type}},
                {"earning_power", {&Level::Player::earningPower, optional_fixed, Fixed::zero()}},
                {"races", nullptr}});  // TODO(sfiera): populate field
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

// clang-format off
#define COMMON_LEVEL_FIELDS                                                                      \
            {"type", {&Level::type, required_level_type}},                                       \
            {"chapter", {&Level::chapter, optional_int}},                                        \
            {"title", {&Level::name, required_string_copy}},                                     \
            {"initials", {&Level::initials, optional_array<Initial, initial>}},                  \
            {"conditions",                                                                       \
             {&Level::conditions, optional_array<std::unique_ptr<Condition>, condition>}},       \
            {"briefings", {&Level::briefings, optional_array<Briefing, briefing>}},              \
            {"starmap", {&Level::starmap, optional_rect}},                                       \
            {"song", {&Level::song, optional_string_copy}},                                      \
            {"status",                                                                           \
             {&Level::status, optional_array<Level::StatusLine, required_status_line>}},         \
            {"start_time", {&Level::startTime, optional_secs, secs(0)}},                         \
            {"skip", {&Level::skip, optional_level}},                                            \
            {"angle", {&Level::angle, optional_int32, -1}},                                      \
            {"par", {&Level::par, optional_par}}
// clang-format on

Level demo_level(path_value x) {
    return required_struct<Level>(
            x, {
                       COMMON_LEVEL_FIELDS,
                       {"players",
                        {&Level::players,
                         required_array<Level::Player, required_player<LevelType::DEMO>>}},
               });
}

Level solo_level(path_value x) {
    return required_struct<Level>(
            x, {
                       COMMON_LEVEL_FIELDS,
                       {"players",
                        {&Level::players,
                         required_array<Level::Player, required_player<LevelType::SOLO>>}},
                       {"no_ships", {&Level::own_no_ships_text, optional_string, ""}},
                       {"prologue", {&Level::prologue, optional_string, ""}},
                       {"epilogue", {&Level::epilogue, optional_string, ""}},
               });
}

Level net_level(path_value x) {
    return required_struct<Level>(
            x, {
                       COMMON_LEVEL_FIELDS,
                       {"players",
                        {&Level::players,
                         required_array<Level::Player, required_player<LevelType::NET>>}},
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

}  // namespace antares

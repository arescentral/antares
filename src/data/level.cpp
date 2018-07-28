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

static Level::Player::Type required_player_type(path_value x) {
    return required_enum<Level::Player::Type>(
            x, {{"human", Level::Player::Type::HUMAN}, {"cpu", Level::Player::Type::CPU}});
}
DEFAULT_READER(Level::Player::Type, required_player_type);

Level::DemoPlayer required_demo_player(path_value x) {
    return required_struct<Level::DemoPlayer>(
            x, {{"name", &Level::Player::name},
                {"race", &Level::Player::playerRace},
                {"earning_power", &Level::Player::earningPower}});
}
DEFAULT_READER(Level::DemoPlayer, required_demo_player);

Level::SoloPlayer required_solo_player(path_value x) {
    return required_struct<Level::SoloPlayer>(
            x, {{"type", &Level::Player::playerType},
                {"name", &Level::Player::name},
                {"race", &Level::Player::playerRace},
                {"hue", &Level::Player::hue},
                {"earning_power", &Level::Player::earningPower}});
}
DEFAULT_READER(Level::SoloPlayer, required_solo_player);

Level::NetPlayer required_net_player(path_value x) {
    return required_struct<Level::NetPlayer>(
            x, {{"type", &Level::Player::playerType},
                {"earning_power", &Level::Player::earningPower},
                {"races", nullptr}});  // TODO(sfiera): populate field
}
DEFAULT_READER(Level::NetPlayer, required_net_player);

static game_ticks required_game_ticks(path_value x) { return game_ticks{required_ticks(x)}; }
DEFAULT_READER(game_ticks, required_game_ticks);

static Level::Par optional_par(path_value x) {
    return optional_struct<Level::Par>(
                   x,
                   {
                           {"time", &Level::Par::time},
                           {"kills", &Level::Par::kills},
                           {"losses", &Level::Par::losses},
                   })
            .value_or(Level::Par{game_ticks{ticks{0}}, 0, 0});
}
DEFAULT_READER(Level::Par, optional_par);

static sfz::optional<Level::StatusLine::Counter> optional_status_line_counter(path_value x) {
    return optional_struct<Level::StatusLine::Counter>(
            x, {{"player", &Level::StatusLine::Counter::player},
                {"which", &Level::StatusLine::Counter::which},
                {"fixed", &Level::StatusLine::Counter::fixed}});
};
DEFAULT_READER(sfz::optional<Level::StatusLine::Counter>, optional_status_line_counter);

static Level::StatusLine required_status_line(path_value x) {
    return required_struct<Level::StatusLine>(
            x, {
                       {"text", &Level::StatusLine::text},
                       {"prefix", &Level::StatusLine::prefix},

                       {"condition", &Level::StatusLine::condition},
                       {"true", &Level::StatusLine::true_},
                       {"false", &Level::StatusLine::false_},

                       {"minuend", &Level::StatusLine::minuend},
                       {"counter", &Level::StatusLine::counter},

                       {"suffix", &Level::StatusLine::suffix},
                       {"underline", &Level::StatusLine::underline},
               });
};
DEFAULT_READER(Level::StatusLine, required_status_line);

// clang-format off
#define COMMON_LEVEL_FIELDS                                                                      \
            {"type", &Level::type},                                                              \
            {"chapter", &Level::chapter},                                                        \
            {"title", &Level::name},                                                             \
            {"initials", &Level::initials},                                                      \
            {"conditions", &Level::conditions},                                                  \
            {"briefings", &Level::briefings},                                                    \
            {"starmap", &Level::starmap},                                                        \
            {"song", &Level::song},                                                              \
            {"status", &Level::status},                                                          \
            {"start_time", &Level::start_time},                                                  \
            {"skip", &Level::skip},                                                              \
            {"angle", &Level::angle},                                                            \
            {"par", &Level::par}
// clang-format on

static Level::Type required_level_type(path_value x) {
    return required_enum<Level::Type>(
            x,
            {{"solo", Level::Type::SOLO}, {"net", Level::Type::NET}, {"demo", Level::Type::DEMO}});
}
DEFAULT_READER(Level::Type, required_level_type);

static Level demo_level(path_value x) {
    return required_struct<Level>(x, {COMMON_LEVEL_FIELDS, {"players", &Level::demo_players}});
}

static Level solo_level(path_value x) {
    return required_struct<Level>(
            x, {COMMON_LEVEL_FIELDS,
                {"players", &Level::solo_players},
                {"no_ships", &Level::own_no_ships_text},
                {"prologue", &Level::prologue},
                {"epilogue", &Level::epilogue}});
}

static Level net_level(path_value x) {
    return required_struct<Level>(
            x, {COMMON_LEVEL_FIELDS,
                {"players", &Level::net_players},
                {"own_no_ships", &Level::own_no_ships_text},
                {"foe_no_ships", &Level::foe_no_ships_text},
                {"description", &Level::description}});
}

Level level(pn::value_cref x0) {
    path_value x{x0};
    switch (required_object_type(x, required_level_type)) {
        case Level::Type::DEMO: return demo_level(x);
        case Level::Type::SOLO: return solo_level(x);
        case Level::Type::NET: return net_level(x);
    }
}

}  // namespace antares

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

Level::Type Level::type() const { return base.type; }

Level::Level() : base{} {}
Level::Level(DemoLevel l) : demo(std::move(l)) {}
Level::Level(SoloLevel l) : solo(std::move(l)) {}
Level::Level(NetLevel l) : net(std::move(l)) {}

Level::Level(Level&& l) {
    switch (l.type()) {
        case Level::Type::NONE: new (this) Level(); break;
        case Level::Type::DEMO: new (this) Level(std::move(l.demo)); break;
        case Level::Type::SOLO: new (this) Level(std::move(l.solo)); break;
        case Level::Type::NET: new (this) Level(std::move(l.net)); break;
    }
}

Level& Level::operator=(Level&& l) {
    this->~Level();
    new (this) Level(std::move(l));
    return *this;
}

Level::~Level() {
    switch (type()) {
        case Level::Type::NONE: base.~LevelBase(); break;
        case Level::Type::DEMO: demo.~DemoLevel(); break;
        case Level::Type::SOLO: solo.~SoloLevel(); break;
        case Level::Type::NET: net.~NetLevel(); break;
    }
}

const Level* Level::get(int number) { return plug.chapters[number]; }

const Level* Level::get(pn::string_view name) {
    auto it = plug.levels.find(name.copy());
    if (it == plug.levels.end()) {
        return nullptr;
    } else {
        return &it->second;
    }
}

FIELD_READER(LevelBase::PlayerType) {
    return required_enum<LevelBase::PlayerType>(
            x, {{"human", LevelBase::PlayerType::HUMAN}, {"cpu", LevelBase::PlayerType::CPU}});
}

FIELD_READER(DemoLevel::Player) {
    return required_struct<DemoLevel::Player>(
            x, {{"name", &DemoLevel::Player::name},
                {"race", &DemoLevel::Player::race},
                {"earning_power", &DemoLevel::Player::earning_power}});
}

FIELD_READER(SoloLevel::Player) {
    return required_struct<SoloLevel::Player>(
            x, {{"type", &SoloLevel::Player::type},
                {"name", &SoloLevel::Player::name},
                {"race", &SoloLevel::Player::race},
                {"hue", &SoloLevel::Player::hue},
                {"earning_power", &SoloLevel::Player::earning_power}});
}

FIELD_READER(NetLevel::Player) {
    return required_struct<NetLevel::Player>(
            x, {{"type", &NetLevel::Player::type},
                {"earning_power", &NetLevel::Player::earning_power},
                {"races", nullptr}});  // TODO(sfiera): populate field
}

FIELD_READER(game_ticks) { return game_ticks{read_field<ticks>(x)}; }

FIELD_READER(SoloLevel::Par) {
    return optional_struct<SoloLevel::Par>(
                   x, {{"time", &SoloLevel::Par::time},
                       {"kills", &SoloLevel::Par::kills},
                       {"losses", &SoloLevel::Par::losses}})
            .value_or(SoloLevel::Par{game_ticks{ticks{0}}, 0, 0});
}

FIELD_READER(LevelBase::StatusLine) {
    return required_struct<LevelBase::StatusLine>(
            x, {{"text", &LevelBase::StatusLine::text},
                {"prefix", &LevelBase::StatusLine::prefix},

                {"condition", &LevelBase::StatusLine::condition},
                {"true", &LevelBase::StatusLine::true_},
                {"false", &LevelBase::StatusLine::false_},

                {"minuend", &LevelBase::StatusLine::minuend},
                {"counter", &LevelBase::StatusLine::counter},

                {"suffix", &LevelBase::StatusLine::suffix},
                {"underline", &LevelBase::StatusLine::underline}});
};

// clang-format off
#define COMMON_LEVEL_FIELDS                                                                      \
            {"type", &LevelBase::type},                                                          \
            {"chapter", &LevelBase::chapter},                                                    \
            {"title", &LevelBase::name},                                                         \
            {"initials", &LevelBase::initials},                                                  \
            {"conditions", &LevelBase::conditions},                                              \
            {"briefings", &LevelBase::briefings},                                                \
            {"starmap", &LevelBase::starmap},                                                    \
            {"song", &LevelBase::song},                                                          \
            {"status", &LevelBase::status},                                                      \
            {"start_time", &LevelBase::start_time},                                              \
            {"angle", &LevelBase::angle}
// clang-format on

FIELD_READER(LevelBase::Type) {
    return required_enum<LevelBase::Type>(
            x, {{"solo", LevelBase::Type::SOLO},
                {"net", LevelBase::Type::NET},
                {"demo", LevelBase::Type::DEMO}});
}

static Level demo_level(path_value x) {
    return required_struct<DemoLevel>(x, {COMMON_LEVEL_FIELDS, {"players", &DemoLevel::players}});
}

static Level solo_level(path_value x) {
    return required_struct<SoloLevel>(
            x, {COMMON_LEVEL_FIELDS,
                {"players", &SoloLevel::players},
                {"par", &SoloLevel::par},
                {"skip", &SoloLevel::skip},
                {"no_ships", &SoloLevel::no_ships},
                {"prologue", &SoloLevel::prologue},
                {"epilogue", &SoloLevel::epilogue}});
}

static Level net_level(path_value x) {
    return required_struct<NetLevel>(
            x, {COMMON_LEVEL_FIELDS,
                {"players", &NetLevel::players},
                {"description", &NetLevel::description},
                {"own_no_ships", &NetLevel::own_no_ships},
                {"foe_no_ships", &NetLevel::foe_no_ships}});
}

Level level(pn::value_cref x0) {
    path_value x{x0};
    switch (required_object_type(x, read_field<Level::Type>)) {
        case Level::Type::NONE: throw std::runtime_error("level type NONE?");
        case Level::Type::DEMO: return demo_level(x);
        case Level::Type::SOLO: return solo_level(x);
        case Level::Type::NET: return net_level(x);
    }
}

}  // namespace antares

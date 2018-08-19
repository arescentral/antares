// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// Copyright (C) 2016-2017 The Antares Authors
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

#include "game/condition.hpp"

#include "data/condition.hpp"
#include "data/plugin.hpp"
#include "game/action.hpp"
#include "game/admiral.hpp"
#include "game/globals.hpp"
#include "game/initial.hpp"
#include "game/level.hpp"
#include "game/messages.hpp"
#include "game/player-ship.hpp"
#include "game/space-object.hpp"
#include "math/macros.hpp"

namespace antares {

static bool is_true(const ConditionWhen& c);

const Condition* Condition::get(int number) {
    if ((0 <= number) && (number < g.level->base.conditions.size())) {
        return &g.level->base.conditions[number];
    }
    return nullptr;
}

HandleList<const Condition> Condition::all() {
    return HandleList<const Condition>(0, g.level->base.conditions.size());
}

template <typename X, typename Y>
static bool op_compare(ConditionOp op, const X& x, const Y& y) {
    switch (op) {
        case ConditionOp::EQ: return (x == y);
        case ConditionOp::NE: return (x != y);
        case ConditionOp::LT: return (x < y);
        case ConditionOp::GT: return (x > y);
        case ConditionOp::LE: return (x <= y);
        case ConditionOp::GE: return (x >= y);
    }
}

template <typename X, typename Y>
static bool op_eq(ConditionEqOp op, const X& x, const Y& y) {
    switch (op) {
        case ConditionEqOp::EQ: return (x == y);
        case ConditionEqOp::NE: return (x != y);
    }
}

static bool is_true(const AutopilotCondition& c) {
    if (!c.player.get()) {
        return false;
    }
    auto flagship = c.player->flagship();
    if (!flagship.get()) {
        return false;
    }
    bool on_autopilot = flagship->attributes & kOnAutoPilot;
    return op_eq(c.op, on_autopilot, c.value);
}

static bool is_true(const BuildingCondition& c) {
    if (!c.player.get()) {
        return false;
    }
    auto build_object = GetAdmiralBuildAtObject(c.player);
    if (!build_object.get()) {
        return false;
    }
    return op_eq(c.op, build_object->totalBuildTime > ticks(0), c.value);
}

static bool is_true(const CashCondition& c) { return op_compare(c.op, c.player->cash(), c.value); }

static bool is_true(const ComputerCondition& c) {
    if (c.line.has_value()) {
        return op_eq(
                c.op, std::pair<Screen, int>(g.mini.currentScreen, g.mini.selectLine),
                std::pair<Screen, int>(c.screen, *c.line));
    } else {
        return op_eq(c.op, g.mini.currentScreen, c.screen);
    }
}

static bool is_true(const CountCondition& c) {
    int count = 0;
    for (const ConditionWhen& sub : c.of) {
        if (is_true(sub)) {
            ++count;
        }
    }
    return op_compare(c.op, count, c.value);
}

static bool is_true(const DestroyedCondition& c) {
    auto sObject = resolve_object_ref(c.object);
    return op_eq(c.op, !sObject.get(), c.value);
}

static bool is_true(const DistanceCondition& c) {
    auto sObject = resolve_object_ref(c.from);
    auto dObject = resolve_object_ref(c.to);
    if (sObject.get() && dObject.get()) {
        int64_t xdist = ABS<int>(sObject->location.h - dObject->location.h);
        int64_t ydist = ABS<int>(sObject->location.v - dObject->location.v);
        return op_compare(c.op, (ydist * ydist) + (xdist * xdist), c.value.squared);
    }
    return false;
}

static bool is_true(const HealthCondition& c) {
    auto   sObject = resolve_object_ref(c.object);
    double health  = 0.0;
    if (sObject.get()) {
        health = sObject->health();
        health /= sObject->max_health();
    }
    return op_compare(c.op, health, c.value);
}

static bool is_true(const MessageCondition& c) {
    auto current = Messages::current();
    if (!current.first.has_value()) {
        return false;
    }
    return op_eq(
            c.op, std::pair<int64_t, int64_t>(*current.first, current.second),
            std::make_pair(c.id, c.page - 1));
}

static bool is_true(const IdentityCondition& c) {
    auto a = resolve_object_ref(c.a);
    auto b = resolve_object_ref(c.b);
    if (!(a.get() && b.get())) {
        return false;
    }
    return op_eq(c.op, a, b);
}

static bool is_true(const OwnerCondition& c) {
    auto sObject = resolve_object_ref(c.object);
    return sObject.get() && op_eq(c.op, c.player, sObject->owner);
}

static bool is_true(const ScoreCondition& c) {
    return op_compare(c.op, GetAdmiralScore(c.counter), c.value);
}

static bool is_true(const ShipsCondition& c) {
    return op_compare(c.op, GetAdmiralShipsLeft(c.player), c.value);
}

static bool is_true(const SpeedCondition& c) {
    auto sObject = resolve_object_ref(c.object);
    return sObject.get() &&
           op_compare(c.op, std::max(ABS(sObject->velocity.h), ABS(sObject->velocity.v)), c.value);
}

static bool is_true(const TargetCondition& c) {
    auto sObject = resolve_object_ref(c.object);
    auto dObject = resolve_object_ref(c.target);
    return sObject.get() && dObject.get() &&
           op_eq(c.op, std::make_pair(sObject->destObject, sObject->destObjectID),
                 std::make_pair(dObject, dObject->id));
}

static bool is_true(const TimeCondition& c) {
    game_ticks t = game_ticks{c.duration};
    if (c.legacy_start_time.value_or(false)) {
        // Tricky: the original code for handling startTime counted g.time in major ticks,
        // but new code uses minor ticks, as game/main.cpp does. So, time before the epoch
        // (game start) counts as 1/3 towards time conditions to preserve old behavior.
        if ((3 * c.duration) < g.level->base.start_time.value_or(secs(0))) {
            t = game_ticks{(3 * c.duration) - g.level->base.start_time.value_or(secs(0))};
        } else {
            t = game_ticks{c.duration - (g.level->base.start_time.value_or(secs(0)) / 3)};
        }
    }
    return op_compare(c.op, g.time, t);
}

static bool is_true(const ZoomCondition& c) { return op_compare(c.op, g.zoom, c.value); }

static bool is_true(const ConditionWhen& c) {
    switch (c.type()) {
        case ConditionWhen::Type::NONE: return false;
        case ConditionWhen::Type::AUTOPILOT: return is_true(c.autopilot);
        case ConditionWhen::Type::BUILDING: return is_true(c.building);
        case ConditionWhen::Type::CASH: return is_true(c.cash);
        case ConditionWhen::Type::COMPUTER: return is_true(c.computer);
        case ConditionWhen::Type::COUNT: return is_true(c.count);
        case ConditionWhen::Type::DESTROYED: return is_true(c.destroyed);
        case ConditionWhen::Type::DISTANCE: return is_true(c.distance);
        case ConditionWhen::Type::HEALTH: return is_true(c.health);
        case ConditionWhen::Type::IDENTITY: return is_true(c.identity);
        case ConditionWhen::Type::MESSAGE: return is_true(c.message);
        case ConditionWhen::Type::OWNER: return is_true(c.owner);
        case ConditionWhen::Type::SCORE: return is_true(c.score);
        case ConditionWhen::Type::SHIPS: return is_true(c.ships);
        case ConditionWhen::Type::SPEED: return is_true(c.speed);
        case ConditionWhen::Type::TARGET: return is_true(c.target);
        case ConditionWhen::Type::TIME: return is_true(c.time);
        case ConditionWhen::Type::ZOOM: return is_true(c.zoom);
    }
}

void CheckLevelConditions() {
    for (auto& c : g.level->base.conditions) {
        int index = (&c - g.level->base.conditions.data());
        if (g.condition_enabled[index] && is_true(c.when)) {
            if (!c.persistent.value_or(false)) {
                g.condition_enabled[index] = false;
            }
            auto subject = resolve_object_ref(c.subject);
            auto direct  = resolve_object_ref(c.direct);
            exec(c.action, subject, direct, {0, 0});
        }
    }
}

}  // namespace antares

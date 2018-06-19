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

const Condition* Condition::get(int number) {
    if ((0 <= number) && (number < g.level->conditions.size())) {
        return &g.level->conditions[number];
    }
    return nullptr;
}

HandleList<const Condition> Condition::all() {
    return HandleList<const Condition>(0, g.level->conditions.size());
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
static bool op_eq(ConditionOp op, const X& x, const Y& y) {
    switch (op) {
        case ConditionOp::EQ: return (x == y);
        case ConditionOp::NE: return (x != y);
        default: return false;
    }
}

static bool is_true(const AutopilotCondition& c) {
    return op_eq(c.op, IsPlayerShipOnAutoPilot(), c.value);
}

static bool is_true(const BuildingCondition& c) {
    auto buildAtObject = GetAdmiralBuildAtObject(g.admiral);
    return buildAtObject.get() && op_eq(c.op, buildAtObject->totalBuildTime > ticks(0), c.value);
}

static bool is_true(const ComputerCondition& c) {
    if (c.line.has_value()) {
        return op_eq(
                c.op, std::pair<Screen, int>(g.mini.currentScreen, g.mini.selectLine),
                std::pair<Screen, int>(c.screen, *c.line));
    } else {
        return op_eq(c.op, g.mini.currentScreen, c.screen);
    }
}

static bool is_true(const CounterCondition& c) {
    return op_compare(c.op, GetAdmiralScore(c.player, c.counter), c.value);
}

static bool is_true(const DestroyedCondition& c) {
    auto sObject = GetObjectFromInitialNumber(c.initial);
    return op_eq(c.op, !sObject.get(), c.value);
}

static bool is_true(const DistanceCondition& c) {
    auto sObject = GetObjectFromInitialNumber(c.subject);
    auto dObject = GetObjectFromInitialNumber(c.object);
    if (sObject.get() && dObject.get()) {
        uint32_t xdist = ABS<int>(sObject->location.h - dObject->location.h);
        uint32_t ydist = ABS<int>(sObject->location.v - dObject->location.v);

        if ((xdist < kMaximumRelevantDistance) && (ydist < kMaximumRelevantDistance)) {
            return op_compare(c.op, (ydist * ydist) + (xdist * xdist), c.value);
        }
    }
    return false;
}

static bool is_true(const HealthCondition& c) {
    auto   sObject = GetObjectFromInitialNumber(c.subject);
    double health  = 0.0;
    if (sObject.get()) {
        health = sObject->health();
        health /= sObject->max_health();
    }
    return op_compare(c.op, health, c.value);
}

static bool is_true(const MessageCondition& c) {
    return op_eq(c.op, Messages::current(), c.id + c.page - 1);
}

static bool is_true(const OrderedCondition& c) {
    auto sObject = GetObjectFromInitialNumber(c.subject);
    auto dObject = GetObjectFromInitialNumber(c.object);
    return sObject.get() && dObject.get() &&
           op_eq(c.op, std::make_pair(sObject->destObject, sObject->destObjectID),
                 std::make_pair(dObject, dObject->id));
}

static bool is_true(const OwnerCondition& c) {
    auto sObject = GetObjectFromInitialNumber(c.subject);
    return sObject.get() && op_eq(c.op, c.player, sObject->owner);
}

static bool is_true(const ShipsCondition& c) {
    return op_compare(c.op, GetAdmiralShipsLeft(c.player), c.value);
}

static bool is_true(const SpeedCondition& c) {
    auto sObject = GetObjectFromInitialNumber(c.subject);
    return sObject.get() &&
           op_compare(c.op, std::max(ABS(sObject->velocity.h), ABS(sObject->velocity.v)), c.value);
}

static bool is_true(const SubjectCondition& c) {
    auto sObject = GetObjectFromInitialNumber(c.subject);
    switch (c.value) {
        case SubjectCondition::Value::CONTROL:
            return sObject.get() && op_eq(c.op, sObject, g.admiral->control());
        case SubjectCondition::Value::TARGET:
            return sObject.get() && op_eq(c.op, sObject, g.admiral->target());
        case SubjectCondition::Value::PLAYER: return sObject.get() && op_eq(c.op, sObject, g.ship);
    }
}

static bool is_true(const TimeCondition& c) {
    game_ticks t = game_ticks{c.duration};
    if (c.legacy_start_time) {
        // Tricky: the original code for handling startTime counted g.time in major ticks,
        // but new code uses minor ticks, as game/main.cpp does. So, time before the epoch
        // (game start) counts as 1/3 towards time conditions to preserve old behavior.
        if ((3 * c.duration) < g.level->startTime) {
            t = game_ticks{(3 * c.duration) - g.level->startTime};
        } else {
            t = game_ticks{c.duration - (g.level->startTime / 3)};
        }
    }
    return op_compare(c.op, g.time, t);
}

static bool is_true(const ZoomCondition& c) { return op_compare(c.op, g.zoom, c.value); }

static bool is_true(const Condition& c) {
    switch (c.type()) {
        case Condition::Type::AUTOPILOT: return is_true(c.autopilot);
        case Condition::Type::BUILDING: return is_true(c.building);
        case Condition::Type::COMPUTER: return is_true(c.computer);
        case Condition::Type::COUNTER: return is_true(c.counter);
        case Condition::Type::DESTROYED: return is_true(c.destroyed);
        case Condition::Type::DISTANCE: return is_true(c.distance);
        case Condition::Type::HEALTH: return is_true(c.health);
        case Condition::Type::MESSAGE: return is_true(c.message);
        case Condition::Type::ORDERED: return is_true(c.ordered);
        case Condition::Type::OWNER: return is_true(c.owner);
        case Condition::Type::SHIPS: return is_true(c.ships);
        case Condition::Type::SPEED: return is_true(c.speed);
        case Condition::Type::SUBJECT: return is_true(c.subject);
        case Condition::Type::TIME: return is_true(c.time);
        case Condition::Type::ZOOM: return is_true(c.zoom);
    }
}

void CheckLevelConditions() {
    for (auto& c : g.level->conditions) {
        int index = (&c - g.level->conditions.data());
        if ((g.condition_enabled[index] || c.base.persistent) && is_true(c)) {
            g.condition_enabled[index] = false;
            auto  sObject              = GetObjectFromInitialNumber(c.base.subject);
            auto  dObject              = GetObjectFromInitialNumber(c.base.object);
            Point offset;
            exec(c.base.action, sObject, dObject, &offset);
        }
    }
}

}  // namespace antares

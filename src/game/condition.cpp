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
        return g.level->conditions[number].get();
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

bool AutopilotCondition::is_true() const { return op_eq(op, IsPlayerShipOnAutoPilot(), value); }

bool BuildingCondition::is_true() const {
    auto buildAtObject = GetAdmiralBuildAtObject(g.admiral);
    return buildAtObject.get() && op_eq(op, buildAtObject->totalBuildTime > ticks(0), value);
}

bool ComputerCondition::is_true() const {
    if (line < 0) {
        return op_eq(op, g.mini.currentScreen, static_cast<int>(screen));
    } else {
        return op_eq(
                op, std::make_pair(g.mini.currentScreen, g.mini.selectLine),
                std::make_pair(static_cast<int>(screen), line));
    }
}

bool CounterCondition::is_true() const {
    return op_compare(op, GetAdmiralScore(player, counter), value);
}

bool DestroyedCondition::is_true() const {
    auto sObject = GetObjectFromInitialNumber(initial);
    return op_eq(op, !sObject.get(), value);
}

bool DistanceCondition::is_true() const {
    auto sObject = GetObjectFromInitialNumber(subject);
    auto dObject = GetObjectFromInitialNumber(object);
    if (sObject.get() && dObject.get()) {
        int32_t  difference = ABS<int>(sObject->location.h - dObject->location.h);
        uint32_t dcalc      = difference;
        difference          = ABS<int>(sObject->location.v - dObject->location.v);
        uint32_t distance   = difference;

        if ((dcalc < kMaximumRelevantDistance) && (distance < kMaximumRelevantDistance)) {
            distance = distance * distance + dcalc * dcalc;
            return op_compare(op, distance, value);
        }
    }
    return false;
}

bool FalseCondition::is_true() const { return false; }

bool HealthCondition::is_true() const {
    auto   sObject = GetObjectFromInitialNumber(subject);
    double health  = 0.0;
    if (sObject.get()) {
        health = sObject->health();
        health /= sObject->max_health();
    }
    return op_compare(op, health, value);
}

bool MessageCondition::is_true() const { return op_eq(op, Messages::current(), id + page - 1); }

bool OrderedCondition::is_true() const {
    auto sObject = GetObjectFromInitialNumber(subject);
    auto dObject = GetObjectFromInitialNumber(object);
    return sObject.get() && dObject.get() &&
           op_eq(op, std::make_pair(sObject->destObject, sObject->destObjectID),
                 std::make_pair(dObject, dObject->id));
}

bool OwnerCondition::is_true() const {
    auto sObject = GetObjectFromInitialNumber(subject);
    return sObject.get() && op_eq(op, player, sObject->owner);
}

bool ShipsCondition::is_true() const { return op_compare(op, GetAdmiralShipsLeft(player), value); }

bool SpeedCondition::is_true() const {
    auto sObject = GetObjectFromInitialNumber(subject);
    return sObject.get() &&
           op_compare(op, std::max(ABS(sObject->velocity.h), ABS(sObject->velocity.v)), value);
}

bool SubjectCondition::is_true() const {
    auto sObject = GetObjectFromInitialNumber(subject);
    switch (value) {
        case SubjectValue::CONTROL:
            return sObject.get() && op_eq(op, sObject, g.admiral->control());
        case SubjectValue::TARGET: return sObject.get() && op_eq(op, sObject, g.admiral->target());
        case SubjectValue::PLAYER: return sObject.get() && op_eq(op, sObject, g.ship);
    }
}

bool TimeCondition::is_true() const {
    // Tricky: the original code for handling startTime counted g.time in major ticks,
    // but new code uses minor ticks, as game/main.cpp does. So, time before the epoch
    // (game start) counts as 1/3 towards time conditions to preserve old behavior.
    game_ticks t;
    if (!legacy_start_time) {
        t = game_ticks{duration};
    } else if ((3 * duration) < g.level->startTime) {
        t = game_ticks{(3 * duration) - g.level->startTime};
    } else {
        t = game_ticks{duration - (g.level->startTime / 3)};
    }
    return op_compare(op, g.time, t);
}

bool ZoomCondition::is_true() const { return op_compare(op, g.zoom, value); }

void CheckLevelConditions() {
    for (auto& c : g.level->conditions) {
        int index = (&c - g.level->conditions.data());
        if ((g.condition_enabled[index] || c->persistent) && c->is_true()) {
            g.condition_enabled[index] = false;
            auto  sObject              = GetObjectFromInitialNumber(c->subject);
            auto  dObject              = GetObjectFromInitialNumber(c->object);
            Point offset;
            exec(c->action, sObject, dObject, &offset);
        }
    }
}

}  // namespace antares

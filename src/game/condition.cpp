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

template <typename X, typename Y>
static bool op_compare(Level::ConditionBase::Op op, const X& x, const Y& y) {
    switch (op) {
        case Level::ConditionBase::Op::EQ: return (x == y);
        case Level::ConditionBase::Op::NE: return (x != y);
        case Level::ConditionBase::Op::LT: return (x < y);
        case Level::ConditionBase::Op::GT: return (x > y);
        case Level::ConditionBase::Op::LE: return (x <= y);
        case Level::ConditionBase::Op::GE: return (x >= y);
    }
}

template <typename X, typename Y>
static bool op_eq(Level::ConditionBase::Op op, const X& x, const Y& y) {
    switch (op) {
        case Level::ConditionBase::Op::EQ: return (x == y);
        case Level::ConditionBase::Op::NE: return (x != y);
        default: return false;
    }
}

bool Level::AutopilotCondition::is_true() const {
    return op_eq(op, IsPlayerShipOnAutoPilot(), value);
}

bool Level::BuildingCondition::is_true() const {
    auto buildAtObject = GetAdmiralBuildAtObject(g.admiral);
    return buildAtObject.get() && op_eq(op, buildAtObject->totalBuildTime > ticks(0), value);
}

bool Level::ComputerCondition::is_true() const {
    if (line < 0) {
        return op_eq(op, g.mini.currentScreen, screen);
    } else {
        return op_eq(
                op, std::make_pair(g.mini.currentScreen, g.mini.selectLine),
                std::make_pair(screen, line));
    }
}

bool Level::CounterCondition::is_true() const {
    return op_compare(op, GetAdmiralScore(player, counter), value);
}

bool Level::DestroyedCondition::is_true() const {
    auto sObject = GetObjectFromInitialNumber(initial);
    return op_eq(op, !sObject.get(), value);
}

bool Level::DistanceCondition::is_true() const {
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

bool Level::FalseCondition::is_true() const { return false; }

bool Level::HealthCondition::is_true() const {
    auto   sObject = GetObjectFromInitialNumber(subject);
    double health  = 0.0;
    if (sObject.get()) {
        health = sObject->health();
        health /= sObject->max_health();
    }
    return op_compare(op, health, value);
}

bool Level::MessageCondition::is_true() const {
    return op_eq(op, Messages::current(), start + page - 1);
}

bool Level::OrderedCondition::is_true() const {
    auto sObject = GetObjectFromInitialNumber(subject);
    auto dObject = GetObjectFromInitialNumber(object);
    return sObject.get() && dObject.get() &&
           op_eq(op, std::make_pair(sObject->destObject, sObject->destObjectID),
                 std::make_pair(dObject, dObject->id));
}

bool Level::OwnerCondition::is_true() const {
    auto sObject = GetObjectFromInitialNumber(subject);
    return sObject.get() && op_eq(op, player, sObject->owner);
}

bool Level::ShipsCondition::is_true() const {
    return op_compare(op, GetAdmiralShipsLeft(player), value);
}

bool Level::SpeedCondition::is_true() const {
    auto sObject = GetObjectFromInitialNumber(subject);
    return sObject.get() &&
           op_compare(op, std::max(ABS(sObject->velocity.h), ABS(sObject->velocity.v)), value);
}

bool Level::SubjectCondition::is_true() const {
    auto sObject = GetObjectFromInitialNumber(subject);
    switch (value) {
        case Value::CONTROL: return sObject.get() && op_eq(op, sObject, g.admiral->control());
        case Value::TARGET: return sObject.get() && op_eq(op, sObject, g.admiral->target());
        case Value::PLAYER: return sObject.get() && op_eq(op, sObject, g.ship);
    }
}

bool Level::TimeCondition::is_true() const {
    // Tricky: the original code for handling startTime counted g.time in major ticks,
    // but new code uses minor ticks, as game/main.cpp does. So, time before the epoch
    // (game start) counts as 1/3 towards time conditions to preserve old behavior.
    ticks game_time  = g.time.time_since_epoch();
    ticks start_time = g.level->startTime / 3;
    if (g.time < game_ticks()) {
        game_time /= 3;
    }
    return op_compare(op, game_time + start_time, value);
}

bool Level::ZoomCondition::is_true() const { return op_compare(op, g.zoom, value); }

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

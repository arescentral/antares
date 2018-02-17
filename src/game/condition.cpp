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

bool Level::NullCondition::is_true() const { return false; }

bool Level::LocationCondition::is_true() const {
    return false;  // not implemented
}

bool Level::CounterCondition::is_true() const {
    Handle<Admiral> a = counter.whichPlayer;
    return (GetAdmiralScore(a, counter.whichCounter) == counter.amount);
}

bool Level::ProximityCondition::is_true() const {
    auto sObject = GetObjectFromInitialNumber(subject);
    auto dObject = GetObjectFromInitialNumber(object);
    if (sObject.get() && dObject.get()) {
        int32_t  difference = ABS<int>(sObject->location.h - dObject->location.h);
        uint32_t dcalc      = difference;
        difference          = ABS<int>(sObject->location.v - dObject->location.v);
        uint32_t distance   = difference;

        if ((dcalc < kMaximumRelevantDistance) && (distance < kMaximumRelevantDistance)) {
            distance = distance * distance + dcalc * dcalc;
            if (distance < unsignedLongValue) {
                return true;
            }
        }
    }
    return false;
}

bool Level::OwnerCondition::is_true() const {
    auto sObject = GetObjectFromInitialNumber(subject);
    auto a       = Handle<Admiral>(longValue);
    return sObject.get() && (a == sObject->owner);
}

bool Level::DestructionCondition::is_true() const {
    auto sObject = GetObjectFromInitialNumber(longValue);
    return !sObject.get();
}

bool Level::AgeCondition::is_true() const {
    return false;  // not implemented
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
    return (game_time + start_time) >= timeValue;
}

bool Level::RandomCondition::is_true() const {
    return false;  // not implemented
}

bool Level::HalfHealthCondition::is_true() const {
    auto sObject = GetObjectFromInitialNumber(subject);
    return !sObject.get() || (sObject->health() <= (sObject->max_health() >> 1));
}

bool Level::IsAuxiliaryObject::is_true() const {
    auto sObject = GetObjectFromInitialNumber(subject);
    auto dObject = g.admiral->control();
    return sObject.get() && dObject.get() && (dObject == sObject);
}

bool Level::IsTargetObject::is_true() const {
    auto sObject = GetObjectFromInitialNumber(subject);
    auto dObject = g.admiral->target();
    return sObject.get() && dObject.get() && (dObject == sObject);
}

bool Level::CounterGreaterCondition::is_true() const {
    Handle<Admiral> a = counter.whichPlayer;
    return (GetAdmiralScore(a, counter.whichCounter) >= counter.amount);
}

bool Level::CounterNotCondition::is_true() const {
    Handle<Admiral> a = counter.whichPlayer;
    return (GetAdmiralScore(a, counter.whichCounter) != counter.amount);
}

bool Level::DistanceGreaterCondition::is_true() const {
    auto sObject = GetObjectFromInitialNumber(subject);
    auto dObject = GetObjectFromInitialNumber(object);
    if (sObject.get() && dObject.get()) {
        int32_t  difference = ABS<int>(sObject->location.h - dObject->location.h);
        uint32_t dcalc      = difference;
        difference          = ABS<int>(sObject->location.v - dObject->location.v);
        uint32_t distance   = difference;

        if ((dcalc < kMaximumRelevantDistance) && (distance < kMaximumRelevantDistance)) {
            distance = distance * distance + dcalc * dcalc;
            if (distance >= unsignedLongValue) {
                return true;
            }
        }
    }
    return false;
}

bool Level::VelocityLessThanEqualToCondition::is_true() const {
    auto sObject = GetObjectFromInitialNumber(subject);
    return sObject.get() && (ABS(sObject->velocity.h) < fixedValue) &&
           (ABS(sObject->velocity.v) < fixedValue);
}

bool Level::NoShipsLeftCondition::is_true() const {
    return GetAdmiralShipsLeft(Handle<Admiral>(longValue)) <= 0;
}

bool Level::CurrentMessageCondition::is_true() const {
    return Messages::current() == (location.h + location.v - 1);
}

bool Level::CurrentComputerCondition::is_true() const {
    return (g.mini.currentScreen == location.h) &&
           ((location.v < 0) || (g.mini.selectLine == location.v));
}

bool Level::ZoomLevelCondition::is_true() const { return g.zoom == longValue; }

bool Level::AutopilotCondition::is_true() const { return IsPlayerShipOnAutoPilot(); }

bool Level::NotAutopilotCondition::is_true() const { return !IsPlayerShipOnAutoPilot(); }

bool Level::ObjectIsBeingBuilt::is_true() const {
    auto buildAtObject = GetAdmiralBuildAtObject(g.admiral);
    return buildAtObject.get() && (buildAtObject->totalBuildTime > ticks(0));
}

bool Level::DirectIsSubjectTarget::is_true() const {
    auto sObject = GetObjectFromInitialNumber(subject);
    auto dObject = GetObjectFromInitialNumber(object);
    return sObject.get() && dObject.get() && (sObject->destObject == dObject) &&
           (sObject->destObjectID == dObject->id);
}

bool Level::SubjectIsPlayerCondition::is_true() const {
    auto sObject = GetObjectFromInitialNumber(subject);
    return sObject.get() && (sObject == g.ship);
}

void CheckLevelConditions() {
    for (auto& c : g.level->conditions) {
        if (c.active() && c->is_true()) {
            c->enabled    = false;
            auto  sObject = GetObjectFromInitialNumber(c->subject);
            auto  dObject = GetObjectFromInitialNumber(c->object);
            Point offset;
            exec(c->action, sObject, dObject, &offset);
        }
    }
}

}  // namespace antares

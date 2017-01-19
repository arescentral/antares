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

Level::Condition* Level::condition(size_t at) const {
    return &plug.conditions[conditionFirst + at];
}

bool Level::Condition::active() const {
    return !(flags & kTrueOnlyOnce) || !(flags & kHasBeenTrue);
}

void Level::Condition::set_true_yet(bool state) {
    if (state) {
        flags |= kHasBeenTrue;
    } else {
        flags &= ~kHasBeenTrue;
    }
}

bool Level::Condition::true_yet() const {
    return flags & kHasBeenTrue;
}

bool Level::Condition::is_true() const {
    int32_t         difference;
    Handle<Admiral> a;
    uint32_t        distance, dcalc;

    switch (condition) {
        case kCounterCondition:
            a = conditionArgument.counter.whichPlayer;
            if (GetAdmiralScore(a, conditionArgument.counter.whichCounter) ==
                conditionArgument.counter.amount) {
                return true;
            }
            break;

        case kCounterGreaterCondition:
            a = conditionArgument.counter.whichPlayer;
            if (GetAdmiralScore(a, conditionArgument.counter.whichCounter) >=
                conditionArgument.counter.amount) {
                return true;
            }
            break;

        case kCounterNotCondition:
            a = conditionArgument.counter.whichPlayer;
            if (GetAdmiralScore(a, conditionArgument.counter.whichCounter) !=
                conditionArgument.counter.amount) {
                return true;
            }
            break;

        case kDestructionCondition: {
            auto sObject = GetObjectFromInitialNumber(conditionArgument.longValue);
            return !sObject.get();
        }

        case kOwnerCondition: {
            auto sObject = GetObjectFromInitialNumber(subjectObject);
            auto a       = Handle<Admiral>(conditionArgument.longValue);
            return sObject.get() && (a == sObject->owner);
        }

        case kTimeCondition: {
            // Tricky: the original code for handling startTime counted g.time in major ticks,
            // but new code uses minor ticks, as game/main.cpp does. So, time before the epoch
            // (game start) counts as 1/3 towards time conditions to preserve old behavior.
            ticks game_time  = g.time.time_since_epoch();
            ticks start_time = g.level->startTime / 3;
            if (g.time < game_ticks()) {
                game_time /= 3;
            }
            return (game_time + start_time) >= conditionArgument.timeValue;
        }

        case kProximityCondition: {
            auto sObject = GetObjectFromInitialNumber(subjectObject);
            auto dObject = GetObjectFromInitialNumber(directObject);
            if (sObject.get() && dObject.get()) {
                difference = ABS<int>(sObject->location.h - dObject->location.h);
                dcalc      = difference;
                difference = ABS<int>(sObject->location.v - dObject->location.v);
                distance   = difference;

                if ((dcalc < kMaximumRelevantDistance) && (distance < kMaximumRelevantDistance)) {
                    distance = distance * distance + dcalc * dcalc;
                    if (distance < conditionArgument.unsignedLongValue) {
                        return true;
                    }
                }
            }
            break;
        }

        case kDistanceGreaterCondition: {
            auto sObject = GetObjectFromInitialNumber(subjectObject);
            auto dObject = GetObjectFromInitialNumber(directObject);
            if (sObject.get() && dObject.get()) {
                difference = ABS<int>(sObject->location.h - dObject->location.h);
                dcalc      = difference;
                difference = ABS<int>(sObject->location.v - dObject->location.v);
                distance   = difference;

                if ((dcalc < kMaximumRelevantDistance) && (distance < kMaximumRelevantDistance)) {
                    distance = distance * distance + dcalc * dcalc;
                    if (distance >= conditionArgument.unsignedLongValue) {
                        return true;
                    }
                }
            }
            break;
        }

        case kHalfHealthCondition: {
            auto sObject = GetObjectFromInitialNumber(subjectObject);
            return !sObject.get() || (sObject->health() <= (sObject->max_health() >> 1));
        }

        case kIsAuxiliaryObject: {
            auto sObject = GetObjectFromInitialNumber(subjectObject);
            auto dObject = g.admiral->control();
            return sObject.get() && dObject.get() && (dObject == sObject);
        }

        case kIsTargetObject: {
            auto sObject = GetObjectFromInitialNumber(subjectObject);
            auto dObject = g.admiral->target();
            return sObject.get() && dObject.get() && (dObject == sObject);
        }

        case kVelocityLessThanEqualToCondition: {
            auto sObject = GetObjectFromInitialNumber(subjectObject);
            return sObject.get() && (ABS(sObject->velocity.h) < conditionArgument.fixedValue) &&
                   (ABS(sObject->velocity.v) < conditionArgument.fixedValue);
        }

        case kNoShipsLeftCondition:
            return GetAdmiralShipsLeft(Handle<Admiral>(conditionArgument.longValue)) <= 0;

        case kCurrentMessageCondition:
            return Messages::current() ==
                   (conditionArgument.location.h + conditionArgument.location.v - 1);

        case kCurrentComputerCondition:
            return (g.mini.currentScreen == conditionArgument.location.h) &&
                   ((conditionArgument.location.v < 0) ||
                    (g.mini.selectLine == conditionArgument.location.v));

        case kZoomLevelCondition: return g.zoom == conditionArgument.longValue;

        case kAutopilotCondition: return IsPlayerShipOnAutoPilot();

        case kNotAutopilotCondition: return !IsPlayerShipOnAutoPilot();

        case kObjectIsBeingBuilt: {
            auto buildAtObject = GetAdmiralBuildAtObject(g.admiral);
            return buildAtObject.get() && (buildAtObject->totalBuildTime > ticks(0));
        }

        case kDirectIsSubjectTarget: {
            auto sObject = GetObjectFromInitialNumber(subjectObject);
            auto dObject = GetObjectFromInitialNumber(directObject);
            return sObject.get() && dObject.get() && (sObject->destObject == dObject) &&
                   (sObject->destObjectID == dObject->id);
        }

        case kSubjectIsPlayerCondition: {
            auto sObject = GetObjectFromInitialNumber(subjectObject);
            return sObject.get() && (sObject == g.ship);
        }
    }
    return false;
}

void CheckLevelConditions() {
    for (int32_t i = 0; i < g.level->conditionNum; i++) {
        auto c = g.level->condition(i);
        if (c->active() && c->is_true()) {
            c->set_true_yet(true);
            auto  sObject = GetObjectFromInitialNumber(c->subjectObject);
            auto  dObject = GetObjectFromInitialNumber(c->directObject);
            Point offset;
            exec(c->action, sObject, dObject, &offset);
        }
    }
}

}  // namespace antares

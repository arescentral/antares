// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// Copyright (C) 2008-2018 The Antares Authors
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

#ifndef ANTARES_DATA_CONDITION_HPP_
#define ANTARES_DATA_CONDITION_HPP_

#include <memory>
#include <sfz/sfz.hpp>
#include <vector>

#include "data/enums.hpp"
#include "data/handle.hpp"
#include "math/fixed.hpp"
#include "math/units.hpp"

namespace antares {

union Action;
struct Initial;
class path_value;

struct ObjectRef {
    enum class Type { INITIAL, FLAGSHIP, CONTROL, TARGET } type;
    Handle<const Initial> initial;
    Handle<Admiral>       admiral;
};

enum class ConditionType {
    AUTOPILOT,
    BUILDING,
    COMPUTER,
    COUNTER,
    DESTROYED,
    DISTANCE,
    HEALTH,
    MESSAGE,
    OBJECT,
    OWNER,
    SHIPS,
    SPEED,
    TARGET,
    TIME,
    ZOOM,
};

enum class ConditionOp { EQ, NE, LT, GT, LE, GE };

struct ConditionBase {
    ConditionType type;
    ConditionOp   op = ConditionOp::EQ;
};

// Ops: EQ, NE
// Compares player’s autopilot state (on = true; off = false) to `value`.
struct AutopilotCondition : ConditionBase {
    Handle<Admiral> player;
    bool            value;
};

// Ops: EQ, NE
// Precondition: player has a build object.
// Compares player’s build object state (building = true; not building = false) to `value`.
struct BuildingCondition : ConditionBase {
    Handle<Admiral> player;
    bool            value;
};

// Ops: EQ, NE
// Compares local player’s (screen, line), or just screen if line < 0.
//
// Warning: not net-safe.
struct ComputerCondition : ConditionBase {
    Screen                 screen;
    sfz::optional<int64_t> line;
};

// Ops: EQ, NE, LT, GT, LE, GE
// Compares given counter of given admiral to `value`.
struct CounterCondition : ConditionBase {
    Handle<Admiral> player;
    int64_t         counter;
    int64_t         value;
};

// Ops: EQ, NE
// Compares state of given object (destroyed = true; alive = false) to `value`.
//
// Note: an initially-hidden object that has not yet been unhidden is considered “destroyed”
struct DestroyedCondition : ConditionBase {
    ObjectRef what;
    bool      value;
};

// Ops: EQ, NE, LT, GT, LE, GE
// Precondition: `from` and `to` exist.
// Compares distance between `from` and `to` to `value`.
//
// TODO(sfiera): provide a definition of “distance” in this context.
struct DistanceCondition : ConditionBase {
    ObjectRef from;
    ObjectRef to;
    int64_t   value;
};

// Ops: EQ, NE, LT, GT, LE, GE
// Compares health fraction of `what` (e.g. 0.5 for half health) to `value`.
//
// Note: an initially-hidden object that has not yet been unhidden is considered “destroyed”; i.e.
// its health fraction is 0.0.
struct HealthCondition : ConditionBase {
    ObjectRef what;
    double    value;
};

// Ops: EQ, NE
// Compares (id, page) of local player’s current message to (id, page).
//
// Warning: not net-safe.
struct MessageCondition : ConditionBase {
    int64_t id;
    int64_t page;
};

// Ops: EQ, NE
// Precondition: `a` and `b` exist.
// Compares `a` to `b`.
struct ObjectCondition : ConditionBase {
    ObjectRef a, b;
};

// Ops: EQ, NE
// Precondition: `what` exists.
// Compares owner of `what` to `player`.
struct OwnerCondition : ConditionBase {
    ObjectRef       what;
    Handle<Admiral> player;
};

// Ops: EQ, NE, LT, GT, LE, GE
// Compares ship count of `player` to `value`.
struct ShipsCondition : ConditionBase {
    Handle<Admiral> player;
    int64_t         value;
};

// Ops: EQ, NE, LT, GT, LE, GE
// Precondition: `what` exists.
// Compares speed of `what` to `value`.
struct SpeedCondition : ConditionBase {
    ObjectRef what;
    Fixed     value;
};

// Ops: EQ, NE
// Precondition: `what` and `target` exist.
// Compares target of `what` to `target`.
struct TargetCondition : ConditionBase {
    ObjectRef what;
    ObjectRef target;
};

// Ops: EQ, NE, LT, GT, LE, GE
// Compares the game time to `duration`. Zero is the start of play; setup time is negative.
//
// `legacy_start_time` specifies an alternate mode where the setup time counts for only 1/3 as much
// as time after the setup finishes.
struct TimeCondition : ConditionBase {
    ticks duration;
    bool  legacy_start_time;
};

// Ops: EQ, NE, LT, GT, LE, GE
// Compares zoom level of the local player to `value`.
//
// Warning: not net-safe.
struct ZoomCondition : ConditionBase {
    Zoom value;
};

struct Condition {
    bool disabled   = false;
    bool persistent = false;

    union When {
        using Type = ConditionType;

        ConditionBase base;
        ConditionType type() const;

        AutopilotCondition autopilot;
        BuildingCondition  building;
        ComputerCondition  computer;
        CounterCondition   counter;
        DestroyedCondition destroyed;
        DistanceCondition  distance;
        HealthCondition    health;
        MessageCondition   message;
        ObjectCondition    object;
        OwnerCondition     owner;
        ShipsCondition     ships;
        SpeedCondition     speed;
        TargetCondition    target;
        TimeCondition      time;
        ZoomCondition      zoom;

        When();
        When(AutopilotCondition c);
        When(BuildingCondition c);
        When(ComputerCondition c);
        When(CounterCondition c);
        When(DestroyedCondition c);
        When(DistanceCondition c);
        When(HealthCondition c);
        When(MessageCondition c);
        When(ObjectCondition c);
        When(OwnerCondition c);
        When(ShipsCondition c);
        When(SpeedCondition c);
        When(TargetCondition c);
        When(TimeCondition c);
        When(ZoomCondition c);

        ~When();
        When(When&&);
        When& operator=(When&&);
    } when;

    sfz::optional<ObjectRef> subject;
    sfz::optional<ObjectRef> object;
    std::vector<Action>      action;

    static const Condition*            get(int n);
    static HandleList<const Condition> all();
};

Condition condition(path_value x);

}  // namespace antares

#endif  // ANTARES_DATA_CONDITION_HPP_

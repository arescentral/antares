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

enum class ConditionType {
    AUTOPILOT,
    BUILDING,
    COMPUTER,
    COUNTER,
    DESTROYED,
    DISTANCE,
    HEALTH,
    MESSAGE,
    ORDERED,
    OWNER,
    SHIPS,
    SPEED,
    SUBJECT,
    TIME,
    ZOOM,
};

enum class ConditionOp { EQ, NE, LT, GT, LE, GE };

struct ConditionBase {
    ConditionType         type;
    ConditionOp           op         = ConditionOp::EQ;
    bool                  disabled   = false;
    bool                  persistent = false;
    Handle<const Initial> subject;
    Handle<const Initial> object;
    std::vector<Action>   action;
};

// Ops: EQ, NE
// Compares player’s autopilot state (on = true; off = false) to `value`.
struct AutopilotCondition : ConditionBase {
    Handle<Admiral> player;
    bool            value;
};

// Ops: EQ, NE
// Precondition: local player has a build object.
// Compares local player’s build object state (building = true; not building = false) to `value`.
//
// Warning: not net-safe.
struct BuildingCondition : ConditionBase {
    bool value;
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
// Compares state of given initial (destroyed = true; alive = false) to `value`.
//
// Note: the initial object referenced here can be (and usually is) different from either `subject`
// or `object`.
// Note: an initially-hidden object that has not yet been unhidden is considered “destroyed”
struct DestroyedCondition : ConditionBase {
    Handle<const Initial> initial;
    bool                  value;
};

// Ops: EQ, NE, LT, GT, LE, GE
// Precondition: `subject` and `object` exist; `subject` and `object` are not “extremely” distant.
// Compares distance between `subject` and `object` to `value`.
//
// TODO(sfiera): provide a definition of “distance” in this context, and especially what
// “extremely” distant means.
struct DistanceCondition : ConditionBase {
    int64_t value;
};

// Ops: EQ, NE, LT, GT, LE, GE
// Compares health fraction of `subject` (e.g. 0.5 for half health) to `value`.
//
// Note: an initially-hidden object that has not yet been unhidden is considered “destroyed”; i.e.
// its health fraction is 0.0.
struct HealthCondition : ConditionBase {
    double value;
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
// Precondition: `subject` and `object` exist.
// Compares target of `subject` to `object`.
struct OrderedCondition : ConditionBase {};

// Ops: EQ, NE
// Precondition: `subject` exists.
// Compares owner of `subject` to `player`.
struct OwnerCondition : ConditionBase {
    Handle<Admiral> player;
};

// Ops: EQ, NE, LT, GT, LE, GE
// Compares ship count of `player` to `value`.
struct ShipsCondition : ConditionBase {
    Handle<Admiral> player;
    int64_t         value;
};

// Ops: EQ, NE, LT, GT, LE, GE
// Precondition: `subject` exists.
// Compares speed of `subject` to `value`.
struct SpeedCondition : ConditionBase {
    Fixed value;
};

// Ops: EQ, NE, LT, GT, LE, GE
// Precondition: `subject` exists.
// Compares `subject` to the control, target, or flagship of the local player, per `value`.
//
// Warning: not net-safe.
struct SubjectCondition : ConditionBase {
    enum class Value { CONTROL, TARGET, PLAYER };

    Value value;
};

// Ops: EQ, NE, LT, GT, LE, GE
// Compares `subject` to the control, target, or flagship of the local player, per `value`.
//
// Note: On a level that specifies a `start_time`, the setup time counts for only 1/3 as much as
// time after the
//
// TODO(sfiera): provide a way to specify game time “normally”
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

union Condition {
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
    OrderedCondition   ordered;
    OwnerCondition     owner;
    ShipsCondition     ships;
    SpeedCondition     speed;
    SubjectCondition   subject;
    TimeCondition      time;
    ZoomCondition      zoom;

    Condition(AutopilotCondition c);
    Condition(BuildingCondition c);
    Condition(ComputerCondition c);
    Condition(CounterCondition c);
    Condition(DestroyedCondition c);
    Condition(DistanceCondition c);
    Condition(HealthCondition c);
    Condition(MessageCondition c);
    Condition(OrderedCondition c);
    Condition(OwnerCondition c);
    Condition(ShipsCondition c);
    Condition(SpeedCondition c);
    Condition(SubjectCondition c);
    Condition(TimeCondition c);
    Condition(ZoomCondition c);

    ~Condition();
    Condition(Condition&&);
    Condition& operator=(Condition&&);

    static const Condition*            get(int n);
    static HandleList<const Condition> all();
};

Condition condition(path_value x);

}  // namespace antares

#endif  // ANTARES_DATA_CONDITION_HPP_

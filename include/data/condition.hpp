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
#include "data/object-ref.hpp"
#include "math/fixed.hpp"
#include "math/units.hpp"

namespace antares {

union Action;
union ConditionWhen;
struct Initial;
class path_value;

enum class ConditionType {
    NONE = 0,
    AUTOPILOT,
    BUILDING,
    CASH,
    COMPUTER,
    COUNT,
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
enum class ConditionEqOp { EQ, NE };

struct ConditionBase {
    ConditionType type = ConditionType::NONE;
};

// Compares player’s autopilot state (on = true; off = false) to `value`.
struct AutopilotCondition : ConditionBase {
    ConditionEqOp   op = ConditionEqOp::EQ;
    Handle<Admiral> player;
    bool            value;
};

// Precondition: player has a build object.
// Compares player’s build object state (building = true; not building = false) to `value`.
struct BuildingCondition : ConditionBase {
    ConditionEqOp   op = ConditionEqOp::EQ;
    Handle<Admiral> player;
    bool            value;
};

// Compares player’s cash reserves to `value`.
//
// Warning: not net-safe.
struct CashCondition : ConditionBase {
    ConditionOp     op = ConditionOp::EQ;
    Handle<Admiral> player;
    Fixed           value;
};

// Compares local player’s (screen, line), or just screen if line < 0.
//
// Warning: not net-safe.
struct ComputerCondition : ConditionBase {
    ConditionEqOp          op = ConditionEqOp::EQ;
    Screen                 screen;
    sfz::optional<int64_t> line;
};

// Compares the number of true sub-conditions to `value`.
struct CountCondition : ConditionBase {
    ConditionOp                op = ConditionOp::EQ;
    int64_t                    value;
    std::vector<ConditionWhen> of;
};

// Compares given counter of given admiral to `value`.
struct CounterCondition : ConditionBase {
    ConditionOp     op = ConditionOp::EQ;
    Handle<Admiral> player;
    int64_t         counter;
    int64_t         value;
};

// Compares state of given object (destroyed = true; alive = false) to `value`.
//
// Note: an initially-hidden object that has not yet been unhidden is considered “destroyed”
struct DestroyedCondition : ConditionBase {
    ConditionEqOp op = ConditionEqOp::EQ;
    ObjectRef     what;
    bool          value;
};

// Precondition: `from` and `to` exist.
// Compares distance between `from` and `to` to `value`.
//
// TODO(sfiera): provide a definition of “distance” in this context.
struct DistanceCondition : ConditionBase {
    ConditionOp op = ConditionOp::EQ;
    ObjectRef   from;
    ObjectRef   to;
    int64_t     value;
};

// Compares health fraction of `what` (e.g. 0.5 for half health) to `value`.
//
// Note: an initially-hidden object that has not yet been unhidden is considered “destroyed”; i.e.
// its health fraction is 0.0.
struct HealthCondition : ConditionBase {
    ConditionOp op = ConditionOp::EQ;
    ObjectRef   what;
    double      value;
};

// Compares (id, page) of local player’s current message to (id, page).
//
// Warning: not net-safe.
struct MessageCondition : ConditionBase {
    ConditionEqOp op = ConditionEqOp::EQ;
    int64_t       id;
    int64_t       page;
};

// Precondition: `a` and `b` exist.
// Compares `a` to `b`.
struct ObjectCondition : ConditionBase {
    ConditionEqOp op = ConditionEqOp::EQ;
    ObjectRef     a, b;
};

// Precondition: `what` exists.
// Compares owner of `what` to `player`.
struct OwnerCondition : ConditionBase {
    ConditionEqOp   op = ConditionEqOp::EQ;
    ObjectRef       what;
    Handle<Admiral> player;
};

// Compares ship count of `player` to `value`.
struct ShipsCondition : ConditionBase {
    ConditionOp     op = ConditionOp::EQ;
    Handle<Admiral> player;
    int64_t         value;
};

// Precondition: `what` exists.
// Compares speed of `what` to `value`.
struct SpeedCondition : ConditionBase {
    ConditionOp op = ConditionOp::EQ;
    ObjectRef   what;
    Fixed       value;
};

// Precondition: `what` and `target` exist.
// Compares target of `what` to `target`.
struct TargetCondition : ConditionBase {
    ConditionEqOp op = ConditionEqOp::EQ;
    ObjectRef     what;
    ObjectRef     target;
};

// Compares the game time to `duration`. Zero is the start of play; setup time is negative.
//
// `legacy_start_time` specifies an alternate mode where the setup time counts for only 1/3 as much
// as time after the setup finishes.
struct TimeCondition : ConditionBase {
    ConditionOp op = ConditionOp::EQ;
    ticks       duration;
    bool        legacy_start_time;
};

// Compares zoom level of the local player to `value`.
//
// Warning: not net-safe.
struct ZoomCondition : ConditionBase {
    ConditionOp op = ConditionOp::EQ;
    Zoom        value;
};

union ConditionWhen {
    using Type = ConditionType;

    ConditionBase base;
    ConditionType type() const;

    AutopilotCondition autopilot;
    BuildingCondition  building;
    CashCondition      cash;
    ComputerCondition  computer;
    CountCondition     count;
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

    ConditionWhen();
    ConditionWhen(AutopilotCondition c);
    ConditionWhen(BuildingCondition c);
    ConditionWhen(CashCondition c);
    ConditionWhen(ComputerCondition c);
    ConditionWhen(CountCondition c);
    ConditionWhen(CounterCondition c);
    ConditionWhen(DestroyedCondition c);
    ConditionWhen(DistanceCondition c);
    ConditionWhen(HealthCondition c);
    ConditionWhen(MessageCondition c);
    ConditionWhen(ObjectCondition c);
    ConditionWhen(OwnerCondition c);
    ConditionWhen(ShipsCondition c);
    ConditionWhen(SpeedCondition c);
    ConditionWhen(TargetCondition c);
    ConditionWhen(TimeCondition c);
    ConditionWhen(ZoomCondition c);

    ~ConditionWhen();
    ConditionWhen(ConditionWhen&&);
    ConditionWhen& operator=(ConditionWhen&&);
};

struct Condition {
    bool disabled   = false;
    bool persistent = false;

    ConditionWhen when;

    sfz::optional<ObjectRef> subject;
    sfz::optional<ObjectRef> object;
    std::vector<Action>      action;

    static const Condition*            get(int n);
    static HandleList<const Condition> all();
};

Condition condition(path_value x);

}  // namespace antares

#endif  // ANTARES_DATA_CONDITION_HPP_

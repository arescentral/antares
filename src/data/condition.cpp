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

#include "data/condition.hpp"

#include "data/action.hpp"
#include "data/field.hpp"
#include "data/initial.hpp"

namespace antares {

static ConditionWhen when(path_value x);

// clang-format off
#define COMMON_CONDITION_FIELDS                                                                   \
            {"type", {&ConditionBase::type, required_condition_type}}
// clang-format on

ConditionWhen::Type ConditionWhen::type() const { return base.type; }

ConditionWhen::ConditionWhen() : base{} {}
ConditionWhen::ConditionWhen(AutopilotCondition a) : autopilot(std::move(a)) {}
ConditionWhen::ConditionWhen(BuildingCondition a) : building(std::move(a)) {}
ConditionWhen::ConditionWhen(CashCondition a) : cash(std::move(a)) {}
ConditionWhen::ConditionWhen(ComputerCondition a) : computer(std::move(a)) {}
ConditionWhen::ConditionWhen(CountCondition a) : count(std::move(a)) {}
ConditionWhen::ConditionWhen(CounterCondition a) : counter(std::move(a)) {}
ConditionWhen::ConditionWhen(DestroyedCondition a) : destroyed(std::move(a)) {}
ConditionWhen::ConditionWhen(DistanceCondition a) : distance(std::move(a)) {}
ConditionWhen::ConditionWhen(HealthCondition a) : health(std::move(a)) {}
ConditionWhen::ConditionWhen(MessageCondition a) : message(std::move(a)) {}
ConditionWhen::ConditionWhen(ObjectCondition a) : object(std::move(a)) {}
ConditionWhen::ConditionWhen(OwnerCondition a) : owner(std::move(a)) {}
ConditionWhen::ConditionWhen(ShipsCondition a) : ships(std::move(a)) {}
ConditionWhen::ConditionWhen(SpeedCondition a) : speed(std::move(a)) {}
ConditionWhen::ConditionWhen(TargetCondition a) : target(std::move(a)) {}
ConditionWhen::ConditionWhen(TimeCondition a) : time(std::move(a)) {}
ConditionWhen::ConditionWhen(ZoomCondition a) : zoom(std::move(a)) {}

ConditionWhen::ConditionWhen(ConditionWhen&& a) {
    switch (a.type()) {
        case ConditionWhen::Type::NONE: new (this) ConditionWhen(); break;
        case ConditionWhen::Type::AUTOPILOT:
            new (this) ConditionWhen(std::move(a.autopilot));
            break;
        case ConditionWhen::Type::BUILDING: new (this) ConditionWhen(std::move(a.building)); break;
        case ConditionWhen::Type::CASH: new (this) ConditionWhen(std::move(a.cash)); break;
        case ConditionWhen::Type::COMPUTER: new (this) ConditionWhen(std::move(a.computer)); break;
        case ConditionWhen::Type::COUNT: new (this) ConditionWhen(std::move(a.count)); break;
        case ConditionWhen::Type::COUNTER: new (this) ConditionWhen(std::move(a.counter)); break;
        case ConditionWhen::Type::DESTROYED:
            new (this) ConditionWhen(std::move(a.destroyed));
            break;
        case ConditionWhen::Type::DISTANCE: new (this) ConditionWhen(std::move(a.distance)); break;
        case ConditionWhen::Type::HEALTH: new (this) ConditionWhen(std::move(a.health)); break;
        case ConditionWhen::Type::MESSAGE: new (this) ConditionWhen(std::move(a.message)); break;
        case ConditionWhen::Type::TARGET: new (this) ConditionWhen(std::move(a.target)); break;
        case ConditionWhen::Type::OBJECT: new (this) ConditionWhen(std::move(a.object)); break;
        case ConditionWhen::Type::OWNER: new (this) ConditionWhen(std::move(a.owner)); break;
        case ConditionWhen::Type::SHIPS: new (this) ConditionWhen(std::move(a.ships)); break;
        case ConditionWhen::Type::SPEED: new (this) ConditionWhen(std::move(a.speed)); break;
        case ConditionWhen::Type::TIME: new (this) ConditionWhen(std::move(a.time)); break;
        case ConditionWhen::Type::ZOOM: new (this) ConditionWhen(std::move(a.zoom)); break;
    }
}

ConditionWhen& ConditionWhen::operator=(ConditionWhen&& a) {
    this->~ConditionWhen();
    new (this) ConditionWhen(std::move(a));
    return *this;
}

ConditionWhen::~ConditionWhen() {
    switch (type()) {
        case ConditionWhen::Type::NONE: base.~ConditionBase(); break;
        case ConditionWhen::Type::AUTOPILOT: autopilot.~AutopilotCondition(); break;
        case ConditionWhen::Type::BUILDING: building.~BuildingCondition(); break;
        case ConditionWhen::Type::CASH: cash.~CashCondition(); break;
        case ConditionWhen::Type::COMPUTER: computer.~ComputerCondition(); break;
        case ConditionWhen::Type::COUNT: count.~CountCondition(); break;
        case ConditionWhen::Type::COUNTER: counter.~CounterCondition(); break;
        case ConditionWhen::Type::DESTROYED: destroyed.~DestroyedCondition(); break;
        case ConditionWhen::Type::DISTANCE: distance.~DistanceCondition(); break;
        case ConditionWhen::Type::HEALTH: health.~HealthCondition(); break;
        case ConditionWhen::Type::MESSAGE: message.~MessageCondition(); break;
        case ConditionWhen::Type::OBJECT: object.~ObjectCondition(); break;
        case ConditionWhen::Type::OWNER: owner.~OwnerCondition(); break;
        case ConditionWhen::Type::SHIPS: ships.~ShipsCondition(); break;
        case ConditionWhen::Type::SPEED: speed.~SpeedCondition(); break;
        case ConditionWhen::Type::TARGET: target.~TargetCondition(); break;
        case ConditionWhen::Type::TIME: time.~TimeCondition(); break;
        case ConditionWhen::Type::ZOOM: zoom.~ZoomCondition(); break;
    }
}

static ConditionWhen::Type required_condition_type(path_value x) {
    return required_enum<ConditionWhen::Type>(
            x, {{"autopilot", ConditionWhen::Type::AUTOPILOT},
                {"building", ConditionWhen::Type::BUILDING},
                {"cash", ConditionWhen::Type::CASH},
                {"computer", ConditionWhen::Type::COMPUTER},
                {"count", ConditionWhen::Type::COUNT},
                {"counter", ConditionWhen::Type::COUNTER},
                {"destroyed", ConditionWhen::Type::DESTROYED},
                {"distance", ConditionWhen::Type::DISTANCE},
                {"health", ConditionWhen::Type::HEALTH},
                {"message", ConditionWhen::Type::MESSAGE},
                {"owner", ConditionWhen::Type::OWNER},
                {"ships", ConditionWhen::Type::SHIPS},
                {"speed", ConditionWhen::Type::SPEED},
                {"object", ConditionWhen::Type::OBJECT},
                {"target", ConditionWhen::Type::TARGET},
                {"time", ConditionWhen::Type::TIME},
                {"zoom", ConditionWhen::Type::ZOOM}});
}

static ConditionOp required_condition_op(path_value x) {
    return required_enum<ConditionOp>(
            x, {{"eq", ConditionOp::EQ},
                {"ne", ConditionOp::NE},
                {"lt", ConditionOp::LT},
                {"gt", ConditionOp::GT},
                {"le", ConditionOp::LE},
                {"ge", ConditionOp::GE}});
}

static ConditionEqOp required_condition_eq_op(path_value x) {
    return required_enum<ConditionEqOp>(x, {{"eq", ConditionEqOp::EQ}, {"ne", ConditionEqOp::NE}});
}

static ConditionWhen autopilot_condition(path_value x) {
    return required_struct<AutopilotCondition>(
            x, {COMMON_CONDITION_FIELDS,
                {"op", {&AutopilotCondition::op, required_condition_eq_op}},
                {"player", {&AutopilotCondition::player, required_admiral}},
                {"value", {&AutopilotCondition::value, required_bool}}});
}

static ConditionWhen building_condition(path_value x) {
    return required_struct<BuildingCondition>(
            x, {COMMON_CONDITION_FIELDS,
                {"op", {&BuildingCondition::op, required_condition_eq_op}},
                {"player", {&BuildingCondition::player, required_admiral}},
                {"value", {&BuildingCondition::value, required_bool}}});
}

static ConditionWhen cash_condition(path_value x) {
    return required_struct<CashCondition>(
            x, {COMMON_CONDITION_FIELDS,
                {"op", {&CashCondition::op, required_condition_op}},
                {"player", {&CashCondition::player, required_admiral}},
                {"value", {&CashCondition::value, required_fixed}}});
}

static ConditionWhen computer_condition(path_value x) {
    return required_struct<ComputerCondition>(
            x, {COMMON_CONDITION_FIELDS,
                {"op", {&ComputerCondition::op, required_condition_eq_op}},
                {"player", nullptr},
                {"screen", {&ComputerCondition::screen, required_screen}},
                {"line", {&ComputerCondition::line, optional_int}}});
}

static ConditionWhen count_condition(path_value x) {
    return required_struct<CountCondition>(
            x, {COMMON_CONDITION_FIELDS,
                {"op", {&CountCondition::op, required_condition_op}},
                {"value", {&CountCondition::value, required_int}},
                {"of", {&CountCondition::of, required_array<ConditionWhen, when>}}});
}

static ConditionWhen counter_condition(path_value x) {
    return required_struct<CounterCondition>(
            x, {COMMON_CONDITION_FIELDS,
                {"op", {&CounterCondition::op, required_condition_op}},
                {"player", {&CounterCondition::player, required_admiral}},
                {"counter", {&CounterCondition::counter, required_int}},
                {"value", {&CounterCondition::value, required_int}}});
}

static ConditionWhen destroyed_condition(path_value x) {
    return required_struct<DestroyedCondition>(
            x, {COMMON_CONDITION_FIELDS,
                {"op", {&DestroyedCondition::op, required_condition_eq_op}},
                {"what", {&DestroyedCondition::what, required_object_ref}},
                {"value", {&DestroyedCondition::value, required_bool}}});
}

static int64_t distance(path_value x) {
    double d = required_double(x);
    d        = floor(pow(d, 2));
    return d;
}

static ConditionWhen distance_condition(path_value x) {
    return required_struct<DistanceCondition>(
            x, {COMMON_CONDITION_FIELDS,
                {"op", {&DistanceCondition::op, required_condition_op}},
                {"value", {&DistanceCondition::value, distance}},
                {"from", {&DistanceCondition::from, required_object_ref}},
                {"to", {&DistanceCondition::to, required_object_ref}}});
}

static ConditionWhen health_condition(path_value x) {
    return required_struct<HealthCondition>(
            x, {COMMON_CONDITION_FIELDS,
                {"op", {&HealthCondition::op, required_condition_op}},
                {"value", {&HealthCondition::value, required_double}},
                {"what", {&HealthCondition::what, required_object_ref}}});
}

static ConditionWhen message_condition(path_value x) {
    return required_struct<MessageCondition>(
            x, {COMMON_CONDITION_FIELDS,
                {"op", {&MessageCondition::op, required_condition_eq_op}},
                {"player", nullptr},
                {"id", {&MessageCondition::id, required_int}},
                {"page", {&MessageCondition::page, required_int}}});
}

static ConditionWhen owner_condition(path_value x) {
    return required_struct<OwnerCondition>(
            x, {COMMON_CONDITION_FIELDS,
                {"op", {&OwnerCondition::op, required_condition_eq_op}},
                {"player", {&OwnerCondition::player, required_admiral}},
                {"what", {&OwnerCondition::what, required_object_ref}}});
}

static ConditionWhen ships_condition(path_value x) {
    return required_struct<ShipsCondition>(
            x, {COMMON_CONDITION_FIELDS,
                {"op", {&ShipsCondition::op, required_condition_op}},
                {"player", {&ShipsCondition::player, required_admiral}},
                {"value", {&ShipsCondition::value, required_int}}});
}

static ConditionWhen speed_condition(path_value x) {
    return required_struct<SpeedCondition>(
            x, {COMMON_CONDITION_FIELDS,
                {"op", {&SpeedCondition::op, required_condition_op}},
                {"value", {&SpeedCondition::value, required_fixed}},
                {"what", {&SpeedCondition::what, required_object_ref}}});
}

static ConditionWhen object_condition(path_value x) {
    return required_struct<ObjectCondition>(
            x, {COMMON_CONDITION_FIELDS,
                {"op", {&ObjectCondition::op, required_condition_eq_op}},
                {"a", {&ObjectCondition::a, required_object_ref}},
                {"b", {&ObjectCondition::b, required_object_ref}}});
}

static ConditionWhen target_condition(path_value x) {
    return required_struct<TargetCondition>(
            x, {COMMON_CONDITION_FIELDS,
                {"op", {&TargetCondition::op, required_condition_eq_op}},
                {"what", {&TargetCondition::what, required_object_ref}},
                {"target", {&TargetCondition::target, required_object_ref}}});
}

static ConditionWhen time_condition(path_value x) {
    return required_struct<TimeCondition>(
            x, {COMMON_CONDITION_FIELDS,
                {"op", {&TimeCondition::op, required_condition_op}},
                {"duration", {&TimeCondition::duration, required_ticks}},
                {"legacy_start_time", {&TimeCondition::legacy_start_time, optional_bool, false}}});
}

static ConditionWhen zoom_condition(path_value x) {
    return required_struct<ZoomCondition>(
            x, {COMMON_CONDITION_FIELDS,
                {"op", {&ZoomCondition::op, required_condition_op}},
                {"player", nullptr},
                {"value", {&ZoomCondition::value, required_zoom}}});
}

static ConditionWhen when(path_value x) {
    if (!x.value().is_map()) {
        throw std::runtime_error(pn::format("{0}: must be map", x.path()).c_str());
    }

    switch (required_condition_type(x.get("type"))) {
        case ConditionWhen::Type::NONE: throw std::runtime_error("condition type none?");
        case ConditionWhen::Type::AUTOPILOT: return autopilot_condition(x);
        case ConditionWhen::Type::BUILDING: return building_condition(x);
        case ConditionWhen::Type::CASH: return cash_condition(x);
        case ConditionWhen::Type::COMPUTER: return computer_condition(x);
        case ConditionWhen::Type::COUNT: return count_condition(x);
        case ConditionWhen::Type::COUNTER: return counter_condition(x);
        case ConditionWhen::Type::DESTROYED: return destroyed_condition(x);
        case ConditionWhen::Type::DISTANCE: return distance_condition(x);
        case ConditionWhen::Type::HEALTH: return health_condition(x);
        case ConditionWhen::Type::MESSAGE: return message_condition(x);
        case ConditionWhen::Type::TARGET: return target_condition(x);
        case ConditionWhen::Type::OBJECT: return object_condition(x);
        case ConditionWhen::Type::OWNER: return owner_condition(x);
        case ConditionWhen::Type::SHIPS: return ships_condition(x);
        case ConditionWhen::Type::SPEED: return speed_condition(x);
        case ConditionWhen::Type::TIME: return time_condition(x);
        case ConditionWhen::Type::ZOOM: return zoom_condition(x);
    }
}

Condition condition(path_value x) {
    return required_struct<Condition>(
            x, {{"persistent", {&Condition::persistent, optional_bool, false}},
                {"disabled", {&Condition::disabled, optional_bool, false}},
                {"when", {&Condition::when, when}},
                {"subject", {&Condition::subject, optional_object_ref}},
                {"object", {&Condition::object, optional_object_ref}},
                {"action", {&Condition::action, required_array<Action, action>}}});
}

}  // namespace antares

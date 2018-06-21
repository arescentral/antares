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

// clang-format off
#define COMMON_CONDITION_FIELDS                                                                   \
            {"type", {&ConditionBase::type, required_condition_type}},                            \
            {"op", {&ConditionBase::op, required_condition_op}},                                  \
            {"persistent", {&ConditionBase::persistent, optional_bool, false}},                   \
            {"disabled", {&ConditionBase::disabled, optional_bool, false}},                       \
            {"subject", {&ConditionBase::subject, optional_initial, Initial::none()}},            \
            {"object", {&ConditionBase::object, optional_initial, Initial::none()}},              \
            {"action", {&ConditionBase::action, required_array<Action, action>}}
// clang-format on

Condition::Type Condition::type() const { return base.type; }

Condition::Condition(AutopilotCondition a) : autopilot(std::move(a)) {}
Condition::Condition(BuildingCondition a) : building(std::move(a)) {}
Condition::Condition(ComputerCondition a) : computer(std::move(a)) {}
Condition::Condition(CounterCondition a) : counter(std::move(a)) {}
Condition::Condition(DestroyedCondition a) : destroyed(std::move(a)) {}
Condition::Condition(DistanceCondition a) : distance(std::move(a)) {}
Condition::Condition(HealthCondition a) : health(std::move(a)) {}
Condition::Condition(MessageCondition a) : message(std::move(a)) {}
Condition::Condition(OrderedCondition a) : ordered(std::move(a)) {}
Condition::Condition(OwnerCondition a) : owner(std::move(a)) {}
Condition::Condition(ShipsCondition a) : ships(std::move(a)) {}
Condition::Condition(SpeedCondition a) : speed(std::move(a)) {}
Condition::Condition(SubjectCondition a) : subject(std::move(a)) {}
Condition::Condition(TimeCondition a) : time(std::move(a)) {}
Condition::Condition(ZoomCondition a) : zoom(std::move(a)) {}

Condition::Condition(Condition&& a) {
    switch (a.type()) {
        case Condition::Type::AUTOPILOT: new (this) Condition(std::move(a.autopilot)); break;
        case Condition::Type::BUILDING: new (this) Condition(std::move(a.building)); break;
        case Condition::Type::COMPUTER: new (this) Condition(std::move(a.computer)); break;
        case Condition::Type::COUNTER: new (this) Condition(std::move(a.counter)); break;
        case Condition::Type::DESTROYED: new (this) Condition(std::move(a.destroyed)); break;
        case Condition::Type::DISTANCE: new (this) Condition(std::move(a.distance)); break;
        case Condition::Type::HEALTH: new (this) Condition(std::move(a.health)); break;
        case Condition::Type::MESSAGE: new (this) Condition(std::move(a.message)); break;
        case Condition::Type::ORDERED: new (this) Condition(std::move(a.ordered)); break;
        case Condition::Type::OWNER: new (this) Condition(std::move(a.owner)); break;
        case Condition::Type::SHIPS: new (this) Condition(std::move(a.ships)); break;
        case Condition::Type::SPEED: new (this) Condition(std::move(a.speed)); break;
        case Condition::Type::SUBJECT: new (this) Condition(std::move(a.subject)); break;
        case Condition::Type::TIME: new (this) Condition(std::move(a.time)); break;
        case Condition::Type::ZOOM: new (this) Condition(std::move(a.zoom)); break;
    }
}

Condition& Condition::operator=(Condition&& a) {
    this->~Condition();
    new (this) Condition(std::move(a));
    return *this;
}

Condition::~Condition() {
    switch (type()) {
        case Condition::Type::AUTOPILOT: autopilot.~AutopilotCondition(); break;
        case Condition::Type::BUILDING: building.~BuildingCondition(); break;
        case Condition::Type::COMPUTER: computer.~ComputerCondition(); break;
        case Condition::Type::COUNTER: counter.~CounterCondition(); break;
        case Condition::Type::DESTROYED: destroyed.~DestroyedCondition(); break;
        case Condition::Type::DISTANCE: distance.~DistanceCondition(); break;
        case Condition::Type::HEALTH: health.~HealthCondition(); break;
        case Condition::Type::MESSAGE: message.~MessageCondition(); break;
        case Condition::Type::ORDERED: ordered.~OrderedCondition(); break;
        case Condition::Type::OWNER: owner.~OwnerCondition(); break;
        case Condition::Type::SHIPS: ships.~ShipsCondition(); break;
        case Condition::Type::SPEED: speed.~SpeedCondition(); break;
        case Condition::Type::SUBJECT: subject.~SubjectCondition(); break;
        case Condition::Type::TIME: time.~TimeCondition(); break;
        case Condition::Type::ZOOM: zoom.~ZoomCondition(); break;
    }
}

static Condition::Type required_condition_type(path_value x) {
    return required_enum<Condition::Type>(
            x, {{"autopilot", Condition::Type::AUTOPILOT},
                {"building", Condition::Type::BUILDING},
                {"computer", Condition::Type::COMPUTER},
                {"counter", Condition::Type::COUNTER},
                {"destroyed", Condition::Type::DESTROYED},
                {"distance", Condition::Type::DISTANCE},
                {"health", Condition::Type::HEALTH},
                {"message", Condition::Type::MESSAGE},
                {"ordered", Condition::Type::ORDERED},
                {"owner", Condition::Type::OWNER},
                {"ships", Condition::Type::SHIPS},
                {"speed", Condition::Type::SPEED},
                {"subject", Condition::Type::SUBJECT},
                {"time", Condition::Type::TIME},
                {"zoom", Condition::Type::ZOOM}});
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

static Condition autopilot_condition(path_value x) {
    return required_struct<AutopilotCondition>(
            x, {COMMON_CONDITION_FIELDS,
                {"player", {&AutopilotCondition::player, required_admiral}},
                {"value", {&AutopilotCondition::value, required_bool}}});
}

static Condition building_condition(path_value x) {
    return required_struct<BuildingCondition>(
            x, {COMMON_CONDITION_FIELDS,
                {"player", {&BuildingCondition::player, required_admiral}},
                {"value", {&BuildingCondition::value, required_bool}}});
}

static Condition computer_condition(path_value x) {
    return required_struct<ComputerCondition>(
            x, {COMMON_CONDITION_FIELDS,
                {"player", nullptr},
                {"screen", {&ComputerCondition::screen, required_screen}},
                {"line", {&ComputerCondition::line, optional_int}}});
}

static Condition counter_condition(path_value x) {
    return required_struct<CounterCondition>(
            x, {COMMON_CONDITION_FIELDS,
                {"player", {&CounterCondition::player, required_admiral}},
                {"counter", {&CounterCondition::counter, required_int}},
                {"value", {&CounterCondition::value, required_int}}});
}

static Condition destroyed_condition(path_value x) {
    return required_struct<DestroyedCondition>(
            x, {COMMON_CONDITION_FIELDS,
                {"initial", {&DestroyedCondition::initial, required_initial}},
                {"value", {&DestroyedCondition::value, required_bool}}});
}

static Condition distance_condition(path_value x) {
    return required_struct<DistanceCondition>(
            x, {COMMON_CONDITION_FIELDS,
                {"player", nullptr},
                {"value", {&DistanceCondition::value, required_int}}});
}

static Condition health_condition(path_value x) {
    return required_struct<HealthCondition>(
            x, {COMMON_CONDITION_FIELDS, {"value", {&HealthCondition::value, required_double}}});
}

static Condition message_condition(path_value x) {
    return required_struct<MessageCondition>(
            x, {COMMON_CONDITION_FIELDS,
                {"player", nullptr},
                {"id", {&MessageCondition::id, required_int}},
                {"page", {&MessageCondition::page, required_int}}});
}

static Condition ordered_condition(path_value x) {
    return required_struct<OrderedCondition>(
            x, {COMMON_CONDITION_FIELDS, {"player", nullptr}, {"value", nullptr}});
}

static Condition owner_condition(path_value x) {
    return required_struct<OwnerCondition>(
            x, {COMMON_CONDITION_FIELDS, {"player", {&OwnerCondition::player, required_admiral}}});
}

static Condition ships_condition(path_value x) {
    return required_struct<ShipsCondition>(
            x, {COMMON_CONDITION_FIELDS,
                {"player", {&ShipsCondition::player, required_admiral}},
                {"value", {&ShipsCondition::value, required_int}}});
}

static Condition speed_condition(path_value x) {
    return required_struct<SpeedCondition>(
            x, {COMMON_CONDITION_FIELDS, {"value", {&SpeedCondition::value, required_fixed}}});
}

static SubjectCondition::Value required_subject_value(path_value x) {
    return required_enum<SubjectCondition::Value>(
            x, {{"control", SubjectCondition::Value::CONTROL},
                {"target", SubjectCondition::Value::TARGET},
                {"player", SubjectCondition::Value::PLAYER}});
}

static Condition subject_condition(path_value x) {
    return required_struct<SubjectCondition>(
            x, {COMMON_CONDITION_FIELDS,
                {"player", nullptr},
                {"value", {&SubjectCondition::value, required_subject_value}}});
}

static Condition time_condition(path_value x) {
    return required_struct<TimeCondition>(
            x, {COMMON_CONDITION_FIELDS,
                {"duration", {&TimeCondition::duration, required_ticks}},
                {"legacy_start_time", {&TimeCondition::legacy_start_time, optional_bool, false}}});
}

static Condition zoom_condition(path_value x) {
    return required_struct<ZoomCondition>(
            x, {COMMON_CONDITION_FIELDS,
                {"player", nullptr},
                {"value", {&ZoomCondition::value, required_zoom}}});
}

Condition condition(path_value x) {
    if (!x.value().is_map()) {
        throw std::runtime_error(pn::format("{0}: must be map", x.path()).c_str());
    }

    switch (required_condition_type(x.get("type"))) {
        case Condition::Type::AUTOPILOT: return autopilot_condition(x);
        case Condition::Type::BUILDING: return building_condition(x);
        case Condition::Type::COMPUTER: return computer_condition(x);
        case Condition::Type::COUNTER: return counter_condition(x);
        case Condition::Type::DESTROYED: return destroyed_condition(x);
        case Condition::Type::DISTANCE: return distance_condition(x);
        case Condition::Type::HEALTH: return health_condition(x);
        case Condition::Type::MESSAGE: return message_condition(x);
        case Condition::Type::ORDERED: return ordered_condition(x);
        case Condition::Type::OWNER: return owner_condition(x);
        case Condition::Type::SHIPS: return ships_condition(x);
        case Condition::Type::SPEED: return speed_condition(x);
        case Condition::Type::SUBJECT: return subject_condition(x);
        case Condition::Type::TIME: return time_condition(x);
        case Condition::Type::ZOOM: return zoom_condition(x);
    }
}

}  // namespace antares

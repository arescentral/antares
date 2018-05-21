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

ConditionType Condition::type() const { return base.type; }

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
        case ConditionType::AUTOPILOT: new (this) Condition(std::move(a.autopilot)); break;
        case ConditionType::BUILDING: new (this) Condition(std::move(a.building)); break;
        case ConditionType::COMPUTER: new (this) Condition(std::move(a.computer)); break;
        case ConditionType::COUNTER: new (this) Condition(std::move(a.counter)); break;
        case ConditionType::DESTROYED: new (this) Condition(std::move(a.destroyed)); break;
        case ConditionType::DISTANCE: new (this) Condition(std::move(a.distance)); break;
        case ConditionType::HEALTH: new (this) Condition(std::move(a.health)); break;
        case ConditionType::MESSAGE: new (this) Condition(std::move(a.message)); break;
        case ConditionType::ORDERED: new (this) Condition(std::move(a.ordered)); break;
        case ConditionType::OWNER: new (this) Condition(std::move(a.owner)); break;
        case ConditionType::SHIPS: new (this) Condition(std::move(a.ships)); break;
        case ConditionType::SPEED: new (this) Condition(std::move(a.speed)); break;
        case ConditionType::SUBJECT: new (this) Condition(std::move(a.subject)); break;
        case ConditionType::TIME: new (this) Condition(std::move(a.time)); break;
        case ConditionType::ZOOM: new (this) Condition(std::move(a.zoom)); break;
    }
}

Condition& Condition::operator=(Condition&& a) {
    this->~Condition();
    new (this) Condition(std::move(a));
    return *this;
}

Condition::~Condition() {
    switch (type()) {
        case ConditionType::AUTOPILOT: autopilot.~AutopilotCondition(); break;
        case ConditionType::BUILDING: building.~BuildingCondition(); break;
        case ConditionType::COMPUTER: computer.~ComputerCondition(); break;
        case ConditionType::COUNTER: counter.~CounterCondition(); break;
        case ConditionType::DESTROYED: destroyed.~DestroyedCondition(); break;
        case ConditionType::DISTANCE: distance.~DistanceCondition(); break;
        case ConditionType::HEALTH: health.~HealthCondition(); break;
        case ConditionType::MESSAGE: message.~MessageCondition(); break;
        case ConditionType::ORDERED: ordered.~OrderedCondition(); break;
        case ConditionType::OWNER: owner.~OwnerCondition(); break;
        case ConditionType::SHIPS: ships.~ShipsCondition(); break;
        case ConditionType::SPEED: speed.~SpeedCondition(); break;
        case ConditionType::SUBJECT: subject.~SubjectCondition(); break;
        case ConditionType::TIME: time.~TimeCondition(); break;
        case ConditionType::ZOOM: zoom.~ZoomCondition(); break;
    }
}

static ConditionType required_condition_type(path_value x) {
    return required_enum<ConditionType>(
            x, {{"autopilot", ConditionType::AUTOPILOT},
                {"building", ConditionType::BUILDING},
                {"computer", ConditionType::COMPUTER},
                {"counter", ConditionType::COUNTER},
                {"destroyed", ConditionType::DESTROYED},
                {"distance", ConditionType::DISTANCE},
                {"health", ConditionType::HEALTH},
                {"message", ConditionType::MESSAGE},
                {"ordered", ConditionType::ORDERED},
                {"owner", ConditionType::OWNER},
                {"ships", ConditionType::SHIPS},
                {"speed", ConditionType::SPEED},
                {"subject", ConditionType::SUBJECT},
                {"time", ConditionType::TIME},
                {"zoom", ConditionType::ZOOM}});
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
                {"player", nullptr},
                {"value", {&AutopilotCondition::value, required_bool}}});
}

static Condition building_condition(path_value x) {
    return required_struct<BuildingCondition>(
            x, {COMMON_CONDITION_FIELDS,
                {"player", nullptr},
                {"value", {&BuildingCondition::value, required_bool}}});
}

static Condition computer_condition(path_value x) {
    return required_struct<ComputerCondition>(
            x, {COMMON_CONDITION_FIELDS,
                {"player", nullptr},
                {"screen", {&ComputerCondition::screen, required_screen}},
                {"line", {&ComputerCondition::line, optional_int, -1}}});
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

static SubjectValue required_subject_value(path_value x) {
    return required_enum<SubjectValue>(
            x, {{"control", SubjectValue::CONTROL},
                {"target", SubjectValue::TARGET},
                {"player", SubjectValue::PLAYER}});
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
        case ConditionType::AUTOPILOT: return autopilot_condition(x);
        case ConditionType::BUILDING: return building_condition(x);
        case ConditionType::COMPUTER: return computer_condition(x);
        case ConditionType::COUNTER: return counter_condition(x);
        case ConditionType::DESTROYED: return destroyed_condition(x);
        case ConditionType::DISTANCE: return distance_condition(x);
        case ConditionType::HEALTH: return health_condition(x);
        case ConditionType::MESSAGE: return message_condition(x);
        case ConditionType::ORDERED: return ordered_condition(x);
        case ConditionType::OWNER: return owner_condition(x);
        case ConditionType::SHIPS: return ships_condition(x);
        case ConditionType::SPEED: return speed_condition(x);
        case ConditionType::SUBJECT: return subject_condition(x);
        case ConditionType::TIME: return time_condition(x);
        case ConditionType::ZOOM: return zoom_condition(x);
    }
}

}  // namespace antares

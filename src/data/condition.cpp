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

DECLARE_FIELD_READER(ConditionWhen);

#define COMMON_CONDITION_FIELDS \
    { "type", &ConditionBase::type }

ConditionWhen::Type ConditionWhen::type() const { return base.type; }

ConditionWhen::ConditionWhen() : base{} {}
ConditionWhen::ConditionWhen(AutopilotCondition a) : autopilot(std::move(a)) {}
ConditionWhen::ConditionWhen(BuildingCondition a) : building(std::move(a)) {}
ConditionWhen::ConditionWhen(CashCondition a) : cash(std::move(a)) {}
ConditionWhen::ConditionWhen(ComputerCondition a) : computer(std::move(a)) {}
ConditionWhen::ConditionWhen(CountCondition a) : count(std::move(a)) {}
ConditionWhen::ConditionWhen(DestroyedCondition a) : destroyed(std::move(a)) {}
ConditionWhen::ConditionWhen(DistanceCondition a) : distance(std::move(a)) {}
ConditionWhen::ConditionWhen(HealthCondition a) : health(std::move(a)) {}
ConditionWhen::ConditionWhen(IdentityCondition a) : identity(std::move(a)) {}
ConditionWhen::ConditionWhen(MessageCondition a) : message(std::move(a)) {}
ConditionWhen::ConditionWhen(OwnerCondition a) : owner(std::move(a)) {}
ConditionWhen::ConditionWhen(ScoreCondition a) : score(std::move(a)) {}
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
        case ConditionWhen::Type::DESTROYED:
            new (this) ConditionWhen(std::move(a.destroyed));
            break;
        case ConditionWhen::Type::DISTANCE: new (this) ConditionWhen(std::move(a.distance)); break;
        case ConditionWhen::Type::HEALTH: new (this) ConditionWhen(std::move(a.health)); break;
        case ConditionWhen::Type::IDENTITY: new (this) ConditionWhen(std::move(a.identity)); break;
        case ConditionWhen::Type::MESSAGE: new (this) ConditionWhen(std::move(a.message)); break;
        case ConditionWhen::Type::OWNER: new (this) ConditionWhen(std::move(a.owner)); break;
        case ConditionWhen::Type::SCORE: new (this) ConditionWhen(std::move(a.score)); break;
        case ConditionWhen::Type::SHIPS: new (this) ConditionWhen(std::move(a.ships)); break;
        case ConditionWhen::Type::SPEED: new (this) ConditionWhen(std::move(a.speed)); break;
        case ConditionWhen::Type::TARGET: new (this) ConditionWhen(std::move(a.target)); break;
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
        case ConditionWhen::Type::DESTROYED: destroyed.~DestroyedCondition(); break;
        case ConditionWhen::Type::DISTANCE: distance.~DistanceCondition(); break;
        case ConditionWhen::Type::HEALTH: health.~HealthCondition(); break;
        case ConditionWhen::Type::IDENTITY: identity.~IdentityCondition(); break;
        case ConditionWhen::Type::MESSAGE: message.~MessageCondition(); break;
        case ConditionWhen::Type::OWNER: owner.~OwnerCondition(); break;
        case ConditionWhen::Type::SCORE: score.~ScoreCondition(); break;
        case ConditionWhen::Type::SHIPS: ships.~ShipsCondition(); break;
        case ConditionWhen::Type::SPEED: speed.~SpeedCondition(); break;
        case ConditionWhen::Type::TARGET: target.~TargetCondition(); break;
        case ConditionWhen::Type::TIME: time.~TimeCondition(); break;
        case ConditionWhen::Type::ZOOM: zoom.~ZoomCondition(); break;
    }
}

FIELD_READER(ConditionWhen::Type) {
    return required_enum<ConditionWhen::Type>(
            x, {{"autopilot", ConditionWhen::Type::AUTOPILOT},
                {"building", ConditionWhen::Type::BUILDING},
                {"cash", ConditionWhen::Type::CASH},
                {"computer", ConditionWhen::Type::COMPUTER},
                {"count", ConditionWhen::Type::COUNT},
                {"destroyed", ConditionWhen::Type::DESTROYED},
                {"distance", ConditionWhen::Type::DISTANCE},
                {"health", ConditionWhen::Type::HEALTH},
                {"identity", ConditionWhen::Type::IDENTITY},
                {"message", ConditionWhen::Type::MESSAGE},
                {"owner", ConditionWhen::Type::OWNER},
                {"score", ConditionWhen::Type::SCORE},
                {"ships", ConditionWhen::Type::SHIPS},
                {"speed", ConditionWhen::Type::SPEED},
                {"target", ConditionWhen::Type::TARGET},
                {"time", ConditionWhen::Type::TIME},
                {"zoom", ConditionWhen::Type::ZOOM}});
}

FIELD_READER(ConditionOp) {
    return required_enum<ConditionOp>(
            x, {{"eq", ConditionOp::EQ},
                {"ne", ConditionOp::NE},
                {"lt", ConditionOp::LT},
                {"gt", ConditionOp::GT},
                {"le", ConditionOp::LE},
                {"ge", ConditionOp::GE}});
}

FIELD_READER(ConditionEqOp) {
    return required_enum<ConditionEqOp>(x, {{"eq", ConditionEqOp::EQ}, {"ne", ConditionEqOp::NE}});
}

static ConditionWhen autopilot_condition(path_value x) {
    return required_struct<AutopilotCondition>(
            x, {COMMON_CONDITION_FIELDS,
                {"op", &AutopilotCondition::op},
                {"player", &AutopilotCondition::player},
                {"value", &AutopilotCondition::value}});
}

static ConditionWhen building_condition(path_value x) {
    return required_struct<BuildingCondition>(
            x, {COMMON_CONDITION_FIELDS,
                {"op", &BuildingCondition::op},
                {"player", &BuildingCondition::player},
                {"value", &BuildingCondition::value}});
}

static ConditionWhen cash_condition(path_value x) {
    return required_struct<CashCondition>(
            x, {COMMON_CONDITION_FIELDS,
                {"op", &CashCondition::op},
                {"player", &CashCondition::player},
                {"value", &CashCondition::value}});
}

static ConditionWhen computer_condition(path_value x) {
    return required_struct<ComputerCondition>(
            x, {COMMON_CONDITION_FIELDS,
                {"op", &ComputerCondition::op},
                {"player", nullptr},
                {"screen", &ComputerCondition::screen},
                {"line", &ComputerCondition::line}});
}

static ConditionWhen count_condition(path_value x) {
    return required_struct<CountCondition>(
            x, {COMMON_CONDITION_FIELDS,
                {"op", &CountCondition::op},
                {"value", &CountCondition::value},
                {"of", &CountCondition::of}});
}

static ConditionWhen score_condition(path_value x) {
    return required_struct<ScoreCondition>(
            x, {COMMON_CONDITION_FIELDS,
                {"op", &ScoreCondition::op},
                {"counter", &ScoreCondition::counter},
                {"value", &ScoreCondition::value}});
}

static ConditionWhen destroyed_condition(path_value x) {
    return required_struct<DestroyedCondition>(
            x, {COMMON_CONDITION_FIELDS,
                {"op", &DestroyedCondition::op},
                {"object", &DestroyedCondition::object},
                {"value", &DestroyedCondition::value}});
}

static ConditionWhen distance_condition(path_value x) {
    return required_struct<DistanceCondition>(
            x, {COMMON_CONDITION_FIELDS,
                {"op", &DistanceCondition::op},
                {"value", &DistanceCondition::value},
                {"from", &DistanceCondition::from},
                {"to", &DistanceCondition::to}});
}

static ConditionWhen health_condition(path_value x) {
    return required_struct<HealthCondition>(
            x, {COMMON_CONDITION_FIELDS,
                {"op", &HealthCondition::op},
                {"value", &HealthCondition::value},
                {"object", &HealthCondition::object}});
}

static ConditionWhen message_condition(path_value x) {
    return required_struct<MessageCondition>(
            x, {COMMON_CONDITION_FIELDS,
                {"op", &MessageCondition::op},
                {"player", nullptr},
                {"id", &MessageCondition::id},
                {"page", &MessageCondition::page}});
}

static ConditionWhen owner_condition(path_value x) {
    return required_struct<OwnerCondition>(
            x, {COMMON_CONDITION_FIELDS,
                {"op", &OwnerCondition::op},
                {"player", &OwnerCondition::player},
                {"object", &OwnerCondition::object}});
}

static ConditionWhen ships_condition(path_value x) {
    return required_struct<ShipsCondition>(
            x, {COMMON_CONDITION_FIELDS,
                {"op", &ShipsCondition::op},
                {"player", &ShipsCondition::player},
                {"value", &ShipsCondition::value}});
}

static ConditionWhen speed_condition(path_value x) {
    return required_struct<SpeedCondition>(
            x, {COMMON_CONDITION_FIELDS,
                {"op", &SpeedCondition::op},
                {"value", &SpeedCondition::value},
                {"object", &SpeedCondition::object}});
}

static ConditionWhen identity_condition(path_value x) {
    return required_struct<IdentityCondition>(
            x, {COMMON_CONDITION_FIELDS,
                {"op", &IdentityCondition::op},
                {"a", &IdentityCondition::a},
                {"b", &IdentityCondition::b}});
}

static ConditionWhen target_condition(path_value x) {
    return required_struct<TargetCondition>(
            x, {COMMON_CONDITION_FIELDS,
                {"op", &TargetCondition::op},
                {"object", &TargetCondition::object},
                {"target", &TargetCondition::target}});
}

static ConditionWhen time_condition(path_value x) {
    return required_struct<TimeCondition>(
            x, {COMMON_CONDITION_FIELDS,
                {"op", &TimeCondition::op},
                {"duration", &TimeCondition::duration},
                {"legacy_start_time", &TimeCondition::legacy_start_time}});
}

static ConditionWhen zoom_condition(path_value x) {
    return required_struct<ZoomCondition>(
            x, {COMMON_CONDITION_FIELDS,
                {"op", &ZoomCondition::op},
                {"player", nullptr},
                {"value", &ZoomCondition::value}});
}

DEFINE_FIELD_READER(ConditionWhen) {
    switch (required_object_type(x, read_field<ConditionType>)) {
        case ConditionWhen::Type::NONE: throw std::runtime_error("condition type none?");
        case ConditionWhen::Type::AUTOPILOT: return autopilot_condition(x);
        case ConditionWhen::Type::BUILDING: return building_condition(x);
        case ConditionWhen::Type::CASH: return cash_condition(x);
        case ConditionWhen::Type::COMPUTER: return computer_condition(x);
        case ConditionWhen::Type::COUNT: return count_condition(x);
        case ConditionWhen::Type::DESTROYED: return destroyed_condition(x);
        case ConditionWhen::Type::DISTANCE: return distance_condition(x);
        case ConditionWhen::Type::HEALTH: return health_condition(x);
        case ConditionWhen::Type::MESSAGE: return message_condition(x);
        case ConditionWhen::Type::TARGET: return target_condition(x);
        case ConditionWhen::Type::IDENTITY: return identity_condition(x);
        case ConditionWhen::Type::OWNER: return owner_condition(x);
        case ConditionWhen::Type::SCORE: return score_condition(x);
        case ConditionWhen::Type::SHIPS: return ships_condition(x);
        case ConditionWhen::Type::SPEED: return speed_condition(x);
        case ConditionWhen::Type::TIME: return time_condition(x);
        case ConditionWhen::Type::ZOOM: return zoom_condition(x);
    }
}

DEFINE_FIELD_READER(Condition) {
    return required_struct<Condition>(
            x, {{"persistent", &Condition::persistent},
                {"disabled", &Condition::disabled},
                {"when", &Condition::when},
                {"subject", &Condition::subject},
                {"direct", &Condition::direct},
                {"action", &Condition::action}});
}

}  // namespace antares

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
            {"type", nullptr},                                                                    \
            {"op", {&Condition::op, required_condition_op}},                                      \
            {"persistent", {&Condition::persistent, optional_bool, false}},                       \
            {"disabled", {&Condition::disabled, optional_bool, false}},                           \
            {"subject", {&Condition::subject, optional_initial, Initial::none()}},                \
            {"object", {&Condition::object, optional_initial, Initial::none()}},                  \
            {"action",                                                                            \
             {&Condition::action, required_array<std::unique_ptr<const Action>, action>}}
// clang-format on

template <typename T>
static std::unique_ptr<Condition> condition_ptr(T t) {
    return std::unique_ptr<Condition>(new T(std::move(t)));
}

static std::unique_ptr<Condition> autopilot_condition(path_value x) {
    return condition_ptr(required_struct<AutopilotCondition>(
            x, {COMMON_CONDITION_FIELDS,
                {"player", nullptr},
                {"value", {&AutopilotCondition::value, required_bool}}}));
}

static std::unique_ptr<Condition> building_condition(path_value x) {
    return condition_ptr(required_struct<BuildingCondition>(
            x, {COMMON_CONDITION_FIELDS,
                {"player", nullptr},
                {"value", {&BuildingCondition::value, required_bool}}}));
}

static std::unique_ptr<Condition> computer_condition(path_value x) {
    return condition_ptr(required_struct<ComputerCondition>(
            x, {COMMON_CONDITION_FIELDS,
                {"player", nullptr},
                {"screen", {&ComputerCondition::screen, required_screen}},
                {"line", {&ComputerCondition::line, optional_int, -1}}}));
}

static std::unique_ptr<Condition> counter_condition(path_value x) {
    return condition_ptr(required_struct<CounterCondition>(
            x, {COMMON_CONDITION_FIELDS,
                {"player", {&CounterCondition::player, required_admiral}},
                {"counter", {&CounterCondition::counter, required_int}},
                {"value", {&CounterCondition::value, required_int}}}));
}

static std::unique_ptr<Condition> destroyed_condition(path_value x) {
    return condition_ptr(required_struct<DestroyedCondition>(
            x, {COMMON_CONDITION_FIELDS,
                {"initial", {&DestroyedCondition::initial, required_initial}},
                {"value", {&DestroyedCondition::value, required_bool}}}));
}

static std::unique_ptr<Condition> distance_condition(path_value x) {
    return condition_ptr(required_struct<DistanceCondition>(
            x, {COMMON_CONDITION_FIELDS,
                {"player", nullptr},
                {"value", {&DistanceCondition::value, required_int}}}));
}

static std::unique_ptr<Condition> health_condition(path_value x) {
    return condition_ptr(required_struct<HealthCondition>(
            x, {COMMON_CONDITION_FIELDS, {"value", {&HealthCondition::value, required_double}}}));
}

static std::unique_ptr<Condition> message_condition(path_value x) {
    return condition_ptr(required_struct<MessageCondition>(
            x, {COMMON_CONDITION_FIELDS,
                {"player", nullptr},
                {"id", {&MessageCondition::id, required_int}},
                {"page", {&MessageCondition::page, required_int}}}));
}

static std::unique_ptr<Condition> ordered_condition(path_value x) {
    return condition_ptr(required_struct<OrderedCondition>(
            x, {COMMON_CONDITION_FIELDS, {"player", nullptr}, {"value", nullptr}}));
}

static std::unique_ptr<Condition> owner_condition(path_value x) {
    return condition_ptr(required_struct<OwnerCondition>(
            x,
            {COMMON_CONDITION_FIELDS, {"player", {&OwnerCondition::player, required_admiral}}}));
}

static std::unique_ptr<Condition> ships_condition(path_value x) {
    return condition_ptr(required_struct<ShipsCondition>(
            x, {COMMON_CONDITION_FIELDS,
                {"player", {&ShipsCondition::player, required_admiral}},
                {"value", {&ShipsCondition::value, required_int}}}));
}

static std::unique_ptr<Condition> speed_condition(path_value x) {
    return condition_ptr(required_struct<SpeedCondition>(
            x, {COMMON_CONDITION_FIELDS, {"value", {&SpeedCondition::value, required_fixed}}}));
}

static std::unique_ptr<Condition> subject_condition(path_value x) {
    return condition_ptr(required_struct<SubjectCondition>(
            x, {COMMON_CONDITION_FIELDS,
                {"player", nullptr},
                {"value", {&SubjectCondition::value, required_subject_value}}}));
}

static std::unique_ptr<Condition> time_condition(path_value x) {
    return condition_ptr(required_struct<TimeCondition>(
            x,
            {COMMON_CONDITION_FIELDS,
             {"duration", {&TimeCondition::duration, required_ticks}},
             {"legacy_start_time", {&TimeCondition::legacy_start_time, optional_bool, false}}}));
}

static std::unique_ptr<Condition> zoom_condition(path_value x) {
    return condition_ptr(required_struct<ZoomCondition>(
            x, {COMMON_CONDITION_FIELDS,
                {"player", nullptr},
                {"value", {&ZoomCondition::value, required_zoom}}}));
}

std::unique_ptr<Condition> condition(path_value x) {
    if (!x.value().is_map()) {
        throw std::runtime_error(pn::format("{0}: must be map", x.path()).c_str());
    }

    pn::string_view            type = required_string(x.get("type"));
    std::unique_ptr<Condition> c;
    if (type == "autopilot") {
        return autopilot_condition(x);
    } else if (type == "building") {
        return building_condition(x);
    } else if (type == "computer") {
        return computer_condition(x);
    } else if (type == "counter") {
        return counter_condition(x);
    } else if (type == "destroyed") {
        return destroyed_condition(x);
    } else if (type == "distance") {
        return distance_condition(x);
    } else if (type == "false") {
        return std::unique_ptr<Condition>(new FalseCondition);
    } else if (type == "health") {
        return health_condition(x);
    } else if (type == "message") {
        return message_condition(x);
    } else if (type == "ordered") {
        return ordered_condition(x);
    } else if (type == "owner") {
        return owner_condition(x);
    } else if (type == "ships") {
        return ships_condition(x);
    } else if (type == "speed") {
        return speed_condition(x);
    } else if (type == "subject") {
        return subject_condition(x);
    } else if (type == "time") {
        return time_condition(x);
    } else if (type == "zoom") {
        return zoom_condition(x);
    } else {
        throw std::runtime_error(pn::format("unknown type: {0}", type).c_str());
    }
    return c;
}

}  // namespace antares

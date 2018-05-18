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

static std::unique_ptr<Condition> autopilot_condition(path_value x) {
    std::unique_ptr<AutopilotCondition> c(new AutopilotCondition);
    c->value = required_bool(x.get("value"));
    return std::move(c);
}

static std::unique_ptr<Condition> building_condition(path_value x) {
    std::unique_ptr<BuildingCondition> c(new BuildingCondition);
    c->value = required_bool(x.get("value"));
    return std::move(c);
}

static std::unique_ptr<Condition> computer_condition(path_value x) {
    std::unique_ptr<ComputerCondition> c(new ComputerCondition);
    c->screen = required_screen(x.get("screen"));
    c->line   = optional_int(x.get("line")).value_or(-1);
    return std::move(c);
}

static std::unique_ptr<Condition> counter_condition(path_value x) {
    std::unique_ptr<CounterCondition> c(new CounterCondition);
    c->player  = required_admiral(x.get("player"));
    c->counter = required_int(x.get("counter"));
    c->value   = required_int(x.get("value"));
    return std::move(c);
}

static std::unique_ptr<Condition> destroyed_condition(path_value x) {
    std::unique_ptr<DestroyedCondition> c(new DestroyedCondition);
    c->initial = required_initial(x.get("initial"));
    c->value   = required_bool(x.get("value"));
    return std::move(c);
}

static std::unique_ptr<Condition> distance_condition(path_value x) {
    std::unique_ptr<DistanceCondition> c(new DistanceCondition);
    c->value = required_int(x.get("value"));
    return std::move(c);
}

static std::unique_ptr<Condition> health_condition(path_value x) {
    std::unique_ptr<HealthCondition> c(new HealthCondition);
    c->value = required_double(x.get("value"));
    return std::move(c);
}

static std::unique_ptr<Condition> message_condition(path_value x) {
    std::unique_ptr<MessageCondition> c(new MessageCondition);
    c->id   = required_int(x.get("id"));
    c->page = required_int(x.get("page"));
    return std::move(c);
}

static std::unique_ptr<Condition> ordered_condition(path_value x) {
    return std::unique_ptr<OrderedCondition>(new OrderedCondition);
}

static std::unique_ptr<Condition> owner_condition(path_value x) {
    std::unique_ptr<OwnerCondition> c(new OwnerCondition);
    c->player = required_admiral(x.get("player"));
    return std::move(c);
}

static std::unique_ptr<Condition> ships_condition(path_value x) {
    std::unique_ptr<ShipsCondition> c(new ShipsCondition);
    c->player = required_admiral(x.get("player"));
    c->value  = required_int(x.get("value"));
    return std::move(c);
}

static std::unique_ptr<Condition> speed_condition(path_value x) {
    std::unique_ptr<SpeedCondition> c(new SpeedCondition);
    c->value = required_fixed(x.get("value"));
    return std::move(c);
}

static std::unique_ptr<Condition> subject_condition(path_value x) {
    std::unique_ptr<SubjectCondition> c(new SubjectCondition);
    c->value = required_subject_value(x.get("value"));
    return std::move(c);
}

static std::unique_ptr<Condition> time_condition(path_value x) {
    std::unique_ptr<TimeCondition> c(new TimeCondition);
    c->duration          = required_ticks(x.get("duration"));
    c->legacy_start_time = optional_bool(x.get("legacy_start_time")).value_or(false);
    return std::move(c);
}

static std::unique_ptr<Condition> zoom_condition(path_value x) {
    std::unique_ptr<ZoomCondition> c(new ZoomCondition);
    c->value = required_zoom(x.get("value"));
    return std::move(c);
}

std::unique_ptr<Condition> condition(path_value x) {
    if (!x.value().is_map()) {
        throw std::runtime_error(pn::format("{0}: must be map", x.path()).c_str());
    }

    pn::string_view            type = required_string(x.get("type"));
    std::unique_ptr<Condition> c;
    if (type == "autopilot") {
        c = autopilot_condition(x);
    } else if (type == "building") {
        c = building_condition(x);
    } else if (type == "computer") {
        c = computer_condition(x);
    } else if (type == "counter") {
        c = counter_condition(x);
    } else if (type == "destroyed") {
        c = destroyed_condition(x);
    } else if (type == "distance") {
        c = distance_condition(x);
    } else if (type == "false") {
        return std::unique_ptr<Condition>(new FalseCondition);
    } else if (type == "health") {
        c = health_condition(x);
    } else if (type == "message") {
        c = message_condition(x);
    } else if (type == "ordered") {
        c = ordered_condition(x);
    } else if (type == "owner") {
        c = owner_condition(x);
    } else if (type == "ships") {
        c = ships_condition(x);
    } else if (type == "speed") {
        c = speed_condition(x);
    } else if (type == "subject") {
        c = subject_condition(x);
    } else if (type == "time") {
        c = time_condition(x);
    } else if (type == "zoom") {
        c = zoom_condition(x);
    } else {
        throw std::runtime_error(pn::format("unknown type: {0}", type).c_str());
    }

    c->op                = required_condition_op(x.get("op"));
    c->persistent        = optional_bool(x.get("persistent")).value_or(false);
    c->initially_enabled = !optional_bool(x.get("initially_disabled")).value_or(false);
    c->subject           = optional_initial(x.get("subject")).value_or(Initial::none());
    c->object            = optional_initial(x.get("object")).value_or(Initial::none());
    c->action            = required_action_array(x.get("action"));

    return c;
}

}  // namespace antares

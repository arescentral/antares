// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// Copyright (C) 2015-2017 The Antares Authors
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

#include "data/action.hpp"

#include <sfz/sfz.hpp>

#include "data/base-object.hpp"
#include "data/field.hpp"
#include "data/level.hpp"
#include "data/resource.hpp"

namespace antares {

namespace {

static std::unique_ptr<Action> age_action(path_value x) {
    std::unique_ptr<AgeAction> a(new AgeAction);
    a->relative = optional_bool(x.get("relative"));
    a->value    = required_ticks_range(x.get("value"));
    return std::move(a);
}

static std::unique_ptr<Action> assume_action(path_value x) {
    std::unique_ptr<AssumeAction> a(new AssumeAction);
    a->which = required_int(x.get("which"));
    return std::move(a);
}

static std::unique_ptr<Action> cap_speed_action(path_value x) {
    std::unique_ptr<CapSpeedAction> a(new CapSpeedAction);
    a->value = optional_fixed(x.get("value"));
    return std::move(a);
}

static std::unique_ptr<Action> capture_action(path_value x) {
    std::unique_ptr<CaptureAction> a(new CaptureAction);
    a->player = optional_admiral(x.get("player"));
    return std::move(a);
}

static std::unique_ptr<Action> cloak_action(path_value x) {
    return std::unique_ptr<CloakAction>(new CloakAction);
}

static std::unique_ptr<Action> condition_action(path_value x) {
    std::unique_ptr<ConditionAction> a(new ConditionAction);
    a->enable = optional_condition_range(x.get("enable"))
                        .value_or(HandleList<Level::Condition>{-1, -1});
    a->disable = optional_condition_range(x.get("disable"))
                         .value_or(HandleList<Level::Condition>{-1, -1});
    return std::move(a);
}

static std::unique_ptr<Action> create_action(path_value x) {
    std::unique_ptr<CreateAction> a(new CreateAction);
    a->base               = required_base(x.get("base"));
    a->count              = optional_int_range(x.get("count")).value_or(Range<int64_t>{1, 2});
    a->relative_velocity  = optional_bool(x.get("relative_velocity"));
    a->relative_direction = optional_bool(x.get("relative_direction"));
    a->distance           = optional_int(x.get("distance")).value_or(0);
    a->inherit            = optional_bool(x.get("inherit"));
    a->legacy_random      = optional_bool(x.get("legacy_random"));
    return std::move(a);
}

static std::unique_ptr<Action> disable_action(path_value x) {
    std::unique_ptr<DisableAction> a(new DisableAction);
    a->value = required_fixed_range(x.get("value"));
    return std::move(a);
}

static std::unique_ptr<Action> energize_action(path_value x) {
    std::unique_ptr<EnergizeAction> a(new EnergizeAction);
    a->value = required_int(x.get("value"));
    return std::move(a);
}

static std::unique_ptr<Action> equip_action(path_value x) {
    std::unique_ptr<EquipAction> a(new EquipAction);
    a->which = required_weapon(x.get("which"));
    a->base  = required_base(x.get("base"));
    return std::move(a);
}

static std::unique_ptr<Action> fire_action(path_value x) {
    std::unique_ptr<FireAction> a(new FireAction);
    a->which = required_weapon(x.get("which"));
    return std::move(a);
}

static std::unique_ptr<Action> flash_action(path_value x) {
    std::unique_ptr<FlashAction> a(new FlashAction);
    a->length = required_int(x.get("length"));
    a->shade  = required_int(x.get("shade"));
    a->hue    = required_hue(x.get("hue"));
    return std::move(a);
}

static std::unique_ptr<Action> heal_action(path_value x) {
    std::unique_ptr<HealAction> a(new HealAction);
    a->value = required_int(x.get("value"));
    return std::move(a);
}

static std::unique_ptr<Action> hold_action(path_value x) {
    return std::unique_ptr<HoldPositionAction>(new HoldPositionAction);
}

static std::unique_ptr<Action> key_action(path_value x) {
    std::unique_ptr<KeyAction> a(new KeyAction);
    a->enable  = optional_keys(x.get("enable"));
    a->disable = optional_keys(x.get("disable"));
    return std::move(a);
}

static std::unique_ptr<Action> kill_action(path_value x) {
    std::unique_ptr<KillAction> a(new KillAction);
    a->kind = required_kill_kind(x.get("kind"));
    return std::move(a);
}

static std::unique_ptr<Action> land_action(path_value x) {
    std::unique_ptr<LandAction> a(new LandAction);
    a->speed = required_int(x.get("speed"));
    return std::move(a);
}

static std::unique_ptr<Action> message_action(path_value x) {
    std::unique_ptr<MessageAction> a(new MessageAction);
    a->id    = required_int(x.get("id"));
    a->pages = required_string_array(x.get("pages"));
    return std::move(a);
}

static std::unique_ptr<Action> morph_action(path_value x) {
    std::unique_ptr<MorphAction> a(new MorphAction);
    a->base      = required_base(x.get("base"));
    a->keep_ammo = optional_bool(x.get("keep_ammo"));
    return std::move(a);
}

static std::unique_ptr<Action> move_action(path_value x) {
    std::unique_ptr<MoveAction> a(new MoveAction);
    a->origin   = optional_origin(x.get("origin")).value_or(MoveOrigin::LEVEL);
    Point p     = optional_point(x.get("to")).value_or(Point{0, 0});
    a->to.h     = p.h;
    a->to.v     = p.v;
    a->distance = optional_int(x.get("distance")).value_or(0);
    return std::move(a);
}

static std::unique_ptr<Action> occupy_action(path_value x) {
    std::unique_ptr<OccupyAction> a(new OccupyAction);
    a->value = required_int(x.get("value"));
    return std::move(a);
}

static std::unique_ptr<Action> order_action(path_value x) {
    return std::unique_ptr<OrderAction>(new OrderAction);
}

static std::unique_ptr<Action> pay_action(path_value x) {
    std::unique_ptr<PayAction> a(new PayAction);
    a->value  = required_fixed(x.get("value"));
    a->player = optional_admiral(x.get("player"));
    return std::move(a);
}

static std::unique_ptr<Action> push_action(path_value x) {
    std::unique_ptr<PushAction> a(new PushAction);
    a->kind  = required_push_kind(x.get("kind"));
    a->value = optional_fixed(x.get("value")).value_or(Fixed::zero());
    return std::move(a);
}

static std::unique_ptr<Action> reveal_action(path_value x) {
    std::unique_ptr<RevealAction> a(new RevealAction);
    a->initial = required_initial_range(x.get("which"));
    return std::move(a);
}

static std::unique_ptr<Action> score_action(path_value x) {
    std::unique_ptr<ScoreAction> a(new ScoreAction);
    a->player = optional_admiral(x.get("player"));
    a->which  = required_int(x.get("which"));
    a->value  = required_int(x.get("value"));
    return std::move(a);
}

static std::unique_ptr<Action> select_action(path_value x) {
    std::unique_ptr<SelectAction> a(new SelectAction);
    a->screen = required_screen(x.get("screen"));
    a->line   = required_int(x.get("line"));
    return std::move(a);
}

static std::unique_ptr<Action> sound_action(path_value x) {
    std::unique_ptr<SoundAction> a(new SoundAction);
    a->priority    = required_int(x.get("priority"));
    a->persistence = required_ticks(x.get("persistence"));
    a->absolute    = optional_bool(x.get("absolute")).value_or(false);
    a->volume      = required_int(x.get("volume"));
    a->id          = required_int_range(x.get("id"));
    return std::move(a);
}

static std::unique_ptr<Action> spark_action(path_value x) {
    std::unique_ptr<SparkAction> a(new SparkAction);
    a->count    = required_int(x.get("count"));
    a->hue      = required_hue(x.get("hue"));
    a->decay    = required_int(x.get("decay"));
    a->velocity = required_fixed(x.get("velocity"));
    return std::move(a);
}

static std::unique_ptr<Action> spin_action(path_value x) {
    std::unique_ptr<SpinAction> a(new SpinAction);
    a->value = required_fixed_range(x.get("value"));
    return std::move(a);
}

static std::unique_ptr<Action> thrust_action(path_value x) {
    std::unique_ptr<ThrustAction> a(new ThrustAction);
    a->relative = optional_bool(x.get("relative"));
    a->value    = required_fixed_range(x.get("value"));
    a->relative = false;  // TODO(sfiera): verify that this is correct.
    return std::move(a);
}

static std::unique_ptr<Action> warp_action(path_value x) {
    return std::unique_ptr<WarpAction>(new WarpAction);
}

static std::unique_ptr<Action> win_action(path_value x) {
    std::unique_ptr<WinAction> a(new WinAction);
    a->player = optional_admiral(x.get("player"));
    a->next   = optional_level(x.get("next"));
    a->text   = required_string(x.get("text")).copy();
    return std::move(a);
}

static std::unique_ptr<Action> zoom_action(path_value x) {
    std::unique_ptr<ZoomAction> a(new ZoomAction);
    a->value = required_zoom(x.get("value"));
    return std::move(a);
}

template <typename T>
T* init(std::unique_ptr<Action>* action) {
    T* t;
    action->reset(t = new T());
    return t;
}

template <typename T>
Owner owner_cast(T t) {
    switch (t) {
        case static_cast<T>(Owner::ANY): return Owner::ANY;
        case static_cast<T>(Owner::SAME): return Owner::SAME;
        case static_cast<T>(Owner::DIFFERENT): return Owner::DIFFERENT;
    }
    return owner_cast(Owner::ANY);
}

std::unique_ptr<Action> action(path_value x) {
    if (!x.value().is_map()) {
        throw std::runtime_error(pn::format("{0}: must be map", x.path()).c_str());
    }

    pn::string_view         type = required_string(x.get("type"));
    std::unique_ptr<Action> a;
    if (type == "age") {
        a = age_action(x);
    } else if (type == "assume") {
        a = assume_action(x);
    } else if (type == "cap-speed") {
        a = cap_speed_action(x);
    } else if (type == "capture") {
        a = capture_action(x);
    } else if (type == "cloak") {
        a = cloak_action(x);
    } else if (type == "condition") {
        a = condition_action(x);
    } else if (type == "create") {
        a = create_action(x);
    } else if (type == "disable") {
        a = disable_action(x);
    } else if (type == "energize") {
        a = energize_action(x);
    } else if (type == "equip") {
        a = equip_action(x);
    } else if (type == "fire") {
        a = fire_action(x);
    } else if (type == "flash") {
        a = flash_action(x);
    } else if (type == "heal") {
        a = heal_action(x);
    } else if (type == "hold") {
        a = hold_action(x);
    } else if (type == "key") {
        a = key_action(x);
    } else if (type == "kill") {
        a = kill_action(x);
    } else if (type == "land") {
        a = land_action(x);
    } else if (type == "message") {
        a = message_action(x);
    } else if (type == "morph") {
        a = morph_action(x);
    } else if (type == "move") {
        a = move_action(x);
    } else if (type == "occupy") {
        a = occupy_action(x);
    } else if (type == "order") {
        a = order_action(x);
    } else if (type == "pay") {
        a = pay_action(x);
    } else if (type == "push") {
        a = push_action(x);
    } else if (type == "reveal") {
        a = reveal_action(x);
    } else if (type == "score") {
        a = score_action(x);
    } else if (type == "select") {
        a = select_action(x);
    } else if (type == "sound") {
        a = sound_action(x);
    } else if (type == "spark") {
        a = spark_action(x);
    } else if (type == "spin") {
        a = spin_action(x);
    } else if (type == "thrust") {
        a = thrust_action(x);
    } else if (type == "warp") {
        a = warp_action(x);
    } else if (type == "win") {
        a = win_action(x);
    } else if (type == "zoom") {
        a = zoom_action(x);
    } else {
        throw std::runtime_error(pn::format("unknown type: {0}", type).c_str());
    }

    a->reflexive = optional_bool(x.get("reflexive")).value_or(false);

    a->inclusive_filter = optional_object_attributes(x.get("inclusive_filter")).value_or(0);
    a->level_key_tag    = optional_string(x.get("level_key_filter")).value_or("").copy();

    a->owner = optional_owner(x.get("owner")).value_or(Owner::ANY);
    a->delay = optional_ticks(x.get("delay")).value_or(ticks(0));

    a->initialSubjectOverride =
            optional_initial(x.get("initial_subject")).value_or(Level::Initial::none());
    a->initialDirectOverride =
            optional_initial(x.get("initial_object")).value_or(Level::Initial::none());
    return a;
}

std::unique_ptr<Action> action(pn::value_cref x0) {
    if (x0.is_null()) {
        return std::unique_ptr<Action>(new NoAction);
    } else if (!x0.is_map()) {
        throw std::runtime_error("must be null or map");
    }
    return action(path_value{x0});
}

}  // namespace

std::vector<std::unique_ptr<const Action>> read_actions(int begin, int end) {
    if (end <= begin) {
        return std::vector<std::unique_ptr<const Action>>{};
    }

    std::vector<std::unique_ptr<const Action>> actions;
    actions.resize(end - begin);
    for (int i : sfz::range(begin, end)) {
        pn::string path = pn::format("actions/{0}.pn", i);
        try {
            Resource   r = Resource::path(path);
            pn::value  x;
            pn_error_t e;
            if (!pn::parse(r.data().open(), x, &e)) {
                throw std::runtime_error(
                        pn::format("{1}:{2}: {3}", e.lineno, e.column, pn_strerror(e.code))
                                .c_str());
            }
            actions[i - begin] = action(x);
        } catch (...) {
            std::throw_with_nested(std::runtime_error(path.c_str()));
        }
    }
    return actions;
}

std::vector<std::unique_ptr<const Action>> required_action_array(path_value x) {
    if (x.value().is_array()) {
        std::vector<std::unique_ptr<const Action>> a;
        for (int i = 0; i < x.value().as_array().size(); ++i) {
            a.push_back(action(x.get(i)));
        }
        return a;
    } else {
        throw std::runtime_error(pn::format("{0}: must be array", x.path()).c_str());
    }
}

Handle<BaseObject> Action::created_base() const { return BaseObject::none(); }
Range<int64_t>     Action::sound_range() const { return Range<int64_t>::empty(); }
bool               Action::alters_owner() const { return false; }
bool               Action::check_conditions() const { return false; }

Handle<BaseObject> CreateAction::created_base() const { return base; }
Handle<BaseObject> MorphAction::created_base() const { return base; }
Handle<BaseObject> EquipAction::created_base() const { return base; }

Range<int64_t> SoundAction::sound_range() const { return id; }

bool CaptureAction::alters_owner() const { return true; }

bool ScoreAction::check_conditions() const { return true; }
bool MessageAction::check_conditions() const { return true; }

}  // namespace antares

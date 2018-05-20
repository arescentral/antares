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
#include "data/initial.hpp"
#include "data/level.hpp"
#include "data/resource.hpp"

namespace antares {

// clang-format off
#define COMMON_ACTION_FIELDS                                                                      \
            {"type", {&ActionBase::type, required_action_type}},                                  \
            {"reflexive", {&ActionBase::reflexive, optional_bool, false}},                        \
            {"if", {&ActionBase::filter, optional_action_filter}},                                \
            {"delay", {&ActionBase::delay, optional_ticks, ticks(0)}},                            \
            {"override", {&ActionBase::override_, optional_action_override}}
// clang-format on

ActionType Action::type() const { return base.type; }

Action::Action(AgeAction a) : age(std::move(a)) {}
Action::Action(AssumeAction a) : assume(std::move(a)) {}
Action::Action(CapSpeedAction a) : cap_speed(std::move(a)) {}
Action::Action(CaptureAction a) : capture(std::move(a)) {}
Action::Action(CloakAction a) : cloak(std::move(a)) {}
Action::Action(ConditionAction a) : condition(std::move(a)) {}
Action::Action(CreateAction a) : create(std::move(a)) {}
Action::Action(DisableAction a) : disable(std::move(a)) {}
Action::Action(EnergizeAction a) : energize(std::move(a)) {}
Action::Action(EquipAction a) : equip(std::move(a)) {}
Action::Action(FireAction a) : fire(std::move(a)) {}
Action::Action(FlashAction a) : flash(std::move(a)) {}
Action::Action(HealAction a) : heal(std::move(a)) {}
Action::Action(HoldPositionAction a) : hold_position(std::move(a)) {}
Action::Action(KeyAction a) : key(std::move(a)) {}
Action::Action(KillAction a) : kill(std::move(a)) {}
Action::Action(LandAction a) : land(std::move(a)) {}
Action::Action(MessageAction a) : message(std::move(a)) {}
Action::Action(MorphAction a) : morph(std::move(a)) {}
Action::Action(MoveAction a) : move(std::move(a)) {}
Action::Action(OccupyAction a) : occupy(std::move(a)) {}
Action::Action(OrderAction a) : order(std::move(a)) {}
Action::Action(PayAction a) : pay(std::move(a)) {}
Action::Action(PushAction a) : push(std::move(a)) {}
Action::Action(RevealAction a) : reveal(std::move(a)) {}
Action::Action(ScoreAction a) : score(std::move(a)) {}
Action::Action(SelectAction a) : select(std::move(a)) {}
Action::Action(PlayAction a) : play(std::move(a)) {}
Action::Action(SparkAction a) : spark(std::move(a)) {}
Action::Action(SpinAction a) : spin(std::move(a)) {}
Action::Action(ThrustAction a) : thrust(std::move(a)) {}
Action::Action(WarpAction a) : warp(std::move(a)) {}
Action::Action(WinAction a) : win(std::move(a)) {}
Action::Action(ZoomAction a) : zoom(std::move(a)) {}

Action::Action(Action&& a) {
    switch (a.type()) {
        case ActionType::AGE: new (this) Action(std::move(a.age)); break;
        case ActionType::ASSUME: new (this) Action(std::move(a.assume)); break;
        case ActionType::CAP_SPEED: new (this) Action(std::move(a.cap_speed)); break;
        case ActionType::CAPTURE: new (this) Action(std::move(a.capture)); break;
        case ActionType::CLOAK: new (this) Action(std::move(a.cloak)); break;
        case ActionType::CONDITION: new (this) Action(std::move(a.condition)); break;
        case ActionType::CREATE: new (this) Action(std::move(a.create)); break;
        case ActionType::DISABLE: new (this) Action(std::move(a.disable)); break;
        case ActionType::ENERGIZE: new (this) Action(std::move(a.energize)); break;
        case ActionType::EQUIP: new (this) Action(std::move(a.equip)); break;
        case ActionType::FIRE: new (this) Action(std::move(a.fire)); break;
        case ActionType::FLASH: new (this) Action(std::move(a.flash)); break;
        case ActionType::HEAL: new (this) Action(std::move(a.heal)); break;
        case ActionType::HOLD: new (this) Action(std::move(a.hold_position)); break;
        case ActionType::KEY: new (this) Action(std::move(a.key)); break;
        case ActionType::KILL: new (this) Action(std::move(a.kill)); break;
        case ActionType::LAND: new (this) Action(std::move(a.land)); break;
        case ActionType::MESSAGE: new (this) Action(std::move(a.message)); break;
        case ActionType::MORPH: new (this) Action(std::move(a.morph)); break;
        case ActionType::MOVE: new (this) Action(std::move(a.move)); break;
        case ActionType::OCCUPY: new (this) Action(std::move(a.occupy)); break;
        case ActionType::ORDER: new (this) Action(std::move(a.order)); break;
        case ActionType::PAY: new (this) Action(std::move(a.pay)); break;
        case ActionType::PUSH: new (this) Action(std::move(a.push)); break;
        case ActionType::REVEAL: new (this) Action(std::move(a.reveal)); break;
        case ActionType::SCORE: new (this) Action(std::move(a.score)); break;
        case ActionType::SELECT: new (this) Action(std::move(a.select)); break;
        case ActionType::PLAY: new (this) Action(std::move(a.play)); break;
        case ActionType::SPARK: new (this) Action(std::move(a.spark)); break;
        case ActionType::SPIN: new (this) Action(std::move(a.spin)); break;
        case ActionType::THRUST: new (this) Action(std::move(a.thrust)); break;
        case ActionType::WARP: new (this) Action(std::move(a.warp)); break;
        case ActionType::WIN: new (this) Action(std::move(a.win)); break;
        case ActionType::ZOOM: new (this) Action(std::move(a.zoom)); break;
    }
}

Action& Action::operator=(Action&& a) {
    this->~Action();
    new (this) Action(std::move(a));
    return *this;
}

Action::~Action() {
    switch (type()) {
        case ActionType::AGE: age.~AgeAction(); break;
        case ActionType::ASSUME: assume.~AssumeAction(); break;
        case ActionType::CAP_SPEED: cap_speed.~CapSpeedAction(); break;
        case ActionType::CAPTURE: capture.~CaptureAction(); break;
        case ActionType::CLOAK: cloak.~CloakAction(); break;
        case ActionType::CONDITION: condition.~ConditionAction(); break;
        case ActionType::CREATE: create.~CreateAction(); break;
        case ActionType::DISABLE: disable.~DisableAction(); break;
        case ActionType::ENERGIZE: energize.~EnergizeAction(); break;
        case ActionType::EQUIP: equip.~EquipAction(); break;
        case ActionType::FIRE: fire.~FireAction(); break;
        case ActionType::FLASH: flash.~FlashAction(); break;
        case ActionType::HEAL: heal.~HealAction(); break;
        case ActionType::HOLD: hold_position.~HoldPositionAction(); break;
        case ActionType::KEY: key.~KeyAction(); break;
        case ActionType::KILL: kill.~KillAction(); break;
        case ActionType::LAND: land.~LandAction(); break;
        case ActionType::MESSAGE: message.~MessageAction(); break;
        case ActionType::MORPH: morph.~MorphAction(); break;
        case ActionType::MOVE: move.~MoveAction(); break;
        case ActionType::OCCUPY: occupy.~OccupyAction(); break;
        case ActionType::ORDER: order.~OrderAction(); break;
        case ActionType::PAY: pay.~PayAction(); break;
        case ActionType::PUSH: push.~PushAction(); break;
        case ActionType::REVEAL: reveal.~RevealAction(); break;
        case ActionType::SCORE: score.~ScoreAction(); break;
        case ActionType::SELECT: select.~SelectAction(); break;
        case ActionType::PLAY: play.~PlayAction(); break;
        case ActionType::SPARK: spark.~SparkAction(); break;
        case ActionType::SPIN: spin.~SpinAction(); break;
        case ActionType::THRUST: thrust.~ThrustAction(); break;
        case ActionType::WARP: warp.~WarpAction(); break;
        case ActionType::WIN: win.~WinAction(); break;
        case ActionType::ZOOM: zoom.~ZoomAction(); break;
    }
}

static uint32_t optional_flags(path_value x, const std::map<pn::string_view, int>& flags) {
    if (x.value().is_null()) {
        return 0;
    } else if (x.value().is_map()) {
        uint32_t result = 0;
        for (auto kv : flags) {
            if (optional_bool(x.get(kv.first)).value_or(false)) {
                result |= 1 << kv.second;
            }
        }
        for (auto kv : x.value().as_map()) {
            if (flags.find(kv.key()) == flags.end()) {
                path_value v = x.get(kv.key());
                throw std::runtime_error(pn::format("{0}unknown flag", v.prefix()).c_str());
            }
        }
        return result;
    } else {
        throw std::runtime_error(pn::format("{0}must be null or map", x.prefix()).c_str());
    }
}

static uint32_t optional_object_attributes(path_value x) {
    return optional_flags(
            x, {{"can_be_engaged", 1},
                {"does_bounce", 6},
                {"can_be_destination", 10},
                {"can_engage", 11},
                {"can_evade", 12},
                {"can_accept_build", 14},
                {"can_accept_destination", 15},
                {"autotarget", 16},
                {"animation_cycle", 17},
                {"can_collide", 18},
                {"can_be_hit", 19},
                {"is_destination", 20},
                {"hide_effect", 21},
                {"release_energy_on_death", 22},
                {"hated", 23},
                {"occupies_space", 24},
                {"static_destination", 25},
                {"can_be_evaded", 26},
                {"neutral_death", 27},
                {"is_guided", 28},
                {"appear_on_radar", 29}});
}

static std::map<pn::string, bool> optional_tags(path_value x) {
    if (x.value().is_null()) {
        return {};
    } else if (x.value().is_map()) {
        pn::map_cref               m = x.value().as_map();
        std::map<pn::string, bool> result;
        for (const auto& kv : m) {
            auto v = optional_bool(x.get(kv.key()));
            if (v.has_value()) {
                result[kv.key().copy()] = *v;
            }
        }
        return result;
    } else {
        throw std::runtime_error(pn::format("{0}: must be null or map", x.path()).c_str());
    }
}

static ActionBase::Filter optional_action_filter(path_value x) {
    return optional_struct<ActionBase::Filter>(
                   x,
                   {{"attributes", {&ActionBase::Filter::attributes, optional_object_attributes}},
                    {"tags", {&ActionBase::Filter::tags, optional_tags}},
                    {"owner", {&ActionBase::Filter::owner, optional_owner, Owner::ANY}}})
            .value_or(ActionBase::Filter{});
}

static sfz::optional<Handle<const Initial>> optional_initial_override(path_value x) {
    if (x.value().is_null()) {
        return sfz::nullopt;
    } else if (x.value().is_map()) {
        struct InitialNumber {
            int64_t number;
        };
        auto i = required_struct<InitialNumber>(
                x, {{"initial", {&InitialNumber::number, required_int}}});
        return sfz::make_optional(Handle<const Initial>(i.number));
    } else if (x.value().as_string() == "player") {
        return sfz::make_optional(Handle<const Initial>(-2));
    } else {
        throw std::runtime_error(
                pn::format("{0}: must be null, map, or \"player\"", x.path()).c_str());
    }
}

static ActionBase::Override optional_action_override(path_value x) {
    return optional_struct<ActionBase::Override>(
                   x, {{"subject", {&ActionBase::Override::subject, optional_initial_override}},
                       {"object", {&ActionBase::Override::object, optional_initial_override}}})
            .value_or(ActionBase::Override{});
}

static Action age_action(path_value x) {
    return required_struct<AgeAction>(
            x, {COMMON_ACTION_FIELDS,
                {"relative", {&AgeAction::relative, optional_bool, false}},
                {"value", {&AgeAction::value, required_ticks_range}}});
}

static Action assume_action(path_value x) {
    return required_struct<AssumeAction>(
            x, {COMMON_ACTION_FIELDS, {"which", {&AssumeAction::which, required_int}}});
}

static Action cap_speed_action(path_value x) {
    return required_struct<CapSpeedAction>(
            x, {COMMON_ACTION_FIELDS, {"value", {&CapSpeedAction::value, optional_fixed}}});
}

static Action capture_action(path_value x) {
    return required_struct<CaptureAction>(
            x, {COMMON_ACTION_FIELDS, {"player", {&CaptureAction::player, optional_admiral}}});
}

static Action cloak_action(path_value x) {
    return required_struct<CloakAction>(x, {COMMON_ACTION_FIELDS});
}

static Action condition_action(path_value x) {
    return required_struct<ConditionAction>(
            x, {COMMON_ACTION_FIELDS,
                {"enable", {&ConditionAction::enable, optional_condition_range}},
                {"disable", {&ConditionAction::disable, optional_condition_range}}});
}

static Action create_action(path_value x) {
    return required_struct<CreateAction>(
            x, {COMMON_ACTION_FIELDS,
                {"base", {&CreateAction::base, required_base}},
                {"count", {&CreateAction::count, optional_int_range, Range<int64_t>{1, 2}}},
                {"relative_velocity", {&CreateAction::relative_velocity, optional_bool, false}},
                {"relative_direction", {&CreateAction::relative_direction, optional_bool, false}},
                {"distance", {&CreateAction::distance, optional_int, 0}},
                {"inherit", {&CreateAction::inherit, optional_bool, false}},
                {"legacy_random", {&CreateAction::legacy_random, optional_bool, false}}});
}

static Action disable_action(path_value x) {
    return required_struct<DisableAction>(
            x, {COMMON_ACTION_FIELDS, {"value", {&DisableAction::value, required_fixed_range}}});
}

static Action energize_action(path_value x) {
    return required_struct<EnergizeAction>(
            x, {COMMON_ACTION_FIELDS, {"value", {&EnergizeAction::value, required_int}}});
}

static Action equip_action(path_value x) {
    return required_struct<EquipAction>(
            x, {COMMON_ACTION_FIELDS,
                {"which", {&EquipAction::which, required_weapon}},
                {"base", {&EquipAction::base, required_base}}});
}

static Action fire_action(path_value x) {
    return required_struct<FireAction>(
            x, {COMMON_ACTION_FIELDS, {"which", {&FireAction::which, required_weapon}}});
}

static uint8_t required_shade(path_value x) { return required_int(x, {1, 17}); }

static Action flash_action(path_value x) {
    return required_struct<FlashAction>(
            x, {COMMON_ACTION_FIELDS,
                {"length", {&FlashAction::length, required_int}},
                {"hue", {&FlashAction::hue, required_hue}},
                {"shade", {&FlashAction::shade, required_shade}}});
}

static Action heal_action(path_value x) {
    return required_struct<HealAction>(
            x, {COMMON_ACTION_FIELDS, {"value", {&HealAction::value, required_int}}});
}

static Action hold_action(path_value x) {
    return required_struct<HoldPositionAction>(x, {COMMON_ACTION_FIELDS});
}

static Action key_action(path_value x) {
    return required_struct<KeyAction>(
            x, {COMMON_ACTION_FIELDS,
                {"enable", {&KeyAction::enable, optional_keys}},
                {"disable", {&KeyAction::disable, optional_keys}}});
}

static Action kill_action(path_value x) {
    return required_struct<KillAction>(
            x, {COMMON_ACTION_FIELDS, {"kind", {&KillAction::kind, required_kill_kind}}});
}

static Action land_action(path_value x) {
    return required_struct<LandAction>(
            x, {COMMON_ACTION_FIELDS, {"speed", {&LandAction::speed, required_int}}});
}

static Action message_action(path_value x) {
    return required_struct<MessageAction>(
            x, {COMMON_ACTION_FIELDS,
                {"id", {&MessageAction::id, required_int}},
                {"pages",
                 {&MessageAction::pages, required_array<pn::string, required_string_copy>}}});
}

static Action morph_action(path_value x) {
    return required_struct<MorphAction>(
            x, {COMMON_ACTION_FIELDS,
                {"base", {&MorphAction::base, required_base}},
                {"keep_ammo", {&MorphAction::keep_ammo, optional_bool, false}}});
}

static sfz::optional<coordPointType> optional_coord_point(path_value x) {
    sfz::optional<Point> p = optional_point(x);
    if (!p.has_value()) {
        return sfz::nullopt;
    }
    return sfz::make_optional(coordPointType{(uint32_t)p->h, (uint32_t)p->v});
}

static Action move_action(path_value x) {
    return required_struct<MoveAction>(
            x, {COMMON_ACTION_FIELDS,
                {"origin", {&MoveAction::origin, optional_origin, MoveOrigin::LEVEL}},
                {"to", {&MoveAction::to, optional_coord_point, coordPointType{0, 0}}},
                {"distance", {&MoveAction::distance, optional_int, 0}}});
}

static Action occupy_action(path_value x) {
    return required_struct<OccupyAction>(
            x, {COMMON_ACTION_FIELDS, {"value", {&OccupyAction::value, required_int}}});
}

static Action order_action(path_value x) {
    return required_struct<OrderAction>(x, {COMMON_ACTION_FIELDS});
}

static Action pay_action(path_value x) {
    return required_struct<PayAction>(
            x, {COMMON_ACTION_FIELDS,
                {"value", {&PayAction::value, required_fixed}},
                {"player", {&PayAction::player, optional_admiral}}});
}

static Action push_action(path_value x) {
    return required_struct<PushAction>(
            x, {COMMON_ACTION_FIELDS,
                {"kind", {&PushAction::kind, required_push_kind}},
                {"value", {&PushAction::value, optional_fixed, Fixed::zero()}}});
}

static Action reveal_action(path_value x) {
    return required_struct<RevealAction>(
            x,
            {COMMON_ACTION_FIELDS, {"which", {&RevealAction::initial, required_initial_range}}});
}

static Action score_action(path_value x) {
    return required_struct<ScoreAction>(
            x, {COMMON_ACTION_FIELDS,
                {"player", {&ScoreAction::player, optional_admiral}},
                {"which", {&ScoreAction::which, required_int}},
                {"value", {&ScoreAction::value, required_int}}});
}

static Action select_action(path_value x) {
    return required_struct<SelectAction>(
            x, {COMMON_ACTION_FIELDS,
                {"screen", {&SelectAction::screen, required_screen}},
                {"line", {&SelectAction::line, required_int}}});
}

static PlayAction::Sound required_sound(path_value x) {
    return required_struct<PlayAction::Sound>(
            x, {{"sound", {&PlayAction::Sound::sound, required_string_copy}}});
}

static uint8_t required_sound_priority(path_value x) { return required_int(x, {0, 6}); }

static Action play_action(path_value x) {
    return required_struct<PlayAction>(
            x, {COMMON_ACTION_FIELDS,
                {"priority", {&PlayAction::priority, required_sound_priority}},
                {"persistence", {&PlayAction::persistence, required_ticks}},
                {"absolute", {&PlayAction::absolute, optional_bool, false}},
                {"volume", {&PlayAction::volume, required_int}},
                {"sound", {&PlayAction::sound, optional_string_copy}},
                {"any", {&PlayAction::any, optional_array<PlayAction::Sound, required_sound>}}});
}

static Action spark_action(path_value x) {
    return required_struct<SparkAction>(
            x, {COMMON_ACTION_FIELDS,
                {"count", {&SparkAction::count, required_int}},
                {"hue", {&SparkAction::hue, required_hue}},
                {"decay", {&SparkAction::decay, required_int}},
                {"velocity", {&SparkAction::velocity, required_fixed}}});
}

static Action spin_action(path_value x) {
    return required_struct<SpinAction>(
            x, {COMMON_ACTION_FIELDS, {"value", {&SpinAction::value, required_fixed_range}}});
}

static Action thrust_action(path_value x) {
    return required_struct<ThrustAction>(
            x, {COMMON_ACTION_FIELDS,
                {"value", {&ThrustAction::value, required_fixed_range}},
                {"relative", nullptr}});
}

static Action warp_action(path_value x) {
    return required_struct<WarpAction>(x, {COMMON_ACTION_FIELDS});
}

static Action win_action(path_value x) {
    return required_struct<WinAction>(
            x, {COMMON_ACTION_FIELDS,
                {"player", {&WinAction::player, optional_admiral}},
                {"next", {&WinAction::next, optional_level}},
                {"text", {&WinAction::text, required_string_copy}}});
}

static Action zoom_action(path_value x) {
    return required_struct<ZoomAction>(
            x, {COMMON_ACTION_FIELDS, {"value", {&ZoomAction::value, required_zoom}}});
}

Action action(path_value x) {
    if (!x.value().is_map()) {
        throw std::runtime_error(pn::format("{0}: must be map", x.path()).c_str());
    }

    switch (required_action_type(x.get("type"))) {
        case ActionType::AGE: return age_action(x);
        case ActionType::ASSUME: return assume_action(x);
        case ActionType::CAP_SPEED: return cap_speed_action(x);
        case ActionType::CAPTURE: return capture_action(x);
        case ActionType::CLOAK: return cloak_action(x);
        case ActionType::CONDITION: return condition_action(x);
        case ActionType::CREATE: return create_action(x);
        case ActionType::DISABLE: return disable_action(x);
        case ActionType::ENERGIZE: return energize_action(x);
        case ActionType::EQUIP: return equip_action(x);
        case ActionType::FIRE: return fire_action(x);
        case ActionType::FLASH: return flash_action(x);
        case ActionType::HEAL: return heal_action(x);
        case ActionType::HOLD: return hold_action(x);
        case ActionType::KEY: return key_action(x);
        case ActionType::KILL: return kill_action(x);
        case ActionType::LAND: return land_action(x);
        case ActionType::MESSAGE: return message_action(x);
        case ActionType::MORPH: return morph_action(x);
        case ActionType::MOVE: return move_action(x);
        case ActionType::OCCUPY: return occupy_action(x);
        case ActionType::ORDER: return order_action(x);
        case ActionType::PAY: return pay_action(x);
        case ActionType::PUSH: return push_action(x);
        case ActionType::REVEAL: return reveal_action(x);
        case ActionType::SCORE: return score_action(x);
        case ActionType::SELECT: return select_action(x);
        case ActionType::PLAY: return play_action(x);
        case ActionType::SPARK: return spark_action(x);
        case ActionType::SPIN: return spin_action(x);
        case ActionType::THRUST: return thrust_action(x);
        case ActionType::WARP: return warp_action(x);
        case ActionType::WIN: return win_action(x);
        case ActionType::ZOOM: return zoom_action(x);
    }
}

}  // namespace antares

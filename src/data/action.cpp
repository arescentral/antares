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
            {"type", nullptr},                                                                    \
            {"reflexive", {&Action::reflexive, optional_bool, false}},                            \
            {"if", {&Action::filter, optional_action_filter}},                                    \
            {"delay", {&Action::delay, optional_ticks, ticks(0)}},                                \
            {"override", {&Action::override_, optional_action_override}}
// clang-format on

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
        throw std::runtime_error(pn::format("{0}: must be null or array", x.path()).c_str());
    }
}

static Action::Filter optional_action_filter(path_value x) {
    return optional_struct<Action::Filter>(
                   x, {{"attributes", {&Action::Filter::attributes, optional_object_attributes}},
                       {"tags", {&Action::Filter::tags, optional_tags}},
                       {"owner", {&Action::Filter::owner, optional_owner, Owner::ANY}}})
            .value_or(Action::Filter{});
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

static Action::Override optional_action_override(path_value x) {
    return optional_struct<Action::Override>(
                   x, {{"subject", {&Action::Override::subject, optional_initial_override}},
                       {"object", {&Action::Override::object, optional_initial_override}}})
            .value_or(Action::Override{});
}

template <typename T>
static std::unique_ptr<Action> action_ptr(T t) {
    return std::unique_ptr<Action>(new T(std::move(t)));
}

static std::unique_ptr<Action> age_action(path_value x) {
    return action_ptr(required_struct<AgeAction>(
            x, {COMMON_ACTION_FIELDS,
                {"relative", {&AgeAction::relative, optional_bool, false}},
                {"value", {&AgeAction::value, required_ticks_range}}}));
}

static std::unique_ptr<Action> assume_action(path_value x) {
    return action_ptr(required_struct<AssumeAction>(
            x, {COMMON_ACTION_FIELDS, {"which", {&AssumeAction::which, required_int}}}));
}

static std::unique_ptr<Action> cap_speed_action(path_value x) {
    return action_ptr(required_struct<CapSpeedAction>(
            x, {COMMON_ACTION_FIELDS, {"value", {&CapSpeedAction::value, optional_fixed}}}));
}

static std::unique_ptr<Action> capture_action(path_value x) {
    return action_ptr(required_struct<CaptureAction>(
            x, {COMMON_ACTION_FIELDS, {"player", {&CaptureAction::player, optional_admiral}}}));
}

static std::unique_ptr<Action> cloak_action(path_value x) {
    return action_ptr(required_struct<CloakAction>(x, {COMMON_ACTION_FIELDS}));
}

static std::unique_ptr<Action> condition_action(path_value x) {
    return action_ptr(required_struct<ConditionAction>(
            x, {COMMON_ACTION_FIELDS,
                {"enable", {&ConditionAction::enable, optional_condition_range}},
                {"disable", {&ConditionAction::disable, optional_condition_range}}}));
}

static std::unique_ptr<Action> create_action(path_value x) {
    return action_ptr(required_struct<CreateAction>(
            x, {COMMON_ACTION_FIELDS,
                {"base", {&CreateAction::base, required_base}},
                {"count", {&CreateAction::count, optional_int_range, Range<int64_t>{1, 2}}},
                {"relative_velocity", {&CreateAction::relative_velocity, optional_bool, false}},
                {"relative_direction", {&CreateAction::relative_direction, optional_bool, false}},
                {"distance", {&CreateAction::distance, optional_int, 0}},
                {"inherit", {&CreateAction::inherit, optional_bool, false}},
                {"legacy_random", {&CreateAction::legacy_random, optional_bool, false}}}));
}

static std::unique_ptr<Action> disable_action(path_value x) {
    return action_ptr(required_struct<DisableAction>(
            x, {COMMON_ACTION_FIELDS, {"value", {&DisableAction::value, required_fixed_range}}}));
}

static std::unique_ptr<Action> energize_action(path_value x) {
    return action_ptr(required_struct<EnergizeAction>(
            x, {COMMON_ACTION_FIELDS, {"value", {&EnergizeAction::value, required_int}}}));
}

static std::unique_ptr<Action> equip_action(path_value x) {
    return action_ptr(required_struct<EquipAction>(
            x, {COMMON_ACTION_FIELDS,
                {"which", {&EquipAction::which, required_weapon}},
                {"base", {&EquipAction::base, required_base}}}));
}

static std::unique_ptr<Action> fire_action(path_value x) {
    return action_ptr(required_struct<FireAction>(
            x, {COMMON_ACTION_FIELDS, {"which", {&FireAction::which, required_weapon}}}));
}

static uint8_t required_shade(path_value x) { return required_int(x, {1, 17}); }

static std::unique_ptr<Action> flash_action(path_value x) {
    return action_ptr(required_struct<FlashAction>(
            x, {COMMON_ACTION_FIELDS,
                {"length", {&FlashAction::length, required_int}},
                {"hue", {&FlashAction::hue, required_hue}},
                {"shade", {&FlashAction::shade, required_shade}}}));
}

static std::unique_ptr<Action> heal_action(path_value x) {
    return action_ptr(required_struct<HealAction>(
            x, {COMMON_ACTION_FIELDS, {"value", {&HealAction::value, required_int}}}));
}

static std::unique_ptr<Action> hold_action(path_value x) {
    return action_ptr(required_struct<HoldPositionAction>(x, {COMMON_ACTION_FIELDS}));
}

static std::unique_ptr<Action> key_action(path_value x) {
    return action_ptr(required_struct<KeyAction>(
            x, {COMMON_ACTION_FIELDS,
                {"enable", {&KeyAction::enable, optional_keys}},
                {"disable", {&KeyAction::disable, optional_keys}}}));
}

static std::unique_ptr<Action> kill_action(path_value x) {
    return action_ptr(required_struct<KillAction>(
            x, {COMMON_ACTION_FIELDS, {"kind", {&KillAction::kind, required_kill_kind}}}));
}

static std::unique_ptr<Action> land_action(path_value x) {
    return action_ptr(required_struct<LandAction>(
            x, {COMMON_ACTION_FIELDS, {"speed", {&LandAction::speed, required_int}}}));
}

static std::unique_ptr<Action> message_action(path_value x) {
    return action_ptr(required_struct<MessageAction>(
            x, {COMMON_ACTION_FIELDS,
                {"id", {&MessageAction::id, required_int}},
                {"pages", {&MessageAction::pages, required_string_array}}}));
}

static std::unique_ptr<Action> morph_action(path_value x) {
    return action_ptr(required_struct<MorphAction>(
            x, {COMMON_ACTION_FIELDS,
                {"base", {&MorphAction::base, required_base}},
                {"keep_ammo", {&MorphAction::keep_ammo, optional_bool, false}}}));
}

static sfz::optional<coordPointType> optional_coord_point(path_value x) {
    sfz::optional<Point> p = optional_point(x);
    if (!p.has_value()) {
        return sfz::nullopt;
    }
    return sfz::make_optional(coordPointType{(uint32_t)p->h, (uint32_t)p->v});
}

static std::unique_ptr<Action> move_action(path_value x) {
    return action_ptr(required_struct<MoveAction>(
            x, {COMMON_ACTION_FIELDS,
                {"origin", {&MoveAction::origin, optional_origin, MoveOrigin::LEVEL}},
                {"to", {&MoveAction::to, optional_coord_point, coordPointType{0, 0}}},
                {"distance", {&MoveAction::distance, optional_int, 0}}}));
}

static std::unique_ptr<Action> occupy_action(path_value x) {
    return action_ptr(required_struct<OccupyAction>(
            x, {COMMON_ACTION_FIELDS, {"value", {&OccupyAction::value, required_int}}}));
}

static std::unique_ptr<Action> order_action(path_value x) {
    return action_ptr(required_struct<OrderAction>(x, {COMMON_ACTION_FIELDS}));
}

static std::unique_ptr<Action> pay_action(path_value x) {
    return action_ptr(required_struct<PayAction>(
            x, {COMMON_ACTION_FIELDS,
                {"value", {&PayAction::value, required_fixed}},
                {"player", {&PayAction::player, optional_admiral}}}));
}

static std::unique_ptr<Action> push_action(path_value x) {
    return action_ptr(required_struct<PushAction>(
            x, {COMMON_ACTION_FIELDS,
                {"kind", {&PushAction::kind, required_push_kind}},
                {"value", {&PushAction::value, optional_fixed, Fixed::zero()}}}));
}

static std::unique_ptr<Action> reveal_action(path_value x) {
    return action_ptr(required_struct<RevealAction>(
            x,
            {COMMON_ACTION_FIELDS, {"which", {&RevealAction::initial, required_initial_range}}}));
}

static std::unique_ptr<Action> score_action(path_value x) {
    return action_ptr(required_struct<ScoreAction>(
            x, {COMMON_ACTION_FIELDS,
                {"player", {&ScoreAction::player, optional_admiral}},
                {"which", {&ScoreAction::which, required_int}},
                {"value", {&ScoreAction::value, required_int}}}));
}

static std::unique_ptr<Action> select_action(path_value x) {
    return action_ptr(required_struct<SelectAction>(
            x, {COMMON_ACTION_FIELDS,
                {"screen", {&SelectAction::screen, required_screen}},
                {"line", {&SelectAction::line, required_int}}}));
}

static PlayAction::Sound required_sound(path_value x) {
    return required_struct<PlayAction::Sound>(
            x, {{"sound", {&PlayAction::Sound::sound, required_string_copy}}});
}

static std::vector<PlayAction::Sound> optional_sound_list(path_value x) {
    return optional_array<PlayAction::Sound, required_sound>(x);
}

static uint8_t required_sound_priority(path_value x) { return required_int(x, {0, 6}); }

static std::unique_ptr<Action> play_action(path_value x) {
    return action_ptr(required_struct<PlayAction>(
            x, {COMMON_ACTION_FIELDS,
                {"priority", {&PlayAction::priority, required_sound_priority}},
                {"persistence", {&PlayAction::persistence, required_ticks}},
                {"absolute", {&PlayAction::absolute, optional_bool, false}},
                {"volume", {&PlayAction::volume, required_int}},
                {"sound", {&PlayAction::sound, optional_string_copy}},
                {"any", {&PlayAction::any, optional_sound_list}}}));
}

static std::unique_ptr<Action> spark_action(path_value x) {
    return action_ptr(required_struct<SparkAction>(
            x, {COMMON_ACTION_FIELDS,
                {"count", {&SparkAction::count, required_int}},
                {"hue", {&SparkAction::hue, required_hue}},
                {"decay", {&SparkAction::decay, required_int}},
                {"velocity", {&SparkAction::velocity, required_fixed}}}));
}

static std::unique_ptr<Action> spin_action(path_value x) {
    return action_ptr(required_struct<SpinAction>(
            x, {COMMON_ACTION_FIELDS, {"value", {&SpinAction::value, required_fixed_range}}}));
}

static std::unique_ptr<Action> thrust_action(path_value x) {
    return action_ptr(required_struct<ThrustAction>(
            x, {COMMON_ACTION_FIELDS,
                {"value", {&ThrustAction::value, required_fixed_range}},
                {"relative", nullptr}}));
}

static std::unique_ptr<Action> warp_action(path_value x) {
    return action_ptr(required_struct<WarpAction>(x, {COMMON_ACTION_FIELDS}));
}

static std::unique_ptr<Action> win_action(path_value x) {
    return action_ptr(required_struct<WinAction>(
            x, {COMMON_ACTION_FIELDS,
                {"player", {&WinAction::player, optional_admiral}},
                {"next", {&WinAction::next, optional_level}},
                {"text", {&WinAction::text, required_string_copy}}}));
}

static std::unique_ptr<Action> zoom_action(path_value x) {
    return action_ptr(required_struct<ZoomAction>(
            x, {COMMON_ACTION_FIELDS, {"value", {&ZoomAction::value, required_zoom}}}));
}

std::unique_ptr<const Action> action(path_value x) {
    if (!x.value().is_map()) {
        throw std::runtime_error(pn::format("{0}: must be map", x.path()).c_str());
    }

    pn::string_view         type = required_string(x.get("type"));
    std::unique_ptr<Action> a;
    if (type == "age") {
        return age_action(x);
    } else if (type == "assume") {
        return assume_action(x);
    } else if (type == "cap-speed") {
        return cap_speed_action(x);
    } else if (type == "capture") {
        return capture_action(x);
    } else if (type == "cloak") {
        return cloak_action(x);
    } else if (type == "condition") {
        return condition_action(x);
    } else if (type == "create") {
        return create_action(x);
    } else if (type == "disable") {
        return disable_action(x);
    } else if (type == "energize") {
        return energize_action(x);
    } else if (type == "equip") {
        return equip_action(x);
    } else if (type == "fire") {
        return fire_action(x);
    } else if (type == "flash") {
        return flash_action(x);
    } else if (type == "heal") {
        return heal_action(x);
    } else if (type == "hold") {
        return hold_action(x);
    } else if (type == "key") {
        return key_action(x);
    } else if (type == "kill") {
        return kill_action(x);
    } else if (type == "land") {
        return land_action(x);
    } else if (type == "message") {
        return message_action(x);
    } else if (type == "morph") {
        return morph_action(x);
    } else if (type == "move") {
        return move_action(x);
    } else if (type == "occupy") {
        return occupy_action(x);
    } else if (type == "order") {
        return order_action(x);
    } else if (type == "pay") {
        return pay_action(x);
    } else if (type == "play") {
        return play_action(x);
    } else if (type == "push") {
        return push_action(x);
    } else if (type == "reveal") {
        return reveal_action(x);
    } else if (type == "score") {
        return score_action(x);
    } else if (type == "select") {
        return select_action(x);
    } else if (type == "spark") {
        return spark_action(x);
    } else if (type == "spin") {
        return spin_action(x);
    } else if (type == "thrust") {
        return thrust_action(x);
    } else if (type == "warp") {
        return warp_action(x);
    } else if (type == "win") {
        return win_action(x);
    } else if (type == "zoom") {
        return zoom_action(x);
    } else {
        throw std::runtime_error(pn::format("unknown type: {0}", type).c_str());
    }
}

const NamedHandle<const BaseObject>* Action::created_base() const { return nullptr; }
std::vector<pn::string>              Action::sound_ids() const { return {}; }
bool                                 Action::alters_owner() const { return false; }
bool                                 Action::check_conditions() const { return false; }

const NamedHandle<const BaseObject>* CreateAction::created_base() const { return &base; }
const NamedHandle<const BaseObject>* MorphAction::created_base() const { return &base; }
const NamedHandle<const BaseObject>* EquipAction::created_base() const { return &base; }

std::vector<pn::string> PlayAction::sound_ids() const {
    std::vector<pn::string> result;
    if (sound.has_value()) {
        result.push_back(sound->copy());
    } else {
        for (auto& s : any) {
            result.push_back(s.sound.copy());
        }
    }
    return result;
}

bool CaptureAction::alters_owner() const { return true; }

bool ScoreAction::check_conditions() const { return true; }
bool MessageAction::check_conditions() const { return true; }

}  // namespace antares

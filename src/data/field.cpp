// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// Copyright (C) 2018 The Antares Authors
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

#include "data/field.hpp"

#include "data/level.hpp"

namespace antares {

pn::string path_value::path() const {
    if (_parent && (_parent->_kind != Kind::ROOT)) {
        if (_kind == Kind::KEY) {
            return pn::format("{0}.{1}", _parent->path(), _key);
        } else {
            return pn::format("{0}[{1}]", _parent->path(), _index);
        }
    } else {
        if (_kind == Kind::KEY) {
            return _key.copy();
        } else {
            return pn::format("[{0}]", _index);
        }
    }
}

pn::value_cref path_value::array_get(pn::array_cref a, int64_t index) {
    if ((0 <= index) && (index < a.size())) {
        return a[index];
    } else {
        return pn::value_cref{&pn_null};
    }
}

sfz::optional<bool> optional_bool(path_value x) {
    if (x.value().is_null()) {
        return sfz::nullopt;
    } else if (x.value().is_bool()) {
        return sfz::make_optional(x.value().as_bool());
    } else {
        throw std::runtime_error(pn::format("{0}: must be null or bool", x.path()).c_str());
    }
}

bool required_bool(path_value x) {
    if (x.value().is_bool()) {
        return x.value().as_bool();
    } else {
        throw std::runtime_error(pn::format("{0}: must be bool", x.path()).c_str());
    }
}

sfz::optional<int64_t> optional_int(path_value x) {
    if (x.value().is_null()) {
        return sfz::nullopt;
    } else if (x.value().is_int()) {
        return sfz::make_optional(x.value().as_int());
    } else {
        throw std::runtime_error(pn::format("{0}: must be null or int", x.path()).c_str());
    }
}

int64_t required_int(path_value x) {
    if (x.value().is_int()) {
        return x.value().as_int();
    } else {
        throw std::runtime_error(pn::format("{0}: must be int", x.path()).c_str());
    }
}

double required_double(path_value x) {
    if (x.value().is_float()) {
        return x.value().as_float();
    } else {
        throw std::runtime_error(pn::format("{0}: must be float", x.path()).c_str());
    }
}

sfz::optional<Fixed> optional_fixed(path_value x) {
    if (x.value().is_null()) {
        return sfz::nullopt;
    } else if (x.value().is_float()) {
        return sfz::make_optional(Fixed::from_float(x.value().as_float()));
    } else {
        throw std::runtime_error(pn::format("{0}: must be float", x.path()).c_str());
    }
}

Fixed required_fixed(path_value x) {
    if (x.value().is_float()) {
        return Fixed::from_float(x.value().as_float());
    } else {
        throw std::runtime_error(pn::format("{0}: must be float", x.path()).c_str());
    }
}

sfz::optional<pn::string_view> optional_string(path_value x) {
    if (x.value().is_null()) {
        return sfz::nullopt;
    } else if (x.value().is_string()) {
        return sfz::make_optional(x.value().as_string());
    } else {
        throw std::runtime_error(pn::format("{0}: must be string", x.path()).c_str());
    }
}

pn::string_view required_string(path_value x) {
    if (x.value().is_string()) {
        return x.value().as_string();
    } else {
        throw std::runtime_error(pn::format("{0}: must be string", x.path()).c_str());
    }
}

template <typename T, int N>
sfz::optional<T> optional_enum(path_value x, const std::pair<pn::string_view, T> (&values)[N]) {
    if (x.value().is_null()) {
        return sfz::nullopt;
    } else if (x.value().is_string()) {
        pn::string_view s = x.value().as_string();
        for (auto kv : values) {
            if (s == kv.first) {
                sfz::optional<T> t;
                t.emplace(kv.second);
                return t;
            }
        }
    }

    pn::array keys;
    for (auto kv : values) {
        keys.push_back(kv.first.copy());
    }
    throw std::runtime_error(pn::format("{0}: must be one of {1}", x.path(), keys).c_str());
}

sfz::optional<ticks> optional_ticks(path_value x) {
    sfz::optional<int64_t> i = optional_int(x);
    if (i.has_value()) {
        return sfz::make_optional(ticks(*i));
    } else {
        return sfz::nullopt;
    }
}

ticks required_ticks(path_value x) { return ticks(required_int(x)); }

sfz::optional<secs> optional_secs(path_value x) {
    sfz::optional<int64_t> i = optional_int(x);
    if (i.has_value()) {
        return sfz::make_optional(secs(*i));
    } else {
        return sfz::nullopt;
    }
}

sfz::optional<Handle<Admiral>> optional_admiral(path_value x) {
    sfz::optional<int64_t> i = optional_int(x);
    if (i.has_value()) {
        return sfz::make_optional(Handle<Admiral>(*i));
    } else {
        return sfz::nullopt;
    }
}

Handle<Admiral> required_admiral(path_value x) { return Handle<Admiral>(required_int(x)); }

Handle<BaseObject> required_base(path_value x) { return Handle<BaseObject>(required_int(x)); }

sfz::optional<Handle<Level::Initial>> optional_initial(path_value x) {
    if (x.value().is_null()) {
        return sfz::nullopt;
    } else if (x.value().is_int()) {
        return sfz::make_optional(Handle<Level::Initial>(x.value().as_int()));
    } else if (x.value().as_string() == "player") {
        return sfz::make_optional(Handle<Level::Initial>(-2));
    } else {
        throw std::runtime_error(
                pn::format("{0}: must be null, int, or \"player\"", x.path()).c_str());
    }
}

Handle<Level::Initial> required_initial(path_value x) {
    if (x.value().is_int()) {
        return Handle<Level::Initial>(x.value().as_int());
    } else if (x.value().as_string() == "player") {
        return Handle<Level::Initial>(-2);
    } else {
        throw std::runtime_error(pn::format("{0}: must be int, or \"player\"", x.path()).c_str());
    }
}

sfz::optional<Handle<Level>> optional_level(path_value x) {
    sfz::optional<int64_t> i = optional_int(x);
    if (i.has_value()) {
        return sfz::make_optional(Handle<Level>(*i));
    } else {
        return sfz::nullopt;
    }
}

sfz::optional<Owner> optional_owner(path_value x) {
    return optional_enum<Owner>(
            x, {{"any", Owner::ANY}, {"same", Owner::SAME}, {"different", Owner::DIFFERENT}});
}

Handle<Race> required_race(path_value x) { return Handle<Race>(required_int(x)); }

template <typename T, int N>
T required_enum(path_value x, const std::pair<pn::string_view, T> (&values)[N]) {
    if (x.value().is_string()) {
        for (auto kv : values) {
            pn::string_view s = x.value().as_string();
            if (s == kv.first) {
                return kv.second;
            }
        }
    }

    pn::array keys;
    for (auto kv : values) {
        keys.push_back(kv.first.copy());
    }
    throw std::runtime_error(pn::format("{0}: must be one of {1}", x.path(), keys).c_str());
}

sfz::optional<Range<int64_t>> optional_int_range(path_value x) {
    if (x.value().is_null()) {
        return sfz::nullopt;
    } else if (x.value().is_int()) {
        int64_t begin = x.value().as_int();
        return sfz::make_optional(Range<int64_t>{begin, begin + 1});
    } else if (x.value().is_map()) {
        int64_t begin = required_int(x.get("begin"));
        int64_t end   = required_int(x.get("end"));
        return sfz::make_optional(Range<int64_t>{begin, end});
    } else {
        throw std::runtime_error(pn::format("{0}: must be null, int, or map", x.path()).c_str());
    }
}

Range<int64_t> required_int_range(path_value x) {
    if (x.value().is_int()) {
        int64_t begin = x.value().as_int();
        return Range<int64_t>{begin, begin + 1};
    } else if (x.value().is_map()) {
        int64_t begin = required_int(x.get("begin"));
        int64_t end   = required_int(x.get("end"));
        return Range<int64_t>{begin, end};
    } else {
        throw std::runtime_error(pn::format("{0}: must be null, int, or map", x.path()).c_str());
    }
}

Range<Fixed> required_fixed_range(path_value x) {
    if (x.value().is_float()) {
        Fixed begin = Fixed::from_float(x.value().as_float());
        Fixed end   = Fixed::from_val(begin.val() + 1);
        return {begin, end};
    } else if (x.value().is_map()) {
        Fixed begin = required_fixed(x.get("begin"));
        Fixed end   = required_fixed(x.get("end"));
        return {begin, end};
    } else {
        throw std::runtime_error(pn::format("{0}: must be float or map", x.path()).c_str());
    }
}

Range<ticks> required_ticks_range(path_value x) {
    auto range = required_int_range(x);
    return {ticks(range.begin), ticks(range.end)};
}

sfz::optional<HandleList<Level_Condition>> optional_condition_range(path_value x) {
    auto range = optional_int_range(x);
    if (range.has_value()) {
        return sfz::make_optional(HandleList<Level::Condition>(range->begin, range->end));
    } else {
        return sfz::nullopt;
    }
}

HandleList<Level::Initial> required_initial_range(path_value x) {
    auto range = required_int_range(x);
    return HandleList<Level::Initial>(range.begin, range.end);
}

sfz::optional<Point> optional_point(path_value x) {
    if (x.value().is_null()) {
        return sfz::nullopt;
    } else if (x.value().is_map()) {
        int32_t px = required_int(x.get("x"));
        int32_t py = required_int(x.get("y"));
        return sfz::make_optional<Point>({px, py});
    } else {
        throw std::runtime_error(pn::format("{0}: must be map", x.path()).c_str());
    }
}

Point required_point(path_value x) {
    if (x.value().is_map()) {
        int32_t px = required_int(x.get("x"));
        int32_t py = required_int(x.get("y"));
        return {px, py};
    } else {
        throw std::runtime_error(pn::format("{0}: must be map", x.path()).c_str());
    }
}

Hue required_hue(path_value x) {
    return required_enum<Hue>(
            x, {{"red", Hue::RED},
                {"orange", Hue::ORANGE},
                {"yellow", Hue::YELLOW},
                {"blue", Hue::BLUE},
                {"green", Hue::GREEN},
                {"purple", Hue::PURPLE},
                {"indigo", Hue::INDIGO},
                {"salmon", Hue::SALMON},
                {"gold", Hue::GOLD},
                {"aqua", Hue::AQUA},
                {"pink", Hue::PINK},
                {"pale green", Hue::PALE_GREEN},
                {"pale purple", Hue::PALE_PURPLE},
                {"sky blue", Hue::SKY_BLUE},
                {"tan", Hue::TAN},
                {"gray", Hue::GRAY}});
}

KillKind required_kill_kind(path_value x) {
    return required_enum<KillKind>(
            x, {{"none", KillKind::NONE},
                {"expire", KillKind::EXPIRE},
                {"destroy", KillKind::DESTROY}});
}

sfz::optional<MoveOrigin> optional_origin(path_value x) {
    return optional_enum<MoveOrigin>(
            x, {{"level", MoveOrigin::LEVEL},
                {"subject", MoveOrigin::SUBJECT},
                {"object", MoveOrigin::OBJECT}});
}

int required_key(path_value x) {
    return required_enum<int>(
            x, {{"up", 0},
                {"down", 1},
                {"left", 2},
                {"right", 3},
                {"fire_1", 4},
                {"fire_2", 5},
                {"fire_s", 6},
                {"warp", 0},
                {"select_friend", 8},
                {"select_foe", 9},
                {"select_base", 10},
                {"target", 11},
                {"order", 12},
                {"zoom_in", 13},
                {"zoom_out", 14},
                {"comp_up", 15},
                {"comp_down", 16},
                {"comp_accept", 17},
                {"comp_back", 18},

                {"comp_message", 26},
                {"comp_special", 27},
                {"comp_build", 28},
                {"zoom_shortcut", 29},
                {"send_message", 30},
                {"mouse", 31}});
}

ConditionOp required_condition_op(path_value x) {
    return required_enum<ConditionOp>(
            x, {{"eq", ConditionOp::EQ},
                {"ne", ConditionOp::NE},
                {"lt", ConditionOp::LT},
                {"gt", ConditionOp::GT},
                {"le", ConditionOp::LE},
                {"ge", ConditionOp::GE}});
}

LevelType required_level_type(path_value x) {
    return required_enum<LevelType>(
            x, {{"solo", LevelType::SOLO}, {"net", LevelType::NET}, {"demo", LevelType::DEMO}});
}

PlayerType required_player_type(path_value x) {
    return required_enum<PlayerType>(x, {{"human", PlayerType::HUMAN}, {"cpu", PlayerType::CPU}});
}

PushKind required_push_kind(path_value x) {
    return required_enum<PushKind>(
            x, {{"stop", PushKind::STOP},
                {"collide", PushKind::COLLIDE},
                {"decelerate", PushKind::DECELERATE},
                {"boost", PushKind::BOOST},
                {"set", PushKind::SET},
                {"cruise", PushKind::CRUISE}});
}

Screen required_screen(path_value x) {
    return required_enum<Screen>(
            x, {{"main", Screen::MAIN},
                {"build", Screen::BUILD},
                {"special", Screen::SPECIAL},
                {"message", Screen::MESSAGE},
                {"status", Screen::STATUS}});
}

SubjectValue required_subject_value(path_value x) {
    return required_enum<SubjectValue>(
            x, {{"control", SubjectValue::CONTROL},
                {"target", SubjectValue::TARGET},
                {"player", SubjectValue::PLAYER}});
}

Weapon required_weapon(path_value x) {
    return required_enum<Weapon>(
            x, {{"pulse", Weapon::PULSE}, {"beam", Weapon::BEAM}, {"special", Weapon::SPECIAL}});
}

Zoom required_zoom(path_value x) {
    return required_enum<Zoom>(
            x, {{"2:1", Zoom::DOUBLE},
                {"1:1", Zoom::ACTUAL},
                {"1:2", Zoom::HALF},
                {"1:4", Zoom::QUARTER},
                {"1:16", Zoom::SIXTEENTH},
                {"foe", Zoom::FOE},
                {"object", Zoom::OBJECT},
                {"all", Zoom::ALL}});
}

sfz::optional<int32_t> optional_object_attributes(path_value x) {
    if (x.value().is_null()) {
        return sfz::nullopt;
    } else if (x.value().is_map()) {
        static const pn::string_view flags[32] = {"can_turn",
                                                  "can_be_engaged",
                                                  "has_direction_goal",
                                                  "is_remote",
                                                  "is_human_controlled",
                                                  "is_beam",
                                                  "does_bounce",
                                                  "is_self_animated",
                                                  "shape_from_direction",
                                                  "is_player_ship",
                                                  "can_be_destination",
                                                  "can_engage",
                                                  "can_evade",
                                                  "can_accept_messages",
                                                  "can_accept_build",
                                                  "can_accept_destination",
                                                  "autotarget",
                                                  "animation_cycle",
                                                  "can_collide",
                                                  "can_be_hit",
                                                  "is_destination",
                                                  "hide_effect",
                                                  "release_energy_on_death",
                                                  "hated",
                                                  "occupies_space",
                                                  "static_destination",
                                                  "can_be_evaded",
                                                  "neutral_death",
                                                  "is_guided",
                                                  "appear_on_radar",
                                                  "bit31",
                                                  "on_autopilot"};

        int32_t bit    = 0x00000001;
        int32_t result = 0x00000000;
        for (pn::string_view flag : flags) {
            if (optional_bool(x.get(flag)).value_or(false)) {
                result |= bit;
            }
            bit <<= 1;
        }
        return sfz::make_optional(+result);
    } else {
        throw std::runtime_error(pn::format("{0}: must be null or map", x.path()).c_str());
    }
}

sfz::optional<int32_t> optional_initial_attributes(path_value x) {
    if (x.value().is_null()) {
        return sfz::nullopt;
    } else if (x.value().is_map()) {
        static const pn::string_view flags[32] = {"can_turn",
                                                  "can_be_engaged",
                                                  "has_direction_goal",
                                                  "is_remote",
                                                  "fixed_race",
                                                  "initially_hidden",
                                                  "does_bounce",
                                                  "is_self_animated",
                                                  "shape_from_direction",
                                                  "is_player_ship",
                                                  "can_be_destination",
                                                  "can_engage",
                                                  "can_evade",
                                                  "can_accept_messages",
                                                  "can_accept_build",
                                                  "can_accept_destination",
                                                  "autotarget",
                                                  "animation_cycle",
                                                  "can_collide",
                                                  "can_be_hit",
                                                  "is_destination",
                                                  "hide_effect",
                                                  "release_energy_on_death",
                                                  "hated",
                                                  "occupies_space",
                                                  "static_destination",
                                                  "can_be_evaded",
                                                  "neutral_death",
                                                  "is_guided",
                                                  "appear_on_radar",
                                                  "bit31",
                                                  "on_autopilot"};

        int32_t bit    = 0x00000001;
        int32_t result = 0x00000000;
        for (pn::string_view flag : flags) {
            if (optional_bool(x.get(flag)).value_or(false)) {
                result |= bit;
            }
            bit <<= 1;
        }
        return sfz::make_optional(+result);
    } else {
        throw std::runtime_error(pn::format("{0}: must be null or map", x.path()).c_str());
    }
}

int32_t optional_keys(path_value x) {
    if (x.value().is_null()) {
        return 0x00000000;
    } else if (x.value().is_array()) {
        pn::array_cref a      = x.value().as_array();
        int32_t        result = 0x00000000;
        for (int i = 0; i < a.size(); ++i) {
            int key = required_key(x.get(i));
            result |= (0x1 << key);
        }
        return result;
    } else {
        throw std::runtime_error(pn::format("{0}: must be null or list", x.path()).c_str());
    }
}

std::vector<pn::string> required_string_array(path_value x) {
    if (x.value().is_array()) {
        pn::array_cref          a = x.value().as_array();
        std::vector<pn::string> result;
        for (int i = 0; i < a.size(); ++i) {
            result.emplace_back(required_string(x.get(i)).copy());
        }
        return result;
    } else {
        throw std::runtime_error(pn::format("{0}: must be array", x.path()).c_str());
    }
}

sfz::optional<std::vector<pn::string>> optional_string_array(path_value x) {
    if (x.value().is_null()) {
        return sfz::nullopt;
    } else if (x.value().is_array()) {
        pn::array_cref          a = x.value().as_array();
        std::vector<pn::string> result;
        for (int i = 0; i < a.size(); ++i) {
            result.emplace_back(required_string(x.get(i)).copy());
        }
        return sfz::make_optional(std::move(result));
    } else {
        throw std::runtime_error(pn::format("{0}: must be null or array", x.path()).c_str());
    }
}

sfz::optional<std::vector<int>> optional_int_array(path_value x) {
    if (x.value().is_null()) {
        return sfz::nullopt;
    } else if (x.value().is_array()) {
        pn::array_cref   a = x.value().as_array();
        std::vector<int> result;
        for (int i = 0; i < a.size(); ++i) {
            result.emplace_back(required_int(x.get(i)));
        }
        return sfz::make_optional<std::vector<int>>(std::move(result));
    } else {
        throw std::runtime_error(pn::format("{0}: must be null or array", x.path()).c_str());
    }
}

}  // namespace antares

#include <pn/fwd>

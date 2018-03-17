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
#include "data/level.hpp"
#include "data/resource.hpp"

namespace antares {

namespace {

enum objectVerbIDEnum {
    kNoAction            = 0 << 8,
    kCreateObject        = 1 << 8,
    kPlaySound           = 2 << 8,
    kAlter               = 3 << 8,
    kMakeSparks          = 4 << 8,
    kReleaseEnergy       = 5 << 8,
    kLandAt              = 6 << 8,
    kEnterWarp           = 7 << 8,
    kDisplayMessage      = 8 << 8,
    kChangeScore         = 9 << 8,
    kDeclareWinner       = 10 << 8,
    kDie                 = 11 << 8,
    kSetDestination      = 12 << 8,
    kActivateSpecial     = 13 << 8,
    kActivatePulse       = 14 << 8,
    kActivateBeam        = 15 << 8,
    kColorFlash          = 16 << 8,
    kCreateObjectSetDest = 17 << 8,  // creates an object with the same destination as anObject's
                                     // (either subject or direct)
    kNilTarget           = 18 << 8,
    kDisableKeys         = 19 << 8,
    kEnableKeys          = 20 << 8,
    kSetZoom             = 21 << 8,
    kComputerSelect      = 22 << 8,  // selects a line & screen of the minicomputer
    kAssumeInitialObject = 23 << 8,  // assumes the identity of an intial object; for tutorial
};

enum alterVerbIDType {
    kAlterDamage      = kAlter | 0,
    kAlterVelocity    = kAlter | 1,
    kAlterThrust      = kAlter | 2,
    kAlterMaxThrust   = kAlter | 3,
    kAlterMaxVelocity = kAlter | 4,
    kAlterMaxTurnRate = kAlter | 5,
    kAlterLocation    = kAlter | 6,
    kAlterScale       = kAlter | 7,
    kAlterWeapon1     = kAlter | 8,
    kAlterWeapon2     = kAlter | 9,
    kAlterSpecial     = kAlter | 10,
    kAlterEnergy      = kAlter | 11,
    kAlterOwner       = kAlter | 12,
    kAlterHidden      = kAlter | 13,
    kAlterCloak       = kAlter | 14,
    kAlterOffline     = kAlter | 15,
    kAlterSpin        = kAlter | 16,
    kAlterBaseType    = kAlter | 17,
    kAlterConditionTrueYet =
            kAlter | 18,  // relative = state, min = which condition basically force to recheck
    kAlterOccupation = kAlter | 19,  // for special neutral death objects
    kAlterAbsoluteCash =
            kAlter |
            20,  // relative: true = cash to object : false = range = admiral who gets cash
    kAlterAge              = kAlter | 21,
    kAlterAttributes       = kAlter | 22,
    kAlterLevelKeyTag      = kAlter | 23,
    kAlterOrderKeyTag      = kAlter | 24,
    kAlterEngageKeyTag     = kAlter | 25,
    kAlterAbsoluteLocation = kAlter | 26,
};

class path_value {
    enum class Kind { ROOT, KEY, INDEX };

  public:
    path_value(pn::value_cref x) : _kind{Kind::ROOT}, _value{x} {}

    pn::value_cref value() const { return _value; }

    path_value get(pn::string_view key) const {
        return path_value{this, Kind::KEY, key, 0, _value.as_map().get(key)};
    }
    path_value get(int64_t index) const {
        return path_value{this, Kind::INDEX, pn::string_view{}, index,
                          array_get(_value.as_array(), index)};
    }

    pn::string path() const {
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

  private:
    static pn::value_cref array_get(pn::array_cref a, int64_t index) {
        if ((0 <= index) && (index < a.size())) {
            return a[index];
        } else {
            return pn::value_cref{&pn_null};
        }
    }

    path_value(
            const path_value* parent, Kind kind, pn::string_view key, int64_t index,
            pn::value_cref value)
            : _parent{parent}, _kind{kind}, _key{key}, _index{index}, _value{value} {}

    const path_value* _parent = nullptr;
    Kind              _kind;
    pn::string_view   _key;
    int64_t           _index;
    pn::value_cref    _value;
};

static sfz::optional<bool> optional_bool(path_value x) {
    if (x.value().is_null()) {
        return sfz::nullopt;
    } else if (x.value().is_bool()) {
        return sfz::make_optional(x.value().as_bool());
    } else {
        throw std::runtime_error(pn::format("{0}: must be null or bool", x.path()).c_str());
    }
}

static sfz::optional<int64_t> optional_int(path_value x) {
    if (x.value().is_null()) {
        return sfz::nullopt;
    } else if (x.value().is_int()) {
        return sfz::make_optional(x.value().as_int());
    } else {
        throw std::runtime_error(pn::format("{0}: must be null or int", x.path()).c_str());
    }
}

static int64_t required_int(path_value x) {
    if (x.value().is_int()) {
        return x.value().as_int();
    } else {
        throw std::runtime_error(pn::format("{0}: must be int", x.path()).c_str());
    }
}

static sfz::optional<Fixed> optional_fixed(path_value x) {
    if (x.value().is_null()) {
        return sfz::nullopt;
    } else if (x.value().is_float()) {
        return sfz::make_optional(Fixed::from_float(x.value().as_float()));
    } else {
        throw std::runtime_error(pn::format("{0}: must be float", x.path()).c_str());
    }
}

static Fixed required_fixed(path_value x) {
    if (x.value().is_float()) {
        return Fixed::from_float(x.value().as_float());
    } else {
        throw std::runtime_error(pn::format("{0}: must be float", x.path()).c_str());
    }
}

static sfz::optional<pn::string_view> optional_string(path_value x) {
    if (x.value().is_null()) {
        return sfz::nullopt;
    } else if (x.value().is_string()) {
        return sfz::make_optional(x.value().as_string());
    } else {
        throw std::runtime_error(pn::format("{0}: must be string", x.path()).c_str());
    }
}

static pn::string_view required_string(path_value x) {
    if (x.value().is_string()) {
        return x.value().as_string();
    } else {
        throw std::runtime_error(pn::format("{0}: must be string", x.path()).c_str());
    }
}

template <typename T, int N>
static sfz::optional<T> optional_enum(
        path_value x, const std::pair<pn::string_view, T> (&values)[N]) {
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

static sfz::optional<ticks> optional_ticks(path_value x) {
    sfz::optional<int64_t> i = optional_int(x);
    if (i.has_value()) {
        return sfz::make_optional(ticks(*i));
    } else {
        return sfz::nullopt;
    }
}

static ticks required_ticks(path_value x) { return ticks(required_int(x)); }

static sfz::optional<Handle<Admiral>> optional_admiral(path_value x) {
    sfz::optional<int64_t> i = optional_int(x);
    if (i.has_value()) {
        return sfz::make_optional(Handle<Admiral>(*i));
    } else {
        return sfz::nullopt;
    }
}

static Handle<BaseObject> required_base(path_value x) {
    return Handle<BaseObject>(required_int(x));
}

static sfz::optional<Handle<Level::Initial>> optional_initial(path_value x) {
    sfz::optional<int64_t> i = optional_int(x);
    if (i.has_value()) {
        return sfz::make_optional(Handle<Level::Initial>(*i));
    } else {
        return sfz::nullopt;
    }
}

static sfz::optional<Handle<Level>> optional_level(path_value x) {
    sfz::optional<int64_t> i = optional_int(x);
    if (i.has_value()) {
        return sfz::make_optional(Handle<Level>(*i));
    } else {
        return sfz::nullopt;
    }
}

static sfz::optional<Owner> optional_owner(path_value x) {
    return optional_enum<Owner>(
            x, {{"any", Owner::ANY}, {"same", Owner::SAME}, {"different", Owner::DIFFERENT}});
}

template <typename T, int N>
static T required_enum(path_value x, const std::pair<pn::string_view, T> (&values)[N]) {
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

static sfz::optional<Range<int64_t>> optional_int_range(path_value x) {
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

static Range<int64_t> required_int_range(path_value x) {
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

static Range<Fixed> required_fixed_range(path_value x) {
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

static Range<ticks> required_ticks_range(path_value x) {
    auto range = required_int_range(x);
    return {ticks(range.begin), ticks(range.end)};
}

static HandleList<Level::Initial> required_initial_range(path_value x) {
    auto range = required_int_range(x);
    return HandleList<Level::Initial>(range.begin, range.end);
}

static sfz::optional<coordPointType> optional_point(path_value x) {
    if (x.value().is_null()) {
        return sfz::nullopt;
    } else if (x.value().is_map()) {
        uint32_t px = required_int(x.get("x"));
        uint32_t py = required_int(x.get("y"));
        return sfz::make_optional<coordPointType>({px, py});
    } else {
        throw std::runtime_error(pn::format("{0}: must be map", x.path()).c_str());
    }
}

static Hue required_hue(path_value x) {
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
                {"pale-green", Hue::PALE_GREEN},
                {"pale-purple", Hue::PALE_PURPLE},
                {"sky-blue", Hue::SKY_BLUE},
                {"tan", Hue::TAN},
                {"gray", Hue::GRAY}});
}

static KillAction::Kind required_kill_kind(path_value x) {
    return required_enum<KillAction::Kind>(
            x, {{"none", KillAction::Kind::NONE},
                {"expire", KillAction::Kind::EXPIRE},
                {"destroy", KillAction::Kind::DESTROY}});
}

static sfz::optional<MoveAction::Origin> optional_origin(path_value x) {
    return optional_enum<MoveAction::Origin>(
            x, {{"level", MoveAction::Origin::LEVEL},
                {"subject", MoveAction::Origin::SUBJECT},
                {"object", MoveAction::Origin::OBJECT}});
}

static int required_key(path_value x) {
    return required_enum<int>(x, {{"up", 0},
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

static PushAction::Kind required_push_kind(path_value x) {
    return required_enum<PushAction::Kind>(
            x, {{"stop", PushAction::Kind::STOP},
                {"collide", PushAction::Kind::COLLIDE},
                {"decelerate", PushAction::Kind::DECELERATE},
                {"boost", PushAction::Kind::BOOST},
                {"set", PushAction::Kind::SET},
                {"cruise", PushAction::Kind::CRUISE}});
}

static Screen required_screen(path_value x) {
    return required_enum<Screen>(
            x, {{"main", Screen::MAIN},
                {"build", Screen::BUILD},
                {"special", Screen::SPECIAL},
                {"message", Screen::MESSAGE},
                {"status", Screen::STATUS}});
}

static Weapon required_weapon(path_value x) {
    return required_enum<Weapon>(
            x, {{"pulse", Weapon::PULSE}, {"beam", Weapon::BEAM}, {"special", Weapon::SPECIAL}});
}

static Zoom required_zoom(path_value x) {
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

static sfz::optional<int32_t> optional_object_attributes(path_value x) {
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

static int32_t optional_keys(path_value x) {
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

static std::vector<pn::string> required_string_array(path_value x) {
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
    a->enable  = optional_int_range(x.get("enable")).value_or(Range<int64_t>::empty());
    a->disable = optional_int_range(x.get("disable")).value_or(Range<int64_t>::empty());
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
    a->origin   = optional_origin(x.get("origin")).value_or(MoveAction::Origin::LEVEL);
    a->to       = optional_point(x.get("to")).value_or(coordPointType{0, 0});
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

std::unique_ptr<Action> action(pn::value_cref x0) {
    if (x0.is_null()) {
        return std::unique_ptr<Action>(new NoAction);
    } else if (!x0.is_map()) {
        throw std::runtime_error("must be null or map");
    }
    path_value x{x0};

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

    if (a) {
        a->reflexive = optional_bool(x.get("reflexive")).value_or(false);

        a->inclusive_filter = optional_object_attributes(x.get("inclusive_filter")).value_or(0);
        a->level_key_tag    = optional_string(x.get("level_key_filter")).value_or("").copy();

        a->owner = optional_owner(x.get("owner")).value_or(Owner::ANY);
        a->delay = optional_ticks(x.get("delay")).value_or(ticks(0));

        a->initialSubjectOverride =
                optional_initial(x.get("initial_subject")).value_or(Level::Initial::none());
        a->initialDirectOverride =
                optional_initial(x.get("initial_object")).value_or(Level::Initial::none());
    }
    return a;
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

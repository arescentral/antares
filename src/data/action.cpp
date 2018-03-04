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

static sfz::optional<Handle<Level::Initial>> optional_initial(path_value x) {
    sfz::optional<int64_t> i = optional_int(x);
    if (i.has_value()) {
        return sfz::make_optional(Handle<Level::Initial>(*i));
    } else {
        return sfz::nullopt;
    }
}

static sfz::optional<Owner> optional_owner(path_value x) {
    return optional_enum<Owner>(
            x, {{"any", Owner::ANY}, {"same", Owner::SAME}, {"different", Owner::DIFFERENT}});
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

bool read_from(pn::file_view in, CreateAction* create, bool inherit) {
    int32_t base_type;
    int32_t count_minimum, count_range;
    uint8_t relative_velocity, relative_direction;
    if (!in.read(
                &base_type, &count_minimum, &count_range, &relative_velocity, &relative_direction,
                &create->distance)) {
        return false;
    }
    create->base               = Handle<BaseObject>(base_type);
    create->count              = {count_minimum, count_minimum + std::max(count_range, 0)};
    create->relative_velocity  = relative_velocity;
    create->relative_direction = relative_direction;
    create->inherit            = inherit;
    create->legacy_random      = (count_range == 1);
    return true;
}

bool read_from(pn::file_view in, SoundAction* sound) {
    uint8_t absolute;
    int32_t persistence;
    int32_t id_minimum, id_range;
    if (!in.read(
                &sound->priority, pn::pad(1), &persistence, &absolute, pn::pad(1), &sound->volume,
                pn::pad(4), &id_minimum, &id_range)) {
        return false;
    }
    sound->id.begin    = id_minimum;
    sound->id.end      = id_minimum + id_range + 1;
    sound->absolute    = absolute;
    sound->persistence = ticks(persistence);
    return true;
}

bool read_from(pn::file_view in, HealAction* heal) { return in.read(pn::pad(1), &heal->value); }

bool read_from(pn::file_view in, EnergizeAction* energize) {
    return in.read(pn::pad(1), &energize->value);
}

bool read_from(pn::file_view in, OccupyAction* occupy) {
    return in.read(pn::pad(1), &occupy->value);
}

bool read_from(pn::file_view in, Weapon which, EquipAction* equip) {
    int32_t base;
    if (!in.read(pn::pad(1), &base)) {
        return false;
    }
    equip->which = which;
    equip->base  = Handle<BaseObject>(base);
    return true;
}

bool read_from(pn::file_view in, RevealAction* reveal) {
    int32_t first, count_minus_1;
    if (!in.read(pn::pad(1), &first, &count_minus_1)) {
        return false;
    }
    reveal->initial = HandleList<Level::Initial>(first, first + std::max(count_minus_1, 0) + 1);
    return true;
}

bool read_from(pn::file_view in, DisableAction* disable) {
    int32_t minimum, range;
    if (!in.read(pn::pad(1), &minimum, &range)) {
        return false;
    }
    disable->value.begin = Fixed::from_val(minimum);
    disable->value.end   = Fixed::from_val(minimum + range);
    return true;
}

bool read_from(pn::file_view in, SpinAction* spin) {
    int32_t minimum, range;
    if (!in.read(pn::pad(1), &minimum, &range)) {
        return false;
    }
    spin->value.begin = Fixed::from_val(minimum);
    spin->value.end   = Fixed::from_val(minimum + range);
    return true;
}

bool read_from(pn::file_view in, bool reflexive, PushAction* push) {
    uint8_t relative;
    int32_t value;
    if (!in.read(&relative, &value)) {
        return false;
    }
    if (relative) {
        if (reflexive) {
            push->kind = PushAction::Kind::BOOST;
        } else if (value >= 0) {
            push->kind = PushAction::Kind::COLLIDE;
        } else {
            push->kind = PushAction::Kind::DECELERATE;
            value      = -value;
        }
    } else {
        if (value == 0) {
            push->kind = PushAction::Kind::STOP;
        } else if (reflexive) {
            push->kind = PushAction::Kind::CRUISE;
        } else {
            push->kind = PushAction::Kind::SET;
        }
    }
    push->value = Fixed::from_val(value);
    return true;
}

bool read_from(pn::file_view in, CapSpeedAction* cap_speed) {
    int32_t value;
    if (!in.read(pn::pad(1), &value)) {
        return false;
    }
    if (value < 0) {
        cap_speed->value.reset();
    } else {
        cap_speed->value.emplace(Fixed::from_val(value));
    }
    return true;
}

bool read_from(pn::file_view in, ThrustAction* thrust) {
    int32_t minimum, range;
    if (!in.read(pn::pad(1), &minimum, &range)) {
        return false;
    }
    thrust->value.begin = Fixed::from_val(minimum);
    thrust->value.end   = Fixed::from_val(minimum + range);
    thrust->relative    = false;
    return true;
}

bool read_from(pn::file_view in, MorphAction* morph) {
    uint8_t keep_ammo;
    int32_t base;
    if (!in.read(&keep_ammo, &base)) {
        return false;
    }
    morph->keep_ammo = keep_ammo;
    morph->base      = Handle<BaseObject>(base);
    return true;
}

bool read_from(pn::file_view in, CaptureAction* capture) {
    uint8_t relative;
    int32_t admiral;
    if (!in.read(&relative, &admiral)) {
        return false;
    }
    if (relative) {
        capture->player.reset();
    } else {
        capture->player.emplace(Handle<Admiral>(admiral));
    }
    return true;
}

bool read_from(pn::file_view in, ConditionAction* condition) {
    uint8_t disabled;
    int32_t first, count_minus_1;
    if (!in.read(&disabled, &first, &count_minus_1)) {
        return false;
    }
    if (disabled) {
        condition->enable  = Range<int32_t>::empty();
        condition->disable = {first, first + std::max(count_minus_1, 0) + 1};
    } else {
        condition->enable  = {first, first + std::max(count_minus_1, 0) + 1};
        condition->disable = Range<int32_t>::empty();
    }
    return true;
}

bool read_from(pn::file_view in, PayAction* pay) {
    uint8_t  relative;
    uint32_t player;
    int32_t  value;
    if (!(in.read(&relative, &value, &player))) {
        return false;
    }
    pay->value = Fixed::from_val(value);
    if (relative) {
        pay->player.reset();
    } else {
        pay->player.emplace(Handle<Admiral>(player));
    }
    return true;
}

bool read_from(pn::file_view in, AgeAction* argument) {
    uint8_t relative;
    int32_t minimum, range;
    if (!in.read(&relative, &minimum, &range)) {
        return false;
    }
    argument->relative    = relative;
    argument->value.begin = ticks(minimum);
    argument->value.end   = ticks(minimum + range);
    return true;
}

bool read_from_relative(pn::file_view in, bool reflexive, MoveAction* move) {
    uint8_t relative;
    int32_t distance;
    if (!in.read(&relative, &distance)) {
        return false;
    }
    if (!relative) {
        move->origin = MoveAction::Origin::LEVEL;
    } else if (reflexive) {
        move->origin = MoveAction::Origin::OBJECT;
    } else {
        move->origin = MoveAction::Origin::SUBJECT;
    }
    move->to       = {0, 0};
    move->distance = distance;
    return true;
}

bool read_from_absolute(pn::file_view in, bool reflexive, MoveAction* move) {
    uint8_t  relative;
    uint32_t x, y;
    if (!in.read(&relative, &x, &y)) {
        return false;
    }
    if (!relative) {
        move->origin = MoveAction::Origin::LEVEL;
    } else if (reflexive) {
        move->origin = MoveAction::Origin::SUBJECT;
    } else {
        move->origin = MoveAction::Origin::OBJECT;
    }
    move->to       = {x, y};
    move->distance = 0;
    return true;
}

bool read_from(pn::file_view in, SparkAction* sparks) {
    uint8_t hue;
    int32_t velocity;
    if (!in.read(&sparks->count, &sparks->decay, &velocity, &hue)) {
        return false;
    }
    sparks->velocity = Fixed::from_val(velocity);
    sparks->hue      = Hue(hue);
    return true;
}

bool read_from(pn::file_view in, LandAction* land) { return in.read(&land->speed); }

bool read_from(pn::file_view in, MessageAction* message) {
    int16_t page_count;
    if (!in.read(&message->id, &page_count)) {
        return false;
    }
    message->pages.clear();
    for (int id : sfz::range<int>(message->id, message->id + page_count)) {
        try {
            message->pages.push_back(Resource::text(id));
        } catch (...) {
            message->pages.push_back("<RESOURCE NOT FOUND>");
        }
    }
    return true;
}

bool read_from(pn::file_view in, ScoreAction* score) {
    int32_t admiral;
    if (!in.read(&admiral, &score->which, &score->value)) {
        return false;
    }
    if (admiral < 0) {
        score->player = sfz::nullopt;
    } else {
        score->player = sfz::make_optional(Handle<Admiral>(admiral));
    }
    return true;
}

bool read_from(pn::file_view in, WinAction* win) {
    int32_t admiral, next, text_id;
    if (!in.read(&admiral, &next, &text_id)) {
        return false;
    }
    if (admiral < 0) {
        win->player = sfz::nullopt;
    } else {
        win->player = sfz::make_optional(Handle<Admiral>(admiral));
    }
    if (next < 0) {
        win->next = sfz::nullopt;
    } else {
        win->next.emplace(next);
    }
    win->text = Resource::text(text_id);
    return true;
}

bool read_from(pn::file_view in, KillAction* kill) {
    uint8_t kind;
    if (!in.read(&kind)) {
        return false;
    }
    kill->kind = static_cast<KillAction::Kind>(kind);
    return true;
}

bool read_from(pn::file_view in, FlashAction* flash) {
    uint8_t hue;
    if (!in.read(&flash->length, &hue, &flash->shade)) {
        return false;
    }
    flash->hue = Hue(hue);
    return true;
}

bool read_enable_keys_from(pn::file_view in, KeyAction* key) {
    key->disable = 0x00000000;
    return in.read(&key->enable);
}

bool read_disable_keys_from(pn::file_view in, KeyAction* key) {
    key->enable = 0x00000000;
    return in.read(&key->disable);
}

bool read_from(pn::file_view in, ZoomAction* zoom) {
    int32_t value;
    if (!in.read(&value)) {
        return false;
    }
    zoom->value = Zoom(value);
    return true;
}

bool read_from(pn::file_view in, SelectAction* select) {
    int32_t screen;
    if (!in.read(&screen, &select->line)) {
        return false;
    }
    select->screen = Screen(screen);
    return true;
}

bool read_from(pn::file_view in, AssumeAction* assume) { return in.read(&assume->which); }

template <typename T>
T* init(std::unique_ptr<Action>* action) {
    T* t;
    action->reset(t = new T());
    return t;
}

bool read_argument(int verb, bool reflexive, std::unique_ptr<Action>* action, pn::file_view sub) {
    switch (static_cast<objectVerbIDEnum>(verb)) {
        case kReleaseEnergy:
        case kNoAction: init<NoAction>(action); return true;

        case kSetDestination: init<OrderAction>(action); return true;
        case kActivateSpecial: init<FireAction>(action)->which = Weapon::SPECIAL; return true;
        case kActivatePulse: init<FireAction>(action)->which = Weapon::PULSE; return true;
        case kActivateBeam: init<FireAction>(action)->which = Weapon::BEAM; return true;
        case kNilTarget: init<HoldPositionAction>(action); return true;

        case kCreateObject: return read_from(sub, init<CreateAction>(action), false);
        case kCreateObjectSetDest: return read_from(sub, init<CreateAction>(action), true);

        case kPlaySound: return read_from(sub, init<SoundAction>(action));

        case kAlter: {
            uint8_t alter;
            if (!sub.read(&alter)) {
                return false;
            }
            verb |= alter;
            switch (static_cast<alterVerbIDType>(verb)) {
                case kAlterMaxThrust: init<NoAction>(action); return true;
                case kAlterMaxTurnRate: init<NoAction>(action); return true;
                case kAlterScale: init<NoAction>(action); return true;
                case kAlterAttributes: init<NoAction>(action); return true;
                case kAlterLevelKeyTag: init<NoAction>(action); return true;
                case kAlterOrderKeyTag: init<NoAction>(action); return true;
                case kAlterEngageKeyTag: init<NoAction>(action); return true;

                case kAlterCloak: init<CloakAction>(action); return true;

                case kAlterDamage: return read_from(sub, init<HealAction>(action));
                case kAlterEnergy: return read_from(sub, init<EnergizeAction>(action));
                case kAlterHidden: return read_from(sub, init<RevealAction>(action));
                case kAlterSpin: return read_from(sub, init<SpinAction>(action));
                case kAlterOffline: return read_from(sub, init<DisableAction>(action));
                case kAlterVelocity: return read_from(sub, reflexive, init<PushAction>(action));
                case kAlterMaxVelocity: return read_from(sub, init<CapSpeedAction>(action));
                case kAlterThrust: return read_from(sub, init<ThrustAction>(action));
                case kAlterBaseType: return read_from(sub, init<MorphAction>(action));
                case kAlterOwner: return read_from(sub, init<CaptureAction>(action));
                case kAlterConditionTrueYet: return read_from(sub, init<ConditionAction>(action));
                case kAlterOccupation: return read_from(sub, init<OccupyAction>(action));
                case kAlterAbsoluteCash: return read_from(sub, init<PayAction>(action));
                case kAlterAge: return read_from(sub, init<AgeAction>(action));
                case kAlterLocation:
                    return read_from_relative(sub, reflexive, init<MoveAction>(action));
                case kAlterAbsoluteLocation:
                    return read_from_absolute(sub, reflexive, init<MoveAction>(action));
                case kAlterWeapon1:
                    return read_from(sub, Weapon::PULSE, init<EquipAction>(action));
                case kAlterWeapon2: return read_from(sub, Weapon::BEAM, init<EquipAction>(action));
                case kAlterSpecial:
                    return read_from(sub, Weapon::SPECIAL, init<EquipAction>(action));
            }
        }

        case kMakeSparks: return read_from(sub, init<SparkAction>(action));

        case kLandAt: return read_from(sub, init<LandAction>(action));

        case kEnterWarp: init<WarpAction>(action); return true;

        case kDisplayMessage: return read_from(sub, init<MessageAction>(action));

        case kChangeScore: return read_from(sub, init<ScoreAction>(action));

        case kDeclareWinner: return read_from(sub, init<WinAction>(action));

        case kDie: return read_from(sub, init<KillAction>(action));

        case kColorFlash: return read_from(sub, init<FlashAction>(action));

        case kDisableKeys: return read_disable_keys_from(sub, init<KeyAction>(action));
        case kEnableKeys: return read_enable_keys_from(sub, init<KeyAction>(action));

        case kSetZoom: return read_from(sub, init<ZoomAction>(action));

        case kComputerSelect: return read_from(sub, init<SelectAction>(action));

        case kAssumeInitialObject: return read_from(sub, init<AssumeAction>(action));
    }
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

    pn::string_view type = required_string(x.get("type"));
    static_cast<void>(type);

    uint8_t  verb, reflexive;
    pn::data section;
    section.resize(24);
    std::unique_ptr<Action> a;
    if (!(x.get("bin").value().as_data().open().read(&verb, &reflexive, pn::pad(22), &section) &&
          read_argument(verb << 8, reflexive, &a, section.open()))) {
        throw std::runtime_error("read failed");
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
Range<int>         Action::sound_range() const { return Range<int>::empty(); }
bool               Action::alters_owner() const { return false; }
bool               Action::check_conditions() const { return false; }

Handle<BaseObject> CreateAction::created_base() const { return base; }
Handle<BaseObject> MorphAction::created_base() const { return base; }
Handle<BaseObject> EquipAction::created_base() const { return base; }

Range<int> SoundAction::sound_range() const { return id; }

bool CaptureAction::alters_owner() const { return true; }

bool ScoreAction::check_conditions() const { return true; }
bool MessageAction::check_conditions() const { return true; }

}  // namespace antares

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

bool read_from(pn::file_view in, CreateAction* create, bool inherit) {
    int32_t base_type;
    uint8_t relative_velocity, relative_direction;
    if (!in.read(
                &base_type, &create->count_minimum, &create->count_range, &relative_velocity,
                &relative_direction, &create->distance)) {
        return false;
    }
    create->base               = Handle<BaseObject>(base_type);
    create->relative_velocity  = relative_velocity;
    create->relative_direction = relative_direction;
    create->inherit            = inherit;
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
    sound->id.first    = id_minimum;
    sound->id.second   = id_minimum + id_range + 1;
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

bool read_from(pn::file_view in, EquipAction::Which which, EquipAction* equip) {
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
    disable->value.first  = Fixed::from_val(minimum);
    disable->value.second = Fixed::from_val(minimum + range);
    return true;
}

bool read_from(pn::file_view in, SpinAction* spin) {
    int32_t minimum, range;
    if (!in.read(pn::pad(1), &minimum, &range)) {
        return false;
    }
    spin->value.first  = Fixed::from_val(minimum);
    spin->value.second = Fixed::from_val(minimum + range);
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
    cap_speed->value = Fixed::from_val(value);
    return true;
}

bool read_from(pn::file_view in, ThrustAction* thrust) {
    int32_t minimum, range;
    if (!in.read(pn::pad(1), &minimum, &range)) {
        return false;
    }
    thrust->value.first  = Fixed::from_val(minimum);
    thrust->value.second = Fixed::from_val(minimum + range);
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
    capture->relative = relative;
    capture->player   = Handle<Admiral>(admiral);
    return true;
}

bool read_from(pn::file_view in, ConditionAction* condition) {
    uint8_t disabled;
    int32_t first, count_minus_1;
    if (!in.read(&disabled, &first, &count_minus_1)) {
        return false;
    }
    condition->enabled      = !disabled;
    condition->which.first  = first;
    condition->which.second = first + std::max(count_minus_1, 0) + 1;
    return true;
}

bool read_from(pn::file_view in, PayAction* pay) {
    uint8_t  relative;
    uint32_t player;
    int32_t  value;
    if (!(in.read(&relative, &value, &player))) {
        return false;
    }
    pay->relative = relative;
    pay->value    = Fixed::from_val(value);
    pay->player   = Handle<Admiral>(player);
    return true;
}

bool read_from(pn::file_view in, AgeAction* argument) {
    uint8_t relative;
    int32_t minimum, range;
    if (!in.read(&relative, &minimum, &range)) {
        return false;
    }
    argument->relative     = relative;
    argument->value.first  = ticks(minimum);
    argument->value.second = ticks(minimum + range);
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

bool read_from_absolute(pn::file_view in, MoveAction* move) {
    uint8_t  relative;
    uint32_t x, y;
    if (!in.read(&relative, &x, &y)) {
        return false;
    }
    if (!relative) {
        move->origin = MoveAction::Origin::LEVEL;
    } else {
        move->origin = MoveAction::Origin::FOCUS;
    }
    move->to       = {x, y};
    move->distance = 0;
    return true;
}

bool read_from(pn::file_view in, SparkAction* sparks) {
    return in.read(&sparks->count, &sparks->decay) && read_from(in, &sparks->velocity) &&
           in.read(&sparks->hue);
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
    score->player = Handle<Admiral>(admiral);
    return true;
}

bool read_from(pn::file_view in, WinAction* win) {
    int32_t admiral;
    int32_t text_id;
    if (!in.read(&admiral, &win->next, &text_id)) {
        return false;
    }
    win->player = Handle<Admiral>(admiral);
    win->text   = Resource::text(text_id);
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
    return in.read(&flash->length, &flash->hue, &flash->shade);
}

bool read_enable_keys_from(pn::file_view in, KeyAction* key) {
    key->disable = 0x00000000;
    return in.read(&key->enable);
}

bool read_disable_keys_from(pn::file_view in, KeyAction* key) {
    key->enable = 0x00000000;
    return in.read(&key->disable);
}

bool read_from(pn::file_view in, ZoomAction* zoom) { return in.read(&zoom->value); }

bool read_from(pn::file_view in, SelectAction* select) {
    return in.read(&select->screen, &select->line);
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
        case kActivateSpecial:
            init<FireAction>(action)->which = FireAction::Which::SPECIAL;
            return true;
        case kActivatePulse:
            init<FireAction>(action)->which = FireAction::Which::PULSE;
            return true;
        case kActivateBeam: init<FireAction>(action)->which = FireAction::Which::BEAM; return true;
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
                    return read_from_absolute(sub, init<MoveAction>(action));
                case kAlterWeapon1:
                    return read_from(sub, EquipAction::Which::PULSE, init<EquipAction>(action));
                case kAlterWeapon2:
                    return read_from(sub, EquipAction::Which::BEAM, init<EquipAction>(action));
                case kAlterSpecial:
                    return read_from(sub, EquipAction::Which::SPECIAL, init<EquipAction>(action));
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
Action::Owner owner_cast(T t) {
    switch (t) {
        case static_cast<T>(Action::Owner::ANY): return Action::Owner::ANY;
        case static_cast<T>(Action::Owner::SAME): return Action::Owner::SAME;
        case static_cast<T>(Action::Owner::DIFFERENT): return Action::Owner::DIFFERENT;
    }
    return owner_cast(Action::Owner::ANY);
}

}  // namespace

bool read_from(pn::file_view in, std::unique_ptr<const Action>* action) {
    uint8_t  verb, reflexive;
    uint32_t inclusive_filter, exclusive_filter;
    uint32_t delay;
    int16_t  owner, subject_override, object_override;
    pn::data section;
    section.resize(24);
    std::unique_ptr<Action> a;
    if (!(in.read(&verb, &reflexive, &inclusive_filter, &exclusive_filter, &owner, &delay,
                  &subject_override, &object_override, pn::pad(4), &section) &&
          read_argument(verb << 8, reflexive, &a, section.open()))) {
        return false;
    }

    if (a) {
        a->reflexive       = reflexive;
        a->inclusiveFilter = inclusive_filter;
        a->exclusiveFilter = exclusive_filter;
        if (exclusive_filter == 0xffffffff) {
            a->levelKeyTag = (inclusive_filter & kLevelKeyTag) >> kLevelKeyTagShift;
        } else {
            a->levelKeyTag = 0;
        }
        a->owner                  = owner_cast(owner);
        a->delay                  = ticks(delay);
        a->initialSubjectOverride = Handle<Level::Initial>(subject_override);
        a->initialDirectOverride  = Handle<Level::Initial>(object_override);
        *action                   = std::move(a);
    }
    return true;
}

std::vector<std::unique_ptr<const Action>> read_actions(int begin, int end) {
    if (end <= begin) {
        return std::vector<std::unique_ptr<const Action>>{};
    }

    std::vector<std::unique_ptr<const Action>> actions;
    actions.resize(end - begin);
    for (int i : sfz::range(begin, end)) {
        Resource r = Resource::path(pn::format("actions/{0}.bin", i));
        read_from(r.data().open(), &actions[i - begin]);
    }
    return actions;
}

Handle<BaseObject>  Action::created_base() const { return BaseObject::none(); }
std::pair<int, int> Action::sound_range() const { return std::make_pair(-1, -1); }
bool                Action::alters_owner() const { return false; }
bool                Action::check_conditions() const { return false; }

Handle<BaseObject> CreateAction::created_base() const { return base; }
Handle<BaseObject> MorphAction::created_base() const { return base; }
Handle<BaseObject> EquipAction::created_base() const { return base; }

std::pair<int, int> SoundAction::sound_range() const { return id; }

bool CaptureAction::alters_owner() const { return true; }

bool ScoreAction::check_conditions() const { return true; }
bool MessageAction::check_conditions() const { return true; }

}  // namespace antares

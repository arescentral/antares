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

bool read_from(pn::file_view in, EnableKeysAction* keys) { return in.read(&keys->enable); }

bool read_from(pn::file_view in, DisableKeysAction* keys) { return in.read(&keys->disable); }

bool read_from(pn::file_view in, ZoomAction* zoom) { return in.read(&zoom->value); }

bool read_from(pn::file_view in, SelectAction* select) {
    return in.read(&select->screen, &select->line);
}

bool read_from(pn::file_view in, AssumeAction* assume) { return in.read(&assume->which); }

bool read_argument(int verb, bool reflexive, Action* action, pn::file_view sub) {
    switch (static_cast<objectVerbIDEnum>(verb)) {
        case kReleaseEnergy:
        case kNoAction: action->init<NoAction>(); return true;

        case kSetDestination: action->init<RetargetAction>(); return true;
        case kActivateSpecial:
            action->init<FireAction>()->which = FireAction::Which::SPECIAL;
            return true;
        case kActivatePulse:
            action->init<FireAction>()->which = FireAction::Which::PULSE;
            return true;
        case kActivateBeam:
            action->init<FireAction>()->which = FireAction::Which::BEAM;
            return true;
        case kNilTarget: action->init<DetargetAction>(); return true;

        case kCreateObject: return read_from(sub, action->init<CreateAction>(), false);
        case kCreateObjectSetDest: return read_from(sub, action->init<CreateAction>(), true);

        case kPlaySound: return read_from(sub, action->init<SoundAction>());

        case kAlter: {
            uint8_t alter;
            if (!sub.read(&alter)) {
                return false;
            }
            verb |= alter;
            switch (static_cast<alterVerbIDType>(verb)) {
                case kAlterMaxThrust: action->init<NoAction>(); return true;
                case kAlterMaxTurnRate: action->init<NoAction>(); return true;
                case kAlterScale: action->init<NoAction>(); return true;
                case kAlterAttributes: action->init<NoAction>(); return true;
                case kAlterLevelKeyTag: action->init<NoAction>(); return true;
                case kAlterOrderKeyTag: action->init<NoAction>(); return true;
                case kAlterEngageKeyTag: action->init<NoAction>(); return true;

                case kAlterCloak: action->init<CloakAction>(); return true;

                case kAlterDamage: return read_from(sub, action->init<HealAction>());
                case kAlterEnergy: return read_from(sub, action->init<EnergizeAction>());
                case kAlterHidden: return read_from(sub, action->init<RevealAction>());
                case kAlterSpin: return read_from(sub, action->init<SpinAction>());
                case kAlterOffline: return read_from(sub, action->init<DisableAction>());
                case kAlterVelocity: return read_from(sub, reflexive, action->init<PushAction>());
                case kAlterMaxVelocity: return read_from(sub, action->init<CapSpeedAction>());
                case kAlterThrust: return read_from(sub, action->init<ThrustAction>());
                case kAlterBaseType: return read_from(sub, action->init<MorphAction>());
                case kAlterOwner: return read_from(sub, action->init<CaptureAction>());
                case kAlterConditionTrueYet:
                    return read_from(sub, action->init<ConditionAction>());
                case kAlterOccupation: return read_from(sub, action->init<OccupyAction>());
                case kAlterAbsoluteCash: return read_from(sub, action->init<PayAction>());
                case kAlterAge: return read_from(sub, action->init<AgeAction>());
                case kAlterLocation:
                    return read_from_relative(sub, reflexive, action->init<MoveAction>());
                case kAlterAbsoluteLocation:
                    return read_from_absolute(sub, action->init<MoveAction>());
                case kAlterWeapon1:
                    return read_from(sub, EquipAction::Which::PULSE, action->init<EquipAction>());
                case kAlterWeapon2:
                    return read_from(sub, EquipAction::Which::BEAM, action->init<EquipAction>());
                case kAlterSpecial:
                    return read_from(
                            sub, EquipAction::Which::SPECIAL, action->init<EquipAction>());
            }
        }

        case kMakeSparks: return read_from(sub, action->init<SparkAction>());

        case kLandAt: return read_from(sub, action->init<LandAction>());

        case kEnterWarp: action->init<WarpAction>(); return true;

        case kDisplayMessage: return read_from(sub, action->init<MessageAction>());

        case kChangeScore: return read_from(sub, action->init<ScoreAction>());

        case kDeclareWinner: return read_from(sub, action->init<WinAction>());

        case kDie: return read_from(sub, action->init<KillAction>());

        case kColorFlash: return read_from(sub, action->init<FlashAction>());

        case kDisableKeys: return read_from(sub, action->init<DisableKeysAction>());
        case kEnableKeys: return read_from(sub, action->init<EnableKeysAction>());

        case kSetZoom: return read_from(sub, action->init<ZoomAction>());

        case kComputerSelect: return read_from(sub, action->init<SelectAction>());

        case kAssumeInitialObject: return read_from(sub, action->init<AssumeAction>());
    }
}

template <typename T>
ActionBase::Owner owner_cast(T t) {
    switch (t) {
        case static_cast<T>(ActionBase::Owner::ANY): return ActionBase::Owner::ANY;
        case static_cast<T>(ActionBase::Owner::SAME): return ActionBase::Owner::SAME;
        case static_cast<T>(ActionBase::Owner::DIFFERENT): return ActionBase::Owner::DIFFERENT;
    }
    return owner_cast(ActionBase::Owner::ANY);
}

}  // namespace

bool read_from(pn::file_view in, Action* action) {
    uint8_t  verb, reflexive;
    uint32_t inclusive_filter, exclusive_filter;
    uint32_t delay;
    int16_t  owner, subject_override, object_override;
    pn::data section;
    section.resize(24);
    if (!(in.read(&verb, &reflexive, &inclusive_filter, &exclusive_filter, &owner, &delay,
                  &subject_override, &object_override, pn::pad(4), &section) &&
          read_argument(verb << 8, reflexive, action, section.open()))) {
        return false;
    }

    if (*action) {
        auto& base            = *action;
        base->reflexive       = reflexive;
        base->inclusiveFilter = inclusive_filter;
        base->exclusiveFilter = exclusive_filter;
        if (exclusive_filter == 0xffffffff) {
            base->levelKeyTag = (inclusive_filter & kLevelKeyTag) >> kLevelKeyTagShift;
        } else {
            base->levelKeyTag = 0;
        }
        base->owner                  = owner_cast(owner);
        base->delay                  = ticks(delay);
        base->initialSubjectOverride = Handle<Level::Initial>(subject_override);
        base->initialDirectOverride  = Handle<Level::Initial>(object_override);
    }
    return true;
}

std::vector<Action> read_actions(int begin, int end) {
    if (end <= begin) {
        return std::vector<Action>{};
    }
    Resource r = Resource::path("actions.bin");

    pn::data_view d = r.data();
    if ((begin < 0) || ((d.size() / ActionBase::byte_size) < end)) {
        throw std::runtime_error(pn::format(
                                         "action range {{{0}, {1}}} outside {{0, {2}}}", begin,
                                         end, d.size() / ActionBase::byte_size)
                                         .c_str());
    }

    int      count = end - begin;
    pn::file f     = d.slice(ActionBase::byte_size * begin, ActionBase::byte_size * count).open();
    std::vector<Action> actions;
    actions.resize(count);
    for (Action& a : actions) {
        read_from(f, &a);
    }
    return actions;
}

Handle<BaseObject>  ActionBase::created_base() const { return BaseObject::none(); }
std::pair<int, int> ActionBase::sound_range() const { return std::make_pair(-1, -1); }
bool                ActionBase::alters_owner() const { return false; }
bool                ActionBase::check_conditions() const { return false; }
bool                ActionBase::should_end() const { return false; }

bool NoAction::should_end() const { return true; }

Handle<BaseObject> CreateAction::created_base() const { return base; }
Handle<BaseObject> MorphAction::created_base() const { return base; }
Handle<BaseObject> EquipAction::created_base() const { return base; }

std::pair<int, int> SoundAction::sound_range() const { return id; }

bool CaptureAction::alters_owner() const { return true; }

bool ScoreAction::check_conditions() const { return true; }
bool MessageAction::check_conditions() const { return true; }

}  // namespace antares

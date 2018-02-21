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

bool read_from(pn::file_view in, CreateObjectAction* create, bool inherit) {
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

bool read_from(pn::file_view in, PlaySoundAction* play) {
    uint8_t absolute;
    int32_t persistence;
    int32_t id_minimum, id_range;
    if (!in.read(
                &play->priority, pn::pad(1), &persistence, &absolute, pn::pad(1), &play->volume,
                pn::pad(4), &id_minimum, &id_range)) {
        return false;
    }
    play->id.first    = id_minimum;
    play->id.second   = id_minimum + id_range + 1;
    play->absolute    = absolute;
    play->persistence = ticks(persistence);
    return true;
}

bool read_from(pn::file_view in, argumentType::AlterSimple* argument) {
    uint8_t unused;
    return in.read(&unused, &argument->amount);
}

bool read_from(pn::file_view in, argumentType::AlterWeapon* argument) {
    uint8_t unused;
    int32_t base;
    if (!in.read(&unused, &base)) {
        return false;
    }
    argument->base = Handle<BaseObject>(base);
    return true;
}

bool read_from(pn::file_view in, argumentType::AlterAbsoluteLocation* argument) {
    return in.read(&argument->relative) && read_from(in, &argument->at);
}

bool read_from(pn::file_view in, argumentType::AlterHidden* argument) {
    uint8_t unused;
    return in.read(&unused, &argument->first, &argument->count_minus_1);
}

bool read_from(pn::file_view in, argumentType::AlterFixedRange* argument) {
    uint8_t unused;
    return in.read(&unused) && read_from(in, &argument->minimum) &&
           read_from(in, &argument->range);
}

bool read_from(pn::file_view in, argumentType::AlterVelocity* argument) {
    return in.read(&argument->relative) && read_from(in, &argument->amount);
}

bool read_from(pn::file_view in, argumentType::AlterMaxVelocity* argument) {
    uint8_t unused;
    return in.read(&unused) && read_from(in, &argument->amount);
}

bool read_from(pn::file_view in, argumentType::AlterThrust* argument) {
    return in.read(&argument->relative) && read_from(in, &argument->minimum) &&
           read_from(in, &argument->range);
}

bool read_from(pn::file_view in, argumentType::AlterBaseType* argument) {
    int32_t base;
    if (!in.read(&argument->keep_ammo, &base)) {
        return false;
    }
    argument->base = Handle<BaseObject>(base);
    return true;
}

bool read_from(pn::file_view in, argumentType::AlterOwner* argument) {
    uint32_t admiral;
    if (!in.read(&argument->relative, &admiral)) {
        return false;
    }
    argument->admiral = Handle<Admiral>(admiral);
    return true;
}

bool read_from(pn::file_view in, argumentType::AlterConditionTrueYet* argument) {
    return in.read(&argument->true_yet, &argument->first, &argument->count_minus_1);
}

bool read_from(pn::file_view in, argumentType::AlterCash* argument) {
    uint32_t admiral;
    if (!(in.read(&argument->relative) && read_from(in, &argument->amount) && in.read(&admiral))) {
        return false;
    }
    argument->admiral = Handle<Admiral>(admiral);
    return true;
}

bool read_from(pn::file_view in, argumentType::AlterAge* argument) {
    int32_t minimum, range;
    if (!in.read(&argument->relative, &minimum, &range)) {
        return false;
    }
    argument->minimum = ticks(minimum);
    argument->range   = ticks(range);
    return true;
}

bool read_from(pn::file_view in, argumentType::AlterLocation* argument) {
    return in.read(&argument->relative, &argument->by);
}

bool read_from(pn::file_view in, MakeSparksAction* sparks) {
    return in.read(&sparks->count, &sparks->decay) && read_from(in, &sparks->velocity) &&
           in.read(&sparks->hue);
}

bool read_from(pn::file_view in, LandAtAction* land) { return in.read(&land->speed); }

bool read_from(pn::file_view in, DisplayMessageAction* message) {
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

bool read_from(pn::file_view in, ChangeScoreAction* score) {
    int32_t admiral;
    if (!in.read(&admiral, &score->which, &score->value)) {
        return false;
    }
    score->player = Handle<Admiral>(admiral);
    return true;
}

bool read_from(pn::file_view in, DeclareWinnerAction* win) {
    int32_t admiral;
    int32_t text_id;
    if (!in.read(&admiral, &win->next, &text_id)) {
        return false;
    }
    win->player = Handle<Admiral>(admiral);
    win->text   = Resource::text(text_id);
    return true;
}

bool read_from(pn::file_view in, DieAction* kill) {
    uint8_t kind;
    if (!in.read(&kind)) {
        return false;
    }
    kill->kind = static_cast<DieAction::Kind>(kind);
    return true;
}

bool read_from(pn::file_view in, ColorFlashAction* flash) {
    return in.read(&flash->length, &flash->hue, &flash->shade);
}

bool read_from(pn::file_view in, EnableKeysAction* keys) { return in.read(&keys->enable); }

bool read_from(pn::file_view in, DisableKeysAction* keys) { return in.read(&keys->disable); }

bool read_from(pn::file_view in, SetZoomAction* zoom) { return in.read(&zoom->value); }

bool read_from(pn::file_view in, ComputerSelectAction* select) {
    return in.read(&select->screen, &select->line);
}

bool read_from(pn::file_view in, AssumeInitialObjectAction* assume) {
    return in.read(&assume->which);
}

bool read_argument(int* composite_verb, Action* action, pn::file_view sub) {
    switch (static_cast<objectVerbIDEnum>(*composite_verb)) {
        case kReleaseEnergy:
        case kNoAction: action->init<NoAction>(); return true;

        case kSetDestination: action->init<SetDestinationAction>(); return true;
        case kActivateSpecial: action->init<ActivateSpecialAction>(); return true;
        case kActivatePulse: action->init<ActivatePulseAction>(); return true;
        case kActivateBeam: action->init<ActivateBeamAction>(); return true;
        case kNilTarget: action->init<NilTargetAction>(); return true;

        case kCreateObject: return read_from(sub, action->init<CreateObjectAction>(), false);
        case kCreateObjectSetDest: return read_from(sub, action->init<CreateObjectAction>(), true);

        case kPlaySound: return read_from(sub, action->init<PlaySoundAction>());

        case kAlter: {
            uint8_t alter;
            if (!sub.read(&alter)) {
                return false;
            }
            *composite_verb |= alter;
            switch (static_cast<alterVerbIDType>(*composite_verb)) {
                case kAlterMaxThrust: action->init<NoAction>(); return true;
                case kAlterMaxTurnRate: action->init<NoAction>(); return true;
                case kAlterScale: action->init<NoAction>(); return true;
                case kAlterAttributes: action->init<NoAction>(); return true;
                case kAlterLevelKeyTag: action->init<NoAction>(); return true;
                case kAlterOrderKeyTag: action->init<NoAction>(); return true;
                case kAlterEngageKeyTag: action->init<NoAction>(); return true;

                case kAlterCloak: action->init<AlterCloakAction>(); return true;

                case kAlterDamage:
                    return read_from(
                            sub, &action->init<AlterDamageAction>()->argument.alterDamage);
                case kAlterEnergy:
                    return read_from(
                            sub, &action->init<AlterEnergyAction>()->argument.alterEnergy);
                case kAlterHidden:
                    return read_from(
                            sub, &action->init<AlterHiddenAction>()->argument.alterHidden);
                case kAlterSpin:
                    return read_from(sub, &action->init<AlterSpinAction>()->argument.alterSpin);
                case kAlterOffline:
                    return read_from(
                            sub, &action->init<AlterOfflineAction>()->argument.alterOffline);
                case kAlterVelocity:
                    return read_from(
                            sub, &action->init<AlterVelocityAction>()->argument.alterVelocity);
                case kAlterMaxVelocity:
                    return read_from(
                            sub,
                            &action->init<AlterMaxVelocityAction>()->argument.alterMaxVelocity);
                case kAlterThrust:
                    return read_from(
                            sub, &action->init<AlterThrustAction>()->argument.alterThrust);
                case kAlterBaseType:
                    return read_from(
                            sub, &action->init<AlterBaseTypeAction>()->argument.alterBaseType);
                case kAlterOwner:
                    return read_from(sub, &action->init<AlterOwnerAction>()->argument.alterOwner);
                case kAlterConditionTrueYet:
                    return read_from(
                            sub, &action->init<AlterConditionTrueYetAction>()
                                          ->argument.alterConditionTrueYet);
                case kAlterOccupation:
                    return read_from(
                            sub, &action->init<AlterOccupationAction>()->argument.alterOccupation);
                case kAlterAbsoluteCash:
                    return read_from(
                            sub,
                            &action->init<AlterAbsoluteCashAction>()->argument.alterAbsoluteCash);
                case kAlterAge:
                    return read_from(sub, &action->init<AlterAgeAction>()->argument.alterAge);
                case kAlterLocation:
                    return read_from(
                            sub, &action->init<AlterLocationAction>()->argument.alterLocation);
                case kAlterAbsoluteLocation:
                    return read_from(
                            sub, &action->init<AlterAbsoluteLocationAction>()
                                          ->argument.alterAbsoluteLocation);
                case kAlterWeapon1:
                    return read_from(
                            sub, &action->init<AlterWeapon1Action>()->argument.alterWeapon);
                case kAlterWeapon2:
                    return read_from(
                            sub, &action->init<AlterWeapon2Action>()->argument.alterWeapon);
                case kAlterSpecial:
                    return read_from(
                            sub, &action->init<AlterSpecialAction>()->argument.alterWeapon);
            }
        }

        case kMakeSparks: return read_from(sub, action->init<MakeSparksAction>());

        case kLandAt: return read_from(sub, action->init<LandAtAction>());

        case kEnterWarp: action->init<EnterWarpAction>(); return true;

        case kDisplayMessage: return read_from(sub, action->init<DisplayMessageAction>());

        case kChangeScore: return read_from(sub, action->init<ChangeScoreAction>());

        case kDeclareWinner: return read_from(sub, action->init<DeclareWinnerAction>());

        case kDie: return read_from(sub, action->init<DieAction>());

        case kColorFlash: return read_from(sub, action->init<ColorFlashAction>());

        case kDisableKeys: return read_from(sub, action->init<DisableKeysAction>());
        case kEnableKeys: return read_from(sub, action->init<EnableKeysAction>());

        case kSetZoom: return read_from(sub, action->init<SetZoomAction>());

        case kComputerSelect: return read_from(sub, action->init<ComputerSelectAction>());

        case kAssumeInitialObject:
            return read_from(sub, action->init<AssumeInitialObjectAction>());
    }
}

}  // namespace

bool read_from(pn::file_view in, Action* action) {
    uint8_t  verb, reflexive;
    uint32_t inclusive_filter, exclusive_filter;
    uint32_t delay;
    int16_t  owner, subject_override, object_override;
    pn::data section;
    section.resize(24);
    if (!in.read(
                &verb, &reflexive, &inclusive_filter, &exclusive_filter, &owner, &delay,
                &subject_override, &object_override, pn::pad(4), &section)) {
        return false;
    }

    int composite_verb = verb << 8;
    read_argument(&composite_verb, action, section.open());

    if (*action) {
        auto& base            = *action;
        base->verb            = composite_verb;
        base->reflexive       = reflexive;
        base->inclusiveFilter = inclusive_filter;
        base->exclusiveFilter = exclusive_filter;
        if (exclusive_filter == 0xffffffff) {
            base->levelKeyTag = (inclusive_filter & kLevelKeyTag) >> kLevelKeyTagShift;
        } else {
            base->levelKeyTag = 0;
        }
        base->owner                  = owner;
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
Handle<BaseObject>  CreateObjectAction::created_base() const { return base; }
std::pair<int, int> ActionBase::sound_range() const { return std::make_pair(-1, -1); }
std::pair<int, int> PlaySoundAction::sound_range() const { return id; }

}  // namespace antares

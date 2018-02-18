// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// Copyright (C) 2008-2017 The Antares Authors
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

#include "game/action.hpp"

#include <set>
#include <sfz/sfz.hpp>

#include "data/base-object.hpp"
#include "data/plugin.hpp"
#include "data/resource.hpp"
#include "drawing/color.hpp"
#include "drawing/sprite-handling.hpp"
#include "game/admiral.hpp"
#include "game/condition.hpp"
#include "game/globals.hpp"
#include "game/initial.hpp"
#include "game/labels.hpp"
#include "game/level.hpp"
#include "game/messages.hpp"
#include "game/minicomputer.hpp"
#include "game/motion.hpp"
#include "game/non-player-ship.hpp"
#include "game/player-ship.hpp"
#include "game/space-object.hpp"
#include "game/starfield.hpp"
#include "game/sys.hpp"
#include "game/vector.hpp"
#include "lang/defines.hpp"
#include "math/macros.hpp"
#include "math/random.hpp"
#include "math/rotation.hpp"
#include "math/special.hpp"
#include "math/units.hpp"
#include "video/transitions.hpp"

using sfz::range;
using std::set;
using std::unique_ptr;

namespace antares {

const size_t kActionQueueLength = 120;

struct actionQueueType {
    const Action*       actionBegin;
    const Action*       actionEnd;
    ticks               scheduledTime;
    actionQueueType*    nextActionQueue;
    Handle<SpaceObject> subjectObject;
    int32_t             subjectObjectNum;
    int32_t             subjectObjectID;
    Handle<SpaceObject> directObject;
    int32_t             directObjectNum;
    int32_t             directObjectID;
    Point               offset;

    bool empty() const { return actionBegin == actionEnd; }
};

static ANTARES_GLOBAL actionQueueType* gFirstActionQueue = NULL;

static ANTARES_GLOBAL unique_ptr<actionQueueType[]> gActionQueueData;

#ifdef DATA_COVERAGE
ANTARES_GLOBAL set<int32_t> covered_actions;
#endif  // DATA_COVERAGE

static void queue_action(
        const Action* begin, const Action* end, ticks delayTime, Handle<SpaceObject> subjectObject,
        Handle<SpaceObject> directObject, Point* offset);

bool action_filter_applies_to(const Action& action, Handle<BaseObject> target) {
    if (action->exclusiveFilter == 0xffffffff) {
        return action->levelKeyTag == target->levelKeyTag;
    } else {
        return (action->inclusiveFilter & target->attributes) == action->inclusiveFilter;
    }
}

bool action_filter_applies_to(const Action& action, Handle<SpaceObject> target) {
    if (action->exclusiveFilter == 0xffffffff) {
        return action->levelKeyTag == target->baseType->levelKeyTag;
    } else {
        return (action->inclusiveFilter & target->attributes) == action->inclusiveFilter;
    }
}

void NoAction::apply(
        Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
        Point* offset) {}

void CreateObjectAction::apply(
        Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
        Point* offset) {
    const auto& create     = argument.createObject;
    const auto  baseObject = create.whichBaseType;
    auto        count      = create.howManyMinimum;
    if (create.howManyRange > 0) {
        count += subject->randomSeed.next(create.howManyRange);
    }
    for (int i = 0; i < count; ++i) {
        fixedPointType vel = {Fixed::zero(), Fixed::zero()};
        if (create.velocityRelative) {
            vel = subject->velocity;
        }
        int32_t direction = 0;
        if (baseObject->attributes & kAutoTarget) {
            direction = focus->targetAngle;
        } else if (create.directionRelative) {
            direction = subject->direction;
        }
        coordPointType at = subject->location;
        if (offset != NULL) {
            at.h += offset->h;
            at.v += offset->v;
        }

        const int32_t distance = create.randomDistance;
        if (distance > 0) {
            at.h += subject->randomSeed.next(distance * 2) - distance;
            at.v += subject->randomSeed.next(distance * 2) - distance;
        }

        auto product =
                CreateAnySpaceObject(baseObject, &vel, &at, direction, subject->owner, 0, -1);
        if (!product.get()) {
            continue;
        }

        if (product->attributes & kCanAcceptDestination) {
            uint32_t save_attributes = product->attributes;
            product->attributes &= ~kStaticDestination;
            if (product->owner.get()) {
                if (reflexive) {
                    if (verb != kCreateObjectSetDest) {
                        OverrideObjectDestination(product, subject);
                    } else if (subject->destObject.get()) {
                        OverrideObjectDestination(product, subject->destObject);
                    }
                }
            } else if (reflexive) {
                product->timeFromOrigin = kTimeToCheckHome;
                product->runTimeFlags &= ~kHasArrived;
                product->destObject       = subject;  // a->destinationObject;
                product->destObjectDest   = subject->destObject;
                product->destObjectID     = subject->id;
                product->destObjectDestID = subject->destObjectID;
            }
            product->attributes = save_attributes;
        }
        product->targetObject   = subject->targetObject;
        product->targetObjectID = subject->targetObjectID;
        product->closestObject  = product->targetObject;

        //  ugly though it is, we have to fill in the rest of
        //  a new beam's fields after it's created.
        if (product->attributes & kIsVector) {
            if (product->frame.vector->vectorKind != Vector::BOLT) {
                // special beams need special post-creation acts
                Vectors::set_attributes(product, subject);
            }
        }
    }
}

void PlaySoundAction::apply(
        Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
        Point* offset) {
    const auto& sound = argument.playSound;
    auto        id    = sound.idMinimum;
    if (sound.idRange > 0) {
        id += focus->randomSeed.next(sound.idRange + 1);
    }
    if (sound.absolute) {
        sys.sound.play(id, sound.volumeMinimum, sound.persistence, sound.priority);
    } else {
        sys.sound.play_at(id, sound.volumeMinimum, sound.persistence, sound.priority, focus);
    }
}

void MakeSparksAction::apply(
        Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
        Point* offset) {
    const auto& sparks = argument.makeSparks;
    Point       location;
    if (focus->sprite.get()) {
        location.h = focus->sprite->where.h;
        location.v = focus->sprite->where.v;
    } else {
        int32_t l = (focus->location.h - gGlobalCorner.h) * gAbsoluteScale;
        l >>= SHIFT_SCALE;
        if ((l > -kSpriteMaxSize) && (l < kSpriteMaxSize)) {
            location.h = l + viewport().left;
        } else {
            location.h = -kSpriteMaxSize;
        }

        l = (focus->location.v - gGlobalCorner.v) * gAbsoluteScale;
        l >>= SHIFT_SCALE; /*+ CLIP_TOP*/
        if ((l > -kSpriteMaxSize) && (l < kSpriteMaxSize)) {
            location.v = l + viewport().top;
        } else {
            location.v = -kSpriteMaxSize;
        }
    }
    globals()->starfield.make_sparks(
            sparks.howMany, sparks.speed, sparks.velocityRange, sparks.color, &location);
}

void DieAction::apply(
        Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
        Point* offset) {
    bool destroy = false;
    switch (argument.killObject.dieType) {
        case kDieExpire:
            if (subject.get()) {
                focus = subject;
            } else {
                return;
            }
            break;

        case kDieDestroy:
            if (subject.get()) {
                focus   = subject;
                destroy = true;
            } else {
                return;
            }
            break;
    }

    // if the object is occupied by a human, eject him since he can't die
    if ((focus->attributes & (kIsPlayerShip | kRemoteOrHuman)) &&
        !focus->baseType->destroyDontDie) {
        focus->create_floating_player_body();
    }
    if (destroy) {
        if (focus.get()) {
            focus->destroy();
        }
    } else {
        focus->active = kObjectToBeFreed;
    }
}

void NilTargetAction::apply(
        Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
        Point* offset) {
    focus->targetObject   = SpaceObject::none();
    focus->targetObjectID = kNoShip;
    focus->lastTarget     = SpaceObject::none();
}

void AlterDamageAction::apply(
        Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
        Point* offset) {
    const auto alter = argument.alterDamage;
    focus->alter_health(alter.amount);
}

void AlterEnergyAction::apply(
        Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
        Point* offset) {
    const auto alter = argument.alterEnergy;
    focus->alter_energy(alter.amount);
}

void AlterHiddenAction::apply(
        Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
        Point* offset) {
    const auto alter = argument.alterHidden;
    int32_t    begin = alter.first;
    int32_t    end   = begin + std::max(0, alter.count_minus_1) + 1;
    for (auto i : HandleList<Level::Initial>(begin, end)) {
        UnhideInitialObject(i);
    }
}

void AlterCloakAction::apply(
        Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
        Point* offset) {
    focus->set_cloak(true);
}

void AlterSpinAction::apply(
        Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
        Point* offset) {
    const auto alter = argument.alterSpin;
    if (focus->attributes & kCanTurn) {
        Fixed f  = focus->turn_rate() * (alter.minimum + focus->randomSeed.next(alter.range));
        Fixed f2 = focus->baseType->mass;
        if (f2 == Fixed::zero()) {
            f = kFixedNone;
        } else {
            f /= f2;
        }
        focus->turnVelocity = f;
    }
}

void AlterOfflineAction::apply(
        Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
        Point* offset) {
    const auto alter = argument.alterOffline;
    Fixed      f     = alter.minimum + focus->randomSeed.next(alter.range);
    Fixed      f2    = focus->baseType->mass;
    if (f2 == Fixed::zero()) {
        f = kFixedNone;
    } else {
        f /= f2;
    }
    focus->offlineTime = mFixedToLong(f);
}

void AlterVelocityAction::apply(
        Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
        Point* offset) {
    const auto alter = argument.alterVelocity;
    Fixed      f, f2;
    int16_t    angle;
    if (subject.get()) {
        // active (non-reflexive) altering of velocity means a PUSH, just like
        //  two objects colliding.  Negative velocity = slow down
        if (object.get()) {
            if (alter.relative) {
                if ((object->baseType->mass > Fixed::zero()) &&
                    (object->maxVelocity > Fixed::zero())) {
                    if (alter.amount >= Fixed::zero()) {
                        // if the amount >= 0, then PUSH the object like collision
                        f = subject->velocity.h - object->velocity.h;
                        f /= object->baseType->mass.val();
                        f <<= 6L;
                        object->velocity.h += f;
                        f = subject->velocity.v - object->velocity.v;
                        f /= object->baseType->mass.val();
                        f <<= 6L;
                        object->velocity.v += f;

                        // make sure we're not going faster than our top speed
                        angle = ratio_to_angle(object->velocity.h, object->velocity.v);
                    } else {
                        // if the minumum < 0, then STOP the object like applying breaks
                        f = object->velocity.h;
                        f = f * alter.amount;
                        object->velocity.h += f;
                        f = object->velocity.v;
                        f = f * alter.amount;
                        object->velocity.v += f;

                        // make sure we're not going faster than our top speed
                        angle = ratio_to_angle(object->velocity.h, object->velocity.v);
                    }

                    // get the maxthrust of new vector
                    GetRotPoint(&f, &f2, angle);
                    f  = object->maxVelocity * f;
                    f2 = object->maxVelocity * f2;

                    if (f < Fixed::zero()) {
                        if (object->velocity.h < f) {
                            object->velocity.h = f;
                        }
                    } else {
                        if (object->velocity.h > f) {
                            object->velocity.h = f;
                        }
                    }

                    if (f2 < Fixed::zero()) {
                        if (object->velocity.v < f2) {
                            object->velocity.v = f2;
                        }
                    } else {
                        if (object->velocity.v > f2) {
                            object->velocity.v = f2;
                        }
                    }
                }
            } else {
                GetRotPoint(&f, &f2, subject->direction);
                f                 = alter.amount * f;
                f2                = alter.amount * f2;
                focus->velocity.h = f;
                focus->velocity.v = f2;
            }
        } else {
            // reflexive alter velocity means a burst of speed in the direction
            // the object is facing, where negative speed means backwards. Object can
            // excede its max velocity.
            // Minimum value is absolute speed in direction.
            GetRotPoint(&f, &f2, focus->direction);
            f  = alter.amount * f;
            f2 = alter.amount * f2;
            if (alter.relative) {
                focus->velocity.h += f;
                focus->velocity.v += f2;
            } else {
                focus->velocity.h = f;
                focus->velocity.v = f2;
            }
        }
    }
}

void AlterMaxVelocityAction::apply(
        Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
        Point* offset) {
    const auto alter = argument.alterMaxVelocity;
    if (alter.amount < Fixed::zero()) {
        focus->maxVelocity = focus->baseType->maxVelocity;
    } else {
        focus->maxVelocity = alter.amount;
    }
}

void AlterThrustAction::apply(
        Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
        Point* offset) {
    const auto alter = argument.alterThrust;
    Fixed      f     = alter.minimum + focus->randomSeed.next(alter.range);
    if (alter.relative) {
        focus->thrust += f;
    } else {
        focus->thrust = f;
    }
}

void AlterBaseTypeAction::apply(
        Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
        Point* offset) {
    const auto alter = argument.alterBaseType;
    if (reflexive || object.get()) {
        focus->change_base_type(alter.base, -1, alter.keep_ammo);
    }
}

void AlterOwnerAction::apply(
        Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
        Point* offset) {
    const auto alter = argument.alterOwner;
    if (!focus.get()) {
        return;
    }
    if (alter.relative) {
        // if it's relative AND reflexive, we take the direct
        // object's owner, since relative & reflexive would
        // do nothing.
        if (reflexive && object.get()) {
            focus->set_owner(object->owner, true);
        } else {
            focus->set_owner(subject->owner, true);
        }
    } else {
        focus->set_owner(alter.admiral, false);
    }
}

void AlterConditionTrueYetAction::apply(
        Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
        Point* offset) {
    const auto alter = argument.alterConditionTrueYet;
    int32_t    begin = alter.first;
    int32_t    end   = begin + std::max(0, alter.count_minus_1) + 1;
    for (auto l : range(begin, end)) {
        g.condition_enabled[l] = !alter.true_yet;
    }
}

void AlterOccupationAction::apply(
        Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
        Point* offset) {
    const auto alter = argument.alterOccupation;
    if (focus.get()) {
        focus->alter_occupation(subject->owner, alter.amount, true);
    }
}

void AlterAbsoluteCashAction::apply(
        Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
        Point* offset) {
    const auto      alter = argument.alterAbsoluteCash;
    Handle<Admiral> admiral;
    if (alter.relative) {
        if (focus.get()) {
            admiral = focus->owner;
        }
    } else {
        admiral = alter.admiral;
    }
    if (admiral.get()) {
        admiral->pay_absolute(alter.amount);
    }
}

void AlterAgeAction::apply(
        Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
        Point* offset) {
    const auto alter = argument.alterAge;
    ticks      t     = alter.minimum + focus->randomSeed.next(alter.range);

    if (alter.relative) {
        if (focus->expires) {
            focus->expire_after += t;
        } else {
            focus->expire_after += t;
            focus->expires = (focus->expire_after >= ticks(0));
        }
    } else {
        focus->expire_after = t;
        focus->expires      = (focus->expire_after >= ticks(0));
    }
}

void AlterLocationAction::apply(
        Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
        Point* offset) {
    const auto     alter = argument.alterLocation;
    coordPointType newLocation;
    if (alter.relative) {
        if (object.get()) {
            newLocation.h = subject->location.h;
            newLocation.v = subject->location.v;
        } else {
            newLocation.h = object->location.h;
            newLocation.v = object->location.v;
        }
    } else {
        newLocation.h = newLocation.v = 0;
    }
    newLocation.h += focus->randomSeed.next(alter.by << 1) - alter.by;
    newLocation.v += focus->randomSeed.next(alter.by << 1) - alter.by;
    focus->location.h = newLocation.h;
    focus->location.v = newLocation.v;
}

void AlterAbsoluteLocationAction::apply(
        Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
        Point* offset) {
    const auto alter = argument.alterAbsoluteLocation;
    if (alter.relative) {
        focus->location.h += alter.at.h;
        focus->location.v += alter.at.v;
    } else {
        focus->location = Translate_Coord_To_Level_Rotation(alter.at.h, alter.at.v);
    }
}

static void alter_weapon(
        const ActionBase& action, Handle<SpaceObject> focus, SpaceObject::Weapon& weapon) {
    const auto alter = action.argument.alterWeapon;
    weapon.base      = alter.base;
    if (weapon.base.get()) {
        auto baseObject = weapon.base;
        weapon.ammo     = baseObject->frame.weapon.ammo;
        weapon.time     = g.time;
        weapon.position = 0;
        if (baseObject->frame.weapon.range > focus->longestWeaponRange) {
            focus->longestWeaponRange = baseObject->frame.weapon.range;
        }
        if (baseObject->frame.weapon.range < focus->shortestWeaponRange) {
            focus->shortestWeaponRange = baseObject->frame.weapon.range;
        }
    } else {
        weapon.base = BaseObject::none();
        weapon.ammo = 0;
        weapon.time = g.time;
    }
}

void AlterWeapon1Action::apply(
        Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
        Point* offset) {
    alter_weapon(*this, focus, focus->pulse);
}

void AlterWeapon2Action::apply(
        Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
        Point* offset) {
    alter_weapon(*this, focus, focus->beam);
}

void AlterSpecialAction::apply(
        Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
        Point* offset) {
    alter_weapon(*this, focus, focus->special);
}

void LandAtAction::apply(
        Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
        Point* offset) {
    // even though this is never a reflexive verb, we only effect ourselves
    if (subject->attributes & (kIsPlayerShip | kRemoteOrHuman)) {
        subject->create_floating_player_body();
    }
    subject->presenceState          = kLandingPresence;
    subject->presence.landing.speed = argument.landAt.landingSpeed;
    subject->presence.landing.scale = subject->baseType->naturalScale;
}

void EnterWarpAction::apply(
        Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
        Point* offset) {
    subject->presenceState             = kWarpInPresence;
    subject->presence.warp_in.progress = ticks(0);
    subject->presence.warp_in.step     = 0;
    subject->attributes &= ~kOccupiesSpace;
    fixedPointType newVel = {Fixed::zero(), Fixed::zero()};
    CreateAnySpaceObject(
            plug.info.warpInFlareID, &newVel, &subject->location, subject->direction,
            Admiral::none(), 0, -1);
}

void ChangeScoreAction::apply(
        Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
        Point* offset) {
    const auto&     score   = argument.changeScore;
    Handle<Admiral> admiral = score.whichPlayer;
    if ((!score.whichPlayer.get() && focus.get())) {
        admiral = focus->owner;
    }
    if (admiral.get()) {
        AlterAdmiralScore(admiral, score.whichScore, score.amount);
    }
}

void DeclareWinnerAction::apply(
        Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
        Point* offset) {
    const auto&     winner  = argument.declareWinner;
    Handle<Admiral> admiral = winner.whichPlayer;
    if ((!winner.whichPlayer.get() && focus.get())) {
        admiral = focus->owner;
    }
    DeclareWinner(admiral, winner.nextLevel, winner.text);
}

void DisplayMessageAction::apply(
        Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
        Point* offset) {
    const auto&             message = argument.displayMessage;
    std::vector<pn::string> pages;
    for (pn::string_view s : message.pages) {
        pages.push_back(s.copy());
    }
    Messages::start(message.resID, std::move(pages));
}

void SetDestinationAction::apply(
        Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
        Point* offset) {
    uint32_t save_attributes = subject->attributes;
    subject->attributes &= ~kStaticDestination;
    OverrideObjectDestination(subject, focus);
    subject->attributes = save_attributes;
}

void ActivateSpecialAction::apply(
        Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
        Point* offset) {
    fire_weapon(subject, SpaceObject::none(), subject->baseType->special, subject->special);
}

void ActivatePulseAction::apply(
        Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
        Point* offset) {
    fire_weapon(subject, SpaceObject::none(), subject->baseType->pulse, subject->pulse);
}

void ActivateBeamAction::apply(
        Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
        Point* offset) {
    fire_weapon(subject, SpaceObject::none(), subject->baseType->beam, subject->beam);
}

void ColorFlashAction::apply(
        Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
        Point* offset) {
    uint8_t tinyColor =
            GetTranslateColorShade(argument.colorFlash.color, argument.colorFlash.shade);
    globals()->transitions.start_boolean(
            argument.colorFlash.length, argument.colorFlash.length, tinyColor);
}

void EnableKeysAction::apply(
        Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
        Point* offset) {
    g.key_mask = g.key_mask & ~argument.keys.keyMask;
}

void DisableKeysAction::apply(
        Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
        Point* offset) {
    g.key_mask = g.key_mask | argument.keys.keyMask;
}

void SetZoomAction::apply(
        Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
        Point* offset) {
    if (argument.zoom.zoomLevel != g.zoom) {
        g.zoom = static_cast<ZoomType>(argument.zoom.zoomLevel);
        sys.sound.click();
        Messages::zoom(g.zoom);
    }
}

void ComputerSelectAction::apply(
        Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
        Point* offset) {
    MiniComputer_SetScreenAndLineHack(
            argument.computerSelect.screenNumber, argument.computerSelect.lineNumber);
}

void AssumeInitialObjectAction::apply(
        Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
        Point* offset) {
    Handle<Admiral> player1(0);
    int index            = argument.assumeInitial.whichInitialObject + GetAdmiralScore(player1, 0);
    g.initials[index]    = focus;
    g.initial_ids[index] = focus->id;
}

static void execute_actions(
        const Action* begin, const Action* end, const Handle<SpaceObject> original_subject,
        const Handle<SpaceObject> original_object, Point* offset, bool allowDelay) {
    bool checkConditions = false;

    for (const Action* curr : sfz::range(begin, end)) {
        const Action& action = *curr;
#ifdef DATA_COVERAGE
        covered_actions.insert(action.number());
#endif  // DATA_COVERAGE

        if (action->verb == kNoAction) {
            break;
        }
        auto subject = original_subject;
        if (action->initialSubjectOverride.number() != kNoShip) {
            subject = GetObjectFromInitialNumber(
                    Handle<Level::Initial>(action->initialSubjectOverride));
        }
        auto object = original_object;
        if (action->initialDirectOverride.number() != kNoShip) {
            object = GetObjectFromInitialNumber(
                    Handle<Level::Initial>(action->initialDirectOverride));
        }

        if ((action->delay > ticks(0)) && allowDelay) {
            queue_action(curr, end, action->delay, subject, object, offset);
            return;
        }
        allowDelay = true;

        auto focus = object;
        if (action->reflexive || !focus.get()) {
            focus = subject;
        }

        if (object.get() && subject.get()) {
            if ((action->owner < -1) ||
                ((action->owner == -1) && (object->owner == subject->owner)) ||
                ((action->owner == 1) && (object->owner != subject->owner)) ||
                (action->owner > 1)) {
                continue;
            }
        }

        if ((action->inclusiveFilter || action->exclusiveFilter) &&
            (!object.get() || !action_filter_applies_to(action, object))) {
            continue;
        }

        pn::format(stderr, "{0}\n", action->verb);
        action->apply(subject, focus, object, offset);
        switch (action->verb) {
            case kChangeScore:
            case kDisplayMessage: checkConditions = true; break;
        }
    }

    if (checkConditions) {
        CheckLevelConditions();
    }
}

void exec(
        const std::vector<Action>& actions, Handle<SpaceObject> sObject,
        Handle<SpaceObject> dObject, Point* offset) {
    execute_actions(
            actions.data(), actions.data() + actions.size(), sObject, dObject, offset, true);
}

void reset_action_queue() {
    gActionQueueData.reset(new actionQueueType[kActionQueueLength]);

    gFirstActionQueue = NULL;

    actionQueueType* action = gActionQueueData.get();
    for (int32_t i = 0; i < kActionQueueLength; i++) {
        action->actionBegin = action->actionEnd = nullptr;
        ++action;
    }
}

static void queue_action(
        const Action* begin, const Action* end, ticks delayTime, Handle<SpaceObject> subjectObject,
        Handle<SpaceObject> directObject, Point* offset) {
    int32_t          queueNumber = 0;
    actionQueueType* actionQueue = gActionQueueData.get();
    while (!actionQueue->empty() && (queueNumber < kActionQueueLength)) {
        actionQueue++;
        queueNumber++;
    }

    if (queueNumber == kActionQueueLength) {
        return;
    }
    actionQueue->actionBegin   = begin;
    actionQueue->actionEnd     = end;
    actionQueue->scheduledTime = delayTime;

    if (offset) {
        actionQueue->offset = *offset;
    } else {
        actionQueue->offset = Point{0, 0};
    }

    actionQueue->subjectObject = subjectObject;
    if (subjectObject.get()) {
        actionQueue->subjectObjectNum = subjectObject->number();
        actionQueue->subjectObjectID  = subjectObject->id;
    } else {
        actionQueue->subjectObjectNum = -1;
        actionQueue->subjectObjectID  = -1;
    }

    actionQueue->directObject = directObject;
    if (directObject.get()) {
        actionQueue->directObjectNum = directObject->number();
        actionQueue->directObjectID  = directObject->id;
    } else {
        actionQueue->directObjectNum = -1;
        actionQueue->directObjectID  = -1;
    }

    actionQueueType* previousQueue = NULL;
    actionQueueType* nextQueue     = gFirstActionQueue;
    while (nextQueue && (nextQueue->scheduledTime < delayTime)) {
        previousQueue = nextQueue;
        nextQueue     = nextQueue->nextActionQueue;
    }
    if (previousQueue) {
        actionQueue->nextActionQueue = previousQueue->nextActionQueue;

        previousQueue->nextActionQueue = actionQueue;
    } else {
        actionQueue->nextActionQueue = gFirstActionQueue;
        gFirstActionQueue            = actionQueue;
    }
}

void execute_action_queue() {
    for (int32_t i = 0; i < kActionQueueLength; i++) {
        auto actionQueue = &gActionQueueData[i];
        if (!actionQueue->empty()) {
            actionQueue->scheduledTime -= kMajorTick;
        }
    }

    while (gFirstActionQueue && !gFirstActionQueue->empty() &&
           (gFirstActionQueue->scheduledTime <= ticks(0))) {
        int32_t subjectid = -1;
        if (gFirstActionQueue->subjectObject.get() && gFirstActionQueue->subjectObject->active) {
            subjectid = gFirstActionQueue->subjectObject->id;
        }

        int32_t directid = -1;
        if (gFirstActionQueue->directObject.get() && gFirstActionQueue->directObject->active) {
            directid = gFirstActionQueue->directObject->id;
        }
        if ((subjectid == gFirstActionQueue->subjectObjectID) &&
            (directid == gFirstActionQueue->directObjectID)) {
            execute_actions(
                    gFirstActionQueue->actionBegin, gFirstActionQueue->actionEnd,
                    gFirstActionQueue->subjectObject, gFirstActionQueue->directObject,
                    &gFirstActionQueue->offset, false);
        }

        gFirstActionQueue->actionBegin = gFirstActionQueue->actionEnd = nullptr;
        gFirstActionQueue = gFirstActionQueue->nextActionQueue;
    }
}

}  // namespace antares

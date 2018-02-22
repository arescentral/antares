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

void CreateAction::apply(
        Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
        Point* offset) {
    auto count = count_minimum;
    if (count_range > 0) {
        count += subject->randomSeed.next(count_range);
    }
    for (int i = 0; i < count; ++i) {
        fixedPointType vel = {Fixed::zero(), Fixed::zero()};
        if (relative_velocity) {
            vel = subject->velocity;
        }
        int32_t direction = 0;
        if (base->attributes & kAutoTarget) {
            direction = focus->targetAngle;
        } else if (relative_direction) {
            direction = subject->direction;
        }
        coordPointType at = subject->location;
        if (offset != NULL) {
            at.h += offset->h;
            at.v += offset->v;
        }

        if (distance > 0) {
            at.h += subject->randomSeed.next(distance * 2) - distance;
            at.v += subject->randomSeed.next(distance * 2) - distance;
        }

        auto product = CreateAnySpaceObject(base, &vel, &at, direction, subject->owner, 0, -1);
        if (!product.get()) {
            continue;
        }

        if (product->attributes & kCanAcceptDestination) {
            uint32_t save_attributes = product->attributes;
            product->attributes &= ~kStaticDestination;
            if (product->owner.get()) {
                if (reflexive) {
                    if (!inherit) {
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

void SoundAction::apply(
        Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
        Point* offset) {
    auto    pick  = id.first;
    int32_t range = id.second - id.first;
    if (range > 1) {
        pick += focus->randomSeed.next(range);
    }
    if (absolute) {
        sys.sound.play(pick, volume, persistence, priority);
    } else {
        sys.sound.play_at(pick, volume, persistence, priority, focus);
    }
}

void SparkAction::apply(
        Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
        Point* offset) {
    Point location;
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
    globals()->starfield.make_sparks(count, decay, velocity, hue, &location);
}

void KillAction::apply(
        Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
        Point* offset) {
    bool destroy = false;
    switch (kind) {
        case Kind::EXPIRE:
            if (subject.get()) {
                focus = subject;
            } else {
                return;
            }
            break;

        case Kind::DESTROY:
            if (subject.get()) {
                focus   = subject;
                destroy = true;
            } else {
                return;
            }
            break;

        case Kind::NONE: break;
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

void DetargetAction::apply(
        Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
        Point* offset) {
    focus->targetObject   = SpaceObject::none();
    focus->targetObjectID = kNoShip;
    focus->lastTarget     = SpaceObject::none();
}

void HealAction::apply(
        Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
        Point* offset) {
    focus->alter_health(value);
}

void EnergizeAction::apply(
        Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
        Point* offset) {
    focus->alter_energy(value);
}

void RevealAction::apply(
        Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
        Point* offset) {
    for (auto i : initial) {
        UnhideInitialObject(i);
    }
}

void CloakAction::apply(
        Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
        Point* offset) {
    focus->set_cloak(true);
}

void SpinAction::apply(
        Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
        Point* offset) {
    if (focus->attributes & kCanTurn) {
        Fixed f = focus->turn_rate() *
                  (value.first + focus->randomSeed.next(value.second - value.first));
        Fixed f2 = focus->baseType->mass;
        if (f2 == Fixed::zero()) {
            f = kFixedNone;
        } else {
            f /= f2;
        }
        focus->turnVelocity = f;
    }
}

void DisableAction::apply(
        Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
        Point* offset) {
    Fixed f  = value.first + focus->randomSeed.next(value.second - value.first);
    Fixed f2 = focus->baseType->mass;
    if (f2 == Fixed::zero()) {
        f = kFixedNone;
    } else {
        f /= f2;
    }
    focus->offlineTime = mFixedToLong(f);
}

void cap_velocity(Handle<SpaceObject> object) {
    int16_t angle = ratio_to_angle(object->velocity.h, object->velocity.v);
    Fixed   f, f2;

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

void PushAction::apply(
        Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
        Point* offset) {
    if (!subject.get()) {
        return;
    }

    switch (kind) {
        case Kind::STOP: focus->velocity = {Fixed::zero(), Fixed::zero()}; break;

        case Kind::BOOST: {
            Fixed fx, fy;
            GetRotPoint(&fx, &fy, focus->direction);
            focus->velocity.h += value * fx;
            focus->velocity.v += value * fy;
            break;
        }

        case Kind::CRUISE: {
            Fixed fx, fy;
            GetRotPoint(&fx, &fy, focus->direction);
            focus->velocity = {value * fx, value * fy};
            break;
        }

        case Kind::SET: {
            Fixed fx, fy;
            GetRotPoint(&fx, &fy, subject->direction);
            focus->velocity = {value * fx, value * fy};
            break;
        }

        case Kind::COLLIDE: {
            if ((focus->baseType->mass <= Fixed::zero()) ||
                (focus->maxVelocity <= Fixed::zero())) {
                return;
            }

            // if colliding, then PUSH the focus like collision
            focus->velocity.h +=
                    ((subject->velocity.h - focus->velocity.h) / focus->baseType->mass.val())
                    << 6L;
            focus->velocity.v +=
                    ((subject->velocity.v - focus->velocity.v) / focus->baseType->mass.val())
                    << 6L;

            // make sure we're not going faster than our top speed
            cap_velocity(focus);
            break;
        }

        case Kind::DECELERATE: {
            if ((focus->baseType->mass <= Fixed::zero()) ||
                (focus->maxVelocity <= Fixed::zero())) {
                return;
            }

            // if decelerating, then STOP the focus like applying brakes
            focus->velocity.h += focus->velocity.h * -value;
            focus->velocity.v += focus->velocity.v * -value;

            // make sure we're not going faster than our top speed
            cap_velocity(focus);
            break;
        }
    }
}

void CapSpeedAction::apply(
        Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
        Point* offset) {
    if (value < Fixed::zero()) {
        focus->maxVelocity = focus->baseType->maxVelocity;
    } else {
        focus->maxVelocity = value;
    }
}

void ThrustAction::apply(
        Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
        Point* offset) {
    Fixed f = value.first + focus->randomSeed.next(value.second - value.first);
    if (relative) {
        focus->thrust += f;
    } else {
        focus->thrust = f;
    }
}

void MorphAction::apply(
        Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
        Point* offset) {
    if (focus.get()) {
        focus->change_base_type(base, -1, keep_ammo);
    }
}

void CaptureAction::apply(
        Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
        Point* offset) {
    if (!focus.get()) {
        return;
    }
    if (relative) {
        // if it's relative AND reflexive, we take the direct
        // object's owner, since relative & reflexive would
        // do nothing.
        if (reflexive && object.get()) {
            focus->set_owner(object->owner, true);
        } else {
            focus->set_owner(subject->owner, true);
        }
    } else {
        focus->set_owner(player, false);
    }
}

void ConditionAction::apply(
        Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
        Point* offset) {
    for (auto l = which.first; l < which.second; ++l) {
        g.condition_enabled[l] = enabled;
    }
}

void OccupyAction::apply(
        Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
        Point* offset) {
    if (focus.get()) {
        focus->alter_occupation(subject->owner, value, true);
    }
}

void PayAction::apply(
        Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
        Point* offset) {
    Handle<Admiral> admiral;
    if (relative) {
        if (focus.get()) {
            admiral = focus->owner;
        }
    } else {
        admiral = player;
    }
    if (admiral.get()) {
        admiral->pay_absolute(value);
    }
}

void AgeAction::apply(
        Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
        Point* offset) {
    ticks t = value.first + focus->randomSeed.next(value.second - value.first);

    if (relative) {
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

void MoveAction::apply(
        Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
        Point* offset) {
    coordPointType newLocation;
    switch (origin) {
        case LEVEL: newLocation = Translate_Coord_To_Level_Rotation(to.h, to.v); break;
        case SUBJECT: newLocation = subject->location; break;
        case OBJECT: newLocation = object->location; break;
        case FOCUS: newLocation = focus->location; break;
    }
    newLocation.h += focus->randomSeed.next(distance << 1) - distance;
    newLocation.v += focus->randomSeed.next(distance << 1) - distance;
    focus->location.h = newLocation.h;
    focus->location.v = newLocation.v;
}

static void alter_weapon(
        Handle<BaseObject> base, Handle<SpaceObject> focus, SpaceObject::Weapon& weapon) {
    weapon.base = base;
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

void EquipAction::apply(
        Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
        Point* offset) {
    switch (which) {
        case Which::PULSE: alter_weapon(base, focus, focus->pulse); break;
        case Which::BEAM: alter_weapon(base, focus, focus->beam); break;
        case Which::SPECIAL: alter_weapon(base, focus, focus->special); break;
    }
}

void LandAction::apply(
        Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
        Point* offset) {
    // even though this is never a reflexive verb, we only effect ourselves
    if (subject->attributes & (kIsPlayerShip | kRemoteOrHuman)) {
        subject->create_floating_player_body();
    }
    subject->presenceState          = kLandingPresence;
    subject->presence.landing.speed = speed;
    subject->presence.landing.scale = subject->baseType->naturalScale;
}

void WarpAction::apply(
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

void ScoreAction::apply(
        Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
        Point* offset) {
    Handle<Admiral> admiral = player;
    if (!admiral.get() && focus.get()) {
        admiral = focus->owner;
    }
    if (admiral.get()) {
        AlterAdmiralScore(player, which, value);
    }
}

void WinAction::apply(
        Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
        Point* offset) {
    Handle<Admiral> admiral = player;
    if (!player.get() && focus.get()) {
        admiral = focus->owner;
    }
    DeclareWinner(admiral, next, text);
}

void MessageAction::apply(
        Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
        Point* offset) {
    Messages::start(id, &pages);
}

void RetargetAction::apply(
        Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
        Point* offset) {
    uint32_t save_attributes = subject->attributes;
    subject->attributes &= ~kStaticDestination;
    OverrideObjectDestination(subject, focus);
    subject->attributes = save_attributes;
}

void FireAction::apply(
        Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
        Point* offset) {
    switch (which) {
        case Which::PULSE:
            fire_weapon(subject, SpaceObject::none(), subject->baseType->pulse, subject->pulse);
            break;
        case Which::BEAM:
            fire_weapon(subject, SpaceObject::none(), subject->baseType->beam, subject->beam);
            break;
        case Which::SPECIAL:
            fire_weapon(
                    subject, SpaceObject::none(), subject->baseType->special, subject->special);
            break;
    }
}

void FlashAction::apply(
        Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
        Point* offset) {
    globals()->transitions.start_boolean(length, GetTranslateColorShade(hue, shade));
}

void KeyAction::apply(
        Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
        Point* offset) {
    g.key_mask = (g.key_mask & ~enable) | disable;
}

void ZoomAction::apply(
        Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
        Point* offset) {
    if (value != g.zoom) {
        g.zoom = static_cast<ZoomType>(value);
        sys.sound.click();
        Messages::zoom(g.zoom);
    }
}

void SelectAction::apply(
        Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
        Point* offset) {
    MiniComputer_SetScreenAndLineHack(screen, line);
}

void AssumeAction::apply(
        Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
        Point* offset) {
    Handle<Admiral> player1(0);
    int             index = which + GetAdmiralScore(player1, 0);
    g.initials[index]     = focus;
    g.initial_ids[index]  = focus->id;
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

        if (action->should_end()) {
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
            if (((action->owner == ActionBase::Owner::DIFFERENT) &&
                 (object->owner == subject->owner)) ||
                ((action->owner == ActionBase::Owner::SAME) &&
                 (object->owner != subject->owner))) {
                continue;
            }
        }

        if ((action->inclusiveFilter || action->exclusiveFilter) &&
            (!object.get() || !action_filter_applies_to(action, object))) {
            continue;
        }

        action->apply(subject, focus, object, offset);
        checkConditions = checkConditions || action->check_conditions();
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

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

#include "game/space-object.hpp"

#include <set>
#include <sfz/sfz.hpp>

#include "data/base-object.hpp"
#include "data/plugin.hpp"
#include "data/resource.hpp"
#include "data/string-list.hpp"
#include "drawing/color.hpp"
#include "drawing/sprite-handling.hpp"
#include "game/action.hpp"
#include "game/admiral.hpp"
#include "game/globals.hpp"
#include "game/labels.hpp"
#include "game/level.hpp"
#include "game/messages.hpp"
#include "game/minicomputer.hpp"
#include "game/motion.hpp"
#include "game/player-ship.hpp"
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

using sfz::BytesSlice;
using sfz::Exception;
using sfz::ReadSource;
using sfz::String;
using sfz::StringSlice;
using sfz::read;
using std::max;
using std::min;
using std::set;
using std::unique_ptr;

namespace antares {

const uint8_t kFriendlyColor = GREEN;
const uint8_t kHostileColor  = RED;
const uint8_t kNeutralColor  = SKY_BLUE;

const Fixed kDefaultTurnRate = Fixed::from_long(2.000);

#ifdef DATA_COVERAGE
ANTARES_GLOBAL set<int32_t> covered_objects;
#endif  // DATA_COVERAGE

void SpaceObjectHandlingInit() {
    g.objects.reset(new SpaceObject[kMaxSpaceObject]);
    ResetAllSpaceObjects();
    reset_action_queue();
}

void ResetAllSpaceObjects() {
    g.root = SpaceObject::none();
    for (auto anObject : SpaceObject::all()) {
        anObject->active = kObjectAvailable;
        anObject->sprite = Sprite::none();
    }
}

BaseObject* BaseObject::get(int number) {
    if ((0 <= number) && (number < plug.objects.size())) {
        return &plug.objects[number];
    }
    return nullptr;
}

HandleList<BaseObject> BaseObject::all() {
    return HandleList<BaseObject>(0, plug.objects.size());
}

Action* Action::get(int32_t number) {
    if ((0 <= number) && (number < plug.actions.size())) {
        return &plug.actions[number];
    }
    return nullptr;
}

Handle<BaseObject> mGetBaseObjectFromClassRace(int class_, int race) {
    if (class_ >= kLiteralClass) {
        return Handle<BaseObject>(class_ - kLiteralClass);
    }
    for (auto o : BaseObject::all()) {
        if ((o->baseClass == class_) && (o->baseRace == race)) {
            return o;
        }
    }
    return BaseObject::none();
}

static Handle<SpaceObject> next_free_space_object() {
    for (auto obj : SpaceObject::all()) {
        if (!obj->active) {
            return obj;
        }
    }
    return SpaceObject::none();
}

static Handle<SpaceObject> AddSpaceObject(SpaceObject* sourceObject) {
    auto obj = next_free_space_object();
    if (!obj.get()) {
        return SpaceObject::none();
    }

    NatePixTable* spriteTable = nullptr;
    if (sourceObject->pixResID != kNoSpriteTable) {
        spriteTable = sys.pix.get(sourceObject->pixResID);
        if (!spriteTable) {
            throw Exception("Received an unexpected request to load a sprite");
        }
    }

    *obj = *sourceObject;

    Point where(
            (int32_t((obj->location.h - gGlobalCorner.h) * gAbsoluteScale) >> SHIFT_SCALE) +
                    viewport().left,
            (int32_t((obj->location.v - gGlobalCorner.v) * gAbsoluteScale) >> SHIFT_SCALE));

    if (obj->sprite.get()) {
        RemoveSprite(obj->sprite);
    }

    obj->sprite = Sprite::none();
    if (spriteTable) {
        uint8_t tinyShade;
        switch (obj->layer) {
            case kFirstSpriteLayer: tinyShade = MEDIUM; break;

            case kMiddleSpriteLayer: tinyShade = LIGHT; break;

            case kLastSpriteLayer: tinyShade = VERY_LIGHT; break;

            default: tinyShade = DARK; break;
        }

        RgbColor tinyColor;
        if (obj->tinySize == 0) {
            tinyColor = RgbColor::clear();
        } else if (obj->owner == g.admiral) {
            tinyColor = GetRGBTranslateColorShade(kFriendlyColor, tinyShade);
        } else if (obj->owner.get()) {
            tinyColor = GetRGBTranslateColorShade(kHostileColor, tinyShade);
        } else {
            tinyColor = GetRGBTranslateColorShade(kNeutralColor, tinyShade);
        }

        int16_t whichShape = 0;
        int16_t angle;
        if (obj->attributes & kIsSelfAnimated) {
            whichShape = more_evil_fixed_to_long(obj->frame.animation.thisShape);
        } else if (obj->attributes & kShapeFromDirection) {
            angle = obj->direction;
            mAddAngle(angle, obj->baseType->frame.rotation.rotRes >> 1);
            whichShape = angle / obj->baseType->frame.rotation.rotRes;
        }

        obj->sprite = AddSprite(
                where, spriteTable, sourceObject->pixResID, whichShape, obj->naturalScale,
                obj->tinySize, obj->layer, tinyColor);
        obj->tinyColor = tinyColor;

        if (!obj->sprite.get()) {
            g.game_over    = true;
            g.game_over_at = g.time;
            obj->active    = kObjectAvailable;
            return SpaceObject::none();
        }
    }

    if (obj->attributes & kIsVector) {
        const auto& vector = obj->baseType->frame.vector;
        obj->frame.vector  = Vectors::add(
                &(obj->location), vector.color, vector.kind, vector.accuracy, vector.range);
    }

    obj->nextObject     = g.root;
    obj->previousObject = SpaceObject::none();
    if (g.root.get()) {
        g.root->previousObject = obj;
    }
    g.root = obj;

    return obj;
}

void RemoveAllSpaceObjects() {
    for (auto obj : SpaceObject::all()) {
        if (obj->sprite.get()) {
            RemoveSprite(obj->sprite);
            obj->sprite = Sprite::none();
        }
        obj->active         = kObjectAvailable;
        obj->nextNearObject = obj->nextFarObject = SpaceObject::none();
        obj->attributes                          = 0;
    }
}

SpaceObject::SpaceObject(
        Handle<BaseObject> type, Random seed, int32_t object_id,
        const coordPointType& initial_location, int32_t relative_direction,
        fixedPointType* relative_velocity, Handle<Admiral> new_owner, int16_t spriteIDOverride) {
    base       = type;
    baseType   = type.get();
    active     = kObjectInUse;
    randomSeed = seed;
    owner      = new_owner;
    location   = initial_location;
    id         = object_id;
    sprite     = Sprite::none();

    attributes   = baseType->attributes;
    shieldColor  = baseType->shieldColor;
    tinySize     = baseType->tinySize;
    layer        = baseType->pixLayer;
    maxVelocity  = baseType->maxVelocity;
    naturalScale = baseType->naturalScale;

    _health  = max_health();
    _energy  = max_energy();
    _battery = max_battery();

    if (owner.get()) {
        myPlayerFlag = 1 << owner.number();
    }

    // We used to irrelevantly set 'id' here.  Now, we just cycle
    // through values to maintain replay-compatibility of randomSeed.
    while (randomSeed.next(-32768) == -1) {
        continue;
    }

    if (baseType->activatePeriod != ticks(0)) {
        periodicTime = baseType->activatePeriod + randomSeed.next(baseType->activatePeriodRange);
    }

    direction = baseType->initialDirection;
    mAddAngle(direction, relative_direction);
    if (baseType->initialDirectionRange > 0) {
        mAddAngle(direction, randomSeed.next(baseType->initialDirectionRange));
    }

    Fixed f = baseType->initialVelocity;
    if (baseType->initialVelocityRange > Fixed::zero()) {
        f += randomSeed.next(baseType->initialVelocityRange);
    }
    GetRotPoint(&velocity.h, &velocity.v, direction);
    velocity.h = (velocity.h * f);
    velocity.v = (velocity.v * f);

    if (relative_velocity) {
        velocity.h += relative_velocity->h;
        velocity.v += relative_velocity->v;
    }

    if (!(attributes & (kCanThink | kRemoteOrHuman))) {
        thrust = baseType->maxThrust;
    }

    if (attributes & kIsSelfAnimated) {
        frame.animation.thisShape = baseType->frame.animation.frameShape;
        if (baseType->frame.animation.frameShapeRange > Fixed::zero()) {
            frame.animation.thisShape +=
                    randomSeed.next(baseType->frame.animation.frameShapeRange);
        }
        frame.animation.frameDirection = baseType->frame.animation.frameDirection;
        if (baseType->frame.animation.frameDirectionRange == -1) {
            if (randomSeed.next(2) == 1) {
                frame.animation.frameDirection = 1;
            }
        } else if (baseType->frame.animation.frameDirectionRange > 0) {
            frame.animation.frameDirection +=
                    randomSeed.next(baseType->frame.animation.frameDirectionRange);
        }
        frame.animation.frameFraction = Fixed::zero();
        frame.animation.frameSpeed    = baseType->frame.animation.frameSpeed;
    }

    if (baseType->initialAge >= ticks(0)) {
        expire_after = baseType->initialAge + randomSeed.next(baseType->initialAgeRange);
        expires      = true;
    } else {
        expires = false;
    }

    if (spriteIDOverride == -1) {
        pixResID = baseType->pixResID;
    } else {
        pixResID = spriteIDOverride;
    }

    if (baseType->attributes & kCanThink) {
        pixResID += (GetAdmiralColor(owner) << kSpriteTableColorShift);
    }

    pulse.base   = baseType->pulse.base;
    beam.base    = baseType->beam.base;
    special.base = baseType->special.base;

    longestWeaponRange  = 0;
    shortestWeaponRange = kMaximumRelevantDistance;

    for (auto weapon : {&pulse, &beam, &special}) {
        if (weapon->base.get()) {
            const auto& frame = weapon->base->frame.weapon;
            weapon->ammo      = frame.ammo;
            if ((frame.range > 0) && (frame.usage & kUseForAttacking)) {
                longestWeaponRange  = max(frame.range, longestWeaponRange);
                shortestWeaponRange = min(frame.range, shortestWeaponRange);
            }
        }
    }

    // if we don't have any weapon, then shortest range is 0 too
    shortestWeaponRange = min(longestWeaponRange, shortestWeaponRange);
    engageRange         = max(kEngageRange, longestWeaponRange);

    if (attributes & (kCanCollide | kCanBeHit | kIsDestination | kCanThink | kRemoteOrHuman)) {
        uint32_t ydiff, xdiff;
        auto     player = g.ship;
        if (player.get() && player->active) {
            xdiff = ABS<int>(player->location.h - location.h);
            ydiff = ABS<int>(player->location.v - location.v);
        } else {
            xdiff = ABS<int>(gGlobalCorner.h - location.h);
            ydiff = ABS<int>(gGlobalCorner.v - location.v);
        }
        if ((xdiff > kMaximumRelevantDistance) || (ydiff > kMaximumRelevantDistance)) {
            distanceFromPlayer =
                    MyWideMul<uint64_t>(xdiff, xdiff) + MyWideMul<uint64_t>(ydiff, ydiff);
        } else {
            distanceFromPlayer = ydiff * ydiff + xdiff * xdiff;
        }
    }
}

//
// change_base_type:
// This is a very RISKY procedure. You probably shouldn't change anything fundamental about the
// object--
// meaning, attributes that change the way the object behaves, or the way other objects treat this
// object--
// so don't, for instance, give something the kCanThink attribute if it couldn't before.
// This routine is similar to "InitSpaceObjectFromBaseObject" except that it doesn't change many
// things
// (like the velocity, direction, or randomseed) AND it handles the sprite data itself!
// Can you change the frame type? Like from a direction frame to a self-animated frame? I'm not
// sure...
//

void SpaceObject::change_base_type(
        Handle<BaseObject> base, int32_t spriteIDOverride, bool relative) {
    auto          obj = this;
    int16_t       angle;
    int32_t       r;
    NatePixTable* spriteTable;

#ifdef DATA_COVERAGE
    covered_objects.insert(base.number());
    for (auto weapon : {base->pulse.base, base->beam.base, base->special.base}) {
        if (weapon.get()) {
            covered_objects.insert(weapon.number());
        }
    }
#endif  // DATA_COVERAGE

    obj->attributes = base->attributes | (obj->attributes & (kIsHumanControlled | kIsRemote |
                                                             kIsPlayerShip | kStaticDestination));
    obj->baseType      = base.get();
    obj->base          = base;
    obj->tinySize      = base->tinySize;
    obj->shieldColor   = base->shieldColor;
    obj->layer         = base->pixLayer;
    obj->directionGoal = 0;
    obj->turnFraction = obj->turnVelocity = Fixed::zero();

    if (obj->attributes & kIsSelfAnimated) {
        obj->frame.animation.thisShape = base->frame.animation.frameShape;
        if (base->frame.animation.frameShapeRange > Fixed::zero()) {
            obj->frame.animation.thisShape +=
                    obj->randomSeed.next(base->frame.animation.frameShapeRange);
        }
        obj->frame.animation.frameDirection = base->frame.animation.frameDirection;
        if (base->frame.animation.frameDirectionRange == -1) {
            if (obj->randomSeed.next(2) == 1) {
                obj->frame.animation.frameDirection = 1;
            }
        } else if (base->frame.animation.frameDirectionRange > 0) {
            obj->frame.animation.frameDirection +=
                    obj->randomSeed.next(base->frame.animation.frameDirectionRange);
        }
        obj->frame.animation.frameFraction = Fixed::zero();
        obj->frame.animation.frameSpeed    = base->frame.animation.frameSpeed;
    }

    obj->maxVelocity = base->maxVelocity;

    if (base->initialAge >= ticks(0)) {
        obj->expire_after = base->initialAge + obj->randomSeed.next(base->initialAgeRange);
        obj->expires      = true;
    } else {
        obj->expires = false;

        // Compatibility: discard a random number. Used to be that a
        // random age was unconditionally generated, even for objects
        // that wouldn't expire after altering their base-type.
        obj->randomSeed.next(1);
    }

    obj->naturalScale = base->naturalScale;

    // not setting id

    obj->active = kObjectInUse;

    // not setting sprite, targetObjectNumber, lastTarget, lastTargetDistance;

    if (spriteIDOverride == -1) {
        obj->pixResID = base->pixResID;
    } else {
        obj->pixResID = spriteIDOverride;
    }

    if (base->attributes & kCanThink) {
        obj->pixResID += (GetAdmiralColor(obj->owner) << kSpriteTableColorShift);
    }

    // check periodic time
    obj->periodicTime = ticks(0);
    if (base->activatePeriod != ticks(0)) {
        obj->periodicTime = base->activatePeriod + obj->randomSeed.next(base->activatePeriodRange);
    }

    obj->pulse.base          = base->pulse.base;
    obj->beam.base           = base->beam.base;
    obj->special.base        = base->special.base;
    obj->longestWeaponRange  = 0;
    obj->shortestWeaponRange = kMaximumRelevantDistance;

    for (auto* weapon : {&obj->pulse, &obj->beam, &obj->special}) {
        if (!weapon->base.get()) {
            weapon->time = game_ticks();
            continue;
        }

        if (!relative) {
            weapon->ammo     = weapon->base->frame.weapon.ammo;
            weapon->position = 0;
            if (weapon->time > g.time + weapon->base->frame.weapon.fireTime) {
                weapon->time = g.time + weapon->base->frame.weapon.fireTime;
            }
        }
        r = weapon->base->frame.weapon.range;
        if ((r > 0) && (weapon->base->frame.weapon.usage & kUseForAttacking)) {
            if (r > obj->longestWeaponRange) {
                obj->longestWeaponRange = r;
            }
            if (r < obj->shortestWeaponRange) {
                obj->shortestWeaponRange = r;
            }
        }
    }

    // if we don't have any weapon, then shortest range is 0 too
    if (obj->longestWeaponRange == 0)
        obj->shortestWeaponRange = 0;
    if (obj->longestWeaponRange > kEngageRange) {
        obj->engageRange = obj->longestWeaponRange;
    } else {
        obj->engageRange = kEngageRange;
    }

    // HANDLE THE NEW SPRITE DATA:
    if (obj->pixResID != kNoSpriteTable) {
        spriteTable = sys.pix.get(obj->pixResID);

        if (spriteTable == NULL) {
            throw Exception("Couldn't load a requested sprite");
            spriteTable = sys.pix.add(obj->pixResID);
        }

        obj->sprite->table      = spriteTable;
        obj->sprite->tinySize   = base->tinySize;
        obj->sprite->whichLayer = base->pixLayer;
        obj->sprite->scale      = base->naturalScale;

        if (obj->attributes & kIsSelfAnimated) {
            obj->sprite->whichShape = more_evil_fixed_to_long(obj->frame.animation.thisShape);
        } else if (obj->attributes & kShapeFromDirection) {
            angle = obj->direction;
            mAddAngle(angle, base->frame.rotation.rotRes >> 1);
            obj->sprite->whichShape = angle / base->frame.rotation.rotRes;
        } else {
            obj->sprite->whichShape = 0;
        }
    }
}

Handle<SpaceObject> CreateAnySpaceObject(
        Handle<BaseObject> whichBase, fixedPointType* velocity, coordPointType* location,
        int32_t direction, Handle<Admiral> owner, uint32_t specialAttributes,
        int16_t spriteIDOverride) {
    Random      random{g.random.next(32766)};
    int32_t     id = g.random.next(16384);
    SpaceObject newObject(
            whichBase, random, id, *location, direction, velocity, owner, spriteIDOverride);

    auto obj = AddSpaceObject(&newObject);
    if (!obj.get()) {
        return SpaceObject::none();
    }

#ifdef DATA_COVERAGE
    covered_objects.insert(whichBase.number());
    for (auto weapon : {whichBase->pulse.base, whichBase->beam.base, whichBase->special.base}) {
        if (!weapon.get()) {
            covered_objects.insert(weapon.number());
        }
    }
#endif  // DATA_COVERAGE

    obj->attributes |= specialAttributes;
    exec(obj->baseType->create, obj, SpaceObject::none(), NULL);
    return obj;
}

int32_t CountObjectsOfBaseType(Handle<BaseObject> whichType, Handle<Admiral> owner) {
    int32_t result = 0;
    for (auto anObject : SpaceObject::all()) {
        if (anObject->active && (!whichType.get() || (anObject->base == whichType)) &&
            (!owner.get() || (anObject->owner == owner))) {
            ++result;
        }
    }
    return result;
}

void SpaceObject::alter_health(int32_t amount) {
    if (amount <= 0) {
        _health += amount;
    } else if (_health >= (2147483647 - amount)) {
        _health = 2147483647;
    } else {
        _health += amount;
    }
    if (_health < 0) {
        destroy();
    }
}

void SpaceObject::alter_energy(int32_t amount) {
    _energy += amount;
    if (_energy < 0) {
        _energy = 0;
    } else if (_energy > max_energy()) {
        alter_battery(_energy - max_energy());
        _energy = max_energy();
    }
}

void SpaceObject::alter_battery(int32_t amount) {
    _battery += amount;
    if (_battery > max_battery()) {
        if (owner.get()) {
            owner->pay(Fixed::from_val(_battery - max_battery()));
        }
        _battery = max_battery();
    }
}

bool SpaceObject::collect_warp_energy(int32_t amount) {
    if (amount >= _energy) {
        warpEnergyCollected += _energy;
        _energy = 0;
        return false;
    } else {
        _energy -= amount;
        warpEnergyCollected += amount;
        return true;
    }
}

void SpaceObject::refund_warp_energy() {
    alter_battery(warpEnergyCollected);
    warpEnergyCollected = 0;
}

void SpaceObject::set_owner(Handle<Admiral> owner, bool message) {
    auto object = Handle<SpaceObject>(number());
    if (object->owner == owner) {
        return;
    }

    // if the object is occupied by a human, eject him since he can't change sides
    if ((object->attributes & (kIsPlayerShip | kRemoteOrHuman)) &&
        !object->baseType->destroyDontDie) {
        object->create_floating_player_body();
    }

    Handle<Admiral> old_owner = object->owner;
    object->owner             = owner;

    if (owner.get() && (object->attributes & kIsDestination)) {
        if (!owner->control().get()) {
            owner->set_control(object);
        }

        if (!GetAdmiralBuildAtObject(owner).get()) {
            if (BaseHasSomethingToBuild(object)) {
                SetAdmiralBuildAtObject(owner, object);
            }
        }
        if (!owner->target().get()) {
            owner->set_target(object);
        }
    }

    if (object->attributes & kNeutralDeath) {
        object->attributes = object->baseType->attributes;
    }

    if (object->sprite.get()) {
        uint8_t tinyShade;
        switch (object->sprite->whichLayer) {
            case kFirstSpriteLayer: tinyShade  = MEDIUM; break;
            case kMiddleSpriteLayer: tinyShade = LIGHT; break;
            case kLastSpriteLayer: tinyShade   = VERY_LIGHT; break;
            default: tinyShade                 = DARK; break;
        }

        RgbColor tinyColor;
        if (owner == g.admiral) {
            tinyColor = GetRGBTranslateColorShade(kFriendlyColor, tinyShade);
        } else if (owner.get()) {
            tinyColor = GetRGBTranslateColorShade(kHostileColor, tinyShade);
        } else {
            tinyColor = GetRGBTranslateColorShade(kNeutralColor, tinyShade);
        }
        object->tinyColor = object->sprite->tinyColor = tinyColor;

        if (object->attributes & kCanThink) {
            NatePixTable* pixTable;

            if ((object->pixResID == object->baseType->pixResID) ||
                (object->pixResID == (object->baseType->pixResID |
                                      (GetAdmiralColor(old_owner) << kSpriteTableColorShift)))) {
                object->pixResID = object->baseType->pixResID |
                                   (GetAdmiralColor(owner) << kSpriteTableColorShift);

                pixTable = sys.pix.get(object->pixResID);
                if (pixTable != NULL) {
                    object->sprite->table = pixTable;
                }
            }
        }
    }

    object->remoteFoeStrength = object->remoteFriendStrength = object->escortStrength =
            object->localFoeStrength = object->localFriendStrength = Fixed::zero();
    object->bestConsideredTargetValue = object->currentTargetValue = kFixedNone;
    object->bestConsideredTargetNumber                             = SpaceObject::none();

    for (auto fixObject : SpaceObject::all()) {
        if ((fixObject->destObject == object) && (fixObject->active != kObjectAvailable) &&
            (fixObject->attributes & kCanThink)) {
            fixObject->currentTargetValue = kFixedNone;
            if (fixObject->owner != owner) {
                object->remoteFoeStrength += fixObject->baseType->offenseValue;
            } else {
                object->remoteFriendStrength += fixObject->baseType->offenseValue;
                object->escortStrength += fixObject->baseType->offenseValue;
            }
        }
    }

    if (object->attributes & kIsDestination) {
        if (object->attributes & kNeutralDeath) {
            ClearAllOccupants(object->asDestination, owner, object->baseType->occupy_count);
        }
        StopBuilding(object->asDestination);
        if (message) {
            String destination_name(GetDestBalanceName(object->asDestination));
            if (owner.get()) {
                String new_owner_name(GetAdmiralName(object->owner));
                Messages::add(format("{0} captured by {1}.", destination_name, new_owner_name));
            } else if (old_owner.get()) {  // must be since can't both be -1
                String old_owner_name(GetAdmiralName(old_owner));
                Messages::add(format("{0} lost by {1}.", destination_name, old_owner_name));
            }
        }
        RecalcAllAdmiralBuildData();
    } else {
        if (message) {
            StringSlice object_name = get_object_name(object->base);
            if (owner.get()) {
                String new_owner_name(GetAdmiralName(object->owner));
                Messages::add(format("{0} captured by {1}.", object_name, new_owner_name));
            } else if (old_owner.get()) {  // must be since can't both be -1
                String old_owner_name(GetAdmiralName(old_owner));
                Messages::add(format("{0} lost by {1}.", object_name, old_owner_name));
            }
        }
    }
}

void SpaceObject::alter_occupation(Handle<Admiral> owner, int32_t howMuch, bool message) {
    auto object = this;
    if (object->active && (object->attributes & kIsDestination) &&
        (object->attributes & kNeutralDeath)) {
        if (AlterDestinationObjectOccupation(object->asDestination, owner, howMuch) >=
            object->baseType->occupy_count) {
            object->set_owner(owner, message);
        }
    }
}

void SpaceObject::set_cloak(bool cloak) {
    auto object = Handle<SpaceObject>(number());
    if (cloak && (object->cloakState == 0)) {
        object->cloakState = 1;
        sys.sound.cloak_on_at(object);
    } else if ((!cloak || (object->attributes & kRemoteOrHuman)) && (object->cloakState >= 250)) {
        object->cloakState = kCloakOffStateMax;
        sys.sound.cloak_off_at(object);
    }
}

void SpaceObject::destroy() {
    auto object = Handle<SpaceObject>(number());
    if (object->active != kObjectInUse) {
        return;
    } else if (object->attributes & kNeutralDeath) {
        object->_health = object->max_health();
        // if anyone is targeting it, they should stop
        for (auto fixObject : SpaceObject::all()) {
            if ((fixObject->attributes & kCanAcceptDestination) &&
                (fixObject->active != kObjectAvailable)) {
                if (fixObject->targetObject == object) {
                    fixObject->targetObject = SpaceObject::none();
                }
            }
        }

        object->set_owner(Admiral::none(), true);
        object->attributes &= ~(kHated | kCanEngage | kCanCollide | kCanBeHit);
        exec(object->baseType->destroy, object, SpaceObject::none(), NULL);
    } else {
        AddKillToAdmiral(object);
        if (object->attributes & kReleaseEnergyOnDeath) {
            int16_t energyNum = object->energy() / kEnergyPodAmount;
            while (energyNum > 0) {
                CreateAnySpaceObject(
                        plug.meta.energyBlobID, &object->velocity, &object->location,
                        object->direction, Admiral::none(), 0, -1);
                energyNum--;
            }
        }

        // if it's a destination, we keep anyone from thinking they have it as a destination
        // (all at once since this should be very rare)
        if ((object->attributes & kIsDestination) && !object->baseType->destroyDontDie) {
            RemoveDestination(object->asDestination);
            for (auto fixObject : SpaceObject::all()) {
                if ((fixObject->attributes & kCanAcceptDestination) &&
                    (fixObject->active != kObjectAvailable)) {
                    if (fixObject->destObject == object) {
                        fixObject->destObject = SpaceObject::none();
                        fixObject->attributes &= ~kStaticDestination;
                    }
                }
            }
        }

        exec(object->baseType->destroy, object, SpaceObject::none(), NULL);

        if (object->attributes & kCanAcceptDestination) {
            RemoveObjectFromDestination(object);
        }
        if (!object->baseType->destroyDontDie) {
            object->active = kObjectToBeFreed;
        }
    }
}

void SpaceObject::free() {
    if (attributes & kIsVector) {
        if (frame.vector.get()) {
            frame.vector->killMe = true;
        }
    } else {
        if (sprite.get()) {
            sprite->killMe = true;
        }
    }
    active         = kObjectAvailable;
    attributes     = 0;
    nextNearObject = nextFarObject = SpaceObject::none();
    if (previousObject.get()) {
        auto bObject        = previousObject;
        bObject->nextObject = nextObject;
    }
    if (nextObject.get()) {
        auto bObject            = nextObject;
        bObject->previousObject = previousObject;
    }
    if (g.root.get() == this) {
        g.root = nextObject;
    }
    nextObject     = SpaceObject::none();
    previousObject = SpaceObject::none();

    // Unlink admirals' flagships, so we don't need to track the id of
    // each admiral's flagship.
    for (auto adm : Admiral::all()) {
        if (adm->flagship().get() == this) {
            adm->set_flagship(SpaceObject::none());
        }
    }
}

void SpaceObject::create_floating_player_body() {
    auto       obj       = Handle<SpaceObject>(number());
    const auto body_type = plug.meta.playerBodyID;
    // if we're already in a body, don't create a body from it
    // a body expiring is handled elsewhere
    if (obj->base == body_type) {
        return;
    }

    auto body = CreateAnySpaceObject(
            body_type, &obj->velocity, &obj->location, obj->direction, obj->owner, 0, -1);
    if (body.get()) {
        ChangePlayerShipNumber(obj->owner, body);
    } else {
        PlayerShipBodyExpire(obj);
    }
}

sfz::StringSlice SpaceObject::name() const {
    if (attributes & kIsDestination) {
        return GetDestBalanceName(asDestination);
    } else {
        return get_object_name(base);
    }
}

sfz::StringSlice SpaceObject::short_name() const {
    if (attributes & kIsDestination) {
        return GetDestBalanceName(asDestination);
    } else {
        return get_object_short_name(base);
    }
}

bool SpaceObject::engages(const SpaceObject& b) const {
    if ((baseType->buildFlags & kCanOnlyEngage) || (b.baseType->buildFlags & kOnlyEngagedBy)) {
        return baseType->engageKeyTag == b.baseType->levelKeyTag;
    }
    return true;
}

Fixed SpaceObject::turn_rate() const {
    // design flaw: can't have turn rate unless shapefromdirection
    if (attributes & kShapeFromDirection) {
        return baseType->frame.rotation.maxTurnRate;
    }
    return kDefaultTurnRate;
}

StringSlice get_object_name(Handle<BaseObject> id) {
    return id->name;  // TODO(sfiera): use directly.
}

StringSlice get_object_short_name(Handle<BaseObject> id) {
    return id->short_name;  // TODO(sfiera): use directly.
}

int32_t SpaceObject::number() const {
    return this - g.objects.get();
}

}  // namespace antares

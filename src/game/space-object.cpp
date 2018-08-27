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

#include <pn/file>
#include <set>

#include "data/base-object.hpp"
#include "data/plugin.hpp"
#include "data/races.hpp"
#include "data/resource.hpp"
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

using std::max;
using std::min;
using std::set;
using std::unique_ptr;

namespace antares {

const NamedHandle<const BaseObject> kWarpInFlare{"sfx/warp/in"};
const NamedHandle<const BaseObject> kWarpOutFlare{"sfx/warp/out"};
const NamedHandle<const BaseObject> kPlayerBody{"sfx/crew"};
const NamedHandle<const BaseObject> kEnergyBlob{"sfx/energy"};

const Hue kFriendlyColor               = Hue::GREEN;
const Hue kHostileColor[kMaxPlayerNum] = {Hue::PINK, Hue::RED, Hue::YELLOW, Hue::ORANGE};
const Hue kNeutralColor                = Hue::SKY_BLUE;

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

BaseObject* BaseObject::get(int number) { return get(pn::dump(number, pn::dump_short)); }

BaseObject* BaseObject::get(pn::string_view name) {
    auto it = plug.objects.find(name.copy());
    if (it != plug.objects.end()) {
        return &it->second;
    }
    return nullptr;
}

NamedHandle<const BaseObject> get_buildable_object_handle(
        const BuildableObject& o, const NamedHandle<const Race>& race) {
    pn::string race_object = pn::format("{0}/{1}", race.name(), o.name);
    if (Resource::object_exists(race_object)) {
        return NamedHandle<const BaseObject>(race_object);
    }
    return NamedHandle<const BaseObject>(o.name.copy());
}

const BaseObject* get_buildable_object(
        const BuildableObject& o, const NamedHandle<const Race>& race) {
    pn::string race_object = pn::format("{0}/{1}", race.name(), o.name);
    if (auto base = BaseObject::get(race_object)) {
        return base;
    }
    return BaseObject::get(o.name);
}

static Handle<SpaceObject> next_free_space_object() {
    for (auto obj : SpaceObject::all()) {
        if (!obj->active) {
            return obj;
        }
    }
    return SpaceObject::none();
}

static uint8_t get_tiny_shade(const SpaceObject& o) {
    switch (o.layer) {
        case BaseObject::Layer::NONE: return DARK; break;
        case BaseObject::Layer::BASES: return MEDIUM; break;
        case BaseObject::Layer::SHIPS: return LIGHT; break;
        case BaseObject::Layer::SHOTS: return LIGHTEST; break;
    }
}

static Hue get_tiny_color(const SpaceObject& o) {
    if (o.owner == g.admiral) {
        return kFriendlyColor;
    } else if (o.owner.get()) {
        return kHostileColor[o.owner.number()];
    } else {
        return kNeutralColor;
    }
}

static Handle<SpaceObject> AddSpaceObject(SpaceObject* sourceObject) {
    auto obj = next_free_space_object();
    if (!obj.get()) {
        return SpaceObject::none();
    }

    NatePixTable* spriteTable = nullptr;
    if (sourceObject->pix_id.has_value()) {
        spriteTable = sys.pix.get(sourceObject->pix_id->name, sourceObject->pix_id->hue);
        if (!spriteTable) {
            throw std::runtime_error(pn::format(
                                             "{0}/{1}: sprite not loaded",
                                             sourceObject->pix_id->name,
                                             static_cast<int>(sourceObject->pix_id->hue))
                                             .c_str());
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
        int16_t whichShape = 0;
        int16_t angle;
        if (obj->attributes & kIsSelfAnimated) {
            whichShape = more_evil_fixed_to_long(obj->frame.animation.thisShape);
        } else if (obj->attributes & kShapeFromDirection) {
            angle = obj->direction;
            mAddAngle(angle, rotation_resolution(*obj->base) >> 1);
            whichShape = angle / rotation_resolution(*obj->base);
        }

        obj->sprite = AddSprite(
                where, spriteTable, sourceObject->pix_id->name, sourceObject->pix_id->hue,
                whichShape, obj->naturalScale, obj->icon, obj->layer, get_tiny_color(*obj),
                get_tiny_shade(*obj));

        if (!obj->sprite.get()) {
            g.game_over    = true;
            g.game_over_at = g.time;
            obj->active    = kObjectAvailable;
            return SpaceObject::none();
        }
    }

    if (obj->attributes & kIsVector) {
        if (obj->base->ray.has_value()) {
            obj->frame.vector = Vectors::add(&(obj->location), *obj->base->ray);
        } else {
            obj->frame.vector = Vectors::add(&(obj->location), *obj->base->bolt);
        }
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
        const BaseObject& type, Random seed, int32_t object_id,
        const coordPointType& initial_location, int32_t relative_direction,
        fixedPointType* relative_velocity, Handle<Admiral> new_owner,
        sfz::optional<pn::string_view> spriteIDOverride) {
    base       = &type;
    active     = kObjectInUse;
    randomSeed = seed;
    owner      = new_owner;
    location   = initial_location;
    id         = object_id;
    sprite     = Sprite::none();

    attributes   = base->attributes;
    shieldColor  = base->shieldColor;
    icon         = base->icon;
    layer        = sprite_layer(*base);
    maxVelocity  = base->maxVelocity;
    naturalScale = sprite_scale(*base);

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

    if (base->activate.period.has_value()) {
        periodicTime =
                base->activate.period->begin + randomSeed.next(base->activate.period->range());
    }

    direction = base->initial_direction.begin;
    mAddAngle(direction, relative_direction);
    if (base->initial_direction.range() > 1) {
        mAddAngle(direction, randomSeed.next(base->initial_direction.range()));
    }

    Fixed f = base->maxVelocity;
    if (base->initial_velocity.has_value()) {
        f = base->initial_velocity->begin;
        if (base->initial_velocity->range() > Fixed::from_val(1)) {
            f += randomSeed.next(base->initial_velocity->range());
        }
    }
    GetRotPoint(&velocity.h, &velocity.v, direction);
    velocity.h = (velocity.h * f);
    velocity.v = (velocity.v * f);

    if (relative_velocity) {
        velocity.h += relative_velocity->h;
        velocity.v += relative_velocity->v;
    }

    if (!(attributes & (kCanThink | kRemoteOrHuman))) {
        thrust = base->thrust;
    }

    if (attributes & kIsSelfAnimated) {
        frame.animation.thisShape = base->animation->first.begin;
        if (base->animation->first.range() > Fixed::from_val(1)) {
            frame.animation.thisShape += randomSeed.next(base->animation->first.range());
        }
        frame.animation.direction = base->animation->direction;
        if (base->animation->direction == BaseObject::Animation::Direction::RANDOM) {
            if (randomSeed.next(2) == 1) {
                frame.animation.direction = BaseObject::Animation::Direction::PLUS;
            } else {
                frame.animation.direction = BaseObject::Animation::Direction::MINUS;
            }
        }
        frame.animation.frameFraction = Fixed::zero();
        frame.animation.speed         = base->animation->speed;
    }

    if (base->expire.after.age.has_value()) {
        expire_after =
                base->expire.after.age->begin + randomSeed.next(base->expire.after.age->range());
        expires = true;
    } else {
        expires = false;
    }

    auto pix_resource = sprite_resource(*base);
    if (pix_resource.has_value()) {
        pix_id.emplace();
        if (spriteIDOverride.has_value()) {
            pix_id->name = *spriteIDOverride;
        } else {
            pix_id->name = *pix_resource;
        }
        if (base->attributes & kCanThink) {
            pix_id->hue = GetAdmiralColor(owner);
        } else {
            pix_id->hue = Hue::GRAY;
        }
    }

    pulse.base   = base->weapons.pulse.has_value() ? base->weapons.pulse->base.get() : nullptr;
    beam.base    = base->weapons.beam.has_value() ? base->weapons.beam->base.get() : nullptr;
    special.base = base->weapons.special.has_value() ? base->weapons.special->base.get() : nullptr;

    longestWeaponRange  = 0;
    shortestWeaponRange = kMaximumRelevantDistance;

    for (auto weapon : {&pulse, &beam, &special}) {
        if (weapon->base) {
            const auto& frame = weapon->base->device;
            weapon->ammo      = frame->ammo;
            if ((frame->range.squared > 0) && (frame->usage.attacking)) {
                longestWeaponRange  = max<int32_t>(frame->range.squared, longestWeaponRange);
                shortestWeaponRange = min<int32_t>(frame->range.squared, shortestWeaponRange);
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
        const BaseObject& base, sfz::optional<pn::string_view> spriteIDOverride, bool relative) {
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

    obj->attributes  = base.attributes | (obj->attributes & (kIsPlayerShip | kStaticDestination));
    obj->base        = &base;
    obj->icon        = base.icon;
    obj->shieldColor = base.shieldColor;
    obj->layer       = sprite_layer(base);
    obj->directionGoal = 0;
    obj->turnFraction = obj->turnVelocity = Fixed::zero();

    if (obj->attributes & kIsSelfAnimated) {
        obj->frame.animation.thisShape = base.animation->first.begin;
        if (base.animation->first.range() > Fixed::from_val(1)) {
            obj->frame.animation.thisShape += obj->randomSeed.next(base.animation->first.range());
        }
        frame.animation.direction = base.animation->direction;
        if (base.animation->direction == BaseObject::Animation::Direction::RANDOM) {
            if (randomSeed.next(2) == 1) {
                frame.animation.direction = BaseObject::Animation::Direction::PLUS;
            } else {
                frame.animation.direction = BaseObject::Animation::Direction::MINUS;
            }
        }
        obj->frame.animation.frameFraction = Fixed::zero();
        obj->frame.animation.speed         = base.animation->speed;
    }

    obj->maxVelocity = base.maxVelocity;

    if (base.expire.after.age.has_value()) {
        obj->expire_after = base.expire.after.age->begin +
                            obj->randomSeed.next(base.expire.after.age->range());
        obj->expires = true;
    } else {
        obj->expires = false;

        // Compatibility: discard a random number. Used to be that a
        // random age was unconditionally generated, even for objects
        // that wouldn't expire after altering their base-type.
        obj->randomSeed.next(1);
    }

    obj->naturalScale = sprite_scale(base);

    // not setting id

    obj->active = kObjectInUse;

    // not setting sprite, targetObjectNumber, lastTarget, lastTargetDistance;

    auto pix_resource = sprite_resource(base);
    if (pix_resource.has_value()) {
        pix_id.emplace();
        if (spriteIDOverride.has_value()) {
            pix_id->name = *spriteIDOverride;
        } else {
            pix_id->name = *pix_resource;
        }
        if (base.attributes & kCanThink) {
            pix_id->hue = GetAdmiralColor(owner);
        } else {
            pix_id->hue = Hue::GRAY;
        }
    }

    // check periodic time
    obj->periodicTime = ticks(0);
    if (base.activate.period.has_value()) {
        obj->periodicTime =
                base.activate.period->begin + obj->randomSeed.next(base.activate.period->range());
    }

    obj->pulse.base = base.weapons.pulse.has_value() ? base.weapons.pulse->base.get() : nullptr;
    obj->beam.base  = base.weapons.beam.has_value() ? base.weapons.beam->base.get() : nullptr;
    obj->special.base =
            base.weapons.special.has_value() ? base.weapons.special->base.get() : nullptr;
    obj->longestWeaponRange  = 0;
    obj->shortestWeaponRange = kMaximumRelevantDistance;

    for (auto* weapon : {&obj->pulse, &obj->beam, &obj->special}) {
        if (!weapon->base) {
            weapon->time = game_ticks();
            continue;
        }

        if (!relative) {
            weapon->ammo     = weapon->base->device->ammo;
            weapon->position = 0;
            if (weapon->time > g.time + weapon->base->device->fireTime) {
                weapon->time = g.time + weapon->base->device->fireTime;
            }
        }
        r = weapon->base->device->range.squared;
        if ((r > 0) && (weapon->base->device->usage.attacking)) {
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
    if (obj->pix_id.has_value()) {
        spriteTable = sys.pix.get(obj->pix_id->name, obj->pix_id->hue);

        if (spriteTable == NULL) {
            throw std::runtime_error("Couldn't load a requested sprite");
        }

        obj->sprite->table = spriteTable;
        obj->sprite->icon =
                base.icon.value_or(BaseObject::Icon{BaseObject::Icon::Shape::SQUARE, 0});
        obj->sprite->whichLayer = sprite_layer(base);
        obj->sprite->scale      = sprite_scale(base);

        if (obj->attributes & kIsSelfAnimated) {
            obj->sprite->whichShape = more_evil_fixed_to_long(obj->frame.animation.thisShape);
        } else if (obj->attributes & kShapeFromDirection) {
            angle = obj->direction;
            mAddAngle(angle, rotation_resolution(base) >> 1);
            obj->sprite->whichShape = angle / rotation_resolution(base);
        } else {
            obj->sprite->whichShape = 0;
        }
    }
}

Handle<SpaceObject> CreateAnySpaceObject(
        const BaseObject& whichBase, fixedPointType* velocity, coordPointType* location,
        int32_t direction, Handle<Admiral> owner, uint32_t specialAttributes,
        sfz::optional<pn::string_view> spriteIDOverride) {
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
    exec(obj->base->create.action, obj, SpaceObject::none(), {0, 0});
    return obj;
}

int32_t CountObjectsOfBaseType(const BaseObject* whichType, Handle<Admiral> owner) {
    int32_t result = 0;
    for (auto anObject : SpaceObject::all()) {
        if (anObject->active && (!whichType || (anObject->base == whichType)) &&
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
            owner->pay(Cash{Fixed::from_val(_battery - max_battery())});
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

void SpaceObject::set_owner(Handle<Admiral> new_owner, bool message) {
    auto object = Handle<SpaceObject>(number());
    if (object->owner == new_owner) {
        return;
    }

    // if the object is occupied by a human, eject him since he can't change sides
    if ((object->attributes & (kIsPlayerShip | kRemoteOrHuman)) && object->base->destroy.die) {
        object->create_floating_player_body();
    }

    Handle<Admiral> old_owner = object->owner;
    object->owner             = new_owner;

    if (new_owner.get() && (object->attributes & kIsDestination)) {
        if (!new_owner->control().get()) {
            new_owner->set_control(object);
        }

        if (!GetAdmiralBuildAtObject(new_owner).get()) {
            if (BaseHasSomethingToBuild(object)) {
                SetAdmiralBuildAtObject(new_owner, object);
            }
        }
        if (!new_owner->target().get()) {
            new_owner->set_target(object);
        }
    }

    if (object->attributes & kNeutralDeath) {
        object->attributes = object->base->attributes;
    }

    if (object->sprite.get()) {
        object->sprite->tinyColor.hue = get_tiny_color(*object);

        if (object->attributes & kCanThink) {
            NatePixTable* pixTable;

            object->pix_id->hue = GetAdmiralColor(new_owner);
            pixTable            = sys.pix.get(object->pix_id->name, object->pix_id->hue);
            if (pixTable != NULL) {
                object->sprite->table = pixTable;
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
            if (fixObject->owner != new_owner) {
                object->remoteFoeStrength += fixObject->base->ai.escort.power;
            } else {
                object->remoteFriendStrength += fixObject->base->ai.escort.power;
                object->escortStrength += fixObject->base->ai.escort.power;
            }
        }
    }

    if (object->attributes & kIsDestination) {
        if (object->attributes & kNeutralDeath) {
            ClearAllOccupants(object->asDestination, new_owner, object->base->occupy_count);
        }
        StopBuilding(object->asDestination);
        RecalcAllAdmiralBuildData();
    }
    if (message) {
        if (new_owner.get()) {
            Messages::add(
                    pn::format("{0} captured by {1}.", object->long_name(), new_owner->name()));
        } else if (old_owner.get()) {  // must be since can't both be -1
            Messages::add(pn::format("{0} lost by {1}.", object->long_name(), old_owner->name()));
        }
    }
}

void SpaceObject::alter_occupation(Handle<Admiral> owner, int32_t howMuch, bool message) {
    auto object = this;
    if (object->active && (object->attributes & kIsDestination) &&
        (object->attributes & kNeutralDeath)) {
        if (AlterDestinationObjectOccupation(object->asDestination, owner, howMuch) >=
            object->base->occupy_count) {
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
        exec(object->base->destroy.action, object, SpaceObject::none(), {0, 0});
    } else {
        AddKillToAdmiral(object);
        if (object->attributes & kReleaseEnergyOnDeath) {
            int16_t energyNum = object->energy() / kEnergyPodAmount;
            while (energyNum > 0) {
                CreateAnySpaceObject(
                        *kEnergyBlob, &object->velocity, &object->location, object->direction,
                        Admiral::none(), 0, sfz::nullopt);
                energyNum--;
            }
        }

        // if it's a destination, we keep anyone from thinking they have it as a destination
        // (all at once since this should be very rare)
        if ((object->attributes & kIsDestination) && object->base->destroy.die) {
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

        exec(object->base->destroy.action, object, SpaceObject::none(), {0, 0});

        if (object->attributes & kCanAcceptDestination) {
            RemoveObjectFromDestination(object);
        }
        if (object->base->destroy.die) {
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
    if (g.ship.get() == this) {
        g.ship = SpaceObject::none();
    }
}

void SpaceObject::create_floating_player_body() {
    auto              obj       = Handle<SpaceObject>(number());
    const BaseObject& body_type = *kPlayerBody;
    // if we're already in a body, don't create a body from it
    // a body expiring is handled elsewhere
    if (obj->base == &body_type) {
        return;
    }

    auto body = CreateAnySpaceObject(
            body_type, &obj->velocity, &obj->location, obj->direction, obj->owner, 0,
            sfz::nullopt);
    if (body.get()) {
        ChangePlayerShipNumber(obj->owner, body);
    } else {
        PlayerShipBodyExpire(obj);
    }
}

pn::string_view SpaceObject::long_name() const {
    if (attributes & kIsDestination) {
        return GetDestBalanceName(asDestination);
    } else {
        return base->long_name;
    }
}

pn::string_view SpaceObject::short_name() const {
    if (attributes & kIsDestination) {
        return GetDestBalanceName(asDestination);
    } else {
        return base->short_name;
    }
}

bool SpaceObject::engages(const SpaceObject& b) const {
    return tags_match(*b.base, base->ai.combat.engages.if_.tags) &&
           tags_match(*base, b.base->ai.combat.engaged.if_.tags);
}

Fixed SpaceObject::turn_rate() const { return base->turn_rate; }

int32_t SpaceObject::number() const { return this - g.objects.get(); }

bool tags_match(const BaseObject& o, const Tags& query) {
    for (const auto& kv : query.tags) {
        auto it      = o.tags.tags.find(kv.first);
        bool has_tag = ((it != o.tags.tags.end()) && it->second);
        if (kv.second != has_tag) {
            return false;
        }
    }
    return true;
}

sfz::optional<pn::string_view> sprite_resource(const BaseObject& o) {
    if (o.attributes & kShapeFromDirection) {
        return sfz::make_optional<pn::string_view>(o.rotation->sprite);
    } else if (o.attributes & kIsSelfAnimated) {
        return sfz::make_optional<pn::string_view>(o.animation->sprite);
    } else {
        return sfz::nullopt;
    }
}

BaseObject::Layer sprite_layer(const BaseObject& o) {
    if (o.attributes & kShapeFromDirection) {
        return o.rotation->layer;
    } else if (o.attributes & kIsSelfAnimated) {
        return o.animation->layer;
    } else {
        return BaseObject::Layer::NONE;
    }
}

int32_t sprite_scale(const BaseObject& o) {
    if (o.attributes & kShapeFromDirection) {
        return o.rotation->scale.factor;
    } else if (o.attributes & kIsSelfAnimated) {
        return o.animation->scale.factor;
    } else {
        return SCALE_SCALE;
    }
}

int32_t rotation_resolution(const BaseObject& o) {
    if (o.attributes & kShapeFromDirection) {
        return 360 / o.rotation->frames.range();
    } else {
        return 360;
    }
}

}  // namespace antares

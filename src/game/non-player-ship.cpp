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

#include "game/non-player-ship.hpp"

#include <pn/output>

#include "config/keys.hpp"
#include "data/plugin.hpp"
#include "drawing/color.hpp"
#include "drawing/sprite-handling.hpp"
#include "game/action.hpp"
#include "game/admiral.hpp"
#include "game/globals.hpp"
#include "game/level.hpp"
#include "game/messages.hpp"
#include "game/motion.hpp"
#include "game/player-ship.hpp"
#include "game/space-object.hpp"
#include "game/starfield.hpp"
#include "game/sys.hpp"
#include "math/macros.hpp"
#include "math/random.hpp"
#include "math/rotation.hpp"
#include "math/special.hpp"
#include "math/units.hpp"
#include "sound/fx.hpp"
#include "video/transitions.hpp"

namespace antares {

const int32_t kDirectionError = 5;   // how picky in degrees we are about angle
const int32_t kShootAngle     = 15;  // how picky we are about shooting in degrees
const int32_t kParanoiaAngle  = 30;  // angle of terror
const int32_t kEvadeAngle     = 30;  // we'd like to turn this far away

const uint32_t kMotionMargin    = 5000;  // margin of change in distance before we care
const uint32_t kLandingDistance = 1000;
const uint32_t kWarpInDistance  = 16777216;

const ticks   kRechargeSpeed      = ticks(12);
const int32_t kHealthRatio        = 5;
const int32_t kWeaponRatio        = 2;
const int32_t kEnergyChunk        = kHealthRatio + (kWeaponRatio * 3);
const int32_t kWarpInEnergyFactor = 3;

static const int16_t kShipDestroyedString = 17;

static const ticks    kCollideFlashDuration = ticks{3};
static const RgbColor kCollideFlashColor    = rgba(255, 255, 255, 127);

uint32_t ThinkObjectNormalPresence(Handle<SpaceObject> anObject, const BaseObject* baseObject);
uint32_t ThinkObjectWarpingPresence(Handle<SpaceObject> anObject);
uint32_t ThinkObjectWarpInPresence(Handle<SpaceObject> anObject);
uint32_t ThinkObjectWarpOutPresence(Handle<SpaceObject> anObject, const BaseObject* baseObject);
uint32_t ThinkObjectLandingPresence(Handle<SpaceObject> anObject);
void     ThinkObjectGetCoordVector(
            Handle<SpaceObject> anObject, Point* dest, uint32_t* distance, int16_t* angle);
void ThinkObjectGetCoordDistance(Handle<SpaceObject> anObject, Point* dest, uint32_t* distance);
void ThinkObjectResolveDestination(
        Handle<SpaceObject> anObject, Point* dest, Handle<SpaceObject>* targetObject);
bool ThinkObjectResolveTarget(
        Handle<SpaceObject> anObject, Point* dest, uint32_t* distance,
        Handle<SpaceObject>* targetObject);
uint32_t ThinkObjectEngageTarget(
        Handle<SpaceObject> anObject, Handle<SpaceObject> targetObject, uint32_t distance,
        int16_t* theta);

void SpaceObject::recharge() {
    if ((_energy < (max_energy() - kEnergyChunk)) && (_battery > kEnergyChunk)) {
        _battery -= kEnergyChunk;
        _energy += kEnergyChunk;
    }

    if ((_health < (max_health() / 2)) && (_energy > kHealthRatio)) {
        _health++;
        _energy -= kHealthRatio;
    }

    for (auto* weapon : {&pulse, &beam, &special}) {
        if (!weapon->base ||                                        // no weapon in this slot
            (weapon->ammo >= (weapon->base->device->ammo >> 1)) ||  // ammo above half maximum
            (_energy < kWeaponRatio)) {                             // not enough energy to charge
            continue;
        }
        weapon->charge++;
        _energy -= kWeaponRatio;

        if ((weapon->base->device->restockCost < 0) ||               // non-rechargeable weapon
            (weapon->charge < weapon->base->device->restockCost)) {  // not enough charge yet
            continue;
        }
        weapon->charge -= weapon->base->device->restockCost;
        weapon->ammo++;
    }
}

static void tick_weapon(
        Handle<SpaceObject> subject, Handle<SpaceObject> target, uint32_t key,
        const sfz::optional<BaseObject::Weapon>& base_weapon, SpaceObject::Weapon& weapon) {
    if (subject->keysDown & key) {
        fire_weapon(
                subject, target, weapon,
                base_weapon.has_value() ? base_weapon->positions : std::vector<fixedPointType>{});
    }
}

void fire_weapon(
        Handle<SpaceObject> subject, Handle<SpaceObject> target, SpaceObject::Weapon& weapon,
        const std::vector<fixedPointType>& positions) {
    if ((weapon.time > g.time) || !weapon.base) {
        return;
    }

    auto weaponObject = weapon.base;
    if ((subject->energy() < weaponObject->device->energyCost) ||
        ((weaponObject->device->ammo > 0) && (weapon.ammo <= 0))) {
        return;
    }
    if ((&weapon != &subject->special) && (subject->cloakState > 0)) {
        subject->set_cloak(false);
    }
    subject->_energy -= weaponObject->device->energyCost;
    weapon.position++;
    if (weapon.position >= positions.size()) {
        weapon.position = 0;
    }

    int16_t angle = subject->direction;
    mAddAngle(angle, -90);
    Fixed fcos, fsin;
    GetRotPoint(&fcos, &fsin, angle);
    fcos = -fcos;
    fsin = -fsin;

    Point offset;
    Point at = {0, 0};
    if ((&weapon != &subject->special) && !positions.empty()) {
        offset.h = mFixedToLong(
                (positions[weapon.position].h * fcos) + (positions[weapon.position].v * -fsin));
        offset.v = mFixedToLong(
                (positions[weapon.position].h * fsin) + (positions[weapon.position].v * fcos));
        at = offset;
    }

    weapon.time = g.time + weaponObject->device->fireTime;
    if (weaponObject->device->ammo > 0) {
        weapon.ammo--;
    }
    exec(weaponObject->activate.action, subject, target, at);
}

static void tick_pulse(Handle<SpaceObject> subject, Handle<SpaceObject> target) {
    tick_weapon(subject, target, kPulseKey, subject->base->weapons.pulse, subject->pulse);
}

static void tick_beam(Handle<SpaceObject> subject, Handle<SpaceObject> target) {
    tick_weapon(subject, target, kBeamKey, subject->base->weapons.beam, subject->beam);
}

static void tick_special(Handle<SpaceObject> subject, Handle<SpaceObject> target) {
    tick_weapon(subject, target, kSpecialKey, subject->base->weapons.special, subject->special);
}

static uint8_t get_tiny_shade(const SpaceObject& o) {
    switch (o.layer) {
        case BaseObject::Layer::NONE: return DARK; break;
        case BaseObject::Layer::BASES: return MEDIUM; break;
        case BaseObject::Layer::SHIPS: return LIGHT; break;
        case BaseObject::Layer::SHOTS: return LIGHTEST; break;
    }
}

void NonplayerShipThink() {
    uint8_t friendSick, foeSick, neutralSick;
    switch ((std::chrono::time_point_cast<ticks>(g.time).time_since_epoch().count() / 9) % 4) {
        case 0: friendSick = foeSick = neutralSick = MEDIUM; break;
        case 1: friendSick = foeSick = neutralSick = DARK; break;
        case 2: friendSick = foeSick = neutralSick = DARKER; break;
        case 3:
            friendSick = neutralSick = DARKEST;
            foeSick                  = DARKER - 1;
            break;
    }

    g.sync = g.random.seed;
    for (int32_t count = 0; count < kMaxPlayerNum; count++) {
        Handle<Admiral>(count)->shipsLeft() = 0;
    }

    // it probably doesn't matter what order we do this in, but we'll do
    // it in the "ideal" order anyway
    SpaceObject* o = nullptr;
    for (auto o_handle = g.root; (o = o_handle.get()); o_handle = o->nextObject) {
        if (!o->active) {
            continue;
        }

        g.sync += o->location.h;
        g.sync += o->location.v;

        // strobe its symbol if it's not feeling well
        if (o->sprite.get()) {
            if ((o->health() > 0) && (o->health() <= (o->max_health() >> 2))) {
                if (o->owner == g.admiral) {
                    o->sprite->tinyColor.shade = friendSick;
                } else if (o->owner.get()) {
                    o->sprite->tinyColor.shade = foeSick;
                } else {
                    o->sprite->tinyColor.shade = neutralSick;
                }
            } else {
                o->sprite->tinyColor.shade = get_tiny_shade(*o);
            }
        }

        // if the object can think, or is human controlled
        if (!(o->attributes & (kCanThink | kRemoteOrHuman))) {
            continue;
        }

        // get the object's base object
        auto baseObject = o->base;
        o->targetAngle = o->directionGoal = o->direction;

        // incremenent its admiral's # of ships
        if (o->owner.get()) {
            o->owner->shipsLeft()++;
        }

        uint32_t keysDown;
        switch (o->presenceState) {
            case kNormalPresence:
                keysDown = ThinkObjectNormalPresence(o_handle, baseObject);
                break;

            case kWarpingPresence: keysDown = ThinkObjectWarpingPresence(o_handle); break;

            case kWarpInPresence: keysDown = ThinkObjectWarpInPresence(o_handle); break;

            case kWarpOutPresence:
                keysDown = ThinkObjectWarpOutPresence(o_handle, baseObject);
                break;

            case kLandingPresence: keysDown = ThinkObjectLandingPresence(o_handle); break;
        }

        if (!(o->attributes & kRemoteOrHuman) || (o->attributes & kOnAutoPilot)) {
            if (o->attributes & kHasDirectionGoal) {
                if (o->attributes & kShapeFromDirection) {
                    if ((o->attributes & kIsGuided) && o->targetObject.get()) {
                        int32_t difference = o->targetAngle - o->direction;
                        if ((difference < -60) || (difference > 60)) {
                            o->targetObject   = SpaceObject::none();
                            o->targetObjectID = kNoShip;
                            o->directionGoal  = o->direction;
                        }
                    }
                }
                Point offset;
                offset.h           = mAngleDifference(o->directionGoal, o->direction);
                offset.v           = mFixedToLong(o->turn_rate() << 1);
                int32_t difference = ABS(offset.h);
                if (difference > offset.v) {
                    if (offset.h < 0) {
                        keysDown |= kRightKey;
                    } else if (offset.h > 0) {
                        keysDown |= kLeftKey;
                    }
                }
            }
            // and here?
            if (!(o->keysDown & kManualOverrideFlag)) {
                if (o->closestDistance < kEngageRange) {
                    // why do we only do this randomly when closest is within engagerange?
                    // to simulate the innaccuracy of battle
                    // (to keep things from wiggling, really)
                    if (o->randomSeed.next(baseObject->ai.combat.skill.den) <
                        baseObject->ai.combat.skill.num) {
                        o->keysDown &= ~kMotionKeyMask;
                        o->keysDown |= keysDown & kMotionKeyMask;
                    }
                    if (o->randomSeed.next(3) == 1) {
                        o->keysDown &= ~kWeaponKeyMask;
                        o->keysDown |= keysDown & kWeaponKeyMask;
                    }
                    o->keysDown &= ~kMiscKeyMask;
                    o->keysDown |= keysDown & kMiscKeyMask;
                } else {
                    o->keysDown = (o->keysDown & kSpecialKeyMask) | keysDown;
                }
            } else {
                o->keysDown &= ~kManualOverrideFlag;
            }
        }

        // Take care of any "keys" being pressed
        if (o->keysDown & kAdoptTargetKey) {
            SetObjectDestination(o_handle);
        }
        if (o->keysDown & kAutoPilotKey) {
            TogglePlayerAutoPilot(o_handle);
        }
        if (o->keysDown & kGiveCommandKey) {
            PlayerShipGiveCommand(o->owner);
        }
        o->keysDown &= ~kSpecialKeyMask;

        if (o->offlineTime > 0) {
            if (o->randomSeed.next(o->offlineTime) > 5) {
                o->keysDown = 0;
            }
            o->offlineTime--;
        }

        if ((o->attributes & kRemoteOrHuman) && (!(o->attributes & kCanThink)) &&
            (!o->expires || (o->expire_after < secs(2)))) {
            PlayerShipBodyExpire(o_handle);
        }

        if ((o->attributes & kHasDirectionGoal) && (o->offlineTime <= 0)) {
            if (o->keysDown & kLeftKey) {
                o->turnVelocity = -o->turn_rate();
            } else if (o->keysDown & kRightKey) {
                o->turnVelocity = o->turn_rate();
            } else {
                o->turnVelocity = Fixed::zero();
            }
        }

        if (o->keysDown & kUpKey) {
            if ((o->presenceState != kWarpInPresence) && (o->presenceState != kWarpingPresence) &&
                (o->presenceState != kWarpOutPresence)) {
                o->thrust = baseObject->thrust;
            }
        } else if (o->keysDown & kDownKey) {
            o->thrust = -baseObject->thrust;
        } else {
            o->thrust = Fixed::zero();
        }

        if (o->rechargeTime < kRechargeSpeed) {
            o->rechargeTime += kMajorTick;
        } else {
            o->rechargeTime = ticks(0);

            if (o->presenceState == kWarpingPresence) {
                o->collect_warp_energy(1);
            } else if (o->presenceState == kNormalPresence) {
                o->recharge();
            }
        }

        // targetObject is set for all three weapons -- do not change
        auto targetObject = SpaceObject::none();
        if (o->targetObject.get()) {
            targetObject = o->targetObject;
        }

        tick_pulse(o_handle, targetObject);
        tick_beam(o_handle, targetObject);
        tick_special(o_handle, targetObject);

        if ((o->keysDown & kWarpKey) && (baseObject->warpSpeed > Fixed::zero()) &&
            (o->energy() > 0)) {
            if (o->presenceState == kWarpingPresence) {
                o->thrust = baseObject->thrust * o->presence.warping;
            } else if (o->presenceState == kWarpOutPresence) {
                o->thrust = baseObject->thrust * o->presence.warp_out;
            } else if (
                    (o->presenceState == kNormalPresence) &&
                    (o->energy() > (o->max_energy() >> kWarpInEnergyFactor))) {
                o->presenceState             = kWarpInPresence;
                o->presence.warp_in.step     = 0;
                o->presence.warp_in.progress = ticks(0);
            }
        } else {
            if (o->presenceState == kWarpInPresence) {
                o->presenceState = kNormalPresence;
            } else if (o->presenceState == kWarpingPresence) {
                o->presenceState = kWarpOutPresence;
            } else if (o->presenceState == kWarpOutPresence) {
                o->thrust = baseObject->thrust * o->presence.warp_out;
            }
        }
    }
}

uint32_t use_weapons_for_defense(Handle<SpaceObject> obj) {
    uint32_t keys = 0;

    if (obj->pulse.base) {
        auto weaponObject = obj->pulse.base;
        if (weaponObject->device->usage.defense) {
            keys |= kPulseKey;
        }
    }

    if (obj->beam.base) {
        auto weaponObject = obj->beam.base;
        if (weaponObject->device->usage.defense) {
            keys |= kBeamKey;
        }
    }

    if (obj->special.base) {
        auto weaponObject = obj->special.base;
        if (weaponObject->device->usage.defense) {
            keys |= kSpecialKey;
        }
    }

    return keys;
}

static bool can_engage(const Handle<SpaceObject>& a, const Handle<SpaceObject>& b) {
    return (a->attributes & kCanEngage) && (b->attributes & kCanBeEngaged);
}

static bool can_evade(const Handle<SpaceObject>& a, const Handle<SpaceObject>& b) {
    return (a->attributes & kCanEvade) && (b->attributes & kCanBeEvaded);
}

uint32_t ThinkObjectNormalPresence(Handle<SpaceObject> anObject, const BaseObject* baseObject) {
    uint32_t keysDown = anObject->keysDown & kSpecialKeyMask;

    if (!(anObject->attributes & kRemoteOrHuman) || (anObject->attributes & kOnAutoPilot)) {
        // if target object exists and is within engage range
        Handle<SpaceObject> targetObject;
        Point               dest;
        uint32_t            distance;
        ThinkObjectResolveTarget(anObject, &dest, &distance, &targetObject);

        ///--->>> BEGIN TARGETING <<<---///
        if ((anObject->targetObject.get()) &&
            ((anObject->attributes & kIsGuided) ||
             (can_engage(anObject, targetObject) && !(anObject->attributes & kRemoteOrHuman) &&
              (distance < static_cast<uint32_t>(anObject->engageRange)) &&
              (anObject->timeFromOrigin < kTimeToCheckHome)))) {
            int16_t theta;
            keysDown |= ThinkObjectEngageTarget(anObject, targetObject, distance, &theta);
            ///--->>> END TARGETING <<<---///

            // if I'm in target object's range & it's looking at us & my health is less
            // than 1/2 its -- or I can't engage it
            if (can_evade(anObject, targetObject) &&
                (distance < static_cast<uint32_t>(targetObject->longestWeaponRange)) &&
                (targetObject->attributes & kHated) && (ABS(theta) < kParanoiaAngle) &&
                ((!(targetObject->attributes & kCanBeEngaged)) ||
                 (anObject->health() <= targetObject->health()))) {
                // try to evade, flee, run away
                if (anObject->attributes & kHasDirectionGoal) {
                    keysDown |= use_weapons_for_defense(anObject);

                    anObject->directionGoal = targetObject->direction;

                    int16_t angle_offset = kEvadeAngle;
                    if (targetObject->attributes & kIsGuided) {
                        angle_offset = 90;
                    }
                    if (theta > 0) {
                        mAddAngle(anObject->directionGoal, angle_offset);
                    } else if (theta < 0) {
                        mAddAngle(anObject->directionGoal, -angle_offset);
                    } else if (anObject->location.h & 0x00000001) {
                        mAddAngle(anObject->directionGoal, -angle_offset);
                    } else {
                        mAddAngle(anObject->directionGoal, angle_offset);
                    }
                } else if (anObject->randomSeed.next(2)) {
                    mAddAngle(anObject->direction, -kEvadeAngle);
                } else {
                    mAddAngle(anObject->direction, kEvadeAngle);
                }
                keysDown |= kUpKey;
            } else if (
                    (distance > static_cast<uint32_t>(anObject->shortestWeaponRange)) ||
                    (anObject->attributes & kIsGuided)) {
                // if we're not afraid, then
                // if we are not within our closest weapon range then
                keysDown |= kUpKey;
            } else if (
                    (distance < kMotionMargin) ||
                    ((distance + kMotionMargin) <
                     static_cast<uint32_t>(anObject->lastTargetDistance))) {
                // if we are as close as we like
                // if we're getting closer
                keysDown |= kDownKey;
                anObject->lastTargetDistance = distance;
            } else if (
                    (distance - kMotionMargin) >
                    static_cast<uint32_t>(anObject->lastTargetDistance)) {
                // if we're not getting closer, then if we're getting farther
                keysDown |= kUpKey;
                anObject->lastTargetDistance = distance;
            }

            if ((anObject->targetObject == anObject->destObject) &&
                (distance < static_cast<uint32_t>(baseObject->arrive.distance.squared)) &&
                !baseObject->arrive.action.empty() && !(anObject->runTimeFlags & kHasArrived)) {
                exec(baseObject->arrive.action, anObject, anObject->destObject, {0, 0});
                anObject->runTimeFlags |= kHasArrived;
            }
        } else if (anObject->attributes & kIsGuided) {
            keysDown |= kUpKey;
        } else {  // not guided & no target object or target object is out of engage range
            ///--->>> BEGIN TARGETING <<<---///
            if ((anObject->targetObject.get()) &&
                (((!(anObject->attributes & kRemoteOrHuman)) &&
                  (distance < static_cast<uint32_t>(anObject->engageRange))) ||
                 (anObject->attributes & kIsGuided))) {
                int16_t theta;
                keysDown |= ThinkObjectEngageTarget(anObject, targetObject, distance, &theta);
                if (can_engage(anObject, targetObject) &&
                    (distance < static_cast<uint32_t>(anObject->longestWeaponRange)) &&
                    (targetObject->attributes & kHated)) {
                } else if (
                        can_evade(anObject, targetObject) && (targetObject->attributes & kHated) &&
                        (((distance < static_cast<uint32_t>(targetObject->longestWeaponRange)) &&
                          (ABS(theta) < kParanoiaAngle)) ||
                         (targetObject->attributes & kIsGuided))) {
                    // try to evade, flee, run away
                    if (anObject->attributes & kHasDirectionGoal) {
                        if (distance < static_cast<uint32_t>(anObject->longestWeaponRange)) {
                            keysDown |= use_weapons_for_defense(anObject);
                        }

                        anObject->directionGoal = targetObject->direction;
                        if (theta > 0) {
                            mAddAngle(anObject->directionGoal, kEvadeAngle);
                        } else if (theta < 0) {
                            mAddAngle(anObject->directionGoal, -kEvadeAngle);
                        } else if (anObject->location.h & 0x00000001) {
                            mAddAngle(anObject->directionGoal, -kEvadeAngle);
                        } else {
                            mAddAngle(anObject->directionGoal, kEvadeAngle);
                        }
                    } else if (anObject->randomSeed.next(2)) {
                        mAddAngle(anObject->direction, -kEvadeAngle);
                    } else {
                        mAddAngle(anObject->direction, kEvadeAngle);
                    }
                    keysDown |= kUpKey;
                }
            }
            ///--->>> END TARGETING <<<---///
            if ((anObject->attributes & kIsDestination) ||
                (!anObject->destObject.get() &&
                 (anObject->destinationLocation.h == kNoDestinationCoord))) {
                if (anObject->attributes & kOnAutoPilot) {
                    TogglePlayerAutoPilot(anObject);
                }
                keysDown |= kDownKey;
                anObject->timeFromOrigin = ticks(0);
            } else {
                if (anObject->destObject.get()) {
                    targetObject = anObject->destObject;
                    if (targetObject.get() && targetObject->active &&
                        (targetObject->id == anObject->destObjectID)) {
                        if (targetObject->seenByPlayerFlags & anObject->myPlayerFlag) {
                            dest.h                          = targetObject->location.h;
                            dest.v                          = targetObject->location.v;
                            anObject->destinationLocation.h = dest.h;
                            anObject->destinationLocation.v = dest.v;
                        } else {
                            dest.h = anObject->destinationLocation.h;
                            dest.v = anObject->destinationLocation.v;
                        }
                        anObject->destObjectDest   = targetObject->destObject;
                        anObject->destObjectDestID = targetObject->destObjectID;
                    } else {
                        anObject->duty = eNoDuty;
                        anObject->attributes &= ~kStaticDestination;
                        if (!targetObject.get()) {
                            keysDown |= kDownKey;
                            anObject->destObjectDest = SpaceObject::none();
                            anObject->destObject     = SpaceObject::none();
                            dest.h                   = anObject->location.h;
                            dest.v                   = anObject->location.v;
                            if (anObject->attributes & kOnAutoPilot) {
                                TogglePlayerAutoPilot(anObject);
                            }
                        } else {
                            anObject->destObject = anObject->destObjectDest;
                            if (anObject->destObject.get()) {
                                targetObject = anObject->destObject;
                                if (targetObject->id != anObject->destObjectDestID) {
                                    targetObject = SpaceObject::none();
                                }
                            } else {
                                targetObject = SpaceObject::none();
                            }
                            if (targetObject.get()) {
                                anObject->destObjectID     = targetObject->id;
                                anObject->destObjectDest   = targetObject->destObject;
                                anObject->destObjectDestID = targetObject->destObjectID;
                                dest.h                     = targetObject->location.h;
                                dest.v                     = targetObject->location.v;
                            } else {
                                anObject->duty = eNoDuty;
                                keysDown |= kDownKey;
                                anObject->destObject     = SpaceObject::none();
                                anObject->destObjectDest = SpaceObject::none();
                                dest.h                   = anObject->location.h;
                                dest.v                   = anObject->location.v;
                                if (anObject->attributes & kOnAutoPilot) {
                                    TogglePlayerAutoPilot(anObject);
                                }
                            }
                        }
                    }
                } else {  // no destination object; just coords
                    if (anObject->attributes & kOnAutoPilot) {
                        TogglePlayerAutoPilot(anObject);
                    }
                    targetObject = SpaceObject::none();
                    dest.h       = anObject->destinationLocation.h;
                    dest.v       = anObject->destinationLocation.v;
                }

                int16_t angle;
                ThinkObjectGetCoordVector(anObject, &dest, &distance, &angle);

                int16_t theta;
                if (anObject->attributes & kHasDirectionGoal) {
                    theta = mAngleDifference(angle, anObject->directionGoal);
                    if (ABS(theta) > kDirectionError) {
                        anObject->directionGoal = angle;
                    }

                    theta = mAngleDifference(anObject->direction, anObject->directionGoal);
                    theta = ABS(theta);
                } else {
                    anObject->direction = angle;
                    theta               = 0;
                }

                if (distance < kEngageRange) {
                    anObject->timeFromOrigin = ticks(0);
                }

                if (distance > static_cast<uint32_t>(baseObject->arrive.distance.squared)) {
                    if (theta < kEvadeAngle) {
                        keysDown |= kUpKey;
                    }
                    anObject->lastTargetDistance = distance;
                    if (anObject->special.base && (distance > kWarpInDistance) &&
                        (theta <= kDirectionError)) {
                        if (anObject->special.base->device->usage.transportation) {
                            keysDown |= kSpecialKey;
                        }
                    }
                    if ((baseObject->warpSpeed > Fixed::zero()) &&
                        (anObject->energy() > (anObject->max_energy() >> kWarpInEnergyFactor)) &&
                        (distance > kWarpInDistance) && (theta <= kDirectionError)) {
                        keysDown |= kWarpKey;
                    }
                } else {
                    if (targetObject.get() && (targetObject->owner == anObject->owner) &&
                        (targetObject->attributes & anObject->attributes & kHasDirectionGoal)) {
                        anObject->directionGoal = targetObject->direction;
                        if ((targetObject->keysDown & kWarpKey) &&
                            (baseObject->warpSpeed > Fixed::zero())) {
                            theta = mAngleDifference(anObject->direction, targetObject->direction);
                            if (ABS(theta) < kDirectionError) {
                                keysDown |= kWarpKey;
                            }
                        }
                    }

                    if (distance < static_cast<uint32_t>(baseObject->arrive.distance.squared)) {
                        if (baseObject->arrive.action.size() > 0) {
                            if (!(anObject->runTimeFlags & kHasArrived)) {
                                exec(baseObject->arrive.action, anObject, anObject->destObject,
                                     {0, 0});
                                anObject->runTimeFlags |= kHasArrived;
                            }
                        }
                    }

                    // if we're getting closer
                    if ((distance + kMotionMargin) <
                        static_cast<uint32_t>(anObject->lastTargetDistance)) {
                        keysDown |= kDownKey;
                        anObject->lastTargetDistance = distance;
                        // if we're not getting closer, then if we're getting farther
                    } else if (
                            (distance - kMotionMargin) >
                            static_cast<uint32_t>(anObject->lastTargetDistance)) {
                        if (theta < kEvadeAngle) {
                            keysDown |= kUpKey;
                        } else {
                            keysDown |= kDownKey;
                        }
                        anObject->lastTargetDistance = distance;
                    }
                }
            }
        }
    } else {  // object is human controlled -- we need to calc target angle
        Handle<SpaceObject> targetObject;
        Point               dest;
        uint32_t            distance;
        ThinkObjectResolveTarget(anObject, &dest, &distance, &targetObject);

        if ((anObject->attributes & kCanEngage) &&
            (distance < static_cast<uint32_t>(anObject->engageRange)) &&
            (anObject->targetObject.get())) {
            // if target is in our weapon range & we hate the object
            if ((distance < static_cast<uint32_t>(anObject->longestWeaponRange)) &&
                (targetObject->attributes & kHated)) {
                // find "best" weapon (how do we want to aim?)
                const BaseObject* beam          = anObject->beam.base;
                const BaseObject* pulse         = anObject->pulse.base;
                const BaseObject* special       = anObject->special.base;
                const BaseObject* bestWeapon    = beam;
                int32_t           closest_range = anObject->longestWeaponRange;

                if (beam && beam->device->usage.attacking &&
                    (static_cast<uint32_t>(beam->device->range.squared) >= distance) &&
                    (beam->device->range.squared < closest_range)) {
                    bestWeapon    = beam;
                    closest_range = beam->device->range.squared;
                }

                if (pulse && pulse->device->usage.attacking &&
                    (static_cast<uint32_t>(pulse->device->range.squared) >= distance) &&
                    (pulse->device->range.squared < closest_range)) {
                    bestWeapon    = pulse;
                    closest_range = pulse->device->range.squared;
                }

                if (special && special->device->usage.attacking &&
                    (static_cast<uint32_t>(special->device->range.squared) >= distance) &&
                    (special->device->range.squared < closest_range)) {
                    bestWeapon    = special;
                    closest_range = special->device->range.squared;
                }

                // offset dest for anticipated position -- overkill?
                if (bestWeapon) {
                    uint32_t dcalc = lsqrt(distance);
                    Fixed    fdist = Fixed::from_long(dcalc) * bestWeapon->device->speed.inverse;
                    dest.h -= mFixedToLong(
                            (targetObject->velocity.h - anObject->velocity.h) * fdist);
                    dest.v -= mFixedToLong(
                            (targetObject->velocity.v - anObject->velocity.v) * fdist);
                }
            }  // target is not in our weapon range (or we don't hate it)

            // this is human controlled--if it's too far away, tough nougies
            // find angle between me & dest
            Fixed slope = MyFixRatio(anObject->location.h - dest.h, anObject->location.v - dest.v);
            int16_t angle = AngleFromSlope(slope);

            if (dest.h < anObject->location.h) {
                mAddAngle(angle, 180);
            } else if ((anObject->location.h == dest.h) && (dest.v < anObject->location.v)) {
                angle = 0;
            }

            if (targetObject->cloakState > 250) {
                angle -= 45;
                mAddAngle(angle, anObject->randomSeed.next(90));
            }
            anObject->targetAngle = angle;
        }
    }

    return keysDown;
}

uint32_t ThinkObjectWarpInPresence(Handle<SpaceObject> anObject) {
    uint32_t keysDown = anObject->keysDown & kSpecialKeyMask;

    if ((!(anObject->attributes & kRemoteOrHuman)) || (anObject->attributes & kOnAutoPilot)) {
        keysDown = kWarpKey;
    }
    auto& presence = anObject->presence.warp_in;
    presence.progress += kMajorTick;
    for (int i = 0; i < 4; ++i) {
        if ((presence.step == i) && (presence.progress > ticks(25 * i))) {
            sys.sound.warp(i, anObject);
            ++presence.step;
            break;
        }
    }

    if (presence.progress > ticks(100)) {
        if (anObject->collect_warp_energy(anObject->max_energy() >> kWarpInEnergyFactor)) {
            anObject->presenceState    = kWarpingPresence;
            anObject->presence.warping = anObject->base->warpSpeed;
            anObject->attributes &= ~kOccupiesSpace;
            CreateAnySpaceObject(
                    *kWarpInFlare, {Fixed::zero(), Fixed::zero()}, anObject->location,
                    anObject->direction, Admiral::none(), 0, sfz::nullopt);
        } else {
            anObject->presenceState = kNormalPresence;
            anObject->_energy       = 0;
        }
    }

    return keysDown;
}

uint32_t ThinkObjectWarpingPresence(Handle<SpaceObject> anObject) {
    uint32_t keysDown = anObject->keysDown & kSpecialKeyMask;

    if (anObject->energy() <= 0) {
        anObject->presenceState = kWarpOutPresence;
    }
    if ((!(anObject->attributes & kRemoteOrHuman)) || (anObject->attributes & kOnAutoPilot)) {
        Point               dest;
        Handle<SpaceObject> target;
        ThinkObjectResolveDestination(anObject, &dest, &target);
        uint32_t distance;
        int16_t  angle;
        ThinkObjectGetCoordVector(anObject, &dest, &distance, &angle);

        if (!(anObject->attributes & kHasDirectionGoal)) {
            anObject->direction = angle;
        } else if (ABS(mAngleDifference(angle, anObject->directionGoal)) > kDirectionError) {
            anObject->directionGoal = angle;
        }

        if ((distance >= anObject->base->warpOutDistance.squared) ||
            (target.get() && ((target->presenceState == kWarpInPresence) ||
                              (target->presenceState == kWarpingPresence)))) {
            keysDown |= kWarpKey;
        }
    }
    return keysDown;
}

uint32_t ThinkObjectWarpOutPresence(Handle<SpaceObject> anObject, const BaseObject* baseObject) {
    uint32_t keysDown = anObject->keysDown & kSpecialKeyMask;
    anObject->presence.warp_out -= Fixed::from_long(kWarpAcceleration);
    if (anObject->presence.warp_out < anObject->maxVelocity) {
        anObject->refund_warp_energy();
        anObject->presenceState = kNormalPresence;
        anObject->attributes |= baseObject->attributes & kOccupiesSpace;

        // Clamp speed to max velocity in current direction
        Fixed x, y;
        GetRotPoint(&x, &y, anObject->direction);
        anObject->velocity = fixedPointType{
                anObject->maxVelocity * x,
                anObject->maxVelocity * y,
        };

        CreateAnySpaceObject(
                *kWarpOutFlare, {Fixed::zero(), Fixed::zero()}, anObject->location,
                anObject->direction, Admiral::none(), 0, sfz::nullopt);
    }
    return keysDown;
}

uint32_t ThinkObjectLandingPresence(Handle<SpaceObject> anObject) {
    uint32_t keysDown = 0;

    Handle<SpaceObject> target;
    uint32_t            distance;
    int16_t             theta = 0;

    // we repeat an object's normal action for having a destination

    if ((anObject->attributes & kIsDestination) ||
        (!anObject->destObject.get() &&
         (anObject->destinationLocation.h == kNoDestinationCoord))) {
        if (anObject->attributes & kOnAutoPilot) {
            TogglePlayerAutoPilot(anObject);
        }
        keysDown |= kDownKey;
        distance = 0;
    } else {
        Point dest;
        if (anObject->destObject.get()) {
            target = anObject->destObject;
            if (target.get() && target->active && (target->id == anObject->destObjectID)) {
                if (target->seenByPlayerFlags & anObject->myPlayerFlag) {
                    dest.h                          = target->location.h;
                    dest.v                          = target->location.v;
                    anObject->destinationLocation.h = dest.h;
                    anObject->destinationLocation.v = dest.v;
                } else {
                    dest.h = anObject->destinationLocation.h;
                    dest.v = anObject->destinationLocation.v;
                }
                anObject->destObjectDest   = target->destObject;
                anObject->destObjectDestID = target->destObjectID;
            } else {
                anObject->duty = eNoDuty;
                anObject->attributes &= ~kStaticDestination;
                if (!target.get()) {
                    keysDown |= kDownKey;
                    anObject->destObject     = SpaceObject::none();
                    anObject->destObjectDest = SpaceObject::none();
                    dest.h                   = anObject->location.h;
                    dest.v                   = anObject->location.v;
                } else {
                    anObject->destObject = anObject->destObjectDest;
                    if (anObject->destObject.get()) {
                        target = anObject->destObject;
                        if (target->id != anObject->destObjectDestID) {
                            target = SpaceObject::none();
                        }
                    } else {
                        target = SpaceObject::none();
                    }
                    if (target.get()) {
                        anObject->destObjectID     = target->id;
                        anObject->destObjectDest   = target->destObject;
                        anObject->destObjectDestID = target->destObjectID;
                        dest.h                     = target->location.h;
                        dest.v                     = target->location.v;
                    } else {
                        keysDown |= kDownKey;
                        anObject->destObject     = SpaceObject::none();
                        anObject->destObjectDest = SpaceObject::none();
                        dest.h                   = anObject->location.h;
                        dest.v                   = anObject->location.v;
                    }
                }
            }
        } else {  // no destination object; just coords
            if (anObject->attributes & kOnAutoPilot) {
                TogglePlayerAutoPilot(anObject);
            }
            dest.h = anObject->location.h;
            dest.v = anObject->location.v;
        }

        int16_t  angle;
        uint32_t xdiff = ABS<int>(dest.h - anObject->location.h);
        uint32_t ydiff = ABS<int>(dest.v - anObject->location.v);
        if ((xdiff > kMaximumAngleDistance) || (ydiff > kMaximumAngleDistance)) {
            if ((xdiff > kMaximumRelevantDistance) || (ydiff > kMaximumRelevantDistance)) {
                distance = kMaximumRelevantDistanceSquared;
            } else {
                distance = ydiff * ydiff + xdiff * xdiff;
            }
            int16_t shortx = (anObject->location.h - dest.h) >> 4;
            int16_t shorty = (anObject->location.v - dest.v) >> 4;
            // find angle between me & dest
            Fixed slope = MyFixRatio(shortx, shorty);
            angle       = AngleFromSlope(slope);
            if (shortx > 0) {
                mAddAngle(angle, 180);
            } else if ((shortx == 0) && (shorty > 0)) {
                angle = 0;
            }
        } else {
            distance = ydiff * ydiff + xdiff * xdiff;

            // find angle between me & dest
            Fixed slope = MyFixRatio(anObject->location.h - dest.h, anObject->location.v - dest.v);
            angle       = AngleFromSlope(slope);

            if (dest.h < anObject->location.h) {
                mAddAngle(angle, 180);
            } else if ((anObject->location.h == dest.h) && (dest.v < anObject->location.v)) {
                angle = 0;
            }
        }

        if (anObject->attributes & kHasDirectionGoal) {
            if (ABS(mAngleDifference(angle, anObject->directionGoal)) > kDirectionError) {
                anObject->directionGoal = angle;
            }
            theta = ABS(mAngleDifference(anObject->direction, anObject->directionGoal));
        } else {
            anObject->direction = angle;
        }
    }

    if (distance > kLandingDistance) {
        if (theta < kEvadeAngle) {
            keysDown |= kUpKey;
        } else {
            keysDown |= kDownKey;
        }
        anObject->lastTargetDistance = distance;
    } else {
        keysDown |= kDownKey;
        anObject->presence.landing.scale.factor -= anObject->presence.landing.speed;
    }

    if (anObject->presence.landing.scale <= Scale{0}) {
        exec(anObject->base->expire.action, anObject, target, {0, 0});
        anObject->active = kObjectToBeFreed;
    } else if (anObject->sprite.get()) {
        anObject->sprite->scale = anObject->presence.landing.scale;
    }

    return keysDown;
}

// this gets the distance & angle between an object and arbitrary coords
void ThinkObjectGetCoordVector(
        Handle<SpaceObject> anObject, Point* dest, uint32_t* distance, int16_t* angle) {
    int32_t  difference;
    uint32_t dcalc;
    int16_t  shortx, shorty;
    Fixed    slope;

    difference = ABS<int>(dest->h - anObject->location.h);
    dcalc      = difference;
    difference = ABS<int>(dest->v - anObject->location.v);
    *distance  = difference;
    if ((*distance == 0) && (dcalc == 0)) {
        *angle = anObject->direction;
        return;
    }

    if ((dcalc > kMaximumAngleDistance) || (*distance > kMaximumAngleDistance)) {
        if ((dcalc > kMaximumRelevantDistance) || (*distance > kMaximumRelevantDistance)) {
            *distance = kMaximumRelevantDistanceSquared;
        } else {
            *distance = *distance * *distance + dcalc * dcalc;
        }
        shortx = (anObject->location.h - dest->h) >> 4;
        shorty = (anObject->location.v - dest->v) >> 4;
        // find angle between me & dest
        slope  = MyFixRatio(shortx, shorty);
        *angle = AngleFromSlope(slope);
        if (shortx > 0) {
            mAddAngle(*angle, 180);
        } else if ((shortx == 0) && (shorty > 0)) {
            *angle = 0;
        }
    } else {
        *distance = *distance * *distance + dcalc * dcalc;

        // find angle between me & dest
        slope  = MyFixRatio(anObject->location.h - dest->h, anObject->location.v - dest->v);
        *angle = AngleFromSlope(slope);

        if (dest->h < anObject->location.h)
            mAddAngle(*angle, 180);
        else if ((anObject->location.h == dest->h) && (dest->v < anObject->location.v))
            *angle = 0;
    }
}

void ThinkObjectGetCoordDistance(Handle<SpaceObject> anObject, Point dest, uint32_t* distance) {
    int32_t  difference;
    uint32_t dcalc;

    difference = ABS<int>(dest.h - anObject->location.h);
    dcalc      = difference;
    difference = ABS<int>(dest.v - anObject->location.v);
    *distance  = difference;
    if ((*distance == 0) && (dcalc == 0)) {
        return;
    }

    if ((dcalc > kMaximumAngleDistance) || (*distance > kMaximumAngleDistance)) {
        if ((dcalc > kMaximumRelevantDistance) || (*distance > kMaximumRelevantDistance)) {
            *distance = kMaximumRelevantDistanceSquared;
        } else {
            *distance = *distance * *distance + dcalc * dcalc;
        }
    } else {
        *distance = *distance * *distance + dcalc * dcalc;
    }
}

// this resolves an object's destination to its coordinates, returned in dest
void ThinkObjectResolveDestination(
        Handle<SpaceObject> anObject, Point* dest, Handle<SpaceObject>* targetObject) {
    *targetObject = SpaceObject::none();

    if ((anObject->attributes & kIsDestination) ||
        ((!anObject->destObject.get()) &&
         (anObject->destinationLocation.h == kNoDestinationCoord))) {
        if (anObject->attributes & kOnAutoPilot) {
            TogglePlayerAutoPilot(anObject);
        }
        dest->h = anObject->location.h;
        dest->v = anObject->location.v;
    } else {
        if (anObject->destObject.get()) {
            *targetObject = anObject->destObject;
            if ((*targetObject).get() && ((*targetObject)->active) &&
                ((*targetObject)->id == anObject->destObjectID)) {
                if ((*targetObject)->seenByPlayerFlags & anObject->myPlayerFlag) {
                    dest->h                         = (*targetObject)->location.h;
                    dest->v                         = (*targetObject)->location.v;
                    anObject->destinationLocation.h = dest->h;
                    anObject->destinationLocation.v = dest->v;
                } else {
                    dest->h = anObject->destinationLocation.h;
                    dest->v = anObject->destinationLocation.v;
                }
                anObject->destObjectDest   = (*targetObject)->destObject;
                anObject->destObjectDestID = (*targetObject)->destObjectID;
            } else {
                anObject->duty = eNoDuty;
                anObject->attributes &= ~kStaticDestination;
                if (!(*targetObject).get()) {
                    anObject->destObject     = SpaceObject::none();
                    anObject->destObjectDest = SpaceObject::none();
                    dest->h                  = anObject->location.h;
                    dest->v                  = anObject->location.v;
                } else {
                    anObject->destObject = anObject->destObjectDest;
                    if (anObject->destObject.get()) {
                        (*targetObject) = anObject->destObject;
                        if ((*targetObject)->id != anObject->destObjectDestID) {
                            *targetObject = SpaceObject::none();
                        }
                    } else {
                        *targetObject = SpaceObject::none();
                    }
                    if ((*targetObject).get()) {
                        anObject->destObjectID     = (*targetObject)->id;
                        anObject->destObjectDest   = (*targetObject)->destObject;
                        anObject->destObjectDestID = (*targetObject)->destObjectID;
                        dest->h                    = (*targetObject)->location.h;
                        dest->v                    = (*targetObject)->location.v;
                    } else {
                        anObject->duty           = eNoDuty;
                        anObject->destObject     = SpaceObject::none();
                        anObject->destObjectDest = SpaceObject::none();
                        dest->h                  = anObject->location.h;
                        dest->v                  = anObject->location.v;
                    }
                }
            }
        } else  // no destination object; just coords
        {
            (*targetObject) = SpaceObject::none();
            if (anObject->destinationLocation.h == kNoDestinationCoord) {
                if (anObject->attributes & kOnAutoPilot) {
                    TogglePlayerAutoPilot(anObject);
                }
                dest->h = anObject->location.h;
                dest->v = anObject->location.v;
            } else {
                dest->h = anObject->destinationLocation.h;
                dest->v = anObject->destinationLocation.v;
            }
        }
    }
}

bool ThinkObjectResolveTarget(
        Handle<SpaceObject> o, Point* dest, uint32_t* distance, Handle<SpaceObject>* target) {
    *target      = o->targetObject;
    auto closest = o->closestObject;

    // if we have no target, then
    if (!o->targetObject.get()) {
        if (!closest.get() || !(closest->attributes & kPotentialTarget)) {
            // no target, no closest, cancel
            *target = o->targetObject = SpaceObject::none();
            o->targetObjectID         = kNoShip;
            *dest                     = o->location;
            *distance                 = o->engageRange;
            return false;
        }
        // if the closest object is appropriate (if it exists, it should be)
        // select closest object as target (and for now be satisfied with our direction)
        if (o->attributes & kHasDirectionGoal) {
            o->directionGoal = o->direction;
        }
        *target = o->targetObject = closest;
        o->targetObjectID         = (*target)->id;
    }

    // if the object is wrong or smells at all funny, then
    if ((!((*target)->active)) ||                                                // Inactive
        ((*target)->id != o->targetObjectID) ||                                  // Recreated
        (((*target)->owner == o->owner) && ((*target)->attributes & kHated)) ||  // Hated friendly
        ((!((*target)->attributes & kPotentialTarget)) &&
         (!((*target)->attributes & kHated)))) {  // Non-hated invalid target
        if (!closest.get() || !(closest->attributes & kPotentialTarget)) {
            // no legal target, no closest, cancel
            *target = o->targetObject = SpaceObject::none();
            o->targetObjectID         = kNoShip;
            *dest                     = o->location;
            *distance                 = o->engageRange;
            return false;
        }
        // if we have a closest ship make it our target
        *target = o->targetObject = closest;
        o->targetObjectID         = (*target)->id;
    }

    *dest = (*target)->location;
    // if it's not the closest object & we have a closest object
    if ((closest.get()) && (o->targetObject != closest) && (!(o->attributes & kIsGuided)) &&
        (closest->attributes & kPotentialTarget)) {
        // then calculate the distance
        ThinkObjectGetCoordDistance(o, *dest, distance);
        if (((*distance >> 1L) > o->closestDistance) || (!(o->attributes & kCanEngage)) ||
            (o->attributes & kRemoteOrHuman)) {
            *target = o->targetObject = closest;
            o->targetObjectID         = (*target)->id;
            *dest                     = (*target)->location;
            *distance                 = o->closestDistance;
            if ((*target)->cloakState > 250) {
                dest->h -= 200;
                dest->v -= 200;
            }
        }
    } else {
        // otherwise if target is closest object then distance is the closestDistance
        *distance = o->closestDistance;
    }
    return true;
}

uint32_t ThinkObjectEngageTarget(
        Handle<SpaceObject> anObject, Handle<SpaceObject> targetObject, uint32_t distance,
        int16_t* theta) {
    Point dest = {targetObject->location.h, targetObject->location.v};
    if (targetObject->cloakState > 250) {
        dest.h -= 70;
        dest.h += anObject->randomSeed.next(140);
        dest.v -= 70;
        dest.v += anObject->randomSeed.next(140);
    }

    // if target is in our weapon range & we hate the object
    if ((distance < static_cast<uint32_t>(anObject->longestWeaponRange)) &&
        (targetObject->attributes & kCanBeEngaged) && (targetObject->attributes & kHated) &&
        (anObject->attributes & kCanAcceptDestination)) {
        anObject->timeFromOrigin += kMajorTick;
    }

    // We don't need to worry if it is very far away, since it must be within farthest weapon range
    // find angle between me & dest
    Fixed   slope = MyFixRatio(anObject->location.h - dest.h, anObject->location.v - dest.v);
    int16_t angle = AngleFromSlope(slope);
    if (dest.h < anObject->location.h) {
        mAddAngle(angle, 180);
    } else if ((anObject->location.h == dest.h) && (dest.v < anObject->location.v)) {
        angle = 0;
    }

    if (targetObject->cloakState > 250) {
        angle -= 45;
        mAddAngle(angle, anObject->randomSeed.next(90));
    }
    anObject->targetAngle = angle;

    if (anObject->attributes & kHasDirectionGoal) {
        *theta = mAngleDifference(angle, anObject->directionGoal);
        if ((ABS(*theta) > kDirectionError) || (!(anObject->attributes & kIsGuided))) {
            anObject->directionGoal = angle;
        }

        int16_t beta = targetObject->direction;
        mAddAngle(beta, ROT_180);
        *theta = mAngleDifference(beta, angle);
    } else {
        anObject->direction = angle;
        *theta              = 0;
    }

    // if target object is not in range or not hated
    if ((distance >= static_cast<uint32_t>(anObject->longestWeaponRange)) ||
        !(targetObject->attributes & kHated)) {
        return 0;
    }  // else fire away

    bool              in_front = ABS(mAngleDifference(anObject->direction, angle)) <= kShootAngle;
    const BaseObject* pulse    = anObject->pulse.base;
    const BaseObject* beam     = anObject->beam.base;
    const BaseObject* special  = anObject->special.base;

    uint32_t keysDown = 0;
    if (pulse && pulse->device->usage.attacking &&
        (in_front || (pulse->attributes & kAutoTarget)) &&
        (distance < static_cast<uint32_t>(pulse->device->range.squared))) {
        keysDown |= kPulseKey;
    }
    if (beam && beam->device->usage.attacking && (in_front || (beam->attributes & kAutoTarget)) &&
        (distance < static_cast<uint32_t>(beam->device->range.squared))) {
        keysDown |= kBeamKey;
    }
    if (special && special->device->usage.attacking &&
        (in_front || (special->attributes & kAutoTarget)) &&
        (distance < static_cast<uint32_t>(special->device->range.squared))) {
        keysDown |= kSpecialKey;
    }
    return keysDown;
}

static bool can_hit(const Handle<SpaceObject>& a, const Handle<SpaceObject>& b) {
    return (a->attributes & kCanCollide) && (b->attributes & kCanBeHit);
}

void HitObject(Handle<SpaceObject> anObject, Handle<SpaceObject> sObject) {
    if (anObject->active != kObjectInUse) {
        return;
    } else if (!can_hit(sObject, anObject)) {
        return;
    }

    anObject->timeFromOrigin = ticks(0);
    if (((anObject->_health - sObject->base->collide.damage) < 0) &&
        (anObject->attributes & (kIsPlayerShip | kRemoteOrHuman)) && anObject->base->destroy.die) {
        anObject->create_floating_player_body();
    }
    anObject->alter_health(-sObject->base->collide.damage);
    if (anObject->shieldColor.has_value()) {
        anObject->hitState = (anObject->health() * kHitStateMax) / anObject->max_health();
        anObject->hitState += 16;
    }

    if (anObject->cloakState > 0) {
        anObject->cloakState = 1;
    }

    if (anObject->health() < 0 && (anObject->owner == g.admiral) &&
        (anObject->attributes & kCanAcceptDestination)) {
        int count = CountObjectsOfBaseType(anObject->base, anObject->owner) - 1;
        Messages::add(pn::format(
                sys.messages.at(kShipDestroyedString).c_str(), anObject->long_name(), count));
    }

    if (sObject->active == kObjectInUse) {
        exec(sObject->base->collide.action, sObject, anObject, {0, 0});
    }

    if (anObject->owner == g.admiral && (anObject->attributes & kIsPlayerShip) &&
        (sObject->base->collide.damage > 0)) {
        globals()->transitions.start_boolean(kCollideFlashDuration, kCollideFlashColor);
    }
}

static bool allegiance_is(
        Allegiance allegiance, Handle<Admiral> admiral, Handle<SpaceObject> object) {
    switch (allegiance) {
        case FRIENDLY_OR_HOSTILE: return true;
        case FRIENDLY: return object->owner == admiral;
        case HOSTILE: return object->owner != admiral;
    }
}

// GetManualSelectObject:
//  For the human player selecting a ship.  If friend or foe = 0, will get any ship.  If it's
//  positive, will get only friendly ships.  If it's negative, only unfriendly ships.

Handle<SpaceObject> GetManualSelectObject(
        Handle<SpaceObject> sourceObject, int32_t direction, uint32_t inclusiveAttributes,
        uint32_t exclusiveAttributes, const uint64_t* fartherThan, Handle<SpaceObject> currentShip,
        Allegiance allegiance) {
    const uint32_t myOwnerFlag = 1 << sourceObject->owner.number();

    uint64_t wideClosestDistance = 0x3fffffff3fffffffull;
    uint64_t wideFartherDistance = 0x3fffffff3fffffffull;

    // Here's what you've got to do next:
    // start with the currentShip
    // try to get any ship but the current ship
    // stop trying when we've made a full circle (we're back on currentShip)

    Handle<SpaceObject> anObject;
    auto                whichShip = currentShip;
    auto                startShip = currentShip;
    if (whichShip.get()) {
        anObject = startShip;
        if (anObject->active != kObjectInUse) {  // if it's not in the loop
            anObject  = g.root;
            startShip = whichShip = g.root;
        }
    } else {
        anObject  = g.root;
        startShip = whichShip = g.root;
    }

    Handle<SpaceObject> nextShipOut, closestShip;
    do {
        if (anObject->active && (anObject != sourceObject) &&
            (anObject->seenByPlayerFlags & myOwnerFlag) &&
            (anObject->attributes & inclusiveAttributes) &&
            !(anObject->attributes & exclusiveAttributes) &&
            allegiance_is(allegiance, sourceObject->owner, anObject)) {
            uint32_t xdiff = ABS<int>(sourceObject->location.h - anObject->location.h);
            uint32_t ydiff = ABS<int>(sourceObject->location.v - anObject->location.v);

            uint64_t thisWideDistance;
            if ((xdiff > kMaximumRelevantDistance) || (ydiff > kMaximumRelevantDistance)) {
                thisWideDistance =
                        MyWideMul<uint64_t>(xdiff, xdiff) + MyWideMul<uint64_t>(ydiff, ydiff);
            } else {
                thisWideDistance = ydiff * ydiff + xdiff * xdiff;
            }

            // Nearer than the nearest candidate so far.
            bool is_closest = thisWideDistance < wideClosestDistance;

            // Farther than *fartherThan, but nearer than any other candidate so far.
            bool is_closest_far_object =
                    (thisWideDistance > *fartherThan) && (wideFartherDistance > thisWideDistance);

            if (is_closest || is_closest_far_object) {
                int32_t hdif = sourceObject->location.h - anObject->location.h;
                int32_t vdif = sourceObject->location.v - anObject->location.v;
                while ((ABS(hdif) > kMaximumAngleDistance) ||
                       (ABS(vdif) > kMaximumAngleDistance)) {
                    hdif >>= 1;
                    vdif >>= 1;
                }

                int16_t angle = AngleFromSlope(MyFixRatio(hdif, vdif));

                if (hdif > 0) {
                    mAddAngle(angle, 180);
                } else if ((hdif == 0) && (vdif > 0)) {
                    angle = 0;
                }

                if (ABS(mAngleDifference(angle, direction)) < 30) {
                    if (is_closest) {
                        closestShip         = whichShip;
                        wideClosestDistance = thisWideDistance;
                    }

                    if (is_closest_far_object) {
                        nextShipOut         = whichShip;
                        wideFartherDistance = thisWideDistance;
                    }
                }
            }
        }
        whichShip = anObject = anObject->nextObject;
        if (!anObject.get()) {
            whichShip = anObject = g.root;
        }
    } while (whichShip != startShip);

    if ((!nextShipOut.get() && closestShip.get()) || (nextShipOut == currentShip)) {
        nextShipOut = closestShip;
    }

    return nextShipOut;
}

Handle<SpaceObject> GetSpritePointSelectObject(
        Rect* bounds, Handle<SpaceObject> sourceObject, uint32_t anyOneAttribute,
        Handle<SpaceObject> currentShip, Allegiance allegiance) {
    const uint32_t myOwnerFlag = 1 << sourceObject->owner.number();

    Handle<SpaceObject> resultShip, closestShip;
    for (auto anObject : SpaceObject::all()) {
        if (!anObject->active || !anObject->sprite.get() ||
            !(anObject->seenByPlayerFlags & myOwnerFlag) ||
            ((anyOneAttribute != 0) && ((anObject->attributes & anyOneAttribute) == 0)) ||
            !allegiance_is(allegiance, sourceObject->owner, anObject) ||
            (bounds->right < anObject->sprite->where.h) ||
            (bounds->bottom < anObject->sprite->where.v) ||
            (bounds->left > anObject->sprite->where.h) ||
            (bounds->top > anObject->sprite->where.v)) {
            continue;
        }
        if (!closestShip.get()) {
            closestShip = anObject;
        }
        if ((anObject.number() > currentShip.number()) && !resultShip.get()) {
            resultShip = anObject;
        }
    }
    if ((!resultShip.get() && closestShip.get()) || (resultShip == currentShip)) {
        resultShip = closestShip;
    }

    return resultShip;
}

}  // namespace antares

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

struct ActionCursor {
    const Action* begin = nullptr;
    const Action* end   = nullptr;

    Handle<SpaceObject> subject;
    int32_t             subject_id;
    Handle<SpaceObject> direct;
    int32_t             direct_id;

    Point offset;

    std::unique_ptr<ActionCursor> continuation;

    ActionCursor() = default;
    ActionCursor(
            const std::vector<Action>& actions, Handle<SpaceObject> subject,
            Handle<SpaceObject> direct, Point offset)
            : begin{actions.data()},
              end{actions.data() + actions.size()},
              subject{subject},
              subject_id{subject.get() ? subject->id : -1},
              direct{direct},
              direct_id{direct.get() ? direct->id : -1},
              offset{offset} {}
    ActionCursor(
            const std::vector<Action>& actions, Handle<SpaceObject> subject,
            Handle<SpaceObject> direct, Point offset, ActionCursor continuation)
            : begin{actions.data()},
              end{actions.data() + actions.size()},
              subject{subject},
              subject_id{subject.get() ? subject->id : -1},
              direct{direct},
              direct_id{direct.get() ? direct->id : -1},
              offset{offset},
              continuation{new ActionCursor{std::move(continuation)}} {}
};

struct actionQueueType {
    ActionCursor     cursor;
    ticks            scheduledTime;
    actionQueueType* nextActionQueue;

    bool empty() const { return cursor.begin == cursor.end; }
};

static ANTARES_GLOBAL actionQueueType* gFirstActionQueue = NULL;

static ANTARES_GLOBAL unique_ptr<actionQueueType[]> gActionQueueData;

#ifdef DATA_COVERAGE
ANTARES_GLOBAL set<int32_t> covered_actions;
#endif  // DATA_COVERAGE

static void queue_action(ActionCursor cursor, ticks delayTime);

bool action_filter_applies_to(const Action& action, Handle<SpaceObject> target) {
    if (!tags_match(*target->base, action.base.filter.tags)) {
        return false;
    }

    if (action.base.filter.attributes.bits & ~target->attributes) {
        return false;
    }

    return true;
}

static Point random_point_in_circle(Random* r, int32_t distance) {
    int32_t angle  = r->next(ROT_POS);
    int32_t radius = std::max(r->next(distance + 1), r->next(distance + 1));
    Fixed   x, y;
    GetRotPoint(&x, &y, angle);
    Point p = {mFixedToLong(x * radius), mFixedToLong(y * radius)};
    return p;
}

static Point random_point_in_square(Random* r, int32_t distance) {
    Point p = {r->next(distance * 2) - distance, r->next(distance * 2) - distance};
    return p;
}

static Point random_point(Random* r, int32_t distance, Within within) {
    switch (within) {
        case Within::CIRCLE: return random_point_in_circle(r, distance);
        case Within::SQUARE: return random_point_in_square(r, distance);
    }
}

static void apply(
        const CreateAction& a, Handle<SpaceObject> subject, Handle<SpaceObject> direct,
        Point offset) {
    auto count = a.count.value_or(Range<int64_t>{1, 2});
    auto c     = count.begin;
    if (count.range() > 1) {
        c += direct->randomSeed.next(count.range());
    } else if (a.legacy_random.value_or(false)) {
        // It used to be that the range test above was >0 instead of >1. That worked for most
        // objects, which had ranges of 0. However, the Nastiroid shooter on Mothership Connection
        // specified a range of 1. This was meaningless as far as the actual object count went, but
        // caused a random number to be consumed unnecessarily. To preserve replay-compatibility,
        // it now specifies `legacy_random`.
        direct->randomSeed.next(1);
    }
    for (int i = 0; i < c; ++i) {
        fixedPointType vel = {Fixed::zero(), Fixed::zero()};
        if (a.relative_velocity.value_or(false)) {
            vel = direct->velocity;
        }
        int32_t direction = 0;
        if (a.base->attributes & kAutoTarget) {
            direction = direct->targetAngle;
        } else if (a.relative_direction.value_or(false)) {
            direction = direct->direction;
        }
        coordPointType at = direct->location;
        at.h += offset.h;
        at.v += offset.v;

        if (a.distance.has_value()) {
            Point p = random_point(&direct->randomSeed, *a.distance, a.within);
            at.h += p.h;
            at.v += p.v;
        }

        auto product = CreateAnySpaceObject(
                *a.base, &vel, &at, direction, direct->owner, 0, sfz::nullopt);
        if (!product.get()) {
            continue;
        }

        if (product->attributes & kCanAcceptDestination) {
            uint32_t save_attributes = product->attributes;
            product->attributes &= ~kStaticDestination;
            if (product->owner.get()) {
                if (a.inherit.value_or(false) && direct->destObject.get()) {
                    OverrideObjectDestination(product, direct->destObject);
                } else {
                    OverrideObjectDestination(product, direct);
                }
            } else {
                product->timeFromOrigin = kTimeToCheckHome;
                product->runTimeFlags &= ~kHasArrived;
                product->destObject       = direct;  // a->destinationObject;
                product->destObjectDest   = direct->destObject;
                product->destObjectID     = direct->id;
                product->destObjectDestID = direct->destObjectID;
            }
            product->attributes = save_attributes;
        }
        product->targetObject   = direct->targetObject;
        product->targetObjectID = direct->targetObjectID;
        product->closestObject  = product->targetObject;

        //  ugly though it is, we have to fill in the rest of
        //  a new beam's fields after it's created.
        if (product->attributes & kIsVector) {
            if (product->frame.vector->is_ray) {
                // special beams need special post-creation acts
                Vectors::set_attributes(product, direct);
            }
        }
    }
}

static void apply(
        const PlayAction& a, Handle<SpaceObject> subject, Handle<SpaceObject> direct,
        Point offset) {
    pn::string_view pick;
    if (a.sound.has_value()) {
        pick = *a.sound;
    } else if (a.any.size() > 1) {
        pick = a.any[direct->randomSeed.next(a.any.size())].sound;
    } else {
        return;
    }

    if (a.absolute.value_or(false)) {
        sys.sound.play(pick, a.volume, a.persistence, a.priority.level);
    } else {
        sys.sound.play_at(pick, a.volume, a.persistence, a.priority.level, direct);
    }
}

static void apply(
        const SparkAction& a, Handle<SpaceObject> subject, Handle<SpaceObject> direct,
        Point offset) {
    Point location;
    if (direct->sprite.get()) {
        location.h = direct->sprite->where.h;
        location.v = direct->sprite->where.v;
    } else {
        int32_t l = (direct->location.h - gGlobalCorner.h) * gAbsoluteScale;
        l >>= SHIFT_SCALE;
        if ((l > -kSpriteMaxSize) && (l < kSpriteMaxSize)) {
            location.h = l + viewport().left;
        } else {
            location.h = -kSpriteMaxSize;
        }

        l = (direct->location.v - gGlobalCorner.v) * gAbsoluteScale;
        l >>= SHIFT_SCALE; /*+ CLIP_TOP*/
        if ((l > -kSpriteMaxSize) && (l < kSpriteMaxSize)) {
            location.v = l + viewport().top;
        } else {
            location.v = -kSpriteMaxSize;
        }
    }
    int32_t decay = round(1023 / a.age.count());
    globals()->starfield.make_sparks(a.count, decay, a.velocity, a.hue, &location);
}

static void apply(
        const DestroyAction& a, Handle<SpaceObject> subject, Handle<SpaceObject> direct,
        Point offset) {
    if (!direct.get()) {
        return;
    }
    // If the object is occupied by a human, eject a body since players can't die.
    if ((direct->attributes & (kIsPlayerShip | kRemoteOrHuman)) && direct->base->destroy.die) {
        direct->create_floating_player_body();
    }
    direct->destroy();
}

static void apply(
        const RemoveAction& a, Handle<SpaceObject> subject, Handle<SpaceObject> direct,
        Point offset) {
    if (!direct.get()) {
        return;
    }
    // If the object is occupied by a human, eject a body since players can't die.
    if ((direct->attributes & (kIsPlayerShip | kRemoteOrHuman)) && direct->base->destroy.die) {
        direct->create_floating_player_body();
    }
    direct->active = kObjectToBeFreed;
}

static void apply(
        const HoldAction& a, Handle<SpaceObject> subject, Handle<SpaceObject> direct,
        Point offset) {
    direct->targetObject   = SpaceObject::none();
    direct->targetObjectID = kNoShip;
    direct->lastTarget     = SpaceObject::none();
}

static void apply(
        const HealAction& a, Handle<SpaceObject> subject, Handle<SpaceObject> direct,
        Point offset) {
    direct->alter_health(a.value);
}

static void apply(
        const EnergizeAction& a, Handle<SpaceObject> subject, Handle<SpaceObject> direct,
        Point offset) {
    direct->alter_energy(a.value);
}

static void apply(
        const RevealAction& a, Handle<SpaceObject> subject, Handle<SpaceObject> direct,
        Point offset) {
    for (auto i : a.initial) {
        UnhideInitialObject(i);
    }
}

static void apply(
        const CheckAction& a, Handle<SpaceObject> subject, Handle<SpaceObject> direct,
        Point offset) {
    CheckLevelConditions();
}

static void apply(
        const CloakAction& a, Handle<SpaceObject> subject, Handle<SpaceObject> direct,
        Point offset) {
    direct->set_cloak(true);
}

static void apply(
        const SpinAction& a, Handle<SpaceObject> subject, Handle<SpaceObject> direct,
        Point offset) {
    if (direct->attributes & kCanTurn) {
        Fixed f = direct->turn_rate() * (a.value.begin + direct->randomSeed.next(a.value.range()));
        Fixed f2 = direct->base->mass;
        if (f2 == Fixed::zero()) {
            f = kFixedNone;
        } else {
            f /= f2;
        }
        direct->turnVelocity = f;
    }
}

static void apply(
        const DisableAction& a, Handle<SpaceObject> subject, Handle<SpaceObject> direct,
        Point offset) {
    Fixed begin = Fixed::from_long(a.duration.begin.count()) / 3;
    Fixed end   = Fixed::from_long(a.duration.end.count()) / 3;
    Fixed mass  = direct->base->mass;
    Fixed f     = begin + direct->randomSeed.next(end - begin);
    if (mass == Fixed::zero()) {
        f = kFixedNone;
    } else {
        f /= mass;
    }
    direct->offlineTime = mFixedToLong(f);
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

static void apply(
        const PushAction& a, Handle<SpaceObject> subject, Handle<SpaceObject> direct,
        Point offset) {
    if (!direct.get()) {
        return;
    }

    if (a.value.has_value()) {
        Fixed fx, fy;
        GetRotPoint(&fx, &fy, subject->direction);
        direct->velocity = {*a.value * fx, *a.value * fy};
    } else if ((direct->base->mass > Fixed::zero()) && (direct->maxVelocity > Fixed::zero())) {
        // if colliding, then PUSH the direct like collision
        direct->velocity.h +=
                ((subject->velocity.h - direct->velocity.h) / direct->base->mass.val()) << 6L;
        direct->velocity.v +=
                ((subject->velocity.v - direct->velocity.v) / direct->base->mass.val()) << 6L;

        // make sure we're not going faster than our top speed
        cap_velocity(direct);
    }
}

static void apply(
        const CapSpeedAction& a, Handle<SpaceObject> subject, Handle<SpaceObject> direct,
        Point offset) {
    if (a.value.has_value()) {
        direct->maxVelocity = *a.value;
    } else {
        direct->maxVelocity = direct->base->maxVelocity;
    }
}

static void apply(
        const ThrustAction& a, Handle<SpaceObject> subject, Handle<SpaceObject> direct,
        Point offset) {
    Fixed f        = a.value.begin + direct->randomSeed.next(a.value.range());
    direct->thrust = f;
}

static void apply(
        const MorphAction& a, Handle<SpaceObject> subject, Handle<SpaceObject> direct,
        Point offset) {
    if (direct.get()) {
        direct->change_base_type(*a.base, sfz::nullopt, a.keep_ammo.value_or(false));
    }
}

static void apply(
        const CaptureAction& a, Handle<SpaceObject> subject, Handle<SpaceObject> direct,
        Point offset) {
    if (!direct.get()) {
        return;
    }
    if (a.player.has_value()) {
        direct->set_owner(*a.player, false);
    } else {
        direct->set_owner(subject->owner, true);
    }
}

static void apply(
        const ConditionAction& a, Handle<SpaceObject> subject, Handle<SpaceObject> direct,
        Point offset) {
    for (auto c : a.enable) {
        g.condition_enabled[c.number()] = true;
    }
    for (auto c : a.disable) {
        g.condition_enabled[c.number()] = false;
    }
}

static void apply(
        const OccupyAction& a, Handle<SpaceObject> subject, Handle<SpaceObject> direct,
        Point offset) {
    if (direct.get()) {
        direct->alter_occupation(subject->owner, a.value, true);
    }
}

static void apply(
        const PayAction& a, Handle<SpaceObject> subject, Handle<SpaceObject> direct,
        Point offset) {
    Handle<Admiral> admiral;
    if (a.player.has_value()) {
        admiral = *a.player;
    } else {
        if (direct.get()) {
            admiral = direct->owner;
        }
    }
    if (admiral.get()) {
        admiral->pay_absolute(a.value);
    }
}

static void apply(
        const AgeAction& a, Handle<SpaceObject> subject, Handle<SpaceObject> direct,
        Point offset) {
    ticks t = a.value.begin + direct->randomSeed.next(a.value.range());

    if (a.relative.value_or(false)) {
        if (direct->expires) {
            direct->expire_after += t;
        } else {
            direct->expire_after += t;
            direct->expires = (direct->expire_after >= ticks(0));
        }
    } else {
        direct->expire_after = t;
        direct->expires      = (direct->expire_after >= ticks(0));
    }
}

static void apply(
        const MoveAction& a, Handle<SpaceObject> subject, Handle<SpaceObject> direct,
        Point offset) {
    coordPointType newLocation;
    switch (a.origin.value_or(MoveAction::Origin::LEVEL)) {
        case MoveAction::Origin::LEVEL: newLocation = {kUniversalCenter, kUniversalCenter}; break;
        case MoveAction::Origin::SUBJECT:
            newLocation = a.reflexive ? direct->location : subject->location;
            break;
        case MoveAction::Origin::DIRECT:
            newLocation = a.reflexive ? subject->location : direct->location;
            break;
    }

    coordPointType off = a.to.value_or(coordPointType{0, 0});
    off                = Translate_Coord_To_Level_Rotation(off.h, off.v);
    newLocation.h += off.h - kUniversalCenter;
    newLocation.v += off.v - kUniversalCenter;

    Point random = random_point(&direct->randomSeed, a.distance.value_or(0), a.within);
    newLocation.h += random.h;
    newLocation.v += random.v;

    direct->location.h = newLocation.h;
    direct->location.v = newLocation.v;
}

static void alter_weapon(
        const BaseObject* base, Handle<SpaceObject> direct, SpaceObject::Weapon& weapon) {
    weapon.base     = base;
    weapon.time     = g.time;
    weapon.position = 0;
    if (!base) {
        weapon.ammo = 0;
        return;
    }

    weapon.ammo = base->device->ammo;
    if (base->device->range.squared > direct->longestWeaponRange) {
        direct->longestWeaponRange = base->device->range.squared;
    }
    if (base->device->range.squared < direct->shortestWeaponRange) {
        direct->shortestWeaponRange = base->device->range.squared;
    }
}

static void apply(
        const EquipAction& a, Handle<SpaceObject> subject, Handle<SpaceObject> direct,
        Point offset) {
    switch (a.which) {
        case Weapon::PULSE: alter_weapon(a.base.get(), direct, direct->pulse); break;
        case Weapon::BEAM: alter_weapon(a.base.get(), direct, direct->beam); break;
        case Weapon::SPECIAL: alter_weapon(a.base.get(), direct, direct->special); break;
    }
}

static void apply(
        const LandAction& a, Handle<SpaceObject> subject, Handle<SpaceObject> direct,
        Point offset) {
    // even though this is never a reflexive verb, we only effect ourselves
    if (direct->attributes & (kIsPlayerShip | kRemoteOrHuman)) {
        direct->create_floating_player_body();
    }
    direct->presenceState          = kLandingPresence;
    direct->presence.landing.speed = a.speed;
    direct->presence.landing.scale = sprite_scale(*direct->base);
}

static void apply(
        const WarpAction& a, Handle<SpaceObject> subject, Handle<SpaceObject> direct,
        Point offset) {
    direct->presenceState             = kWarpInPresence;
    direct->presence.warp_in.progress = ticks(0);
    direct->presence.warp_in.step     = 0;
    direct->attributes &= ~kOccupiesSpace;
    fixedPointType newVel = {Fixed::zero(), Fixed::zero()};
    CreateAnySpaceObject(
            *kWarpInFlare, &newVel, &direct->location, direct->direction, Admiral::none(), 0,
            sfz::nullopt);
}

static void apply(
        const ScoreAction& a, Handle<SpaceObject> subject, Handle<SpaceObject> direct,
        Point offset) {
    Counter counter;
    counter.which = a.counter.which;
    if (a.counter.player.has_value()) {
        counter.player = *a.counter.player;
    } else if (direct.get()) {
        counter.player = direct->owner;
    }
    if (counter.player.get()) {
        AlterAdmiralScore(counter, a.value);
    }
}

static void apply(
        const WinAction& a, Handle<SpaceObject> subject, Handle<SpaceObject> direct,
        Point offset) {
    Handle<Admiral> admiral;
    if (a.player.has_value()) {
        admiral = *a.player;
    } else if (direct.get()) {
        admiral = direct->owner;
    }
    if (a.next.has_value()) {
        DeclareWinner(admiral, a.next->get(), a.text);
    } else {
        DeclareWinner(admiral, nullptr, a.text);
    }
}

static void apply(
        const MessageAction& a, Handle<SpaceObject> subject, Handle<SpaceObject> direct,
        Point offset) {
    Messages::start(a.id, &a.pages);
}

static void apply(
        const TargetAction& a, Handle<SpaceObject> subject, Handle<SpaceObject> direct,
        Point offset) {
    uint32_t save_attributes = subject->attributes;
    subject->attributes &= ~kStaticDestination;
    OverrideObjectDestination(subject, direct);
    subject->attributes = save_attributes;
}

static void apply(
        const FireAction& a, Handle<SpaceObject> subject, Handle<SpaceObject> direct,
        Point offset) {
    switch (a.which) {
        case Weapon::PULSE:
            fire_weapon(
                    direct, SpaceObject::none(), direct->pulse,
                    direct->base->weapons.pulse.has_value()
                            ? direct->base->weapons.pulse->positions
                            : std::vector<fixedPointType>{});
            break;
        case Weapon::BEAM:
            fire_weapon(
                    direct, SpaceObject::none(), direct->beam,
                    direct->base->weapons.beam.has_value() ? direct->base->weapons.beam->positions
                                                           : std::vector<fixedPointType>{});
            break;
        case Weapon::SPECIAL:
            fire_weapon(
                    direct, SpaceObject::none(), direct->special,
                    direct->base->weapons.special.has_value()
                            ? direct->base->weapons.special->positions
                            : std::vector<fixedPointType>{});
            break;
    }
}

static void apply(
        const FlashAction& a, Handle<SpaceObject> subject, Handle<SpaceObject> direct,
        Point offset) {
    globals()->transitions.start_boolean(a.duration, a.color);
}

static void apply(
        const KeyAction& a, Handle<SpaceObject> subject, Handle<SpaceObject> direct,
        Point offset) {
    for (KeyAction::Key key : a.enable) {
        g.key_mask = g.key_mask & ~(1 << static_cast<int32_t>(key));
    }
    for (KeyAction::Key key : a.disable) {
        g.key_mask = g.key_mask | (1 << static_cast<int32_t>(key));
    }
}

static void apply(
        const ZoomAction& a, Handle<SpaceObject> subject, Handle<SpaceObject> direct,
        Point offset) {
    if (a.value != g.zoom) {
        g.zoom = a.value;
        sys.sound.click();
        Messages::zoom(g.zoom);
    }
}

static void apply(
        const SelectAction& a, Handle<SpaceObject> subject, Handle<SpaceObject> direct,
        Point offset) {
    MiniComputer_SetScreenAndLineHack(a.screen, a.line);
}

static void apply(
        const SlowAction& a, Handle<SpaceObject> subject, Handle<SpaceObject> direct,
        Point offset) {
    if (!(direct.get() && (direct->maxVelocity > Fixed::zero()))) {
        return;
    }

    // if decelerating, then STOP the direct like applying brakes
    direct->velocity.h += direct->velocity.h * (a.value - Fixed::from_long(1));
    direct->velocity.v += direct->velocity.v * (a.value - Fixed::from_long(1));

    // make sure we're not going faster than our top speed
    cap_velocity(direct);
}

static void apply(
        const SpeedAction& a, Handle<SpaceObject> subject, Handle<SpaceObject> direct,
        Point offset) {
    if (!direct.get()) {
        return;
    }

    Fixed fx, fy;
    GetRotPoint(&fx, &fy, direct->direction);
    if (a.relative.value_or(false)) {
        direct->velocity.h += a.value * fx;
        direct->velocity.v += a.value * fy;
    } else {
        direct->velocity = {a.value * fx, a.value * fy};
    }
}

static void apply(
        const StopAction& a, Handle<SpaceObject> subject, Handle<SpaceObject> direct,
        Point offset) {
    if (!direct.get()) {
        return;
    }
    direct->velocity = {Fixed::zero(), Fixed::zero()};
}

static void apply(
        const AssumeAction& a, Handle<SpaceObject> subject, Handle<SpaceObject> direct,
        Point offset) {
    int index            = a.which + GetAdmiralScore({Handle<Admiral>{0}, 0});
    g.initials[index]    = direct;
    g.initial_ids[index] = direct->id;
}

static ActionCursor apply(
        const Action& a, Handle<SpaceObject> subject, Handle<SpaceObject> direct, Point offset,
        ActionCursor next) {
    switch (a.type()) {
        case Action::Type::DELAY:
            queue_action(std::move(next), a.delay.duration);
            return ActionCursor{};

        case Action::Type::GROUP:
            return ActionCursor{a.group.of, subject, direct, offset, std::move(next)};

        case Action::Type::AGE: apply(a.age, subject, direct, offset); break;
        case Action::Type::ASSUME: apply(a.assume, subject, direct, offset); break;
        case Action::Type::CAPTURE: apply(a.capture, subject, direct, offset); break;
        case Action::Type::CAP_SPEED: apply(a.cap_speed, subject, direct, offset); break;
        case Action::Type::CHECK: apply(a.check, subject, direct, offset); break;
        case Action::Type::CLOAK: apply(a.cloak, subject, direct, offset); break;
        case Action::Type::CONDITION: apply(a.condition, subject, direct, offset); break;
        case Action::Type::CREATE: apply(a.create, subject, direct, offset); break;
        case Action::Type::DESTROY: apply(a.destroy, subject, direct, offset); break;
        case Action::Type::DISABLE: apply(a.disable, subject, direct, offset); break;
        case Action::Type::ENERGIZE: apply(a.energize, subject, direct, offset); break;
        case Action::Type::EQUIP: apply(a.equip, subject, direct, offset); break;
        case Action::Type::FIRE: apply(a.fire, subject, direct, offset); break;
        case Action::Type::FLASH: apply(a.flash, subject, direct, offset); break;
        case Action::Type::HEAL: apply(a.heal, subject, direct, offset); break;
        case Action::Type::HOLD: apply(a.hold, subject, direct, offset); break;
        case Action::Type::KEY: apply(a.key, subject, direct, offset); break;
        case Action::Type::LAND: apply(a.land, subject, direct, offset); break;
        case Action::Type::MESSAGE: apply(a.message, subject, direct, offset); break;
        case Action::Type::MORPH: apply(a.morph, subject, direct, offset); break;
        case Action::Type::MOVE: apply(a.move, subject, direct, offset); break;
        case Action::Type::OCCUPY: apply(a.occupy, subject, direct, offset); break;
        case Action::Type::PAY: apply(a.pay, subject, direct, offset); break;
        case Action::Type::PLAY: apply(a.play, subject, direct, offset); break;
        case Action::Type::PUSH: apply(a.push, subject, direct, offset); break;
        case Action::Type::REMOVE: apply(a.remove, subject, direct, offset); break;
        case Action::Type::REVEAL: apply(a.reveal, subject, direct, offset); break;
        case Action::Type::SCORE: apply(a.score, subject, direct, offset); break;
        case Action::Type::SELECT: apply(a.select, subject, direct, offset); break;
        case Action::Type::SLOW: apply(a.slow, subject, direct, offset); break;
        case Action::Type::SPEED: apply(a.speed, subject, direct, offset); break;
        case Action::Type::STOP: apply(a.stop, subject, direct, offset); break;
        case Action::Type::SPARK: apply(a.spark, subject, direct, offset); break;
        case Action::Type::SPIN: apply(a.spin, subject, direct, offset); break;
        case Action::Type::TARGET: apply(a.target, subject, direct, offset); break;
        case Action::Type::THRUST: apply(a.thrust, subject, direct, offset); break;
        case Action::Type::WARP: apply(a.warp, subject, direct, offset); break;
        case Action::Type::WIN: apply(a.win, subject, direct, offset); break;
        case Action::Type::ZOOM: apply(a.zoom, subject, direct, offset); break;
    }

    return next;
}

static void execute_actions(ActionCursor cursor) {
    while (true) {
        while (cursor.begin != cursor.end) {
            const Action& action = *(cursor.begin++);

            auto subject = cursor.subject;
            auto direct  = cursor.direct;
            if (action.base.override_.subject.has_value()) {
                subject = resolve_object_ref(*action.base.override_.subject);
            }
            if (action.base.override_.direct.has_value()) {
                direct = resolve_object_ref(*action.base.override_.direct);
            }

            if (!direct.get()) {
                direct = subject;
            }

            auto owner_filter = action.base.filter.owner.value_or(Owner::ANY);
            if (direct.get() && subject.get()) {
                if (((owner_filter == Owner::DIFFERENT) && (direct->owner == subject->owner)) ||
                    ((owner_filter == Owner::SAME) && (direct->owner != subject->owner))) {
                    continue;
                }
            }

            if ((action.base.filter.attributes.bits || !action.base.filter.tags.tags.empty()) &&
                (!direct.get() || !action_filter_applies_to(action, direct))) {
                continue;
            }

            if (action.base.reflexive.value_or(false)) {
                std::swap(subject, direct);
            }

            cursor = apply(action, subject, direct, cursor.offset, std::move(cursor));
        }

        if (cursor.continuation) {
            cursor = std::move(*cursor.continuation);
        } else {
            break;
        }
    }
}

void exec(
        const std::vector<Action>& actions, Handle<SpaceObject> subject,
        Handle<SpaceObject> direct, Point offset) {
    execute_actions(ActionCursor(actions, subject, direct, offset));
}

void reset_action_queue() {
    gActionQueueData.reset(new actionQueueType[kActionQueueLength]);

    gFirstActionQueue = NULL;

    actionQueueType* action = gActionQueueData.get();
    for (int32_t i = 0; i < kActionQueueLength; i++) {
        action->cursor.begin = action->cursor.end = nullptr;
        ++action;
    }
}

static void queue_action(ActionCursor cursor, ticks delayTime) {
    int32_t          queueNumber = 0;
    actionQueueType* actionQueue = gActionQueueData.get();
    while (!actionQueue->empty() && (queueNumber < kActionQueueLength)) {
        actionQueue++;
        queueNumber++;
    }

    if (queueNumber == kActionQueueLength) {
        return;
    }
    actionQueue->cursor        = std::move(cursor);
    actionQueue->scheduledTime = delayTime;

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
        if (gFirstActionQueue->cursor.subject.get() && gFirstActionQueue->cursor.subject->active) {
            subjectid = gFirstActionQueue->cursor.subject->id;
        }

        int32_t directid = -1;
        if (gFirstActionQueue->cursor.direct.get() && gFirstActionQueue->cursor.direct->active) {
            directid = gFirstActionQueue->cursor.direct->id;
        }
        if ((subjectid == gFirstActionQueue->cursor.subject_id) &&
            (directid == gFirstActionQueue->cursor.direct_id)) {
            execute_actions(std::move(gFirstActionQueue->cursor));
        }

        gFirstActionQueue->cursor.begin = gFirstActionQueue->cursor.end = nullptr;
        gFirstActionQueue = gFirstActionQueue->nextActionQueue;
    }
}

}  // namespace antares

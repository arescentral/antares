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

static void execute_actions(
        const Action* begin, const Action* end, const Handle<SpaceObject> original_subject,
        const Handle<SpaceObject> original_direct, Point* offset);
static void queue_action(
        const Action* begin, const Action* end, ticks delayTime, Handle<SpaceObject> subject,
        Handle<SpaceObject> direct, Point* offset);

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
        const CreateAction& a, Handle<SpaceObject> subject, Handle<SpaceObject> focus,
        Handle<SpaceObject> direct, Point* offset) {
    auto count = a.count.value_or(Range<int64_t>{1, 2});
    auto c     = count.begin;
    if (count.range() > 1) {
        c += subject->randomSeed.next(count.range());
    } else if (a.legacy_random.value_or(false)) {
        // It used to be that the range test above was >0 instead of >1. That worked for most
        // objects, which had ranges of 0. However, the Nastiroid shooter on Mothership Connection
        // specified a range of 1. This was meaningless as far as the actual object count went, but
        // caused a random number to be consumed unnecessarily. To preserve replay-compatibility,
        // it now specifies `legacy_random`.
        subject->randomSeed.next(1);
    }
    for (int i = 0; i < c; ++i) {
        fixedPointType vel = {Fixed::zero(), Fixed::zero()};
        if (a.relative_velocity.value_or(false)) {
            vel = subject->velocity;
        }
        int32_t direction = 0;
        if (a.base->attributes & kAutoTarget) {
            direction = focus->targetAngle;
        } else if (a.relative_direction.value_or(false)) {
            direction = subject->direction;
        }
        coordPointType at = subject->location;
        if (offset != NULL) {
            at.h += offset->h;
            at.v += offset->v;
        }

        if (a.distance.has_value()) {
            Point p = random_point(&subject->randomSeed, *a.distance, a.within);
            at.h += p.h;
            at.v += p.v;
        }

        auto product = CreateAnySpaceObject(
                *a.base, &vel, &at, direction, subject->owner, 0, sfz::nullopt);
        if (!product.get()) {
            continue;
        }

        if (product->attributes & kCanAcceptDestination) {
            uint32_t save_attributes = product->attributes;
            product->attributes &= ~kStaticDestination;
            if (product->owner.get()) {
                if (a.inherit.value_or(false)) {
                    if (subject->destObject.get()) {
                        OverrideObjectDestination(product, subject->destObject);
                    }
                } else if (a.reflexive.value_or(false)) {
                    OverrideObjectDestination(product, subject);
                }
            } else if (a.reflexive.value_or(false)) {
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
            if (product->frame.vector->is_ray) {
                // special beams need special post-creation acts
                Vectors::set_attributes(product, subject);
            }
        }
    }
}

static void apply(
        const PlayAction& a, Handle<SpaceObject> subject, Handle<SpaceObject> focus,
        Handle<SpaceObject> direct, Point* offset) {
    pn::string_view pick;
    if (a.sound.has_value()) {
        pick = *a.sound;
    } else if (a.any.size() > 1) {
        pick = a.any[focus->randomSeed.next(a.any.size())].sound;
    } else {
        return;
    }
    if (a.absolute.value_or(false)) {
        sys.sound.play(pick, a.volume, a.persistence, a.priority.level);
    } else {
        sys.sound.play_at(pick, a.volume, a.persistence, a.priority.level, focus);
    }
}

static void apply(
        const SparkAction& a, Handle<SpaceObject> subject, Handle<SpaceObject> focus,
        Handle<SpaceObject> direct, Point* offset) {
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
    int32_t decay = round(1023 / a.age.count());
    globals()->starfield.make_sparks(a.count, decay, a.velocity, a.hue, &location);
}

static void apply(
        const DestroyAction& a, Handle<SpaceObject> subject, Handle<SpaceObject> focus,
        Handle<SpaceObject> direct, Point* offset) {
    if (!focus.get()) {
        return;
    }
    // If the object is occupied by a human, eject a body since players can't die.
    if ((focus->attributes & (kIsPlayerShip | kRemoteOrHuman)) && focus->base->destroy.die) {
        focus->create_floating_player_body();
    }
    focus->destroy();
}

static void apply(
        const RemoveAction& a, Handle<SpaceObject> subject, Handle<SpaceObject> focus,
        Handle<SpaceObject> direct, Point* offset) {
    if (!focus.get()) {
        return;
    }
    // If the object is occupied by a human, eject a body since players can't die.
    if ((focus->attributes & (kIsPlayerShip | kRemoteOrHuman)) && focus->base->destroy.die) {
        focus->create_floating_player_body();
    }
    focus->active = kObjectToBeFreed;
}

static void apply(
        const HoldAction& a, Handle<SpaceObject> subject, Handle<SpaceObject> focus,
        Handle<SpaceObject> direct, Point* offset) {
    focus->targetObject   = SpaceObject::none();
    focus->targetObjectID = kNoShip;
    focus->lastTarget     = SpaceObject::none();
}

static void apply(
        const HealAction& a, Handle<SpaceObject> subject, Handle<SpaceObject> focus,
        Handle<SpaceObject> direct, Point* offset) {
    focus->alter_health(a.value);
}

static void apply(
        const EnergizeAction& a, Handle<SpaceObject> subject, Handle<SpaceObject> focus,
        Handle<SpaceObject> direct, Point* offset) {
    focus->alter_energy(a.value);
}

static void apply(
        const RevealAction& a, Handle<SpaceObject> subject, Handle<SpaceObject> focus,
        Handle<SpaceObject> direct, Point* offset) {
    for (auto i : a.initial) {
        UnhideInitialObject(i);
    }
}

static void apply(
        const CheckAction& a, Handle<SpaceObject> subject, Handle<SpaceObject> focus,
        Handle<SpaceObject> direct, Point* offset) {
    CheckLevelConditions();
}

static void apply(
        const CloakAction& a, Handle<SpaceObject> subject, Handle<SpaceObject> focus,
        Handle<SpaceObject> direct, Point* offset) {
    focus->set_cloak(true);
}

static void apply(
        const SpinAction& a, Handle<SpaceObject> subject, Handle<SpaceObject> focus,
        Handle<SpaceObject> direct, Point* offset) {
    if (focus->attributes & kCanTurn) {
        Fixed f  = focus->turn_rate() * (a.value.begin + focus->randomSeed.next(a.value.range()));
        Fixed f2 = focus->base->mass;
        if (f2 == Fixed::zero()) {
            f = kFixedNone;
        } else {
            f /= f2;
        }
        focus->turnVelocity = f;
    }
}

static void apply(
        const DisableAction& a, Handle<SpaceObject> subject, Handle<SpaceObject> focus,
        Handle<SpaceObject> direct, Point* offset) {
    Fixed begin = Fixed::from_long(a.duration.begin.count()) / 3;
    Fixed end   = Fixed::from_long(a.duration.end.count()) / 3;
    Fixed mass  = focus->base->mass;
    Fixed f     = begin + focus->randomSeed.next(end - begin);
    if (mass == Fixed::zero()) {
        f = kFixedNone;
    } else {
        f /= mass;
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

static void apply(
        const PushAction& a, Handle<SpaceObject> subject, Handle<SpaceObject> focus,
        Handle<SpaceObject> direct, Point* offset) {
    if (!subject.get()) {
        return;
    }

    switch (a.kind) {
        case PushAction::Kind::BOOST: {
            Fixed fx, fy;
            GetRotPoint(&fx, &fy, focus->direction);
            focus->velocity.h += a.value * fx;
            focus->velocity.v += a.value * fy;
            break;
        }

        case PushAction::Kind::CRUISE: {
            Fixed fx, fy;
            GetRotPoint(&fx, &fy, focus->direction);
            focus->velocity = {a.value * fx, a.value * fy};
            break;
        }

        case PushAction::Kind::SET: {
            Fixed fx, fy;
            GetRotPoint(&fx, &fy, subject->direction);
            focus->velocity = {a.value * fx, a.value * fy};
            break;
        }

        case PushAction::Kind::COLLIDE: {
            if ((focus->base->mass <= Fixed::zero()) || (focus->maxVelocity <= Fixed::zero())) {
                return;
            }

            // if colliding, then PUSH the focus like collision
            focus->velocity.h +=
                    ((subject->velocity.h - focus->velocity.h) / focus->base->mass.val()) << 6L;
            focus->velocity.v +=
                    ((subject->velocity.v - focus->velocity.v) / focus->base->mass.val()) << 6L;

            // make sure we're not going faster than our top speed
            cap_velocity(focus);
            break;
        }

        case PushAction::Kind::DECELERATE: {
            if ((focus->base->mass <= Fixed::zero()) || (focus->maxVelocity <= Fixed::zero())) {
                return;
            }

            // if decelerating, then STOP the focus like applying brakes
            focus->velocity.h += focus->velocity.h * -a.value;
            focus->velocity.v += focus->velocity.v * -a.value;

            // make sure we're not going faster than our top speed
            cap_velocity(focus);
            break;
        }
    }
}

static void apply(
        const CapSpeedAction& a, Handle<SpaceObject> subject, Handle<SpaceObject> focus,
        Handle<SpaceObject> direct, Point* offset) {
    if (a.value.has_value()) {
        focus->maxVelocity = *a.value;
    } else {
        focus->maxVelocity = focus->base->maxVelocity;
    }
}

static void apply(
        const ThrustAction& a, Handle<SpaceObject> subject, Handle<SpaceObject> focus,
        Handle<SpaceObject> direct, Point* offset) {
    Fixed f       = a.value.begin + focus->randomSeed.next(a.value.range());
    focus->thrust = f;
}

static void apply(
        const MorphAction& a, Handle<SpaceObject> subject, Handle<SpaceObject> focus,
        Handle<SpaceObject> direct, Point* offset) {
    if (focus.get()) {
        focus->change_base_type(*a.base, sfz::nullopt, a.keep_ammo.value_or(false));
    }
}

static void apply(
        const CaptureAction& a, Handle<SpaceObject> subject, Handle<SpaceObject> focus,
        Handle<SpaceObject> direct, Point* offset) {
    if (!focus.get()) {
        return;
    }
    if (a.player.has_value()) {
        focus->set_owner(*a.player, false);
    } else {
        // if it's relative AND reflexive, we take the direct
        // object's owner, since relative & reflexive would
        // do nothing.
        if (a.reflexive.value_or(false) && direct.get()) {
            focus->set_owner(direct->owner, true);
        } else {
            focus->set_owner(subject->owner, true);
        }
    }
}

static void apply(
        const ConditionAction& a, Handle<SpaceObject> subject, Handle<SpaceObject> focus,
        Handle<SpaceObject> direct, Point* offset) {
    for (auto c : a.enable) {
        g.condition_enabled[c.number()] = true;
    }
    for (auto c : a.disable) {
        g.condition_enabled[c.number()] = false;
    }
}

static void apply(
        const OccupyAction& a, Handle<SpaceObject> subject, Handle<SpaceObject> focus,
        Handle<SpaceObject> direct, Point* offset) {
    if (focus.get()) {
        focus->alter_occupation(subject->owner, a.value, true);
    }
}

static void apply(
        const PayAction& a, Handle<SpaceObject> subject, Handle<SpaceObject> focus,
        Handle<SpaceObject> direct, Point* offset) {
    Handle<Admiral> admiral;
    if (a.player.has_value()) {
        admiral = *a.player;
    } else {
        if (focus.get()) {
            admiral = focus->owner;
        }
    }
    if (admiral.get()) {
        admiral->pay_absolute(a.value);
    }
}

static void apply(
        const AgeAction& a, Handle<SpaceObject> subject, Handle<SpaceObject> focus,
        Handle<SpaceObject> direct, Point* offset) {
    ticks t = a.value.begin + focus->randomSeed.next(a.value.range());

    if (a.relative.value_or(false)) {
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

static void apply(
        const MoveAction& a, Handle<SpaceObject> subject, Handle<SpaceObject> focus,
        Handle<SpaceObject> direct, Point* offset) {
    coordPointType newLocation;
    switch (a.origin.value_or(MoveAction::Origin::LEVEL)) {
        case MoveAction::Origin::LEVEL: newLocation = {kUniversalCenter, kUniversalCenter}; break;
        case MoveAction::Origin::SUBJECT: newLocation = subject->location; break;
        case MoveAction::Origin::DIRECT: newLocation = direct->location; break;
    }

    coordPointType off = a.to.value_or(coordPointType{0, 0});
    off                = Translate_Coord_To_Level_Rotation(off.h, off.v);
    newLocation.h += off.h - kUniversalCenter;
    newLocation.v += off.v - kUniversalCenter;

    Point random = random_point(&focus->randomSeed, a.distance.value_or(0), a.within);
    newLocation.h += random.h;
    newLocation.v += random.v;

    focus->location.h = newLocation.h;
    focus->location.v = newLocation.v;
}

static void alter_weapon(
        const BaseObject* base, Handle<SpaceObject> focus, SpaceObject::Weapon& weapon) {
    weapon.base     = base;
    weapon.time     = g.time;
    weapon.position = 0;
    if (!base) {
        weapon.ammo = 0;
        return;
    }

    weapon.ammo = base->device->ammo;
    if (base->device->range > focus->longestWeaponRange) {
        focus->longestWeaponRange = base->device->range;
    }
    if (base->device->range < focus->shortestWeaponRange) {
        focus->shortestWeaponRange = base->device->range;
    }
}

static void apply(
        const EquipAction& a, Handle<SpaceObject> subject, Handle<SpaceObject> focus,
        Handle<SpaceObject> direct, Point* offset) {
    switch (a.which) {
        case Weapon::PULSE: alter_weapon(a.base.get(), focus, focus->pulse); break;
        case Weapon::BEAM: alter_weapon(a.base.get(), focus, focus->beam); break;
        case Weapon::SPECIAL: alter_weapon(a.base.get(), focus, focus->special); break;
    }
}

static void apply(
        const LandAction& a, Handle<SpaceObject> subject, Handle<SpaceObject> focus,
        Handle<SpaceObject> direct, Point* offset) {
    // even though this is never a reflexive verb, we only effect ourselves
    if (subject->attributes & (kIsPlayerShip | kRemoteOrHuman)) {
        subject->create_floating_player_body();
    }
    subject->presenceState          = kLandingPresence;
    subject->presence.landing.speed = a.speed;
    subject->presence.landing.scale = sprite_scale(*subject->base);
}

static void apply(
        const WarpAction& a, Handle<SpaceObject> subject, Handle<SpaceObject> focus,
        Handle<SpaceObject> direct, Point* offset) {
    subject->presenceState             = kWarpInPresence;
    subject->presence.warp_in.progress = ticks(0);
    subject->presence.warp_in.step     = 0;
    subject->attributes &= ~kOccupiesSpace;
    fixedPointType newVel = {Fixed::zero(), Fixed::zero()};
    CreateAnySpaceObject(
            *plug.info.warpInFlareID, &newVel, &subject->location, subject->direction,
            Admiral::none(), 0, sfz::nullopt);
}

static void apply(
        const ScoreAction& a, Handle<SpaceObject> subject, Handle<SpaceObject> focus,
        Handle<SpaceObject> direct, Point* offset) {
    Handle<Admiral> admiral;
    if (a.player.has_value()) {
        admiral = *a.player;
    } else if (focus.get()) {
        admiral = focus->owner;
    }
    if (admiral.get()) {
        AlterAdmiralScore(admiral, a.which, a.value);
    }
}

static void apply(
        const WinAction& a, Handle<SpaceObject> subject, Handle<SpaceObject> focus,
        Handle<SpaceObject> direct, Point* offset) {
    Handle<Admiral> admiral;
    if (a.player.has_value()) {
        admiral = *a.player;
    } else if (focus.get()) {
        admiral = focus->owner;
    }
    if (a.next.has_value()) {
        DeclareWinner(admiral, a.next->get(), a.text);
    } else {
        DeclareWinner(admiral, nullptr, a.text);
    }
}

static void apply(
        const MessageAction& a, Handle<SpaceObject> subject, Handle<SpaceObject> focus,
        Handle<SpaceObject> direct, Point* offset) {
    Messages::start(a.id, &a.pages);
}

static void apply(
        const OrderAction& a, Handle<SpaceObject> subject, Handle<SpaceObject> focus,
        Handle<SpaceObject> direct, Point* offset) {
    uint32_t save_attributes = subject->attributes;
    subject->attributes &= ~kStaticDestination;
    OverrideObjectDestination(subject, focus);
    subject->attributes = save_attributes;
}

static void apply(
        const FireAction& a, Handle<SpaceObject> subject, Handle<SpaceObject> focus,
        Handle<SpaceObject> direct, Point* offset) {
    switch (a.which) {
        case Weapon::PULSE:
            fire_weapon(
                    subject, SpaceObject::none(), subject->pulse,
                    subject->base->weapons.pulse.has_value()
                            ? subject->base->weapons.pulse->positions
                            : std::vector<fixedPointType>{});
            break;
        case Weapon::BEAM:
            fire_weapon(
                    subject, SpaceObject::none(), subject->beam,
                    subject->base->weapons.beam.has_value()
                            ? subject->base->weapons.beam->positions
                            : std::vector<fixedPointType>{});
            break;
        case Weapon::SPECIAL:
            fire_weapon(
                    subject, SpaceObject::none(), subject->special,
                    subject->base->weapons.special.has_value()
                            ? subject->base->weapons.special->positions
                            : std::vector<fixedPointType>{});
            break;
    }
}

static void apply(
        const FlashAction& a, Handle<SpaceObject> subject, Handle<SpaceObject> focus,
        Handle<SpaceObject> direct, Point* offset) {
    globals()->transitions.start_boolean(a.duration, a.color);
}

static void apply(
        const GroupAction& a, Handle<SpaceObject> subject, Handle<SpaceObject> focus,
        Handle<SpaceObject> direct, Point* offset) {
    execute_actions(a.of.data(), a.of.data() + a.of.size(), subject, focus, offset);
}

static void apply(
        const KeyAction& a, Handle<SpaceObject> subject, Handle<SpaceObject> focus,
        Handle<SpaceObject> direct, Point* offset) {
    for (KeyAction::Key key : a.enable) {
        g.key_mask = g.key_mask & ~(1 << static_cast<int32_t>(key));
    }
    for (KeyAction::Key key : a.disable) {
        g.key_mask = g.key_mask | (1 << static_cast<int32_t>(key));
    }
}

static void apply(
        const ZoomAction& a, Handle<SpaceObject> subject, Handle<SpaceObject> focus,
        Handle<SpaceObject> direct, Point* offset) {
    if (a.value != g.zoom) {
        g.zoom = a.value;
        sys.sound.click();
        Messages::zoom(g.zoom);
    }
}

static void apply(
        const SelectAction& a, Handle<SpaceObject> subject, Handle<SpaceObject> focus,
        Handle<SpaceObject> direct, Point* offset) {
    MiniComputer_SetScreenAndLineHack(a.screen, a.line);
}

static void apply(
        const AssumeAction& a, Handle<SpaceObject> subject, Handle<SpaceObject> focus,
        Handle<SpaceObject> direct, Point* offset) {
    Handle<Admiral> player1(0);
    int             index = a.which + GetAdmiralScore(player1, 0);
    g.initials[index]     = focus;
    g.initial_ids[index]  = focus->id;
}

static void apply(
        const Action& a, Handle<SpaceObject> subject, Handle<SpaceObject> focus,
        Handle<SpaceObject> direct, Point* offset) {
    switch (a.type()) {
        case Action::Type::AGE: apply(a.age, subject, focus, direct, offset); break;
        case Action::Type::ASSUME: apply(a.assume, subject, focus, direct, offset); break;
        case Action::Type::CAPTURE: apply(a.capture, subject, focus, direct, offset); break;
        case Action::Type::CAP_SPEED: apply(a.cap_speed, subject, focus, direct, offset); break;
        case Action::Type::CHECK: apply(a.check, subject, focus, direct, offset); break;
        case Action::Type::CLOAK: apply(a.cloak, subject, focus, direct, offset); break;
        case Action::Type::CONDITION: apply(a.condition, subject, focus, direct, offset); break;
        case Action::Type::CREATE: apply(a.create, subject, focus, direct, offset); break;
        case Action::Type::DELAY: throw std::runtime_error("delay shouldn’t get here");
        case Action::Type::DESTROY: apply(a.destroy, subject, focus, direct, offset); break;
        case Action::Type::DISABLE: apply(a.disable, subject, focus, direct, offset); break;
        case Action::Type::ENERGIZE: apply(a.energize, subject, focus, direct, offset); break;
        case Action::Type::EQUIP: apply(a.equip, subject, focus, direct, offset); break;
        case Action::Type::FIRE: apply(a.fire, subject, focus, direct, offset); break;
        case Action::Type::FLASH: apply(a.flash, subject, focus, direct, offset); break;
        case Action::Type::GROUP: apply(a.group, subject, focus, direct, offset); break;
        case Action::Type::HEAL: apply(a.heal, subject, focus, direct, offset); break;
        case Action::Type::HOLD: apply(a.hold, subject, focus, direct, offset); break;
        case Action::Type::KEY: apply(a.key, subject, focus, direct, offset); break;
        case Action::Type::LAND: apply(a.land, subject, focus, direct, offset); break;
        case Action::Type::MESSAGE: apply(a.message, subject, focus, direct, offset); break;
        case Action::Type::MORPH: apply(a.morph, subject, focus, direct, offset); break;
        case Action::Type::MOVE: apply(a.move, subject, focus, direct, offset); break;
        case Action::Type::OCCUPY: apply(a.occupy, subject, focus, direct, offset); break;
        case Action::Type::ORDER: apply(a.order, subject, focus, direct, offset); break;
        case Action::Type::PAY: apply(a.pay, subject, focus, direct, offset); break;
        case Action::Type::PLAY: apply(a.play, subject, focus, direct, offset); break;
        case Action::Type::PUSH: apply(a.push, subject, focus, direct, offset); break;
        case Action::Type::REMOVE: apply(a.remove, subject, focus, direct, offset); break;
        case Action::Type::REVEAL: apply(a.reveal, subject, focus, direct, offset); break;
        case Action::Type::SCORE: apply(a.score, subject, focus, direct, offset); break;
        case Action::Type::SELECT: apply(a.select, subject, focus, direct, offset); break;
        case Action::Type::SPARK: apply(a.spark, subject, focus, direct, offset); break;
        case Action::Type::SPIN: apply(a.spin, subject, focus, direct, offset); break;
        case Action::Type::THRUST: apply(a.thrust, subject, focus, direct, offset); break;
        case Action::Type::WARP: apply(a.warp, subject, focus, direct, offset); break;
        case Action::Type::WIN: apply(a.win, subject, focus, direct, offset); break;
        case Action::Type::ZOOM: apply(a.zoom, subject, focus, direct, offset); break;
    }
}

static void execute_actions(
        const Action* begin, const Action* end, const Handle<SpaceObject> original_subject,
        const Handle<SpaceObject> original_direct, Point* offset) {
    for (const Action* curr : sfz::range(begin, end)) {
        const Action& action = *curr;
#ifdef DATA_COVERAGE
        covered_actions.insert(action.number());
#endif  // DATA_COVERAGE

        auto subject = original_subject;
        if (action.base.override_.subject.has_value()) {
            subject = resolve_object_ref(*action.base.override_.subject);
        }
        auto direct = original_direct;
        if (action.base.override_.direct.has_value()) {
            direct = resolve_object_ref(*action.base.override_.direct);
        }

        auto focus = direct;
        if (action.base.reflexive.value_or(false) || !focus.get()) {
            focus = subject;
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

        if (action.type() == ActionType::DELAY) {
            queue_action(
                    curr + 1, end, action.delay.duration, original_subject, original_direct,
                    offset);
            return;
        }

        apply(action, subject, focus, direct, offset);
    }
}

void exec(
        const std::vector<Action>& actions, Handle<SpaceObject> subject,
        Handle<SpaceObject> direct, Point* offset) {
    execute_actions(actions.data(), actions.data() + actions.size(), subject, direct, offset);
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
        const Action* begin, const Action* end, ticks delayTime, Handle<SpaceObject> subject,
        Handle<SpaceObject> direct, Point* offset) {
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

    actionQueue->subjectObject = subject;
    if (subject.get()) {
        actionQueue->subjectObjectNum = subject->number();
        actionQueue->subjectObjectID  = subject->id;
    } else {
        actionQueue->subjectObjectNum = -1;
        actionQueue->subjectObjectID  = -1;
    }

    actionQueue->directObject = direct;
    if (direct.get()) {
        actionQueue->directObjectNum = direct->number();
        actionQueue->directObjectID  = direct->id;
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
                    &gFirstActionQueue->offset);
        }

        gFirstActionQueue->actionBegin = gFirstActionQueue->actionEnd = nullptr;
        gFirstActionQueue = gFirstActionQueue->nextActionQueue;
    }
}

}  // namespace antares

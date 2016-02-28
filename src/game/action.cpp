// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// Copyright (C) 2008-2012 The Antares Authors
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

#include "data/resource.hpp"
#include "data/space-object.hpp"
#include "data/string-list.hpp"
#include "drawing/color.hpp"
#include "drawing/sprite-handling.hpp"
#include "game/admiral.hpp"
#include "game/beam.hpp"
#include "game/globals.hpp"
#include "game/labels.hpp"
#include "game/messages.hpp"
#include "game/minicomputer.hpp"
#include "game/motion.hpp"
#include "game/non-player-ship.hpp"
#include "game/player-ship.hpp"
#include "game/scenario-maker.hpp"
#include "game/space-object.hpp"
#include "game/starfield.hpp"
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
using sfz::range;
using sfz::read;
using std::set;
using std::unique_ptr;

namespace antares {

const size_t kActionQueueLength     = 120;

struct actionQueueType {
    HandleList<Action>              actionRef;
    int32_t                         scheduledTime;
    actionQueueType*                nextActionQueue;
    int32_t                         nextActionQueueNum;
    Handle<SpaceObject>             subjectObject;
    int32_t                         subjectObjectNum;
    int32_t                         subjectObjectID;
    Handle<SpaceObject>             directObject;
    int32_t                         directObjectNum;
    int32_t                         directObjectID;
    Point                           offset;
};

static ANTARES_GLOBAL actionQueueType* gFirstActionQueue = NULL;
static ANTARES_GLOBAL int32_t gFirstActionQueueNumber = -1;

static ANTARES_GLOBAL unique_ptr<actionQueueType[]> gActionQueueData;

#ifdef DATA_COVERAGE
ANTARES_GLOBAL set<int32_t> covered_actions;
#endif  // DATA_COVERAGE

static void queue_action(
        HandleList<Action> actions, int32_t delayTime,
        Handle<SpaceObject> subjectObject, Handle<SpaceObject> directObject, Point* offset);

bool action_filter_applies_to(const Action& action, Handle<BaseObject> target) {
    if (action.exclusiveFilter == 0xffffffff) {
        return action.levelKeyTag == target->levelKeyTag;
    } else {
        return (action.inclusiveFilter & target->attributes) == action.inclusiveFilter;
    }
}

bool action_filter_applies_to(const Action& action, Handle<SpaceObject> target) {
    if (action.exclusiveFilter == 0xffffffff) {
        return action.levelKeyTag == target->baseType->levelKeyTag;
    } else {
        return (action.inclusiveFilter & target->attributes) == action.inclusiveFilter;
    }
}

static void create_object(
        Handle<Action> action, Handle<SpaceObject> subject, Handle<SpaceObject> focus, Point* offset) {
    const auto& create = action->argument.createObject;
    const auto baseObject = create.whichBaseType;
    auto count = create.howManyMinimum;
    if (create.howManyRange > 0) {
        count += focus->randomSeed.next(create.howManyRange);
    }
    for (int i = 0; i < count; ++i) {
        fixedPointType vel = {0, 0};
        if (create.velocityRelative) {
            vel = focus->velocity;
        }
        int32_t direction = 0;
        if (baseObject->attributes & kAutoTarget) {
            direction = subject->targetAngle;
        } else if (create.directionRelative) {
            direction = focus->direction;
        }
        coordPointType at = focus->location;
        if (offset != NULL) {
            at.h += offset->h;
            at.v += offset->v;
        }

        const int32_t distance = create.randomDistance;
        if (distance > 0) {
            at.h += focus->randomSeed.next(distance * 2) - distance;
            at.v += focus->randomSeed.next(distance * 2) - distance;
        }

        auto product = CreateAnySpaceObject(
                baseObject, &vel, &at, direction, focus->owner, 0, -1);
        if (!product.get()) {
            continue;
        }

        if (product->attributes & kCanAcceptDestination) {
            uint32_t save_attributes = product->attributes;
            product->attributes &= ~kStaticDestination;
            if (product->owner.get()) {
                if (action->reflexive) {
                    if (action->verb != kCreateObjectSetDest) {
                        SetObjectDestination(product, focus);
                    } else if (focus->destObject.get()) {
                        SetObjectDestination(product, focus->destObject);
                    }
                }
            } else if (action->reflexive) {
                product->timeFromOrigin = kTimeToCheckHome;
                product->runTimeFlags &= ~kHasArrived;
                product->destObject = focus; //a->destinationObject;
                product->destObjectDest = focus->destObject;
                product->destObjectID = focus->id;
                product->destObjectDestID = focus->destObjectID;
            }
            product->attributes = save_attributes;
        }
        product->targetObject = focus->targetObject;
        product->targetObjectID = focus->targetObjectID;
        product->closestObject = product->targetObject;

        //  ugly though it is, we have to fill in the rest of
        //  a new beam's fields after it's created.
        if (product->attributes & kIsBeam) {
            if (product->frame.beam->beamKind != eKineticBeamKind) {
                // special beams need special post-creation acts
                Beams::set_attributes(product, focus);
            }
        }
    }
}

static void play_sound(Handle<Action> action, Handle<SpaceObject> focus) {
    const auto& sound = action->argument.playSound;
    auto id = sound.idMinimum;
    auto priority = static_cast<soundPriorityType>(sound.priority);
    if (sound.idRange > 0) {
        id += focus->randomSeed.next(sound.idRange + 1);
    }
    if (sound.absolute) {
        PlayVolumeSound(id, sound.volumeMinimum, sound.persistence, priority);
    } else {
        mPlayDistanceSound(sound.volumeMinimum, focus, id, sound.persistence, priority);
    }
}

static void make_sparks(Handle<Action> action, Handle<SpaceObject> focus) {
    const auto& sparks = action->argument.makeSparks;
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
        l >>= SHIFT_SCALE; /*+ CLIP_TOP*/;
        if ((l > -kSpriteMaxSize) && (l < kSpriteMaxSize)) {
            location.v = l + viewport().top;
        } else {
            location.v = -kSpriteMaxSize;
        }
    }
    globals()->starfield.make_sparks(
            sparks.howMany, sparks.speed, sparks.velocityRange, sparks.color, &location);
}

static void die(Handle<Action> action, Handle<SpaceObject> focus, Handle<SpaceObject> subject) {
    bool destroy = false;
    switch (action->argument.killObject.dieType) {
        case kDieExpire:
            if (subject.get()) {
                focus = subject;
            } else {
                return;
            }
            break;

        case kDieDestroy:
            if (subject.get()) {
                focus = subject;
                destroy = true;
            } else {
                return;;
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

static void nil_target(Handle<Action> action, Handle<SpaceObject> focus) {
    focus->targetObject = SpaceObject::none();
    focus->targetObjectID = kNoShip;
    focus->lastTarget = SpaceObject::none();
}

static void alter(
        Handle<Action> action,
        Handle<SpaceObject> focus, Handle<SpaceObject> subject, Handle<SpaceObject> object) {
    const auto alter = action->argument.alterObject;
    int32_t l;
    Fixed f, f2;
    int16_t angle;
    coordPointType newLocation;
    switch (alter.alterType) {
        case kAlterDamage:
            focus->alter_health(alter.minimum);
            break;

        case kAlterEnergy:
            focus->alter_energy(alter.minimum);
            break;

        case kAlterHidden:
            // Preserves old behavior; shouldn't really be adding one to alter.range.
            for (auto i: range(alter.minimum, alter.minimum + alter.range + 1)) {
                UnhideInitialObject(i);
            }
            break;

        case kAlterCloak:
            focus->set_cloak(true);
            break;

        case kAlterSpin:
            if (focus->attributes & kCanTurn) {
                if (focus->attributes & kShapeFromDirection) {
                    f = mMultiplyFixed(
                            focus->baseType->frame.rotation.maxTurnRate,
                            alter.minimum + focus->randomSeed.next(alter.range));
                } else {
                    f = mMultiplyFixed(
                            2 /*kDefaultTurnRate*/,
                            alter.minimum + focus->randomSeed.next(alter.range));
                }
                f2 = focus->baseType->mass;
                if (f2 == 0) {
                    f = -1;
                } else {
                    f = mDivideFixed(f, f2);
                }
                focus->turnVelocity = f;
            }
            break;

        case kAlterOffline:
            f = alter.minimum + focus->randomSeed.next(alter.range);
            f2 = focus->baseType->mass;
            if (f2 == 0) {
                focus->offlineTime = -1;
            } else {
                focus->offlineTime = mDivideFixed(f, f2);
            }
            focus->offlineTime = mFixedToLong(focus->offlineTime);
            break;

        case kAlterVelocity:
            if (subject.get()) {
                // active (non-reflexive) altering of velocity means a PUSH, just like
                //  two objects colliding.  Negative velocity = slow down
                if (object.get()) {
                    if (alter.relative) {
                        if ((object->baseType->mass > 0) &&
                            (object->maxVelocity > 0)) {
                            if (alter.minimum >= 0) {
                                // if the minimum >= 0, then PUSH the object like collision
                                f = subject->velocity.h - object->velocity.h;
                                f /= object->baseType->mass;
                                f <<= 6L;
                                object->velocity.h += f;
                                f = subject->velocity.v - object->velocity.v;
                                f /= object->baseType->mass;
                                f <<= 6L;
                                object->velocity.v += f;

                                // make sure we're not going faster than our top speed
                                angle = ratio_to_angle(object->velocity.h, object->velocity.v);
                            } else {
                                // if the minumum < 0, then STOP the object like applying breaks
                                f = object->velocity.h;
                                f = mMultiplyFixed(f, alter.minimum);
                                object->velocity.h += f;
                                f = object->velocity.v;
                                f = mMultiplyFixed(f, alter.minimum);
                                object->velocity.v += f;

                                // make sure we're not going faster than our top speed
                                angle = ratio_to_angle(object->velocity.h, object->velocity.v);
                            }

                            // get the maxthrust of new vector
                            GetRotPoint(&f, &f2, angle);
                            f = mMultiplyFixed(object->maxVelocity, f);
                            f2 = mMultiplyFixed(object->maxVelocity, f2);

                            if (f < 0) {
                                if (object->velocity.h < f) {
                                    object->velocity.h = f;
                                }
                            } else {
                                if (object->velocity.h > f) {
                                    object->velocity.h = f;
                                }
                            }

                            if (f2 < 0) {
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
                        f = mMultiplyFixed(alter.minimum, f);
                        f2 = mMultiplyFixed(alter.minimum, f2);
                        focus->velocity.h = f;
                        focus->velocity.v = f2;
                    }
                } else {
                    // reflexive alter velocity means a burst of speed in the direction
                    // the object is facing, where negative speed means backwards. Object can
                    // excede its max velocity.
                    // Minimum value is absolute speed in direction.
                    GetRotPoint(&f, &f2, focus->direction);
                    f = mMultiplyFixed(alter.minimum, f);
                    f2 = mMultiplyFixed(alter.minimum, f2);
                    if (alter.relative) {
                        focus->velocity.h += f;
                        focus->velocity.v += f2;
                    } else {
                        focus->velocity.h = f;
                        focus->velocity.v = f2;
                    }
                }
            }
            break;

        case kAlterMaxVelocity:
            if (alter.minimum < 0) {
                focus->maxVelocity = focus->baseType->maxVelocity;
            } else {
                focus->maxVelocity = alter.minimum;
            }
            break;

        case kAlterThrust:
            f = alter.minimum + focus->randomSeed.next(alter.range);
            if (alter.relative) {
                focus->thrust += f;
            } else {
                focus->thrust = f;
            }
            break;

        case kAlterBaseType:
            if (action->reflexive || object.get()) {
                focus->change_base_type(Handle<BaseObject>(alter.minimum), -1, alter.relative);
            }
            break;

        case kAlterOwner:
            if (focus.get()) {
                if (alter.relative) {
                    // if it's relative AND reflexive, we take the direct
                    // object's owner, since relative & reflexive would
                    // do nothing.
                    if (action->reflexive && focus.get() && object.get()) {
                        focus->set_owner(object->owner, true);
                    } else {
                        focus->set_owner(subject->owner, true);
                    }
                } else {
                    focus->set_owner(Handle<Admiral>(alter.minimum), false);
                }
            }
            break;

        case kAlterConditionTrueYet:
            if (alter.range <= 0) {
                g.level->condition(alter.minimum)->set_true_yet(alter.relative);
            } else {
                for (auto l: range(alter.minimum, alter.minimum + alter.range + 1)) {
                    g.level->condition(l)->set_true_yet(alter.relative);
                }
            }
            break;

        case kAlterOccupation:
            if (focus.get()) {
                focus->alter_occupation(subject->owner, alter.minimum, true);
            }
            break;

        case kAlterAbsoluteCash:
            {
                Handle<Admiral> admiral;
                if (alter.relative) {
                    if (focus.get()) {
                        admiral = focus->owner;
                    }
                } else {
                    admiral = Handle<Admiral>(alter.range);
                }
                if (admiral.get()) {
                    admiral->pay_absolute(alter.minimum);
                }
            }
            break;

        case kAlterAge:
            l = alter.minimum + focus->randomSeed.next(alter.range);

            if (alter.relative) {
                if (focus->age >= 0) {
                    focus->age += l;

                    if (focus->age < 0) {
                        focus->age = 0;
                    }
                } else {
                    focus->age += l;
                }
            } else {
                focus->age = l;
            }
            break;

        case kAlterLocation:
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
            newLocation.h += focus->randomSeed.next(alter.minimum << 1) - alter.minimum;
            newLocation.v += focus->randomSeed.next(alter.minimum << 1) - alter.minimum;
            focus->location.h = newLocation.h;
            focus->location.v = newLocation.v;
            break;

        case kAlterAbsoluteLocation:
            if (alter.relative) {
                focus->location.h += alter.minimum;
                focus->location.v += alter.range;
            } else {
                focus->location = Translate_Coord_To_Scenario_Rotation(
                    alter.minimum, alter.range);
            }
            break;

        case kAlterWeapon1:
            focus->pulse.base = Handle<BaseObject>(alter.minimum);
            if (focus->pulse.base.get()) {
                auto baseObject = focus->pulse.base;
                focus->pulse.ammo = baseObject->frame.weapon.ammo;
                focus->pulse.time = focus->pulse.position = 0;
                if (baseObject->frame.weapon.range > focus->longestWeaponRange) {
                    focus->longestWeaponRange = baseObject->frame.weapon.range;
                }
                if (baseObject->frame.weapon.range < focus->shortestWeaponRange) {
                    focus->shortestWeaponRange = baseObject->frame.weapon.range;
                }
            } else {
                focus->pulse.base = BaseObject::none();
                focus->pulse.ammo = 0;
                focus->pulse.time = 0;
            }
            break;

        case kAlterWeapon2:
            focus->beam.base = Handle<BaseObject>(alter.minimum);
            if (focus->beam.base.get()) {
                auto baseObject = focus->beam.base;
                focus->beam.ammo = baseObject->frame.weapon.ammo;
                focus->beam.time = focus->beam.position = 0;
                if (baseObject->frame.weapon.range > focus->longestWeaponRange) {
                    focus->longestWeaponRange = baseObject->frame.weapon.range;
                }
                if (baseObject->frame.weapon.range < focus->shortestWeaponRange) {
                    focus->shortestWeaponRange = baseObject->frame.weapon.range;
                }
            } else {
                focus->beam.base = BaseObject::none();
                focus->beam.ammo = 0;
                focus->beam.time = 0;
            }
            break;

        case kAlterSpecial:
            focus->special.base = Handle<BaseObject>(alter.minimum);
            if (focus->special.base.get()) {
                auto baseObject = focus->special.base;
                focus->special.ammo = baseObject->frame.weapon.ammo;
                focus->special.time = focus->special.position = 0;
                if (baseObject->frame.weapon.range > focus->longestWeaponRange) {
                    focus->longestWeaponRange = baseObject->frame.weapon.range;
                }
                if (baseObject->frame.weapon.range < focus->shortestWeaponRange) {
                    focus->shortestWeaponRange = baseObject->frame.weapon.range;
                }
            } else {
                focus->special.base = BaseObject::none();
                focus->special.ammo = 0;
                focus->special.time = 0;
            }
            break;

        case kAlterLevelKeyTag:
            break;

        default:
            break;
    }
}

static void land_at(Handle<Action> action, Handle<SpaceObject> focus, Handle<SpaceObject> subject) {
    // even though this is never a reflexive verb, we only effect ourselves
    if (subject->attributes & (kIsPlayerShip | kRemoteOrHuman)) {
        subject->create_floating_player_body();
    }
    subject->presenceState = kLandingPresence;
    subject->presence.landing.speed = action->argument.landAt.landingSpeed;
    subject->presence.landing.scale = subject->baseType->naturalScale;
}

static void enter_warp(Handle<Action> action, Handle<SpaceObject> focus, Handle<SpaceObject> subject) {
    subject->presenceState = kWarpInPresence;
    subject->presence.warp_in.progress = 0;
    subject->presence.warp_in.step = 0;
    subject->attributes &= ~kOccupiesSpace;
    fixedPointType newVel = {0, 0};
    CreateAnySpaceObject(
            plug.meta.warpInFlareID, &newVel,
            &subject->location, subject->direction, Admiral::none(), 0, -1);
}

static void change_score(Handle<Action> action, Handle<SpaceObject> focus) {
    const auto& score = action->argument.changeScore;
    Handle<Admiral> admiral = score.whichPlayer;
    if ((!score.whichPlayer.get() && focus.get())) {
        admiral = focus->owner;
    }
    if (admiral.get()) {
        AlterAdmiralScore(admiral, score.whichScore, score.amount);
    }
}

static void declare_winner(Handle<Action> action, Handle<SpaceObject> focus) {
    const auto& winner = action->argument.declareWinner;
    Handle<Admiral> admiral = winner.whichPlayer;
    if ((!winner.whichPlayer.get() && focus.get())) {
        admiral = focus->owner;
    }
    DeclareWinner(admiral, winner.nextLevel, winner.textID);
}

static void display_message(Handle<Action> action, Handle<SpaceObject> focus) {
    const auto& message = action->argument.displayMessage;
    Messages::start(message.resID, message.resID + message.pageNum - 1);
}

static void set_destination(
        Handle<Action> action, Handle<SpaceObject> focus, Handle<SpaceObject> subject) {
    uint32_t save_attributes = subject->attributes;
    subject->attributes &= ~kStaticDestination;
    SetObjectDestination(subject, focus);
    subject->attributes = save_attributes;
}

static void activate_special(
        Handle<Action> action, Handle<SpaceObject> focus, Handle<SpaceObject> subject) {
    fire_weapon(subject, SpaceObject::none(), subject->baseType->special, subject->special);
}

static void activate_pulse(
        Handle<Action> action, Handle<SpaceObject> focus, Handle<SpaceObject> subject) {
    fire_weapon(subject, SpaceObject::none(), subject->baseType->pulse, subject->pulse);
}

static void activate_beam(
        Handle<Action> action, Handle<SpaceObject> focus, Handle<SpaceObject> subject) {
    fire_weapon(subject, SpaceObject::none(), subject->baseType->beam, subject->beam);
}

static void color_flash(Handle<Action> action, Handle<SpaceObject> focus) {
    uint8_t tinyColor = GetTranslateColorShade(
            action->argument.colorFlash.color,
            action->argument.colorFlash.shade);
    globals()->transitions.start_boolean(
            action->argument.colorFlash.length,
            action->argument.colorFlash.length, tinyColor);
}

static void enable_keys(Handle<Action> action, Handle<SpaceObject> focus) {
    globals()->keyMask = globals()->keyMask & ~action->argument.keys.keyMask;
}

static void disable_keys(Handle<Action> action, Handle<SpaceObject> focus) {
    globals()->keyMask = globals()->keyMask | action->argument.keys.keyMask;
}

static void set_zoom(Handle<Action> action, Handle<SpaceObject> focus) {
    if (action->argument.zoom.zoomLevel != globals()->gZoomMode) {
        globals()->gZoomMode = static_cast<ZoomType>(action->argument.zoom.zoomLevel);
        PlayVolumeSound(kComputerBeep3, kMediumVolume, kMediumPersistence, kLowPrioritySound);
        StringList strings(kMessageStringID);
        StringSlice string = strings.at(globals()->gZoomMode + kZoomStringOffset - 1);
        Messages::set_status(string, kStatusLabelColor);
    }
}

static void computer_select(Handle<Action> action, Handle<SpaceObject> focus) {
    MiniComputer_SetScreenAndLineHack(
            action->argument.computerSelect.screenNumber,
            action->argument.computerSelect.lineNumber);
}

static void assume_initial_object(Handle<Action> action, Handle<SpaceObject> focus) {
    Handle<Admiral> player1(0);
    Scenario::InitialObject* initialObject = g.level->initial(
            action->argument.assumeInitial.whichInitialObject + GetAdmiralScore(player1, 0));
    if (initialObject) {
        initialObject->realObjectID = focus->id;
        initialObject->realObject = focus;
    }
}

static void execute_actions(
        const HandleList<Action>& actions,
        const Handle<SpaceObject> original_subject, const Handle<SpaceObject> original_object,
        Point* offset, bool allowDelay) {
    bool checkConditions = false;

    for (auto action: actions) {
#ifdef DATA_COVERAGE
        covered_actions.insert(action.number());
#endif  // DATA_COVERAGE

        if (action->verb == kNoAction) {
            break;
        }
        auto subject = original_subject;
        if (action->initialSubjectOverride != kNoShip) {
            subject = GetObjectFromInitialNumber(action->initialSubjectOverride);
        }
        auto object = original_object;
        if (action->initialDirectOverride != kNoShip) {
            object = GetObjectFromInitialNumber(action->initialDirectOverride);
        }

        if ((action->delay > 0) && allowDelay) {
            queue_action(
                    {action.number(), (*actions.end()).number()},
                    action->delay, subject, object, offset);
            return;
        }
        allowDelay = true;

        auto focus = object;
        if (action->reflexive || !focus.get()) {
            focus = subject;
        }

        if (object.get() && subject.get()) {
            if ((action->owner < -1)
                    || ((action->owner == -1) && (object->owner == subject->owner))
                    || ((action->owner == 1) && (object->owner != subject->owner))
                    || (action->owner > 1)) {
                continue;
            }
        }

        if ((action->inclusiveFilter || action->exclusiveFilter)
                && (!object.get() || !action_filter_applies_to(*action, object))) {
            continue;
        }

        switch (action->verb) {
            case kCreateObject:
            case kCreateObjectSetDest:  create_object(action, focus, subject, offset); break;
            case kPlaySound:            play_sound(action, focus); break;
            case kMakeSparks:           make_sparks(action, focus); break;
            case kDie:                  die(action, focus, subject); break;
            case kNilTarget:            nil_target(action, focus); break;
            case kAlter:                alter(action, focus, subject, object); break;
            case kLandAt:               land_at(action, focus, subject); break;
            case kEnterWarp:            enter_warp(action, focus, subject); break;
            case kChangeScore:          change_score(action, focus); break;
            case kDeclareWinner:        declare_winner(action, focus); break;
            case kDisplayMessage:       display_message(action, focus); break;
            case kSetDestination:       set_destination(action, focus, subject); break;
            case kActivateSpecial:      activate_special(action, focus, subject); break;
            case kActivatePulse:        activate_pulse(action, focus, subject); break;
            case kActivateBeam:         activate_beam(action, focus, subject); break;
            case kColorFlash:           color_flash(action, focus); break;
            case kEnableKeys:           enable_keys(action, focus); break;
            case kDisableKeys:          disable_keys(action, focus); break;
            case kSetZoom:              set_zoom(action, focus); break;
            case kComputerSelect:       computer_select(action, focus); break;
            case kAssumeInitialObject:  assume_initial_object(action, focus); break;
        }

        switch (action->verb) {
            case kChangeScore:
            case kDisplayMessage:
                checkConditions = true;
                break;
        }
    }

    if (checkConditions) {
        CheckScenarioConditions(0);
    }
}

void exec(
        HandleList<Action> actions, Handle<SpaceObject> sObject, Handle<SpaceObject> dObject,
        Point* offset) {
    execute_actions(actions, sObject, dObject, offset, true);
}

void reset_action_queue() {
    gActionQueueData.reset(new actionQueueType[kActionQueueLength]);

    gFirstActionQueueNumber = -1;
    gFirstActionQueue = NULL;

    actionQueueType* action = gActionQueueData.get();
    for (int32_t i = 0; i < kActionQueueLength; i++) {
        action->actionRef = {-1, -1};
        action->nextActionQueueNum = -1;
        action->nextActionQueue = NULL;
        action->scheduledTime = -1;
        action->subjectObject = SpaceObject::none();
        action->subjectObjectNum = -1;
        action->subjectObjectID = -1;
        action->directObject = SpaceObject::none();
        action->directObjectNum = -1;
        action->directObjectID = -1;
        action->offset.h = action->offset.v = 0;
        action++;
    }
}

static void queue_action(
        HandleList<Action> actions, int32_t delayTime,
        Handle<SpaceObject> subjectObject, Handle<SpaceObject> directObject, Point* offset) {
    int32_t queueNumber = 0;
    actionQueueType* actionQueue = gActionQueueData.get();
    while (actionQueue->actionRef.size() && (queueNumber < kActionQueueLength)) {
        actionQueue++;
        queueNumber++;
    }

    if (queueNumber == kActionQueueLength) {
        return;
    }
    actionQueue->actionRef = actions;
    actionQueue->scheduledTime = delayTime;

    if (offset) {
        actionQueue->offset = *offset;
    } else {
        actionQueue->offset = Point{0, 0};
    }

    actionQueue->subjectObject = subjectObject;
    if (subjectObject.get()) {
        actionQueue->subjectObjectNum = subjectObject->number();
        actionQueue->subjectObjectID = subjectObject->id;
    } else {
        actionQueue->subjectObjectNum = -1;
        actionQueue->subjectObjectID = -1;
    }

    actionQueue->directObject = directObject;
    if (directObject.get()) {
        actionQueue->directObjectNum = directObject->number();
        actionQueue->directObjectID = directObject->id;
    } else {
        actionQueue->directObjectNum = -1;
        actionQueue->directObjectID = -1;
    }

    actionQueueType* previousQueue = NULL;
    actionQueueType* nextQueue = gFirstActionQueue;
    while (nextQueue && (nextQueue->scheduledTime < delayTime)) {
        previousQueue = nextQueue;
        nextQueue = nextQueue->nextActionQueue;
    }
    if (previousQueue) {
        actionQueue->nextActionQueue = previousQueue->nextActionQueue;
        actionQueue->nextActionQueueNum = previousQueue->nextActionQueueNum;

        previousQueue->nextActionQueue = actionQueue;
        previousQueue->nextActionQueueNum = queueNumber;
    } else {
        actionQueue->nextActionQueue = gFirstActionQueue;
        actionQueue->nextActionQueueNum = gFirstActionQueueNumber;
        gFirstActionQueue = actionQueue;
        gFirstActionQueueNumber = queueNumber;
    }
}

void execute_action_queue() {
    for (int32_t i = 0; i < kActionQueueLength; i++) {
        auto actionQueue = &gActionQueueData[i];
        if (actionQueue->actionRef.size()) {
            actionQueue->scheduledTime -= kDecideEveryCycles.count();
        }
    }

    while (gFirstActionQueue
            && gFirstActionQueue->actionRef.size()
            && (gFirstActionQueue->scheduledTime <= 0)) {
        int32_t subjectid = -1;
        if (gFirstActionQueue->subjectObject.get() && gFirstActionQueue->subjectObject->active) {
            subjectid = gFirstActionQueue->subjectObject->id;
        }

        int32_t directid = -1;
        if (gFirstActionQueue->directObject.get() && gFirstActionQueue->directObject->active) {
            directid = gFirstActionQueue->directObject->id;
        }
        if ((subjectid == gFirstActionQueue->subjectObjectID)
                && (directid == gFirstActionQueue->directObjectID)) {
            execute_actions(
                    gFirstActionQueue->actionRef,
                    gFirstActionQueue->subjectObject, gFirstActionQueue->directObject,
                    &gFirstActionQueue->offset, false);
        }
        gFirstActionQueue->actionRef = {-1, -1};
        gFirstActionQueue->scheduledTime = -1;
        gFirstActionQueue->subjectObject = SpaceObject::none();
        gFirstActionQueue->subjectObjectNum = -1;
        gFirstActionQueue->subjectObjectID = -1;
        gFirstActionQueue->directObject = SpaceObject::none();
        gFirstActionQueue->directObjectNum = -1;
        gFirstActionQueue->directObjectID = -1;
        gFirstActionQueue->offset = Point{0, 0};

        auto actionQueue = gFirstActionQueue;

        gFirstActionQueueNumber = gFirstActionQueue->nextActionQueueNum;
        gFirstActionQueue = gFirstActionQueue->nextActionQueue;

        actionQueue->nextActionQueueNum = -1;
        actionQueue->nextActionQueue = NULL;
    }
}

}  // namespace antares

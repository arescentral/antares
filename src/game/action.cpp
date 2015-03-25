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
#include "game/player-ship.hpp"
#include "game/scenario-maker.hpp"
#include "game/space-object.hpp"
#include "game/starfield.hpp"
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
using std::unique_ptr;

namespace antares {

const size_t kActionQueueLength     = 120;

struct actionQueueType {
    objectActionType            *action;
    int32_t                         actionNum;
    int32_t                         actionToDo;
    int32_t                         scheduledTime;
    actionQueueType         *nextActionQueue;
    int32_t                         nextActionQueueNum;
    spaceObjectType         *subjectObject;
    int32_t                         subjectObjectNum;
    int32_t                         subjectObjectID;
    spaceObjectType         *directObject;
    int32_t                         directObjectNum;
    int32_t                         directObjectID;
    Point                       offset;
};

static actionQueueType* gFirstActionQueue = NULL;
static int32_t gFirstActionQueueNumber = -1;

static unique_ptr<actionQueueType[]> gActionQueueData;

static baseObjectType kZeroBaseObject;
static spaceObjectType kZeroSpaceObject = {0, &kZeroBaseObject};

static void queue_action(
        objectActionType *action, int32_t actionNumber, int32_t actionToDo,
        int32_t delayTime, spaceObjectType *subjectObject,
        spaceObjectType *directObject, Point* offset);

bool action_filter_applies_to(const objectActionType& action, const baseObjectType& target) {
    if (action.exclusiveFilter == 0xffffffff) {
        return ((action.inclusiveFilter ^ target.buildFlags) & kLevelKeyTagMask) == 0;
    } else {
        return (action.inclusiveFilter & target.attributes) == action.inclusiveFilter;
    }
}

bool action_filter_applies_to(const objectActionType& action, const spaceObjectType& target) {
    if (action.exclusiveFilter == 0xffffffff) {
        return ((action.inclusiveFilter ^ target.baseType->buildFlags) & kLevelKeyTagMask) == 0;
    } else {
        return (action.inclusiveFilter & target.attributes) == action.inclusiveFilter;
    }
}

void execute_actions(
        int32_t whichAction, int32_t actionNum, spaceObjectType *sObject, spaceObjectType *dObject,
        Point* offset, bool allowDelay) {
    spaceObjectType *anObject, *originalSObject = sObject, *originalDObject = dObject;
    baseObjectType  *baseObject;
    int16_t         angle;
    fixedPointType  fpoint, newVel;
    int32_t         l;
    uint32_t        ul1;
    Fixed           f, f2;
    coordPointType  newLocation;
    Point           location;
    bool         OKtoExecute, checkConditions = false;
    Fixed           aFixed;
    uint8_t         tinyColor;

    if ( whichAction < 0) return;
    const auto begin = mGetObjectActionPtr(whichAction);
    const auto end = begin + actionNum;
    for (auto action = begin; action != end; ++action) {
        if (action->verb == kNoAction) {
            break;
        }
        if ( action->initialSubjectOverride != kNoShip)
            sObject = GetObjectFromInitialNumber( action->initialSubjectOverride);
        else
            sObject = originalSObject;
        if ( action->initialDirectOverride != kNoShip)
            dObject = GetObjectFromInitialNumber( action->initialDirectOverride);
         else
            dObject = originalDObject;

        if (( action->delay > 0) && ( allowDelay))
        {
            queue_action(
                    action, action - mGetObjectActionPtr(0), end - action,
                    action->delay, sObject, dObject, offset);
            return;
        }
        allowDelay = true;

        anObject = dObject;
        if (( action->reflexive) || ( anObject == NULL)) anObject = sObject;

        OKtoExecute = false;
        // This pair of conditions is a workaround for a bug which
        // manifests itself for example in the implementation of "Hold
        // Position".  When an object is instructed to hold position, it
        // gains its own location as its destination, triggering its
        // arrive action, but its target is nulled out.
        //
        // Arrive actions are typically only specified on objects with
        // non-zero order flags (so that a transport won't attempt to
        // land on a bunker station, for example).  So, back when Ares
        // ran without protected memory, and NULL pointed to a
        // zeroed-out area of the address space, the flags would prevent
        // the arrive action from triggering.
        //
        // It's not correct to always inhibit the action here, because
        // the arrive action should be triggered when the anObject
        // doesn't have flags.  But we need to prevent it in the case of
        // transports somehow, so we emulate the old behavior of
        // pointing to a zeroed-out object.
        if (dObject == NULL) {
            dObject = &kZeroSpaceObject;
        }
        if (sObject == NULL) {
            sObject = &kZeroSpaceObject;
        }
        if (anObject == NULL) {
        }

        if (anObject == NULL) {
            OKtoExecute = true;
            anObject = &kZeroSpaceObject;
        } else if ( ( action->owner == 0) ||
                    (
                        (
                            ( action->owner == -1) &&
                            ( dObject->owner != sObject->owner)
                        ) ||
                        (
                            ( action->owner == 1) &&
                            ( dObject->owner == sObject->owner)
                        )
                    )
                ) {
            OKtoExecute = action_filter_applies_to(*action, *dObject);
        }

        if (!OKtoExecute) {
            continue;
        }

        switch ( action->verb)
        {
            case kCreateObject:
            case kCreateObjectSetDest: {
                baseObject = mGetBaseObjectPtr( action->argument.createObject.whichBaseType);
                int16_t end = action->argument.createObject.howManyMinimum;
                if ( action->argument.createObject.howManyRange > 0)
                    end += anObject->randomSeed.next(
                            action->argument.createObject.howManyRange);
                while ( end > 0)
                {
                    if ( action->argument.createObject.velocityRelative)
                        fpoint = anObject->velocity;
                    else
                        fpoint.h = fpoint.v = 0;
                    l = 0;
                    if  ( baseObject->attributes & kAutoTarget)
                    {
                        l = sObject->targetAngle;
                    } else if ( action->argument.createObject.directionRelative)
                            l = anObject->direction;
                    /*
                    l += baseObject->initialDirection;
                    if ( baseObject->initialDirectionRange > 0)
                        l += anObject->randomSeed.next(baseObject->initialDirectionRange);
                    */
                    newLocation = anObject->location;
                    if ( offset != NULL)
                    {
                        newLocation.h += offset->h;
                        newLocation.v += offset->v;
                    }

                    if ( action->argument.createObject.randomDistance > 0)
                    {
                        newLocation.h += anObject->randomSeed.next(
                                action->argument.createObject.randomDistance << 1)
                            - action->argument.createObject.randomDistance;
                        newLocation.v += anObject->randomSeed.next(
                                action->argument.createObject.randomDistance << 1)
                            - action->argument.createObject.randomDistance;
                    }

//                      l = CreateAnySpaceObject( action->argument.createObject.whichBaseType, &fpoint,
//                              &newLocation, l, anObject->owner, 0, nil, -1, -1, -1);
                    l = CreateAnySpaceObject( action->argument.createObject.whichBaseType, &fpoint,
                            &newLocation, l, anObject->owner, 0, -1);

                    if ( l >= 0)
                    {
                        spaceObjectType *newObject = mGetSpaceObjectPtr(l);
                        if ( newObject->attributes & kCanAcceptDestination)
                        {
                            ul1 = newObject->attributes;
                            newObject->attributes &= ~kStaticDestination;
                            if ( newObject->owner >= 0)
                            {
                                if ( action->reflexive)
                                {
                                    if ( action->verb != kCreateObjectSetDest)
                                        SetObjectDestination( newObject, anObject);
                                    else if ( anObject->destObjectPtr != NULL)
                                    {
                                        SetObjectDestination( newObject, anObject->destObjectPtr);
                                    }
                                }
                            } else if ( action->reflexive)
                            {
                                newObject->destObjectPtr = anObject;
                                newObject->timeFromOrigin = kTimeToCheckHome;
                                newObject->runTimeFlags &= ~kHasArrived;
                                newObject->destinationObject = anObject->entryNumber; //a->destinationObject;
                                newObject->destObjectDest = anObject->destinationObject;
                                newObject->destObjectID = anObject->id;
                                newObject->destObjectDestID = anObject->destObjectID;

                            }
                            newObject->attributes = ul1;
                        }
                        newObject->targetObjectNumber = anObject->targetObjectNumber;
                        newObject->targetObjectID = anObject->targetObjectID;
                        newObject->closestObject = newObject->targetObjectNumber;

                        //
                        //  ugly though it is, we have to fill in the rest of
                        //  a new beam's fields after it's created.
                        //

                        if ( newObject->attributes & kIsBeam)
                        {
                            if ( newObject->frame.beam.beam->beamKind !=
                                eKineticBeamKind)
                            // special beams need special post-creation acts
                            {
                                Beams::set_attributes(newObject, anObject);
                            }
                        }
                    }

                    end--;
                }
            } break;

            case kPlaySound:
                l = action->argument.playSound.volumeMinimum;
                angle = action->argument.playSound.idMinimum;
                if ( action->argument.playSound.idRange > 0)
                {
                    angle += anObject->randomSeed.next(
                        action->argument.playSound.idRange + 1);
                }
                if ( !action->argument.playSound.absolute)
                {
                    mPlayDistanceSound(l, anObject, angle, action->argument.playSound.persistence, static_cast<soundPriorityType>(action->argument.playSound.priority));
                } else
                {
                    PlayVolumeSound( angle, l,
                                action->argument.playSound.persistence,
                                static_cast<soundPriorityType>(action->argument.playSound.priority));
                }

                break;

            case kMakeSparks:
                if ( anObject->sprite != NULL)
                {
                    location.h = anObject->sprite->where.h;
                    location.v = anObject->sprite->where.v;
                    globals()->starfield.make_sparks(
                            action->argument.makeSparks.howMany,        // sparkNum
                            action->argument.makeSparks.speed,          // sparkSpeed
                            action->argument.makeSparks.velocityRange,  // velocity
                            action->argument.makeSparks.color,          // COLOR
                            &location);                                 // location
                } else
                {
                    l = ( anObject->location.h - gGlobalCorner.h) * gAbsoluteScale;
                    l >>= SHIFT_SCALE;
                    if (( l > -kSpriteMaxSize) && ( l < kSpriteMaxSize))
                        location.h = l + viewport.left;
                    else
                        location.h = -kSpriteMaxSize;

                    l = (anObject->location.v - gGlobalCorner.v) * gAbsoluteScale;
                    l >>= SHIFT_SCALE; /*+ CLIP_TOP*/;
                    if (( l > -kSpriteMaxSize) && ( l < kSpriteMaxSize))
                        location.v = l + viewport.top;
                    else
                        location.v = -kSpriteMaxSize;

                    globals()->starfield.make_sparks(
                            action->argument.makeSparks.howMany,        // sparkNum
                            action->argument.makeSparks.speed,          // sparkSpeed
                            action->argument.makeSparks.velocityRange,  // velocity
                            action->argument.makeSparks.color,          // COLOR
                            &location);                                 // location
                }
                break;

            case kDie:
//                  if ( anObject->attributes & kIsBeam)
//                      anObject->frame.beam.killMe = true;
                switch ( action->argument.killObject.dieType)
                {
                    case kDieExpire:
                        if ( sObject != NULL)
                        {
                            // if the object is occupied by a human, eject him since he can't die
                            if (( sObject->attributes & (kIsPlayerShip | kRemoteOrHuman)) &&
                                (!(sObject->baseType->destroyActionNum & kDestroyActionDontDieFlag)))
                            {
                                CreateFloatingBodyOfPlayer( sObject);
                            }

                            if ( sObject->baseType->expireAction >= 0)
                            {
//                                  ExecuteObjectActions(
//                                      sObject->baseType->expireAction,
//                                      sObject->baseType->expireActionNum
//                                       & kDestroyActionNotMask,
//                                      sObject, dObject, offset, allowDelay);
                            }
                            sObject->active = kObjectToBeFreed;
                        }
                        break;

                    case kDieDestroy:
                        if ( sObject != NULL)
                        {
                            // if the object is occupied by a human, eject him since he can't die
                            if (( sObject->attributes & (kIsPlayerShip | kRemoteOrHuman)) &&
                                (!(sObject->baseType->destroyActionNum & kDestroyActionDontDieFlag)))
                            {
                                CreateFloatingBodyOfPlayer( sObject);
                            }

                            DestroyObject( sObject);
                        }
                        break;

                    default:
                        // if the object is occupied by a human, eject him since he can't die
                        if (( anObject->attributes & (kIsPlayerShip | kRemoteOrHuman)) &&
                            (!(anObject->baseType->destroyActionNum & kDestroyActionDontDieFlag)))
                        {
                            CreateFloatingBodyOfPlayer( anObject);
                        }
                        anObject->active = kObjectToBeFreed;
                        break;
                }
                break;

            case kNilTarget:
                anObject->targetObjectNumber = kNoShip;
                anObject->targetObjectID = kNoShip;
                anObject->lastTarget = kNoShip;
                break;

            case kAlter:
                switch( action->argument.alterObject.alterType)
                {
                    case kAlterDamage:
                        AlterObjectHealth( anObject,
                            action->argument.alterObject.minimum);
                        break;

                    case kAlterEnergy:
                        AlterObjectEnergy( anObject,
                            action->argument.alterObject.minimum);
                        break;

                    /*
                    case 919191://kAlterSpecial:
                        anObject->specialType = action->argument.alterObject.minimum;
                        baseObject = mGetBaseObjectPtr( anObject->specialType);
                        anObject->specialAmmo = baseObject->frame.weapon.ammo;
                        anObject->specialTime = anObject->specialPosition = 0;
                        if ( baseObject->frame.weapon.range > anObject->longestWeaponRange)
                            anObject->longestWeaponRange = baseObject->frame.weapon.range;
                        if ( baseObject->frame.weapon.range < anObject->shortestWeaponRange)
                            anObject->shortestWeaponRange = baseObject->frame.weapon.range;
                        break;
                    */

                    case kAlterHidden:
                        l = 0;
                        do
                        {
                            UnhideInitialObject( action->argument.alterObject.minimum + l);
                            l++;
                        } while ( l <= action->argument.alterObject.range);
                        break;

                    case kAlterCloak:
                        AlterObjectCloakState( anObject, true);
                        break;

                    case kAlterSpin:
                        if ( anObject->attributes & kCanTurn)
                        {
                            if ( anObject->attributes & kShapeFromDirection)
                            {
                                f = mMultiplyFixed( anObject->baseType->frame.rotation.maxTurnRate,
                                                action->argument.alterObject.minimum +
                                                anObject->randomSeed.next(
                                                    action->argument.alterObject.range));
                            } else
                            {
                                f = mMultiplyFixed( 2 /*kDefaultTurnRate*/,
                                                action->argument.alterObject.minimum +
                                                anObject->randomSeed.next(
                                                    action->argument.alterObject.range));
                            }
                            f2 = anObject->baseType->mass;
                            if ( f2 == 0) f = -1;
                            else
                            {
                                f = mDivideFixed( f, f2);
                            }
                            anObject->turnVelocity = f;
                            /*
                            anObject->frame.rotation.turnVelocity =
                                    mMultiplyFixed( anObject->baseType->frame.rotation.maxTurnRate,
                                        action->argument.alterObject.minimum);

                            anObject->frame.rotation.turnVelocity += anObject->randomSeed(f);
                            */
                        }
                        break;

                    case kAlterOffline:
                        f = action->argument.alterObject.minimum +
                            anObject->randomSeed.next(action->argument.alterObject.range);
                        f2 = anObject->baseType->mass;
                        if ( f2 == 0) anObject->offlineTime = -1;
                        else
                        {
                            anObject->offlineTime = mDivideFixed( f, f2);
                        }
                        anObject->offlineTime = mFixedToLong( anObject->offlineTime);
                        break;

                    case kAlterVelocity:
                        if ( sObject != NULL)
                        {
                            // active (non-reflexive) altering of velocity means a PUSH, just like
                            //  two objects colliding.  Negative velocity = slow down
                            if ((dObject != NULL) && (dObject != &kZeroSpaceObject)) {
                                if ( action->argument.alterObject.relative)
                                {
                                    if (( dObject->baseType->mass > 0) &&
                                        ( dObject->maxVelocity > 0))
                                    {
                                        if ( action->argument.alterObject.minimum >= 0)
                                        {
                                            // if the minimum >= 0, then PUSH the object like collision
                                            f = sObject->velocity.h - dObject->velocity.h;
                                            f /= dObject->baseType->mass;
                                            f <<= 6L;
                                            dObject->velocity.h += f;
                                            f = sObject->velocity.v - dObject->velocity.v;
                                            f /= dObject->baseType->mass;
                                            f <<= 6L;
                                            dObject->velocity.v += f;

                                            // make sure we're not going faster than our top speed

                                            if ( dObject->velocity.h == 0)
                                            {
                                                if ( dObject->velocity.v < 0)
                                                    angle = 180;
                                                else angle = 0;
                                            } else
                                            {
                                                aFixed = MyFixRatio( dObject->velocity.h, dObject->velocity.v);

                                                angle = AngleFromSlope( aFixed);
                                                if ( dObject->velocity.h > 0) angle += 180;
                                                if ( angle >= 360) angle -= 360;
                                            }
                                        } else
                                        {
                                            // if the minumum < 0, then STOP the object like applying breaks
                                            f = dObject->velocity.h;
                                            f = mMultiplyFixed( f, action->argument.alterObject.minimum);
//                                              f /= dObject->baseType->mass;
//                                              f <<= 6L;
                                            dObject->velocity.h += f;
                                            f = dObject->velocity.v;
                                            f = mMultiplyFixed( f, action->argument.alterObject.minimum);
//                                              f /= dObject->baseType->mass;
//                                              f <<= 6L;
                                            dObject->velocity.v += f;

                                            // make sure we're not going faster than our top speed

                                            if ( dObject->velocity.h == 0)
                                            {
                                                if ( dObject->velocity.v < 0)
                                                    angle = 180;
                                                else angle = 0;
                                            } else
                                            {
                                                aFixed = MyFixRatio( dObject->velocity.h, dObject->velocity.v);

                                                angle = AngleFromSlope( aFixed);
                                                if ( dObject->velocity.h > 0) angle += 180;
                                                if ( angle >= 360) angle -= 360;
                                            }
                                        }

                                        // get the maxthrust of new vector

                                        GetRotPoint(&f, &f2, angle);

                                        f = mMultiplyFixed( dObject->maxVelocity, f);
                                        f2 = mMultiplyFixed( dObject->maxVelocity, f2);

                                        if ( f < 0)
                                        {
                                            if ( dObject->velocity.h < f)
                                                dObject->velocity.h = f;
                                        } else
                                        {
                                            if ( dObject->velocity.h > f)
                                                dObject->velocity.h = f;
                                        }

                                        if ( f2 < 0)
                                        {
                                            if ( dObject->velocity.v < f2)
                                                dObject->velocity.v = f2;
                                        } else
                                        {
                                            if ( dObject->velocity.v > f2)
                                                dObject->velocity.v = f2;
                                        }
                                    }
                                } else
                                {
                                    GetRotPoint(&f, &f2, sObject->direction);
                                    f = mMultiplyFixed( action->argument.alterObject.minimum, f);
                                    f2 = mMultiplyFixed( action->argument.alterObject.minimum, f2);
                                    anObject->velocity.h = f;
                                    anObject->velocity.v = f2;
                                }
                            } else
                            // reflexive alter velocity means a burst of speed in the direction
                            // the object is facing, where negative speed means backwards. Object can
                            // excede its max velocity.
                            // Minimum value is absolute speed in direction.
                            {
                                GetRotPoint(&f, &f2, anObject->direction);
                                f = mMultiplyFixed( action->argument.alterObject.minimum, f);
                                f2 = mMultiplyFixed( action->argument.alterObject.minimum, f2);
                                if ( action->argument.alterObject.relative)
                                {
                                    anObject->velocity.h += f;
                                    anObject->velocity.v += f2;
                                } else
                                {
                                    anObject->velocity.h = f;
                                    anObject->velocity.v = f2;
                                }
                            }

                        }
                        break;

                    case kAlterMaxVelocity:
                        if ( action->argument.alterObject.minimum < 0)
                        {
                            anObject->maxVelocity = anObject->baseType->maxVelocity;
                        } else
                        {
                            anObject->maxVelocity =
                                action->argument.alterObject.minimum;
                        }
                        break;

                    case kAlterThrust:
                        f = action->argument.alterObject.minimum +
                            anObject->randomSeed.next(action->argument.alterObject.range);
                        if ( action->argument.alterObject.relative)
                        {
                            anObject->thrust += f;
                        } else
                        {
                            anObject->thrust = f;
                        }
                        break;

                    case kAlterBaseType:
                        if ((action->reflexive)
                                || ((dObject != NULL) && (dObject != &kZeroSpaceObject)))
                        ChangeObjectBaseType( anObject, action->argument.alterObject.minimum, -1,
                            action->argument.alterObject.relative);
                        break;

                    case kAlterOwner:
/*                          anObject->owner = action->argument.alterObject.minimum;
                        if ( anObject->attributes & kIsDestination)
                            RecalcAllAdmiralBuildData();
*/
                        if ( action->argument.alterObject.relative)
                        {
                            // if it's relative AND reflexive, we take the direct
                            // object's owner, since relative & reflexive would
                            // do nothing.
                            if ((action->reflexive) && (dObject != NULL)
                                    && (dObject != &kZeroSpaceObject))
                                AlterObjectOwner( anObject, dObject->owner, true);
                            else
                                AlterObjectOwner( anObject, sObject->owner, true);
                        } else
                        {
                            AlterObjectOwner( anObject,
                                    action->argument.alterObject.minimum, false);
                        }
                        break;

                    case kAlterConditionTrueYet:
                        if ( action->argument.alterObject.range <= 0)
                        {
                            gThisScenario->condition(action->argument.alterObject.minimum)
                                ->set_true_yet(action->argument.alterObject.relative);
                        } else
                        {
                            for (
                                    l = action->argument.alterObject.minimum;
                                    l <=    (
                                                action->argument.alterObject.minimum +
                                                action->argument.alterObject.range
                                            )
                                            ;
                                    l++
                                )
                            {
                                gThisScenario->condition(l)->set_true_yet(
                                        action->argument.alterObject.relative);
                            }

                        }
                        break;

                    case kAlterOccupation:
                        AlterObjectOccupation( anObject, sObject->owner, action->argument.alterObject.minimum, true);
                        break;

                    case kAlterAbsoluteCash:
                        if ( action->argument.alterObject.relative)
                        {
                            if (anObject != &kZeroSpaceObject) {
                                PayAdmiralAbsolute( anObject->owner, action->argument.alterObject.minimum);
                            }
                        } else
                        {
                            PayAdmiralAbsolute( action->argument.alterObject.range,
                                action->argument.alterObject.minimum);
                        }
                        break;

                    case kAlterAge:
                        l = action->argument.alterObject.minimum +
                            anObject->randomSeed.next(action->argument.alterObject.range);

                        if ( action->argument.alterObject.relative)
                        {
                            if ( anObject->age >= 0)
                            {
                                anObject->age += l;

                                if ( anObject->age < 0) anObject->age = 0;
                            } else
                            {
                                anObject->age += l;
                            }
                        } else
                        {
                            anObject->age = l;
                        }
                        break;

                    case kAlterLocation:
                        if ( action->argument.alterObject.relative)
                        {
                            if ((dObject == NULL) && (dObject != &kZeroSpaceObject)) {
                                newLocation.h = sObject->location.h;
                                newLocation.v = sObject->location.v;
                            } else {
                                newLocation.h = dObject->location.h;
                                newLocation.v = dObject->location.v;
                            }
                        } else
                        {
                            newLocation.h = newLocation.v = 0;
                        }
                        newLocation.h += anObject->randomSeed.next(
                                action->argument.alterObject.minimum << 1)
                            - action->argument.alterObject.minimum;
                        newLocation.v += anObject->randomSeed.next(
                                action->argument.alterObject.minimum << 1)
                            - action->argument.alterObject.minimum;
                        anObject->location.h = newLocation.h;
                        anObject->location.v = newLocation.v;
                        break;

                    case kAlterAbsoluteLocation:
                        if ( action->argument.alterObject.relative)
                        {
                            anObject->location.h += action->argument.alterObject.minimum;
                            anObject->location.v += action->argument.alterObject.range;
                        } else
                        {
                            anObject->location = Translate_Coord_To_Scenario_Rotation(
                                action->argument.alterObject.minimum,
                                action->argument.alterObject.range);
                        }
                        break;

                    case kAlterWeapon1:
                        anObject->pulseType = action->argument.alterObject.minimum;
                        if ( anObject->pulseType != kNoWeapon)
                        {
                            baseObject = anObject->pulseBase =
                                mGetBaseObjectPtr( anObject->pulseType);
                            anObject->pulseAmmo =
                                baseObject->frame.weapon.ammo;
                            anObject->pulseTime =
                                anObject->pulsePosition = 0;
                            if ( baseObject->frame.weapon.range > anObject->longestWeaponRange)
                                anObject->longestWeaponRange = baseObject->frame.weapon.range;
                            if ( baseObject->frame.weapon.range < anObject->shortestWeaponRange)
                                anObject->shortestWeaponRange = baseObject->frame.weapon.range;
                        } else
                        {
                            anObject->pulseBase = NULL;
                            anObject->pulseAmmo = 0;
                            anObject->pulseTime = 0;
                        }
                        break;

                    case kAlterWeapon2:
                        anObject->beamType = action->argument.alterObject.minimum;
                        if ( anObject->beamType != kNoWeapon)
                        {
                            baseObject = anObject->beamBase =
                                mGetBaseObjectPtr( anObject->beamType);
                            anObject->beamAmmo =
                                baseObject->frame.weapon.ammo;
                            anObject->beamTime =
                                anObject->beamPosition = 0;
                            if ( baseObject->frame.weapon.range > anObject->longestWeaponRange)
                                anObject->longestWeaponRange = baseObject->frame.weapon.range;
                            if ( baseObject->frame.weapon.range < anObject->shortestWeaponRange)
                                anObject->shortestWeaponRange = baseObject->frame.weapon.range;
                        } else
                        {
                            anObject->beamBase = NULL;
                            anObject->beamAmmo = 0;
                            anObject->beamTime = 0;
                        }
                        break;

                    case kAlterSpecial:
                        anObject->specialType = action->argument.alterObject.minimum;
                        if ( anObject->specialType != kNoWeapon)
                        {
                            baseObject = anObject->specialBase =
                                mGetBaseObjectPtr( anObject->specialType);
                            anObject->specialAmmo =
                                baseObject->frame.weapon.ammo;
                            anObject->specialTime =
                                anObject->specialPosition = 0;
                            if ( baseObject->frame.weapon.range > anObject->longestWeaponRange)
                                anObject->longestWeaponRange = baseObject->frame.weapon.range;
                            if ( baseObject->frame.weapon.range < anObject->shortestWeaponRange)
                                anObject->shortestWeaponRange = baseObject->frame.weapon.range;
                        } else
                        {
                            anObject->specialBase = NULL;
                            anObject->specialAmmo = 0;
                            anObject->specialTime = 0;
                        }
                        break;

                    case kAlterLevelKeyTag:
                        break;

                    default:
                        break;

                }
                break;

            case kLandAt:
                // even though this is never a reflexive verb, we only effect ourselves
                if ( sObject->attributes & ( kIsPlayerShip | kRemoteOrHuman))
                {
                    CreateFloatingBodyOfPlayer( sObject);
                }
                sObject->presenceState = kLandingPresence;
                sObject->presenceData = sObject->baseType->naturalScale |
                    (action->argument.landAt.landingSpeed << kPresenceDataHiWordShift);
                break;

            case kEnterWarp:
                sObject->presenceState = kWarpInPresence;
//                  sObject->presenceData = action->argument.enterWarp.warpSpeed;
                sObject->presenceData = sObject->baseType->warpSpeed;
                sObject->attributes &= ~kOccupiesSpace;
                newVel.h = newVel.v = 0;
//                  CreateAnySpaceObject( globals()->scenarioFileInfo.warpInFlareID, &(newVel),
//                      &(sObject->location), sObject->direction, kNoOwner, 0, nil, -1, -1, -1);
                CreateAnySpaceObject( globals()->scenarioFileInfo.warpInFlareID, &(newVel),
                    &(sObject->location), sObject->direction, kNoOwner, 0, -1);
                break;

            case kChangeScore:
                if (( action->argument.changeScore.whichPlayer == -1) && (anObject != &kZeroSpaceObject))
                    l = anObject->owner;
                else
                {
                    l = mGetRealAdmiralNum( action->argument.changeScore.whichPlayer);
                }
                if ( l >= 0)
                {
                    AlterAdmiralScore( l, action->argument.changeScore.whichScore, action->argument.changeScore.amount);
                    checkConditions = true;
                }
                break;

            case kDeclareWinner:
                if (( action->argument.declareWinner.whichPlayer == -1) && (anObject != &kZeroSpaceObject))
                    l = anObject->owner;
                else
                {
                    l = mGetRealAdmiralNum( action->argument.declareWinner.whichPlayer);
                }
                DeclareWinner( l, action->argument.declareWinner.nextLevel, action->argument.declareWinner.textID);
                break;

            case kDisplayMessage:
                Messages::start(
                        action->argument.displayMessage.resID,
                        (action->argument.displayMessage.resID +
                         action->argument.displayMessage.pageNum - 1));
                checkConditions = true;

                break;

            case kSetDestination:
                ul1 = sObject->attributes;
                sObject->attributes &= ~kStaticDestination;
                SetObjectDestination( sObject, anObject);
                sObject->attributes = ul1;
                break;

            case kActivateSpecial:
                ActivateObjectSpecial( sObject);
                break;

            case kColorFlash:
                tinyColor = GetTranslateColorShade(action->argument.colorFlash.color, action->argument.colorFlash.shade);
                globals()->transitions.start_boolean(
                        action->argument.colorFlash.length,
                        action->argument.colorFlash.length, tinyColor);
                break;

            case kEnableKeys:
                globals()->keyMask = globals()->keyMask &
                                                ~action->argument.keys.keyMask;
                break;

            case kDisableKeys:
                globals()->keyMask = globals()->keyMask |
                                                action->argument.keys.keyMask;
                break;

            case kSetZoom:
                if (action->argument.zoom.zoomLevel != globals()->gZoomMode)
                {
                    globals()->gZoomMode = static_cast<ZoomType>(action->argument.zoom.zoomLevel);
                    PlayVolumeSound(  kComputerBeep3, kMediumVolume, kMediumPersistence, kLowPrioritySound);
                    StringList strings(kMessageStringID);
                    StringSlice string = strings.at(globals()->gZoomMode + kZoomStringOffset - 1);
                    Messages::set_status(string, kStatusLabelColor);
                }
                break;

            case kComputerSelect:
                MiniComputer_SetScreenAndLineHack( action->argument.computerSelect.screenNumber,
                    action->argument.computerSelect.lineNumber);
                break;

            case kAssumeInitialObject:
            {
                Scenario::InitialObject *initialObject;

                initialObject = gThisScenario->initial(action->argument.assumeInitial.whichInitialObject+GetAdmiralScore(0, 0));
                if ( initialObject != NULL)
                {
                    initialObject->realObjectID = anObject->id;
                    initialObject->realObjectNumber = anObject->entryNumber;
                }
            }
                break;

            default:
                break;
        }
    }

    if ( checkConditions) CheckScenarioConditions( 0);
}

void reset_action_queue() {
    gActionQueueData.reset(new actionQueueType[kActionQueueLength]);
    actionQueueType *action = gActionQueueData.get();
    int32_t         i;

    gFirstActionQueueNumber = -1;
    gFirstActionQueue = NULL;

    for ( i = 0; i < kActionQueueLength; i++)
    {
        action->actionNum = -1;
        action->actionToDo = 0;
        action->action = NULL;
        action->nextActionQueueNum = -1;
        action->nextActionQueue = NULL;
        action->scheduledTime = -1;
        action->subjectObject = NULL;
        action->subjectObjectNum = -1;
        action->subjectObjectID = -1;
        action->directObject = NULL;
        action->directObjectNum = -1;
        action->directObjectID = -1;
        action->offset.h = action->offset.v = 0;
        action++;
    }
}

static void queue_action(
        objectActionType *action, int32_t actionNumber, int32_t actionToDo,
        int32_t delayTime, spaceObjectType *subjectObject,
        spaceObjectType *directObject, Point* offset) {
    int32_t             queueNumber = 0;
    actionQueueType     *actionQueue = gActionQueueData.get(),
                        *nextQueue = gFirstActionQueue, *previousQueue = NULL;

    while (( actionQueue->action != NULL) && ( queueNumber < kActionQueueLength))
    {
        actionQueue++;
        queueNumber++;
    }

    if ( queueNumber == kActionQueueLength) return;
    actionQueue->action = action;
    actionQueue->actionNum = actionNumber;
    actionQueue->scheduledTime = delayTime;
    actionQueue->subjectObject = subjectObject;
    actionQueue->actionToDo = actionToDo;

    if ( offset == NULL)
    {
        actionQueue->offset.h = actionQueue->offset.v = 0;
    } else
    {
        actionQueue->offset.h = offset->h;
        actionQueue->offset.v = offset->v;
    }

    if ( subjectObject != NULL)
    {
        actionQueue->subjectObjectNum = subjectObject->entryNumber;
        actionQueue->subjectObjectID = subjectObject->id;
    } else
    {
        actionQueue->subjectObjectNum = -1;
        actionQueue->subjectObjectID = -1;
    }
    actionQueue->directObject = directObject;
    if ( directObject != NULL)
    {
        actionQueue->directObjectNum = directObject->entryNumber;
        actionQueue->directObjectID = directObject->id;
    } else
    {
        actionQueue->directObjectNum = -1;
        actionQueue->directObjectID = -1;
    }

    while (( nextQueue != NULL) && ( nextQueue->scheduledTime < delayTime))
    {
        previousQueue = nextQueue;
        nextQueue = nextQueue->nextActionQueue;
    }
    if ( previousQueue == NULL)
    {
        actionQueue->nextActionQueue = gFirstActionQueue;
        actionQueue->nextActionQueueNum = gFirstActionQueueNumber;
        gFirstActionQueue = actionQueue;
        gFirstActionQueueNumber = queueNumber;
    } else
    {
        actionQueue->nextActionQueue = previousQueue->nextActionQueue;
        actionQueue->nextActionQueueNum = previousQueue->nextActionQueueNum;

        previousQueue->nextActionQueue = actionQueue;
        previousQueue->nextActionQueueNum = queueNumber;
    }
}

void execute_action_queue(int32_t unitsToDo) {
//  actionQueueType     *actionQueue = gFirstActionQueue;
    actionQueueType     *actionQueue = gActionQueueData.get();
    int32_t                     subjectid, directid, i;

    for ( i = 0; i < kActionQueueLength; i++)
    {
        if ( actionQueue->action != NULL)
        {
            actionQueue->scheduledTime -= unitsToDo;
        }
        actionQueue++;
    }

    actionQueue = gFirstActionQueue;
    while (( gFirstActionQueue != NULL) &&
        ( gFirstActionQueue->action != NULL) &&
        ( gFirstActionQueue->scheduledTime <= 0))
    {
        subjectid = -1;
        directid = -1;
        if ( gFirstActionQueue->subjectObject != NULL)
        {
            if ( gFirstActionQueue->subjectObject->active)
                subjectid = gFirstActionQueue->subjectObject->id;
        }

        if ( gFirstActionQueue->directObject != NULL)
        {
            if ( gFirstActionQueue->directObject->active)
                directid = gFirstActionQueue->directObject->id;
        }
        if (( subjectid == gFirstActionQueue->subjectObjectID) &&
            ( directid == gFirstActionQueue->directObjectID))
        {
            execute_actions(
                    gFirstActionQueue->actionNum,
                    gFirstActionQueue->actionToDo,
                    gFirstActionQueue->subjectObject, gFirstActionQueue->directObject,
                    &(gFirstActionQueue->offset), false);
        }
        gFirstActionQueue->actionNum = -1;
        gFirstActionQueue->actionToDo = 0;
        gFirstActionQueue->action = NULL;
        gFirstActionQueue->scheduledTime = -1;
        gFirstActionQueue->subjectObject = NULL;
        gFirstActionQueue->subjectObjectNum = -1;
        gFirstActionQueue->subjectObjectID = -1;
        gFirstActionQueue->directObject = NULL;
        gFirstActionQueue->directObjectNum = -1;
        gFirstActionQueue->directObjectID = -1;
        gFirstActionQueue->offset.h = gFirstActionQueue->offset.v = 0;

        actionQueue = gFirstActionQueue;

        gFirstActionQueueNumber = gFirstActionQueue->nextActionQueueNum;
        gFirstActionQueue = gFirstActionQueue->nextActionQueue;

        actionQueue->nextActionQueueNum = -1;
        actionQueue->nextActionQueue = NULL;
    }
}

}  // namespace antares

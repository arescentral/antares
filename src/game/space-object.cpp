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

#include "game/space-object.hpp"

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
namespace macroman = sfz::macroman;

namespace antares {

const size_t kActionQueueLength     = 120;

const uint8_t kFriendlyColor        = GREEN;
const uint8_t kHostileColor         = RED;
const uint8_t kNeutralColor         = SKY_BLUE;

const int32_t kBatteryToEnergyRatio = 5;

struct actionQueueType {
    objectActionType            *action;
    long                            actionNum;
    long                            actionToDo;
    long                            scheduledTime;
    actionQueueType         *nextActionQueue;
    long                            nextActionQueueNum;
    spaceObjectType         *subjectObject;
    long                            subjectObjectNum;
    long                            subjectObjectID;
    spaceObjectType         *directObject;
    long                            directObjectNum;
    long                            directObjectID;
    Point                       offset;
};

spaceObjectType* gRootObject = NULL;
long gRootObjectNumber = -1;
actionQueueType* gFirstActionQueue = NULL;
long gFirstActionQueueNumber = -1;
baseObjectType kZeroBaseObject;
spaceObjectType kZeroSpaceObject = {0, &kZeroBaseObject};

unique_ptr<spaceObjectType[]> gSpaceObjectData;
unique_ptr<baseObjectType[]> gBaseObjectData;
unique_ptr<objectActionType[]> gObjectActionData;
unique_ptr<actionQueueType[]> gActionQueueData;

void SpaceObjectHandlingInit() {
    bool correctBaseObjectColor = false;

    gSpaceObjectData.reset(new spaceObjectType[kMaxSpaceObject]);
    if (gBaseObjectData.get() == NULL) {
        Resource rsrc("objects", "bsob", kBaseObjectResID);
        BytesSlice in(rsrc.data());
        size_t count = rsrc.data().size() / baseObjectType::byte_size;
        globals()->maxBaseObject = count;
        gBaseObjectData.reset(new baseObjectType[count]);
        for (size_t i = 0; i < count; ++i) {
            read(in, gBaseObjectData[i]);
        }
        if (!in.empty()) {
            throw Exception("didn't consume all of base object data");
        }
        correctBaseObjectColor = true;
    }

    if (gObjectActionData.get() == NULL) {
        Resource rsrc("object-actions", "obac", kObjectActionResID);
        BytesSlice in(rsrc.data());
        size_t count = rsrc.data().size() / objectActionType::byte_size;
        globals()->maxObjectAction = count;
        gObjectActionData.reset(new objectActionType[count]);
        for (size_t i = 0; i < count; ++i) {
            read(in, gObjectActionData[i]);
        }
        if (!in.empty()) {
            throw Exception("didn't consume all of object action data");
        }
    }

    gActionQueueData.reset(new actionQueueType[kActionQueueLength]);
    if (correctBaseObjectColor) {
        CorrectAllBaseObjectColor();
    }
    ResetAllSpaceObjects();
    ResetActionQueueData();
}

void CleanupSpaceObjectHandling() {
    gBaseObjectData.reset();
    gSpaceObjectData.reset();
    gObjectActionData.reset();
    gActionQueueData.reset();
}

void ResetAllSpaceObjects() {
    spaceObjectType *anObject = NULL;
    short           i;

    gRootObject = NULL;
    gRootObjectNumber = -1;
    anObject = gSpaceObjectData.get();
    for (i = 0; i < kMaxSpaceObject; i++) {
//      anObject->attributes = 0;
        anObject->active = kObjectAvailable;
        anObject->sprite = NULL;
/*      anObject->whichSprite = kNoSprite;
        anObject->whichLabel = kNoLabel;
        anObject->entryNumber = i;
        anObject->baseType = nil;
        anObject->keysDown = 0;
        anObject->tinySize = 0;
        anObject->tinyColor = 0;
        anObject->shieldColor = kNoTinyColor;
        anObject->direction = 0;
        anObject->location.h = anObject->location.v = 0;
        anObject->collisionGrid.h = anObject->collisionGrid.v = 0;
        anObject->nextNearObject = nil;
        anObject->distanceGrid.h = anObject->distanceGrid.v = 0;
        anObject->nextFarObject = nil;
        anObject->previousObject = nil;
        anObject->previousObjectNumber = -1;
        anObject->nextObject = nil;
        anObject->nextObjectNumber = -1;
        anObject->runTimeFlags = 0;
        anObject->myPlayerFlag = 0;
        anObject->seenByPlayerFlags = 0xffffffff;
        anObject->hostileTowardsFlags = 0;
        anObject->destinationLocation.h = anObject->destinationLocation.v = kNoDestinationCoord;
        anObject->destinationObject = kNoDestinationObject;
        anObject->destObjectPtr = nil;
        anObject->destObjectDest = 0;
        anObject->destObjectID = 0;
        anObject->remoteFoeStrength = anObject->remoteFriendStrength = anObject->escortStrength =
            anObject->localFoeStrength = anObject->localFriendStrength = 0;
        anObject->bestConsideredTargetValue = anObject->currentTargetValue = 0xffffffff;
        anObject->bestConsideredTargetNumber = -1;

        anObject->timeFromOrigin = 0;
        anObject->motionFraction.h = anObject->motionFraction.v = 0;
        anObject->velocity.h = anObject->velocity.v = 0;
        anObject->thrust = 0;
        anObject->scaledCornerOffset.h = anObject->scaledCornerOffset.v = 0;
        anObject->scaledSize.h = anObject->scaledSize.v = 0;
        anObject->absoluteBounds.left = anObject->absoluteBounds.right = 0;
        anObject->randomSeed = 0;
        anObject->health = 0;
        anObject->energy = 0;
        anObject->battery = 0;
        anObject->rechargeTime = anObject->pulseCharge = anObject->beamCharge = anObject->specialCharge = 0;
        anObject->warpEnergyCollected = 0;
        anObject->owner = -1;
        anObject->age = 0;
        anObject->naturalScale = 0;
        anObject->id = -1;
        anObject->distanceFromPlayer.hi = anObject->distanceFromPlayer.lo = 0;
        anObject->closestDistance = kMaximumRelevantDistanceSquared;
        anObject->closestObject = kNoShip;
        anObject->targetObjectNumber = -1;
        anObject->targetObjectID = -1;
        anObject->targetAngle = 0;
        anObject->lastTarget = 0;
        anObject->lastTargetDistance = 0;
        anObject->longestWeaponRange = 0;
        anObject->shortestWeaponRange = 0;
        anObject->presenceState = kNormalPresence;
        anObject->presenceData = 0;
        anObject->pixResID = 0;
        anObject->pulseBase = nil;
        anObject->pulseType = 0;
        anObject->pulseTime = 0;
        anObject->pulseAmmo = 0;
        anObject->pulsePosition = 0;
        anObject->beamBase = nil;
        anObject->beamType = 0;
        anObject->beamTime = 0;
        anObject->beamAmmo = 0;
        anObject->beamPosition = 0;
        anObject->specialBase = nil;
        anObject->specialType = 0;
        anObject->specialTime = 0;
        anObject->specialAmmo = 0;
        anObject->specialPosition = 0;

        anObject->whichLabel = 0;
        anObject->offlineTime = 0;
        anObject->periodicTime = 0;
*/
        anObject++;
    }
}

void ResetActionQueueData( void)
{
    actionQueueType *action = gActionQueueData.get();
    long            i;

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

/* AddSpaceObject:
    Returns -1 if no object available, otherwise returns object #

int AddSpaceObject( spaceObjectType *sourceObject, long *canBuildType,
                short nameResID, short nameStrNum)
*/

int AddSpaceObject( spaceObjectType *sourceObject)

{
    spaceObjectType *destObject = NULL;
    int             whichObject = 0;
    NatePixTable* spriteTable = NULL;
    Point           where;
    long            scaleCalc;
    RgbColor        tinyColor;
    unsigned char   tinyShade;
    short           whichShape = 0, angle;

    destObject = gSpaceObjectData.get();

    while (( destObject->active) && ( whichObject < kMaxSpaceObject))
    {
        whichObject++;
        destObject++;
    }
    if ( whichObject == kMaxSpaceObject)
    {
        return( -1);
    }

    if ( sourceObject->pixResID != kNoSpriteTable)
    {
        spriteTable = GetPixTable( sourceObject->pixResID);

        if (spriteTable == NULL) {
            throw Exception("Received an unexpected request to load a sprite");
            spriteTable = AddPixTable( sourceObject->pixResID);
            if (spriteTable == NULL) {
                return -1;
            }
        }
    }

//  sourceObject->id = whichObject;
    *destObject = *sourceObject;

    destObject->lastLocation = destObject->location;
    destObject->collideObject = NULL;
    destObject->lastLocation.h += 100000;
    destObject->lastLocation.v += 100000;
    destObject->lastDir = destObject->direction;

                scaleCalc = ( destObject->location.h - gGlobalCorner.h) * gAbsoluteScale;
                scaleCalc >>= SHIFT_SCALE;
                where.h = scaleCalc + viewport.left;
                scaleCalc = (destObject->location.v - gGlobalCorner.v) * gAbsoluteScale;
                scaleCalc >>= SHIFT_SCALE; /*+ CLIP_TOP*/;
                where.v = scaleCalc;

//  where.h = destObject->location.h - gGlobalCorner.h + CLIP_LEFT;
//  where.v = destObject->location.v - gGlobalCorner.v /*+ CLIP_TOP*/;
    if ( destObject->sprite != NULL) RemoveSprite( destObject->sprite);

    if (spriteTable != NULL) {
        switch( destObject->layer)
        {
            case kFirstSpriteLayer:
                tinyShade = MEDIUM;
                break;

            case kMiddleSpriteLayer:
                tinyShade = LIGHT;
                break;

            case kLastSpriteLayer:
                tinyShade = VERY_LIGHT;
                break;

            default:
                tinyShade = DARK;
                break;
        }

        if ( destObject->tinySize == 0)
        {
            tinyColor = kNoTinyColor;
        } else if ( destObject->owner == globals()->gPlayerAdmiralNumber)
        {
            tinyColor = GetRGBTranslateColorShade(kFriendlyColor, tinyShade);
        } else if ( destObject->owner <= kNoOwner)
        {
            tinyColor = GetRGBTranslateColorShade(kNeutralColor, tinyShade);
        } else
        {
            tinyColor = GetRGBTranslateColorShade(kHostileColor, tinyShade);
        }

        if ( destObject->attributes & kIsSelfAnimated)
        {
            whichShape = more_evil_fixed_to_long(destObject->frame.animation.thisShape);
        } else if ( destObject->attributes & kShapeFromDirection)
        {
            angle = destObject->direction;
            mAddAngle( angle, destObject->baseType->frame.rotation.rotRes >> 1);
            whichShape = angle / destObject->baseType->frame.rotation.rotRes;
        }

        destObject->sprite = AddSprite( where, spriteTable, sourceObject->pixResID, whichShape,
            destObject->naturalScale, destObject->tinySize, destObject->layer, tinyColor,
            &(destObject->whichSprite));
        destObject->tinyColor = tinyColor;

        if ( destObject->sprite == NULL)
        {
            globals()->gGameOver = -1;
            destObject->active = kObjectAvailable;
            return( -1);
        }

        const NatePixTable::Frame& frame
            = destObject->sprite->table->at(destObject->sprite->whichShape);

        scaleCalc = (frame.width() * destObject->naturalScale);
        scaleCalc >>= SHIFT_SCALE;
        destObject->scaledSize.h = scaleCalc;
        scaleCalc = (frame.height() * destObject->naturalScale);
        scaleCalc >>= SHIFT_SCALE;
        destObject->scaledSize.v = scaleCalc;

        scaleCalc = frame.center().h * destObject->naturalScale;
        scaleCalc >>= SHIFT_SCALE;
        destObject->scaledCornerOffset.h = -scaleCalc;
        scaleCalc = frame.center().v * destObject->naturalScale;
        scaleCalc >>= SHIFT_SCALE;
        destObject->scaledCornerOffset.v = -scaleCalc;

    } else
    {
        destObject->scaledCornerOffset.h = destObject->scaledCornerOffset.v = 0;
        destObject->scaledSize.h = destObject->scaledSize.v = 0;
        destObject->sprite = NULL;
        destObject->whichSprite = kNoSprite;
    }

    if ( destObject->attributes & kIsBeam)
    {
        destObject->frame.beam.beam = AddBeam( &(destObject->location),
            destObject->baseType->frame.beam.color,
            destObject->baseType->frame.beam.kind,
            destObject->baseType->frame.beam.accuracy,
            destObject->baseType->frame.beam.range,
            &(destObject->frame.beam.whichBeam));
    }

    destObject->nextObject = gRootObject;
    destObject->nextObjectNumber = gRootObjectNumber;
    destObject->previousObject = NULL;
    destObject->previousObjectNumber = -1;
    if ( gRootObject != NULL)
    {
        gRootObject->previousObject = destObject;
        gRootObject->previousObjectNumber = whichObject;
    }
    gRootObject = destObject;
    gRootObjectNumber = whichObject;

    destObject->active = kObjectInUse;
    destObject->nextNearObject = destObject->nextFarObject = NULL;
    destObject->whichLabel = kNoLabel;
    destObject->entryNumber = whichObject;
    destObject->cloakState = destObject->hitState = 0;
    destObject->duty = eNoDuty;

//  if ( destObject->attributes & kCanThink)
//      SetObjectDestination( destObject);
//  else destObject->destinationObject = kNoDestinationObject;

/*
    if ( destObject->attributes & kIsDestination)
        destObject->destinationObject = MakeNewDestination( whichObject, canBuildType,
            mFloatToFixed( 1), nameResID, nameStrNum);
*/
    return ( whichObject);
}

int AddNumberedSpaceObject( spaceObjectType *sourceObject, long whichObject)

{
#pragma unused( sourceObject, whichObject)
/*  spaceObjectType *destObject = nil;
    Handle          spriteTable = nil;
    Point           where;
    spritePix       oldStyleSprite;
    long            scaleCalc;

    destObject = gSpaceObjectData.get() + whichObject;

    if ( whichObject == kMaxSpaceObject) return( -1);

    if ( sourceObject->pixResID == kNoSpriteTable)
    {
        spriteTable = GetPixTable( sourceObject->pixResID);
        if ( spriteTable == nil)
        {
            spriteTable = AddPixTable( sourceObject->pixResID);
            if ( spriteTable == nil)
            {
                return (-1);
            }

        }
    } else
    {
        spriteTable = nil;
    }

//  sourceObject->id = whichObject;
    *destObject = *sourceObject;
    where.h = destObject->location.h - gGlobalCorner.h + CLIP_LEFT;
    where.v = destObject->location.v - gGlobalCorner.v; //+ CLIP_TOP
    if ( destObject->sprite != nil) RemoveSprite( destObject->sprite);
    if ( spriteTable != nil)
    {
        destObject->sprite = AddSprite( where, spriteTable, 0, destObject->naturalScale,
                            destObject->tinySize, destObject->tinyColor);

        GetOldSpritePixData( destObject->sprite, &oldStyleSprite);

        scaleCalc = (oldStyleSprite.width * destObject->naturalScale);
        scaleCalc >>= SHIFT_SCALE;
        destObject->scaledSize.h = scaleCalc;
        scaleCalc = (oldStyleSprite.height * destObject->naturalScale);
        scaleCalc >>= SHIFT_SCALE;
        destObject->scaledSize.v = scaleCalc;

        scaleCalc = oldStyleSprite.center.h * destObject->naturalScale;
        scaleCalc >>= SHIFT_SCALE;
        destObject->scaledCornerOffset.h = -scaleCalc;
        scaleCalc = oldStyleSprite.center.v * destObject->naturalScale;
        scaleCalc >>= SHIFT_SCALE;
        destObject->scaledCornerOffset.v = -scaleCalc;
    } else destObject->sprite = nil;
    destObject->active = kObjectInUse;
    destObject->whichLabel = kNoLabel;
    destObject->entryNumber = whichObject;
    return ( whichObject);
*/  return ( 0);
}

void RemoveAllSpaceObjects( void)

{
    spaceObjectType *anObject;
    int             i;

    anObject = gSpaceObjectData.get();
    for ( i = 0; i < kMaxSpaceObject; i++)
    {
        if ( anObject->sprite != NULL)
        {
            RemoveSprite( anObject->sprite);
            anObject->sprite = NULL;
            anObject->whichSprite = kNoSprite;
        }
        anObject->active = kObjectAvailable;
        anObject->nextNearObject = anObject->nextFarObject = NULL;
        anObject->attributes = 0;
        anObject++;
    }
}

void CorrectAllBaseObjectColor( void)

{
    baseObjectType  *aBase = gBaseObjectData.get();
    short           i;

    for ( i = 0; i < globals()->maxBaseObject; i++)
    {
        if (( aBase->shieldColor != 0xFF) && ( aBase->shieldColor != 0))
        {
            aBase->shieldColor = GetTranslateColorShade(aBase->shieldColor, 15);
        }
        if ( aBase->attributes & kIsBeam)
        {
            if ( aBase->frame.beam.color > 16)
                aBase->frame.beam.color = GetTranslateIndex( aBase->frame.beam.color);
            else
            {
                aBase->frame.beam.color = 0;
            }
        }

//      if (( aBase->attributes & kCanThink) && ( aBase->warpSpeed <= 0))
//          aBase->warpSpeed = mLongToFixed( 50);

        if ( aBase->attributes & kIsSelfAnimated)
        {
            aBase->frame.animation.firstShape = mLongToFixed(aBase->frame.animation.firstShape);
            aBase->frame.animation.lastShape = mLongToFixed(aBase->frame.animation.lastShape);
            aBase->frame.animation.frameShape = mLongToFixed(aBase->frame.animation.frameShape);
            aBase->frame.animation.frameShapeRange = mLongToFixed(aBase->frame.animation.frameShapeRange);
        }
        aBase++;
    }

}

void InitSpaceObjectFromBaseObject( spaceObjectType *dObject, long  whichBaseObject, short seed,
            long direction, fixedPointType *velocity, long owner, short spriteIDOverride)

{
    baseObjectType  *sObject = mGetBaseObjectPtr( whichBaseObject), *weaponBase = NULL;
    short           i;
    long            r;
    Fixed           f;
    fixedPointType  newVel;
    long            l;

    dObject->offlineTime = 0;

    dObject->randomSeed = seed;
    dObject->attributes = sObject->attributes;
    dObject->baseType = sObject;
    dObject->whichBaseObject = whichBaseObject;
    dObject->keysDown = 0;
    dObject->timeFromOrigin = 0;
    dObject->runTimeFlags = 0;
    if ( owner >= 0)
        dObject->myPlayerFlag = 1 << owner;
    else dObject->myPlayerFlag = 0x80000000;
    dObject->seenByPlayerFlags = 0xffffffff;
    dObject->hostileTowardsFlags = 0;
    dObject->absoluteBounds.left = dObject->absoluteBounds.right = 0;
    dObject->shieldColor = sObject->shieldColor;
    dObject->tinySize = sObject->tinySize;
    dObject->layer = sObject->pixLayer;
    do
    {
        dObject->id =RandomSeeded( 32768, &(dObject->randomSeed), 'soh0', whichBaseObject);
    } while ( dObject->id == -1);

    dObject->distanceGrid.h = dObject->distanceGrid.v = dObject->collisionGrid.h = dObject->collisionGrid.v = 0;
    if ( sObject->activateActionNum & kPeriodicActionTimeMask)
    {
        dObject->periodicTime = ((sObject->activateActionNum & kPeriodicActionTimeMask) >> kPeriodicActionTimeShift) +
            RandomSeeded( ((sObject->activateActionNum & kPeriodicActionRangeMask) >> kPeriodicActionRangeShift),
            &(dObject->randomSeed), 'soh1', whichBaseObject);
    } else dObject->periodicTime = 0;

    r = sObject->initialDirection;
    mAddAngle( r, direction);
    if ( sObject->initialDirectionRange > 0)
    {
        i = RandomSeeded( sObject->initialDirectionRange, &(dObject->randomSeed), 'soh2',
            whichBaseObject);
        mAddAngle( r, i);
    }
    dObject->direction = r;

    f = sObject->initialVelocity;
    if ( sObject->initialVelocityRange > 0)
    {
        f += RandomSeeded( sObject->initialVelocityRange,
                    &(dObject->randomSeed), 'soh3', whichBaseObject);
    }
    GetRotPoint(&newVel.h, &newVel.v, r);
    newVel.h = mMultiplyFixed( newVel.h, f);
    newVel.v = mMultiplyFixed( newVel.v, f);

    if ( velocity != NULL)
    {
        newVel.h += velocity->h;
        newVel.v += velocity->v;
    }

    dObject->velocity.h = newVel.h;
    dObject->velocity.v = newVel.v;
    dObject->maxVelocity = sObject->maxVelocity;

    dObject->motionFraction.h = dObject->motionFraction.v = 0;
    if ((dObject->attributes & kCanThink) ||
            (dObject->attributes & kRemoteOrHuman))
        dObject->thrust = 0;
    else
        dObject->thrust = sObject->maxThrust;


    dObject->energy = sObject->energy;
    dObject->rechargeTime = dObject->pulseCharge = dObject->beamCharge = dObject->specialCharge = 0;
    dObject->warpEnergyCollected = 0;
    dObject->battery = sObject->energy * kBatteryToEnergyRatio;
    dObject->owner = owner;
    dObject->destinationObject = kNoDestinationObject;
    dObject->destinationLocation.h = dObject->destinationLocation.v = kNoDestinationCoord;
    dObject->destObjectPtr = NULL;
    dObject->destObjectDest = kNoDestinationObject;
    dObject->destObjectID = kNoDestinationObject;
    dObject->destObjectDestID = kNoDestinationObject;
    dObject->remoteFoeStrength = dObject->remoteFriendStrength = dObject->escortStrength =
        dObject->localFoeStrength = dObject->localFriendStrength = 0;
    dObject->bestConsideredTargetValue = dObject->currentTargetValue = 0xffffffff;
    dObject->bestConsideredTargetNumber = -1;

    // not setting: scaledCornerOffset, scaledSize, absoluteBounds;

    if ( dObject->attributes & kCanTurn)
    {
        dObject->directionGoal =
            dObject->turnFraction = dObject->turnVelocity = 0;
    }
    if ( dObject->attributes & kIsSelfAnimated)
    {
        dObject->frame.animation.thisShape = sObject->frame.animation.frameShape;
        if ( sObject->frame.animation.frameShapeRange > 0)
        {
            l = RandomSeeded( sObject->frame.animation.frameShapeRange,
                &(dObject->randomSeed), 'soh5', whichBaseObject);
            dObject->frame.animation.thisShape += l;
        }
        dObject->frame.animation.frameDirection =
            sObject->frame.animation.frameDirection;
        if ( sObject->frame.animation.frameDirectionRange == -1)
        {
            if (RandomSeeded( 2, &(dObject->randomSeed),'so51', whichBaseObject) == 1)
            {
                dObject->frame.animation.frameDirection = 1;
            }
        } else if ( sObject->frame.animation.frameDirectionRange > 0)
        {
            dObject->frame.animation.frameDirection += RandomSeeded(
                sObject->frame.animation.frameDirectionRange,
                &(dObject->randomSeed), 'so52', whichBaseObject);
        }
        dObject->frame.animation.frameFraction = 0;
        dObject->frame.animation.frameSpeed = sObject->frame.animation.frameSpeed;
    } else if ( dObject->attributes & kIsBeam)
    {
//      dObject->frame.beam.killMe = false;
    }

    // not setting lastTimeUpdate;

    dObject->health = sObject->health;

    // not setting owner

    if ( sObject->initialAge >= 0)
        dObject->age = sObject->initialAge + RandomSeeded( sObject->initialAgeRange, &(dObject->randomSeed), 'soh6',
            whichBaseObject);
    else dObject->age = -1;
    dObject->naturalScale = sObject->naturalScale;

    // not setting id

    dObject->active = kObjectInUse;
    dObject->nextNearObject = dObject->nextFarObject = NULL;

    // not setting sprite, targetObjectNumber, lastTarget, lastTargetDistance;

    dObject->closestDistance = kMaximumRelevantDistanceSquared;
    dObject->targetObjectNumber = dObject->lastTarget = dObject->targetObjectID = kNoShip;
    dObject->lastTargetDistance = dObject->targetAngle = 0;

    dObject->closestObject = kNoShip;
    dObject->presenceState = kNormalPresence;
    dObject->presenceData = 0;

    if ( spriteIDOverride == -1)
        dObject->pixResID = sObject->pixResID;
    else dObject->pixResID = spriteIDOverride;

    if ( sObject->attributes & kCanThink)
    {
        dObject->pixResID += (GetAdmiralColor( owner) << kSpriteTableColorShift);
    }

    dObject->pulseType = sObject->pulse;
    if ( dObject->pulseType != kNoWeapon)
        dObject->pulseBase = mGetBaseObjectPtr( dObject->pulseType);
    else dObject->pulseBase = NULL;
    dObject->beamType = sObject->beam;
    if ( dObject->beamType != kNoWeapon)
        dObject->beamBase = mGetBaseObjectPtr( dObject->beamType);
    else dObject->beamBase = NULL;
    dObject->specialType = sObject->special;
    if ( dObject->specialType != kNoWeapon)
        dObject->specialBase = mGetBaseObjectPtr( dObject->specialType);
    else dObject->specialBase = NULL;
    dObject->longestWeaponRange = 0;
    dObject->shortestWeaponRange = kMaximumRelevantDistance;

    if ( dObject->pulseType != kNoWeapon)
    {
        weaponBase = dObject->pulseBase;
        dObject->pulseAmmo = weaponBase->frame.weapon.ammo;
        dObject->pulseTime = dObject->pulsePosition = 0;
        r = weaponBase->frame.weapon.range;
        if (( r > 0) && ( weaponBase->frame.weapon.usage & kUseForAttacking))
        {
            if ( r > dObject->longestWeaponRange) dObject->longestWeaponRange = r;
            if ( r < dObject->shortestWeaponRange) dObject->shortestWeaponRange = r;
        }
    } else dObject->pulseTime = 0;

    if ( dObject->beamType != kNoWeapon)
    {
        weaponBase = dObject->beamBase;
        dObject->beamAmmo = weaponBase->frame.weapon.ammo;
        dObject->beamTime = dObject->beamPosition = 0;
        r = weaponBase->frame.weapon.range;
        if (( r > 0) && ( weaponBase->frame.weapon.usage & kUseForAttacking))
        {
            if ( r > dObject->longestWeaponRange) dObject->longestWeaponRange = r;
            if ( r < dObject->shortestWeaponRange) dObject->shortestWeaponRange = r;
        }
    } else dObject->beamTime = 0;

    if ( dObject->specialType != kNoWeapon)
    {
        weaponBase = dObject->specialBase;
        dObject->specialAmmo = weaponBase->frame.weapon.ammo;
        dObject->specialTime = dObject->specialPosition = 0;
        r = weaponBase->frame.weapon.range;
        if (( r > 0) && ( weaponBase->frame.weapon.usage & kUseForAttacking))
        {
            if ( r > dObject->longestWeaponRange) dObject->longestWeaponRange = r;
            if ( r < dObject->shortestWeaponRange) dObject->shortestWeaponRange = r;
        }
    } else dObject->specialTime = 0;

    // if we don't have any weapon, then shortest range is 0 too
    if ( dObject->longestWeaponRange == 0) dObject->shortestWeaponRange = 0;
    if ( dObject->longestWeaponRange > kEngageRange)
        dObject->engageRange = dObject->longestWeaponRange;
    else
        dObject->engageRange = kEngageRange;
}

//
// ChangeObjectBaseType:
// This is a very RISKY procedure. You probably shouldn't change anything fundamental about the object--
// meaning, attributes that change the way the object behaves, or the way other objects treat this object--
// so don't, for instance, give something the kCanThink attribute if it couldn't before.
// This routine is similar to "InitSpaceObjectFromBaseObject" except that it doesn't change many things
// (like the velocity, direction, or randomseed) AND it handles the sprite data itself!
// Can you change the frame type? Like from a direction frame to a self-animated frame? I'm not sure...
//

void ChangeObjectBaseType( spaceObjectType *dObject, long whichBaseObject,
    long spriteIDOverride, bool relative)

{
    baseObjectType  *sObject = mGetBaseObjectPtr( whichBaseObject), *weaponBase = NULL;
    short           angle;
    long            r;
    NatePixTable* spriteTable;

    dObject->attributes = sObject->attributes | (dObject->attributes &
        (kIsHumanControlled | kIsRemote | kIsPlayerShip | kStaticDestination));
    dObject->baseType = sObject;
    dObject->whichBaseObject = whichBaseObject;
    dObject->tinySize = sObject->tinySize;
    dObject->shieldColor = sObject->shieldColor;
    dObject->layer = sObject->pixLayer;

    if ( dObject->attributes & kCanTurn)
    {
        dObject->directionGoal =
            dObject->turnFraction = dObject->turnVelocity = 0;
    }
    if ( dObject->attributes & kIsSelfAnimated)
    {
        dObject->frame.animation.thisShape = sObject->frame.animation.frameShape;
        if ( sObject->frame.animation.frameShapeRange > 0)
        {
            r = RandomSeeded( sObject->frame.animation.frameShapeRange,
                &(dObject->randomSeed), 'soh6', whichBaseObject);
            dObject->frame.animation.thisShape += r;
        }
        dObject->frame.animation.frameDirection =
            sObject->frame.animation.frameDirection;
        if ( sObject->frame.animation.frameDirectionRange == -1)
        {
            if (RandomSeeded( 2, &(dObject->randomSeed),'so51', whichBaseObject) == 1)
            {
                dObject->frame.animation.frameDirection = 1;
            }
        } else if ( sObject->frame.animation.frameDirectionRange > 0)
        {
            dObject->frame.animation.frameDirection += RandomSeeded(
                sObject->frame.animation.frameDirectionRange,
                &(dObject->randomSeed), 'so52', whichBaseObject);
        }
        dObject->frame.animation.frameFraction = 0;
        dObject->frame.animation.frameSpeed = sObject->frame.animation.frameSpeed;
    } else if ( dObject->attributes & kIsBeam)
    {
//      dObject->frame.beam.killMe = false;
    }

    dObject->maxVelocity = sObject->maxVelocity;

    dObject->age = sObject->initialAge + RandomSeeded( sObject->initialAgeRange,
        &(dObject->randomSeed), 'soh7', whichBaseObject);

    dObject->naturalScale = sObject->naturalScale;

    // not setting id

    dObject->active = kObjectInUse;

    // not setting sprite, targetObjectNumber, lastTarget, lastTargetDistance;

    if ( spriteIDOverride == -1)
        dObject->pixResID = sObject->pixResID;
    else dObject->pixResID = spriteIDOverride;

    if ( sObject->attributes & kCanThink)
    {
        dObject->pixResID += (GetAdmiralColor( dObject->owner) << kSpriteTableColorShift);
    }

    dObject->pulseType = sObject->pulse;
    if ( dObject->pulseType != kNoWeapon)
        dObject->pulseBase = mGetBaseObjectPtr( dObject->pulseType);
    else dObject->pulseBase = NULL;
    dObject->beamType = sObject->beam;
    if ( dObject->beamType != kNoWeapon)
        dObject->beamBase = mGetBaseObjectPtr( dObject->beamType);
    else dObject->beamBase = NULL;
    dObject->specialType = sObject->special;
    if ( dObject->specialType != kNoWeapon)
        dObject->specialBase = mGetBaseObjectPtr( dObject->specialType);
    else dObject->specialBase = NULL;
    dObject->longestWeaponRange = 0;
    dObject->shortestWeaponRange = kMaximumRelevantDistance;

    // check periodic time
    if ( sObject->activateActionNum & kPeriodicActionTimeMask)
    {
        dObject->periodicTime = ((sObject->activateActionNum & kPeriodicActionTimeMask) >> kPeriodicActionTimeShift) +
            RandomSeeded( ((sObject->activateActionNum & kPeriodicActionRangeMask) >> kPeriodicActionRangeShift),
            &(dObject->randomSeed), 'soh8', whichBaseObject);
    } else dObject->periodicTime = 0;

    if ( dObject->pulseType != kNoWeapon)
    {
        weaponBase = dObject->pulseBase;
        if ( !relative)
        {
            dObject->pulseAmmo = weaponBase->frame.weapon.ammo;
            dObject->pulsePosition = 0;
            if ( dObject->pulseTime < 0)
                dObject->pulseTime = 0;
            else if ( dObject->pulseTime > weaponBase->frame.weapon.fireTime)
                dObject->pulseTime = weaponBase->frame.weapon.fireTime;
        }
        r = weaponBase->frame.weapon.range;
        if (( r > 0) && ( weaponBase->frame.weapon.usage & kUseForAttacking))
        {
            if ( r > dObject->longestWeaponRange) dObject->longestWeaponRange = r;
            if ( r < dObject->shortestWeaponRange) dObject->shortestWeaponRange = r;
        }
    } else dObject->pulseTime = 0;

    if ( dObject->beamType != kNoWeapon)
    {
        weaponBase = dObject->beamBase;
        if ( !relative)
        {
            dObject->beamAmmo = weaponBase->frame.weapon.ammo;
            if ( dObject->beamTime < 0)
                dObject->beamTime = 0;
            else if ( dObject->beamTime > weaponBase->frame.weapon.fireTime)
                dObject->beamTime = weaponBase->frame.weapon.fireTime;
            dObject->beamPosition = 0;
        }
        r = weaponBase->frame.weapon.range;
        if (( r > 0) && ( weaponBase->frame.weapon.usage & kUseForAttacking))
        {
            if ( r > dObject->longestWeaponRange) dObject->longestWeaponRange = r;
            if ( r < dObject->shortestWeaponRange) dObject->shortestWeaponRange = r;
        }
    } else dObject->beamTime = 0;

    if ( dObject->specialType != kNoWeapon)
    {
        weaponBase = dObject->specialBase;
        if ( !relative)
        {
            dObject->specialAmmo = weaponBase->frame.weapon.ammo;
            dObject->specialPosition = 0;
            if ( dObject->specialTime < 0)
                dObject->specialTime = 0;
            else if ( dObject->specialTime > weaponBase->frame.weapon.fireTime)
                dObject->specialTime = weaponBase->frame.weapon.fireTime;
        }
        r = weaponBase->frame.weapon.range;
        if (( r > 0) && ( weaponBase->frame.weapon.usage & kUseForAttacking))
        {
            if ( r > dObject->longestWeaponRange) dObject->longestWeaponRange = r;
            if ( r < dObject->shortestWeaponRange) dObject->shortestWeaponRange = r;
        }
    } else dObject->specialTime = 0;

    // if we don't have any weapon, then shortest range is 0 too
    if ( dObject->longestWeaponRange == 0) dObject->shortestWeaponRange = 0;
    if ( dObject->longestWeaponRange > kEngageRange)
        dObject->engageRange = dObject->longestWeaponRange;
    else
        dObject->engageRange = kEngageRange;

    // HANDLE THE NEW SPRITE DATA:
    if ( dObject->pixResID != kNoSpriteTable)
    {
        spriteTable = GetPixTable( dObject->pixResID);

        if (spriteTable == NULL) {
            throw Exception("Couldn't load a requested sprite");
            spriteTable = AddPixTable( dObject->pixResID);
        }

        dObject->sprite->table = spriteTable;
        dObject->sprite->tinySize = sObject->tinySize;
        dObject->sprite->whichLayer = sObject->pixLayer;
        dObject->sprite->scale = sObject->naturalScale;

        if ( dObject->attributes & kIsSelfAnimated)
        {
            dObject->sprite->whichShape = more_evil_fixed_to_long(dObject->frame.animation.thisShape);
        } else if ( dObject->attributes & kShapeFromDirection)
        {
            angle = dObject->direction;
            mAddAngle( angle, sObject->frame.rotation.rotRes >> 1);
            dObject->sprite->whichShape = angle / sObject->frame.rotation.rotRes;
        } else
        {
            dObject->sprite->whichShape = 0;
        }
    }

}

void AddActionToQueue( objectActionType *action, long actionNumber, long actionToDo,
                        long delayTime, spaceObjectType *subjectObject,
                        spaceObjectType *directObject, Point* offset)
{
    long                queueNumber = 0;
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

void ExecuteActionQueue( long unitsToDo)

{
//  actionQueueType     *actionQueue = gFirstActionQueue;
    actionQueueType     *actionQueue = gActionQueueData.get();
    long                        subjectid, directid, i;

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
            ExecuteObjectActions( gFirstActionQueue->actionNum, gFirstActionQueue->actionToDo,
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

void ExecuteObjectActions( long whichAction, long actionNum,
                spaceObjectType *sObject, spaceObjectType *dObject, Point* offset,
                bool allowDelay)

{

    spaceObjectType *anObject, *originalSObject = sObject, *originalDObject = dObject;
    baseObjectType  *baseObject;
    objectActionType    *action = gObjectActionData.get() + whichAction;
    short           end, angle;
    fixedPointType  fpoint, newVel;
    long            l;
    unsigned long   ul1;
    Fixed           f, f2;
    coordPointType  newLocation;
    Point           location;
    bool         OKtoExecute, checkConditions = false;
    Fixed           aFixed;
    unsigned char   tinyColor;

    if ( whichAction < 0) return;
    while (( action->verb != kNoAction) && ( actionNum > 0))
    {
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
            AddActionToQueue( action, whichAction, actionNum,
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
                )
        {
            if ( action->exclusiveFilter == 0xffffffff)
            {
                if (    (action->inclusiveFilter & kLevelKeyTagMask) ==
                            ( dObject->baseType->buildFlags & kLevelKeyTagMask)
                    )
                {
                    OKtoExecute = true;
                }
            } else if ( ( action->inclusiveFilter & dObject->attributes) == action->inclusiveFilter)
            {
                OKtoExecute = true;
            }

        }
/*
        if (    ( anObject == nil) ||
                (( action->exclusiveFilter == 0xffffffff) &&
                    ((action->inclusiveFilter & kLevelKeyTagMask) ==
                        ( anObject->baseType->buildFlags & kLevelKeyTagMask))) ||
                (((action->inclusiveFilter == 0) ||
                    ((action->inclusiveFilter & anObject->attributes) == action->inclusiveFilter))
                    && (( action->owner == 0) || (( action->owner == 1) &&
                    (anObject->owner == sObject->owner)) || (( action->owner == -1) && ( anObject->owner
                    != sObject->owner))) && ( anObject != nil))
            )
*/
        if ( OKtoExecute)
        {
            switch ( action->verb)
            {
                case kCreateObject:
                case kCreateObjectSetDest:
                    baseObject = mGetBaseObjectPtr( action->argument.createObject.whichBaseType);
                    end = action->argument.createObject.howManyMinimum;
                    if ( action->argument.createObject.howManyRange > 0)
                        end += RandomSeeded( action->argument.createObject.howManyRange,
                                &(anObject->randomSeed), 'soh9', action->argument.createObject.whichBaseType);
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
                            l += RandomSeeded( baseObject->initialDirectionRange,
                                        &(anObject->randomSeed));
                        */
                        newLocation = anObject->location;
                        if ( offset != NULL)
                        {
                            newLocation.h += offset->h;
                            newLocation.v += offset->v;
                        }

                        if ( action->argument.createObject.randomDistance > 0)
                        {
                            newLocation.h += RandomSeeded( action->argument.createObject.randomDistance << 1,
                                &(anObject->randomSeed), 'so10',
                                    action->argument.createObject.whichBaseType)
                                    - action->argument.createObject.randomDistance;
                            newLocation.v += RandomSeeded( action->argument.createObject.randomDistance << 1,
                                &(anObject->randomSeed), 'so11',
                                    action->argument.createObject.whichBaseType)
                                    - action->argument.createObject.randomDistance;
                        }

//                      l = CreateAnySpaceObject( action->argument.createObject.whichBaseType, &fpoint,
//                              &newLocation, l, anObject->owner, 0, nil, -1, -1, -1);
                        l = CreateAnySpaceObject( action->argument.createObject.whichBaseType, &fpoint,
                                &newLocation, l, anObject->owner, 0, -1);

                        if ( l >= 0)
                        {
                            spaceObjectType *newObject = gSpaceObjectData.get() + l;
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
                                        SetSpecialBeamAttributes( newObject,
                                        anObject);
                                }
                            }
                        }

                        end--;
                    }

                    break;

                case kPlaySound:
                    l = action->argument.playSound.volumeMinimum;
                    angle = action->argument.playSound.idMinimum;
                    if ( action->argument.playSound.idRange > 0)
                    {
                        angle += RandomSeeded(
                            action->argument.playSound.idRange + 1,
                            &(anObject->randomSeed),
                            'so33', -1);
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
                                                    RandomSeeded( action->argument.alterObject.range,
                                                    &(anObject->randomSeed), 'so13', anObject->whichBaseObject));
                                } else
                                {
                                    f = mMultiplyFixed( 2 /*kDefaultTurnRate*/,
                                                    action->argument.alterObject.minimum +
                                                    RandomSeeded( action->argument.alterObject.range,
                                                    &(anObject->randomSeed), 'so14', anObject->whichBaseObject));
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

                                anObject->frame.rotation.turnVelocity += RandomSeeded( f,
                                                &(anObject->randomSeed));
                                */
                            }
                            break;

                        case kAlterOffline:
                            f = action->argument.alterObject.minimum +
                                RandomSeeded( action->argument.alterObject.range, &(anObject->randomSeed), 'so15',
                                    anObject->whichBaseObject);
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
                                RandomSeeded( action->argument.alterObject.range, &(anObject->randomSeed),
                                    'so16', anObject->whichBaseObject);
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
                                    RandomSeeded( action->argument.alterObject.range,
                                        &(anObject->randomSeed), 'so17', anObject->whichBaseObject);

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
                            newLocation.h += RandomSeeded(
                                action->argument.alterObject.minimum <<
                                1,
                                &(anObject->randomSeed), 'so40', 0) -
                                action->argument.alterObject.minimum;
                            newLocation.v += RandomSeeded(
                                action->argument.alterObject.minimum <<
                                1,
                                &(anObject->randomSeed), 'so41', 0) -
                                action->argument.alterObject.minimum;
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
                    StartLongMessage(
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
                        SetStatusString(string, kStatusLabelColor);
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

        actionNum--;
        action++;
        whichAction++;
    }

    if ( checkConditions) CheckScenarioConditions( 0);
}

long CreateAnySpaceObject( long whichBase, fixedPointType *velocity,
            coordPointType *location, long direction, long owner,
            unsigned long specialAttributes, short spriteIDOverride)

{
    spaceObjectType *madeObject = NULL, newObject, *player = NULL;
    long            newObjectNumber;
    unsigned long   distance, dcalc, difference;
    uint64_t        hugeDistance;

    InitSpaceObjectFromBaseObject( &newObject, whichBase, RandomSeeded( 32766, &gRandomSeed, 'so18', whichBase),
                                    direction, velocity, owner, spriteIDOverride);
    newObject.location = *location;
    if ( globals()->gPlayerShipNumber >= 0)
        player = gSpaceObjectData.get() + globals()->gPlayerShipNumber;
    else player = NULL;
    if (( player != NULL) && ( player->active))
    {
        difference = ABS<int>( player->location.h - newObject.location.h);
        dcalc = difference;
        difference =  ABS<int>( player->location.v - newObject.location.v);
        distance = difference;
    } else
    {
        difference = ABS<int>( gGlobalCorner.h - newObject.location.h);
        dcalc = difference;
        difference =  ABS<int>( gGlobalCorner.v - newObject.location.v);
        distance = difference;
    }
    /*
    if (( dcalc > kMaximumRelevantDistance) ||
        ( distance > kMaximumRelevantDistance))
        distance = kMaximumRelevantDistance;
    else distance = distance * distance + dcalc * dcalc;
    */
    /*
    newObject.distanceFromPlayer = (double long)distance * (double long)distance +
                                    (double long)dcalc * (double long)dcalc;;
    */
    if (( newObject.attributes & kCanCollide) ||
                ( newObject.attributes & kCanBeHit) || ( newObject.attributes & kIsDestination) ||
                ( newObject.attributes & kCanThink) || ( newObject.attributes &
                kRemoteOrHuman))
    {
        if (( dcalc > kMaximumRelevantDistance) ||
            ( distance > kMaximumRelevantDistance))
        {
            hugeDistance = dcalc;    // must be positive
            MyWideMul(hugeDistance, hugeDistance, &hugeDistance);    // ppc automatically generates WideMultiply
            newObject.distanceFromPlayer = distance;
            MyWideMul(newObject.distanceFromPlayer, newObject.distanceFromPlayer, &newObject.distanceFromPlayer);
            newObject.distanceFromPlayer += hugeDistance;
        }
        else
        {
            newObject.distanceFromPlayer = distance * distance + dcalc * dcalc;
        }
    } else
    {
        newObject.distanceFromPlayer = 0;
        /*
        if (( dcalc > kMaximumRelevantDistance) || ( distance > kMaximumRelevantDistance))
            distance = kMaximumRelevantDistanceSquared;
        else distance = distance * distance + dcalc * dcalc;
        newObject.distanceFromPlayer.lo = distance;
        */
    }

    newObject.sprite = NULL;
    newObject.id = RandomSeeded( 16384, &gRandomSeed, 'so19', whichBase);

    if ( newObject.attributes & kCanTurn)
    {
    } else if ( newObject.attributes & kIsSelfAnimated)
    {
//      newObject.frame.animation.thisShape = 0; //direction;
//      newObject.frame.animation.frameDirection = 1;
    }else if ( newObject.attributes & kIsBeam)
    {
/*      newObject.frame.beam.lastGlobalLocation = *location;
        newObject.frame.beam.killMe = false;

        h = ( newObject.location.h - gGlobalCorner.h) * gAbsoluteScale;
        h >>= SHIFT_SCALE;
        newObject.frame.beam.thisLocation.left = h + CLIP_LEFT;
        h = (newObject.location.v - gGlobalCorner.v) * gAbsoluteScale;
        h >>= SHIFT_SCALE; //+ CLIP_TOP
        newObject.frame.beam.thisLocation.top = h;

        newObject.frame.beam.lastLocation.left = newObject.frame.beam.lastLocation.right =
                newObject.frame.beam.thisLocation.left;
        newObject.frame.beam.lastLocation.top = newObject.frame.beam.lastLocation.bottom =
                newObject.frame.beam.thisLocation.top;
*/  }

//  newObjectNumber = AddSpaceObject( &newObject, canBuildType,
//      nameResID, nameStrNum);
    newObjectNumber = AddSpaceObject( &newObject);
    if ( newObjectNumber == -1)
    {
        return ( -1);
    } else
    {
        madeObject = gSpaceObjectData.get() + newObjectNumber;
        madeObject->attributes |= specialAttributes;
        ExecuteObjectActions( madeObject->baseType->createAction, madeObject->baseType->createActionNum,
                            madeObject, NULL, NULL, true);
    }
    return( newObjectNumber);
}

long CountObjectsOfBaseType( long whichType, long owner)

{
    long    count, result = 0;

    spaceObjectType *anObject;

    anObject = gSpaceObjectData.get();
    for ( count = 0; count < kMaxSpaceObject; count++)
    {
        if (( anObject->active) &&
            (( anObject->whichBaseObject == whichType) || ( whichType == -1)) &&
            (( anObject->owner == owner) || ( owner == -1))) result++;
        anObject++;
    }
    return (result);
}

long GetNextObjectWithAttributes( long startWith, unsigned long attributes, bool exclude)

{
    long    original = startWith;
    spaceObjectType *anObject;

    anObject = gSpaceObjectData.get() + startWith;

    if ( exclude)
    {
        do
        {
            startWith = anObject->nextObjectNumber;
            anObject = anObject->nextObject;

            if ( anObject == NULL)
            {
                startWith = gRootObjectNumber;
                anObject = gRootObject;
            }
        } while (( anObject->attributes & attributes) && ( startWith != original));
        if (( startWith == original) && ( anObject->attributes & attributes)) return ( -1);
        else return( startWith);
    } else
    {
        do
        {
            startWith = anObject->nextObjectNumber;
            anObject = anObject->nextObject;

            if ( anObject == NULL)
            {
                startWith = gRootObjectNumber;
                anObject = gRootObject;
            }
        } while ((!( anObject->attributes & attributes)) && ( startWith != original));
        if (( startWith == original) && (!( anObject->attributes & attributes))) return ( -1);
        else return( startWith);
    }
}

void AlterObjectHealth( spaceObjectType *anObject, long health)

{
    if ( health <= 0)
        anObject->health += health;
    else
    {
        if ( anObject->health >= (2147483647 - health))
            anObject->health = 2147483647;
        else
            anObject->health += health;
    }
    if ( anObject->health < 0)
    {
        DestroyObject( anObject);
    }

}

void AlterObjectEnergy( spaceObjectType *anObject, long energy)

{
    anObject->energy += energy;
    if ( anObject->energy < 0)
    {
        anObject->energy = 0;
    } else if ( anObject->energy > anObject->baseType->energy)
    {
        anObject->battery += ( anObject->energy - anObject->baseType->energy);
        if ( anObject->battery > (anObject->baseType->energy * kBatteryToEnergyRatio))
        {
            PayAdmiral( anObject->owner, anObject->battery - (anObject->baseType->energy
                * kBatteryToEnergyRatio));
            anObject->battery = anObject->baseType->energy * kBatteryToEnergyRatio;
        }
        anObject->energy = anObject->baseType->energy;
    }

}

void AlterObjectBattery( spaceObjectType *anObject, long energy)

{
    anObject->battery += energy;
    if ( anObject->battery > (anObject->baseType->energy * kBatteryToEnergyRatio))
    {
        PayAdmiral( anObject->owner, anObject->battery - (anObject->baseType->energy
            * kBatteryToEnergyRatio));
        anObject->battery = anObject->baseType->energy * kBatteryToEnergyRatio;
    }
}


void AlterObjectOwner( spaceObjectType *anObject, long owner, bool message)

{
    spaceObjectType *fixObject = NULL;
    long            i, originalOwner = anObject->owner;
    RgbColor        tinyColor;
    unsigned char   tinyShade;

    if ( anObject->owner != owner)
    {
        // if the object is occupied by a human, eject him since he can't change sides
        if (( anObject->attributes & (kIsPlayerShip | kRemoteOrHuman)) &&
            (!(anObject->baseType->destroyActionNum & kDestroyActionDontDieFlag)))
        {
            CreateFloatingBodyOfPlayer( anObject);
        }

        anObject->owner = owner;

        if (( owner >= 0) && ( anObject->attributes & kIsDestination))
        {
            if ( GetAdmiralConsiderObject( owner) < 0)
                SetAdmiralConsiderObject( owner, anObject->entryNumber);

            if ( GetAdmiralBuildAtObject( owner) < 0)
            {
                if ( BaseHasSomethingToBuild( anObject->entryNumber))
                    SetAdmiralBuildAtObject( owner, anObject->entryNumber);
            }
            if ( GetAdmiralDestinationObject( owner) < 0)
                SetAdmiralDestinationObject( owner, anObject->entryNumber, kObjectDestinationType);

        }

        if ( anObject->attributes & kNeutralDeath)
            anObject->attributes = anObject->baseType->attributes;

        if (( anObject->sprite != NULL)/* && ( anObject->baseType->tinySize != 0)*/)
        {
            switch( anObject->sprite->whichLayer)
            {
                case kFirstSpriteLayer:
                    tinyShade = MEDIUM;
                    break;

                case kMiddleSpriteLayer:
                    tinyShade = LIGHT;
                    break;

                case kLastSpriteLayer:
                    tinyShade = VERY_LIGHT;
                    break;

                default:
                    tinyShade = DARK;
                    break;
            }

            if ( owner == globals()->gPlayerAdmiralNumber)
            {
                tinyColor = GetRGBTranslateColorShade(kFriendlyColor, tinyShade);
            } else if ( owner <= kNoOwner)
            {
                tinyColor = GetRGBTranslateColorShade(kNeutralColor, tinyShade);
            } else
            {
                tinyColor = GetRGBTranslateColorShade(kHostileColor, tinyShade);
            }
            anObject->tinyColor = anObject->sprite->tinyColor = tinyColor;

            if ( anObject->attributes & kCanThink)
            {
                NatePixTable* pixTable;

                if (( anObject->pixResID == anObject->baseType->pixResID) ||
                    ( anObject->pixResID == (anObject->baseType->pixResID |
                    (GetAdmiralColor( originalOwner)
                    << kSpriteTableColorShift))))
                {

                    anObject->pixResID =
                        anObject->baseType->pixResID | (GetAdmiralColor( owner)
                        << kSpriteTableColorShift);

                    pixTable = GetPixTable( anObject->pixResID);
                    if (pixTable != NULL) {
                        anObject->sprite->table = pixTable;
                    }
                }
            }
        }

        anObject->remoteFoeStrength = anObject->remoteFriendStrength = anObject->escortStrength =
            anObject->localFoeStrength = anObject->localFriendStrength = 0;
        anObject->bestConsideredTargetValue = anObject->currentTargetValue = 0xffffffff;
        anObject->bestConsideredTargetNumber = -1;

        fixObject = gSpaceObjectData.get();
        for ( i = 0; i < kMaxSpaceObject; i++)
        {
            if (( fixObject->destinationObject == anObject->entryNumber) && ( fixObject->active !=
                kObjectAvailable) && ( fixObject->attributes & kCanThink))
            {
                fixObject->currentTargetValue = 0xffffffff;
                if ( fixObject->owner != owner)
                {
                    anObject->remoteFoeStrength += fixObject->baseType->offenseValue;
                } else
                {
                    anObject->remoteFriendStrength += fixObject->baseType->offenseValue;
                    anObject->escortStrength += fixObject->baseType->offenseValue;
                }
            }
            fixObject++;
        }

        if ( anObject->attributes & kIsDestination)
        {
            if (( anObject->attributes & kNeutralDeath)/* && ( owner >= 0)*/)
            {
                ClearAllOccupants( anObject->destinationObject, owner, anObject->baseType->initialAgeRange);
            }
            StopBuilding( anObject->destinationObject);
            if (message) {
                String destination_name(GetDestBalanceName(anObject->destinationObject));
                if (owner >= 0) {
                    String new_owner_name(GetAdmiralName(anObject->owner));
                    AddMessage(format("{0} captured by {1}.", destination_name, new_owner_name));
                } else if (originalOwner >= 0) { // must be since can't both be -1
                    String old_owner_name(GetAdmiralName(originalOwner));
                    AddMessage(format("{0} lost by {1}.", destination_name, old_owner_name));
                }
            }
        } else {
            if (message) {
                StringList strings(kSpaceObjectNameResID);
                StringSlice object_name = strings.at(anObject->whichBaseObject);
                if (owner >= 0) {
                    String new_owner_name(GetAdmiralName(anObject->owner));
                    AddMessage(format("{0} captured by {1}.", object_name, new_owner_name));
                } else if (originalOwner >= 0) { // must be since can't both be -1
                    String old_owner_name(GetAdmiralName(originalOwner));
                    AddMessage(format("{0} lost by {1}.", object_name, old_owner_name));
                }
            }
        }
        if ( anObject->attributes & kIsDestination)
            RecalcAllAdmiralBuildData();
    }
//  if ( anObject->attributes & kIsEndgameObject) CheckEndgame();
}

void AlterObjectOccupation( spaceObjectType *anObject, long owner, long howMuch, bool message)
{
    if ( ( anObject->active) && ( anObject->attributes & kIsDestination) && ( anObject->attributes & kNeutralDeath))
    {
        if ( AlterDestinationObjectOccupation( anObject->destinationObject, owner, howMuch) >= anObject->baseType->initialAgeRange)
        {
            AlterObjectOwner( anObject, owner, message);
        }
    }
}

void AlterObjectCloakState( spaceObjectType *anObject, bool cloak)
{
    long            longscrap = kMaxSoundVolume;

    if ( (cloak) && ( anObject->cloakState == 0))
    {
        anObject->cloakState = 1;
        mPlayDistanceSound(longscrap, anObject, kCloakOn, kMediumPersistence, kPrioritySound);

    } else if (((!(cloak)) || ( anObject->attributes & kRemoteOrHuman)) &&
            ( anObject->cloakState >= 250))
    {
        anObject->cloakState = kCloakOffStateMax;
        mPlayDistanceSound(longscrap, anObject, kCloakOff, kMediumPersistence, kPrioritySound);
    }
}

void DestroyObject( spaceObjectType *anObject)

{
    short   energyNum, i;
    spaceObjectType *fixObject;

    if ( anObject->active == kObjectInUse)
    {
        if ( anObject->attributes & kNeutralDeath)
        {
            anObject->health = anObject->baseType->health;
            // if anyone is targeting it, they should stop
            fixObject = gSpaceObjectData.get();
            for ( i = 0; i < kMaxSpaceObject; i++)
            {
                if (( fixObject->attributes & kCanAcceptDestination) && ( fixObject->active !=
                    kObjectAvailable))
                {
                    if ( fixObject->targetObjectNumber == anObject->entryNumber)
                    {
                        fixObject->targetObjectNumber = kNoDestinationObject;
                    }
                }
                fixObject++;
            }

            AlterObjectOwner( anObject, -1, true);
            anObject->attributes &= ~(kHated | kCanEngage | kCanCollide | kCanBeHit);
            ExecuteObjectActions( anObject->baseType->destroyAction,
                    anObject->baseType->destroyActionNum & kDestroyActionNotMask,
                    anObject, NULL, NULL, true);
        } else
        {
            AddKillToAdmiral( anObject);
            if ( anObject->attributes & kReleaseEnergyOnDeath)
            {
                energyNum = anObject->energy / kEnergyPodAmount;
                while ( energyNum > 0)
                {

//                  CreateAnySpaceObject( globals()->scenarioFileInfo.energyBlobID, &(anObject->velocity),
//                      &(anObject->location), anObject->direction, kNoOwner, 0, nil, -1, -1, -1);
                    CreateAnySpaceObject( globals()->scenarioFileInfo.energyBlobID, &(anObject->velocity),
                        &(anObject->location), anObject->direction, kNoOwner, 0, -1);
                    energyNum--;
                }
            }

            // if it's a destination, we keep anyone from thinking they have it as a destination
            // (all at once since this should be very rare)
            if (( anObject->attributes & kIsDestination) &&
                (!(anObject->baseType->destroyActionNum & kDestroyActionDontDieFlag)))
            {
                RemoveDestination( anObject->destinationObject);
                fixObject = gSpaceObjectData.get();
                for ( i = 0; i < kMaxSpaceObject; i++)
                {
                    if (( fixObject->attributes & kCanAcceptDestination) && ( fixObject->active !=
                        kObjectAvailable))
                    {
                        if ( fixObject->destinationObject == anObject->entryNumber)
                        {
                            fixObject->destinationObject = kNoDestinationObject;
                            fixObject->destObjectPtr = NULL;
                            fixObject->attributes &= ~kStaticDestination;
                        }
                    }
                    fixObject++;
                }
            }

            ExecuteObjectActions( anObject->baseType->destroyAction,
                    anObject->baseType->destroyActionNum & kDestroyActionNotMask,
                    anObject, NULL, NULL, true);

            if ( anObject->attributes & kCanAcceptDestination) RemoveObjectFromDestination( anObject);
            if (!(anObject->baseType->destroyActionNum & kDestroyActionDontDieFlag))
                anObject->active = kObjectToBeFreed;

    //      if ( anObject->attributes & kIsEndgameObject)
    //          CheckEndgame();
        }
    } else
    {
    }
}


void ActivateObjectSpecial( spaceObjectType *anObject)
{
    baseObjectType  *weaponObject, *baseObject = anObject->baseType;
    Point           offset;
    short           h;
    Fixed           fcos, fsin;

    if (( anObject->specialTime <= 0) && ( anObject->specialType != kNoWeapon))
    {
        weaponObject = anObject->specialBase;
        if ( (anObject->energy >= weaponObject->frame.weapon.energyCost) &&
            (( weaponObject->frame.weapon.ammo < 0) || ( anObject->specialAmmo > 0)))
        {
            anObject->energy -= weaponObject->frame.weapon.energyCost;
            anObject->specialPosition++;
            if ( anObject->specialPosition >= baseObject->specialPositionNum) anObject->specialPosition = 0;

            h = anObject->direction;
            mAddAngle( h, -90);
            GetRotPoint(&fcos, &fsin, h);
            fcos = -fcos;
            fsin = -fsin;

            offset.h = mMultiplyFixed( baseObject->specialPosition[anObject->specialPosition].h, fcos);
            offset.h -= mMultiplyFixed( baseObject->specialPosition[anObject->specialPosition].v, fsin);
            offset.v = mMultiplyFixed( baseObject->specialPosition[anObject->specialPosition].h, fsin);
            offset.v += mMultiplyFixed( baseObject->specialPosition[anObject->specialPosition].v, fcos);
            offset.h = mFixedToLong( offset.h);
            offset.v = mFixedToLong( offset.v);

            anObject->specialTime = weaponObject->frame.weapon.fireTime;
            if ( weaponObject->frame.weapon.ammo > 0) anObject->specialAmmo--;
            ExecuteObjectActions( weaponObject->activateAction,
                                weaponObject->activateActionNum, anObject, NULL, NULL, true);
        }
    }
}

void CreateFloatingBodyOfPlayer( spaceObjectType *anObject)

{
    long        count;

//  count = CreateAnySpaceObject( globals()->scenarioFileInfo.playerBodyID, &(anObject->velocity),
//      &(anObject->location), anObject->direction, anObject->owner, 0, nil, -1, -1, -1);

    // if we're already in a body, don't create a body from it
    // a body expiring is handled elsewhere
    if ( anObject->whichBaseObject == globals()->scenarioFileInfo.playerBodyID) return;

    count = CreateAnySpaceObject( globals()->scenarioFileInfo.playerBodyID, &(anObject->velocity),
        &(anObject->location), anObject->direction, anObject->owner, 0, -1);
    if ( count >= 0)
    {
/*      if (( anObject->owner == globals()->gPlayerAdmiralNumber) && ( anObject->attributes & kIsHumanControlled))
        {
            attributes = anObject->attributes & ( kIsHumanControlled | kIsPlayerShip);
            anObject->attributes &= (~kIsHumanControlled) & (~kIsPlayerShip);
            globals()->gPlayerShipNumber = count;
            ResetScrollStars( globals()->gPlayerShipNumber);
            anObject = gSpaceObjectData.get() + globals()->gPlayerShipNumber;
            anObject->attributes |= attributes;
        } else
        {
            attributes = anObject->attributes & kIsRemote;
            anObject->attributes &= ~kIsRemote;
            anObject = gSpaceObjectData.get() + count;
            anObject->attributes |= attributes;
        }
        SetAdmiralFlagship( anObject->owner, count);
*/
        ChangePlayerShipNumber( anObject->owner, count);

    } else
    {
        PlayerShipBodyExpire( anObject, true);
    }
}

}  // namespace antares

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

#include <set>
#include <sfz/sfz.hpp>

#include "data/resource.hpp"
#include "data/space-object.hpp"
#include "data/string-list.hpp"
#include "drawing/color.hpp"
#include "drawing/sprite-handling.hpp"
#include "game/action.hpp"
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
using std::set;
using std::unique_ptr;

namespace antares {

const uint8_t kFriendlyColor        = GREEN;
const uint8_t kHostileColor         = RED;
const uint8_t kNeutralColor         = SKY_BLUE;

const int32_t kBatteryToEnergyRatio = 5;

static const int16_t kSpaceObjectNameResID          = 5000;
static const int16_t kSpaceObjectShortNameResID     = 5001;
static StringList* space_object_names;
static StringList* space_object_short_names;

spaceObjectType* gRootObject = NULL;
int32_t gRootObjectNumber = -1;

static unique_ptr<spaceObjectType[]> gSpaceObjectData;
static unique_ptr<baseObjectType[]> gBaseObjectData;
static unique_ptr<objectActionType[]> gObjectActionData;

#ifdef DATA_COVERAGE
set<int32_t> covered_objects;
#endif  // DATA_COVERAGE

void SpaceObjectHandlingInit() {
    {
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

    gSpaceObjectData.reset(new spaceObjectType[kMaxSpaceObject]);
    {
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
    }

    CorrectAllBaseObjectColor();
    ResetAllSpaceObjects();
    reset_action_queue();

    space_object_names = new StringList(kSpaceObjectNameResID);
    space_object_short_names = new StringList(kSpaceObjectShortNameResID);
}

void ResetAllSpaceObjects() {
    spaceObjectType *anObject = NULL;
    int16_t         i;

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

baseObjectType* mGetBaseObjectPtr(int32_t whichObject) {
    if (whichObject >= 0) {
        return gBaseObjectData.get() + whichObject;
    }
    return nullptr;
}

spaceObjectType* mGetSpaceObjectPtr(int32_t whichObject) {
    if (whichObject >= 0) {
        return gSpaceObjectData.get() + whichObject;
    }
    return nullptr;
}

objectActionType* mGetObjectActionPtr(int32_t whichAction) {
    if (whichAction >= 0) {
        return gObjectActionData.get() + whichAction;
    }
    return nullptr;
}

void mGetBaseObjectFromClassRace(
        baseObjectType*& mbaseObject, int32_t& mcount, int mbaseClass, int mbaseRace) {
    mcount = 0;
    if ( mbaseClass >= kLiteralClass)
    {
        mcount = mbaseClass - kLiteralClass;
        mbaseObject = mGetBaseObjectPtr(mcount);
    }
    else
    {
        mbaseObject = mGetBaseObjectPtr( 0);
        while (( mcount < globals()->maxBaseObject) && (( mbaseObject->baseClass != mbaseClass) || ( mbaseObject->baseRace != mbaseRace)))
        {
            mcount++;
            mbaseObject++;
        }
        if ( mcount >= globals()->maxBaseObject) mbaseObject = NULL;
    }
}

int AddSpaceObject( spaceObjectType *sourceObject)

{
    spaceObjectType *destObject = NULL;
    int             whichObject = 0;
    NatePixTable* spriteTable = NULL;
    Point           where;
    int32_t         scaleCalc;
    RgbColor        tinyColor;
    uint8_t         tinyShade;
    int16_t         whichShape = 0, angle;

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
            tinyColor = RgbColor::kClear;
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
        destObject->frame.beam.beam = Beams::add( &(destObject->location),
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
    destObject->whichLabel = Labels::kNone;
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

int AddNumberedSpaceObject( spaceObjectType *sourceObject, int32_t whichObject)

{
#pragma unused( sourceObject, whichObject)
/*  spaceObjectType *destObject = nil;
    Handle          spriteTable = nil;
    Point           where;
    spritePix       oldStyleSprite;
    int32_t         scaleCalc;

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
    int16_t         i;

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

static void InitSpaceObjectFromBaseObject(
        spaceObjectType *dObject, int32_t  whichBaseObject, Random seed,
        int32_t direction, fixedPointType *velocity, int32_t owner, int16_t spriteIDOverride) {
    baseObjectType  *sObject = mGetBaseObjectPtr( whichBaseObject), *weaponBase = NULL;
    int16_t         i;
    int32_t         r;
    Fixed           f;
    fixedPointType  newVel;
    int32_t         l;

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
        dObject->id = dObject->randomSeed.next(32768);
    } while ( dObject->id == -1);

    dObject->distanceGrid.h = dObject->distanceGrid.v = dObject->collisionGrid.h = dObject->collisionGrid.v = 0;
    if (sObject->activatePeriod) {
        dObject->periodicTime =
            sObject->activatePeriod + dObject->randomSeed.next(sObject->activatePeriodRange);
    } else dObject->periodicTime = 0;

    r = sObject->initialDirection;
    mAddAngle( r, direction);
    if ( sObject->initialDirectionRange > 0)
    {
        i = dObject->randomSeed.next(sObject->initialDirectionRange);
        mAddAngle( r, i);
    }
    dObject->direction = r;

    f = sObject->initialVelocity;
    if ( sObject->initialVelocityRange > 0)
    {
        f += dObject->randomSeed.next(sObject->initialVelocityRange);
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
    dObject->rechargeTime = dObject->pulse.charge = dObject->beam.charge = dObject->special.charge = 0;
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
            l = dObject->randomSeed.next(sObject->frame.animation.frameShapeRange);
            dObject->frame.animation.thisShape += l;
        }
        dObject->frame.animation.frameDirection =
            sObject->frame.animation.frameDirection;
        if ( sObject->frame.animation.frameDirectionRange == -1)
        {
            if (dObject->randomSeed.next(2) == 1) {
                dObject->frame.animation.frameDirection = 1;
            }
        } else if ( sObject->frame.animation.frameDirectionRange > 0)
        {
            dObject->frame.animation.frameDirection += dObject->randomSeed.next(
                sObject->frame.animation.frameDirectionRange);
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
        dObject->age = sObject->initialAge + dObject->randomSeed.next(sObject->initialAgeRange);
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

    dObject->pulse.type = sObject->pulse.base;
    if ( dObject->pulse.type != kNoWeapon)
        dObject->pulse.base = mGetBaseObjectPtr( dObject->pulse.type);
    else dObject->pulse.base = NULL;
    dObject->beam.type = sObject->beam.base;
    if ( dObject->beam.type != kNoWeapon)
        dObject->beam.base = mGetBaseObjectPtr( dObject->beam.type);
    else dObject->beam.base = NULL;
    dObject->special.type = sObject->special.base;
    if ( dObject->special.type != kNoWeapon)
        dObject->special.base = mGetBaseObjectPtr( dObject->special.type);
    else dObject->special.base = NULL;
    dObject->longestWeaponRange = 0;
    dObject->shortestWeaponRange = kMaximumRelevantDistance;

    if ( dObject->pulse.type != kNoWeapon)
    {
        weaponBase = dObject->pulse.base;
        dObject->pulse.ammo = weaponBase->frame.weapon.ammo;
        dObject->pulse.time = dObject->pulse.position = 0;
        r = weaponBase->frame.weapon.range;
        if (( r > 0) && ( weaponBase->frame.weapon.usage & kUseForAttacking))
        {
            if ( r > dObject->longestWeaponRange) dObject->longestWeaponRange = r;
            if ( r < dObject->shortestWeaponRange) dObject->shortestWeaponRange = r;
        }
    } else dObject->pulse.time = 0;

    if ( dObject->beam.type != kNoWeapon)
    {
        weaponBase = dObject->beam.base;
        dObject->beam.ammo = weaponBase->frame.weapon.ammo;
        dObject->beam.time = dObject->beam.position = 0;
        r = weaponBase->frame.weapon.range;
        if (( r > 0) && ( weaponBase->frame.weapon.usage & kUseForAttacking))
        {
            if ( r > dObject->longestWeaponRange) dObject->longestWeaponRange = r;
            if ( r < dObject->shortestWeaponRange) dObject->shortestWeaponRange = r;
        }
    } else dObject->beam.time = 0;

    if ( dObject->special.type != kNoWeapon)
    {
        weaponBase = dObject->special.base;
        dObject->special.ammo = weaponBase->frame.weapon.ammo;
        dObject->special.time = dObject->special.position = 0;
        r = weaponBase->frame.weapon.range;
        if (( r > 0) && ( weaponBase->frame.weapon.usage & kUseForAttacking))
        {
            if ( r > dObject->longestWeaponRange) dObject->longestWeaponRange = r;
            if ( r < dObject->shortestWeaponRange) dObject->shortestWeaponRange = r;
        }
    } else dObject->special.time = 0;

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

void ChangeObjectBaseType( spaceObjectType *dObject, int32_t whichBaseObject,
    int32_t spriteIDOverride, bool relative)

{
    baseObjectType  *sObject = mGetBaseObjectPtr( whichBaseObject), *weaponBase = NULL;
    int16_t         angle;
    int32_t         r;
    NatePixTable* spriteTable;

#ifdef DATA_COVERAGE
    covered_objects.insert(whichBaseObject);
    for (int32_t weapon: {sObject->pulse.base, sObject->beam.base, sObject->special.base}) {
        if (weapon != kNoWeapon) {
            covered_objects.insert(weapon);
        }
    }
#endif  // DATA_COVERAGE

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
            r = dObject->randomSeed.next(sObject->frame.animation.frameShapeRange);
            dObject->frame.animation.thisShape += r;
        }
        dObject->frame.animation.frameDirection =
            sObject->frame.animation.frameDirection;
        if ( sObject->frame.animation.frameDirectionRange == -1)
        {
            if (dObject->randomSeed.next(2) == 1) {
                dObject->frame.animation.frameDirection = 1;
            }
        } else if ( sObject->frame.animation.frameDirectionRange > 0)
        {
            dObject->frame.animation.frameDirection += dObject->randomSeed.next(
                sObject->frame.animation.frameDirectionRange);
        }
        dObject->frame.animation.frameFraction = 0;
        dObject->frame.animation.frameSpeed = sObject->frame.animation.frameSpeed;
    } else if ( dObject->attributes & kIsBeam)
    {
//      dObject->frame.beam.killMe = false;
    }

    dObject->maxVelocity = sObject->maxVelocity;

    dObject->age = sObject->initialAge + dObject->randomSeed.next(sObject->initialAgeRange);

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

    dObject->pulse.type = sObject->pulse.base;
    if ( dObject->pulse.type != kNoWeapon)
        dObject->pulse.base = mGetBaseObjectPtr( dObject->pulse.type);
    else dObject->pulse.base = NULL;
    dObject->beam.type = sObject->beam.base;
    if ( dObject->beam.type != kNoWeapon)
        dObject->beam.base = mGetBaseObjectPtr( dObject->beam.type);
    else dObject->beam.base = NULL;
    dObject->special.type = sObject->special.base;
    if ( dObject->special.type != kNoWeapon)
        dObject->special.base = mGetBaseObjectPtr( dObject->special.type);
    else dObject->special.base = NULL;
    dObject->longestWeaponRange = 0;
    dObject->shortestWeaponRange = kMaximumRelevantDistance;

    // check periodic time
    if (sObject->activatePeriod) {
        dObject->periodicTime =
            sObject->activatePeriod + dObject->randomSeed.next(sObject->activatePeriodRange);
    } else dObject->periodicTime = 0;

    if ( dObject->pulse.type != kNoWeapon)
    {
        weaponBase = dObject->pulse.base;
        if ( !relative)
        {
            dObject->pulse.ammo = weaponBase->frame.weapon.ammo;
            dObject->pulse.position = 0;
            if ( dObject->pulse.time < 0)
                dObject->pulse.time = 0;
            else if ( dObject->pulse.time > weaponBase->frame.weapon.fireTime)
                dObject->pulse.time = weaponBase->frame.weapon.fireTime;
        }
        r = weaponBase->frame.weapon.range;
        if (( r > 0) && ( weaponBase->frame.weapon.usage & kUseForAttacking))
        {
            if ( r > dObject->longestWeaponRange) dObject->longestWeaponRange = r;
            if ( r < dObject->shortestWeaponRange) dObject->shortestWeaponRange = r;
        }
    } else dObject->pulse.time = 0;

    if ( dObject->beam.type != kNoWeapon)
    {
        weaponBase = dObject->beam.base;
        if ( !relative)
        {
            dObject->beam.ammo = weaponBase->frame.weapon.ammo;
            if ( dObject->beam.time < 0)
                dObject->beam.time = 0;
            else if ( dObject->beam.time > weaponBase->frame.weapon.fireTime)
                dObject->beam.time = weaponBase->frame.weapon.fireTime;
            dObject->beam.position = 0;
        }
        r = weaponBase->frame.weapon.range;
        if (( r > 0) && ( weaponBase->frame.weapon.usage & kUseForAttacking))
        {
            if ( r > dObject->longestWeaponRange) dObject->longestWeaponRange = r;
            if ( r < dObject->shortestWeaponRange) dObject->shortestWeaponRange = r;
        }
    } else dObject->beam.time = 0;

    if ( dObject->special.type != kNoWeapon)
    {
        weaponBase = dObject->special.base;
        if ( !relative)
        {
            dObject->special.ammo = weaponBase->frame.weapon.ammo;
            dObject->special.position = 0;
            if ( dObject->special.time < 0)
                dObject->special.time = 0;
            else if ( dObject->special.time > weaponBase->frame.weapon.fireTime)
                dObject->special.time = weaponBase->frame.weapon.fireTime;
        }
        r = weaponBase->frame.weapon.range;
        if (( r > 0) && ( weaponBase->frame.weapon.usage & kUseForAttacking))
        {
            if ( r > dObject->longestWeaponRange) dObject->longestWeaponRange = r;
            if ( r < dObject->shortestWeaponRange) dObject->shortestWeaponRange = r;
        }
    } else dObject->special.time = 0;

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

int32_t CreateAnySpaceObject(
        int32_t whichBase, fixedPointType *velocity, coordPointType *location, int32_t direction,
        int32_t owner, uint32_t specialAttributes, int16_t spriteIDOverride) {
    spaceObjectType newObject;
    InitSpaceObjectFromBaseObject(
            &newObject, whichBase, {gRandomSeed.next(32766)}, direction, velocity, owner,
            spriteIDOverride);
    newObject.location = *location;
    spaceObjectType* player = nullptr;
    if (globals()->gPlayerShipNumber >= 0) {
        player = gSpaceObjectData.get() + globals()->gPlayerShipNumber;
    }
    uint32_t distance, dcalc, difference;
    uint64_t hugeDistance;
    if (player && (player->active)) {
        difference = ABS<int>(player->location.h - newObject.location.h);
        dcalc = difference;
        difference =  ABS<int>(player->location.v - newObject.location.v);
        distance = difference;
    } else {
        difference = ABS<int>(gGlobalCorner.h - newObject.location.h);
        dcalc = difference;
        difference =  ABS<int>(gGlobalCorner.v - newObject.location.v);
        distance = difference;
    }
    if ((newObject.attributes & kCanCollide)
            || (newObject.attributes & kCanBeHit)
            || (newObject.attributes & kIsDestination)
            || (newObject.attributes & kCanThink)
            || (newObject.attributes & kRemoteOrHuman)) {
        if ((dcalc > kMaximumRelevantDistance)
                || (distance > kMaximumRelevantDistance)) {
            hugeDistance = dcalc;    // must be positive
            MyWideMul(hugeDistance, hugeDistance, &hugeDistance);    // ppc automatically generates WideMultiply
            newObject.distanceFromPlayer = distance;
            MyWideMul(newObject.distanceFromPlayer, newObject.distanceFromPlayer, &newObject.distanceFromPlayer);
            newObject.distanceFromPlayer += hugeDistance;
        } else {
            newObject.distanceFromPlayer = distance * distance + dcalc * dcalc;
        }
    } else {
        newObject.distanceFromPlayer = 0;
    }

    newObject.sprite = NULL;
    newObject.id = gRandomSeed.next(16384);

    int32_t newObjectNumber = AddSpaceObject(&newObject);
    if (newObjectNumber == -1) {
        return -1;
    }

#ifdef DATA_COVERAGE
    covered_objects.insert(whichBase);
    auto* base = mGetBaseObjectPtr(whichBase);
    for (int32_t weapon: {base->pulse.base, base->beam.base, base->special.base}) {
        if (weapon != kNoWeapon) {
            covered_objects.insert(weapon);
        }
    }
#endif  // DATA_COVERAGE

    spaceObjectType* madeObject = gSpaceObjectData.get() + newObjectNumber;
    madeObject->attributes |= specialAttributes;
    execute_actions(madeObject->baseType->create, madeObject, NULL, NULL, true);
    return newObjectNumber;
}

int32_t CountObjectsOfBaseType( int32_t whichType, int32_t owner)

{
    int32_t count, result = 0;

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

int32_t GetNextObjectWithAttributes( int32_t startWith, uint32_t attributes, bool exclude)

{
    int32_t original = startWith;
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

void AlterObjectHealth(spaceObjectType* object, int32_t health) {
    if (health <= 0) {
        object->health += health;
    } else {
        if (object->health >= (2147483647 - health)) {
            object->health = 2147483647;
        } else {
            object->health += health;
        }
    }
    if (object->health < 0) {
        DestroyObject(object);
    }
}

void AlterObjectEnergy(spaceObjectType* object, int32_t energy) {
    object->energy += energy;
    if (object->energy < 0) {
        object->energy = 0;
    } else if (object->energy > object->baseType->energy) {
        AlterObjectBattery(object, object->energy - object->baseType->energy);
        object->energy = object->baseType->energy;
    }
}

void AlterObjectBattery(spaceObjectType* object, int32_t energy) {
    object->battery += energy;
    if (object->battery > (object->baseType->energy * kBatteryToEnergyRatio)) {
        PayAdmiral(
                object->owner,
                object->battery - (object->baseType->energy * kBatteryToEnergyRatio));
        object->battery = object->baseType->energy * kBatteryToEnergyRatio;
    }
}


void AlterObjectOwner(spaceObjectType* object, int32_t owner, bool message) {
    if (object->owner == owner) {
        return;
    }

    // if the object is occupied by a human, eject him since he can't change sides
    if ((object->attributes & (kIsPlayerShip | kRemoteOrHuman))
            && !object->baseType->destroyDontDie) {
        CreateFloatingBodyOfPlayer(object);
    }

    int32_t old_owner = object->owner;
    object->owner = owner;

    if ((owner >= 0) && (object->attributes & kIsDestination)) {
        if (GetAdmiralConsiderObject(owner) < 0) {
            SetAdmiralConsiderObject(owner, object->entryNumber);
        }

        if (GetAdmiralBuildAtObject(owner) < 0) {
            if (BaseHasSomethingToBuild(object->entryNumber)) {
                SetAdmiralBuildAtObject(owner, object->entryNumber);
            }
        }
        if (GetAdmiralDestinationObject(owner) < 0) {
            SetAdmiralDestinationObject(owner, object->entryNumber, kObjectDestinationType);
        }
    }

    if (object->attributes & kNeutralDeath) {
        object->attributes = object->baseType->attributes;
    }

    if (object->sprite != NULL) {
        uint8_t tinyShade;
        switch (object->sprite->whichLayer) {
          case kFirstSpriteLayer:   tinyShade = MEDIUM; break;
          case kMiddleSpriteLayer:  tinyShade = LIGHT; break;
          case kLastSpriteLayer:    tinyShade = VERY_LIGHT; break;
          default:                  tinyShade = DARK; break;
        }

        RgbColor tinyColor;
        if (owner == globals()->gPlayerAdmiralNumber) {
            tinyColor = GetRGBTranslateColorShade(kFriendlyColor, tinyShade);
        } else if (owner <= kNoOwner) {
            tinyColor = GetRGBTranslateColorShade(kNeutralColor, tinyShade);
        } else {
            tinyColor = GetRGBTranslateColorShade(kHostileColor, tinyShade);
        }
        object->tinyColor = object->sprite->tinyColor = tinyColor;

        if (object->attributes & kCanThink) {
            NatePixTable* pixTable;

            if ((object->pixResID == object->baseType->pixResID)
                    || (object->pixResID == (object->baseType->pixResID |
                                             (GetAdmiralColor(old_owner)
                                              << kSpriteTableColorShift)))) {
                object->pixResID =
                    object->baseType->pixResID | (GetAdmiralColor(owner)
                            << kSpriteTableColorShift);

                pixTable = GetPixTable(object->pixResID);
                if (pixTable != NULL) {
                    object->sprite->table = pixTable;
                }
            }
        }
    }

    object->remoteFoeStrength = object->remoteFriendStrength = object->escortStrength =
        object->localFoeStrength = object->localFriendStrength = 0;
    object->bestConsideredTargetValue = object->currentTargetValue = 0xffffffff;
    object->bestConsideredTargetNumber = -1;

    for (int32_t i = 0; i < kMaxSpaceObject; i++) {
        auto& fixObject = gSpaceObjectData[i];
        if ((fixObject.destinationObject == object->entryNumber)
                && (fixObject.active != kObjectAvailable)
                && (fixObject.attributes & kCanThink)) {
            fixObject.currentTargetValue = 0xffffffff;
            if (fixObject.owner != owner) {
                object->remoteFoeStrength += fixObject.baseType->offenseValue;
            } else {
                object->remoteFriendStrength += fixObject.baseType->offenseValue;
                object->escortStrength += fixObject.baseType->offenseValue;
            }
        }
    }

    if (object->attributes & kIsDestination) {
        if (object->attributes & kNeutralDeath) {
            ClearAllOccupants(
                    object->destinationObject, owner, object->baseType->initialAgeRange);
        }
        StopBuilding(object->destinationObject);
        if (message) {
            String destination_name(GetDestBalanceName(object->destinationObject));
            if (owner >= 0) {
                String new_owner_name(GetAdmiralName(object->owner));
                Messages::add(format("{0} captured by {1}.", destination_name, new_owner_name));
            } else if (old_owner >= 0) { // must be since can't both be -1
                String old_owner_name(GetAdmiralName(old_owner));
                Messages::add(format("{0} lost by {1}.", destination_name, old_owner_name));
            }
        }
        RecalcAllAdmiralBuildData();
    } else {
        if (message) {
            StringSlice object_name = get_object_name(object->whichBaseObject);
            if (owner >= 0) {
                String new_owner_name(GetAdmiralName(object->owner));
                Messages::add(format("{0} captured by {1}.", object_name, new_owner_name));
            } else if (old_owner >= 0) { // must be since can't both be -1
                String old_owner_name(GetAdmiralName(old_owner));
                Messages::add(format("{0} lost by {1}.", object_name, old_owner_name));
            }
        }
    }
}

void AlterObjectOccupation(
        spaceObjectType* object, int32_t owner, int32_t howMuch, bool message) {
    if (object->active
            && (object->attributes & kIsDestination)
            && (object->attributes & kNeutralDeath)) {
        if (AlterDestinationObjectOccupation(object->destinationObject, owner, howMuch)
                >= object->baseType->initialAgeRange) {
            AlterObjectOwner(object, owner, message);
        }
    }
}

void AlterObjectCloakState(spaceObjectType* object, bool cloak) {
    if (cloak && (object->cloakState == 0)) {
        object->cloakState = 1;
        mPlayDistanceSound(kMaxSoundVolume, object, kCloakOn, kMediumPersistence, kPrioritySound);
    } else if ((!cloak || (object->attributes & kRemoteOrHuman))
            && (object->cloakState >= 250)) {
        object->cloakState = kCloakOffStateMax;
        mPlayDistanceSound(kMaxSoundVolume, object, kCloakOff, kMediumPersistence, kPrioritySound);
    }
}

void DestroyObject(spaceObjectType* object) {
    if (object->active != kObjectInUse) {
        return;
    } else if (object->attributes & kNeutralDeath) {
        object->health = object->baseType->health;
        // if anyone is targeting it, they should stop
        for (int16_t i = 0; i < kMaxSpaceObject; i++) {
            auto& fixObject = gSpaceObjectData[i];
            if ((fixObject.attributes & kCanAcceptDestination)
                    && (fixObject.active != kObjectAvailable)) {
                if (fixObject.targetObjectNumber == object->entryNumber) {
                    fixObject.targetObjectNumber = kNoDestinationObject;
                }
            }
        }

        AlterObjectOwner(object, -1, true);
        object->attributes &= ~(kHated | kCanEngage | kCanCollide | kCanBeHit);
        execute_actions(object->baseType->destroy, object, NULL, NULL, true);
    } else {
        AddKillToAdmiral(object);
        if (object->attributes & kReleaseEnergyOnDeath) {
            int16_t energyNum = object->energy / kEnergyPodAmount;
            while (energyNum > 0) {
                CreateAnySpaceObject(
                        globals()->scenarioFileInfo.energyBlobID, &object->velocity,
                        &object->location, object->direction, kNoOwner, 0, -1);
                energyNum--;
            }
        }

        // if it's a destination, we keep anyone from thinking they have it as a destination
        // (all at once since this should be very rare)
        if ((object->attributes & kIsDestination) &&
                !object->baseType->destroyDontDie) {
            RemoveDestination(object->destinationObject);
            for (int16_t i = 0; i < kMaxSpaceObject; i++) {
                auto& fixObject = gSpaceObjectData[i];
                if ((fixObject.attributes & kCanAcceptDestination)
                        && (fixObject.active != kObjectAvailable)) {
                    if (fixObject.destinationObject == object->entryNumber) {
                        fixObject.destinationObject = kNoDestinationObject;
                        fixObject.destObjectPtr = NULL;
                        fixObject.attributes &= ~kStaticDestination;
                    }
                }
            }
        }

        execute_actions(object->baseType->destroy, object, NULL, NULL, true);

        if (object->attributes & kCanAcceptDestination) {
            RemoveObjectFromDestination(object);
        }
        if (!object->baseType->destroyDontDie) {
            object->active = kObjectToBeFreed;
        }
    }
}

void ActivateObjectSpecial(spaceObjectType* object) {
    if ((object->special.time > 0) || (object->special.type == kNoWeapon)) {
        return;
    }

    auto weaponObject = object->special.base;
    auto baseObject = object->baseType;
    if ((object->energy < weaponObject->frame.weapon.energyCost)
            || ((weaponObject->frame.weapon.ammo >= 0) && (object->special.ammo <= 0))) {
        return;
    }

    object->energy -= weaponObject->frame.weapon.energyCost;
    object->special.position++;
    if (object->special.position >= baseObject->special.positionNum) {
        object->special.position = 0;
    }

    int16_t direction = object->direction;
    mAddAngle(direction, -90);
    Fixed fcos, fsin;
    GetRotPoint(&fcos, &fsin, direction);
    fcos = -fcos;
    fsin = -fsin;

    object->special.time = weaponObject->frame.weapon.fireTime;
    if (weaponObject->frame.weapon.ammo > 0) {
        object->special.ammo--;
    }
    execute_actions(weaponObject->activate, object, NULL, NULL, true);
}

void CreateFloatingBodyOfPlayer(spaceObjectType* obj) {
    const auto body = globals()->scenarioFileInfo.playerBodyID;
    // if we're already in a body, don't create a body from it
    // a body expiring is handled elsewhere
    if (obj->whichBaseObject == body) {
        return;
    }

    int32_t ship_number = CreateAnySpaceObject(
            body, &obj->velocity, &obj->location, obj->direction, obj->owner, 0, -1);
    if (ship_number >= 0) {
        ChangePlayerShipNumber(obj->owner, ship_number);
    } else {
        PlayerShipBodyExpire(obj, true);
    }
}

StringSlice get_object_name(int16_t id) {
    return space_object_names->at(id);
}

StringSlice get_object_short_name(int16_t id) {
    return space_object_short_names->at(id);
}

}  // namespace antares

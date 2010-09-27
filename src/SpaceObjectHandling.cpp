// Ares, a tactical space combat game.
// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// 
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

// Space Object Handling >> MUST BE INITED _AFTER_ SCENARIOMAKER << (uses Ares Scenarios file)

#include "SpaceObjectHandling.hpp"

#include <sfz/sfz.hpp>
#include "Admiral.hpp"
#include "AresGlobalType.hpp"
#include "Beam.hpp"
#include "ColorTranslation.hpp"
#include "Debug.hpp"
#include "Error.hpp"
#include "MathMacros.hpp"
#include "MessageScreen.hpp"
#include "Minicomputer.hpp" // for MiniComputer_SetScreenAndLineHack
#include "Motion.hpp"
#include "OffscreenGWorld.hpp"
#include "Options.hpp"
#include "PlayerShip.hpp"
#include "Randomize.hpp"
#include "Resource.hpp"
#include "Rotation.hpp"
#include "ScenarioMaker.hpp"
#include "ScreenLabel.hpp"
#include "ScrollStars.hpp"
#include "SpaceObject.hpp"
#include "SpriteHandling.hpp"
#include "Transitions.hpp"
#include "UniverseUnit.hpp"

using sfz::BytesPiece;
using sfz::ReadSource;
using sfz::read;
using sfz::scoped_array;

namespace antares {

#define kTestSpriteError    "\pSOHD"

#define kActionQueueLength      120

#define kFriendlyColor          GREEN
#define kHostileColor           RED
#define kNeutralColor           SKY_BLUE

#define kBatteryToEnergyRatio   5
#define kSpriteMaxSize          2048

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

extern int32_t gRandomSeed;
extern long             gAbsoluteScale, CLIP_LEFT, CLIP_TOP, CLIP_RIGHT, CLIP_BOTTOM;
extern coordPointType   gGlobalCorner;
extern spriteType**     gSpriteTable;
extern scenarioType*    gThisScenario;
extern spaceObjectType* gScrollStarObject;

spaceObjectType* gRootObject = nil;
long gRootObjectNumber = -1;
actionQueueType* gFirstActionQueue = nil;
long gFirstActionQueueNumber = -1;
spaceObjectType kZeroObject;

scoped_array<spaceObjectType> gSpaceObjectData;
scoped_array<baseObjectType> gBaseObjectData;
scoped_array<objectActionType> gObjectActionData;
scoped_array<actionQueueType> gActionQueueData;

void Translate_Coord_To_Scenario_Rotation( long h, long v, coordPointType *coord);

int SpaceObjectHandlingInit() {
    bool correctBaseObjectColor = false;

    gSpaceObjectData.reset(new spaceObjectType[kMaxSpaceObject]);
    if (gBaseObjectData.get() == nil) {
        Resource rsrc("objects", "bsob", kBaseObjectResID);
        BytesPiece in(rsrc.data());
        size_t count = rsrc.data().size() / baseObjectType::byte_size;
        globals()->maxBaseObject = count;
        gBaseObjectData.reset(new baseObjectType[count]);
        for (size_t i = 0; i < count; ++i) {
            read(&in, gBaseObjectData.get() + i);
        }
        check(in.empty(), "didn't consume all of base object data");
        correctBaseObjectColor = true;
    }

    if (gObjectActionData.get() == nil) {
        Resource rsrc("object-actions", "obac", kObjectActionResID);
        BytesPiece in(rsrc.data());
        size_t count = rsrc.data().size() / objectActionType::byte_size;
        globals()->maxObjectAction = count;
        gObjectActionData.reset(new objectActionType[count]);
        for (size_t i = 0; i < count; ++i) {
            read(&in, gObjectActionData.get() + i);
        }
        check(in.empty(), "didn't consume all of object action data");
    }

    gActionQueueData.reset(new actionQueueType[kActionQueueLength]);
    if (correctBaseObjectColor) {
        CorrectAllBaseObjectColor();
    }
    ResetAllSpaceObjects();
    ResetActionQueueData();
    return ( kNoError);
}

void CleanupSpaceObjectHandling() {
    gBaseObjectData.reset();
    gSpaceObjectData.reset();
    gObjectActionData.reset();
    gActionQueueData.reset();
}

void ResetAllSpaceObjects() {
    spaceObjectType *anObject = nil;
    short           i;

    gRootObject = nil;
    gRootObjectNumber = -1;
    anObject = gSpaceObjectData.get();
    for (i = 0; i < kMaxSpaceObject; i++) {
//      anObject->attributes = 0;
        anObject->active = kObjectAvailable;
        anObject->sprite = nil;
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

    WriteDebugLine("\p>RESETACT");
    gFirstActionQueueNumber = -1;
    gFirstActionQueue = nil;

    for ( i = 0; i < kActionQueueLength; i++)
    {
        action->actionNum = -1;
        action->actionToDo = 0;
        action->action = nil;
        action->nextActionQueueNum = -1;
        action->nextActionQueue = nil;
        action->scheduledTime = -1;
        action->subjectObject = nil;
        action->subjectObjectNum = -1;
        action->subjectObjectID = -1;
        action->directObject = nil;
        action->directObjectNum = -1;
        action->directObjectID = -1;
        action->offset.h = action->offset.v = 0;
        action++;
    }
    WriteDebugLine("\p<RESETACT");
}

/* AddSpaceObject:
    Returns -1 if no object available, otherwise returns object #

int AddSpaceObject( spaceObjectType *sourceObject, long *canBuildType,
                short nameResID, short nameStrNum)
*/

int AddSpaceObject( spaceObjectType *sourceObject)

{
    spaceObjectType *destObject = nil;
    int             whichObject = 0;
    natePixType* spriteTable = NULL;
    Point           where;
    spritePix       oldStyleSprite;
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
//      DebugStr("\pNo Free Objects!");
        return( -1);
    }

    if ( sourceObject->pixResID != kNoSpriteTable)
    {
        spriteTable = GetPixTable( sourceObject->pixResID);

        if (spriteTable == nil) {
            fail("Received an unexpected request to load a sprite");
//          DebugStr("\pAdding Sprite Table in the Middle of Level?");
            spriteTable = AddPixTable( sourceObject->pixResID);
            if (spriteTable == nil) {
                return -1;
            }
        }
    }

//  sourceObject->id = whichObject;
    *destObject = *sourceObject;

    destObject->lastLocation = destObject->location;
    destObject->collideObject = nil;
    destObject->lastLocation.h += 100000;
    destObject->lastLocation.v += 100000;
    destObject->lastDir = destObject->direction;

                scaleCalc = ( destObject->location.h - gGlobalCorner.h) * gAbsoluteScale;
                scaleCalc >>= SHIFT_SCALE;
                where.h = scaleCalc + CLIP_LEFT;
                scaleCalc = (destObject->location.v - gGlobalCorner.v) * gAbsoluteScale;
                scaleCalc >>= SHIFT_SCALE; /*+ CLIP_TOP*/;
                where.v = scaleCalc;

//  where.h = destObject->location.h - gGlobalCorner.h + CLIP_LEFT;
//  where.v = destObject->location.v - gGlobalCorner.v /*+ CLIP_TOP*/;
    if ( destObject->sprite != nil) RemoveSprite( destObject->sprite);

    if (spriteTable != nil) {
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
            GetRGBTranslateColorShade(&tinyColor, kFriendlyColor, tinyShade);
        } else if ( destObject->owner <= kNoOwner)
        {
            GetRGBTranslateColorShade(&tinyColor, kNeutralColor, tinyShade);
        } else
        {
            GetRGBTranslateColorShade(&tinyColor, kHostileColor, tinyShade);
        }

        if ( destObject->attributes & kIsSelfAnimated)
        {
                whichShape = destObject->frame.animation.thisShape >> kFixedBitShiftNumber;
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

        if ( destObject->sprite == nil)
        {
            globals()->gGameOver = -1;
            destObject->active = kObjectAvailable;
            return( -1);
        }
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

    } else
    {
        destObject->scaledCornerOffset.h = destObject->scaledCornerOffset.v = 0;
        destObject->scaledSize.h = destObject->scaledSize.v = 0;
        destObject->sprite = nil;
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
//      if ( destObject->frame.beam.beam == nil) DebugStr("\pAddObj:nil beam");
    }

    destObject->nextObject = gRootObject;
    destObject->nextObjectNumber = gRootObjectNumber;
    destObject->previousObject = nil;
    destObject->previousObjectNumber = -1;
    if ( gRootObject != nil)
    {
        gRootObject->previousObject = destObject;
        gRootObject->previousObjectNumber = whichObject;
    }
    gRootObject = destObject;
    gRootObjectNumber = whichObject;

    destObject->active = kObjectInUse;
    destObject->nextNearObject = destObject->nextFarObject = nil;
    destObject->whichLabel = kNoLabel;
    destObject->entryNumber = whichObject;
    destObject->cloakState = destObject->hitState = 0;
    destObject->duty = eNoDuty;

//  WriteDebugLine((char *)"\pAddObjOwner:");
//  WriteDebugLong( destObject->owner);

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
                WriteDebugLine((char *)"\pSOBJ Error");
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
        if ( anObject->sprite != nil)
        {
            RemoveSprite( anObject->sprite);
            anObject->sprite = nil;
            anObject->whichSprite = kNoSprite;
        }
        anObject->active = kObjectAvailable;
        anObject->nextNearObject = anObject->nextFarObject = nil;
        anObject->attributes = 0;
        anObject++;
    }
}

void CorrectAllBaseObjectColor( void)

{
    baseObjectType  *aBase = gBaseObjectData.get();
    short           i;
    transColorType  *transColor;
    unsigned char   fixColor;

    for ( i = 0; i < kMaxBaseObject; i++)
    {
        if (( aBase->shieldColor != 0xFF) && ( aBase->shieldColor != 0))
        {
//          aBase->shieldColor = GetTranslateColorShade( aBase->shieldColor, VERY_LIGHT);
            mGetTranslateColorShade( aBase->shieldColor, 15, fixColor, transColor);
            aBase->shieldColor = fixColor;
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
            aBase->frame.animation.firstShape <<= kFixedBitShiftNumber;
            aBase->frame.animation.lastShape <<= kFixedBitShiftNumber;
            aBase->frame.animation.frameShape <<= kFixedBitShiftNumber;
            aBase->frame.animation.frameShapeRange <<= kFixedBitShiftNumber;
        }
        aBase++;
    }

}

void InitSpaceObjectFromBaseObject( spaceObjectType *dObject, long  whichBaseObject, short seed,
            long direction, fixedPointType *velocity, long owner, short spriteIDOverride)

{
    baseObjectType  *sObject = mGetBaseObjectPtr( whichBaseObject), *weaponBase = nil;
    short           i;
    long            r;
    smallFixedType  f;
    fixedPointType  newVel;
    long            l;

//  DebugFileAppendString( "\pIN\t");

    dObject->offlineTime = 0;

    dObject->randomSeed = seed;
//  DebugFileAppendLong( dObject->randomSeed);
//  DebugFileAppendString( "\p\t");
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
//  DebugFileAppendLong( dObject->direction);
//  DebugFileAppendString( "\p\t");

    f = sObject->initialVelocity;
    if ( sObject->initialVelocityRange > 0)
    {
        f += RandomSeeded( sObject->initialVelocityRange,
                    &(dObject->randomSeed), 'soh3', whichBaseObject);
    }
    GetRotPoint(&newVel.h, &newVel.v, r);
    newVel.h = mMultiplyFixed( newVel.h, f);
    newVel.v = mMultiplyFixed( newVel.v, f);

    if ( velocity != nil)
    {
        newVel.h += velocity->h;
        newVel.v += velocity->v;
    }

    dObject->velocity.h = newVel.h;
    dObject->velocity.v = newVel.v;
    dObject->maxVelocity = sObject->maxVelocity;
//  DebugFileAppendSmallFixed( dObject->velocity.h);
//  DebugFileAppendString( "\p\t");

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
    dObject->destObjectPtr = nil;
    dObject->destObjectDest = kNoDestinationObject;
    dObject->destObjectID = kNoDestinationObject;
    dObject->destObjectDestID = kNoDestinationObject;
/*
    if (( dObject->attributes & kCanThink) && ( owner != kNoOwner))
    {
        f = dObject->baseType->offenseValue;
        for ( i = 0; i < kScenarioPlayerNum; i++)
        {
            dObject->balance[i] = -f;
        }
        dObject->balance[owner] = f;
        SetObjectDestination( dObject);
    } else
    {
        for ( i = 0; i < kScenarioPlayerNum; i++)
        {
            dObject->balance[i] = 0;
        }
    }
*/
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
//      DebugFileAppendLong( dObject->frame.animation.thisShape);
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
//  DebugFileAppendString( "\p\t");

    // not setting lastTimeUpdate;

    dObject->health = sObject->health;

    // not setting owner

    if ( sObject->initialAge >= 0)
        dObject->age = sObject->initialAge + RandomSeeded( sObject->initialAgeRange, &(dObject->randomSeed), 'soh6',
            whichBaseObject);
    else dObject->age = -1;
//  DebugFileAppendLong( dObject->age);
//  DebugFileAppendString( "\p\r");
    dObject->naturalScale = sObject->naturalScale;

    // not setting id

    dObject->active = kObjectInUse;
    dObject->nextNearObject = dObject->nextFarObject = nil;

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
    else dObject->pulseBase = nil;
    dObject->beamType = sObject->beam;
    if ( dObject->beamType != kNoWeapon)
        dObject->beamBase = mGetBaseObjectPtr( dObject->beamType);
    else dObject->beamBase = nil;
    dObject->specialType = sObject->special;
    if ( dObject->specialType != kNoWeapon)
        dObject->specialBase = mGetBaseObjectPtr( dObject->specialType);
    else dObject->specialBase = nil;
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
    baseObjectType  *sObject = mGetBaseObjectPtr( whichBaseObject), *weaponBase = nil;
    short           angle;
    long            r;
    natePixType* spriteTable;

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
//      DebugFileAppendLong( dObject->frame.animation.thisShape);
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
    else dObject->pulseBase = nil;
    dObject->beamType = sObject->beam;
    if ( dObject->beamType != kNoWeapon)
        dObject->beamBase = mGetBaseObjectPtr( dObject->beamType);
    else dObject->beamBase = nil;
    dObject->specialType = sObject->special;
    if ( dObject->specialType != kNoWeapon)
        dObject->specialBase = mGetBaseObjectPtr( dObject->specialType);
    else dObject->specialBase = nil;
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

        if (spriteTable == nil) {
            fail("Couldn't load a requested sprite");
            spriteTable = AddPixTable( dObject->pixResID);
//          if ( spriteTable == nil) Debugger();
        }

        dObject->sprite->table = spriteTable;
        dObject->sprite->tinySize = sObject->tinySize;
        dObject->sprite->whichLayer = sObject->pixLayer;
        dObject->sprite->scale = sObject->naturalScale;

        if ( dObject->attributes & kIsSelfAnimated)
        {
            dObject->sprite->whichShape = dObject->frame.animation.thisShape >> kFixedBitShiftNumber;
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
                        *nextQueue = gFirstActionQueue, *previousQueue = nil;

    while (( actionQueue->action != nil) && ( queueNumber < kActionQueueLength))
    {
        actionQueue++;
        queueNumber++;
    }

    if ( queueNumber == kActionQueueLength) return; //DebugStr("\pActionQueue FULL!");
//  delayTime += globals()->gGameTime;
    actionQueue->action = action;
    actionQueue->actionNum = actionNumber;
    actionQueue->scheduledTime = delayTime;
    actionQueue->subjectObject = subjectObject;
    actionQueue->actionToDo = actionToDo;

    if ( offset == nil)
    {
        actionQueue->offset.h = actionQueue->offset.v = 0;
    } else
    {
        actionQueue->offset.h = offset->h;
        actionQueue->offset.v = offset->v;
    }

    if ( subjectObject != nil)
    {
        actionQueue->subjectObjectNum = subjectObject->entryNumber;
        actionQueue->subjectObjectID = subjectObject->id;
    } else
    {
        actionQueue->subjectObjectNum = -1;
        actionQueue->subjectObjectID = -1;
    }
    actionQueue->directObject = directObject;
    if ( directObject != nil)
    {
        actionQueue->directObjectNum = directObject->entryNumber;
        actionQueue->directObjectID = directObject->id;
    } else
    {
        actionQueue->directObjectNum = -1;
        actionQueue->directObjectID = -1;
    }

    while (( nextQueue != nil) && ( nextQueue->scheduledTime < delayTime))
    {
        previousQueue = nextQueue;
        nextQueue = nextQueue->nextActionQueue;
    }
    if ( previousQueue == nil)
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
//  if ( gFirstActionQueue != nil) WriteDebugLong( gFirstActionQueue->scheduledTime);
//  WriteDebugLong( globals()->gGameTime);
//  WriteDebugLine((char *)"\p<ADDACT");
}

void ExecuteActionQueue( long unitsToDo)

{
//  actionQueueType     *actionQueue = gFirstActionQueue;
    actionQueueType     *actionQueue = gActionQueueData.get();
    long                        subjectid, directid, i;

    for ( i = 0; i < kActionQueueLength; i++)
    {
        if ( actionQueue->action != nil)
        {
            actionQueue->scheduledTime -= unitsToDo;
        }
        actionQueue++;
    }

    actionQueue = gFirstActionQueue;
    while (( gFirstActionQueue != nil) &&
        ( gFirstActionQueue->action != nil) &&
        ( gFirstActionQueue->scheduledTime <= 0))
//      ( gFirstActionQueue->scheduledTime <= globals()->gGameTime))
    {
        subjectid = -1;
        directid = -1;
        if ( gFirstActionQueue->subjectObject != nil)
        {
            if ( gFirstActionQueue->subjectObject->active)
                subjectid = gFirstActionQueue->subjectObject->id;
        }

        if ( gFirstActionQueue->directObject != nil)
        {
            if ( gFirstActionQueue->directObject->active)
                directid = gFirstActionQueue->directObject->id;
        }
//      WriteDebugLine((char *)"\pSched");
//      WriteDebugLong( gFirstActionQueue->scheduledTime);
//      WriteDebugLong( globals()->gGameTime);
        if (( subjectid == gFirstActionQueue->subjectObjectID) &&
            ( directid == gFirstActionQueue->directObjectID))
        {
//          WriteDebugLine((char *)"\pExec");
            mWriteDebugString("\pExec Q:");
            WriteDebugLong( gFirstActionQueue->actionNum);
            WriteDebugLong( gFirstActionQueue->actionToDo);

            ExecuteObjectActions( gFirstActionQueue->actionNum, gFirstActionQueue->actionToDo,
                gFirstActionQueue->subjectObject, gFirstActionQueue->directObject,
                &(gFirstActionQueue->offset), false);
        }
        gFirstActionQueue->actionNum = -1;
        gFirstActionQueue->actionToDo = 0;
        gFirstActionQueue->action = nil;
        gFirstActionQueue->scheduledTime = -1;
        gFirstActionQueue->subjectObject = nil;
        gFirstActionQueue->subjectObjectNum = -1;
        gFirstActionQueue->subjectObjectID = -1;
        gFirstActionQueue->directObject = nil;
        gFirstActionQueue->directObjectNum = -1;
        gFirstActionQueue->directObjectID = -1;
        gFirstActionQueue->offset.h = gFirstActionQueue->offset.v = 0;

        actionQueue = gFirstActionQueue;

        gFirstActionQueueNumber = gFirstActionQueue->nextActionQueueNum;
        gFirstActionQueue = gFirstActionQueue->nextActionQueue;

        actionQueue->nextActionQueueNum = -1;
        actionQueue->nextActionQueue = nil;
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
    smallFixedType  f, f2;
    coordPointType  newLocation;
    Point           location;
    bool         OKtoExecute, checkConditions = false;
    Fixed           aFixed;
    transColorType  *transColor;
    unsigned char   tinyColor;
    Str255          s;

//  WriteDebugLine( (char *)"\pEX\t");
//  WriteDebugLine( globals()->gGameTime);
//  WriteDebugLine( "\p\t");
//  WriteDebugLong( whichAction);
//  WriteDebugLine( "\p\r");

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

//      WriteDebugLine((char *)"\pACTION");

        anObject = dObject;
        if (( action->reflexive) || ( anObject == nil)) anObject = sObject;

        OKtoExecute = false;
        if (dObject == NULL) {
            // This is a workaround for a bug which manifests itself for example in the
            // implementation of "Hold Position".  When an object is instructed to hold position,
            // it gains its own location as its destination, triggering its arrive action, but its
            // target is nulled out.
            //
            // Arrive actions are typically only specified on objects with non-zero order flags (so
            // that a transport won't attempt to land on a bunker station, for example).  So, back
            // when Ares ran without protected memory, and NULL pointed to a zeroed-out area of the
            // address space, the flags would prevent the arrive action from triggering.
            //
            // It's not correct to always inhibit the action here, because the arrive action should
            // be triggered when the anObject doesn't have flags.  But we need to prevent it in the
            // case of transports somehow, so we emulate the old behavior of pointing to a
            // zeroed-out object.
            dObject = &kZeroObject;
        }
        if (anObject == nil) {
            OKtoExecute = true;
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
//                  WriteDebugLine((char *)"\pCREATE");
//                  WriteDebugLong(action->argument.createObject.whichBaseType);
//                  WriteDebugLong(action->argument.createObject.howManyMinimum);
//                  WriteDebugLong(action->argument.createObject.howManyRange);

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
                        if ( offset != nil)
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
                                        else if ( anObject->destObjectPtr != nil)
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
//                  WriteDebugLine((char *)"\pSOUND");
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
                    if ( anObject->sprite != nil)
                    {
                        location.h = anObject->sprite->where.h;
                        location.v = anObject->sprite->where.v;
                        MakeNewSparks(  action->argument.makeSparks.howMany,                    // sparkNum
                                        action->argument.makeSparks.speed,                      // sparkSpeed
                                        action->argument.makeSparks.velocityRange,      // velocity
                                        action->argument.makeSparks.color,                  // COLOR
                                        &location               // location
                                    );
                    } else
                    {
                        l = ( anObject->location.h - gGlobalCorner.h) * gAbsoluteScale;
                        l >>= SHIFT_SCALE;
                        if (( l > -kSpriteMaxSize) && ( l < kSpriteMaxSize))
                            location.h = l + CLIP_LEFT;
                        else
                            location.h = -kSpriteMaxSize;

                        l = (anObject->location.v - gGlobalCorner.v) * gAbsoluteScale;
                        l >>= SHIFT_SCALE; /*+ CLIP_TOP*/;
                        if (( l > -kSpriteMaxSize) && ( l < kSpriteMaxSize))
                            location.v = l + CLIP_TOP;
                        else
                            location.v = -kSpriteMaxSize;

                        MakeNewSparks(  action->argument.makeSparks.howMany,                    // sparkNum
                                        action->argument.makeSparks.speed,                      // sparkSpeed
                                        action->argument.makeSparks.velocityRange,      // velocity
                                        action->argument.makeSparks.color,                  // COLOR
                                        &location               // location
                                    );
                    }
                    break;

                case kDie:
//                  WriteDebugLine((char *)"\pDIE");
//                  WriteDebugLong( anObject->id);

//                  if ( anObject->attributes & kIsBeam)
//                      anObject->frame.beam.killMe = true;
                    switch ( action->argument.killObject.dieType)
                    {
                        case kDieExpire:
                            if ( sObject != nil)
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
                            if ( sObject != nil)
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
//                  WriteDebugLine((char *)"\pALTER");
                    switch( action->argument.alterObject.alterType)
                    {
                        case kAlterDamage:
                            AlterObjectHealth( anObject,
                                action->argument.alterObject.minimum);
                            break;

                        case kAlterEnergy:
//                  WriteDebugLine((char *)"\pNRG");
//                  WriteDebugLong( sObject->id);

                            if ( action->argument.alterObject.minimum > 10000)
                            {
                                WriteDebugLine("\pNRG");
                                WriteDebugLong( anObject->whichBaseObject);
                                WriteDebugLong( anObject->owner);
                                WriteDebugFixed( action->argument.alterObject.minimum);
                            }
                            AlterObjectEnergy( anObject,
                                action->argument.alterObject.minimum);
                            break;

                        case 919191://kAlterSpecial:
//                  WriteDebugLine((char *)"\pSpecial");
                            anObject->specialType = action->argument.alterObject.minimum;
                            baseObject = mGetBaseObjectPtr( anObject->specialType);
                            anObject->specialAmmo = baseObject->frame.weapon.ammo;
                            anObject->specialTime = anObject->specialPosition = 0;
                            if ( baseObject->frame.weapon.range > anObject->longestWeaponRange)
                                anObject->longestWeaponRange = baseObject->frame.weapon.range;
                            if ( baseObject->frame.weapon.range < anObject->shortestWeaponRange)
                                anObject->shortestWeaponRange = baseObject->frame.weapon.range;
                            break;

                        case kAlterHidden:
//                          WriteDebugLine((char *)"\p>Unhide");
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
//                          WriteDebugLine((char *)"\pOffline!");
                            f2 = anObject->baseType->mass;
                            if ( f2 == 0) anObject->offlineTime = -1;
                            else
                            {
                                anObject->offlineTime = mDivideFixed( f, f2);
                            }
//                          WriteDebugLong( anObject->randomSeed);
//                          WriteDebugLong( anObject->offlineTime);
                            anObject->offlineTime = mFixedToLong( anObject->offlineTime);
                            break;

                        case kAlterVelocity:
//                          if ( action->reflexive) WriteDebugLine((char *)"\pREFLEX!");
                            if ( sObject != nil)
                            {
                                // active (non-reflexive) altering of velocity means a PUSH, just like
                                //  two objects colliding.  Negative velocity = slow down
                                if ( dObject != nil)
                                {
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
                                                    #ifdef powerc
                                                    aFixed = MyFixRatio( dObject->velocity.h, dObject->velocity.v);
                                                    #else
                                                    aFixed = FixRatio( dObject->velocity.h, dObject->velocity.v);
                                                    #endif

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
                                                    #ifdef powerc
                                                    aFixed = MyFixRatio( dObject->velocity.h, dObject->velocity.v);
                                                    #else
                                                    aFixed = FixRatio( dObject->velocity.h, dObject->velocity.v);
                                                    #endif

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
//                                      WriteDebugLine((char *)"\pAbsolute Vel!");
                                    } else
                                    {
                                        anObject->velocity.h = f;
                                        anObject->velocity.v = f2;
//                                      WriteDebugLine((char *)"\pAbsolute Vel!");
                                    }
                                }

                            }// else DebugStr("\pAlter Vel Nil");
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
                            WriteDebugLine("\pAlterBase!");
                            if ( (action->reflexive) || ( dObject != nil))
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
                                if (( action->reflexive) && ( dObject != nil))
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
//                  WriteDebugLine((char *)"\ptrueYET");
                            if ( action->argument.alterObject.range <= 0)
                            {
                                SetScenarioConditionTrueYet( action->argument.alterObject.minimum,
                                                            action->argument.alterObject.relative);
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
                                    SetScenarioConditionTrueYet( l,
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
                                if ( anObject != nil)
                                {
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
                                if ( dObject == nil)
                                {
                                    newLocation.h = sObject->location.h;
                                    newLocation.v = sObject->location.v;
                                } else
                                {
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
                                Translate_Coord_To_Scenario_Rotation(
                                    action->argument.alterObject.minimum,
                                    action->argument.alterObject.range,
                                    &anObject->location);
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
                                anObject->pulseBase = nil;
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
                                anObject->beamBase = nil;
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
                                anObject->specialBase = nil;
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
//                  DebugStr("\pLand At!");
//                  WriteDebugLine((char *)"\pLANDAT");
//                  WriteDebugLong( sObject->whichBaseObject);
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
//                  WriteDebugLine((char *)"\pWARPIN");
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
                    if (( action->argument.changeScore.whichPlayer == -1) && ( anObject != nil))
                        l = anObject->owner;
                    else
                    {
                        l = mGetRealAdmiralNum( action->argument.changeScore.whichPlayer);
                    }
//                  WriteDebugLine((char *)"\pChangeScore");
//                  WriteDebugLong( l);
                    if ( l >= 0)
                    {
                        AlterAdmiralScore( l, action->argument.changeScore.whichScore, action->argument.changeScore.amount);
                        checkConditions = true;
                    }
                    break;

                case kDeclareWinner:
//                  WriteDebugLine((char *)"\pDCLRWIN");
                    if (( action->argument.declareWinner.whichPlayer == -1) && ( anObject != nil))
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
//                  WriteDebugLine((char *)"\pSETDEST");
                    ul1 = sObject->attributes;
                    sObject->attributes &= ~kStaticDestination;
//                  if ( sObject->attributes & kStaticDestination) WriteDebugLine((char *)"\pSTATIC!");
//                  WriteDebugLong( sObject->whichBaseObject);
//                  WriteDebugLong( anObject->whichBaseObject);
                    SetObjectDestination( sObject, anObject);
                    sObject->attributes = ul1;
                    break;

                case kActivateSpecial:
                    WriteDebugLine("\pActivate Special!");
                    ActivateObjectSpecial( sObject);
                    break;

                case kColorFlash:
                    mGetTranslateColorShade( action->argument.colorFlash.color, action->argument.colorFlash.shade, tinyColor, transColor);
                    StartBooleanColorAnimation( action->argument.colorFlash.length,
                        action->argument.colorFlash.length, tinyColor);//GetTranslateColorShade( AQUA, VERY_LIGHT));
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
                        GetIndString( s, kMessageStringID, globals()->gZoomMode + kZoomStringOffset);
                        SetStatusString( s, true, kStatusLabelColor);
                    }
                    break;

                case kComputerSelect:
                    MiniComputer_SetScreenAndLineHack( action->argument.computerSelect.screenNumber,
                        action->argument.computerSelect.lineNumber);
                    break;

                case kAssumeInitialObject:
                {
                    scenarioInitialType *initialObject;

                    initialObject = mGetScenarioInitial( gThisScenario, (action->argument.assumeInitial.whichInitialObject+GetAdmiralScore(0, 0)));
                    if ( initialObject != nil)
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

void DebugExecuteObjectActions( long whichAction, long actionNum,
                spaceObjectType *sObject, spaceObjectType *dObject, Point* offset, char *file, long line)
{
#pragma unused( whichAction, actionNum, sObject, dObject, offset, file, line)
//  DebugFileAppendString( "\pEX\t");
//  DebugFileAppendLong( globals()->gGameTime);
//  DebugFileAppendString( "\p\t");
//  DebugFileAppendCString( file);
//  DebugFileAppendString( "\p\t");
//  DebugFileAppendLong( line);
//  DebugFileAppendString( "\p\t");
//  DebugFileAppendLong( whichAction);
//  DebugFileAppendString( "\p\t");
    if ( sObject != nil)
    {
//      DebugFileAppendLong( sObject->whichBaseObject);
    }
//  DebugFileAppendString( "\p\t");
    if ( dObject != nil)
    {
//      DebugFileAppendLong( dObject->whichBaseObject);
    }
//  DebugFileAppendString( "\p\r");
//  XExecuteObjectActions( whichAction, actionNum, sObject, dObject, offset);
}

/*
long CreateAnySpaceObject( long whichBase, fixedPointType *velocity,
            coordPointType *location, long direction, long owner, unsigned long specialAttributes,
            long *canBuildType, short nameResID, short nameStrNum, short spriteIDOverride)
*/
long CreateAnySpaceObject( long whichBase, fixedPointType *velocity,
            coordPointType *location, long direction, long owner,
            unsigned long specialAttributes, short spriteIDOverride)

{
    spaceObjectType *madeObject = nil, newObject, *player = nil;
    long            newObjectNumber;
    unsigned long   distance, dcalc, difference;
    uint64_t        hugeDistance;

/*  DebugFileAppendString( "\pCR\t");
    DebugFileAppendLong( globals()->gGameTime);
    DebugFileAppendString( "\p\t");
    DebugFileAppendLong( whichBase);
    DebugFileAppendString( "\p\r");
*/
// here
    InitSpaceObjectFromBaseObject( &newObject, whichBase, RandomSeeded( 32766, &gRandomSeed, 'so18', whichBase),
                                    direction, velocity, owner, spriteIDOverride);
    newObject.location = *location;
    if ( globals()->gPlayerShipNumber >= 0)
        player = gSpaceObjectData.get() + globals()->gPlayerShipNumber;
    else player = nil;
    if (( player != nil) && ( player->active))
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

//  WriteDebugLine((char *)"\pOwnerBirth:");
//  WriteDebugLong( owner);

    newObject.sprite = nil;
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
//      DebugStr("\pCouldn't create an object!");
    } else
    {
        madeObject = gSpaceObjectData.get() + newObjectNumber;
        madeObject->attributes |= specialAttributes;
/*      if ( madeObject->sprite != nil)
        {
            DebugFileAppendString( "\pCS\t");
            DebugFileAppendLong( globals()->gGameTime);
            DebugFileAppendString( "\p\t\t");
            DebugFileAppendLong( madeObject->sprite->whichShape);
            DebugFileAppendString( "\p\r");
        }
*/      ExecuteObjectActions( madeObject->baseType->createAction, madeObject->baseType->createActionNum,
                            madeObject, nil, nil, true);
    }
/*  if ( newObject.attributes & kCanThink)
    {
        DebugFileAppendLong( globals()->gGameTime);
        DebugFileAppendString("\p\t");
        DebugFileAppendLong( globals()->gTheseKeys);
        DebugFileAppendString("\p\t");
        DebugFileAppendLong( gRandomSeed);
        DebugFileAppendString("\p\r");
    }
*/  return( newObjectNumber);

}

long DebugCreateAnySpaceObject( long whichBase, fixedPointType *velocity,
            coordPointType *location, long direction, long owner, unsigned long specialAttributes,
            long *canBuildType, short nameResID, short nameStrNum, short spriteIDOverride, char *file, long line)
{
#pragma unused( whichBase, velocity, location, direction, owner, specialAttributes, canBuildType, nameResID, nameStrNum, spriteIDOverride, file, line)
/*  DebugFileAppendString( "\pCR\t");
    DebugFileAppendLong( globals()->gGameTime);
    DebugFileAppendString( "\p\t");
    DebugFileAppendCString( file);
    DebugFileAppendString( "\p\t");
    DebugFileAppendLong( line);
    DebugFileAppendString( "\p\t");
    DebugFileAppendLong( whichBase);
    DebugFileAppendString( "\p\t");
    if ( velocity != nil)
    {
        DebugFileAppendSmallFixed( velocity->h);
    }
    DebugFileAppendString( "\p\t");
    if ( location != nil)
    {
        DebugFileAppendLong( location->h);
    }
    DebugFileAppendString( "\p\r");
*/
//  return ( XCreateAnySpaceObject( whichBase, velocity, location, direction, owner, specialAttributes,
//          canBuildType, nameResID, nameStrNum, spriteIDOverride));
    return( -1);
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

            if ( anObject == nil)
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

            if ( anObject == nil)
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
    spaceObjectType *fixObject = nil;
    long            i, originalOwner = anObject->owner;
    Str255          s;
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

        if (( anObject->sprite != nil)/* && ( anObject->baseType->tinySize != 0)*/)
        {
            mWriteDebugString("\pHas Sprite");
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
                GetRGBTranslateColorShade(&tinyColor, kFriendlyColor, tinyShade);
            } else if ( owner <= kNoOwner)
            {
                GetRGBTranslateColorShade(&tinyColor, kNeutralColor, tinyShade);
            } else
            {
                GetRGBTranslateColorShade(&tinyColor, kHostileColor, tinyShade);
            }
            anObject->tinyColor = anObject->sprite->tinyColor = tinyColor;

            if ( anObject->attributes & kCanThink)
            {
                natePixType* pixTable;

                if (( anObject->pixResID == anObject->baseType->pixResID) ||
                    ( anObject->pixResID == (anObject->baseType->pixResID |
                    (GetAdmiralColor( originalOwner)
                    << kSpriteTableColorShift))))
                {

                    anObject->pixResID =
                        anObject->baseType->pixResID | (GetAdmiralColor( owner)
                        << kSpriteTableColorShift);

                    pixTable = GetPixTable( anObject->pixResID);
                    if (pixTable != nil) {
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
            if ( message)
            {
                StartMessage();
                AppendStringToMessage(GetDestBalanceName( anObject->destinationObject));
                if ( owner >= 0)
                {
                    AppendStringToMessage("\p captured by ");
                    AppendStringToMessage(GetAdmiralName( anObject->owner));
                    AppendStringToMessage("\p.");
                } else if ( originalOwner >= 0) // must be since can't both be -1
                {
                    AppendStringToMessage("\p lost by ");
                    AppendStringToMessage(GetAdmiralName( originalOwner));
                    AppendStringToMessage("\p.");
                }
                EndMessage();
            }
        } else
        {
            if ( message)
            {
                GetIndString( s, kSpaceObjectNameResID, anObject->whichBaseObject + 1);
                StartMessage();
                AppendStringToMessage( s);
                if ( owner >= 0)
                {
                    AppendStringToMessage("\p captured by ");
                    AppendStringToMessage(GetAdmiralName( anObject->owner));
                    AppendStringToMessage("\p.");
                } else if ( originalOwner >= 0) // must be since can't both be -1
                {
                    AppendStringToMessage("\p lost by ");
                    AppendStringToMessage(GetAdmiralName( originalOwner));
                    AppendStringToMessage("\p.");
                }
                EndMessage();
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

/*  DebugFileAppendString( "\pDO\t");
    DebugFileAppendLong( globals()->gGameTime);
    DebugFileAppendString( "\p\t");
    DebugFileAppendLong( gRandomSeed);
    DebugFileAppendString( "\p\t");
    DebugFileAppendLong( anObject->whichBaseObject);
    DebugFileAppendString( "\p\t");
    DebugFileAppendLong( anObject->active);
    DebugFileAppendString( "\p\t");
*/  if ( anObject->active == kObjectInUse)
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
                    anObject, nil, nil, true);
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
//              mWriteDebugString( "\pKillDest");

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
                            fixObject->destObjectPtr = nil;
                            fixObject->attributes &= ~kStaticDestination;
                        }
                    }
                    fixObject++;
                }
            }

//          WriteDebugLine((char *)"\pDestACT");
//          WriteDebugLong(anObject->baseType->destroyAction);
//          WriteDebugLong(anObject->baseType->destroyActionNum & kDestroyActionNotMask);
            ExecuteObjectActions( anObject->baseType->destroyAction,
                    anObject->baseType->destroyActionNum & kDestroyActionNotMask,
                    anObject, nil, nil, true);

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
    smallFixedType  fcos, fsin;

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
                                weaponObject->activateActionNum, anObject, nil, nil, true);
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
            mWriteDebugString("\pREMOTE BODY");
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
/*      DebugStr("\pCouldn't Create Floating Body");
        anObject->health = 1000;
        globals()->gGameOver = -360;
*/
    }
}

void Translate_Coord_To_Scenario_Rotation( long h, long v, coordPointType *coord)

{
    int32_t lcos, lsin, lscrap, angle = globals()->gScenarioRotation;

    mAddAngle( angle, 90);
    GetRotPoint(&lcos, &lsin, angle);
    lcos = -lcos;
    lsin = -lsin;

    lscrap = mMultiplyFixed(h, lcos);
    lscrap -= mMultiplyFixed(v, lsin);
    coord->h = kUniversalCenter;
    coord->h += lscrap;

    lscrap = mMultiplyFixed(h, lsin);
    lscrap += mMultiplyFixed(v, lcos);
    coord->v = kUniversalCenter;
    coord->v += lscrap;
}

void read_from(ReadSource in, objectActionType* action) {
    uint8_t section[24];

    read(in, &action->verb);
    read(in, &action->reflexive);
    read(in, &action->inclusiveFilter);
    read(in, &action->exclusiveFilter);
    read(in, &action->owner);
    read(in, &action->delay);
    read(in, &action->initialSubjectOverride);
    read(in, &action->initialDirectOverride);
    in.shift(4);
    read(in, section, 24);

    BytesPiece sub(BytesPiece(section, 24));
    switch (action->verb) {
      case kNoAction:
      case kSetDestination:
      case kActivateSpecial:
      case kActivatePulse:
      case kActivateBeam:
      case kNilTarget:
        break;

      case kCreateObject:
      case kCreateObjectSetDest:
        read(&sub, &action->argument.createObject);
        break;

      case kPlaySound:
        read(&sub, &action->argument.playSound);
        break;

      case kAlter:
        read(&sub, &action->argument.alterObject);
        break;

      case kMakeSparks:
        read(&sub, &action->argument.makeSparks);
        break;

      case kReleaseEnergy:
        read(&sub, &action->argument.releaseEnergy);
        break;

      case kLandAt:
        read(&sub, &action->argument.landAt);
        break;

      case kEnterWarp:
        read(&sub, &action->argument.enterWarp);
        break;

      case kDisplayMessage:
        read(&sub, &action->argument.displayMessage);
        break;

      case kChangeScore:
        read(&sub, &action->argument.changeScore);
        break;

      case kDeclareWinner:
        read(&sub, &action->argument.declareWinner);
        break;

      case kDie:
        read(&sub, &action->argument.killObject);
        break;

      case kColorFlash:
        read(&sub, &action->argument.colorFlash);
        break;

      case kDisableKeys:
      case kEnableKeys:
        read(&sub, &action->argument.keys);
        break;

      case kSetZoom:
        read(&sub, &action->argument.zoom);
        break;

      case kComputerSelect:
        read(&sub, &action->argument.computerSelect);
        break;

      case kAssumeInitialObject:
        read(&sub, &action->argument.assumeInitial);
        break;
    }
}

void read_from(ReadSource in, argumentType::CreateObject* argument) {
    read(in, &argument->whichBaseType);
    read(in, &argument->howManyMinimum);
    read(in, &argument->howManyRange);
    read(in, &argument->velocityRelative);
    read(in, &argument->directionRelative);
    read(in, &argument->randomDistance);
}

void read_from(ReadSource in, argumentType::PlaySound* argument) {
    read(in, &argument->priority);
    in.shift(1);
    read(in, &argument->persistence);
    read(in, &argument->absolute);
    in.shift(1);
    read(in, &argument->volumeMinimum);
    read(in, &argument->volumeRange);
    read(in, &argument->idMinimum);
    read(in, &argument->idRange);
}

void read_from(ReadSource in, argumentType::AlterObject* argument) {
    read(in, &argument->alterType);
    read(in, &argument->relative);
    read(in, &argument->minimum);
    read(in, &argument->range);
}

void read_from(ReadSource in, argumentType::MakeSparks* argument) {
    read(in, &argument->howMany);
    read(in, &argument->speed);
    read(in, &argument->velocityRange);
    read(in, &argument->color);
}

void read_from(ReadSource in, argumentType::ReleaseEnergy* argument) {
    read(in, &argument->percent);
}

void read_from(ReadSource in, argumentType::LandAt* argument) {
    read(in, &argument->landingSpeed);
}

void read_from(ReadSource in, argumentType::EnterWarp* argument) {
    read(in, &argument->warpSpeed);
}

void read_from(ReadSource in, argumentType::DisplayMessage* argument) {
    read(in, &argument->resID);
    read(in, &argument->pageNum);
}

void read_from(ReadSource in, argumentType::ChangeScore* argument) {
    read(in, &argument->whichPlayer);
    read(in, &argument->whichScore);
    read(in, &argument->amount);
}

void read_from(ReadSource in, argumentType::DeclareWinner* argument) {
    read(in, &argument->whichPlayer);
    read(in, &argument->nextLevel);
    read(in, &argument->textID);
}

void read_from(ReadSource in, argumentType::KillObject* argument) {
    read(in, &argument->dieType);
}

void read_from(ReadSource in, argumentType::ColorFlash* argument) {
    read(in, &argument->length);
    read(in, &argument->color);
    read(in, &argument->shade);
}

void read_from(ReadSource in, argumentType::Keys* argument) {
    read(in, &argument->keyMask);
}

void read_from(ReadSource in, argumentType::Zoom* argument) {
    read(in, &argument->zoomLevel);
}

void read_from(ReadSource in, argumentType::ComputerSelect* argument) {
    read(in, &argument->screenNumber);
    read(in, &argument->lineNumber);
}

void read_from(ReadSource in, argumentType::AssumeInitial* argument) {
    read(in, &argument->whichInitialObject);
}

void read_from(ReadSource in, baseObjectType* object) {
    uint8_t section[32];

    read(in, &object->attributes);
    read(in, &object->baseClass);
    read(in, &object->baseRace);
    read(in, &object->price);

    read(in, &object->offenseValue);
    read(in, &object->destinationClass);

    read(in, &object->maxVelocity);
    read(in, &object->warpSpeed);
    read(in, &object->warpOutDistance);

    read(in, &object->initialVelocity);
    read(in, &object->initialVelocityRange);

    read(in, &object->mass);
    read(in, &object->maxThrust);

    read(in, &object->health);
    read(in, &object->damage);
    read(in, &object->energy);

    read(in, &object->initialAge);
    read(in, &object->initialAgeRange);

    read(in, &object->naturalScale);

    read(in, &object->pixLayer);
    read(in, &object->pixResID);
    read(in, &object->tinySize);
    read(in, &object->shieldColor);
    in.shift(1);

    read(in, &object->initialDirection);
    read(in, &object->initialDirectionRange);

    read(in, &object->pulse);
    read(in, &object->beam);
    read(in, &object->special);

    read(in, &object->pulsePositionNum);
    read(in, &object->beamPositionNum);
    read(in, &object->specialPositionNum);

    read(in, object->pulsePosition, kMaxWeaponPosition);
    read(in, object->beamPosition, kMaxWeaponPosition);
    read(in, object->specialPosition, kMaxWeaponPosition);

    read(in, &object->friendDefecit);
    read(in, &object->dangerThreshold);
    read(in, &object->specialDirection);

    read(in, &object->arriveActionDistance);

    read(in, &object->destroyAction);
    read(in, &object->destroyActionNum);
    read(in, &object->expireAction);
    read(in, &object->expireActionNum);
    read(in, &object->createAction);
    read(in, &object->createActionNum);
    read(in, &object->collideAction);
    read(in, &object->collideActionNum);
    read(in, &object->activateAction);
    read(in, &object->activateActionNum);
    read(in, &object->arriveAction);
    read(in, &object->arriveActionNum);

    read(in, section, 32);

    read(in, &object->buildFlags);
    read(in, &object->orderFlags);
    read(in, &object->buildRatio);
    read(in, &object->buildTime);
    read(in, &object->skillNum);
    read(in, &object->skillDen);
    read(in, &object->skillNumAdj);
    read(in, &object->skillDenAdj);
    read(in, &object->pictPortraitResID);
    in.shift(6);
    read(in, &object->internalFlags);

    BytesPiece sub(BytesPiece(section, 32));
    if (object->attributes & kShapeFromDirection) {
        read(&sub, &object->frame.rotation);
    } else if (object->attributes & kIsSelfAnimated) {
        read(&sub, &object->frame.animation);
    } else if (object->attributes & kIsBeam) {
        read(&sub, &object->frame.beam);
    } else {
        read(&sub, &object->frame.weapon);
    }
}

void read_from(ReadSource in, objectFrameType::Rotation* rotation) {
    read(in, &rotation->shapeOffset);
    read(in, &rotation->rotRes);
    read(in, &rotation->maxTurnRate);
    read(in, &rotation->turnAcceleration);
}

void read_from(ReadSource in, objectFrameType::Animation* animation) {
    read(in, &animation->firstShape);
    read(in, &animation->lastShape);
    read(in, &animation->frameDirection);
    read(in, &animation->frameDirectionRange);
    read(in, &animation->frameSpeed);
    read(in, &animation->frameSpeedRange);
    read(in, &animation->frameShape);
    read(in, &animation->frameShapeRange);
}

void read_from(ReadSource in, objectFrameType::Beam* beam) {
    read(in, &beam->color);
    read(in, &beam->kind);
    read(in, &beam->accuracy);
    read(in, &beam->range);
}

void read_from(ReadSource in, objectFrameType::Weapon* weapon) {
    read(in, &weapon->usage);
    read(in, &weapon->energyCost);
    read(in, &weapon->fireTime);
    read(in, &weapon->ammo);
    read(in, &weapon->range);
    read(in, &weapon->inverseSpeed);
    read(in, &weapon->restockCost);
}

}  // namespace antares

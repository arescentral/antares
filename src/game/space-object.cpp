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
        anObject->active = kObjectAvailable;
        anObject->sprite = NULL;
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
    } else
    {
        destObject->sprite = NULL;
        destObject->whichSprite = kNoSprite;
    }

    if ( destObject->attributes & kIsBeam)
    {
        destObject->frame.beam = Beams::add( &(destObject->location),
            destObject->baseType->frame.beam.color,
            destObject->baseType->frame.beam.kind,
            destObject->baseType->frame.beam.accuracy,
            destObject->baseType->frame.beam.range);
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
    destObject->cloakState = destObject->hitState = 0;
    destObject->duty = eNoDuty;

    return ( whichObject);
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

void spaceObjectType::init(
        int32_t type, Random seed,
        int32_t relative_direction, fixedPointType *relative_velocity,
        int32_t owner, int16_t spriteIDOverride) {
    int16_t         i;
    int32_t         l;

    offlineTime = 0;

    randomSeed = seed;
    baseType = mGetBaseObjectPtr(type);
    attributes = baseType->attributes;
    whichBaseObject = type;
    keysDown = 0;
    timeFromOrigin = 0;
    runTimeFlags = 0;
    if (owner >= 0) {
        myPlayerFlag = 1 << owner;
    } else {
        myPlayerFlag = 0x80000000;
    }
    seenByPlayerFlags = 0xffffffff;
    hostileTowardsFlags = 0;
    absoluteBounds = {0, 0, 0, 0};
    shieldColor = baseType->shieldColor;
    tinySize = baseType->tinySize;
    layer = baseType->pixLayer;
    do {
        id = randomSeed.next(32768);
    } while (id == -1);

    distanceGrid = collisionGrid = {0, 0};
    periodicTime = 0;
    if (baseType->activatePeriod) {
        periodicTime = baseType->activatePeriod + randomSeed.next(baseType->activatePeriodRange);
    }

    direction = baseType->initialDirection;
    mAddAngle(direction, relative_direction);
    if (baseType->initialDirectionRange > 0) {
        i = randomSeed.next(baseType->initialDirectionRange);
        mAddAngle(direction, i);
    }

    Fixed f = baseType->initialVelocity;
    if (baseType->initialVelocityRange > 0) {
        f += randomSeed.next(baseType->initialVelocityRange);
    }
    GetRotPoint(&velocity.h, &velocity.v, direction);
    velocity.h = mMultiplyFixed(velocity.h, f);
    velocity.v = mMultiplyFixed(velocity.v, f);

    if (relative_velocity) {
        velocity.h += relative_velocity->h;
        velocity.v += relative_velocity->v;
    }

    maxVelocity = baseType->maxVelocity;

    motionFraction = {0, 0};
    if (attributes & (kCanThink | kRemoteOrHuman)) {
        thrust = 0;
    } else {
        thrust = baseType->maxThrust;
    }

    _energy = max_energy();
    rechargeTime = pulse.charge = beam.charge = special.charge = 0;
    warpEnergyCollected = 0;
    _battery = max_battery();
    this->owner = owner;
    destinationObject = kNoDestinationObject;
    destinationLocation = {0, 0};
    destObjectPtr = NULL;
    destObjectDest = kNoDestinationObject;
    destObjectID = kNoDestinationObject;
    destObjectDestID = kNoDestinationObject;
    remoteFoeStrength = remoteFriendStrength = escortStrength =
        localFoeStrength = localFriendStrength = 0;
    bestConsideredTargetValue = currentTargetValue = 0xffffffff;
    bestConsideredTargetNumber = -1;

    // not setting: absoluteBounds;

    directionGoal = turnFraction = turnVelocity = 0;
    if (attributes & kIsSelfAnimated) {
        frame.animation.thisShape = baseType->frame.animation.frameShape;
        if (baseType->frame.animation.frameShapeRange > 0) {
            l = randomSeed.next(baseType->frame.animation.frameShapeRange);
            frame.animation.thisShape += l;
        }
        frame.animation.frameDirection = baseType->frame.animation.frameDirection;
        if (baseType->frame.animation.frameDirectionRange == -1) {
            if (randomSeed.next(2) == 1) {
                frame.animation.frameDirection = 1;
            }
        } else if (baseType->frame.animation.frameDirectionRange > 0) {
            frame.animation.frameDirection += randomSeed.next(
                baseType->frame.animation.frameDirectionRange);
        }
        frame.animation.frameFraction = 0;
        frame.animation.frameSpeed = baseType->frame.animation.frameSpeed;
    }

    // not setting lastTimeUpdate;

    _health = max_health();

    // not setting owner

    age = -1;
    if (baseType->initialAge >= 0) {
        age = baseType->initialAge + randomSeed.next(baseType->initialAgeRange);
    }
    naturalScale = baseType->naturalScale;

    // not setting id

    active = kObjectInUse;
    nextNearObject = nextFarObject = NULL;

    // not setting sprite, targetObjectNumber, lastTarget, lastTargetDistance;

    closestDistance = kMaximumRelevantDistanceSquared;
    targetObjectNumber = lastTarget = targetObjectID = kNoShip;
    lastTargetDistance = targetAngle = 0;

    closestObject = kNoShip;
    presenceState = kNormalPresence;

    if (spriteIDOverride == -1) {
        pixResID = baseType->pixResID;
    } else {
        pixResID = spriteIDOverride;
    }

    if (baseType->attributes & kCanThink) {
        pixResID += (GetAdmiralColor(owner) << kSpriteTableColorShift);
    }

    pulse.type = baseType->pulse.base;
    pulse.base = mGetBaseObjectPtr(pulse.type);
    beam.type = baseType->beam.base;
    beam.base = mGetBaseObjectPtr(beam.type);
    special.type = baseType->special.base;
    special.base = mGetBaseObjectPtr(special.type);
    longestWeaponRange = 0;
    shortestWeaponRange = kMaximumRelevantDistance;

    for (auto weapon: {&pulse, &beam, &special}) {
        if (weapon->type != kNoWeapon) {
            auto weaponBase = weapon->base;
            weapon->ammo = weaponBase->frame.weapon.ammo;
            int32_t r = weaponBase->frame.weapon.range;
            if ((r > 0) && (weaponBase->frame.weapon.usage & kUseForAttacking)) {
                if (r > longestWeaponRange) {
                    longestWeaponRange = r;
                }
                if (r < shortestWeaponRange) {
                    shortestWeaponRange = r;
                }
            }
        }
        weapon->time = weapon->position = 0;
    }

    // if we don't have any weapon, then shortest range is 0 too
    if (longestWeaponRange == 0) {
        shortestWeaponRange = 0;
    }
    engageRange = kEngageRange;
    if (longestWeaponRange > kEngageRange) {
        engageRange = longestWeaponRange;
    }
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

void ChangeObjectBaseType(
        spaceObjectType *obj, int32_t whichBaseObject, int32_t spriteIDOverride,
        bool relative) {
    baseObjectType  *base = mGetBaseObjectPtr(whichBaseObject);
    int16_t         angle;
    int32_t         r;
    NatePixTable* spriteTable;

#ifdef DATA_COVERAGE
    covered_objects.insert(whichBaseObject);
    for (int32_t weapon: {base->pulse.base, base->beam.base, base->special.base}) {
        if (weapon != kNoWeapon) {
            covered_objects.insert(weapon);
        }
    }
#endif  // DATA_COVERAGE

    obj->attributes =
        base->attributes
        | (obj->attributes & (kIsHumanControlled | kIsRemote | kIsPlayerShip | kStaticDestination));
    obj->baseType = base;
    obj->whichBaseObject = whichBaseObject;
    obj->tinySize = base->tinySize;
    obj->shieldColor = base->shieldColor;
    obj->layer = base->pixLayer;

    if (obj->attributes & kCanTurn) {
        obj->directionGoal = obj->turnFraction = obj->turnVelocity = 0;
    } else if (obj->attributes & kIsSelfAnimated) {
        obj->frame.animation.thisShape = base->frame.animation.frameShape;
        if (base->frame.animation.frameShapeRange > 0) {
            r = obj->randomSeed.next(base->frame.animation.frameShapeRange);
            obj->frame.animation.thisShape += r;
        }
        obj->frame.animation.frameDirection = base->frame.animation.frameDirection;
        if (base->frame.animation.frameDirectionRange == -1) {
            if (obj->randomSeed.next(2) == 1) {
                obj->frame.animation.frameDirection = 1;
            }
        } else if (base->frame.animation.frameDirectionRange > 0) {
            obj->frame.animation.frameDirection += obj->randomSeed.next(
                base->frame.animation.frameDirectionRange);
        }
        obj->frame.animation.frameFraction = 0;
        obj->frame.animation.frameSpeed = base->frame.animation.frameSpeed;
    }

    obj->maxVelocity = base->maxVelocity;

    obj->age = base->initialAge + obj->randomSeed.next(base->initialAgeRange);

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
    obj->periodicTime = 0;
    if (base->activatePeriod) {
        obj->periodicTime = base->activatePeriod + obj->randomSeed.next(base->activatePeriodRange);
    }

    obj->pulse.type = base->pulse.base;
    obj->beam.type = base->beam.base;
    obj->special.type = base->special.base;
    obj->pulse.base = obj->beam.base = obj->special.base = NULL;
    obj->longestWeaponRange = 0;
    obj->shortestWeaponRange = kMaximumRelevantDistance;

    for (auto* weapon: {&obj->pulse, &obj->beam, &obj->special}) {
        if (weapon->type == kNoWeapon) {
            weapon->time = 0;
            continue;
        }

        weapon->base = mGetBaseObjectPtr(weapon->type);
        if (!relative) {
            weapon->ammo = weapon->base->frame.weapon.ammo;
            weapon->position = 0;
            if (weapon->time < 0) {
                weapon->time = 0;
            } else if (weapon->time > weapon->base->frame.weapon.fireTime) {
                weapon->time = weapon->base->frame.weapon.fireTime;
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
    if (obj->longestWeaponRange == 0) obj->shortestWeaponRange = 0;
    if (obj->longestWeaponRange > kEngageRange) {
        obj->engageRange = obj->longestWeaponRange;
    } else {
        obj->engageRange = kEngageRange;
    }

    // HANDLE THE NEW SPRITE DATA:
    if (obj->pixResID != kNoSpriteTable) {
        spriteTable = GetPixTable(obj->pixResID);

        if (spriteTable == NULL) {
            throw Exception("Couldn't load a requested sprite");
            spriteTable = AddPixTable(obj->pixResID);
        }

        obj->sprite->table = spriteTable;
        obj->sprite->tinySize = base->tinySize;
        obj->sprite->whichLayer = base->pixLayer;
        obj->sprite->scale = base->naturalScale;

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

int32_t CreateAnySpaceObject(
        int32_t whichBase, fixedPointType *velocity, coordPointType *location, int32_t direction,
        int32_t owner, uint32_t specialAttributes, int16_t spriteIDOverride) {
    spaceObjectType newObject;
    newObject.init(
            whichBase, {gRandomSeed.next(32766)}, direction, velocity, owner, spriteIDOverride);
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
    madeObject->baseType->create.run(madeObject, NULL, NULL);
    return newObjectNumber;
}

int32_t CountObjectsOfBaseType(int32_t whichType, int32_t owner) {
    int32_t result = 0;
    for (int32_t i = 0; i < kMaxSpaceObject; ++i) {
        auto anObject = mGetSpaceObjectPtr(i);
        if (anObject->active
                && ((whichType == -1) || (anObject->whichBaseObject == whichType))
                && ((owner == -1) || (anObject->owner == owner))) {
            ++result;
        }
    }
    return result;
}

void spaceObjectType::alter_health(int32_t amount) {
    if (amount <= 0) {
        _health += amount;
    } else if (_health >= (2147483647 - amount)) {
        _health = 2147483647;
    } else {
        _health += amount;
    }
    if (_health < 0) {
        DestroyObject(this);
    }
}

void spaceObjectType::alter_energy(int32_t amount) {
    _energy += amount;
    if (_energy < 0) {
        _energy = 0;
    } else if (_energy > max_energy()) {
        alter_battery(_energy - max_energy());
        _energy = max_energy();
    }
}

void spaceObjectType::alter_battery(int32_t amount) {
    _battery += amount;
    if (_battery > max_battery()) {
        PayAdmiral(owner, _battery - max_battery());
        _battery = max_battery();
    }
}

bool spaceObjectType::collect_warp_energy(int32_t amount) {
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

void spaceObjectType::refund_warp_energy() {
    alter_battery(warpEnergyCollected);
    warpEnergyCollected = 0;
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
            SetAdmiralConsiderObject(owner, object->number());
        }

        if (GetAdmiralBuildAtObject(owner) < 0) {
            if (BaseHasSomethingToBuild(object->number())) {
                SetAdmiralBuildAtObject(owner, object->number());
            }
        }
        if (GetAdmiralDestinationObject(owner) < 0) {
            SetAdmiralDestinationObject(owner, object->number(), kObjectDestinationType);
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
        if ((fixObject.destinationObject == object->number())
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
        object->_health = object->max_health();
        // if anyone is targeting it, they should stop
        for (int16_t i = 0; i < kMaxSpaceObject; i++) {
            auto& fixObject = gSpaceObjectData[i];
            if ((fixObject.attributes & kCanAcceptDestination)
                    && (fixObject.active != kObjectAvailable)) {
                if (fixObject.targetObjectNumber == object->number()) {
                    fixObject.targetObjectNumber = kNoDestinationObject;
                }
            }
        }

        AlterObjectOwner(object, -1, true);
        object->attributes &= ~(kHated | kCanEngage | kCanCollide | kCanBeHit);
        object->baseType->destroy.run(object, NULL, NULL);
    } else {
        AddKillToAdmiral(object);
        if (object->attributes & kReleaseEnergyOnDeath) {
            int16_t energyNum = object->energy() / kEnergyPodAmount;
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
                    if (fixObject.destinationObject == object->number()) {
                        fixObject.destinationObject = kNoDestinationObject;
                        fixObject.destObjectPtr = NULL;
                        fixObject.attributes &= ~kStaticDestination;
                    }
                }
            }
        }

        object->baseType->destroy.run(object, NULL, NULL);

        if (object->attributes & kCanAcceptDestination) {
            RemoveObjectFromDestination(object);
        }
        if (!object->baseType->destroyDontDie) {
            object->active = kObjectToBeFreed;
        }
    }
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

int32_t spaceObjectType::number() const {
    return this - mGetSpaceObjectPtr(0);
}

spaceObjectType::spaceObjectType() {
    memset(this, 0, sizeof(*this));
}

spaceObjectType::spaceObjectType(baseObjectType* base):
        spaceObjectType() {
    baseType = base;
}

}  // namespace antares

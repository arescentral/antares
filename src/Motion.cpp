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

// Motion.c

#include "Motion.hpp"

//#include <math routines.h>
#include <QDOffscreen.h>
#include <Timer.h>

#include "AresGlobalType.hpp"
#include "ConditionalMacros.h"
#include "Debug.hpp"
#include "Error.hpp"
#include "GXMath.h"
#include "MathMacros.hpp"
#include "NatePixTable.hpp"
#include "NonPlayerShip.hpp"
#include "OffscreenGWorld.hpp"
#include "PlayerShip.hpp"
#include "Randomize.hpp"
#include "Resources.h"
#include "Rotation.hpp"
#include "ScreenLabel.hpp"
#include "SoundFX.hpp"
#include "SpaceObject.hpp"
#include "SpaceObjectHandling.hpp"
#include "SpriteHandling.hpp"
#include "ToolUtils.h"
#include "UniverseUnit.hpp"

#define kCenterScaleSize    983040L//61440L         // = 240 * SCALE_SCALE
#define kMotionError        "\pMOTN"

#define kUnitsToCheckNumber     5

#define H_UR                1
#define H_DR                2
#define H_UL                3
#define H_DL                4
#define V_DL                5
#define V_DR                6
#define V_UL                7
#define V_UR                8
#define kNoDir              -1

#define kConsiderDistanceAttributes (kCanCollide | kCanBeHit | kIsDestination | kCanThink | kConsiderDistance | kCanBeEvaded | kIsHumanControlled | kIsRemote)

#define kSpriteMaxSize      2048    // for calcing whether we need to worry about whether or not sprite is visable

//#ifndef powerc
//#define   kUniverseTopLeft        (kUniversalCenter - 32768)
//#define   kUniverseBottomRight    (kUniversalCenter + 32768)
//#else
#define kUniverseTopLeft        (kUniversalCenter - 65534)
#define kUniverseBottomRight    (kUniversalCenter + 65534)
//#endif
#define kThinkiverseTopLeft     (kUniversalCenter - 131068) // universe for thinking or owned objects
#define kThinkiverseBottomRight (kUniversalCenter + 131068)

struct adjacentUnitType {
    long                    adjacentUnit;           // the normal adjacent unit
    longPointType           superOffset;            // the offset of the super unit (for wrap-around)
};

struct proximityUnitType {
    spaceObjectTypePtr      nearObject;                         // for collision checking
    spaceObjectTypePtr      farObject;                          // for distance checking
    adjacentUnitType        unitsToCheck[kUnitsToCheckNumber];  // adjacent units to check
};

extern aresGlobalType   *gAresGlobal;
extern GWorldPtr        gOffWorld, gRealWorld, gSaveWorld;
extern Handle           gPixTable[];
extern PixMapHandle     thePixMapHandle;
extern long             /*gAresGlobal->gPlayerShipNumber, gAresGlobal->gGameTime,*/
                        gAbsoluteScale, CLIP_LEFT, CLIP_TOP,
                        CLIP_RIGHT, CLIP_BOTTOM, gPlayScreenWidth, gPlayScreenHeight,
                        /*gAresGlobal->gZoomMode,*/
                        gRootObjectNumber;
//extern long               gAresGlobal->gPlayerAdmiralNumber;
extern spaceObjectType  *gScrollStarObject, *gRootObject;

static longPointType    cAdjacentUnits[] = {
                                            { 0, 0 },
                                            { 1, 0 },
                                            { -1, 1 },
                                            { 0, 1},
                                            { 1, 1}
                                            };

coordPointType          gGlobalCorner;
//long                  gAresGlobal->gClosestObject = 0, gAresGlobal->gFarthestObject = 0, gAresGlobal->gCenterScaleH = 0, gAresGlobal->gCenterScaleV = 0;
//Handle                    gAresGlobal->gProximityGrid = nil;

// HACK TO FIND BUG:
Boolean gHackMoitionInitedYet = FALSE;

int InitMotion( void)

{
    short                   x, y, i;
    proximityUnitType       *p;
    long                    adjacentAdd = 0, ux, uy, sx, sy;

    gAresGlobal->gCenterScaleH = (gPlayScreenWidth / 2) * SCALE_SCALE;
    gAresGlobal->gCenterScaleV = (gPlayScreenHeight / 2) * SCALE_SCALE;

    gAresGlobal->gProximityGrid.create(kProximityGridDataLength);
    if (gAresGlobal->gProximityGrid.get() == nil)
    {
        ShowErrorAny( eQuitErr, kErrorStrID, nil, nil, nil, nil, MEMORY_ERROR, -1, -1, -1, __FILE__, 1);
        return( MEMORY_ERROR);
    }

//  mHandleLockAndRegister( gAresGlobal->gProximityGrid, nil, nil, nil)

    // initialize the proximityGrid & set up the needed lookups (see Notebook 2 p.34)
    p = *gAresGlobal->gProximityGrid;
    for ( y = 0; y < kProximitySuperSize; y++)
    {
        for ( x = 0; x < kProximitySuperSize; x++)
        {
            p->nearObject = p->farObject = nil;
            adjacentAdd = 0;
            for ( i = 0; i < kUnitsToCheckNumber; i++)
            {
//              if (( y == 0) && ( x == 1) && ( i == 1)) Debugger();

                ux = x;
                uy = y;
                sx = sy = 0;

                ux += cAdjacentUnits[i].h;
                if ( ux < 0)
                {
                    ux += kProximitySuperSize;
                    sx--;
                } else if ( ux >= kProximitySuperSize)
                {
                    ux -= kProximitySuperSize;
                    sx++;
                }

                uy += cAdjacentUnits[i].v;
                if ( uy < 0)
                {
                    uy += kProximitySuperSize;
                    sy--;
                } else if ( uy >= kProximitySuperSize)
                {
                    uy -= kProximitySuperSize;
                    sy++;
                }
                p->unitsToCheck[i].adjacentUnit = (uy << kProximityWidthMultiply) + ux;
                p->unitsToCheck[i].adjacentUnit -= adjacentAdd;


//              WriteDebugLong( p->unitsToCheck[i].adjacentUnit);

                adjacentAdd += p->unitsToCheck[i].adjacentUnit;

                p->unitsToCheck[i].superOffset.h = sx;
                p->unitsToCheck[i].superOffset.v = sy;
            }
            p++;
        }
    }
    gHackMoitionInitedYet = TRUE;
    return ( kNoError);
}

void ResetMotionGlobals( void)
{
    proximityUnitType   *proximityObject;
    long                i;

    gGlobalCorner.h = gGlobalCorner.v = 0;
    gAresGlobal->gClosestObject = 0;
    gAresGlobal->gFarthestObject = 0;

    proximityObject = *gAresGlobal->gProximityGrid;
    for ( i = 0; i < kProximityGridDataLength; i++)
    {
        proximityObject->nearObject = proximityObject->farObject = nil;
        proximityObject++;
    }
}

void HackCheckProxGrid( long sayswho)

{
    if ( gHackMoitionInitedYet)
    {

    proximityUnitType       *p = *gAresGlobal->gProximityGrid;
    long                    count, c2;


    for ( count = 0; count < kProximityGridDataLength; count++)
    {
        for ( c2 = 0; c2 < kUnitsToCheckNumber; c2++)
        {
            if (( p->unitsToCheck[c2].adjacentUnit > 256) || ( p->unitsToCheck[c2].adjacentUnit < -256))
            {
                ShowErrorRecover( SPRITE_DATA_ERROR, kMotionError, sayswho);
            }
        }
        p++;
    }

    }

}

void MotionCleanup( void)

{
    if ( gAresGlobal->gProximityGrid.get() != nil) {
        gAresGlobal->gProximityGrid.destroy();
    }
}

void MoveSpaceObjects( spaceObjectType *table, const long tableLength, const long unitsToDo)

{
    long                    i, h, v, jl;
    smallFixedType          fh, fv, fa, fb, useThrust;
    Fixed                   aFixed;
    short                   angle;
    unsigned long           shortDist, thisDist, longDist;
    spaceObjectType         *anObject;
    baseObjectType          *baseObject;

#pragma unused( table, tableLength)

    if ( unitsToDo == 0) return;

    for ( jl = 0; jl < unitsToDo; jl++)
    {
        anObject = gRootObject;
        while ( anObject != nil)
        {
            if ( anObject->active == kObjectInUse)
            {
                baseObject = anObject->baseType;

//              if  ( !( anObject->attributes & kIsStationary))
                if (( anObject->maxVelocity != 0) || ( anObject->attributes & kCanTurn))
                {
                    if ( anObject->attributes & kCanTurn)
                    {
                        anObject->turnFraction += anObject->turnVelocity;

                        if ( anObject->turnFraction >= 0)
                            h = ( anObject->turnFraction +
                                kFixedPlusPointFive) >> kFixedBitShiftNumber;
                        else
                            h = (( anObject->turnFraction -
                                kFixedPlusPointFive) >> kFixedBitShiftNumber) + 1;
                        anObject->direction += h;
                        anObject->turnFraction -= mLongToFixed(h);

                        while ( anObject->direction >= ROT_POS)
                            anObject->direction -= ROT_POS;
                        while ( anObject->direction < 0)
                            anObject->direction += ROT_POS;
                    }

                    if ( anObject->thrust != 0)
                    {
                        if ( anObject->thrust > 0)
                        {
                            // get the goal dh & dv

                            mGetRotPoint( fa, fb, anObject->direction);

                            // multiply by max velocity

                            if (/*( anObject->presenceState == kWarpInPresence) ||*/
                                ( anObject->presenceState == kWarpingPresence) ||
                                ( anObject->presenceState == kWarpOutPresence))
                            {
                                fa = mMultiplyFixed( fa, anObject->presenceData);
                                fb = mMultiplyFixed( fb, anObject->presenceData);
                            } else
                            {
                                fa = mMultiplyFixed( anObject->maxVelocity, fa);
                                fb = mMultiplyFixed( anObject->maxVelocity, fb);
                            }

                            // the difference between our actual vector and our goal vector is our new vector

                            fa = fa - anObject->velocity.h;
                            fb = fb - anObject->velocity.v;

                            useThrust = anObject->thrust;
                        } else
                        {
                            fa = -anObject->velocity.h;
                            fb = -anObject->velocity.v;
//                              useThrust = -(anObject->thrust>>1L);
                            useThrust = -anObject->thrust;
                        }

                        // get the angle of our new vector

                        if ( fa == 0)
                        {
                            if ( fb < 0)
                                angle = 180;
                            else angle = 0;
                        } else
                        {
                            #ifdef powerc
                            aFixed = MyFixRatio(fa, fb);
                            #else
                            aFixed = FixRatio(fa, fb);
                            #endif

                            angle = AngleFromSlope( aFixed);
                            if ( fa > 0) angle += 180;
                            if ( angle >= 360) angle -= 360;
                        }

                        // get the maxthrust of new vector

                        mGetRotPoint( fh, fv, angle);

                        fh = mMultiplyFixed( useThrust, fh);
                        fv = mMultiplyFixed( useThrust, fv);

                        // if our new vector excedes our max thrust, it must be limited

                        if ( fh < 0)
                        {
                            if ( fa < fh)
                                fa = fh;
                        } else
                        {
                            if ( fa > fh)
                                fa = fh;
                        }

                        if ( fv < 0)
                        {
                            if ( fb < fv)
                                fb = fv;
                        } else
                        {
                            if ( fb > fv)
                                fb = fv;
                        }

                        anObject->velocity.h += fa;
                        anObject->velocity.v += fb;

                    }

                    anObject->motionFraction.h += anObject->velocity.h;
                    anObject->motionFraction.v += anObject->velocity.v;

                    if ( anObject->motionFraction.h >= 0)
                        h = ( anObject->motionFraction.h + kFixedPlusPointFive) >> kFixedBitShiftNumber;
                    else
                        h = (( anObject->motionFraction.h - kFixedPlusPointFive) >> kFixedBitShiftNumber) + 1;
                    anObject->location.h -= h;
                    anObject->motionFraction.h -= mLongToFixed(h);

                    if ( anObject->motionFraction.v >= 0)
                        v = ( anObject->motionFraction.v + kFixedPlusPointFive) >> kFixedBitShiftNumber;
                    else
                        v = (( anObject->motionFraction.v - kFixedPlusPointFive) >> kFixedBitShiftNumber) + 1;
                    anObject->location.v -= v;
                    anObject->motionFraction.v -= mLongToFixed(v);

                } // if ( object is not stationary)

//              if ( anObject->attributes & kIsPlayerShip)
                if ( anObject == gScrollStarObject)
                {
    //              h = kCenterScaleSize / gAbsoluteScale;
                    gGlobalCorner.h = anObject->location.h - (gAresGlobal->gCenterScaleH / gAbsoluteScale);
                    gGlobalCorner.v = anObject->location.v - (gAresGlobal->gCenterScaleV / gAbsoluteScale);
                }

                // check to see if it's out of bounds

/*              if (( anObject->owner < 0) && ( !(anObject->attributes & kIsDestination)))
                {
                    if (( anObject->location.h < kUniverseTopLeft) ||
                        ( anObject->location.v < kUniverseTopLeft) ||
                        ( anObject->location.h > kUniverseBottomRight) ||
                        ( anObject->location.v > kUniverseBottomRight))
                    {
                        anObject->active = kObjectToBeFreed;
                    }
                } else
                {
                    if ( !(anObject->attributes & kCanThink))
*/              {
                    if ( !(anObject->attributes & kDoesBounce))
                    {
                        if (( anObject->location.h < kThinkiverseTopLeft) ||
                            ( anObject->location.v < kThinkiverseTopLeft) ||
                            ( anObject->location.h > kThinkiverseBottomRight) ||
                            ( anObject->location.v > kThinkiverseBottomRight))
                        {
                            anObject->active = kObjectToBeFreed;
                        }
                    } else
                    {
                        if ( anObject->location.h < kThinkiverseTopLeft)
                        {
                            anObject->location.h = kThinkiverseTopLeft;
                            anObject->velocity.h = -anObject->velocity.h;
                        } else if ( anObject->location.h > kThinkiverseBottomRight)
                        {
                            anObject->location.h = kThinkiverseBottomRight;
                            anObject->velocity.h = -anObject->velocity.h;
                        }
                        if ( anObject->location.v < kThinkiverseTopLeft)
                        {
                            anObject->location.v = kThinkiverseTopLeft;
                            anObject->velocity.v = -anObject->velocity.v;
                        } else if ( anObject->location.v > kThinkiverseBottomRight)
                        {
                            anObject->location.v = kThinkiverseBottomRight;
                            anObject->velocity.v = -anObject->velocity.v;
                        }

                    }
                }

                // deal with self-animating shapes
                if ( anObject->attributes & kIsSelfAnimated)
                {
                    if ( baseObject->frame.animation.frameSpeed != 0)
                    {
                        anObject->frame.animation.thisShape +=
                            anObject->frame.animation.frameDirection *
                            anObject->frame.animation.frameSpeed;// * unitsToDo;

                        i = 1;
                        while (( anObject->frame.animation.thisShape >
                            baseObject->frame.animation.lastShape) &&
                            ( anObject->frame.animation.frameDirection > 0) &&
                            ( i))
                        {
                            if ( anObject->attributes & kAnimationCycle)
                            {
                                anObject->frame.animation.thisShape -=
                                    ( baseObject->frame.animation.lastShape -
                                    baseObject->frame.animation.firstShape) +
                                    1;

                            } else
                            {
                                i = 0;
                                anObject->active = kObjectToBeFreed;
                                anObject->frame.animation.thisShape =
                                    baseObject->frame.animation.lastShape;
                            }
                        }

                        while (( anObject->frame.animation.thisShape <
                            baseObject->frame.animation.firstShape) &&
                            ( anObject->frame.animation.frameDirection < 0) &&
                            ( i))
                        {
                            if ( anObject->attributes & kAnimationCycle)
                            {
                                anObject->frame.animation.thisShape +=
                                    ( baseObject->frame.animation.lastShape -
                                    baseObject->frame.animation.firstShape) + 1;

                            } else
                            {
                                i = 0;
                                anObject->active = kObjectToBeFreed;
                                anObject->frame.animation.thisShape = baseObject->frame.animation.lastShape;
                            }
                        }
                    }
                } else if ( anObject->attributes & kIsBeam)
                {
                    if ( anObject->frame.beam.beam != nil)
                    {
                        anObject->frame.beam.beam->objectLocation =
                            anObject->location;
                        if (( anObject->frame.beam.beam->beamKind ==
                                eStaticObjectToObjectKind) ||
                                ( anObject->frame.beam.beam->beamKind ==
                                eBoltObjectToObjectKind))
                        {
                            if ( anObject->frame.beam.beam->toObject != nil)
                            {
                                spaceObjectType *target =
                                    anObject->frame.beam.beam->toObject;

                                if ((target->active) &&
                                    (target->id == anObject->frame.beam.beam->toObjectID))
                                {
                                    anObject->location =
                                        anObject->frame.beam.beam->objectLocation =
                                            target->location;
                                } else
                                {
                                    anObject->active = kObjectToBeFreed;
                                }
                            }

                            if ( anObject->frame.beam.beam->fromObject != nil)
                            {
                                spaceObjectType *target =
                                    anObject->frame.beam.beam->fromObject;

                                if ((target->active) &&
                                    ( target->id == anObject->frame.beam.beam->fromObjectID))

                                {
                                    anObject->frame.beam.beam->lastGlobalLocation =
                                        anObject->frame.beam.beam->lastApparentLocation =
                                            target->location;
                                } else
                                {
                                    anObject->active = kObjectToBeFreed;
                                }
                            }
                        } else if (( anObject->frame.beam.beam->beamKind ==
                                eStaticObjectToRelativeCoordKind) ||
                                ( anObject->frame.beam.beam->beamKind ==
                                eBoltObjectToRelativeCoordKind))
                        {
                            if ( anObject->frame.beam.beam->fromObject != nil)
                            {
                                spaceObjectType *target =
                                    anObject->frame.beam.beam->fromObject;

                                if (( target->active) &&
                                    ( target->id == anObject->frame.beam.beam->fromObjectID))
                                {
                                    anObject->frame.beam.beam->lastGlobalLocation =
                                        anObject->frame.beam.beam->lastApparentLocation =
                                            target->location;

                                    anObject->location.h =
                                        anObject->frame.beam.beam->objectLocation.h =
                                        target->location.h +
                                        anObject->frame.beam.beam->toRelativeCoord.h;

                                    anObject->location.v =
                                        anObject->frame.beam.beam->objectLocation.v =
                                        target->location.v +
                                        anObject->frame.beam.beam->toRelativeCoord.v;
                                } else
                                {
                                    anObject->active = kObjectToBeFreed;
                                }
                            }
                        } else
                        {
    //                      anObject->frame.beam.beam->endLocation
                        }
                    } else MyDebugString( "\pUnexpected error: a beam appears to be missing.");
                }
            } // if (anObject->active)
            anObject = anObject->nextObject;
        }
    }

// !!!!!!!!
// nothing below can effect any object actions (expire actions get executed)
// (but they can effect objects thinking)
// !!!!!!!!
    shortDist = thisDist = static_cast<uint32_t>(kMaximumRelevantDistanceSquared) * 2;
//  gAresGlobal->gClosestObject = 0;
//  gAresGlobal->gFarthestObject = 0;
    longDist = 0;
    anObject = gRootObject;

    while ( anObject != nil)
    {
        if ( anObject->active == kObjectInUse)
        {
            baseObject = anObject->baseType;

            if ( !(anObject->attributes & kIsBeam) && ( anObject->sprite != nil))
            {
                h = ( anObject->location.h - gGlobalCorner.h) * gAbsoluteScale;
                h >>= SHIFT_SCALE;
                if (( h > -kSpriteMaxSize) && ( h < kSpriteMaxSize))
                    anObject->sprite->where.h = h + CLIP_LEFT;
                else
                    anObject->sprite->where.h = -kSpriteMaxSize;

                h = (anObject->location.v - gGlobalCorner.v) * gAbsoluteScale;
                h >>= SHIFT_SCALE; /*+ CLIP_TOP*/;
                if (( h > -kSpriteMaxSize) && ( h < kSpriteMaxSize))
                    anObject->sprite->where.v = h;
                else
                    anObject->sprite->where.v = -kSpriteMaxSize;


                if ( anObject->hitState != 0)
                {
                    anObject->hitState -= unitsToDo << 2L;
                    if ( anObject->hitState <= 0)
                    {
                        anObject->hitState = 0;
                        anObject->sprite->style = spriteNormal;
                        anObject->sprite->styleData = 0;
                    } else
                    {
                        // we know it has sprite
                        anObject->sprite->style = spriteColor;
                        anObject->sprite->styleColor = anObject->shieldColor;
                        anObject->sprite->styleData = anObject->hitState;
                    }
                } else
                {
                    if ( anObject->cloakState > 0)
                    {
                        if ( anObject->cloakState < kCloakOnStateMax)
                        {
                            anObject->runTimeFlags |= kIsCloaked;
                            anObject->cloakState += unitsToDo << 2L;
                            if ( anObject->cloakState > kCloakOnStateMax)
                                anObject->cloakState = kCloakOnStateMax;
                        }
                        anObject->sprite->style = spriteColor;
                        anObject->sprite->styleColor = 0xff;
                        anObject->sprite->styleData = anObject->cloakState;
                        if ( anObject->owner == gAresGlobal->gPlayerAdmiralNumber)
                            anObject->sprite->styleData -=
                                anObject->sprite->styleData >> 2;
                    } else if ( anObject->cloakState < 0)
                    {
                        anObject->cloakState += unitsToDo << 2L;
                        if ( anObject->cloakState >= 0)
                        {
                            anObject->runTimeFlags &= ~kIsCloaked;
                            anObject->cloakState = 0;
                            anObject->sprite->style = spriteNormal;
                        } else
                        {
                            anObject->sprite->style = spriteColor;
                            anObject->sprite->styleColor = 0xff;
                            anObject->sprite->styleData = -anObject->cloakState;
                            if ( anObject->owner == gAresGlobal->gPlayerAdmiralNumber)
                                anObject->sprite->styleData -=
                                    anObject->sprite->styleData >> 2;
                        }
                    }
                }


                if ( anObject->attributes & kIsSelfAnimated)
                {
                    if ( baseObject->frame.animation.frameSpeed != 0)
                    {
                            anObject->sprite->whichShape =
                            anObject->frame.animation.thisShape >> kFixedBitShiftNumber;
                    }
                } else if ( anObject->attributes & kShapeFromDirection)
                {
                    angle = anObject->direction;
                    mAddAngle( angle, baseObject->frame.rotation.rotRes >> 1);
                    anObject->sprite->whichShape = angle / baseObject->frame.rotation.rotRes;
                }
            } //else MyDebugString("\pUnexpected Error: a sprite seems to be missing.");
        }
        anObject = anObject->nextObject;
    }
}

void CollideSpaceObjects( spaceObjectType *table, const long tableLength)

{
    spaceObjectType         *sObject = nil, *dObject = nil, *aObject = nil, *bObject = nil,
                            *player = nil, *taObject, *tbObject;
    long                    i = 0, j = 0, k, xs, xe, ys, ye, xd, yd, superx, supery, scaleCalc, difference;
    short                   cs, ce;
    Boolean                 beamHit;
    unsigned long           distance, dcalc/*,
                            closestDist = kMaximumRelevantDistanceSquared + kMaximumRelevantDistanceSquared*/;
    proximityUnitType       *proximityObject, *currentProximity;
    uint64_t                hackTimeStart, hackTimePassed;

    spritePix               oldStyleSprite; // a temporary hack
    long                    magicHack1 = 0, magicHack2 = 0, magicHack3 = 0;
    uint64_t                farthestDist, hugeDistance, wideScrap, closestDist;
    farthestDist = 0;
    closestDist = 0x7fffffffffffffffull;

    Microseconds( &hackTimeStart);

    // set up player info so we can find closest ship (for scaling)
    if ( gAresGlobal->gPlayerShipNumber >= 0)
        player = table + gAresGlobal->gPlayerShipNumber;
    else player = nil;
    gAresGlobal->gClosestObject = 0;
    gAresGlobal->gFarthestObject = 0;

    // reset the collision grid
    proximityObject = *gAresGlobal->gProximityGrid;
    for ( i = 0; i < kProximityGridDataLength; i++)
    {
        proximityObject->nearObject = proximityObject->farObject = nil;
        proximityObject++;
    }


    aObject = gRootObject;
    if ( aObject == nil) ExitToShell();
    magicHack1 = 123;
    magicHack3 = 0;
    while ( aObject != nil)
    {
        if ( aObject->active)
        {
            if (aObject->age >= 0)
            {
                aObject->age -= 3;
                if ( aObject->age < 0)
                {
                    if ( !(aObject->baseType->expireActionNum &
                        kDestroyActionDontDieFlag))
                        aObject->active = kObjectToBeFreed;

    //              if ( aObject->attributes & kIsBeam)
    //                  aObject->frame.beam.killMe = TRUE;

                    ExecuteObjectActions( aObject->baseType->expireAction,
                            aObject->baseType->expireActionNum
                             & kDestroyActionNotMask, aObject, nil, nil, true);

                }
            }

            if ( aObject->periodicTime > 0)
            {
                aObject->periodicTime--;
                if ( aObject->periodicTime <= 0)
                {
                    ExecuteObjectActions( aObject->baseType->activateAction, aObject->baseType->activateActionNum &
                        kPeriodicActionNotMask, aObject, nil, nil, true);
                    aObject->periodicTime = ((aObject->baseType->activateActionNum & kPeriodicActionTimeMask) >>
                        kPeriodicActionTimeShift) +
                        RandomSeeded(((aObject->baseType->activateActionNum & kPeriodicActionRangeMask) >>
                            kPeriodicActionRangeShift),
                            &(aObject->randomSeed), 'mtn1', aObject->whichBaseObject);
                }
            }
        }

        if (( player != nil) && ( player->active) && ( aObject->active))
        {
            if (aObject->attributes & kAppearOnRadar)
            {
                difference = ABS<int>( player->location.h - aObject->location.h);
                dcalc = difference;
                difference =  ABS<int>( player->location.v - aObject->location.v);
                distance = difference;

                if (( dcalc > kMaximumRelevantDistance) ||
                    ( distance > kMaximumRelevantDistance))
                {
                    wideScrap = dcalc;   // must be positive
                    MyWideMul( wideScrap, wideScrap, &hugeDistance);  // ppc automatically generates WideMultiply
                    wideScrap = distance;
                    MyWideMul( wideScrap, wideScrap, &wideScrap);
                    hugeDistance += wideScrap;
                    aObject->distanceFromPlayer = hugeDistance;
                } else
                {
                    hugeDistance = distance * distance + dcalc * dcalc;
                    aObject->distanceFromPlayer = hugeDistance;
                    /*
                    if ( distance < closestDist)
                    {
                        if (( aObject != gScrollStarObject) && (( gAresGlobal->gZoomMode != kNearestFoeZoom)
                            || ( aObject->owner != player->owner)))
                        {
                            closestDist = distance;
                            gAresGlobal->gClosestObject = aObject->entryNumber;
//                              player->targetObjectNumber = aObject->entryNumber;
                        }
                    }
                    */
                }
                if (closestDist > hugeDistance) {
                    if (( aObject != gScrollStarObject) && (( gAresGlobal->gZoomMode != kNearestFoeZoom)
                        || ( aObject->owner != player->owner)))
                    {
                        closestDist = hugeDistance;
                        gAresGlobal->gClosestObject = aObject->entryNumber;
                    }
                }
                if (hugeDistance > farthestDist) {
                    farthestDist = hugeDistance;
                    gAresGlobal->gFarthestObject = aObject->entryNumber;
                }
            }
        } else
        {
            aObject->distanceFromPlayer = 0x7fffffffffffffffull;
        }

        if (( aObject->active) && ( aObject->attributes & kConsiderDistanceAttributes))
        {
            aObject->localFriendStrength = aObject->baseType->offenseValue;
            aObject->localFoeStrength = 0;
            aObject->closestObject = -1;
            aObject->closestDistance = kMaximumRelevantDistanceSquared;
            aObject->absoluteBounds.right = aObject->absoluteBounds.left = 0;

            // xs = collision unit, xe = super unit
            xs = aObject->location.h;
            xs >>= kCollisionUnitBitShift;
            xe = xs >> kCollisionSuperExtraShift;
            xs &= kProximityUnitAndModulo;

            ys = aObject->location.v;
            ys >>= kCollisionUnitBitShift;
            ye = ys >> kCollisionSuperExtraShift;
            ys &= kProximityUnitAndModulo;

            proximityObject = *gAresGlobal->gProximityGrid + (ys << kProximityWidthMultiply) + xs;
            aObject->nextNearObject = proximityObject->nearObject;
            proximityObject->nearObject = aObject;
            aObject->collisionGrid.h = xe;
            aObject->collisionGrid.v = ye;

            xe >>= kDistanceUnitExtraShift;
            xs = xe >> kDistanceSuperExtraShift;
            xe &= kProximityUnitAndModulo;

            ye >>= kDistanceUnitExtraShift;
            ys = ye >> kDistanceSuperExtraShift;
            ye &= kProximityUnitAndModulo;

            proximityObject = *gAresGlobal->gProximityGrid + (ye << kProximityWidthMultiply) + xe;
            aObject->nextFarObject = proximityObject->farObject;
            proximityObject->farObject = aObject;
            aObject->distanceGrid.h = xs;
            aObject->distanceGrid.v = ys;

            if ( !(aObject->attributes & kIsDestination))
                aObject->seenByPlayerFlags = 0x80000000;
            aObject->runTimeFlags &= ~kIsHidden;

            if ( aObject->sprite != nil)
            {
                aObject->sprite->tinySize = aObject->tinySize;
                if ( aObject->attributes & kIsSelfAnimated)
                {
                    if ( aObject->attributes & kAnimationCycle)
                    {
                        if ( aObject->frame.animation.frameDirection > 0)
                            magicHack3 += (aObject->sprite->whichShape);
                        else
                            magicHack2 += (aObject->sprite->whichShape);
                    }
                } else
                {
//                  magicHack2 += magicHack1 * aObject->active;//(aObject->sprite->whichShape;
                }
            }
//          magicHack1 += aObject->id;
        }

        aObject = aObject->nextObject;
    }

    proximityObject = *gAresGlobal->gProximityGrid;
    for ( j = 0; j < kProximitySuperSize; j++)
    {
        for ( i = 0; i < kProximitySuperSize; i++)
        {
            aObject = proximityObject->nearObject;
            while ( aObject != nil)
            {
                taObject = aObject->nextNearObject;

                // this hack is to get the current bounds of the object in question
                // it could be sped up by accessing the sprite table directly
                if (( aObject->absoluteBounds.left >= aObject->absoluteBounds.right) &&
                    ( aObject->sprite != nil))
                {
                    GetOldSpritePixData( aObject->sprite, &oldStyleSprite);

                    scaleCalc = (oldStyleSprite.width * aObject->naturalScale);
                    scaleCalc >>= SHIFT_SCALE;
                    aObject->scaledSize.h = scaleCalc;
                    scaleCalc = (oldStyleSprite.height * aObject->naturalScale);
                    scaleCalc >>= SHIFT_SCALE;
                    aObject->scaledSize.v = scaleCalc;

                    scaleCalc = oldStyleSprite.center.h * aObject->naturalScale;
                    scaleCalc >>= SHIFT_SCALE;
                    aObject->scaledCornerOffset.h = -scaleCalc;
                    scaleCalc = oldStyleSprite.center.v * aObject->naturalScale;
                    scaleCalc >>= SHIFT_SCALE;
                    aObject->scaledCornerOffset.v = -scaleCalc;

                    aObject->absoluteBounds.left = aObject->location.h +
                                                aObject->scaledCornerOffset.h;
                    aObject->absoluteBounds.right = aObject->absoluteBounds.left +
                                                aObject->scaledSize.h;
                    aObject->absoluteBounds.top = aObject->location.v +
                                                aObject->scaledCornerOffset.v;
                    aObject->absoluteBounds.bottom = aObject->absoluteBounds.top +
                                                aObject->scaledSize.v;
                }

                currentProximity = proximityObject;
                for ( k = 0; k < kUnitsToCheckNumber; k++)
                {
                    if ( k == 0)
                    {
                        bObject = aObject->nextNearObject;
                        superx = aObject->collisionGrid.h;
                        supery = aObject->collisionGrid.v;
                    }
                    else
                    {
                        if (( proximityObject->unitsToCheck[k].adjacentUnit > 256) ||
                            ( proximityObject->unitsToCheck[k].adjacentUnit < -256))
                        {
                            ShowErrorAny( eContinueErr, kErrorStrID, nil, nil, nil, nil, kAdjacentError, -1, -1, -1, __FILE__, proximityObject->unitsToCheck[k].adjacentUnit);
                        }
                        currentProximity += proximityObject->unitsToCheck[k].adjacentUnit;
                        bObject = currentProximity->nearObject;
                        superx = aObject->collisionGrid.h + proximityObject->unitsToCheck[k].superOffset.h;
                        supery = aObject->collisionGrid.v + proximityObject->unitsToCheck[k].superOffset.v;
                    }
                    if (( superx >= 0) && ( supery >= 0))
                    {
                        while ( bObject != nil)
                        {

                            tbObject = bObject->nextNearObject;
                            // this'll be true even ONLY if BOTH objects are not non-physical dest object
                            if ((( (bObject->attributes | aObject->attributes) & kCanCollide) &&
                                (( bObject->attributes | aObject->attributes) & kCanBeHit)) &&
                                /*( bObject->owner != aObject->owner) &&*/ ( bObject->collisionGrid.h ==
                                superx) && ( bObject->collisionGrid.v == supery))
                            {
                                // this hack is to get the current bounds of the object in question
                                // it could be sped up by accessing the sprite table directly
                                if (( bObject->absoluteBounds.left >= bObject->absoluteBounds.right) &&
                                    ( bObject->sprite != nil))
                                {
                                    GetOldSpritePixData( bObject->sprite, &oldStyleSprite);

                                    scaleCalc = (oldStyleSprite.width * bObject->naturalScale);
                                    scaleCalc >>= SHIFT_SCALE;
                                    bObject->scaledSize.h = scaleCalc;
                                    scaleCalc = (oldStyleSprite.height * bObject->naturalScale);
                                    scaleCalc >>= SHIFT_SCALE;
                                    bObject->scaledSize.v = scaleCalc;

                                    scaleCalc = oldStyleSprite.center.h * bObject->naturalScale;
                                    scaleCalc >>= SHIFT_SCALE;
                                    bObject->scaledCornerOffset.h = -scaleCalc;
                                    scaleCalc = oldStyleSprite.center.v * bObject->naturalScale;
                                    scaleCalc >>= SHIFT_SCALE;
                                    bObject->scaledCornerOffset.v = -scaleCalc;

                                    bObject->absoluteBounds.left = bObject->location.h +
                                                                bObject->scaledCornerOffset.h;
                                    bObject->absoluteBounds.right = bObject->absoluteBounds.left +
                                                                bObject->scaledSize.h;
                                    bObject->absoluteBounds.top = bObject->location.v +
                                                                bObject->scaledCornerOffset.v;
                                    bObject->absoluteBounds.bottom = bObject->absoluteBounds.top +
                                                                bObject->scaledSize.v;
                                }
                                if ( aObject->owner != bObject->owner)
                                {
//                                  bObject->foeStrength  += aObject->baseType->offenseValue;
                                    if  (!(( bObject->attributes | aObject->attributes) & kIsBeam))
                                    {
                                        dObject = aObject;
                                        sObject = bObject;
                                        if (!(( sObject->absoluteBounds.right < dObject->absoluteBounds.left) ||
                                            ( sObject->absoluteBounds.left > dObject->absoluteBounds.right) ||
                                            ( sObject->absoluteBounds.bottom < dObject->absoluteBounds.top) ||
                                            ( sObject->absoluteBounds.top > dObject->absoluteBounds.bottom)))
    //                                  if ( aObject->entryNumber != 0)
                                        {
                                            if (( dObject->attributes & kCanBeHit) && ( sObject->attributes & kCanCollide))
                                                HitObject( dObject, sObject);
                                            if (( sObject->attributes & kCanBeHit) && ( dObject->attributes & kCanCollide))
                                                HitObject( sObject, dObject);
                                        }
                                    } else
                                    {
            //                          WriteDebugLong( bObject->active);
                                        if ( bObject->attributes & kIsBeam)
                                        {
                                            sObject = bObject;
                                            dObject = aObject;
                                        } else
                                        {
                                            sObject = aObject;
                                            dObject = bObject;
                                        }

                                        xs = sObject->location.h;
                                        ys = sObject->location.v;
                                        xe = sObject->frame.beam.beam->lastGlobalLocation.h;
                                        ye = sObject->frame.beam.beam->lastGlobalLocation.v;

                                        cs = mClipCode( xs, ys, dObject->absoluteBounds);
                                        ce = mClipCode( xe, ye, dObject->absoluteBounds);
                                        beamHit = TRUE;
                                        if ( sObject->active == kObjectToBeFreed)
                                        {
                                            cs = ce = 1;
                                            beamHit = FALSE;
                                        }

                                        while ( cs | ce)
                                        {
                                            if ( cs & ce)
                                            {
                                                beamHit = FALSE;
                                                break;
                                            }
                                            xd = xe - xs;
                                            yd = ye - ys;
                                            if ( cs)
                                            {
                                                if ( cs & 8)
                                                {
                                                    ys += yd * ( dObject->absoluteBounds.left - xs) / xd;
                                                    xs = dObject->absoluteBounds.left;
                                                } else
                                                if ( cs & 4)
                                                {
                                                    ys += yd * ( dObject->absoluteBounds.right - 1 - xs) / xd;
                                                    xs = dObject->absoluteBounds.right - 1;
                                                } else
                                                if ( cs & 2)
                                                {
                                                    xs += xd * ( dObject->absoluteBounds.top - ys) / yd;
                                                    ys = dObject->absoluteBounds.top;
                                                } else
                                                if ( cs & 1)
                                                {
                                                    xs += xd * ( dObject->absoluteBounds.bottom - 1 - ys) / yd;
                                                    ys = dObject->absoluteBounds.bottom - 1;
                                                }
                                                cs = mClipCode( xs, ys, dObject->absoluteBounds);
                                            } else if ( ce)
                                            {
                                                if ( ce & 8)
                                                {
                                                    ye += yd * ( dObject->absoluteBounds.left - xe) / xd;
                                                    xe = dObject->absoluteBounds.left;
                                                } else
                                                if ( ce & 4)
                                                {
                                                    ye += yd * ( dObject->absoluteBounds.right - 1 - xe) / xd;
                                                    xe = dObject->absoluteBounds.right - 1;
                                                } else
                                                if ( ce & 2)
                                                {
                                                    xe += xd * ( dObject->absoluteBounds.top - ye) / yd;
                                                    ye = dObject->absoluteBounds.top;
                                                } else
                                                if ( ce & 1)
                                                {
                                                    xe += xd * ( dObject->absoluteBounds.bottom - 1 - ye) / yd;
                                                    ye = dObject->absoluteBounds.bottom - 1;
                                                }
                                                ce = mClipCode( xe, ye, dObject->absoluteBounds);
                                            }
                                        }
                                        if ( beamHit)
                                        {
                                            HitObject( dObject, sObject);
                                        }
                                    }
                                } else
                                {
//                                  bObject->friendStrength += aObject->baseType->offenseValue;
//                                  bObject->friendStrength += kFixedOne;
                                }

                                // check to see if the 2 objects occupy same physical space
                                if  (((bObject->attributes & aObject->attributes) & kOccupiesSpace) &&
                                    ( bObject->owner != aObject->owner))
                                {
                                    dObject = aObject;
                                    sObject = bObject;
                                    if (!(( sObject->absoluteBounds.right < dObject->absoluteBounds.left) ||
                                        ( sObject->absoluteBounds.left > dObject->absoluteBounds.right) ||
                                        ( sObject->absoluteBounds.bottom < dObject->absoluteBounds.top) ||
                                        ( sObject->absoluteBounds.top > dObject->absoluteBounds.bottom)))
                                    {
                                        CorrectPhysicalSpace( aObject, bObject); // move them back till they don't touch
                                    } else
                                    {
                                        aObject->collideObject = bObject->collideObject = nil;
                                    }
                                }
                            // one or both objects is non-physical
                            } else if (( bObject->collisionGrid.h ==
                                superx) && ( bObject->collisionGrid.v == supery))
                            {
                                if ( aObject->owner != bObject->owner)
                                {
//                                  bObject->foeStrength  += aObject->baseType->offenseValue;
                                } else
                                {
//                                  bObject->friendStrength += aObject->baseType->offenseValue;
//                                  bObject->friendStrength += kFixedOne;
                                }
                            }
                            bObject = tbObject;
                        }
                    }
                }
                aObject = taObject;
            }
            proximityObject++;
        }
    }

    proximityObject = *gAresGlobal->gProximityGrid;
    for ( j = 0; j < kProximitySuperSize; j++)
    {
        for ( i = 0; i < kProximitySuperSize; i++)
        {
            aObject = proximityObject->farObject;
            while ( aObject != nil)
            {
//              aObject->friendStrength += aObject->baseType->offenseValue;
//              aObject->friendStrength += kFixedOne;
                taObject = aObject->nextFarObject;
                currentProximity = proximityObject;
                for ( k = 0; k < kUnitsToCheckNumber; k++)
                {
                    if ( k == 0)
                    {
                        bObject = aObject->nextFarObject;
                        superx = aObject->distanceGrid.h;
                        supery = aObject->distanceGrid.v;
                    }
                    else
                    {
                        currentProximity += proximityObject->unitsToCheck[k].adjacentUnit;
                        bObject = currentProximity->farObject;
                        superx = aObject->distanceGrid.h + proximityObject->unitsToCheck[k].superOffset.h;
                        supery = aObject->distanceGrid.v + proximityObject->unitsToCheck[k].superOffset.v;
                    }
                    if (( superx >= 0) && ( supery >= 0))
                    {
                        while ( bObject != nil)
                        {
                            tbObject = bObject->nextFarObject;
                            if (( bObject->owner != aObject->owner) && ( bObject->distanceGrid.h ==
                                superx) && ( bObject->distanceGrid.v == supery) &&
                                (( bObject->attributes & kCanThink) ||
                                ( bObject->attributes & kRemoteOrHuman) ||
                                ( bObject->attributes & kHated)) &&
                                (( aObject->attributes & kCanThink) ||
                                ( aObject->attributes & kRemoteOrHuman) ||
                                ( aObject->attributes & kHated)) /*&&
                                ( !(( aObject->attributes & bObject->attributes) & kIsGuided))*/)
                            {
                                difference = ABS<int>( bObject->location.h - aObject->location.h);
                                dcalc = difference;
                                difference =  ABS<int>( bObject->location.v - aObject->location.v);
                                distance = difference;
                                if (( dcalc > kMaximumRelevantDistance) ||
                                    ( distance > kMaximumRelevantDistance))
                                    distance = kMaximumRelevantDistanceSquared;
                                else distance = distance * distance + dcalc * dcalc;

                                if ( distance < kMaximumRelevantDistanceSquared)
                                {
                                    aObject->seenByPlayerFlags |= bObject->myPlayerFlag;
                                    bObject->seenByPlayerFlags |= aObject->myPlayerFlag;

                                    if ( bObject->attributes & kHideEffect)
                                    {
                                        aObject->runTimeFlags |= kIsHidden;
                                    }

                                    if ( aObject->attributes & kHideEffect)
                                    {
                                        bObject->runTimeFlags |= kIsHidden;
                                    }
                                }

                                if  (
                                        (
                                            (aObject->baseType->buildFlags & kCanOnlyEngage) ||
                                            (bObject->baseType->buildFlags & kOnlyEngagedBy)
                                        ) &&
                                        (
                                            (
                                                (aObject->baseType->buildFlags & kEngageKeyTagMask)
                                                << kEngageKeyTagShift
                                            ) !=
                                            (
                                                bObject->baseType->buildFlags & kLevelKeyTagMask
                                            )
                                        )
                                    ) goto hackANoEngageMatch;

                                if (( distance < aObject->closestDistance) && (bObject->attributes & kPotentialTarget))
                                {
                                    aObject->closestDistance = distance;
                                    aObject->closestObject = bObject->entryNumber;
                                }

                            hackANoEngageMatch:
                                if  (
                                        (
                                            (bObject->baseType->buildFlags & kCanOnlyEngage) ||
                                            (aObject->baseType->buildFlags & kOnlyEngagedBy)
                                        ) &&
                                        (
                                            (
                                                (bObject->baseType->buildFlags & kEngageKeyTagMask)
                                                << kEngageKeyTagShift
                                            ) !=
                                            (
                                                aObject->baseType->buildFlags & kLevelKeyTagMask
                                            )
                                        )
                                    ) goto hackBNoEngageMatch;

                                if (( distance < bObject->closestDistance) && ( aObject->attributes & kPotentialTarget))
                                {
                                    bObject->closestDistance = distance;
                                    bObject->closestObject = aObject->entryNumber;
                                }
                            hackBNoEngageMatch:
                                bObject->localFoeStrength += aObject->localFriendStrength;
                                bObject->localFriendStrength += aObject->localFoeStrength;

                            } else if (( bObject->distanceGrid.h ==
                                superx) && ( bObject->distanceGrid.v == supery) && ( k == 0))
                            {
                                if ( aObject->owner != bObject->owner)
                                {
                                    bObject->localFoeStrength += aObject->localFriendStrength;
                                    bObject->localFriendStrength += aObject->localFoeStrength;
                                } else
                                {
                                    bObject->localFoeStrength += aObject->localFoeStrength;
                                    bObject->localFriendStrength += aObject->localFriendStrength;
                                }
                            }
                            bObject = tbObject;
                        }
                    }
                }
                aObject = taObject;
            }
            proximityObject++;
        }
    }

    Microseconds( &hackTimePassed);
    hackTimePassed -= hackTimeStart;
//  WriteDebugLong( hackTimePassed.as_struct.lo);

// here, it doesn't matter in what order we step through the table
    aObject = table;
    dcalc = 1ul << gAresGlobal->gPlayerAdmiralNumber;

    for ( i = 0; i < tableLength; i++)
    {
        if (aObject->active == kObjectToBeFreed)
        {
            if ( aObject->attributes & kIsBeam)
            {
                if ( aObject->frame.beam.beam != nil)
                {
                    aObject->frame.beam.beam->killMe = TRUE;
                }
                aObject->active = kObjectAvailable;
                aObject->attributes = 0;
                aObject->nextNearObject = aObject->nextFarObject = nil;
                if ( aObject->previousObject != nil)
                {
                    bObject = aObject->previousObject;
                    bObject->nextObject = aObject->nextObject;
                    bObject->nextObjectNumber = aObject->nextObjectNumber;
                }
                if ( aObject->nextObject != nil)
                {
                    bObject = aObject->nextObject;
                    bObject->previousObject = aObject->previousObject;
                    bObject->previousObjectNumber = aObject->previousObjectNumber;
                }
                if ( gRootObject == aObject)
                {
                    gRootObject = aObject->nextObject;
                    gRootObjectNumber = aObject->nextObjectNumber;
                }
                aObject->nextObject = nil;
                aObject->nextObjectNumber = -1;
                aObject->previousObject = nil;
                aObject->previousObjectNumber = -1;
            }else
            {
                aObject->active = kObjectAvailable;
                if ( aObject->sprite != nil)
                {
                    aObject->sprite->killMe = TRUE;
                }
                aObject->attributes = 0;
                aObject->nextNearObject = aObject->nextFarObject = nil;
                if ( aObject->previousObject != nil)
                {
                    bObject = aObject->previousObject;
                    bObject->nextObject = aObject->nextObject;
                    bObject->nextObjectNumber = aObject->nextObjectNumber;
                }
                if ( aObject->nextObject != nil)
                {
                    bObject = aObject->nextObject;
                    bObject->previousObject = aObject->previousObject;
                    bObject->previousObjectNumber = aObject->previousObjectNumber;
                }
                if ( gRootObject == aObject)
                {
                    gRootObject = aObject->nextObject;
                    gRootObjectNumber = aObject->nextObjectNumber;
                }
                aObject->nextObject = nil;
                aObject->nextObjectNumber = -1;
                aObject->previousObject = nil;
                aObject->previousObjectNumber = -1;
            }

        }
        if ( aObject->active)
        {
            if (( aObject->attributes & kConsiderDistanceAttributes) &&
                (!(aObject->attributes & kIsDestination)))
            {
                if ( aObject->runTimeFlags & kIsCloaked)
                    aObject->seenByPlayerFlags = 0;
                else if (!(aObject->runTimeFlags & kIsHidden))
                    aObject->seenByPlayerFlags = 0xffffffff;
                aObject->seenByPlayerFlags |= aObject->myPlayerFlag;
                if ((!(aObject->seenByPlayerFlags & dcalc)) &&
                    ( aObject->sprite != nil))
                {
                    aObject->sprite->tinySize = 0;
                }
            }
            if ( aObject->attributes & kIsBeam)
            {
                aObject->frame.beam.beam->lastGlobalLocation = aObject->location;
            }
        }
        aObject->lastLocation = aObject->location;
        aObject->lastDir = aObject->direction;
        aObject++;
    }

}

// CorrectPhysicalSpace-- takes 2 objects that are colliding and moves them back 1
//  bresenham-style step at a time to their previous locations or until they don't
//  collide.  For keeping objects which occupy space from occupying the
//  same space.

void CorrectPhysicalSpace( spaceObjectType *aObject, spaceObjectType *bObject)

{
    long    ah, av, ad, bh, bv, bd, adir = kNoDir, bdir = kNoDir,
            h, v;
    fixedPointType  tvel;
    smallFixedType  force, totalMass, tfix;
    short           angle;
    Fixed           aFixed;


/*  DebugFileAppendString( "\pPY\t");
    DebugFileAppendLong( aObject->entryNumber);
    DebugFileAppendString( "\p\t");
    DebugFileAppendLong( bObject->entryNumber);
    DebugFileAppendString( "\p\t");
    DebugFileAppendLong( (aObject->location.h - bObject->location.h) + ( aObject->location.v - bObject->location.v));
    DebugFileAppendString( "\p\r");
*/
    // calculate the new velocities
    force = ( bObject->velocity.h - aObject->velocity.h);
    force = mMultiplyFixed( force, force);
    totalMass = ( bObject->velocity.v - aObject->velocity.v);
    totalMass = mMultiplyFixed( totalMass, totalMass);
    force += totalMass;
    force = lsqrt( force);  // tvel = force
    ah = bObject->location.h - aObject->location.h;
    av = bObject->location.v - aObject->location.v;

    if ( ah == 0)
    {
        if ( av < 0)
            angle = 180;
        else angle = 0;
    } else
    {
        #ifdef powerc
        aFixed = MyFixRatio(ah, av);
        #else
        aFixed = FixRatio(ah, av);
        #endif

        angle = AngleFromSlope( aFixed);
        if ( ah > 0) angle += 180;
        if ( angle >= 360) angle -= 360;
    }
    totalMass = aObject->baseType->mass + bObject->baseType->mass;  // svel = total mass
    tfix = aObject->baseType->mass;
    tfix = mMultiplyFixed( tfix, force);
    if ( totalMass == 0) tfix = -1;
    else
    {
        tfix = mDivideFixed( tfix, totalMass);
    }
    tfix += aObject->maxVelocity >> 1;
    mGetRotPoint( tvel.h, tvel.v, angle);
    tvel.h = mMultiplyFixed( tfix, tvel.h);
    tvel.v = mMultiplyFixed( tfix, tvel.v);
//  tvel.h = mMultiplyFixed( aObject->baseType->maxVelocity, tvel.h);
//  tvel.v = mMultiplyFixed( aObject->baseType->maxVelocity, tvel.v);
    aObject->velocity.v = tvel.v;
    aObject->velocity.h = tvel.h;

    mAddAngle( angle, 180);
    tfix = bObject->baseType->mass;
    tfix = mMultiplyFixed( tfix, force);
    if ( totalMass == 0) tfix = -1;
    else
    {
        tfix = mDivideFixed( tfix, totalMass);
    }
    tfix += bObject->maxVelocity >> 1;
    mGetRotPoint( tvel.h, tvel.v, angle);
    tvel.h = mMultiplyFixed( tfix, tvel.h);
    tvel.v = mMultiplyFixed( tfix, tvel.v);
//  tvel.h = mMultiplyFixed( bObject->baseType->maxVelocity, tvel.h);
//  tvel.v = mMultiplyFixed( bObject->baseType->maxVelocity, tvel.v);
    bObject->velocity.v = tvel.v;
    bObject->velocity.h = tvel.h;

    ah = aObject->location.h - aObject->absoluteBounds.left;
    ad = aObject->absoluteBounds.right - aObject->location.h;
    av = aObject->location.v - aObject->absoluteBounds.top;
    adir = aObject->absoluteBounds.bottom - aObject->location.v;

    bh = bObject->location.h - bObject->absoluteBounds.left;
    bd = bObject->absoluteBounds.right - bObject->location.h;
    bv = bObject->location.v - bObject->absoluteBounds.top;
    bdir = bObject->absoluteBounds.bottom - bObject->location.v;

    if ( (aObject->velocity.h || aObject->velocity.v || bObject->velocity.h ||
        bObject->velocity.v))
    {
        while ((!(( aObject->absoluteBounds.right   <   bObject->absoluteBounds.left) ||
                (   aObject->absoluteBounds.left    >   bObject->absoluteBounds.right) ||
                (   aObject->absoluteBounds.bottom  <   bObject->absoluteBounds.top) ||
                (   aObject->absoluteBounds.top     >   bObject->absoluteBounds.bottom))))
        {
            aObject->motionFraction.h += aObject->velocity.h;
            aObject->motionFraction.v += aObject->velocity.v;

            if ( aObject->motionFraction.h >= 0)
                h = ( aObject->motionFraction.h + kFixedPlusPointFive) >> kFixedBitShiftNumber;
            else
                h = (( aObject->motionFraction.h - kFixedPlusPointFive) >> kFixedBitShiftNumber) + 1;
            aObject->location.h -= h;
            aObject->motionFraction.h -= mLongToFixed(h);

            if ( aObject->motionFraction.v >= 0)
                v = ( aObject->motionFraction.v + kFixedPlusPointFive) >> kFixedBitShiftNumber;
            else
                v = (( aObject->motionFraction.v - kFixedPlusPointFive) >> kFixedBitShiftNumber) + 1;
            aObject->location.v -= v;
            aObject->motionFraction.v -= mLongToFixed(v);

            bObject->motionFraction.h += bObject->velocity.h;
            bObject->motionFraction.v += bObject->velocity.v;

            if ( bObject->motionFraction.h >= 0)
                h = ( bObject->motionFraction.h + kFixedPlusPointFive) >> kFixedBitShiftNumber;
            else
                h = (( bObject->motionFraction.h - kFixedPlusPointFive) >> kFixedBitShiftNumber) + 1;
            bObject->location.h -= h;
            bObject->motionFraction.h -= mLongToFixed(h);

            if ( bObject->motionFraction.v >= 0)
                v = ( bObject->motionFraction.v + kFixedPlusPointFive) >> kFixedBitShiftNumber;
            else
                v = (( bObject->motionFraction.v - kFixedPlusPointFive) >> kFixedBitShiftNumber) + 1;
            bObject->location.v -= v;
            bObject->motionFraction.v -= mLongToFixed(v);

            aObject->absoluteBounds.left = aObject->location.h - ah;
            aObject->absoluteBounds.right = aObject->location.h + ad;
            aObject->absoluteBounds.top = aObject->location.v - av;
            aObject->absoluteBounds.bottom = aObject->location.v + adir;

            bObject->absoluteBounds.left = bObject->location.h - bh;
            bObject->absoluteBounds.right = bObject->location.h + bd;
            bObject->absoluteBounds.top = bObject->location.v - bv;
            bObject->absoluteBounds.bottom = bObject->location.v + bdir;
        }
    }
}

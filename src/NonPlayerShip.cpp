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

// NonPlayer ships

#include "NonPlayerShip.hpp"

//#include <math routines.h>

#include "Admiral.hpp"
#include "AresGlobalType.hpp"
#include "AresNetwork.hpp"
#include "ColorTranslation.hpp"     // a hack
#include "ConditionalMacros.h"
#include "Debug.hpp"
#include "Error.hpp"
#include "GXMath.h"
#include "KeyCodes.hpp"
#include "KeyMapTranslation.hpp"  // HACK FOR TESTING
#include "MathMacros.hpp"
#include "MessageScreen.hpp"
#include "Motion.hpp"  // HACK FOR TESTING
#include "OffscreenGWorld.hpp"      // a hack
#include "PlayerShip.hpp"
#include "Randomize.hpp"
#include "Resources.h"
#include "Rotation.hpp"
#include "ScenarioMaker.hpp"
#include "ScreenLabel.hpp"
#include "ScrollStars.hpp"  // for resetting the player ship
#include "ShotsBeamsExplosions.hpp"
#include "SoundFX.hpp"
#include "SpaceObjectHandling.hpp"
#include "StringNumerics.hpp"
#include "TimeUnit.hpp"
#include "ToolUtils.h"
#include "Transitions.hpp"
#include "UniverseUnit.hpp"

#define kDirectionError         5           // how picky in degrees we are about angle
#define kShootAngle             15          // how picky we are about shooting in degrees
#define kParanoiaAngle          30          // angle of terror
#define kEvadeAngle             30          // we'd like to turn this far away
#define kWarpOutAngle           90          // difference from desired direction from real direction before we warp out

#define kMotionMargin           5000        // margin of change in distance before we care
#define kDestinationMargin      50000
#define kAutoPilotDistance      262144
#define kLandingDistance        1000//5000
#define kWarpOutDistance        16777216L//250000
#define kWarpInDistance         16777216L//268435456L//536870912//4000000

#define kCloserThanClosest      0x01
#define kFartherThanFarther     0x02
#define kIsFaced                0x04

#define kRechargeSpeed          4
#define kHealthRatio            5
#define kWeaponRatio            2
#define kEnergyChunk            11 // = kHealthRatio + (kWeaponRatio * 3)
#define kWarpInEnergyFactor     3

#define kDefaultTurnRate        0x00000200

#define kFriendlyColor          GREEN
#define kHostileColor           RED
#define kNeutralColor           SKY_BLUE

//#define   kUseOldThinking

extern aresGlobalType           *gAresGlobal;
extern spaceObjectType**        gSpaceObjectData;
extern baseObjectType**         gBaseObjectData;
extern Handle                   gRotTable, gColorTranslateTable;    // for getting weapon position
//extern long                       gAresGlobal->gPlayerAdmiralNumber;
extern netStatusType            gNetworkStatus;
extern long                     /*gAresGlobal->gGameTime, gAresGlobal->gPlayerShipNumber,*/ gRandomSeed,
                                /*gAresGlobal->gGameOver,*/ gRootObjectNumber/*, gAresGlobal->gScrollStarNumber*/;
extern spaceObjectType          *gScrollStarObject, *gRootObject;
extern coordPointType           gGlobalCorner; // for playing distance sound when no player object

unsigned long ThinkObjectNormalPresence( spaceObjectType *, baseObjectType *, long);
unsigned long ThinkObjectWarpingPresence( spaceObjectType *);
unsigned long ThinkObjectWarpInPresence( spaceObjectType *);
unsigned long ThinkObjectWarpOutPresence( spaceObjectType *, baseObjectType *);
unsigned long ThinkObjectLandingPresence( spaceObjectType *);
void ThinkObjectGetCoordVector( spaceObjectType *, coordPointType *, unsigned long *, short *);
void ThinkObjectGetCoordDistance( spaceObjectType *, coordPointType *, unsigned long *);
void ThinkObjectResolveDestination( spaceObjectType *, coordPointType *, spaceObjectType **);
Boolean ThinkObjectResolveTarget( spaceObjectType *, coordPointType *, unsigned long *, spaceObjectType **);
unsigned long ThinkObjectEngageTarget( spaceObjectType *, spaceObjectType *, unsigned long, short *, long);

spaceObjectType *HackNewNonplayerShip( long owner, short type, longRect *bounds)

{
#pragma unused( owner, type, bounds)
    return( nil);
}

#ifdef kUseOldThinking
#else   // if NOT kUseOldThinking
void NonplayerShipThink( long timePass)
{
    admiralType     *anAdmiral;
    spaceObjectType *anObject, *targetObject;
    baseObjectType  *baseObject, *weaponObject;
    longPointType   offset;
    long            count, difference;
    unsigned long   keysDown;
    short           h;
    smallFixedType  fcos, fsin;
    unsigned char   friendSick, foeSick, neutralSick;
    transColorType  *transColor;
    unsigned long   sickCount = gAresGlobal->gGameTime / 9;

    gAresGlobal->gSynchValue = gRandomSeed;
    sickCount &= 0x00000003;
    if ( sickCount == 0)
    {
        mGetTranslateColorShade( kFriendlyColor, MEDIUM, friendSick, transColor);
        mGetTranslateColorShade( kHostileColor, MEDIUM, foeSick, transColor);
        mGetTranslateColorShade( kNeutralColor, MEDIUM, neutralSick, transColor);
    } else if ( sickCount == 1)
    {
        mGetTranslateColorShade( kFriendlyColor, DARK, friendSick, transColor);
        mGetTranslateColorShade( kHostileColor, DARK, foeSick, transColor);
        mGetTranslateColorShade( kNeutralColor, DARK, neutralSick, transColor);
    } else if ( sickCount == 2)
    {
        mGetTranslateColorShade( kFriendlyColor, DARKER, friendSick, transColor);
        mGetTranslateColorShade( kHostileColor, DARKER, foeSick, transColor);
        mGetTranslateColorShade( kNeutralColor, DARKER, neutralSick, transColor);
    } else if ( sickCount == 3)
    {
        mGetTranslateColorShade( kFriendlyColor, DARKEST, friendSick, transColor);
        mGetTranslateColorShade( kHostileColor, DARKER-1, foeSick, transColor);
        mGetTranslateColorShade( kNeutralColor, DARKEST, neutralSick, transColor);
    }

    anAdmiral = mGetAdmiralPtr( 0);
    for ( count = 0; count < kScenarioPlayerNum; count++)
    {
        anAdmiral->shipsLeft = 0;
        anAdmiral++;
    }

// it probably doesn't matter what order we do this in, but we'll do it in the "ideal" order anyway

    anObject = gRootObject;

    while ( anObject != nil)
    {
        if (anObject->active)
        {
            gAresGlobal->gSynchValue += anObject->location.h;
            gAresGlobal->gSynchValue += anObject->location.v;

/*          DebugFileAppendLong( anObject->whichBaseObject);
            DebugFileAppendString( "\p\t");
            DebugFileAppendLong( anObject->entryNumber);
            DebugFileAppendString( "\p\t");
            DebugFileAppendLong( anObject->location.h - kUniversalCenter);
            DebugFileAppendString( "\p\t");
            DebugFileAppendLong( anObject->location.v - kUniversalCenter);
            DebugFileAppendString( "\p\t");
            DebugFileAppendSmallFixed( anObject->velocity.h);
            DebugFileAppendString( "\p\t");
            DebugFileAppendSmallFixed( anObject->velocity.v);
            DebugFileAppendString( "\p\t");
            DebugFileAppendSmallFixed( anObject->motionFraction.h + anObject->motionFraction.v);
            DebugFileAppendString( "\p\t");
            if ( anObject->sprite != nil)
                DebugFileAppendLong( anObject->sprite->whichShape);
            else
                DebugFileAppendString("\p-");
            DebugFileAppendString( "\p\r");
*/
            keysDown = anObject->keysDown & kSpecialKeyMask;

            // pay the admiral if this is a destination object
//          if (( anObject->attributes & kIsDestination) && ( anObject->owner != kNoOwner))
//          {
//              PayAdmiral( anObject->owner, 1);
//          }

            // strobe its symbol if it's not feeling well
            if ( anObject->sprite != nil)
            {
                if ((anObject->health > 0) && ( anObject->health <= ( anObject->baseType->health >> 2)))
                {
                    if ( anObject->owner == gAresGlobal->gPlayerAdmiralNumber)
                        anObject->sprite->tinyColor = friendSick;
                    else if ( anObject->owner < 0)
                        anObject->sprite->tinyColor = neutralSick;
                    else
                        anObject->sprite->tinyColor = foeSick;
                } else
                {
                    anObject->sprite->tinyColor = anObject->tinyColor;
                }
            }

            // if the object can think, or is human controlled
            if ( anObject->attributes & ( kCanThink | kRemoteOrHuman))
            {
                // get the object's base object
                baseObject = anObject->baseType;
                anObject->targetAngle = anObject->directionGoal = anObject->direction;
                // incremenent its admiral's # of ships
                if ( anObject->owner > kNoOwner)
                {
                    anAdmiral = mGetAdmiralPtr( anObject->owner);
                    anAdmiral->shipsLeft++;
                }

                switch( anObject->presenceState)
                {
                    case kNormalPresence:
                        keysDown = ThinkObjectNormalPresence( anObject, baseObject, timePass);
                        break;

                    case kWarpingPresence:
                        keysDown = ThinkObjectWarpingPresence( anObject);
                        break;

                    case kWarpInPresence:
                        keysDown = ThinkObjectWarpInPresence( anObject);
                        break;

                    case kWarpOutPresence:
                        keysDown = ThinkObjectWarpOutPresence( anObject, baseObject);
                        break;

                    case kLandingPresence:
                        keysDown = ThinkObjectLandingPresence( anObject);
                        break;
                }

                if (( !(anObject->attributes & kRemoteOrHuman)) ||
                    ( anObject->attributes & kOnAutoPilot))
                {
                    if ( anObject->attributes & kHasDirectionGoal)
                    {
                        if ( anObject->attributes & kShapeFromDirection)
                        {
                            if (( anObject->attributes & kIsGuided) &&
                                ( anObject->targetObjectNumber != kNoShip))
                            {
                                difference = anObject->targetAngle - anObject->direction;
                                if (( difference < -60) || ( difference > 60))
                                {
                                    anObject->targetObjectNumber = kNoShip;
                                    anObject->targetObjectID = kNoShip;
                                    anObject->directionGoal = anObject->direction;
                                }
                            }

                            offset.h = mAngleDifference( anObject->directionGoal,
                                        anObject->direction);
                            offset.v = mFixedToLong( baseObject->frame.rotation.maxTurnRate << 1);
                            difference = ABS( offset.h);
                        } else
                        {
                            offset.h = mAngleDifference( anObject->directionGoal,
                                        anObject->direction);
                            offset.v = mFixedToLong( kDefaultTurnRate << 1);
                            difference = ABS( offset.h);
                        }
                        if ( difference > offset.v)
                        {
                            if ( offset.h < 0)
                                keysDown |= kRightKey;
                            else if ( offset.h > 0) keysDown |= kLeftKey;
                        }
                    }
    // and here?
                    if ( !(anObject->keysDown & kManualOverrideFlag))
                    {
                        if ( anObject->closestDistance < kEngageRange)
                        {
                            // why do we only do this randomly when closest is within engagerange?
                            // to simulate the innaccuracy of battle
                            // (to keep things from wiggling, really)
                            if  (
                                    RandomSeeded( baseObject->skillDen,
                                        &anObject->randomSeed, 'np99', anObject->whichBaseObject)
                                    <
                                    baseObject->skillNum
                                )
                            {
                                anObject->keysDown &= ~kMotionKeyMask;
                                anObject->keysDown |= keysDown & kMotionKeyMask;
                            }
                            if ( RandomSeeded( 3, &anObject->randomSeed, 'np13', anObject->whichBaseObject) == 1)
                            {
                                anObject->keysDown &= ~kWeaponKeyMask;
                                anObject->keysDown |= keysDown & kWeaponKeyMask;
                            }
                            {
                                anObject->keysDown &= ~kMiscKeyMask;
                                anObject->keysDown |= keysDown & kMiscKeyMask;
                            }
                        } else
                        {
                            anObject->keysDown = (anObject->keysDown & kSpecialKeyMask)
                                | keysDown;
                        }
                    } else
                    {
                        anObject->keysDown &= ~kManualOverrideFlag;
                    }
                }

                // Take care of any "keys" being pressed

/*              if ( ( anObject->attributes & ( kIsRemote | kIsHumanControlled)))
                {
                    DebugFileAppendString( "\pKY\t");
                    DebugFileAppendLong( gAresGlobal->gGameTime);
                    DebugFileAppendString( "\p\t");
                    DebugFileAppendLong( anObject->directionGoal);
                    DebugFileAppendString( "\p\t");
                    DebugFileAppendLong( anObject->location.h);
                    DebugFileAppendString( "\p\r");

                }
*/
/*          DebugFileAppendLong( anObject->closestDistance);
            DebugFileAppendString( "\p\t");
            DebugFileAppendLong( anObject->closestObject);
            DebugFileAppendString( "\p\t");
            DebugFileAppendLong( anObject->destinationObject);
            DebugFileAppendString( "\p\t");
            DebugFileAppendLong( anObject->targetObjectNumber);
            DebugFileAppendString( "\p\t");
            DebugFileAppendLongHex( anObject->keysDown);
            DebugFileAppendString( "\p\t");
            DebugFileAppendLongHex( anObject->seenByPlayerFlags);
            DebugFileAppendString( "\p\t");
            DebugFileAppendLong( anObject->owner);
            if ( anObject->attributes & (kRemoteOrHuman | kIsPlayerShip)) DebugFileAppendString("\p*");
            DebugFileAppendString( "\p\r");
*/
                if ( anObject->keysDown & kAdoptTargetKey)
                {
                    SetObjectDestination( anObject, nil);
                }

                if ( anObject->keysDown & kAutoPilotKey)
                {
                    TogglePlayerAutoPilot( anObject);
                }

                if ( anObject->keysDown & kGiveCommandKey)
                {
                    PlayerShipGiveCommand( anObject->owner);
                }

                anObject->keysDown &= ~kSpecialKeyMask;

                if ( anObject->offlineTime > 0)
                {
                    if ( RandomSeeded( anObject->offlineTime, &(anObject->randomSeed),
                            'np14', anObject->whichBaseObject) > 5)
                        anObject->keysDown = 0;
                    anObject->offlineTime--;
                }

                if ( ( anObject->attributes & kRemoteOrHuman) &&
                    ( !(anObject->attributes & kCanThink)) && ( anObject->age < 120))
                {
                    PlayerShipBodyExpire( anObject, true);
                }

                if (( anObject->attributes & kHasDirectionGoal) &&
                    ( anObject->offlineTime <= 0))
                {
                        if ( anObject->attributes & kShapeFromDirection)    // design flaw: can't have turn rate unless shapefromdirection
                        {
                            if ( anObject->keysDown & kLeftKey)
                            {
                                anObject->turnVelocity =
                                    -baseObject->frame.rotation.maxTurnRate;
                            } else if ( anObject->keysDown & kRightKey)
                            {
                                anObject->turnVelocity =
                                    baseObject->frame.rotation.maxTurnRate;
                            } else anObject->turnVelocity = 0;
                        } else
                        {
                            if ( anObject->keysDown & kLeftKey)
                            {
                                anObject->turnVelocity = -kDefaultTurnRate;
                            } else if ( anObject->keysDown & kRightKey)
                            {
                                anObject->turnVelocity = kDefaultTurnRate;
                            } else anObject->turnVelocity = 0;
                        }
                }

                if ( anObject->keysDown & kUpKey)
                {

                    if (!(( anObject->presenceState == kWarpInPresence) ||
                        ( anObject->presenceState == kWarpingPresence) ||
                        ( anObject->presenceState == kWarpOutPresence)))
                    {
                        anObject->thrust = baseObject->maxThrust;
                    }
                } else if ( anObject->keysDown & kDownKey)
                {
                    if (!(( anObject->presenceState == kWarpInPresence) ||
                        ( anObject->presenceState == kWarpingPresence) ||
                        ( anObject->presenceState == kWarpOutPresence)))
                    {
                        anObject->thrust = -baseObject->maxThrust;
                    }
                    anObject->thrust = -baseObject->maxThrust;
                } else anObject->thrust = 0;

                if ( anObject->rechargeTime < kRechargeSpeed)
                {
                    anObject->rechargeTime++;
                } else
                {
                    anObject->rechargeTime = 0;

                    if ( anObject->presenceState == kWarpingPresence)
                    {
                        anObject->energy -= 1;
                        anObject->warpEnergyCollected += 1;
                        if ( anObject->energy <= 0)
                        {
                            anObject->energy = 0;
                        }
                    }

                    if ( anObject->presenceState == kNormalPresence)
                    {
                        if (( anObject->energy < (baseObject->energy - kEnergyChunk)) &&
                            ( anObject->battery > kEnergyChunk))
                        {
                            anObject->battery -= kEnergyChunk;
                            anObject->energy += kEnergyChunk;
                        }

                        if (( anObject->health < ( baseObject->health >> 1)) &&
                            ( anObject->energy > kHealthRatio))
                        {
                            anObject->health++;
                            anObject->energy -= kHealthRatio;
                        }

                        if ( anObject->pulseType != kNoWeapon)
                        {
                            if (( anObject->pulseAmmo <
                                (anObject->pulseBase->frame.weapon.ammo >> 1)) &&
                                ( anObject->energy >= kWeaponRatio))
                            {
                                anObject->pulseCharge++;
                                anObject->energy -= kWeaponRatio;

                                if (( anObject->pulseBase->frame.weapon.restockCost >= 0)
                                    && (anObject->pulseCharge >=
                                    anObject->pulseBase->frame.weapon.restockCost))
                                {
                                    anObject->pulseCharge -=
                                        anObject->pulseBase->frame.weapon.restockCost;
                                    anObject->pulseAmmo++;
                                }
                            }
                        }

                        if ( anObject->beamType != kNoWeapon)
                        {
                            if (( anObject->beamAmmo <
                                (anObject->beamBase->frame.weapon.ammo >> 1)) &&
                                ( anObject->energy >= kWeaponRatio))
                            {
                                anObject->beamCharge++;
                                anObject->energy -= kWeaponRatio;

                                if ((anObject->beamBase->frame.weapon.restockCost >= 0) &&
                                    ( anObject->beamCharge >=
                                    anObject->beamBase->frame.weapon.restockCost))
                                {
                                    anObject->beamCharge -=
                                        anObject->beamBase->frame.weapon.restockCost;
                                    anObject->beamAmmo++;
                                }
                            }
                        }

                        if ( anObject->specialType != kNoWeapon)
                        {
                            if (( anObject->specialAmmo <
                                (anObject->specialBase->frame.weapon.ammo >> 1)) &&
                                ( anObject->energy >= kWeaponRatio))
                            {
                                anObject->specialCharge++;
                                anObject->energy -= kWeaponRatio;

                                if (( anObject->specialBase->frame.weapon.restockCost >= 0)
                                    && ( anObject->specialCharge >=
                                    anObject->specialBase->frame.weapon.restockCost))
                                {
                                    anObject->specialCharge -=
                                        anObject->specialBase->frame.weapon.restockCost;
                                    anObject->specialAmmo++;
                                }
                            }
                        }
                    }
                }

                // targetObject is set for all three weapons -- do not change
                if ( anObject->targetObjectNumber >= 0)
                {
                    targetObject = *gSpaceObjectData + anObject->targetObjectNumber;
                } else targetObject = nil;

                if ( anObject->pulseTime > 0) anObject->pulseTime -= timePass;
                if (( anObject->keysDown & kOneKey) && ( anObject->pulseTime <= 0) &&
                    ( anObject->pulseType != kNoWeapon))
                {
/*                  DebugFileAppendString( "\pPL\t");
                    DebugFileAppendLong( anObject->id);
                    DebugFileAppendString( "\p\t");
                    DebugFileAppendLong( anObject->closestDistance);
                    DebugFileAppendString( "\p\t");
                    DebugFileAppendLong( anObject->targetAngle);
                    DebugFileAppendString( "\p\t");
                    DebugFileAppendLong( anObject->whichBaseObject);
                    DebugFileAppendString( "\p\r");
*/
                    weaponObject = anObject->pulseBase;
                    if (( anObject->energy >= weaponObject->frame.weapon.energyCost)
                        && (( weaponObject->frame.weapon.ammo < 0) ||
                        ( anObject->pulseAmmo > 0)))
                    {
                        if ( anObject->cloakState > 0)
                            AlterObjectCloakState( anObject, false);
                        anObject->energy -= weaponObject->frame.weapon.energyCost;
                        anObject->pulsePosition++;
                        if ( anObject->pulsePosition >= baseObject->pulsePositionNum)
                            anObject->pulsePosition = 0;

                        h = anObject->direction;
                        mAddAngle( h, -90);
                        mGetRotPoint( fcos, fsin, h);
                        fcos = -fcos;
                        fsin = -fsin;

                        offset.h = mMultiplyFixed( baseObject->pulsePosition[anObject->pulsePosition].h, fcos);
                        offset.h -= mMultiplyFixed( baseObject->pulsePosition[anObject->pulsePosition].v, fsin);
                        offset.v = mMultiplyFixed( baseObject->pulsePosition[anObject->pulsePosition].h, fsin);
                        offset.v += mMultiplyFixed( baseObject->pulsePosition[anObject->pulsePosition].v, fcos);
                        offset.h = mFixedToLong( offset.h);
                        offset.v = mFixedToLong( offset.v);

                        anObject->pulseTime = weaponObject->frame.weapon.fireTime;
                        if ( weaponObject->frame.weapon.ammo > 0)
                            anObject->pulseAmmo--;
                        ExecuteObjectActions( weaponObject->activateAction,
                                            weaponObject->activateActionNum, anObject,
                                            targetObject, &offset, true);
                    }
                }
                if ( anObject->beamTime > 0) anObject->beamTime -= timePass;
                if (( anObject->keysDown & kTwoKey) && ( anObject->beamTime <= 0 ) &&
                    ( anObject->beamType != kNoWeapon) )
                {
                    weaponObject = anObject->beamBase;
                    if ( (anObject->energy >= weaponObject->frame.weapon.energyCost)
                        && (( weaponObject->frame.weapon.ammo < 0) ||
                        ( anObject->beamAmmo > 0)))
                    {
                        if ( anObject->cloakState > 0)
                            AlterObjectCloakState( anObject, false);
                        anObject->energy -= weaponObject->frame.weapon.energyCost;
                        anObject->beamPosition++;
                        if ( anObject->beamPosition >= baseObject->beamPositionNum)
                            anObject->beamPosition = 0;

                        h = anObject->direction;
                        mAddAngle( h, -90);
                        mGetRotPoint( fcos, fsin, h);
                        fcos = -fcos;
                        fsin = -fsin;

                        offset.h = mMultiplyFixed( baseObject->beamPosition[anObject->beamPosition].h, fcos);
                        offset.h -= mMultiplyFixed( baseObject->beamPosition[anObject->beamPosition].v, fsin);
                        offset.v = mMultiplyFixed( baseObject->beamPosition[anObject->beamPosition].h, fsin);
                        offset.v += mMultiplyFixed( baseObject->beamPosition[anObject->beamPosition].v, fcos);
                        offset.h = mFixedToLong( offset.h);
                        offset.v = mFixedToLong( offset.v);

                        anObject->beamTime = weaponObject->frame.weapon.fireTime;
                        if ( weaponObject->frame.weapon.ammo > 0) anObject->beamAmmo--;
                        ExecuteObjectActions( weaponObject->activateAction,
                                            weaponObject->activateActionNum, anObject,
                                            targetObject, &offset, true);
                    }

                }
                if ( anObject->specialTime > 0) anObject->specialTime -= timePass;

                if (( anObject->keysDown & kEnterKey) && ( anObject->specialTime <= 0)
                    && ( anObject->specialType != kNoWeapon))
                {
/*                  DebugFileAppendString( "\pSP\t");
                    DebugFileAppendLong( anObject->id);
                    DebugFileAppendString( "\p\t");
                    DebugFileAppendLong( anObject->closestDistance);
                    DebugFileAppendString( "\p\t");
                    DebugFileAppendLong( anObject->targetAngle);
                    DebugFileAppendString( "\p\t");
                    DebugFileAppendLong( anObject->whichBaseObject);
                    DebugFileAppendString( "\p\r");
*/
                    weaponObject = anObject->specialBase;
                    if ( (anObject->energy >= weaponObject->frame.weapon.energyCost)
                        && (( weaponObject->frame.weapon.ammo < 0) ||
                        ( anObject->specialAmmo > 0)))
                    {
                        anObject->energy -= weaponObject->frame.weapon.energyCost;
                        anObject->specialPosition++;
                        if ( anObject->specialPosition >=
                                baseObject->specialPositionNum)
                            anObject->specialPosition = 0;

                        h = anObject->direction;
                        mAddAngle( h, -90);
                        mGetRotPoint( fcos, fsin, h);
                        fcos = -fcos;
                        fsin = -fsin;

                        offset.h = mMultiplyFixed( baseObject->specialPosition[anObject->specialPosition].h, fcos);
                        offset.h -= mMultiplyFixed( baseObject->specialPosition[anObject->specialPosition].v, fsin);
                        offset.v = mMultiplyFixed( baseObject->specialPosition[anObject->specialPosition].h, fsin);
                        offset.v += mMultiplyFixed( baseObject->specialPosition[anObject->specialPosition].v, fcos);
                        offset.h = mFixedToLong( offset.h);
                        offset.v = mFixedToLong( offset.v);

                        anObject->specialTime = weaponObject->frame.weapon.fireTime;
                        if ( weaponObject->frame.weapon.ammo > 0)
                            anObject->specialAmmo--;
                        /*
                        if ( anObject->targetObjectNumber >= 0)
                        {
                            targetObject = *gSpaceObjectData + anObject->targetObjectNumber;
                        } else targetObject = nil;
                        */
                        ExecuteObjectActions( weaponObject->activateAction,
                                            weaponObject->activateActionNum, anObject,
                                            targetObject, nil, true);
                    }
                }

                if (( anObject->keysDown & kWarpKey) && ( baseObject->warpSpeed > 0) &&
                    ( anObject->energy > 0))
                {
                    if (( anObject->presenceState == kWarpingPresence) ||
                        ( anObject->presenceState == kWarpOutPresence))
                    {
                        anObject->thrust = mMultiplyFixed( baseObject->maxThrust,
                            anObject->presenceData);
                    } else if (( anObject->presenceState == kNormalPresence) &&
                        ( anObject->energy > ( anObject->baseType->energy >> kWarpInEnergyFactor)))
                    {
                        anObject->presenceState = kWarpInPresence;
                        anObject->presenceData = 0;
                    }
                } else
                {
                    if ( anObject->presenceState == kWarpInPresence)
                    {
                        anObject->presenceState = kNormalPresence;
                    } else if ( anObject->presenceState == kWarpingPresence)
                    {
                        anObject->presenceState = kWarpOutPresence;
                    } else if ( anObject->presenceState == kWarpOutPresence)
                    {
                            anObject->thrust = mMultiplyFixed( baseObject->maxThrust,
                                anObject->presenceData);
                    }
                }

            }
        }

        anObject = anObject->nextObject;
    }
}
#endif  // kUseOldThinking

unsigned long ThinkObjectNormalPresence( spaceObjectType *anObject, baseObjectType *baseObject, long timePass)
{
    unsigned long   keysDown = anObject->keysDown & kSpecialKeyMask, distance, dcalc;
    spaceObjectType *targetObject;
    baseObjectType  *bestWeapon, *weaponObject;
    coordPointType  dest;
    long            difference;
    Fixed           slope;
    short           angle, theta, beta;
    smallFixedType  calcv, fdist;
    longPointType   offset;

    if ((!(anObject->attributes & kRemoteOrHuman))
        || ( anObject->attributes & kOnAutoPilot))
    {
        // set all keys off
        keysDown &= kSpecialKeyMask;
        // if target object exists and is within engage range

        ThinkObjectResolveTarget( anObject, &dest, &distance, &targetObject);
//      if (( dest.h == 0xffffffff) || ( dest.v == 0xffffffff) || ( distance == 0xffffffff)) DebugStr("\pResolve 1");

///--->>> BEGIN TARGETING <<<---///
        if (    ( anObject->targetObjectNumber != kNoShip) &&
                (
                    ( anObject->attributes & kIsGuided) ||
                    (
                        ( anObject->attributes & kCanEngage) &&
                        ( !(anObject->attributes &
                            kRemoteOrHuman)) &&
                        ( distance < anObject->engageRange) &&
                        ( anObject->timeFromOrigin < kTimeToCheckHome) &&
                        ( targetObject->attributes & kCanBeEngaged)
                    )
                )
            )
        {
            keysDown |= ThinkObjectEngageTarget( anObject, targetObject, distance, &theta, timePass);
//          if ( theta == 0xffff) DebugStr("\pEngage 1");
///--->>> END TARGETING <<<---///


            // if I'm in target object's range & it's looking at us & my health is less
            // than 1/2 its -- or I can't engage it
            if (    ( anObject->attributes & kCanEvade) &&
                    ( targetObject->attributes & kCanBeEvaded) &&
                    ( distance < targetObject->longestWeaponRange) &&
                    (targetObject->attributes & kHated) &&
                    ( ABS( theta ) < kParanoiaAngle) &&
                    (
                        (!(targetObject->attributes & kCanBeEngaged))
                        ||
                        ( anObject->health <= targetObject->health )
                    )
                )
            {
                // try to evade, flee, run away
                if ( anObject->attributes & kHasDirectionGoal)
                {
                    if ( anObject->beamType != kNoWeapon)
                    {
                        weaponObject = anObject->beamBase;
                        if ( weaponObject->frame.weapon.usage &
                            kUseForDefense)
                        {
                            keysDown |= kTwoKey;
                        }
                    }

                    if ( anObject->pulseType != kNoWeapon)
                    {
                        weaponObject = anObject->pulseBase;
                        if ( weaponObject->frame.weapon.usage &
                            kUseForDefense)
                        {
                            keysDown |= kOneKey;
                        }
                    }

                    if ( anObject->specialType != kNoWeapon)
                    {
                        weaponObject = anObject->specialBase;
                        if ( weaponObject->frame.weapon.usage &
                            kUseForDefense)
                        {
                            keysDown |= kEnterKey;
                        }
                    }

                    anObject->directionGoal = targetObject->direction;

                    if ( targetObject->attributes & kIsGuided)
                    {
                        if ( theta > 0)
                        {
                            mAddAngle( anObject->directionGoal, 90);
                        }
                        else if ( theta < 0)
                        {
                            mAddAngle( anObject->directionGoal, -90);
                        } else
                        {
                            beta = 90;
                            if ( anObject->location.h & 0x00000001)
//                          if ( RandomSeeded( 2,
//                                  &anObject->randomSeed, 'nps4', anObject->whichBaseObject))
                                beta = -90;
                            mAddAngle( anObject->directionGoal, beta);
                        }
                        theta =
                            mAngleDifference( anObject->directionGoal,
                            anObject->direction);
                        if ( ABS( theta) < 90)
                        {
                            keysDown |= kUpKey;
                        } else
                        {
                            keysDown |= kUpKey; // try an always thrust strategy
                        }
                    } else
                        {
                        if ( theta > 0)
                        {
                            mAddAngle( anObject->directionGoal, kEvadeAngle);
                        }
                        else if ( theta < 0)
                        {
                            mAddAngle( anObject->directionGoal, -kEvadeAngle);
                        } else
                        {
                            beta = kEvadeAngle;
//                          if ( RandomSeeded( 2,
//                                  &anObject->randomSeed, 'nps5', anObject->whichBaseObject))
                            if ( anObject->location.h & 0x00000001)
                                beta = -kEvadeAngle;
                            mAddAngle( anObject->directionGoal, beta);
                        }
                        theta = mAngleDifference( anObject->directionGoal,
                            anObject->direction);
                        if ( ABS( theta) < kEvadeAngle)
                        {
                            keysDown |= kUpKey;
                        } else
                        {
                            keysDown |= kUpKey; // try an always thrust strategy
                        }
                    }
                } else
                {
                    beta = kEvadeAngle;
                    if ( RandomSeeded( 2, &anObject->randomSeed,
                        'nps6', anObject->whichBaseObject)) beta = -kEvadeAngle;
                    mAddAngle( anObject->direction, beta);
                    keysDown |= kUpKey;
                }

            // if we're not afraid, then
            } else
            {
                // if we are not within our closest weapon range then

                if (( distance > anObject->shortestWeaponRange) ||
                        ( anObject->attributes & kIsGuided))
                    keysDown |= kUpKey;

                // if we are as close as we like
                else
                {
                    // if we're getting closer
                    if (( distance < kMotionMargin) ||
                        ((distance + kMotionMargin) <
                        anObject->lastTargetDistance))
                    {
                        keysDown |= kDownKey;
                        anObject->lastTargetDistance = distance;
                    // if we're not getting closer, then if we're getting farther
                    } else if ( ( distance - kMotionMargin) >
                        anObject->lastTargetDistance)
                    {
                        keysDown |= kUpKey;
                        anObject->lastTargetDistance = distance;
                    }
                }
            }

            if ( anObject->targetObjectNumber ==
                    anObject->destinationObject)
            {
                if ( distance < baseObject->arriveActionDistance)
                {
                    if ( baseObject->arriveAction >= 0)
                    {
                        if ( !(anObject->runTimeFlags & kHasArrived))
                        {
                            offset.h = offset.v = 0;
                            ExecuteObjectActions(
                                baseObject->arriveAction,
                                baseObject->arriveActionNum,
                                anObject,
                                anObject->destObjectPtr,
                                &offset, true);
                            anObject->runTimeFlags |= kHasArrived;
                        }
                    }
                }
            }
        } else if ( anObject->attributes & kIsGuided)
        {
            keysDown |= kUpKey;
        } else  // not guided & no target object or target object is out of engage range
        {
///--->>> BEGIN TARGETING <<<---///
            if (    (anObject->targetObjectNumber != kNoShip) &&
                    (
                        (
                            (!(anObject->attributes &
                                kRemoteOrHuman)) &&
                            ( distance < anObject->engageRange)
                        ) ||
                        ( anObject->attributes & kIsGuided)
                    )
                )
            {
                keysDown |= ThinkObjectEngageTarget( anObject, targetObject, distance, &theta, timePass);
//              if ( theta == 0xffff) DebugStr("\pEngage 2");

                if (( targetObject->attributes & kCanBeEngaged) &&
                    ( anObject->attributes & kCanEngage) &&
                    ( distance < anObject->longestWeaponRange) &&
                    ( targetObject->attributes & kHated))
                {
                } else if ( ( anObject->attributes & kCanEvade) &&
                            (targetObject->attributes & kHated) &&
                            ( targetObject->attributes & kCanBeEvaded) &&
                            (
                                (
                                    ( distance <
                                        targetObject->longestWeaponRange)
                                    &&
                                    ( ABS( theta ) < kParanoiaAngle)
                                ) ||
                                ( targetObject->attributes & kIsGuided)
                            )
                        )
                {

                    // try to evade, flee, run away
                    if ( anObject->attributes & kHasDirectionGoal)
                    {
                        if ( distance < anObject->longestWeaponRange)
                        {
                            if ( anObject->beamType != kNoWeapon)
                            {
                                weaponObject = anObject->beamBase;
                                if ( weaponObject->frame.weapon.usage &
                                    kUseForDefense)
                                {
                                    keysDown |= kTwoKey;
                                }
                            }

                            if ( anObject->pulseType != kNoWeapon)
                            {
                                weaponObject = anObject->pulseBase;
                                if ( weaponObject->frame.weapon.usage &
                                    kUseForDefense)
                                {
                                    keysDown |= kOneKey;
                                }
                            }

                            if ( anObject->specialType != kNoWeapon)
                            {
                                weaponObject = anObject->specialBase;
                                if ( weaponObject->frame.weapon.usage &
                                    kUseForDefense)
                                {
                                    keysDown |= kEnterKey;
                                }
                            }
                        }

                        anObject->directionGoal =
                            targetObject->direction;

                        if ( theta > 0)
                        {
                            mAddAngle( anObject->directionGoal, kEvadeAngle);
                        }
                        else if ( theta < 0)
                        {
                            mAddAngle( anObject->directionGoal, -kEvadeAngle);
                        } else
                        {
                            beta = kEvadeAngle;
//                          if ( RandomSeeded( 2,
//                              &anObject->randomSeed, 'np10', anObject->whichBaseObject))
                            if ( anObject->location.h & 0x00000001)
                                beta = -kEvadeAngle;
                            mAddAngle( anObject->directionGoal, beta);
                        }
                        theta = mAngleDifference( anObject->directionGoal,
                            anObject->direction);
                        if ( ABS( theta) < kEvadeAngle)
                        {
                            keysDown |= kUpKey;
                        } else
                        {
                            keysDown |= kUpKey;
                        }
                    } else
                    {
                        beta = kEvadeAngle;
                        if ( RandomSeeded( 2, &anObject->randomSeed,
                            'np11', anObject->whichBaseObject)) beta = -kEvadeAngle;
                        mAddAngle( anObject->direction, beta);
                        keysDown |= kUpKey;
                    }
                }
            }
///--->>> END TARGETING <<<---///
            if (( anObject->attributes & kIsDestination) ||
                (( anObject->destinationObject == kNoDestinationObject)
                && ( anObject->destinationLocation.h ==
                kNoDestinationCoord)))
            {
                if (anObject->attributes & kOnAutoPilot)
                {
                    TogglePlayerAutoPilot( anObject);
                }
                keysDown |= kDownKey;
                anObject->timeFromOrigin = 0;
            } else
            {
                if ( anObject->destinationObject !=
                    kNoDestinationObject)
                {
                    targetObject =
                        anObject->destObjectPtr;
                    if (( targetObject != nil) &&
                        ( targetObject->active) &&
                        ( targetObject->id == anObject->destObjectID))
                    {
                            if ( targetObject->seenByPlayerFlags &
                                anObject->myPlayerFlag)
                            {
                                dest.h = targetObject->location.h;
                                dest.v = targetObject->location.v;
                                anObject->destinationLocation.h =
                                    dest.h;
                                anObject->destinationLocation.v =
                                    dest.v;
                            } else
                            {
                                dest.h =
                                    anObject->destinationLocation.h;
                                dest.v =
                                    anObject->destinationLocation.v;
                            }
                            anObject->destObjectDest =
                                targetObject->destinationObject;
                            anObject->destObjectDestID = targetObject->destObjectID;
                    } else
                    {
                        anObject->duty = eNoDuty;
                        anObject->attributes &= ~kStaticDestination;
                        if ( targetObject == nil)
                        {
                            keysDown |= kDownKey;
                            anObject->destObjectDest = kNoDestinationObject;
                            anObject->destinationObject =
                                kNoDestinationObject;
                            dest.h = anObject->location.h;
                            dest.v = anObject->location.v;
                            if (anObject->attributes & kOnAutoPilot)
                            {
                                TogglePlayerAutoPilot( anObject);
                            }
                        } else
                        {
                            anObject->destinationObject =
                                anObject->destObjectDest;
                            if ( anObject->destinationObject !=
                                kNoDestinationObject)
                            {
                                targetObject = *gSpaceObjectData + anObject->destinationObject;
                                if ( targetObject->id != anObject->destObjectDestID) targetObject = nil;
                            } else targetObject = nil;
                            if ( targetObject != nil)
                            {
                                anObject->destObjectPtr =
                                    targetObject;
                                anObject->destObjectID =
                                    targetObject->id;
                                anObject->destObjectDest =
                                    targetObject->destinationObject;
                                anObject->destObjectDestID = targetObject->destObjectID;
                                dest.h = targetObject->location.h;
                                dest.v = targetObject->location.v;
                            } else
                            {
                                anObject->duty = eNoDuty;
                                keysDown |= kDownKey;
                                anObject->destinationObject =
                                    kNoDestinationObject;
                                anObject->destObjectDest = kNoDestinationObject;
                                anObject->destObjectPtr = nil;
                                dest.h = anObject->location.h;
                                dest.v = anObject->location.v;
                                if (anObject->attributes & kOnAutoPilot)
                                {
                                    TogglePlayerAutoPilot( anObject);
                                }
                            }
                        }
                    }
                } else // no destination object; just coords
                {
                    if (anObject->attributes & kOnAutoPilot)
                    {
                        TogglePlayerAutoPilot( anObject);
                    }
                    targetObject = nil;
                    dest.h = anObject->destinationLocation.h;
                    dest.v = anObject->destinationLocation.v;
                }

                ThinkObjectGetCoordVector( anObject, &dest, &distance, &angle);

                if ( anObject->attributes & kHasDirectionGoal)
                {
                    theta = mAngleDifference( angle, anObject->directionGoal);
                    if ( ABS( theta) > kDirectionError)
                    {
                        anObject->directionGoal = angle;
                    }

                    theta = mAngleDifference( anObject->direction,
                            anObject->directionGoal);
                    theta = ABS( theta);
                } else
                {
                    anObject->direction = angle;
                    theta = 0;
                }

/*              if ((anObject->attributes & kOnAutoPilot) &&
                    (( distance < kAutoPilotDistance) ||
                    ( distance < baseObject->arriveActionDistance)))
                {
                    TogglePlayerAutoPilot( anObject);
                }
*/              if ( distance < kEngageRange)
                {
                    anObject->timeFromOrigin = 0;
                }

                if ( distance > baseObject->arriveActionDistance)
                {
                    if ( theta < kEvadeAngle)
                        keysDown |= kUpKey;
                    anObject->lastTargetDistance = distance;
                    if (( anObject->specialType != kNoWeapon) &&
                        ( distance > kWarpInDistance)
                        && ( theta <= kDirectionError))
                    {
                        if ( anObject->specialBase->frame.weapon.usage
                            & kUseForTransportation)
                        {
                            keysDown |= kEnterKey;
                        }
                    }
                    if (( baseObject->warpSpeed > 0) && ( anObject->energy >
                        ( anObject->baseType->energy >> kWarpInEnergyFactor)) &&
                        ( distance > kWarpInDistance)
                        && ( theta <= kDirectionError))
                    {
                        keysDown |= kWarpKey;
                    }
                } else
                {
                    if (( targetObject != nil) &&
                        ( targetObject->owner == anObject->owner) &&
                        ( targetObject->attributes &
                             anObject->attributes & kHasDirectionGoal))
                    {
                        anObject->directionGoal =
                            targetObject->direction;
                        if (( targetObject->keysDown & kWarpKey) &&
                            ( baseObject->warpSpeed > 0))
                        {
                            theta = mAngleDifference( anObject->direction, targetObject->direction);
                            if ( ABS( theta) < kDirectionError)
                                keysDown |= kWarpKey;
                        }
                    }

                    if ( distance < baseObject->arriveActionDistance)
                    {
                        if ( baseObject->arriveAction >= 0)
                        {
                            if ( !(anObject->runTimeFlags &
                                kHasArrived))
                            {
                                offset.h = offset.v = 0;
                                ExecuteObjectActions(
                                    baseObject->arriveAction,
                                    baseObject->arriveActionNum,
                                        anObject,
                                        anObject->destObjectPtr,
                                        &offset, true);
                                anObject->runTimeFlags |= kHasArrived;
                            }
                        }
                    }

                    // if we're getting closer
                    if ((distance + kMotionMargin) <
                            anObject->lastTargetDistance )
                    {
                        keysDown |= kDownKey;
                        anObject->lastTargetDistance = distance;
                    // if we're not getting closer, then if we're getting farther
                    } else if ( ( distance - kMotionMargin) >
                        anObject->lastTargetDistance)
                    {
                        if ( theta < kEvadeAngle)
                            keysDown |= kUpKey;
                        else keysDown |= kDownKey;
                        anObject->lastTargetDistance = distance;
                    }
                }
            }
        }
    } else // object is human controlled -- we need to calc target angle
    {
        ThinkObjectResolveTarget( anObject, &dest, &distance, &targetObject);
//      if (( dest.h == 0xffffffff) || ( dest.v == 0xffffffff) || ( distance == 0xffffffff)) DebugStr("\pResolve 2");

        if (( anObject->attributes & kCanEngage) &&
            ( distance < anObject->engageRange) &&
            (anObject->targetObjectNumber != kNoShip))
        {
            // if target is in our weapon range & we hate the object
            if (( distance < anObject->longestWeaponRange) &&
                ( targetObject->attributes & kHated))
            {
                // find "best" weapon (how do we want to aim?)
                // difference = closest range

                difference = anObject->longestWeaponRange;

                bestWeapon = nil;

                if ( anObject->beamType != kNoWeapon)
                {
                    bestWeapon = weaponObject = anObject->beamBase;
                    if ( ( weaponObject->frame.weapon.usage &
                        kUseForAttacking) &&
                        ( weaponObject->frame.weapon.range >=
                            distance) &&
                        ( weaponObject->frame.weapon.range <
                            difference))
                    {
                        bestWeapon = weaponObject;
                        difference = weaponObject->frame.weapon.range;
                    }
                }

                if ( anObject->pulseType != kNoWeapon)
                {
                    weaponObject = anObject->pulseBase;
                    if ( ( weaponObject->frame.weapon.usage &
                        kUseForAttacking) &&
                        ( weaponObject->frame.weapon.range >= distance)
                        && ( weaponObject->frame.weapon.range <
                        difference))
                    {
                        bestWeapon = weaponObject;
                        difference = weaponObject->frame.weapon.range;
                    }
                }

                if ( anObject->specialType != kNoWeapon)
                {
                    weaponObject = anObject->specialBase;
                    if ( ( weaponObject->frame.weapon.usage &
                        kUseForAttacking) &&
                        ( weaponObject->frame.weapon.range >= distance)
                        && ( weaponObject->frame.weapon.range <
                        difference))
                    {
                        bestWeapon = weaponObject;
                        difference = weaponObject->frame.weapon.range;
                    }
                }

                // offset dest for anticipated position -- overkill?

                if ( bestWeapon != nil)
                {

                    dcalc = lsqrt( distance);

                    calcv = targetObject->velocity.h -
                        anObject->velocity.h;
                    fdist = mLongToFixed( dcalc);
                    fdist = mMultiplyFixed( bestWeapon->frame.weapon.inverseSpeed, fdist);
                    calcv = mMultiplyFixed( calcv, fdist);
                    difference = mFixedToLong( calcv);
                    dest.h -= difference;

                    calcv = targetObject->velocity.v -
                        anObject->velocity.v;
                    calcv = mMultiplyFixed( calcv, fdist);
                    difference = mFixedToLong( calcv);
                    dest.v -= difference;
                }
            } // target is not in our weapon range (or we don't hate it)

            // this is human controlled--if it's too far away, tough nougies
            // find angle between me & dest
            #ifdef powerc
            slope = MyFixRatio(anObject->location.h - dest.h,
                        anObject->location.v - dest.v);
            #else
            slope = FixRatio(anObject->location.h - dest.h,
                        anObject->location.v - dest.v);
            #endif
            angle = AngleFromSlope( slope);

            if ( dest.h < anObject->location.h)
                mAddAngle( angle, 180);
            else if (( anObject->location.h == dest.h) &&
                    ( dest.v < anObject->location.v))
                angle = 0;

            if ( targetObject->cloakState > 250)
            {
                angle -= 45;
                mAddAngle( angle, RandomSeeded( 90,
                    &anObject->randomSeed, 'np12', anObject->whichBaseObject));
            }
            anObject->targetAngle = angle;
        }
    }

    return( keysDown);
}

unsigned long ThinkObjectWarpInPresence( spaceObjectType *anObject)
{
    unsigned long   keysDown = anObject->keysDown & kSpecialKeyMask, distance, dcalc;
    long            longscrap, difference;
    spaceObjectType *targetObject;  // just for scrap
    fixedPointType  newVel;

    if (( !(anObject->attributes & kRemoteOrHuman)) ||
            ( anObject->attributes & kOnAutoPilot))
    {
        keysDown = kWarpKey;
    }
    anObject->presenceData = ( anObject->presenceData & ~0x000000ff)
        | (( anObject->presenceData & 0x000000ff) + kDecideEveryCycles);
    if ( !(anObject->presenceData & 0x10000000))
    {
        longscrap = kMaxSoundVolume;
        mPlayDistanceSound( difference, longscrap, anObject, kWarpOne, kMediumPersistence, kPrioritySound, distance, dcalc, targetObject);
        anObject->presenceData |= 0x10000000;
    } else if (( !(anObject->presenceData & 0x20000000)) &&
        (( anObject->presenceData & 0x000000ff) > 25))
    {
        longscrap = kMaxSoundVolume;
        mPlayDistanceSound( difference, longscrap, anObject, kWarpTwo, kMediumPersistence, kPrioritySound, distance, dcalc, targetObject);
        anObject->presenceData |= 0x20000000;
    } if (( !(anObject->presenceData & 0x40000000)) &&
        (( anObject->presenceData & 0x000000ff) > 50))
    {
        longscrap = kMaxSoundVolume;
        mPlayDistanceSound( difference, longscrap, anObject, kWarpThree, kMediumPersistence, kPrioritySound, distance, dcalc, targetObject);
        anObject->presenceData |= 0x40000000;
    } if (( !(anObject->presenceData & 0x80000000)) &&
        (( anObject->presenceData & 0x000000ff) > 75))
    {
        longscrap = kMaxSoundVolume;
        mPlayDistanceSound( difference, longscrap, anObject, kWarpFour, kMediumPersistence, kPrioritySound, distance, dcalc, targetObject);
        anObject->presenceData |= 0x80000000;
    }

    if ( (anObject->presenceData & 0x000000ff) > 100)
    {
        anObject->energy -= anObject->baseType->energy >> kWarpInEnergyFactor;
        anObject->warpEnergyCollected += anObject->baseType->energy >> kWarpInEnergyFactor;
        if ( anObject->energy <= 0)
        {
            anObject->presenceState = kNormalPresence;
            anObject->energy = 0;
        } else
        {

            anObject->presenceState = kWarpingPresence;
            anObject->presenceData = anObject->baseType->warpSpeed;
            anObject->attributes &= ~kOccupiesSpace;
            newVel.h = newVel.v = 0;
    /*
            CreateAnySpaceObject( gAresGlobal->scenarioFileInfo.warpInFlareID, &(newVel),
                &(anObject->location), anObject->direction, kNoOwner,
                0, nil, -1, -1, -1);
    */
            CreateAnySpaceObject( gAresGlobal->scenarioFileInfo.warpInFlareID, &(newVel),
                &(anObject->location), anObject->direction, kNoOwner,
                0, -1);
        }
    }

    return( keysDown);
}

unsigned long ThinkObjectWarpingPresence( spaceObjectType *anObject)
{
    unsigned long   keysDown = anObject->keysDown & kSpecialKeyMask, distance;
    coordPointType  dest;
    spaceObjectType *targetObject = nil;
    short           angle, theta;

    if ( anObject->energy <= 0)
    {
        anObject->presenceState = kWarpOutPresence;
    }
    if (( !(anObject->attributes & kRemoteOrHuman)) ||
                ( anObject->attributes & kOnAutoPilot))
    {
        ThinkObjectResolveDestination( anObject, &dest, &targetObject);
        ThinkObjectGetCoordVector( anObject, &dest, &distance, &angle);


        if ( anObject->attributes & kHasDirectionGoal)
        {
            theta = mAngleDifference( angle, anObject->directionGoal);
            if ( ABS( theta) > kDirectionError)
            {
                anObject->directionGoal = angle;
            }
        } else
        {
            anObject->direction = angle;
        }

        if ( distance < anObject->baseType->warpOutDistance)
        {
            if ( targetObject != nil)
            {
                if (( targetObject->presenceState == kWarpInPresence)
                    || ( targetObject->presenceState == kWarpingPresence))
                {
                    keysDown |= kWarpKey;
                }
            }
        } else
        {
            keysDown |= kWarpKey;
        }
    }
    return( keysDown);
}

unsigned long ThinkObjectWarpOutPresence( spaceObjectType *anObject, baseObjectType *baseObject)
{
    unsigned long   keysDown = anObject->keysDown & kSpecialKeyMask;
    smallFixedType  calcv, fdist;
    fixedPointType  newVel;

    anObject->presenceData -= mLongToFixed(kWarpAcceleration);
    if ( anObject->presenceData < anObject->maxVelocity)
    {
        AlterObjectBattery( anObject, anObject->warpEnergyCollected);
        anObject->warpEnergyCollected = 0;

        anObject->presenceState = kNormalPresence;
        anObject->attributes |=
            baseObject->attributes & kOccupiesSpace;

        // warp out

        mGetRotPoint( fdist, calcv, anObject->direction);

        // multiply by max velocity

        fdist = mMultiplyFixed( anObject->maxVelocity, fdist);
        calcv = mMultiplyFixed( anObject->maxVelocity, calcv);
        anObject->velocity.h = fdist;
        anObject->velocity.v = calcv;
        newVel.h = newVel.v = 0;


        CreateAnySpaceObject( gAresGlobal->scenarioFileInfo.warpOutFlareID, &(newVel),
            &(anObject->location), anObject->direction, kNoOwner, 0,
            -1);
    }
    return( keysDown);
}

unsigned long ThinkObjectLandingPresence( spaceObjectType *anObject)
{
    unsigned long   keysDown = anObject->keysDown & kSpecialKeyMask, distance, dcalc;
    spaceObjectType *targetObject;
    coordPointType  dest;
    long            difference;
    Fixed           slope;
    short           angle, theta, shortx, shorty;

    keysDown = 0;

    // we repeat an object's normal action for having a destination

    if (( anObject->attributes & kIsDestination) ||
        (( anObject->destinationObject == kNoDestinationObject) &&
        ( anObject->destinationLocation.h == kNoDestinationCoord)))
    {
        if (anObject->attributes & kOnAutoPilot)
        {
            TogglePlayerAutoPilot( anObject);
        }
        keysDown |= kDownKey;
        distance = 0;
    } else
    {
        if ( anObject->destinationObject != kNoDestinationObject)
        {
            targetObject = anObject->destObjectPtr;
            if (( targetObject != nil) && ( targetObject->active) &&
                ( targetObject->id == anObject->destObjectID))
            {
                if ( targetObject->seenByPlayerFlags &
                    anObject->myPlayerFlag)
                {
                    dest.h = targetObject->location.h;
                    dest.v = targetObject->location.v;
                    anObject->destinationLocation.h = dest.h;
                    anObject->destinationLocation.v = dest.v;
                } else
                {
                    dest.h = anObject->destinationLocation.h;
                    dest.v = anObject->destinationLocation.v;
                }
                anObject->destObjectDest =
                    targetObject->destinationObject;
                anObject->destObjectDestID = targetObject->destObjectID;
            } else
            {
                anObject->duty = eNoDuty;
                anObject->attributes &= ~kStaticDestination;
                WriteDebugLine(reinterpret_cast<const char*>("\pOddness"));
                if ( targetObject == nil)
                {
                    keysDown |= kDownKey;
                    anObject->destinationObject = kNoDestinationObject;
                    anObject->destObjectDest = kNoDestinationObject;
                    dest.h = anObject->location.h;
                    dest.v = anObject->location.v;
                } else
                {
                    anObject->destinationObject =
                        anObject->destObjectDest;
                    if ( anObject->destinationObject !=
                        kNoDestinationObject)
                    {
                        targetObject = *gSpaceObjectData + anObject->destinationObject;
                        if ( targetObject->id != anObject->destObjectDestID) targetObject = nil;
                    } else targetObject = nil;
                    if ( targetObject != nil)
                    {
                        anObject->destObjectPtr = targetObject;
                        anObject->destObjectID = targetObject->id;
                        anObject->destObjectDest =
                            targetObject->destinationObject;
                        anObject->destObjectDestID = targetObject->destObjectID;
                        dest.h = targetObject->location.h;
                        dest.v = targetObject->location.v;
                    } else
                    {
                        keysDown |= kDownKey;
                        anObject->destinationObject =
                            kNoDestinationObject;
                        anObject->destObjectDest = kNoDestinationObject;
                        anObject->destObjectPtr = nil;
                        dest.h = anObject->location.h;
                        dest.v = anObject->location.v;
                    }
                }
            }
        } else // no destination object; just coords
        {
            if (anObject->attributes & kOnAutoPilot)
            {
                TogglePlayerAutoPilot( anObject);
            }
            targetObject = nil;
            dest.h = anObject->location.h;
            dest.v = anObject->location.v;
        }

        difference = ABS<int>( dest.h - anObject->location.h);
        dcalc = difference;
        difference =  ABS<int>( dest.v - anObject->location.v);
        distance = difference;
        if (( dcalc > kMaximumAngleDistance) ||
            ( distance > kMaximumAngleDistance))
        {
            if (( dcalc > kMaximumRelevantDistance) ||
                ( distance > kMaximumRelevantDistance))
            {
                distance = kMaximumRelevantDistanceSquared;
            } else
            {
                distance = distance * distance + dcalc * dcalc;
            }
            shortx = (anObject->location.h - dest.h) >> 4;
            shorty = (anObject->location.v - dest.v) >> 4;
            // find angle between me & dest
            #ifdef powerc
            slope = MyFixRatio( shortx, shorty);
            #else
            slope = FixRatio( shortx, shorty);
            #endif
            angle = AngleFromSlope( slope);
            if ( shortx > 0)
            {
                mAddAngle( angle, 180);
            } else if (( shortx == 0) && ( shorty > 0))
            {
                angle = 0;
            }
        }
        else
        {
            distance = distance * distance + dcalc * dcalc;

            // find angle between me & dest
            #ifdef powerc
            slope = MyFixRatio(anObject->location.h - dest.h,
                        anObject->location.v - dest.v);
            #else
            slope = FixRatio(anObject->location.h - dest.h,
                        anObject->location.v - dest.v);
            #endif
            angle = AngleFromSlope( slope);

            if ( dest.h < anObject->location.h)
                mAddAngle( angle, 180);
            else if (( anObject->location.h == dest.h) &&
                    ( dest.v < anObject->location.v))
                angle = 0;
        }

        if ( anObject->attributes & kHasDirectionGoal)
        {
            theta = mAngleDifference( angle, anObject->directionGoal);
            if ( ABS( theta) > kDirectionError)
            {
                anObject->directionGoal = angle;

            }

            theta = mAngleDifference( anObject->direction,
                    anObject->directionGoal);
            theta = ABS( theta);
        } else
        {
            anObject->direction = angle;
            theta = 0;
        }


    }
    if ( distance > kLandingDistance)
    {
        if ( theta < kEvadeAngle)
            keysDown |= kUpKey;
        else keysDown |= kDownKey;
        anObject->lastTargetDistance = distance;
    } else
    {
        keysDown |= kDownKey;
        anObject->presenceData =
            (
                (
                    anObject->presenceData &
                        kPresenceDataLoWordMask
                ) -
                (
                    (
                        anObject->presenceData &
                            kPresenceDataHiWordMask
                    )
                    >> kPresenceDataHiWordShift
                )
            ) |
            (
                anObject->presenceData & kPresenceDataHiWordMask
            );
    }

    if ( (anObject->presenceData & kPresenceDataLoWordMask) <= 0)
    {
        ExecuteObjectActions( anObject->baseType->expireAction,
                            anObject->baseType->expireActionNum
                             & kDestroyActionNotMask,
                            anObject, targetObject, nil, true);
        anObject->active = kObjectToBeFreed;

    } else if ( anObject->sprite != nil)
        anObject->sprite->scale = (anObject->presenceData &
            kPresenceDataLoWordMask);

    return( keysDown);
}

// this gets the distance & angle between an object and arbitrary coords
void ThinkObjectGetCoordVector( spaceObjectType *anObject, coordPointType *dest, unsigned long *distance, short *angle)
{
    long            difference;
    unsigned long   dcalc;
    short           shortx, shorty;
    Fixed           slope;

    difference = ABS<int>( dest->h - anObject->location.h);
    dcalc = difference;
    difference =  ABS<int>( dest->v - anObject->location.v);
    *distance = difference;
    if (( *distance == 0) && ( dcalc == 0))
    {
        *angle = anObject->direction;
        return;
    }

    if (( dcalc > kMaximumAngleDistance) ||
        ( *distance > kMaximumAngleDistance))
    {
        if (( dcalc > kMaximumRelevantDistance) ||
            ( *distance > kMaximumRelevantDistance))
        {
            *distance = kMaximumRelevantDistanceSquared;
        } else
        {
            *distance = *distance * *distance + dcalc * dcalc;
        }
        shortx = (anObject->location.h - dest->h) >> 4;
        shorty = (anObject->location.v - dest->v) >> 4;
        // find angle between me & dest
        #ifdef powerc
        slope = MyFixRatio( shortx, shorty);
        #else
        slope = FixRatio( shortx, shorty);
        #endif
        *angle = AngleFromSlope( slope);
        if ( shortx > 0)
        {
            mAddAngle( *angle, 180);
        } else if (( shortx == 0) && ( shorty > 0))
        {
            *angle = 0;
        }
    }
    else
    {
        *distance = *distance * *distance + dcalc * dcalc;

        // find angle between me & dest
        #ifdef powerc
        slope = MyFixRatio(anObject->location.h - dest->h,
                    anObject->location.v - dest->v);
        #else
        slope = FixRatio(anObject->location.h - dest->h,
                    anObject->location.v - dest->v);
        #endif
        *angle = AngleFromSlope( slope);

        if ( dest->h < anObject->location.h)
            mAddAngle( *angle, 180);
        else if (( anObject->location.h == dest->h) &&
                ( dest->v < anObject->location.v))
            *angle = 0;
    }
}

void ThinkObjectGetCoordDistance( spaceObjectType *anObject, coordPointType *dest, unsigned long *distance)
{
    long            difference;
    unsigned long   dcalc;

    difference = ABS<int>( dest->h - anObject->location.h);
    dcalc = difference;
    difference =  ABS<int>( dest->v - anObject->location.v);
    *distance = difference;
    if (( *distance == 0) && ( dcalc == 0))
    {
        return;
    }

    if (( dcalc > kMaximumAngleDistance) ||
        ( *distance > kMaximumAngleDistance))
    {
        if (( dcalc > kMaximumRelevantDistance) ||
            ( *distance > kMaximumRelevantDistance))
        {
            *distance = kMaximumRelevantDistanceSquared;
        } else
        {
            *distance = *distance * *distance + dcalc * dcalc;
        }
    } else
    {
        *distance = *distance * *distance + dcalc * dcalc;
    }
}

// this resolves an object's destination to its coordinates, returned in dest
void ThinkObjectResolveDestination( spaceObjectType *anObject, coordPointType *dest, spaceObjectType **targetObject)
{
    *targetObject = nil;

    if (( anObject->attributes & kIsDestination) ||
        (( anObject->destinationObject == kNoDestinationObject) &&
        ( anObject->destinationLocation.h == kNoDestinationCoord)))
    {
        if (anObject->attributes & kOnAutoPilot)
        {
            TogglePlayerAutoPilot( anObject);
        }
        dest->h = anObject->location.h;
        dest->v = anObject->location.v;
    } else
    {
        if ( anObject->destinationObject != kNoDestinationObject)
        {
            *targetObject = anObject->destObjectPtr;
            if (( *targetObject != nil) && ( (*targetObject)->active) &&
                ( (*targetObject)->id == anObject->destObjectID))
            {
                if ( (*targetObject)->seenByPlayerFlags &
                    anObject->myPlayerFlag)
                {
                    dest->h = (*targetObject)->location.h;
                    dest->v = (*targetObject)->location.v;
                    anObject->destinationLocation.h = dest->h;
                    anObject->destinationLocation.v = dest->v;
                } else
                {
                    dest->h = anObject->destinationLocation.h;
                    dest->v = anObject->destinationLocation.v;
                }
                anObject->destObjectDest =
                    (*targetObject)->destinationObject;
                anObject->destObjectDestID = (*targetObject)->destObjectID;
            } else
            {
                anObject->duty = eNoDuty;
                anObject->attributes &= ~kStaticDestination;
                if ( (*targetObject) == nil)
                {
                    anObject->destinationObject = kNoDestinationObject;
                    anObject->destObjectDest = kNoDestinationObject;
                    dest->h = anObject->location.h;
                    dest->v = anObject->location.v;
                } else
                {
                    anObject->destinationObject =
                        anObject->destObjectDest;
                    if ( anObject->destinationObject !=
                        kNoDestinationObject)
                    {
                        (*targetObject) = *gSpaceObjectData + anObject->destinationObject;
                        if ( (*targetObject)->id != anObject->destObjectDestID) *targetObject = nil;
                    } else *targetObject = nil;
                    if ( *targetObject != nil)
                    {
                        anObject->destObjectPtr = (*targetObject);
                        anObject->destObjectID = (*targetObject)->id;
                        anObject->destObjectDest =
                            (*targetObject)->destinationObject;
                        anObject->destObjectDestID = (*targetObject)->destObjectID;
                        dest->h = (*targetObject)->location.h;
                        dest->v = (*targetObject)->location.v;
                    } else
                    {
                        anObject->duty = eNoDuty;
                        anObject->destinationObject =
                            kNoDestinationObject;
                        anObject->destObjectDest = kNoDestinationObject;
                        anObject->destObjectPtr = nil;
                        dest->h = anObject->location.h;
                        dest->v = anObject->location.v;
                    }
                }
            }
        } else // no destination object; just coords
        {
            (*targetObject) = nil;
            if ( anObject->destinationLocation.h == kNoDestinationCoord)
            {
                if (anObject->attributes & kOnAutoPilot)
                {
                    TogglePlayerAutoPilot( anObject);
                }
                dest->h = anObject->location.h;
                dest->v = anObject->location.v;
            } else
            {
                dest->h = anObject->destinationLocation.h;
                dest->v = anObject->destinationLocation.v;
            }
        }
    }
}

Boolean ThinkObjectResolveTarget( spaceObjectType *anObject, coordPointType *dest,
    unsigned long *distance, spaceObjectType **targetObject)
{
    spaceObjectType *closestObject;

    dest->h = dest->v = 0xffffffff;
    *distance = 0xffffffff;

    if ( anObject->closestObject != kNoShip)
    {
        closestObject = *gSpaceObjectData + anObject->closestObject;
    } else closestObject = nil;

    // if we have no target  then
    if (( anObject->targetObjectNumber == kNoShip))
    {
        // if the closest object is appropriate (if it exists, it should be, then
        if (( closestObject != nil) &&
            ( closestObject->attributes & kPotentialTarget))
        {
            // select closest object as target (and for now be satisfied with our direction
            if ( anObject->attributes & kHasDirectionGoal)
            {
                anObject->directionGoal = anObject->direction;
            }
            anObject->targetObjectNumber = anObject->closestObject;
            anObject->targetObjectID = closestObject->id;
        } else // otherwise, no target, no closest, cancel
        {
            closestObject = *targetObject = nil;
            anObject->targetObjectNumber = kNoShip;
            anObject->targetObjectID = kNoShip;
            dest->h = anObject->location.h;
            dest->v = anObject->location.v;
            *distance = anObject->engageRange;
            return ( false);
        }
    }

    // if we have a target of any kind (we must by now)
    if ( anObject->targetObjectNumber != kNoShip)
    {
        // make sure we're still talking about the same object
        *targetObject = *gSpaceObjectData + anObject->targetObjectNumber;

        // if the object is wrong or smells at all funny, then
        if  (
                (
                    !((*targetObject)->active)
                )
            ||
                ( (*targetObject)->id != anObject->targetObjectID)
            ||
                (
                    ( (*targetObject)->owner == anObject->owner)
                &&
                    ( (*targetObject)->attributes & kHated)
                )
            ||
                (
                    (
                        !((*targetObject)->attributes & kPotentialTarget)
                    )
                &&
                    (
                        !((*targetObject)->attributes & kHated)
                    )
                )
            )
        {
            // if we have a closest ship
            if ( anObject->closestObject != kNoShip)
            {
                // make it our target
                anObject->targetObjectNumber = anObject->closestObject;
                closestObject = *targetObject = *gSpaceObjectData + anObject->targetObjectNumber;
                anObject->targetObjectID = closestObject->id;
                if ( !((*targetObject)->attributes & kPotentialTarget))
                {   // cancel
                    *targetObject = nil;
                    anObject->targetObjectNumber = kNoShip;
                    anObject->targetObjectID = kNoShip;
                    dest->h = anObject->location.h;
                    dest->v = anObject->location.v;
                    *distance = anObject->engageRange;
                    return ( false);
                }
            } else // no legal target, no closest, cancel
            {
                closestObject = *targetObject = nil;
                anObject->targetObjectNumber = kNoShip;
                anObject->targetObjectID = kNoShip;
                dest->h = anObject->location.h;
                dest->v = anObject->location.v;
                *distance = anObject->engageRange;
                return ( false);
            }
        }/* else // the target *is* legal
        {
            if ( anObject->attributes & kIsGuided)
            {
                if (((!(targetObject->attributes & kHated)) ||
                    ( !(targetObject->active))) &&
                    ( anObject->closestObject != kNoShip))
                {
                    closestObject = *gSpaceObjectData + anObject->closestObject;
                    if ( ( closestObject->attributes & kHated))
                    {
                        targetObject = closestObject;
                        anObject->targetObjectNumber =
                            anObject->closestObject;
                        anObject->targetObjectID = targetObject->id;
                    }
                }
            }
        }*/

        dest->h = (*targetObject)->location.h;
        dest->v = (*targetObject)->location.v;

        // if it's not the closest object & we have a closest object
        if ((anObject->closestObject != kNoShip) &&
            ( anObject->targetObjectNumber != anObject->closestObject)
            && ( !(anObject->attributes & kIsGuided))
            && ( closestObject->attributes & kPotentialTarget))
        {
            // then calculate the distance
            ThinkObjectGetCoordDistance( anObject, dest, distance);

            if (( ( *distance >> 1L) > anObject->closestDistance) ||
                ( ! (anObject->attributes & kCanEngage)) ||
                ( anObject->attributes &
                kRemoteOrHuman))
            {
                anObject->targetObjectNumber = anObject->closestObject;
                *targetObject = *gSpaceObjectData + anObject->targetObjectNumber;
                anObject->targetObjectID = (*targetObject)->id;
                dest->h = (*targetObject)->location.h;
                dest->v = (*targetObject)->location.v;
                *distance = anObject->closestDistance;
                if ( (*targetObject)->cloakState > 250)
                {
                    dest->h -= 200;
                    dest->v -= 200;
                }
            }
            return( true);
        } else // if target is closest object
        {
            // otherwise distance is the closestDistance
            *distance = anObject->closestDistance;
            return( true);
        }
    } else // we don't have a target object
    {
        // set the distance to the engage range ie nothing to engage
        closestObject = *targetObject = nil;
        anObject->targetObjectNumber = kNoShip;
        anObject->targetObjectID = kNoShip;
        dest->h = anObject->location.h;
        dest->v = anObject->location.v;
        *distance = anObject->engageRange;
        return ( false);
    }
}

unsigned long ThinkObjectEngageTarget( spaceObjectType *anObject, spaceObjectType *targetObject,
    unsigned long distance, short *theta, long timePass)
{
    unsigned long   keysDown = 0;
    baseObjectType  *bestWeapon, *weaponObject;
    coordPointType  dest;
    long            difference;
    short           angle, beta;
    Fixed           slope;

    *theta = 0xffff;

    dest.h = targetObject->location.h;
    dest.v = targetObject->location.v;
    if ( targetObject->cloakState > 250)
    {
        dest.h -= 70;
        dest.h += RandomSeeded( 140, &anObject->randomSeed,
            'nps0', anObject->whichBaseObject);
        dest.v -= 70;
        dest.v += RandomSeeded( 140, &anObject->randomSeed,
            'nps1', anObject->whichBaseObject);
    }

    // if target is in our weapon range & we hate the object
    if (    ( distance < anObject->longestWeaponRange) &&
            ( targetObject->attributes & kCanBeEngaged) &&
            ( targetObject->attributes & kHated)
        )
    {
        // find "best" weapon (how do we want to aim?)
        // difference = closest range

        if ( anObject->attributes & kCanAcceptDestination)
        {
            anObject->timeFromOrigin += timePass;
        }

        difference = anObject->longestWeaponRange;

        if ( anObject->beamType != kNoWeapon)
        {
            bestWeapon = weaponObject = anObject->beamBase;
            if ( ( weaponObject->frame.weapon.usage &
                kUseForAttacking) &&
                ( weaponObject->frame.weapon.range >= distance)
                &&
                ( weaponObject->frame.weapon.range <
                difference))
            {
                bestWeapon = weaponObject;
                difference = weaponObject->frame.weapon.range;
            }
        }

        if ( anObject->pulseType != kNoWeapon)
        {
            weaponObject = anObject->pulseBase;
            if ( ( weaponObject->frame.weapon.usage &
                kUseForAttacking) &&
                ( weaponObject->frame.weapon.range >= distance)
                && ( weaponObject->frame.weapon.range <
                difference))
            {
                bestWeapon = weaponObject;
                difference = weaponObject->frame.weapon.range;
            }
        }

        if ( anObject->specialType != kNoWeapon)
        {
            weaponObject = anObject->specialBase;
            if ( ( weaponObject->frame.weapon.usage &
                kUseForAttacking) &&
                ( weaponObject->frame.weapon.range >= distance)
                && ( weaponObject->frame.weapon.range <
                difference))
            {
                bestWeapon = weaponObject;
                difference = weaponObject->frame.weapon.range;
            }
        }
//      dest.h = targetObject->location.h;
//      dest.v = targetObject->location.v;
    } // target is not in our weapon range (or we don't hate it)

    // We don't need to worry if it is very far away, since it must be within farthest weapon range
    // find angle between me & dest
    #ifdef powerc
    slope = MyFixRatio(anObject->location.h - dest.h,
                anObject->location.v - dest.v);
    #else
    slope = FixRatio(anObject->location.h - dest.h,
                anObject->location.v - dest.v);
    #endif
    angle = AngleFromSlope( slope);

    if ( dest.h < anObject->location.h)
        mAddAngle( angle, 180);
    else if (( anObject->location.h == dest.h) &&
            ( dest.v < anObject->location.v))
        angle = 0;

    if ( targetObject->cloakState > 250)
    {
        angle -= 45;
        mAddAngle( angle, RandomSeeded( 90, &anObject->randomSeed, 'nps2', anObject->whichBaseObject));
    }
    anObject->targetAngle = angle;

    if ( anObject->attributes & kHasDirectionGoal)
    {
        *theta = mAngleDifference( angle, anObject->directionGoal);
        if (( ABS( *theta) > kDirectionError) ||
            ( !(anObject->attributes & kIsGuided)))
        {
            anObject->directionGoal = angle;

        }

        beta = targetObject->direction;
        mAddAngle( beta, ROT_180);
        *theta = mAngleDifference( beta, angle);
    } else
    {
        anObject->direction = angle;
        *theta = 0;
    }

    // if target object is in range
    if (( distance < anObject->longestWeaponRange) &&
        ( targetObject->attributes & kHated))
    {
        // fire away
        beta = anObject->direction;
        beta = mAngleDifference( beta, angle);

        if ( anObject->pulseType != kNoWeapon)
        {
            weaponObject = anObject->pulseBase;
            if (( weaponObject->frame.weapon.usage &
                kUseForAttacking) &&
                (( ABS( beta) <= kShootAngle) ||
                ( weaponObject->attributes & kAutoTarget)) &&
                ( distance < weaponObject->frame.weapon.range))
            {
                keysDown |= kOneKey;
            }
        }

        if ( anObject->beamType != kNoWeapon)
        {
            weaponObject = anObject->beamBase;
            if (( weaponObject->frame.weapon.usage &
                kUseForAttacking) &&
                (( ABS( beta) <= kShootAngle) ||
                ( weaponObject->attributes & kAutoTarget)) &&
                ( distance < weaponObject->frame.weapon.range))
            {
                keysDown |= kTwoKey;
            }
        }

        if ( anObject->specialType != kNoWeapon)
        {
            weaponObject = anObject->specialBase;
            if (( weaponObject->frame.weapon.usage &
                kUseForAttacking) &&
                (( ABS( beta) <= kShootAngle) ||
                ( weaponObject->attributes & kAutoTarget)) &&
                ( distance < weaponObject->frame.weapon.range))
            {
                keysDown |= kEnterKey;
            }
        }
    }// target is not in range
    return( keysDown);
}

void HitObject( spaceObjectType *anObject, spaceObjectType *sObject)

{
    long            count;
    Str255  s;

/*  DebugFileAppendString( "\pHT\t");
    DebugFileAppendLong( anObject->entryNumber);
    DebugFileAppendString( "\p\t");
    DebugFileAppendLong( sObject->entryNumber);
    DebugFileAppendString( "\p\t");
    DebugFileAppendLong( (anObject->location.h - sObject->location.h) + ( anObject->location.v - sObject->location.v));
    DebugFileAppendString( "\p\r");
*/
    if ( anObject->active == kObjectInUse)
    {
        anObject->timeFromOrigin = 0;
        if (( (anObject->health - sObject->baseType->damage) < 0)
            && ( anObject->attributes & (kIsPlayerShip | kRemoteOrHuman)) &&
            (!(anObject->baseType->destroyActionNum & kDestroyActionDontDieFlag)))
        {
            CreateFloatingBodyOfPlayer( anObject);
        }
        AlterObjectHealth( anObject, -(sObject->baseType->damage));
        if ( anObject->shieldColor != kNoTinyColor)
        {
            anObject->hitState = ( anObject->health * kHitStateMax) / anObject->baseType->health;
            anObject->hitState += 16;
        }

        if ( anObject->cloakState > 0) anObject->cloakState = 1;

        if ( anObject->health < 0)
        {
//          if ( anObject->attributes & (kIsPlayerShip | kRemoteOrHuman))
//          {
//              CreateFloatingBodyOfPlayer( anObject);
//          }
            if (( anObject->owner == gAresGlobal->gPlayerAdmiralNumber) &&
                ( anObject->attributes & kCanAcceptDestination))
            {
                StartMessage();
                AppendStringToMessage("\p ");
                GetIndString( s, 5000, anObject->whichBaseObject + 1);
                AppendStringToMessage(s);
                AppendStringToMessage("\p destroyed.  ");
                count = CountObjectsOfBaseType( anObject->whichBaseObject, anObject->owner) - 1;
                NumToString( count, s);
                AppendStringToMessage(s);
                AppendStringToMessage("\p remaining. ");
                EndMessage();

//              if ( anObject->attributes & (kIsPlayerShip | kIsRemote | kIsHumanControlled))
//              {
//                  CreateFloatingBodyOfPlayer( anObject);
                    /*
                    // turn the player into a body
                    count = CreateAnySpaceObject( gAresGlobal->scenarioFileInfo.playerBodyID, &(anObject->velocity),
                        &(anObject->location), anObject->direction, anObject->owner, 0, nil, -1, -1, -1);
                    if ( count >= 0)
                    {
                        attributes = anObject->attributes & ( kIsHumanControlled | kIsPlayerShip);
                        anObject->attributes &= (~kIsHumanControlled) & (~kIsPlayerShip);
                        gAresGlobal->gPlayerShipNumber = count;
                        ResetScrollStars( gAresGlobal->gPlayerShipNumber);
                        anObject = *gSpaceObjectData + gAresGlobal->gPlayerShipNumber;
                        anObject->attributes |= attributes;
                    } else
                    {
                        gScrollStarObject = nil;
                        gAresGlobal->gScrollStarNumber = -1;
                        gAresGlobal->gPlayerShipNumber = -1;
                        anObject->health = 1000;
                        gAresGlobal->gGameOver = -360;
                    }
                    */
//              }
            }
        }

        if ( sObject->active == kObjectInUse)
        {
            ExecuteObjectActions( sObject->baseType->collideAction,
                                sObject->baseType->collideActionNum,
                                sObject, anObject, nil, true);
        }

        if ( anObject->owner == gAresGlobal->gPlayerAdmiralNumber)
        {
            if ((anObject->attributes & kIsHumanControlled) && ( sObject->baseType->damage > 0))
            {
//#ifndef   powercc
                StartBooleanColorAnimation( 128, 128, WHITE);//GetTranslateColorShade( AQUA, VERY_LIGHT));
//#endif
            }
        }
    }
}


// GetManualSelectObject:
//  For the human player selecting a ship.  If friend or foe = 0, will get any ship.  If it's
//  positive, will get only friendly ships.  If it's negative, only unfriendly ships.

long GetManualSelectObject( spaceObjectType *sourceObject, unsigned long inclusiveAttributes,
                            unsigned long anyOneAttribute, unsigned long exclusiveAttributes,
                            const UnsignedWide *fartherThan, long currentShipNum, short friendOrFoe)

{
    spaceObjectType *anObject;
    long            whichShip = 0, resultShip = -1, closestShip = -1, startShip = -1, hdif, vdif;
    unsigned long   distance, dcalc, myOwnerFlag = 1 << sourceObject->owner;
    long            difference;
    Fixed           slope;
    short           angle;
//  const wide      kMaxAngleDistance = {0, 1073676289}; // kMaximumAngleDistance ^ 2
    UnsignedWide    wideClosestDistance = {kUniversalCenter, kUniversalCenter}, wideFartherDistance = {kUniversalCenter, kUniversalCenter},
                    thisWideDistance, wideScrap;
    unsigned char   thisDistanceState;

    // Here's what you've got to do next:
    // start with the currentShipNum
    // try to get any ship but the current ship
    // stop trying when we've made a full circle (we're back on currentShipNum)

    WriteDebugLine(reinterpret_cast<const char*>("\pSelecting"));
    WriteDebugLong( currentShipNum);
    whichShip = startShip = currentShipNum;
    if ( whichShip >= 0)
    {
        anObject = *gSpaceObjectData + startShip;
        if ( anObject->active != kObjectInUse) // if it's not in the loop
        {
            anObject = gRootObject;
            startShip = whichShip = gRootObjectNumber;
        }

    } else
    {
        anObject = gRootObject;
        startShip = whichShip = gRootObjectNumber;
    }

    if ( anObject == nil) DebugStr("\pUnexpected error: there don't seem to be any objects.");
    do
    {
        if (( anObject->active) && ( anObject != sourceObject) &&
            ( anObject->seenByPlayerFlags & myOwnerFlag) &&
            (( anObject->attributes & inclusiveAttributes) == inclusiveAttributes) &&
            (( anyOneAttribute == 0) || (( anObject->attributes & anyOneAttribute) != 0)) &&
            ( !(anObject->attributes & exclusiveAttributes)) && ((( friendOrFoe < 0) &&
            ( anObject->owner != sourceObject->owner)) || (( friendOrFoe > 0) &&
            ( anObject->owner == sourceObject->owner)) || ( friendOrFoe == 0)))
        {
            difference = ABS<int>( sourceObject->location.h - anObject->location.h);
            dcalc = difference;
            difference =  ABS<int>( sourceObject->location.v - anObject->location.v);
            distance = difference;

            if (( dcalc > kMaximumRelevantDistance) ||
                ( distance > kMaximumRelevantDistance))
            {
                wideScrap.hi = 0;
                wideScrap.lo = dcalc;   // must be positive
                MyWideMul( wideScrap.lo, wideScrap.lo, &thisWideDistance);  // ppc automatically generates WideMultiply
                wideScrap.lo = distance;
                MyWideMul( wideScrap.lo, wideScrap.lo, &wideScrap);
                MyWideAdd( &thisWideDistance, &wideScrap);
            } else
            {
                thisWideDistance.hi = 0;
                thisWideDistance.lo = distance * distance + dcalc * dcalc;
            }

            thisDistanceState = 0;
/*          WriteDebugDivider();
            WriteDebugLong( anObject->entryNumber);
            WriteDebugLong( anObject->distanceFromPlayer.hi);
            WriteDebugLong( anObject->distanceFromPlayer.lo);
*/          if ( mWideIsGreaterThan( wideClosestDistance, thisWideDistance))
            {
                thisDistanceState |= kCloserThanClosest;
            }


            if ( (( mWideIsGreaterThan( thisWideDistance, *fartherThan)) &&
                ( mWideIsGreaterThan( wideFartherDistance, thisWideDistance))) ||

                (( mWideIsGreaterThan( wideFartherDistance, thisWideDistance)) &&
                ( mWideIsGreaterThanOrEqual( thisWideDistance, *fartherThan))) &&
                ( whichShip > currentShipNum))
            {
                thisDistanceState |= kFartherThanFarther;
            }

            if ( thisDistanceState)
            {
//              Debugger();
                hdif = sourceObject->location.h - anObject->location.h;
                vdif = sourceObject->location.v - anObject->location.v;
                while (((ABS(hdif)) > kMaximumAngleDistance) || ( (ABS(vdif)) > kMaximumAngleDistance))
                {
                    hdif >>= 1;
                    vdif >>= 1;
                }

                #ifdef powerc
                slope = MyFixRatio(hdif, vdif);
                #else
                slope = FixRatio(hdif, vdif);
                #endif
                angle = AngleFromSlope( slope);

                if ( hdif > 0)
                    mAddAngle( angle, 180);
                else if (( hdif == 0) &&
                        ( vdif > 0))
                    angle = 0;

                angle = mAngleDifference( angle, sourceObject->direction);

                if ( ABS( angle) < 30)
                {
                    if ( thisDistanceState & kCloserThanClosest)
                    {
                        closestShip = whichShip;
                        wideClosestDistance = thisWideDistance;
                    }

                    if ( thisDistanceState & kFartherThanFarther)
                    {
                        resultShip = whichShip;
                        wideFartherDistance = thisWideDistance;
                    }
                }
            }

/*          difference = anObject->location.h - sourceObject->location.h;
            difference = ABS( difference);
            if ( difference < kMaximumAngleDistance)
            {
                udif = difference;
                udif *= udif;

                difference = anObject->location.v - sourceObject->location.v;
                difference = ABS( difference);
                if ( difference < kMaximumAngleDistance)
                {
                    distance = difference;
                    distance *= distance;
                    distance += udif;

                    if (( distance < closestDistance) ||
                        (( distance < fartherDistance) && ( distance > fartherThan)) ||
                        (( distance < fartherDistance) && ( distance >= fartherThan) && ( whichShip > currentShipNum)))
                    {
                        #ifdef powerc
                        slope = MyFixRatio( (int)ABS(sourceObject->location.h - anObject->location.h),
                                    (int)(sourceObject->location.v - anObject->location.v));
                        #else
                        slope = FixRatio( (int)ABS(sourceObject->location.h - anObject->location.h),
                                    (int)(sourceObject->location.v - anObject->location.v));
                        #endif
                        angle = AngleFromSlope( slope);

                        if ( anObject->location.h < sourceObject->location.h)
                            mAddAngle( angle, 180)
                        else if (( anObject->location.h == sourceObject->location.h) &&
                                ( anObject->location.v < sourceObject->location.v))
                            angle = 0;

//                      hackcheck = angle;
                        angle = mAngleDifference( angle, sourceObject->direction);
                        if ( ABS( angle) < 30)
                        {
                            if ( distance < closestDistance)
                            {
                                closestShip = whichShip;
                                closestDistance = distance;
                            }
                            if ((( distance > fartherThan) && ( distance < fartherDistance)) ||
                                (( distance < fartherDistance) && (distance >= fartherThan) && ( whichShip != currentShipNum)))
                            {
                                resultShip = whichShip;
                                fartherDistance = distance;
                            }
                        }
                    }
                }
            }
*/
        }
        whichShip = anObject->nextObjectNumber;
        anObject = anObject->nextObject;
        if ( anObject == nil)
        {
            whichShip = gRootObjectNumber;
            anObject = gRootObject;
        }
    } while ( whichShip != startShip);
    if ((( resultShip == -1) && ( closestShip != -1)) || ( resultShip == currentShipNum)) resultShip = closestShip;

    return ( resultShip);
}

long GetSpritePointSelectObject( Rect *bounds, spaceObjectType *sourceObject, unsigned long inclusiveAttributes,
                            unsigned long anyOneAttribute, unsigned long exclusiveAttributes,
                            long currentShipNum, short friendOrFoe)

{
    spaceObjectType *anObject;
    long            whichShip = 0, resultShip = -1, closestShip = -1;
    unsigned long   closestDistance = kMaximumRelevantDistanceSquared << 1,
                    fartherDistance = kMaximumRelevantDistanceSquared << 1,
                    myOwnerFlag = 1 << sourceObject->owner;

    anObject = *gSpaceObjectData;

    for ( whichShip = 0; whichShip < kMaxSpaceObject; whichShip++)
    {
        if (( anObject->active) && ( anObject->sprite != nil) &&
            ( anObject->seenByPlayerFlags & myOwnerFlag) &&
            (( anObject->attributes & inclusiveAttributes) == inclusiveAttributes) &&
            (( anyOneAttribute == 0) || (( anObject->attributes & anyOneAttribute) != 0)) &&
            ( !(anObject->attributes & exclusiveAttributes)) && ((( friendOrFoe < 0) &&
            ( anObject->owner != sourceObject->owner)) || (( friendOrFoe > 0) &&
            ( anObject->owner == sourceObject->owner)) || ( friendOrFoe == 0)))
        {
/*          if ( !((bounds->right < anObject->sprite->thisRect.left) || (bounds->bottom <
                anObject->sprite->thisRect.top) || ( bounds->left > anObject->sprite->thisRect.right)
                || ( bounds->top > anObject->sprite->thisRect.bottom)))
            {
                if ( anObject->sprite->thisRect.right > anObject->sprite->thisRect.left)
                {
                    if ( closestShip < 0) closestShip = whichShip;
                    if (( whichShip > currentShipNum) && ( resultShip < 0)) resultShip = whichShip;
                }
            }
*/          if ( !((bounds->right < anObject->sprite->where.h) || (bounds->bottom <
                anObject->sprite->where.v) || ( bounds->left > anObject->sprite->where.h)
                || ( bounds->top > anObject->sprite->where.v)))
            {
//              if ( anObject->sprite->thisRect.right > anObject->sprite->thisRect.left)
                {
                    if ( closestShip < 0) closestShip = whichShip;
                    if (( whichShip > currentShipNum) && ( resultShip < 0)) resultShip = whichShip;
                }
            }
        }
        anObject++;
    }
    if ((( resultShip == -1) && ( closestShip != -1)) || ( resultShip == currentShipNum)) resultShip = closestShip;

    return ( resultShip);
}

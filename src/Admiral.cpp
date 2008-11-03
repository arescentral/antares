/*
Ares, a tactical space combat game.
Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

// Admiral.c

#include "ConditionalMacros.h"

#if TARGET_OS_WIN32
    #include <QuickTimeVR.h>
    #include "QTUtilities.h"
    #include "QTVRUtilities.h"
    #include <TextUtils.h>
    #include <Script.h>
    #include <string.h>
#endif // TARGET_OS_WIN32

#include "Resources.h"
#include "SpaceObject.h"
#include "AresGlobalType.h"
#include "SpaceObjectHandling.h"
#include "StringHandling.h"
#include "UniverseUnit.h"
#include "AresCheat.h"
#include "AnyChar.h"
#include "MathMacros.h"
#include "Error.h"
#include "Debug.h"
#include "HandleHandling.h"
#include "SoundFX.h"
#include "Randomize.h"
#include "Admiral.h"

#define kConvoySpeed                3

#define kDestNoObject               -1

#define kAdmiralError       "\pADMR"

#define kUnimportantTarget          0x00000000 // 0xffffffff
#define kMostImportantTarget        0x00000200 // 0x00000800
#define kLeastImportantTarget       0x00000100
#define kVeryImportantTarget        0x00000160 // 0x00000600
#define kImportantTarget            0x00000140 // 0x00000400
#define kSomewhatImportantTarget    0x00000120 // 0x00000200
#define kAbsolutelyEssential        0x00008000

extern aresGlobalType   *gAresGlobal;
extern Handle       gSpaceObjectData, gBaseObjectData;
extern long         gRandomSeed, gRootObjectNumber;//, gAresGlobal->gPlayerAdmiralNumber;
//extern long           gAresGlobal->gGameTime; // for debugging only
extern spaceObjectType  *gRootObject;

//Handle                gAresGlobal->gAdmiralData = nil, gAresGlobal->gDestBalanceData = nil;

int AdmiralInit( void)

{
    gAresGlobal->gAdmiralData = NewHandle( sizeof( admiralType) * (long)kScenarioPlayerNum);

    if ( gAresGlobal->gAdmiralData == nil)
    {
        ShowErrorAny( eQuitErr, kErrorStrID, nil, nil, nil, nil, MEMORY_ERROR, -1, -1, -1, __FILE__, 1);
        return( MEMORY_ERROR);
    }

    /*
    MoveHHi( gAresGlobal->gAdmiralData);
    HLock( gAresGlobal->gAdmiralData);
    */
    mHandleLockAndRegister( gAresGlobal->gAdmiralData, nil, nil, nil, "\pgAresGlobal->gAdmiralData")
    ResetAllAdmirals();

    gAresGlobal->gDestBalanceData = NewHandle( sizeof( destBalanceType) * (long)kMaxDestObject);

    if ( gAresGlobal->gDestBalanceData == nil)
    {
        ShowErrorAny( eQuitErr, kErrorStrID, nil, nil, nil, nil, MEMORY_ERROR, -1, -1, -1, __FILE__, 2);
        return( MEMORY_ERROR);
    }
    mHandleLockAndRegister( gAresGlobal->gDestBalanceData, nil, nil, nil, "\pgAresGlobal->gDestBalanceData")

    ResetAllDestObjectData();

    return ( kNoError);
}

void AdmiralCleanup( void)

{
    if ( gAresGlobal->gAdmiralData != nil) DisposeHandle( gAresGlobal->gAdmiralData);
    if ( gAresGlobal->gDestBalanceData != nil) DisposeHandle( gAresGlobal->gDestBalanceData);
}

void ResetAllAdmirals( void)

{
    short       i, j;
    admiralType *a;

    a = ( admiralType *)*gAresGlobal->gAdmiralData;

    for ( i = 0; i < kScenarioPlayerNum; i++)
    {
        a->active = FALSE;
        a->attributes = 0;
        a->destinationObject = kNoDestinationObject;
        a->destinationObjectID = -1;
        a->flagship = kNoShip;
        a->flagshipID = -1;
        a->destType = kNoDestinationType;
        a->considerShip = a->considerDestination = kNoShip;
        a->considerShipID = -1;
        a->buildAtObject = kNoShip;
        a->cash = a->kills = a->losses =a->saveGoal = 0;
        a->thisFreeEscortStrength = a->lastFreeEscortStrength = 0;
        a->blitzkrieg = 0;
        a->shipsLeft = 0;
        for ( j = 0; j < kAdmiralScoreNum; j++) a->score[j] = 0;

        for ( j = 0; j < kMaxNumAdmiralCanBuild; j++)
        {
            a->canBuildType[j].baseNum = -1;
            a->canBuildType[j].base = nil;
            a->canBuildType[j].chanceRange = -1;
        }
        a++;
        gAresGlobal->gActiveCheats[i] = 0;
    }
}

void ResetAllDestObjectData( void)

{
    short           i, j;
    destBalanceType *d = mGetDestObjectBalancePtr( 0);

    for ( i = 0; i < kMaxDestObject; i++)
    {
        d->whichObject = kDestNoObject;
        d->name[0] = 0;
        d->earn = d->totalBuildTime = d->buildTime = 0;
        d->buildObjectBaseNum = kNoShip;
        for ( j = 0; j < kMaxTypeBaseCanBuild; j++)
        {
            d->canBuildType[j] = kNoShip;
        }
        for ( j = 0; j < kScenarioPlayerNum; j++)
        {
            d->occupied[j] = 0;
        }
        d++;
    }
}

long MakeNewAdmiral( long flagship, long destinationObject, destinationType dType,
                    unsigned long attributes, long race, short nameResID, short nameStrNum,
                    smallFixedType earningPower)

{
    long            n = 0, i;
    admiralType     *a;
    Str255          s;
    spaceObjectType *destObject;

    a = ( admiralType *)*gAresGlobal->gAdmiralData;

    while (( a->active) && ( n < kScenarioPlayerNum)) { a++; n++; }

    if ( n == kScenarioPlayerNum) return ( kNoFreeAdmiral);

    a->active = TRUE;
    a->attributes = attributes;
    a->earningPower = earningPower;

    a->destinationObject = destinationObject;
    if ( destinationObject >= 0)
    {
        destObject = (spaceObjectType *)*gSpaceObjectData + destinationObject;
        a->destinationObjectID = destObject->id;
    } else a->destinationObjectID = -1;

    a->flagship = flagship;
    if ( flagship >= 0)
    {
        destObject = (spaceObjectType *)*gSpaceObjectData + flagship;
        a->flagshipID = destObject->id;
    } else
    {
        a->flagshipID = -1;
    }

    a->destType = dType;
    a->race = race;
    a->color = 0;
    a->blitzkrieg = 1200;//2400;    // about a 2 minute blitzkrieg
    a->cash = a->kills = a->losses = a->saveGoal = 0;
    a->thisFreeEscortStrength = a->lastFreeEscortStrength = 0;
    for ( i = 0; i < kAdmiralScoreNum; i++) a->score[i] = 0;
    for ( i = 0; i < kMaxNumAdmiralCanBuild; i++)
    {
        a->canBuildType[i].baseNum = -1;
        a->canBuildType[i].base = nil;
        a->canBuildType[i].chanceRange = -1;
    }
    a->totalBuildChance = 0;
    a->hopeToBuild = -1;
    a->shipsLeft = 0;

    if (( nameResID >= 0))
    {
        GetIndString( s, nameResID, nameStrNum);
        if ( *s > kAdmiralNameLen) *s = kAdmiralNameLen;
        CopyPString( (unsigned char *)a->name, (unsigned char *)s);
    }
    // for now set strategy balance to 0 -- we may want to calc this if player added on the fly?
    return( n);
}

long MakeNewDestination( long whichObject, long *canBuildType, smallFixedType earn, short nameResID,
                        short nameStrNum)

{
    long            i = 0, j;
    destBalanceType *d = mGetDestObjectBalancePtr( 0);
    Str255          s;
    spaceObjectType *object = (spaceObjectType *)*gSpaceObjectData + whichObject;

    while (( i < kMaxDestObject) && ( d->whichObject != kDestNoObject)) { i++; d++;}
    if ( i == kMaxDestObject) return( -1);
    else
    {
        d->whichObject = whichObject;
        d->earn = earn;
        d->totalBuildTime = d->buildTime = 0;

        if ( canBuildType != nil)
        {
            for ( j = 0; j < kMaxTypeBaseCanBuild; j++)
            {
                d->canBuildType[j] = *canBuildType;
                canBuildType++;
            }
        } else
        {
            for ( j = 0; j < kMaxTypeBaseCanBuild; j++)
            {
                d->canBuildType[j] = kNoShip;
            }
        }

        if (( nameResID >= 0))
        {
            GetIndString( s, nameResID, nameStrNum);
            if ( *s > kDestinationNameLen) *s = kDestinationNameLen;
            CopyPString( (unsigned char *)d->name, (unsigned char *)s);
        }

        if ( object->attributes & kNeutralDeath)
        {
            for ( j = 0; j < kScenarioPlayerNum; j++)
                d->occupied[j] = 0;

            if ( object->owner >= 0)
                d->occupied[object->owner] = object->baseType->initialAgeRange;
        }

        return( i);
    }
}

void RemoveDestination( long whichDestination)

{
    destBalanceType *d = mGetDestObjectBalancePtr( whichDestination);
    admiralType     *a;
    short           i;

    if (( whichDestination >= 0) && ( whichDestination < kMaxDestObject))
    {
        a = ( admiralType *)*gAresGlobal->gAdmiralData;

        for ( i = 0; i < kScenarioPlayerNum; i++)
        {
            if ( a->active)
            {
    //          WriteDebugLong( i);
                if ( a->destinationObject == d->whichObject)
                {
                    a->destinationObject = kNoDestinationObject;
                    a->destinationObjectID = -1;
                    a->destType = kNoDestinationType;

    //              mWriteDebugString("\padestob");
                }
                if ( a->considerDestination == whichDestination)
                {
                    a->considerDestination = kNoDestinationObject;

    //              mWriteDebugString("\pacondest");
                }

                if ( a->buildAtObject == whichDestination)
                {
                    a->buildAtObject = kNoShip;

    //              mWriteDebugString("\pabuildob");
                }
            }
            a++;
        }

        d->whichObject = kDestNoObject;
        d->name[0] = 0;
        d->earn = d->totalBuildTime = d->buildTime = 0;
        d->buildObjectBaseNum = kNoShip;
        for ( i = 0; i < kMaxTypeBaseCanBuild; i++)
        {
            d->canBuildType[i] = kNoShip;
        }

        for ( i = 0; i < kScenarioPlayerNum; i++)
        {
            d->occupied[i] = 0;
        }
    } else
    {
        //Debugger();
    }
}

void RecalcAllAdmiralBuildData( void)
{
    short           i, j, k;
    admiralType     *a = ( admiralType *)*gAresGlobal->gAdmiralData;
    spaceObjectType *anObject= nil;
    baseObjectType  *baseObject = nil;
    destBalanceType *d = mGetDestObjectBalancePtr( 0);
    long            l;

    // first clear all the data
    for ( i = 0; i < kScenarioPlayerNum; i++)
    {
        for ( j = 0; j < kMaxNumAdmiralCanBuild; j++)
        {
            a->canBuildType[j].baseNum = -1;
            a->canBuildType[j].base = nil;
            a->canBuildType[j].chanceRange = -1;
        }
        a->totalBuildChance = 0;
        a->hopeToBuild = -1;
        a++;
    }

    for ( i = 0; i < kMaxDestObject; i++)
    {
        if ( d->whichObject != kDestNoObject)
        {
            anObject = (spaceObjectType *)*gSpaceObjectData + d->whichObject;
            if ( anObject->owner >= 0)
            {
                a = ( admiralType *)*gAresGlobal->gAdmiralData + anObject->owner;
                for ( k = 0; k < kMaxTypeBaseCanBuild; k++)
                {
                    if ( d->canBuildType[k] >= 0)
                    {
                        j = 0;
                        while ((a->canBuildType[j].baseNum != d->canBuildType[k]) && ( j < kMaxNumAdmiralCanBuild)) j++;
                        if ( j == kMaxNumAdmiralCanBuild)
                        {
                            mGetBaseObjectFromClassRace( baseObject, l, d->canBuildType[k], a->race)
                            j = 0;
                            while ((a->canBuildType[j].baseNum != -1) && ( j < kMaxNumAdmiralCanBuild)) j++;
                            if ( j == kMaxNumAdmiralCanBuild) MyDebugString("\pToo Many Types to Build!");
                            a->canBuildType[j].baseNum = d->canBuildType[k];
                            a->canBuildType[j].base = baseObject;
                            a->canBuildType[j].chanceRange = a->totalBuildChance;
                            a->totalBuildChance += baseObject->buildRatio;
                        }
                    }
                }
            }
        }
        d++;
    }
}

void SetAdmiralAttributes( long whichAdmiral, unsigned long attributes)
{
    admiralType     *a;

    a = ( admiralType *)*gAresGlobal->gAdmiralData + whichAdmiral;
    a->attributes = attributes;
}


void SetAdmiralColor( long whichAdmiral, unsigned char color)
{
    admiralType     *a;

    a = ( admiralType *)*gAresGlobal->gAdmiralData + whichAdmiral;
    a->color = color;
}

unsigned char GetAdmiralColor( long whichAdmiral)
{
    admiralType     *a;

    if ( whichAdmiral < 0) return( 0);
    a = ( admiralType *)*gAresGlobal->gAdmiralData + whichAdmiral;
    return( a->color);
}

long GetAdmiralRace( long whichAdmiral)
{
    admiralType     *a;

    if ( whichAdmiral < 0) return( -1);
    a = ( admiralType *)*gAresGlobal->gAdmiralData + whichAdmiral;
    return( a->race);
}

void SetAdmiralFlagship( long whichAdmiral, long whichShip)

{
    admiralType     *a;
    spaceObjectType *anObject;

    if ( whichAdmiral < 0)
    {
        MyDebugString("\pCan't set flagship of -1 admiral.");
    }

    a = ( admiralType *)*gAresGlobal->gAdmiralData + whichAdmiral;
    if ( whichShip >= 0)
    {
        a->flagship = whichShip;
        anObject = (spaceObjectType *)*gSpaceObjectData + whichShip;
        a->flagshipID = anObject->id;
    } else
    {
        a->flagship - 1;
        a->flagshipID = -1;
    }
}

spaceObjectType *GetAdmiralFlagship( long whichAdmiral)
{
    admiralType     *a;
    spaceObjectType *anObject;

    if ( whichAdmiral < 0) return( nil);
    a = ( admiralType *)*gAresGlobal->gAdmiralData + whichAdmiral;
    if ( a->flagship == kNoShip) return( nil);
    anObject = (spaceObjectType *)*gSpaceObjectData + a->flagship;
    if ( anObject->id == a->flagshipID)
        return( anObject);
    else return( nil);
}

void SetAdmiralEarningPower( long whichAdmiral, smallFixedType power)
{
    admiralType     *a;
    if ( whichAdmiral >= 0)
    {
        a = ( admiralType *)*gAresGlobal->gAdmiralData + whichAdmiral;
        a->earningPower = power;
    }
}

smallFixedType GetAdmiralEarningPower( long whichAdmiral)
{
    admiralType     *a;

    if ( whichAdmiral >= 0)
    {
        a = ( admiralType *)*gAresGlobal->gAdmiralData + whichAdmiral;
        return ( a->earningPower);
    } else return( 0);
}

void SetAdmiralDestinationObject( long whichAdmiral, long whichObject, destinationType dType)

{
    admiralType     *a;
    spaceObjectType *destObject;

    a = ( admiralType *)*gAresGlobal->gAdmiralData + whichAdmiral;
    a->destinationObject = whichObject;
    if ( whichObject >= 0)
    {
        destObject = (spaceObjectType *)*gSpaceObjectData + whichObject;
        a->destinationObjectID = destObject->id;
    } else a->destinationObjectID = -1;
    a->destType = dType;
}

long GetAdmiralDestinationObject( long whichAdmiral)

{
    admiralType     *a;
    spaceObjectType *destObject;

    a = ( admiralType *)*gAresGlobal->gAdmiralData + whichAdmiral;

    if ( a->destinationObject < 0)
        return ( a->destinationObject);

    destObject = (spaceObjectType *)*gSpaceObjectData + a->destinationObject;
    if (( destObject->id == a->destinationObjectID) && ( destObject->active == kObjectInUse)) return( a->destinationObject);
    else
    {
        a->destinationObject = -1;
        a->destinationObjectID = -1;
        return ( -1);
    }
}

void SetAdmiralConsiderObject( long whichAdmiral, long whichObject)

{
    admiralType     *a;
    spaceObjectType *anObject= (spaceObjectType *)*gSpaceObjectData +
                        whichObject;
    destBalanceType *d = mGetDestObjectBalancePtr( 0);
    long            buildAtNum, l;

    if ( whichAdmiral < 0) MyDebugString("\pCan't set consider ship for -1 admiral.");
    a = ( admiralType *)*gAresGlobal->gAdmiralData + whichAdmiral;
    a->considerShip = whichObject;
    if ( whichObject >= 0)
    {
        a->considerShipID = anObject->id;
        if ( anObject->attributes & kCanAcceptBuild)
        {
            buildAtNum = 0;
            while (( d->whichObject != whichObject) && ( buildAtNum < kMaxDestObject))
            {
                buildAtNum++;
                d++;
            }
            if ( buildAtNum < kMaxDestObject)
            {
                l = 0;
                while (( l < kMaxShipCanBuild) &&
                    ( d->canBuildType[l] == kNoShip)) l++;
                if ( l < kMaxShipCanBuild)
                    a->buildAtObject = buildAtNum;
            }
        }
    } else a->considerShipID = -1;
}

Boolean BaseHasSomethingToBuild( long whichObject)
{
    long            buildAtNum, l;
    destBalanceType *d = mGetDestObjectBalancePtr( 0);
    spaceObjectType *anObject= (spaceObjectType *)*gSpaceObjectData + whichObject;

    if ( anObject->attributes & kCanAcceptBuild)
    {
        buildAtNum = 0;
        while (( d->whichObject != whichObject) && ( buildAtNum < kMaxDestObject))
        {
            buildAtNum++;
            d++;
        }
        if ( buildAtNum < kMaxDestObject)
        {
            l = 0;
            while (( l < kMaxShipCanBuild) &&
                ( d->canBuildType[l] == kNoShip)) l++;
            if ( l < kMaxShipCanBuild)
                return true;
            else return false;
        } else return false;
    } else return false;
    return false;
}

long GetAdmiralConsiderObject( long whichAdmiral)

{
    admiralType     *a;
    spaceObjectType *anObject;
    a = ( admiralType *)*gAresGlobal->gAdmiralData + whichAdmiral;

    if ( whichAdmiral < 0) return( -1);
    if ( a->considerShip >= 0)
    {
        anObject = (spaceObjectType *)*gSpaceObjectData + a->considerShip;
        if (( anObject->id == a->considerShipID) &&
            ( anObject->active == kObjectInUse) &&
            ( anObject->owner == whichAdmiral))
        {
            return( a->considerShip);
        } else
        {
            a->considerShip = -1;
            a->considerShipID = -1;
            return( -1);
        }
    } else
    {
        if ( a->considerShip != -1) MyDebugString("\pStrange Admiral Consider Ship");
        return ( a->considerShip);
    }
}

long GetAdmiralBuildAtObject( long whichAdmiral)

{
    admiralType     *a;
    destBalanceType *destBalance;
    spaceObjectType *anObject;

    a = ( admiralType *)*gAresGlobal->gAdmiralData + whichAdmiral;
    if ( a->buildAtObject >= 0)
    {
        destBalance = mGetDestObjectBalancePtr( a->buildAtObject);
        if ( destBalance->whichObject >= 0)
        {
            anObject = ( spaceObjectType *)*gSpaceObjectData +
                destBalance->whichObject;
            if ( anObject->owner != whichAdmiral)
                a->buildAtObject = kNoShip;
        } else a->buildAtObject = kNoShip;
    }
    return ( a->buildAtObject);
}

void SetAdmiralBuildAtObject( long whichAdmiral, long whichObject)
{
    admiralType     *a;
    spaceObjectType *anObject= (spaceObjectType *)*gSpaceObjectData + whichObject;
    destBalanceType *d = mGetDestObjectBalancePtr( 0);
    long            buildAtNum, l;

    if ( whichAdmiral < 0) MyDebugString("\pCan't set consider ship for -1 admiral.");
    a = ( admiralType *)*gAresGlobal->gAdmiralData + whichAdmiral;
    if ( whichObject >= 0)
    {
        if ( anObject->attributes & kCanAcceptBuild)
        {
            buildAtNum = 0;
            while (( d->whichObject != whichObject) && ( buildAtNum < kMaxDestObject))
            {
                buildAtNum++;
                d++;
            }
            if ( buildAtNum < kMaxDestObject)
            {
                l = 0;
                while (( l < kMaxShipCanBuild) &&
                    ( d->canBuildType[l] == kNoShip)) l++;
                if ( l < kMaxShipCanBuild)
                    a->buildAtObject = buildAtNum;
            }
        }
    }
}

anyCharType *GetAdmiralBuildAtName( long whichAdmiral)

{
    admiralType     *a;
    destBalanceType *destObject;

    a = ( admiralType *)*gAresGlobal->gAdmiralData + whichAdmiral;
    destObject = mGetDestObjectBalancePtr( a->buildAtObject);
    return ( destObject->name);
}

void SetAdmiralBuildAtName( long whichAdmiral, StringPtr name)
{
    admiralType     *a;
    destBalanceType *destObject;

    a = ( admiralType *)*gAresGlobal->gAdmiralData + whichAdmiral;
    destObject = mGetDestObjectBalancePtr( a->buildAtObject);
    if ( name[0] > kDestinationNameLen) name[0] = kDestinationNameLen;
    CopyPString( destObject->name, name);
}

anyCharType *GetDestBalanceName( long whichDestObject)

{
    destBalanceType *destObject;

    destObject = mGetDestObjectBalancePtr( whichDestObject);
    return ( destObject->name);
}

anyCharType *GetAdmiralName( long whichAdmiral)

{
    admiralType     *a;

    if (( whichAdmiral >= 0) && ( whichAdmiral < kScenarioPlayerNum))
    {
        a = ( admiralType *)*gAresGlobal->gAdmiralData + whichAdmiral;
        return ( a->name);
    } else return ( nil);
}

void SetAdmiralName( long whichAdmiral, anyCharType *name)

{
    admiralType     *a;

    if (( whichAdmiral >= 0) && ( whichAdmiral < kScenarioPlayerNum))
    {
        a = ( admiralType *)*gAresGlobal->gAdmiralData + whichAdmiral;
        if ( *name > kAdmiralNameLen) *name = kAdmiralNameLen;
        CopyPString( (unsigned char *)a->name, (unsigned char *)name);

    }
}

/*
void SetObjectDestination( spaceObjectType *o)

{
    admiralType     *a;
    spaceObjectType *dObject;
    destBalanceType *destBalance;
    longPointType   difference;
    long            count;
    unsigned long   distance, time;
    float           f;
    smallFixedType  strategicValue;

    if ( o->owner == kNoOwner)
    {
        o->destinationObject = kNoDestinationObject;
        o->destinationLocation.h = o->destinationLocation.v = kNoDestinationCoord;
        o->timeFromOrigin = 0;
        o->idealLocationCalc.h = o->idealLocationCalc.v = 0;
        o->originLocation = o->location;
        return;
    }

    if (( o->owner < 0) || ( o->owner >= kScenarioPlayerNum)) { WriteDebugLong( o->owner); return;}
    a = ( admiralType *)*gAresGlobal->gAdmiralData + o->owner;

    if (( !a->active) || ( a->destType == kNoDestinationType) ||
        ( a->destinationObject == kNoDestinationObject))
    {
        o->destinationObject = kNoDestinationObject;
        o->destinationLocation.h = o->destinationLocation.v = kNoDestinationCoord;
        o->timeFromOrigin = 0;
        o->idealLocationCalc.h = o->idealLocationCalc.v = 0;
        o->originLocation = o->location;
    } else
    {

        // if this is a destination object, we need to calc balance for all players

        // first get rid of original balance
        if ( o->destinationObject != kNoDestinationObject)
        {
            dObject = (spaceObjectType *)*gSpaceObjectData + o->destinationObject;
            // if the object's owner is the same as the dest's owner then use defensive val
            if ( dObject->owner == o->owner) strategicValue = o->baseType->defenseValue;
            else strategicValue = o->baseType->offenseValue;
            destBalance = mGetDestObjectBalancePtr( dObject->destinationObject);
            for ( count = 0; count < kScenarioPlayerNum; count++)
            {
                if ( count == o->owner)
                {
                    destBalance->balance[count] -= strategicValue;
                } else destBalance->balance[count] += strategicValue;
            }
        }

        if ( a->destinationObject != kNoDestinationObject)
        {
            dObject = (spaceObjectType *)*gSpaceObjectData + a->destinationObject;
            // if the object's owner is the same as the dest's owner then use defensive val
            if ( dObject->owner == o->owner) strategicValue = o->baseType->defenseValue;
            else strategicValue = o->baseType->offenseValue;
            destBalance = mGetDestObjectBalancePtr( dObject->destinationObject);
            for ( count = 0; count < kScenarioPlayerNum; count++)
            {
                if ( count == o->owner)
                {
                    destBalance->balance[count] += strategicValue;
                } else destBalance->balance[count] -= strategicValue;
            }
        }

        dObject = (spaceObjectType *)*gSpaceObjectData + a->destinationObject;

        if ( a->destType == kCoordinateDestinationType)
        {
            o->destinationLocation = dObject->location;
            o->destinationObject = kNoDestinationObject;
        } else
        {
            o->destinationLocation = dObject->location;
            o->destinationObject = a->destinationObject;
        }
        o->originLocation = o->location;
        difference.h = o->destinationLocation.h - o->originLocation.h;
        difference.v = o->destinationLocation.v - o->originLocation.v;
        distance = difference.h * difference.h + difference.v * difference.v;
        distance = lsqrt( distance);
        time = distance / kConvoySpeed;
        f = (float)difference.h / (float)time;
        o->idealLocationCalc.h = mFloatToFixed( f);
        f = (float)difference.v / (float)time;
        o->idealLocationCalc.v = mFloatToFixed( f);
        o->timeFromOrigin = 0;
    }
}
*/

void SetObjectLocationDestination( spaceObjectType *o, coordPointType *where)
{
    admiralType     *a;

    // if the object does not have an alliance, then something is wrong here--forget it
    if ( o->owner <= kNoOwner)
    {
        o->destinationObject = kNoDestinationObject;
        o->destObjectDest = kNoDestinationObject;
        o->destObjectID = -1;
        o->destObjectPtr = nil;
        o->destinationLocation.h = o->destinationLocation.v = kNoDestinationCoord;
        o->timeFromOrigin = 0;
        o->idealLocationCalc.h = o->idealLocationCalc.v = 0;
        o->originLocation = o->location;
        return;
    }

    // if this object can't accept a destination, then forget it
    if ( !(o->attributes & kCanAcceptDestination)) return;

    // if this object has a locked destination, then forget it
    if ( o->attributes & kStaticDestination) return;

    // if the owner is not legal, something is very very wrong
    if (( o->owner < 0) || ( o->owner >= kScenarioPlayerNum)) { WriteDebugLong( o->owner); return;}

    // get the admiral
    a = ( admiralType *)*gAresGlobal->gAdmiralData + o->owner;

    // if the admiral is not legal, or the admiral has no destination, then forget about it
    if ( !a->active)
    {
        o->destinationObject = kNoDestinationObject;
        o->destObjectDest = kNoDestinationObject;
        o->destObjectPtr = nil;
        o->destinationLocation.h = o->destinationLocation.v = kNoDestinationCoord;
        o->timeFromOrigin = 0;
        o->idealLocationCalc.h = o->idealLocationCalc.v = 0;
        o->originLocation = o->location;
    } else
    // the object is OK, the admiral is OK, then go about setting its destination
    {
        if ( o->attributes & kCanAcceptDestination)
        {
            o->timeFromOrigin = kTimeToCheckHome;
        } else
        {
            o->timeFromOrigin = 0;
        }

        // remove this object from its destination
        if ( o->destinationObject != kNoDestinationObject)
            RemoveObjectFromDestination( o);

        o->destinationLocation = o->originLocation = *where;
        o->destinationObject = kNoDestinationObject;
        o->destObjectPtr = nil;
        o->timeFromOrigin = 0;
        o->idealLocationCalc.h = o->idealLocationCalc.v = 0;
    }
}

void SetObjectDestination( spaceObjectType *o, spaceObjectType *overrideObject)

{
    admiralType     *a;
    spaceObjectType *dObject = overrideObject;

/*  DebugFileAppendString( "\pDT\t");
    DebugFileAppendLong( o->entryNumber);
    DebugFileAppendString( "\p\t");
    if ( overrideObject != nil)
        DebugFileAppendLong( overrideObject->entryNumber);
    else
        DebugFileAppendString("\p-");
    DebugFileAppendString( "\p\t");
*/
    /* MORE DEBUGFILE BELOW! */

    // if the object does not have an alliance, then something is wrong here--forget it
    if ( o->owner <= kNoOwner)
    {
        o->destinationObject = kNoDestinationObject;
        o->destObjectDest = kNoDestinationObject;
        o->destObjectID = -1;
        o->destObjectPtr = nil;
        o->destinationLocation.h = o->destinationLocation.v = kNoDestinationCoord;
        o->timeFromOrigin = 0;
        o->idealLocationCalc.h = o->idealLocationCalc.v = 0;
        o->originLocation = o->location;
//      Debugger();
        return;
    }

    // if this object can't accept a destination, then forget it
    if ( !(o->attributes & kCanAcceptDestination))
    {
//      Debugger();
        return;
    }

    // if this object has a locked destination, then forget it
    if (( o->attributes & kStaticDestination) && ( overrideObject == nil))
    {
//      Debugger();
        return;
    }

    // HACK for now: if the object doesn't have a "normal" presence, then forget it
//  if ( o->presenceState != kNormalPresence) return;

    // if the owner is not legal, something is very very wrong
    if (( o->owner < 0) || ( o->owner >= kScenarioPlayerNum))
    {
        WriteDebugLong( o->owner);
//      Debugger();
        return;
    }

    // get the admiral
    a = ( admiralType *)*gAresGlobal->gAdmiralData + o->owner;

    // if the admiral is not legal, or the admiral has no destination, then forget about it
    if (( dObject == nil) && (( !a->active) ||
        ( a->destType == kNoDestinationType) ||
        ( a->destinationObject == kNoDestinationObject) ||
        ( a->destinationObjectID == o->id)))
    {
        o->destinationObject = kNoDestinationObject;
        o->destObjectDest = kNoDestinationObject;
        o->destObjectPtr = nil;
        o->destinationLocation.h = o->destinationLocation.v = kNoDestinationCoord;
        o->timeFromOrigin = 0;
        o->idealLocationCalc.h = o->idealLocationCalc.v = 0;
        o->originLocation = o->location;
//      DebugFileAppendString("\p-\r");
//      Debugger();
    } else
    // the object is OK, the admiral is OK, then go about setting its destination
    {

        // first make sure we're still looking at the same object
        if ( dObject == nil)
            dObject = (spaceObjectType *)*gSpaceObjectData + a->destinationObject;

        if ((dObject->active == kObjectInUse) &&
            (( dObject->id == a->destinationObjectID) ||
            ( overrideObject != nil)))
        {
//          DebugFileAppendLong( a->destinationObject);
//          DebugFileAppendString("\p\r");

            if ( o->attributes & kCanAcceptDestination)
            {
                o->timeFromOrigin = kTimeToCheckHome;
            } else
            {
                o->timeFromOrigin = 0;
            }
        /*
        // this while loop depends on the 2nd two comparisons not being exucted if the 1st is false
        while (( dObject != nil) && ( !(dObject->attributes & kIsDestination)) &&
            ( dObject != o))
        {
            if ( dObject->destinationObject != kNoDestinationObject)
                dObject = (spaceObjectType *)dObject->destObjectPtr;
            else dObject = nil;
        }

        // if the reference is not circular, then continue
        if ( dObject != o)
        */
            // remove this object from its destination
            if ( o->destinationObject != kNoDestinationObject)
                RemoveObjectFromDestination( o);

            // add this object to its destination
            if ( o != dObject)
            {
//              WriteDebugLine((char *)"\pNot Me!");
                o->runTimeFlags &= ~kHasArrived;
                o->destinationObject = dObject->entryNumber; //a->destinationObject;
                o->destObjectPtr = (spaceObjectTypePtr)dObject;
                o->destObjectDest = dObject->destinationObject;
                o->destObjectDestID = dObject->destObjectID;
                o->destObjectID = dObject->id;

                if ( dObject->owner == o->owner)
                {
                    dObject->remoteFriendStrength += o->baseType->offenseValue;
                    dObject->escortStrength += o->baseType->offenseValue;
                    if ( dObject->attributes & kIsDestination)
                    {
                        if ( dObject->escortStrength < dObject->baseType->friendDefecit)
                            o->duty = eGuardDuty;
                        else
                            o->duty = eNoDuty;
                    } else
                    {
                        if ( dObject->escortStrength < dObject->baseType->friendDefecit)
                            o->duty = eEscortDuty;
                        else
                            o->duty = eNoDuty;
                    }
                } else
                {
                    dObject->remoteFoeStrength += o->baseType->offenseValue;
                    if ( dObject->attributes & kIsDestination)
                    {
                        o->duty = eAssaultDuty;
                    } else
                        o->duty = eAssaultDuty;
                }
            } else
            {
                o->destinationObject = kNoDestinationObject;
                o->destObjectDest = kNoDestinationObject;
                o->destObjectPtr = nil;
                o->destinationLocation.h = o->destinationLocation.v = kNoDestinationCoord;
                o->timeFromOrigin = 0;
                o->idealLocationCalc.h = o->idealLocationCalc.v = 0;
                o->originLocation = o->location;
//              DebugFileAppendString("\p-\r");
            }
            /*
            dObject  = o;
            while ( dObject != nil)
            {
                if (( dObject->destinationObject != kNoDestinationObject) && ( !(dObject->attributes & kIsDestination)))
                {
                    dObject = (spaceObjectType *)dObject->destObjectPtr;

                    for ( count = 0; count < kScenarioPlayerNum; count++)
                    {
                        dObject->balance[count] += o->balance[count];
                    }
                } else dObject = nil;
            }
            */
        } else
        {
            o->destinationObject = kNoDestinationObject;
            o->destObjectDest = kNoDestinationObject;
            o->destObjectPtr = nil;
            o->destinationLocation.h = o->destinationLocation.v = kNoDestinationCoord;
            o->timeFromOrigin = 0;
            o->idealLocationCalc.h = o->idealLocationCalc.v = 0;
            o->originLocation = o->location;
//          DebugFileAppendString("\p-\r");
        }

    }
}

void RemoveObjectFromDestination( spaceObjectType *o)

{
    spaceObjectType *dObject;

    dObject  = o;
    /*
    while ( dObject != nil)
    {
        if (( dObject->destinationObject != kNoDestinationObject) && ( !(dObject->attributes & kIsDestination)))
        {
            dObject = (spaceObjectType *)dObject->destObjectPtr;

            for ( count = 0; count < kScenarioPlayerNum; count++)
            {
                dObject->balance[count] -= o->balance[count];
            }
        } else dObject = nil;
    }
    */
    if (( o->destinationObject != kNoDestinationObject) && ( o->destObjectPtr != nil))
    {
        dObject = (spaceObjectType *)o->destObjectPtr;
        if ( dObject->id == o->destObjectID)
        {
            if ( dObject->owner == o->owner)
            {
                dObject->remoteFriendStrength -= o->baseType->offenseValue;
                dObject->escortStrength -= o->baseType->offenseValue;
            } else
            {
                dObject->remoteFoeStrength -= o->baseType->offenseValue;
            }
        }
    }

    o->destinationObject = kNoDestinationObject;
    o->destObjectDest = kNoDestinationObject;
    o->destObjectID = -1;
    o->destObjectPtr = nil;

}

void AdmiralThink( void)

{
    short           i, j, k;
    admiralType     *a =( admiralType *)*gAresGlobal->gAdmiralData;
    spaceObjectType *anObject, *destObject, *otherDestObject, *stepObject;
    destBalanceType *destBalance;
    long            origObject, origDest, baseNum,
                    difference;
    smallFixedType  friendValue, foeValue, thisValue;
    baseObjectType  *baseObject;
    longPointType   gridLoc;

    destBalance = mGetDestObjectBalancePtr( 0);
    for ( i = 0; i < kMaxDestObject; i++)
    {
        destBalance->buildTime -= 10;
        if ( destBalance->buildTime <= 0)
        {
            destBalance->buildTime = 0;
            if ( destBalance->buildObjectBaseNum != kNoShip)
            {
                anObject = (spaceObjectType *)*gSpaceObjectData + destBalance->whichObject;
                AdmiralBuildAtObject( anObject->owner, destBalance->buildObjectBaseNum, i);
                destBalance->buildObjectBaseNum = kNoShip;
            }
        }

        anObject = ( spaceObjectType *)*gSpaceObjectData + destBalance->whichObject;
        if ( anObject->owner >= 0)
        {
            PayAdmiral( anObject->owner, destBalance->earn);
        }
        destBalance++;
    }

    for ( i = 0; i < kScenarioPlayerNum; i++)
    {
        if (( a->attributes & kAIsComputer) && ( !(a->attributes & kAIsRemote)))
        {
            if ( a->blitzkrieg > 0)
            {
                a->blitzkrieg--;
                if ( a->blitzkrieg <= 0)
                {
                    a->blitzkrieg = 0 - (RandomSeeded( 1200, &gRandomSeed, 'adm1', -1) + 1200);//really 48
                    anObject = (spaceObjectType *)*gSpaceObjectData;
                    for ( j = 0; j < kMaxSpaceObject; j++)
                    {
                        if ( anObject->owner == i)
                        {
                            anObject->currentTargetValue = 0x00000000;
                        }
                        anObject++;
                    }
//                  WriteDebugLine((char *)"\pEndBlitz!");
//                  WriteDebugLong( i);
                }
            } else
            {
                a->blitzkrieg++;
                if ( a->blitzkrieg >= 0)
                {
                    a->blitzkrieg = RandomSeeded( 1200, &gRandomSeed, 'adm2', -1) + 1200;//really 48
                    anObject = (spaceObjectType *)*gSpaceObjectData;
                    for ( j = 0; j < kMaxSpaceObject; j++)
                    {
                        if ( anObject->owner == i)
                        {
                            anObject->currentTargetValue = 0x00000000;
                        }
                        anObject++;
                    }
//                  WriteDebugLine((char *)"\p>BLITZKRIEG<");
//                  WriteDebugLong( i);
                }
            }

            // get the current object
            if ( a->considerShip < 0)
            {
                a->considerShip = gRootObjectNumber;
                anObject = (spaceObjectType *)*gSpaceObjectData + a->considerShip;
                a->considerShipID = anObject->id;
            } else
            {
                anObject = (spaceObjectType *)*gSpaceObjectData + a->considerShip;
            }

            if ( a->destinationObject < 0) a->destinationObject = gRootObjectNumber;

            if ( anObject->active != kObjectInUse)
            {
                a->considerShip = gRootObjectNumber;
                anObject = (spaceObjectType *)*gSpaceObjectData + a->considerShip;
                a->considerShipID = anObject->id;
            }

            if ( a->destinationObject >= 0)
            {
                destObject = (spaceObjectType *)*gSpaceObjectData + a->destinationObject;
                if ( destObject->active != kObjectInUse)
                {
                    destObject = gRootObject;
                    a->destinationObject = gRootObjectNumber;
                }
                origDest = a->destinationObject;
                do
                {
                    a->destinationObject = destObject->nextObjectNumber;

                    // if we've gone through all of the objects
                    if ( a->destinationObject < 0)
                    {
                        // ********************************
                        // SHIP MUST DECIDE, THEN INCREASE CONSIDER SHIP
                        // ********************************
                        if (( anObject->duty != eEscortDuty) && (anObject->duty != eHostileBaseDuty) &&
                            (anObject->bestConsideredTargetValue > anObject->currentTargetValue))
                        {
//                          WriteDebugLine((char *)"\pDEST");
//                          WriteDebugLong( a->considerShip);
                            a->destinationObject = anObject->bestConsideredTargetNumber;
                            a->destType = kObjectDestinationType;
                            if ( a->destinationObject >= 0)
                            {
                                destObject = (spaceObjectType *)*gSpaceObjectData + a->destinationObject;
                                if ( destObject->active == kObjectInUse)
                                {
                                    a->destinationObjectID = destObject->id;
                                    anObject->currentTargetValue = anObject->bestConsideredTargetValue;// + (anObject->baseType->offenseValue << (smallFixedType)4);
                                    thisValue = RandomSeeded( kFixedPlusPointFive, &(anObject->randomSeed), 'adm3', anObject->whichBaseObject) - kFixedOneQuarter;
                                    thisValue = mMultiplyFixed( thisValue, anObject->currentTargetValue);
                                    anObject->currentTargetValue += thisValue;
                                    SetObjectDestination( anObject, nil);
                                }
                            }
                            a->destType = kNoDestinationType;
                        }

                        if (( anObject->duty != eEscortDuty) && (anObject->duty != eHostileBaseDuty))
                        {
                            a->thisFreeEscortStrength += anObject->baseType->offenseValue;
                        }

                        anObject->bestConsideredTargetValue = 0xffffffff;
                        // start back with 1st ship
                        a->destinationObject = gRootObjectNumber;
                        destObject = gRootObject;

                        // >>> INCREASE CONSIDER SHIP
                        origObject = a->considerShip;
                        anObject = (spaceObjectType *)*gSpaceObjectData + a->considerShip;
                        if ( anObject->active != kObjectInUse)
                        {
                            anObject = gRootObject;
                            a->considerShip = gRootObjectNumber;
                            a->considerShipID = anObject->id;
                        }
                        do
                        {
                            a->considerShip = anObject->nextObjectNumber;
                            if ( a->considerShip < 0)
                            {
                                a->considerShip = gRootObjectNumber;
                                anObject = gRootObject;
                                a->considerShipID = anObject->id;
                                a->lastFreeEscortStrength = a->thisFreeEscortStrength;
                                a->thisFreeEscortStrength = 0;
                            } else
                            {
                                anObject = (spaceObjectType *)anObject->nextObject;
                                a->considerShipID = anObject->id;
                            }
                        } while ((( anObject->owner != i) || ( !(anObject->attributes & kCanAcceptDestination)) ||
                            ( anObject->active != kObjectInUse)) &&
                            ( a->considerShip != origObject));
                    } else
                    {
                        destObject = (spaceObjectType *)destObject->nextObject;
                    }
                    a->destinationObjectID = destObject->id;
                } while ((( !(destObject->attributes & (kCanBeDestination))) || ( a->destinationObject == a->considerShip) ||
                ( destObject->active != kObjectInUse) || (!( destObject->attributes & kCanBeDestination))) &&
                ( a->destinationObject != origDest));

            // if our object is legal and our destination is legal
            if (( anObject->owner == i) && ( anObject->attributes & kCanAcceptDestination) &&
                ( anObject->active == kObjectInUse) &&
                ( destObject->attributes & ( kCanBeDestination)) &&
                ( destObject->active == kObjectInUse) && (( anObject->owner != destObject->owner) ||
                ( anObject->baseType->destinationClass < destObject->baseType->destinationClass))/* &&
                ( ( !(anObject->attributes & kDestinationOnly)) || ( destObject->attributes & kIsDestination))*/)
                /*( destBalance->whichObject != kDestNoObject))*/
            {
                gridLoc = destObject->distanceGrid;
                stepObject = otherDestObject = destObject;
                while ( stepObject->nextFarObject != nil)
                {
                    if (( stepObject->distanceGrid.h == gridLoc.h) && ( stepObject->distanceGrid.v == gridLoc.v))
                        otherDestObject = stepObject;
                    stepObject = (spaceObjectType *)stepObject->nextFarObject;
                }
                if ( otherDestObject->owner == anObject->owner)
                {
                    friendValue = otherDestObject->localFriendStrength;
                    foeValue = otherDestObject->localFoeStrength;
                } else
                {
                    foeValue = otherDestObject->localFriendStrength;
                    friendValue = otherDestObject->localFoeStrength;
                }


                thisValue = kUnimportantTarget;
                if ( destObject->owner == anObject->owner)
                {
                    if ( destObject->attributes & kIsDestination)
                    {
                        if ( destObject->escortStrength < destObject->baseType->friendDefecit)
                        {
                            thisValue = kAbsolutelyEssential;
                        } else if ( foeValue)
                        {
                            if ( foeValue >= friendValue)
                                thisValue = kMostImportantTarget;
                            else if ( foeValue > (friendValue >> (smallFixedType)1))
                                thisValue = kVeryImportantTarget;
                            else thisValue = kUnimportantTarget;
                        } else
                        {
                            if (( a->blitzkrieg > 0) && ( anObject->duty == eGuardDuty))
                            {
                                thisValue = kUnimportantTarget;
                            } else
                            {
                                if ( foeValue > 0)
                                {
                                    thisValue = kSomewhatImportantTarget;
                                } else
                                {
                                    thisValue = kUnimportantTarget;
                                }
                            }
                        }
                        if ( anObject->baseType->orderFlags & kTargetIsBase)
                            thisValue <<= (smallFixedType)3;
                        if ( anObject->baseType->orderFlags & kHardTargetIsNotBase)
                            thisValue = 0;
                    } else
                    {
                        if ( destObject->baseType->destinationClass > anObject->baseType->destinationClass)
                        {
                            if ( foeValue > friendValue)
                            {
                                thisValue = kMostImportantTarget;
                            } else
                            {
                                if ( destObject->escortStrength < destObject->baseType->friendDefecit)
                                {
                                    thisValue = kMostImportantTarget;
                                } else
                                {
                                    thisValue = kUnimportantTarget;
                                }
                            }
                        } else
                        {
                            thisValue = kUnimportantTarget;
                        }
                        if ( anObject->baseType->orderFlags & kTargetIsNotBase)
                            thisValue <<= (smallFixedType)3;
                        if ( anObject->baseType->orderFlags & kHardTargetIsBase)
                            thisValue = 0;
                    }
                    if ( anObject->baseType->orderFlags & kTargetIsFriend)
                        thisValue <<= (smallFixedType)3;
                    if ( anObject->baseType->orderFlags & kHardTargetIsFoe)
                        thisValue = 0;
                } else if ( destObject->owner >= 0)
                {
                    if (( anObject->duty == eGuardDuty) || ( anObject->duty == eNoDuty))
                    {
                        if ( destObject->attributes & kIsDestination)
                        {
                            if ( foeValue < friendValue)
                            {
                                thisValue = kMostImportantTarget;
                            } else
                            {
                                thisValue = kSomewhatImportantTarget;
                            }
                            if ( a->blitzkrieg > 0)
                            {
                                thisValue <<= (smallFixedType)2;
                            }
                            if ( anObject->baseType->orderFlags & kTargetIsBase)
                                thisValue <<= (smallFixedType)3;

                            if ( anObject->baseType->orderFlags & kHardTargetIsNotBase)
                                thisValue = 0;
                        } else
                        {
                            if ( friendValue)
                            {
                                if ( friendValue < foeValue)
                                {
                                    thisValue = kSomewhatImportantTarget;
                                } else
                                {
                                    thisValue = kUnimportantTarget;
                                }
                            } else
                            {
                                thisValue = kLeastImportantTarget;
                            }
                            if ( anObject->baseType->orderFlags & kTargetIsNotBase)
                                thisValue <<= (smallFixedType)1;

                            if ( anObject->baseType->orderFlags & kHardTargetIsBase)
                                thisValue = 0;
                        }
                    }
                    if ( anObject->baseType->orderFlags & kTargetIsFoe)
                        thisValue <<= (smallFixedType)3;
                    if ( anObject->baseType->orderFlags & kHardTargetIsFriend)
                        thisValue = 0;
                } else
                {
                    if ( destObject->attributes & kIsDestination)
                    {
                        thisValue = kVeryImportantTarget;
                        if ( a->blitzkrieg > 0)
                        {
                            thisValue <<= (smallFixedType)2;
                        }
                        if ( anObject->baseType->orderFlags & kTargetIsBase)
                            thisValue <<= (smallFixedType)3;
                        if ( anObject->baseType->orderFlags & kHardTargetIsNotBase)
                            thisValue = 0;
                    } else
                    {
                        if ( anObject->baseType->orderFlags & kTargetIsNotBase)
                            thisValue <<= (smallFixedType)3;
                        if ( anObject->baseType->orderFlags & kHardTargetIsBase)
                            thisValue = 0;
                    }
                    if ( anObject->baseType->orderFlags & kTargetIsFoe)
                        thisValue <<= (smallFixedType)3;
                    if ( anObject->baseType->orderFlags & kHardTargetIsFriend)
                        thisValue = 0;
                }

                difference = ABS( (long)destObject->location.h - (long)anObject->location.h);
                gridLoc.h = difference;
                difference =  ABS( (long)destObject->location.v - (long)anObject->location.v);
                gridLoc.v = difference;

                if (( gridLoc.h < kMaximumRelevantDistance) && ( gridLoc.v < kMaximumRelevantDistance))
                {
                    if ( anObject->baseType->orderFlags & kTargetIsLocal)
                        thisValue <<= (smallFixedType)3;
                    if ( anObject->baseType->orderFlags & kHardTargetIsRemote)
                        thisValue = 0;
                } else
                {
                    if ( anObject->baseType->orderFlags & kTargetIsRemote)
                        thisValue <<= (smallFixedType)3;
                    if ( anObject->baseType->orderFlags & kHardTargetIsLocal)
                        thisValue = 0;
                }


                if (( (anObject->baseType->orderFlags & kLevelKeyTagMask) != 0) &&
                    ( (anObject->baseType->orderFlags & kLevelKeyTagMask) ==
                        (destObject->baseType->buildFlags & kLevelKeyTagMask)))
                {
                    thisValue <<= (smallFixedType)3;
//                  WriteDebugLine( (char *)"\pHARDMATCH");
//                  WriteDebugFixed( thisValue);
                } else if ( anObject->baseType->orderFlags & kHardMatchingFoe)
                {
                    thisValue = 0;
                }

                if ( thisValue > 0)
                    thisValue += RandomSeeded( thisValue >> (long)1, &(anObject->randomSeed), 'adm4', anObject->whichBaseObject) - (thisValue >> (long)2);
                if ( thisValue > anObject->bestConsideredTargetValue)
                {
                    anObject->bestConsideredTargetValue = thisValue;
                    anObject->bestConsideredTargetNumber = a->destinationObject;
                }
//              if ( anObject->baseType->orderFlags & kTargetIsFoe)
//                  thisValue <<= (smallFixedType)3;
            }
        }

        // if we've saved enough for our dreams
        if ( a->cash > a->saveGoal)
        {
                a->saveGoal = 0;

                // consider what ship to build
                if ( a->buildAtObject < 0) a->buildAtObject = 0;
                origDest = a->buildAtObject;
                destBalance = mGetDestObjectBalancePtr( a->buildAtObject);

                // try to find the next destination object that we own & that can build
                do
                {
                    a->buildAtObject++;
                    destBalance++;
                    if ( a->buildAtObject >= kMaxDestObject)
                    {
                        a->buildAtObject = 0;
                        destBalance = mGetDestObjectBalancePtr( 0);
                    }
                    if ( destBalance->whichObject >= 0)
                    {
                        anObject = (spaceObjectType *)*gSpaceObjectData + destBalance->whichObject;
                        if (( anObject->owner != i) || ( !(anObject->attributes & kCanAcceptBuild)))
                            anObject = nil;
                    } else anObject = nil;
                } while (( anObject == nil) && ( a->buildAtObject != origDest));

                // if we have a legal object
                if ( anObject != nil)
                {
                    if ( destBalance->buildTime <= 0)
                    {
                        if ( a->hopeToBuild < 0)
                        {
                            k = 0;
                            while (( a->hopeToBuild < 0) && ( k < 7))
                            {
                                k++;
                                // choose something to build
                                thisValue = RandomSeeded( a->totalBuildChance, &gRandomSeed, 'adm5', -1);
                                friendValue = 0xffffffff; // equals the highest qualifying object
                                for ( j = 0; j < kMaxNumAdmiralCanBuild; j++)
                                {
                                    if (( a->canBuildType[j].chanceRange <= thisValue) && ( a->canBuildType[j].chanceRange > friendValue))
                                    {
                                        friendValue = a->canBuildType[j].chanceRange;
                                        a->hopeToBuild = a->canBuildType[j].baseNum;
                                    }
                                }
                                if ( a->hopeToBuild >= 0)
                                {
                                    mGetBaseObjectFromClassRace( baseObject, baseNum, a->hopeToBuild, a->race)
    /*
                                    if (    (baseObject->buildFlags & kSufficientEscortsExist) &&
                                            ( baseObject->friendDefecit > a->lastFreeEscortStrength) &&
                                            ( k < 7))
                                    {
                                        WriteDebugLine((char *)"\pInsufficient");
                                        WriteDebugSmallFixed( a->lastFreeEscortStrength);
                                        a->hopeToBuild = -1;
                                    }
    */
                                    if ( baseObject->buildFlags & kSufficientEscortsExist)
                                    {
                                        anObject = (spaceObjectType *)*gSpaceObjectData;
                                        j = 0;
                                        while ( j < kMaxSpaceObject)
                                        {
                                            if (    ( anObject->active) &&
                                                    ( anObject->owner == i) &&
                                                    ( anObject->whichBaseObject == baseNum) &&
                                                    ( anObject->escortStrength < baseObject->friendDefecit)
                                                )
                                            {
//                                              WriteDebugLine((char *)"\pInsufficient!");
//                                              WriteDebugLong( a->hopeToBuild);
//                                              WriteDebugLong( anObject->owner);
//                                              WriteDebugLong( i);
                                                a->hopeToBuild = -1;
                                                j = kMaxSpaceObject;
                                            }
                                            j++;
                                            anObject++;
                                        }
                                    }

                                    if ( baseObject->buildFlags & kMatchingFoeExists)
                                    {
                                        thisValue = 0;
                                        anObject = (spaceObjectType *)*gSpaceObjectData;
                                        for ( j = 0; j < kMaxSpaceObject; j++)
                                        {
                                            if (    ( anObject->active) &&
                                                    ( anObject->owner != i) &&
                                                    (
                                                        ( anObject->baseType->buildFlags & kLevelKeyTagMask) ==
                                                        ( baseObject->orderFlags & kLevelKeyTagMask)
                                                    )
                                                )
                                            {
                                                thisValue = 1;
//                                              WriteDebugLine((char *)"\pFoeExists!");
//                                              WriteDebugLong( a->hopeToBuild);
//                                              WriteDebugLong( anObject->owner);
//                                              WriteDebugLong( i);
                                            }
                                            anObject++;
                                        }
                                        if ( !thisValue)
                                        {
                                            a->hopeToBuild = -1;
                                        }
                                    }
                                }
                            }
                        }
                        j = 0;
                        while (( destBalance->canBuildType[j] != a->hopeToBuild) && ( j < kMaxTypeBaseCanBuild)) j++;
                        if (( j < kMaxTypeBaseCanBuild) && ( a->hopeToBuild != kNoShip))
                        {
                            mGetBaseObjectFromClassRace( baseObject, baseNum, a->hopeToBuild, a->race)
                            if ( a->cash >= mLongToFixed(baseObject->price))
                            {
//                                              WriteDebugLine((char *)"\pBUILD:");
//                                              WriteDebugLong( a->hopeToBuild);
//                                              WriteDebugLong( i);
                                AdmiralScheduleBuild( i, j);
                                a->hopeToBuild = -1;
                                a->saveGoal = 0;
                            } else a->saveGoal = mLongToFixed(baseObject->price);
                        } // otherwise just wait until we get to it
                    }
                }
            }
        }
        a++;
    }
}

smallFixedType HackGetObjectStrength( spaceObjectType *anObject)

{
    spaceObjectType *tObject = anObject;
    longPointType   gridLoc;
    long            owner = anObject->owner;

    gridLoc = anObject->distanceGrid;
    while ( anObject->nextFarObject != nil)
    {
        if (( anObject->distanceGrid.h == gridLoc.h) && ( anObject->distanceGrid.v == gridLoc.v))
            tObject = anObject;
        anObject = (spaceObjectType *)anObject->nextFarObject;
    }

    if ( tObject->owner == owner)
    {
//      return( mLongToFixed( tObject->entryNumber));
//      return( tObject->friendStrength);
        return ( tObject->localFriendStrength - tObject->localFoeStrength + tObject->escortStrength);
    } else
    {
//      return( kFixedNegativePointFive);
        return (tObject->localFoeStrength - tObject->localFriendStrength - tObject->escortStrength);
    }
}

// assumes you can afford it & base has time
void AdmiralBuildAtObject( long whichAdmiral, long baseTypeNum, long whichDestObject)

{
    admiralType     *admiral = ( admiralType *)*gAresGlobal->gAdmiralData + whichAdmiral;
    destBalanceType *buildAtDest = mGetDestObjectBalancePtr( whichDestObject);
    baseObjectType  *buildBaseObject = nil;
    spaceObjectType *buildAtObject = nil;
    long            newObject;
    coordPointType  coord;
    fixedPointType  v = {0, 0};

    if (( baseTypeNum >= 0) && ( admiral->buildAtObject >= 0))
    {

            buildAtObject = (spaceObjectType *)*gSpaceObjectData + buildAtDest->whichObject;
            coord = buildAtObject->location;

//          newObject = CreateAnySpaceObject( baseTypeNum, &v, &coord, 0, whichAdmiral, 0, nil, -1, -1, -1);
            newObject = CreateAnySpaceObject( baseTypeNum, &v, &coord, 0, whichAdmiral, 0, -1);

            if ( newObject >= 0)
            {
                buildAtObject = (spaceObjectType *)*gSpaceObjectData + newObject;
                SetObjectDestination( buildAtObject, nil);
//              if ( admiral->attributes & kAIsHuman)
                if ( whichAdmiral == gAresGlobal->gPlayerAdmiralNumber)
                    PlayVolumeSound(  kComputerBeep2, kMediumVolume, kMediumPersistence, kLowPrioritySound);
            }

    }
}

Boolean AdmiralScheduleBuild( long whichAdmiral, long buildWhichType)

{
    admiralType     *admiral = ( admiralType *)*gAresGlobal->gAdmiralData + whichAdmiral;
    destBalanceType *buildAtDest = mGetDestObjectBalancePtr( admiral->buildAtObject);
    baseObjectType  *buildBaseObject = nil;
    long            baseNum;

    GetAdmiralBuildAtObject( whichAdmiral);
    if (( buildWhichType >= 0) && ( buildWhichType < kMaxTypeBaseCanBuild) &&
            ( admiral->buildAtObject >= 0) && ( buildAtDest->buildTime <= 0))
    {
        mGetBaseObjectFromClassRace( buildBaseObject, baseNum, buildAtDest->canBuildType[buildWhichType], admiral->race)
        if (( buildBaseObject != nil) && ( buildBaseObject->price <= mFixedToLong(admiral->cash)))
        {
            admiral->cash -= (mLongToFixed(buildBaseObject->price));
            if ( gAresGlobal->gActiveCheats[whichAdmiral] & kBuildFastBit)
            {
                buildAtDest->buildTime = 9;
                buildAtDest->totalBuildTime = 9;
            } else
            {
                buildAtDest->buildTime = buildBaseObject->buildTime;//buildBaseObject->price;
                buildAtDest->totalBuildTime = buildAtDest->buildTime;
            }
            buildAtDest->buildObjectBaseNum = baseNum;
            return ( TRUE);
        } else return ( FALSE);
    } else return ( FALSE);
}

void StopBuilding( long whichDestObject)
{
    destBalanceType *destObject;

    destObject = mGetDestObjectBalancePtr( whichDestObject);
    destObject->totalBuildTime = destObject->buildTime = 0;
    destObject->buildObjectBaseNum = kNoShip;
}

void PayAdmiral( long whichAdmiral, smallFixedType howMuch)

{
    admiralType     *admiral = ( admiralType *)*gAresGlobal->gAdmiralData + whichAdmiral;

    if (( whichAdmiral >= 0) && ( whichAdmiral < kScenarioPlayerNum))
    {
//      admiral->cash += howMuch * 100;
        admiral->cash += mMultiplyFixed( howMuch, admiral->earningPower);
    }
}

void PayAdmiralAbsolute( long whichAdmiral, smallFixedType howMuch)

{
    admiralType     *admiral = ( admiralType *)*gAresGlobal->gAdmiralData + whichAdmiral;

    if (( whichAdmiral >= 0) && ( whichAdmiral < kScenarioPlayerNum))
    {
        admiral->cash += howMuch;
        if ( admiral->cash < 0) admiral->cash = 0;
    }
}

void AlterAdmiralScore( long whichAdmiral, long whichScore, long amount)
{
    admiralType     *admiral = ( admiralType *)*gAresGlobal->gAdmiralData + whichAdmiral;

    if (( whichAdmiral >= 0) && ( whichAdmiral < kScenarioPlayerNum) &&
        ( whichScore >= 0) && ( whichScore < kAdmiralScoreNum))
    {
        admiral->score[whichScore] += amount;
    }

}

long GetAdmiralScore( long whichAdmiral, long whichScore)
{
    admiralType     *admiral = ( admiralType *)*gAresGlobal->gAdmiralData + whichAdmiral;

    if (( whichAdmiral >= 0) && ( whichAdmiral < kScenarioPlayerNum) &&
        ( whichScore >= 0) && ( whichScore < kAdmiralScoreNum))
        return( admiral->score[whichScore]);
    else return (0);
}

long GetAdmiralShipsLeft( long whichAdmiral)
{
    admiralType     *admiral = ( admiralType *)*gAresGlobal->gAdmiralData + whichAdmiral;

    if (( whichAdmiral >= 0) && ( whichAdmiral < kScenarioPlayerNum))
        return( admiral->shipsLeft);
    else return (0);
}

long AlterDestinationObjectOccupation( long whichDestination, long whichAdmiral, long amount)
{
    destBalanceType *d = mGetDestObjectBalancePtr( whichDestination);

//  WriteDebugLine((char *)"\pAlterOcc");
    if ( whichAdmiral >= 0)
    {
        d->occupied[whichAdmiral] += amount;
        return( d->occupied[whichAdmiral]);
    } else return ( -1);
}

void ClearAllOccupants( long whichDestination, long whichAdmiral, long fullAmount)
{
    destBalanceType *d = mGetDestObjectBalancePtr( whichDestination);
    long            i;

//  WriteDebugLine((char *)"\pClearAllOcc");
    for ( i = 0; i < kScenarioPlayerNum; i++)
    {
        d->occupied[i] = 0;
    }
    if ( whichAdmiral >= 0)
        d->occupied[whichAdmiral] = fullAmount;
}

void AddKillToAdmiral( spaceObjectType *anObject)
{
    admiralType     *admiral = ( admiralType *)*gAresGlobal->gAdmiralData +
        gAresGlobal->gPlayerAdmiralNumber; // only for player

    if ( anObject->attributes & kCanAcceptDestination)
    {
        if ( anObject->owner == gAresGlobal->gPlayerAdmiralNumber)
        {
            admiral->losses++;
        } else
        {
            admiral->kills++;
        }
    }
}

long GetAdmiralLoss( long whichAdmiral)
{
    admiralType     *admiral = ( admiralType *)*gAresGlobal->gAdmiralData + whichAdmiral;

    if (( whichAdmiral >= 0) && ( whichAdmiral < kScenarioPlayerNum))
        return( admiral->losses);
    else return (0);
}

long GetAdmiralKill( long whichAdmiral)
{
    admiralType     *admiral = ( admiralType *)*gAresGlobal->gAdmiralData + whichAdmiral;

    if (( whichAdmiral >= 0) && ( whichAdmiral < kScenarioPlayerNum))
        return( admiral->kills);
    else return (0);
}

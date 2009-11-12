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

#include "ScenarioMaker.hpp"

#include "Admiral.hpp"
#include "AresGlobalType.hpp"
#include "Beam.hpp"
#include "BinaryStream.hpp"
#include "ColorTranslation.hpp"
#include "Debug.hpp"
#include "Error.hpp"
#include "Instruments.hpp"
#include "KeyMapTranslation.hpp"        // major hack for testing
#include "MathMacros.hpp"
#include "MessageScreen.hpp" // for checking which message we're lookg at
#include "Minicomputer.hpp"
#include "Motion.hpp"
#include "NatePixTable.hpp"
#include "NonPlayerShip.hpp"
#include "Options.hpp"
#include "PlayerInterface.hpp"
#include "PlayerShip.hpp"
#include "Races.hpp"
#include "Randomize.hpp"
#include "Resource.hpp"
#include "Rotation.hpp"
#include "ScreenLabel.hpp"
#include "ScrollStars.hpp"
#include "SpaceObjectHandling.hpp"
#include "TimeUnit.hpp"
#include "UniverseUnit.hpp"

namespace antares {

#define kScenarioError          "\pSCNR"

#define kOwnerMayChangeFlag         0x80000000
#define kAnyOwnerColorFlag          0x0000ffff

#define kLevelNameID                4600

// given a baseobject, a verb type, and a verb num, this sets a ptr to that action



enum endgameCheckStatus {
    kUnchecked = 1,
    kPositivecheck = 2,
    kNegativecheck = 3
};

struct endgameCheckType {
    endgameCheckStatus  checked;
//  scenarioEndgameType endgame;
};

extern int32_t gRandomSeed;
extern long gAbsoluteScale;
extern scoped_array<spaceObjectType> gSpaceObjectData;
extern scoped_array<baseObjectType> gBaseObjectData;
extern scoped_array<objectActionType> gObjectActionData;

scenarioType* gThisScenario = nil;

short ScenarioMakerInit( void)

{
/*
    globals()->gScenarioRefID = ARF_OpenResFile( kScenarioResFileName);
    if ( globals()->gScenarioRefID == -1)
    {
        ShowErrorAny( eQuitErr, kErrorStrID, nil, nil, nil, nil, kScenariosFileError, kDataFolderError, -1, -1, __FILE__, 1);
        return( RESOURCE_ERROR);
    }
*/
//  if ( globals()->externalFileRefNum > 0)
//      UseResFile( globals()->externalFileRefNum);

    {
        Resource rsrc('nlAG', 128);
        BufferBinaryReader bin(rsrc.data(), rsrc.size());
        bin.read(&globals()->scenarioFileInfo);
        check(bin.bytes_read() == rsrc.size(), "didn't consume all of scenario file info data");
    }

    if (globals()->gScenarioData.get() == nil) {
        Resource rsrc('snro', kScenarioResID);
        BufferBinaryReader bin(rsrc.data(), rsrc.size());
        size_t count = rsrc.size() / scenarioType::byte_size;
        globals()->scenarioNum = count;
        globals()->gScenarioData.reset(new scenarioType[count]);
        for (size_t i = 0; i < count; ++i) {
            bin.read(globals()->gScenarioData.get() + i);
        }
        check(bin.bytes_read() == rsrc.size(), "didn't consume all of scenario data");
    }

    if (globals()->gScenarioInitialData.get() == nil) {
        Resource rsrc('snit', kScenarioInitialResID);
        BufferBinaryReader bin(rsrc.data(), rsrc.size());
        size_t count = rsrc.size() / scenarioInitialType::byte_size;
        globals()->maxScenarioInitial = count;
        globals()->gScenarioInitialData.reset(new scenarioInitialType[count]);
        for (size_t i = 0; i < count; ++i) {
            bin.read(globals()->gScenarioInitialData.get() + i);
        }
        check(bin.bytes_read() == rsrc.size(), "didn't consume all of initial object data");
    }

    if (globals()->gScenarioConditionData.get() == nil) {
        Resource rsrc('sncd', kScenarioConditionResID);
        BufferBinaryReader bin(rsrc.data(), rsrc.size());
        size_t count = rsrc.size() / scenarioConditionType::byte_size;
        globals()->maxScenarioCondition = count;
        globals()->gScenarioConditionData.reset(new scenarioConditionType[count]);
        for (size_t i = 0; i < count; ++i) {
            bin.read(globals()->gScenarioConditionData.get() + i);
        }
        check(bin.bytes_read() == rsrc.size(), "didn't consume all of condition data");
    }

    if (globals()->gScenarioBriefData.get() == nil) {
        Resource rsrc('snbf', kScenarioBriefResID);
        BufferBinaryReader bin(rsrc.data(), rsrc.size());
        size_t count = rsrc.size() / briefPointType::byte_size;
        globals()->maxScenarioBrief = count;
        globals()->gScenarioBriefData.reset(new briefPointType[count]);
        for (size_t i = 0; i < count; ++i) {
            bin.read(globals()->gScenarioBriefData.get() + i);
        }
        check(bin.bytes_read() == rsrc.size(), "didn't consume all of briefing data");
    }

    return ( InitRaces());
}

void ScenarioMakerCleanup( void)

{
    globals()->gScenarioData.reset();
    globals()->gScenarioBriefData.reset();
    globals()->gScenarioInitialData.reset();
    globals()->gScenarioConditionData.reset();
    CleanupRaces();
}


bool ConstructScenario( long which)

{
    long                count, owner, type, specialAttributes,
                        newShipNum, c2, c3, baseClass, race;
    coordPointType      coord;
    fixedPointType      v = {0, 0};
    baseObjectType      *baseObject;
    spaceObjectType     *anObject;
    scenarioConditionType   *condition;
    scenarioInitialType     *initial;
    objectActionType        *action;
    Rect                    loadingRect;
    long                    stepNumber, currentStep = 0;
    Str255              s;

    v.h = 0; v.v = 0;

    WriteDebugLine("\pSIZE:");
    WriteDebugLong( sizeof( scenarioInitialType));

    ResetAllSpaceObjects();
    ResetActionQueueData();
    ResetBeams();
    ResetAllSprites();
    ResetAllLabels();
    ResetInstruments();
    ResetAllAdmirals();
    ResetAllDestObjectData();
//  ClearMessage();             // moved so tutorial labels aren't convered by ship labels
    ResetMotionGlobals();
    gAbsoluteScale = kTimesTwoScale; //SCALE_SCALE;
    globals()->gSynchValue = 0;

    if ( globals()->gOptions & kOptionNetworkOn)
    {
#ifdef NETSPROCKET_AVAILABLE
        if ( IAmHosting())
        {
            globals()->gThisScenarioNumber = which;
            gRandomSeed = Randomize( 32760);
            SendStartMessage();
        } else
        {
            globals()->gThisScenarioNumber = -1;
            gRandomSeed = -1;
            if (( globals()->gThisScenarioNumber == -1) && ( gRandomSeed == -1))
            {
                if ( WaitForAllStart() == false)
                {
                    StopNetworking();
                    return( false);
                }
            }
            which = globals()->gThisScenarioNumber;
        }
#endif
    } else
    {
        globals()->gThisScenarioNumber = which;
    }


    count = GetScenarioAngle( globals()->gThisScenarioNumber);
    if ( count < 0)
        globals()->gScenarioRotation = RandomSeeded( ROT_POS, &gRandomSeed, 'scm0', -1);
    else
        globals()->gScenarioRotation = count;

    gThisScenario = globals()->gScenarioData.get() + which;

    globals()->gScenarioWinner.player = -1;
    globals()->gScenarioWinner.next = -1;
    globals()->gScenarioWinner.text = -1;

    SetMiniScreenStatusStrList( gThisScenario->scoreStringResID);

//  WriteDebugDivider();

    // *** BEGIN INIT ADMIRALS ***

    mWriteDebugString("\pSCN#");
    WriteDebugLong( which);
    WriteDebugLong( gThisScenario->playerNum);
    for( count = 0; count < kScenarioPlayerNum; count++)
    {
        gThisScenario->player[count].admiralNumber = -1;
    }

    for ( count = 0; count < gThisScenario->playerNum; count++)
    {
        if ( globals()->gOptions & kOptionNetworkOn)
        {
#ifdef NETSPROCKET_AVAILABLE
            if ( gThisScenario->player[count].playerType == kComputerPlayer)
            {
                gThisScenario->player[count].admiralNumber =
                    MakeNewAdmiral( kNoShip, kNoDestinationObject, kNoDestinationType,
                                            kAIsComputer, gThisScenario->player[count].playerRace,
                                            gThisScenario->player[count].nameResID,
                                            gThisScenario->player[count].nameStrNum,
                                            gThisScenario->player[count].earningPower);
                PayAdmiral( gThisScenario->player[count].admiralNumber, mLongToFixed( 5000));
            } else if ( GetPlayerRace( count) >= 0)
            {
                if ( count == globals()->gPlayerAdmiralNumber)
                {
                    admiralType = 0;
                } else
                {
                    admiralType = kAIsRemote;
                }
                gThisScenario->player[count].admiralNumber =
                    MakeNewAdmiral( kNoShip, kNoDestinationObject,
                        kNoDestinationType, kAIsHuman | admiralType,
                        GetRaceIDFromNum( GetPlayerRace( count)),
                        gThisScenario->player[count].nameResID,
                        gThisScenario->player[count].nameStrNum,
                        gThisScenario->player[count].earningPower);
                PayAdmiral( gThisScenario->player[count].admiralNumber,
                    mLongToFixed( 5000));
                SetAdmiralColor( gThisScenario->player[count].admiralNumber,
                    GetPlayerColor( count));
                SetAdmiralName( gThisScenario->player[count].admiralNumber,
                    (anyCharType *)GetPlayerName( count));
            }
#endif NETSPROCKET_AVAILABLE
        } else
        {
            if ( gThisScenario->player[count].playerType == kSingleHumanPlayer)
            {
                if ( globals()->gOptions & kOptionAutoPlay)
                {
                    gThisScenario->player[count].admiralNumber = MakeNewAdmiral( kNoShip, kNoDestinationObject, kNoDestinationType,
                                                kAIsComputer, gThisScenario->player[count].playerRace,
                                                gThisScenario->player[count].nameResID,
                                                gThisScenario->player[count].nameStrNum, gThisScenario->player[count].earningPower);
                    PayAdmiral( gThisScenario->player[count].admiralNumber, mLongToFixed( 5000));
                } else
                {
                    gThisScenario->player[count].admiralNumber = MakeNewAdmiral( kNoShip, kNoDestinationObject, kNoDestinationType,
                                                kAIsHuman, gThisScenario->player[count].playerRace,
                                                gThisScenario->player[count].nameResID,
                                                gThisScenario->player[count].nameStrNum, gThisScenario->player[count].earningPower);
                    PayAdmiral( gThisScenario->player[count].admiralNumber, mLongToFixed( 5000));
                }
                globals()->gPlayerAdmiralNumber = gThisScenario->player[count].admiralNumber;
                mWriteDebugString("\pSETADM#:");
                WriteDebugLong( globals()->gPlayerAdmiralNumber);
            } else
            {
                gThisScenario->player[count].admiralNumber = MakeNewAdmiral( kNoShip, kNoDestinationObject, kNoDestinationType,
                                            kAIsComputer, gThisScenario->player[count].playerRace,
                                                    gThisScenario->player[count].nameResID,
                                                    gThisScenario->player[count].nameStrNum, gThisScenario->player[count].earningPower);
                PayAdmiral( gThisScenario->player[count].admiralNumber, mLongToFixed( 5000));
            }
        }
    }

    // *** END INIT ADMIRALS ***

    ///// FIRST SELECT WHAT MEDIA WE NEED TO USE:

    // uncheck all base objects
    SetAllBaseObjectsUnchecked();
    // uncheck all sounds
    SetAllSoundsNoKeep();
    SetAllPixTablesNoKeep();

    stepNumber = gThisScenario->initialNum * 4L + (gThisScenario->startTime & kScenario_StartTimeMask); // for each run through the initial num
    GetIndString( s, kLevelNameID, gThisScenario->levelNameStrNum);
    DoLoadingInterface( &loadingRect, s);
    UpdateLoadingInterface( currentStep, stepNumber, &loadingRect);

    WriteDebugLine("\p- C H E C K -");

    // for each initial object

    check(globals()->scenarioFileInfo.energyBlobID >= 0, "No energy blob defined");
    check(globals()->scenarioFileInfo.warpInFlareID >= 0, "No warp in flare defined");
    check(globals()->scenarioFileInfo.warpOutFlareID >= 0, "No warp out flare defined");
    check(globals()->scenarioFileInfo.playerBodyID >= 0, "No player body defined");

    for ( count = 0; count < gThisScenario->playerNum; count++)
    {
        baseObject = mGetBaseObjectPtr( globals()->scenarioFileInfo.energyBlobID);
        if ( baseObject != nil)
            CheckBaseObjectMedia( baseObject, 0);   // special case; always neutral
        baseObject = mGetBaseObjectPtr( globals()->scenarioFileInfo.warpInFlareID);
        if ( baseObject != nil)
            CheckBaseObjectMedia( baseObject, 0); // special case; always neutral
        baseObject = mGetBaseObjectPtr( globals()->scenarioFileInfo.warpOutFlareID);
        if ( baseObject != nil)
            CheckBaseObjectMedia( baseObject, 0); // special case; always neutral
        baseObject = mGetBaseObjectPtr( globals()->scenarioFileInfo.playerBodyID);
        if ( baseObject != nil)
            CheckBaseObjectMedia( baseObject, GetAdmiralColor( count));
    }

    for ( count = 0; count < gThisScenario->initialNum; count++)
    {
        initial = mGetScenarioInitial( gThisScenario, count);
        currentStep++;
        UpdateLoadingInterface( currentStep, stepNumber, &loadingRect);
        // get the base object equiv
        baseObject = mGetBaseObjectPtr( initial->type);
        if ((globals()->gOptions & kOptionNetworkOn) && (GetAdmiralRace( initial->owner) >= 0) &&
            ( !(initial->attributes & kFixedRace)))
        {
            baseClass = baseObject->baseClass;
            race = GetAdmiralRace( initial->owner);
            mGetBaseObjectFromClassRace( baseObject, newShipNum, baseClass, race);
            if ( baseObject == nil) baseObject = mGetBaseObjectPtr( initial->type);
        }
        // check the media for this object
//      WriteDebugLine((char *)"\pChecking...");
//      WriteDebugLong( initial->type);
        if ( baseObject->attributes & kIsDestination)
        {
            for ( c2 = 0; c2 < gThisScenario->playerNum; c2++)
            {
                CheckBaseObjectMedia( baseObject, GetAdmiralColor(c2));
            }
        } else
        {
            CheckBaseObjectMedia( baseObject, GetAdmiralColor(initial->owner));
        }

        // check any objects this object can build
        for ( c2 = 0; c2 < kMaxTypeBaseCanBuild; c2++)
        {
            if ( initial->canBuild[c2] != kNoClass)
            {
                // check for each player
                for ( c3 = 0; c3 < gThisScenario->playerNum; c3++)
                {
//                  mGetBaseObjectFromClassRace( baseObject, newShipNum, initial->canBuild[c2], gThisScenario->player[c3].playerRace)
                    mGetBaseObjectFromClassRace( baseObject, newShipNum, initial->canBuild[c2], GetAdmiralRace( c3));
//      WriteDebugLine((char *)"\pMake");
//      WriteDebugLong( newShipNum);
//                  WriteDebugLine((char *)"\pChecking Class...");
//                  WriteDebugLong( initial->canBuild[c2]);
                    if ( baseObject != nil)
                        CheckBaseObjectMedia( baseObject, GetAdmiralColor( c3));
                }
            }
        }
    }

    // check media for all condition actions
    condition = mGetScenarioCondition( gThisScenario, 0);
    for ( count = 0; count < gThisScenario->conditionNum; count++)
    {
        CheckActionMedia( condition->startVerb, condition->verbNum, 0);
        condition = mGetScenarioCondition( gThisScenario, count);

//      condition++;
    }

    // make sure we check things whose owner may change
    for ( count = 0; count < kMaxBaseObject; count++)
    {
        baseObject = mGetBaseObjectPtr( count);
        if ( (baseObject->internalFlags & kOwnerMayChangeFlag) &&
            ( baseObject->internalFlags & kAnyOwnerColorFlag))
        {
            for ( c2 = 0; c2 < gThisScenario->playerNum; c2++)
            {
                CheckBaseObjectMedia( baseObject, GetAdmiralColor( c2));
            }
        }
    }

    SetAllBaseObjectsUnchecked();

    WriteDebugLine("\p- A D D -");
    RemoveAllUnusedSounds();
    RemoveAllUnusedPixTables();

    for ( count = 0; count < gThisScenario->playerNum; count++)
    {
        baseObject = mGetBaseObjectPtr( globals()->scenarioFileInfo.energyBlobID);
        if ( baseObject != nil)
        {
            AddBaseObjectMedia( globals()->scenarioFileInfo.energyBlobID, 0); // special case; always neutral
    //      scenario = *globals()->gScenarioData + which;
        }
        baseObject = mGetBaseObjectPtr( globals()->scenarioFileInfo.warpInFlareID);
        if ( baseObject != nil)
        {
            AddBaseObjectMedia( globals()->scenarioFileInfo.warpInFlareID, 0); // special case; always neutral
    //      scenario = *globals()->gScenarioData + which;
        }
        baseObject = mGetBaseObjectPtr( globals()->scenarioFileInfo.warpOutFlareID);
        if ( baseObject != nil)
        {
            AddBaseObjectMedia( globals()->scenarioFileInfo.warpOutFlareID, 0); // special case; always neutral
    //      scenario = *globals()->gScenarioData + which;
        }
        baseObject = mGetBaseObjectPtr( globals()->scenarioFileInfo.playerBodyID);
        if ( baseObject != nil)
        {
            AddBaseObjectMedia( globals()->scenarioFileInfo.playerBodyID, GetAdmiralColor( count));
    //      scenario = *globals()->gScenarioData + which;
        }
    }

    for ( count = 0; count < gThisScenario->initialNum; count++)
    {
        currentStep++;
        UpdateLoadingInterface( currentStep, stepNumber, &loadingRect);

        initial = mGetScenarioInitial( gThisScenario, count);

        // get the base object equiv
        type = initial->type;
        baseObject = mGetBaseObjectPtr( type);
        if ((globals()->gOptions & kOptionNetworkOn) && (GetAdmiralRace( initial->owner) >= 0) &&
            ( !(initial->attributes & kFixedRace)))
        {
            baseClass = baseObject->baseClass;
            race = GetAdmiralRace( initial->owner);
            mGetBaseObjectFromClassRace( baseObject, type, baseClass, race);
            if ( baseObject == nil)
            {
                baseObject = mGetBaseObjectPtr( initial->type);
                type = initial->type;
            }
        }
        // check the media for this object
        if ( baseObject->attributes & kIsDestination)
        {
            for ( c2 = 0; c2 < gThisScenario->playerNum; c2++)
            {
                AddBaseObjectMedia( type, GetAdmiralColor(c2));
            }
        } else
        {
            AddBaseObjectMedia( type, GetAdmiralColor(initial->owner));
        }

        // we may have just moved memory, so let's make sure our ptrs are correct
//      scenario = *globals()->gScenarioData + which;
        initial = mGetScenarioInitial( gThisScenario, count);
        baseObject = mGetBaseObjectPtr( type);

        // make sure we're not overriding the sprite
        if ( initial->spriteIDOverride >= 0)
        {
            if ( baseObject->attributes & kCanThink)
            {
                AddPixTable( initial->spriteIDOverride +
                    (GetAdmiralColor( initial->owner) << kSpriteTableColorShift));
            } else
            {
                AddPixTable( initial->spriteIDOverride);
            }
//          scenario = *globals()->gScenarioData + which;
        }

        // check any objects this object can build
        for ( c2 = 0; c2 < kMaxTypeBaseCanBuild; c2++)
        {
            initial = mGetScenarioInitial( gThisScenario, count);
            if ( initial->canBuild[c2] != kNoClass)
            {
                // check for each player
                for ( c3 = 0; c3 < gThisScenario->playerNum; c3++)
                {
                    initial = mGetScenarioInitial( gThisScenario, count);
                    baseObject = mGetBaseObjectPtr( type);
//                  mGetBaseObjectFromClassRace( baseObject, newShipNum, initial->canBuild[c2], gThisScenario->player[c3].playerRace)
                    mGetBaseObjectFromClassRace( baseObject, newShipNum, initial->canBuild[c2], GetAdmiralRace( c3));
                    if ( baseObject != nil)
                    {
                        AddBaseObjectMedia( newShipNum, GetAdmiralColor( c3));
//                      scenario = *globals()->gScenarioData + which;
                    }
                }
            }
        }
    }

    // add media for all condition actions
    condition = mGetScenarioCondition( gThisScenario, 0);
    for ( count = 0; count < gThisScenario->conditionNum; count++)
    {
        condition = mGetScenarioCondition( gThisScenario, count);
        action = gObjectActionData.get() + condition->startVerb;
        for ( c2 = 0; c2 < condition->verbNum; c2++)
        {
            condition = mGetScenarioCondition( gThisScenario, count);
            action = gObjectActionData.get() + condition->startVerb + c2;
            AddActionMedia( action, 0);
//          action++;
        }
//      condition++;
    }

    // make sure we check things whose owner may change
    for ( count = 0; count < kMaxBaseObject; count++)
    {
        baseObject = mGetBaseObjectPtr( count);
        if ( (baseObject->internalFlags & kOwnerMayChangeFlag) &&
            ( baseObject->internalFlags & kAnyOwnerColorFlag))
        {
            for ( c2 = 0; c2 < gThisScenario->playerNum; c2++)
            {
                AddBaseObjectMedia( count, GetAdmiralColor( c2));
            }
        }
    }

    SetAllBaseObjectsUnchecked();

    // begin init admirals used to be here
    condition = mGetScenarioCondition( gThisScenario, 0);
    for ( count = 0; count < gThisScenario->conditionNum; count++)
    {
        if ( condition->flags & kInitiallyTrue)
            condition->flags |= kHasBeenTrue;
        else
            condition->flags &= ~kHasBeenTrue;
        condition++;
    }

    for ( ; count < kScenarioEndgameNum; count++)
    {
    }

//  WriteDebugDivider();

    initial = mGetScenarioInitial( gThisScenario, 0);
    for ( count = 0; count < gThisScenario->initialNum; count++)
    {
        currentStep++;
        UpdateLoadingInterface( currentStep, stepNumber, &loadingRect);

        if ( !(initial->attributes & kInitiallyHidden))
        {
            GetInitialCoord( initial, &coord, globals()->gScenarioRotation);

            if ( initial->owner > kScenarioNoOwner)
                owner = gThisScenario->player[initial->owner].admiralNumber;
            else owner = kScenarioNoOwner;

            specialAttributes = initial->attributes & ( ~kInitialAttributesMask);
            if ( initial->attributes & kIsPlayerShip)
            {
                if ( GetAdmiralFlagship( owner) == nil)
                {
                    if ( owner == globals()->gPlayerAdmiralNumber)
                    {
                        //specialAttributes &= (~( kCanThink | kCanEngage | kCanEvade | kHasDirectionGoal));
                        if ( globals()->gOptions & kOptionAutoPlay)
                        {
        //                  specialAttributes |= kIsPlayerShip;
                        } else
                        {
                            specialAttributes |= kIsHumanControlled;// | kIsPlayerShip;
                        }
                    } else
                    {
    //                  specialAttributes &= ~kIsPlayerShip;
                        if ( globals()->gOptions & kOptionNetworkOn)
                        {
                            specialAttributes |= kIsRemote;
                        } else
                        {
                            specialAttributes &= ~kIsPlayerShip;
                        }
                    }
                } else // we already have a flagship; this should not override
                {
                    specialAttributes &= ~kIsPlayerShip;
                }
            }


            type = initial->type;
            if ((globals()->gOptions & kOptionNetworkOn) && (GetAdmiralRace( initial->owner) >= 0) &&
                ( !(initial->attributes & kFixedRace)))
            {
                baseObject = mGetBaseObjectPtr( type);
                baseClass = baseObject->baseClass;
                race = GetAdmiralRace( initial->owner);
                mGetBaseObjectFromClassRace( baseObject, type, baseClass, race);
                if ( baseObject == nil)
                {
                    baseObject = mGetBaseObjectPtr( initial->type);
                    type = initial->type;
                }
            }
/*          initial->realObjectNumber = newShipNum = CreateAnySpaceObject( type, &v, &coord,
                                                globals()->gScenarioRotation,
                                                owner,
                                                specialAttributes,
                                                initial->canBuild,
                                                initial->nameResID,
                                                initial->nameStrNum,
                                                initial->spriteIDOverride);
*/
            initial->realObjectNumber = newShipNum = CreateAnySpaceObject( type, &v, &coord,
                                                globals()->gScenarioRotation,
                                                owner,
                                                specialAttributes,
                                                initial->spriteIDOverride);

            anObject = gSpaceObjectData.get() + newShipNum;
            if ( anObject->attributes & kIsDestination)
            {
                anObject->destinationObject = MakeNewDestination( newShipNum, initial->canBuild,
                    initial->earning, initial->nameResID, initial->nameStrNum);
            }
//          WriteDebugLine((char *)"\pROTATION");
//          WriteDebugLong( globals()->gScenarioRotation);
//          WriteDebugLong( anObject->direction);
            initial->realObjectID = anObject->id;
            if (( initial->attributes & kIsPlayerShip) && ( GetAdmiralFlagship( owner)
                == nil))
            {
                SetAdmiralFlagship( owner, newShipNum);
                if ( owner == globals()->gPlayerAdmiralNumber)
                {
                    ResetPlayerShip( newShipNum);
                } else
                {
                    if ( globals()->gOptions & kOptionNetworkOn)
                    {
                        anObject->attributes |= kIsRemote;
                    }
                }
            }

            if ( anObject->attributes & kIsDestination)
            {
                if ( owner >= 0)
                {
                    if ( initial->canBuild[0] >= 0)
                    {
                        if ( GetAdmiralBuildAtObject( owner) < 0)
                        {
                            SetAdmiralConsiderObject( owner, newShipNum);
                            SetAdmiralDestinationObject( owner, newShipNum, kObjectDestinationType);
                        }
                    }
                }
            }
        } else
        {
            initial->realObjectNumber = -1;
        }
        initial++;
    }

    // double back and set up any defined initial destinations
    for ( count = 0; count < gThisScenario->initialNum; count++)
    {
        currentStep++;
        UpdateLoadingInterface( currentStep, stepNumber, &loadingRect);

        initial = mGetScenarioInitial( gThisScenario, count);
        // if the initial object has an initial destination
        if (( initial->realObjectNumber >= 0) && ( initial->initialDestination >= 0))
        {
            // only objects controlled by an Admiral can have destinations
            if ( initial->owner > kScenarioNoOwner)
            {
                // get the correct admiral #

                owner = gThisScenario->player[initial->owner].admiralNumber;
                initial = mGetScenarioInitial( gThisScenario, initial->initialDestination);

                // set the admiral's dest object to the mapped initial dest object
                SetAdmiralDestinationObject( owner,
                    initial->realObjectNumber,
                    kObjectDestinationType);

                // now give the mapped initial object the admiral's destination

                initial = mGetScenarioInitial( gThisScenario, count);
                anObject = gSpaceObjectData.get() + initial->realObjectNumber;
                specialAttributes = anObject->attributes; // preserve the attributes
                anObject->attributes &= ~kStaticDestination; // we've got to force this off so we can set dest
                SetObjectDestination( anObject, nil);
                anObject->attributes = specialAttributes;
            }
        }
    }

    // set up all the admiral's destination objects
    RecalcAllAdmiralBuildData();

    mWriteDebugString("\pPlayerAdmiral");
    WriteDebugLong( globals()->gPlayerAdmiralNumber);
    if ( globals()->gOptions & kOptionNetworkOn)
    {
        for ( c2 = 0; c2 < gThisScenario->playerNum; c2++)
        {
            mWriteDebugString("\pChecking #");
            WriteDebugLong( c2);
            if ( GetAdmiralFlagship( c2) == nil)
            {
                mWriteDebugString("\pAdmiral Needs Flagship!");
                anObject = gSpaceObjectData.get();
                count = 0;
                while ((((anObject->attributes & kCanThink) != kCanThink) ||
                    (anObject->owner != c2)) && ( count < kMaxSpaceObject))
                {
                    count++;
                    anObject++;
                }

                if ( count < kMaxSpaceObject)
                {
                    mWriteDebugString("\pGot it!");
                    WriteDebugLong( count);
                    WriteDebugLong( anObject->owner);
                    WriteDebugLong( c2);
                    SetAdmiralFlagship( c2, count);
                    anObject->attributes |= kIsPlayerShip;
                    if ( c2 != globals()->gPlayerAdmiralNumber)
                    {
                        mWriteDebugString("\pREMOTE");
                        anObject->attributes |= kIsRemote;
                    } else
                    {
                        mWriteDebugString("\pHUMAN");
                        anObject->attributes |= kIsHumanControlled;
                        ResetPlayerShip( count);
                    }
                }
            }
        }
    }
/*  for ( count = 0; count < kMaxSpaceObject; count++)
    {
        if ( anObject->active);
        {
            DebugFileAppendString("\p-\t");
            DebugFileAppendLong( globals()->gGameTime);
            DebugFileAppendString("\p\t");
//          DebugFileAppendLongHex( globals()->gTheseKeys);
            DebugFileAppendString("\p\r");
        }
        anObject++;
    }
*/
    ClearMessage();

    c2 = 0;
    for ( count = 0; count < ((gThisScenario->startTime & kScenario_StartTimeMask) * 20); count++)
    {
        globals()->gGameTime = count;
        MoveSpaceObjects( gSpaceObjectData.get(), kMaxSpaceObject,
                    kDecideEveryCycles);
        NonplayerShipThink( kDecideEveryCycles);
        AdmiralThink();
        ExecuteActionQueue( kDecideEveryCycles);
        CollideSpaceObjects( gSpaceObjectData.get(), kMaxSpaceObject);
        c2++;
        if ( c2 == 30)
        {
            c2 = 0;
            CheckScenarioConditions( 0);
        }
        CullSprites();
        CullBeams();
        if ((count % kScenarioTimeMultiple) == 0)
        {
            currentStep++;
            UpdateLoadingInterface( currentStep, stepNumber, &loadingRect);
        }
    }
    globals()->gGameTime = (gThisScenario->startTime & kScenario_StartTimeMask) * kScenarioTimeMultiple;

    return( true);
}

void SetAllBaseObjectsUnchecked( void)

{
    baseObjectType  *aBase = gBaseObjectData.get();
    long            count;

    for ( count = 0; count < kMaxBaseObject; count++)
    {
//      aBase->attributes &= ~kHaveCheckedMedia;
        aBase->internalFlags = 0;
        aBase++;
    }
}

void CheckBaseObjectMedia( baseObjectType *aBase, unsigned char color)

{
    baseObjectType  *weapon;

    if ( !(aBase->internalFlags & (0x00000001 << color)))
    {
        aBase->internalFlags |= (0x00000001 << color);
        if ( aBase->attributes & kCanThink)
        {
            if ( aBase->pixResID != kNoSpriteTable)
                KeepPixTable( aBase->pixResID +
                    (color << kSpriteTableColorShift));
        } else
        {
            if ( aBase->pixResID != kNoSpriteTable)
                KeepPixTable( aBase->pixResID);
        }

        CheckActionMedia( aBase->destroyAction, (aBase->destroyActionNum & kDestroyActionNotMask), color);
        CheckActionMedia( aBase->expireAction, (aBase->expireActionNum & kDestroyActionNotMask), color);
        CheckActionMedia( aBase->createAction, aBase->createActionNum, color);
        CheckActionMedia( aBase->collideAction, aBase->collideActionNum, color);
        CheckActionMedia( aBase->activateAction, (aBase->activateActionNum & kPeriodicActionNotMask), color);
        CheckActionMedia( aBase->arriveAction, aBase->arriveActionNum, color);

        if ( aBase->pulse != kNoWeapon)
        {
            weapon = mGetBaseObjectPtr( aBase->pulse);
            CheckBaseObjectMedia( weapon, color);
        }
        if ( aBase->beam != kNoWeapon)
        {
            weapon = mGetBaseObjectPtr( aBase->beam);
            CheckBaseObjectMedia( weapon, color);
        }
        if ( aBase->special != kNoWeapon)
        {
            weapon = mGetBaseObjectPtr( aBase->special);
            CheckBaseObjectMedia( weapon, color);
        }
    }
}

void CheckActionMedia( long whichAction, long actionNum, unsigned char color)

{
    baseObjectType      *baseObject;
    objectActionType    *action = gObjectActionData.get() + whichAction;
    bool             OKtoExecute;
    long                count;

    while ((actionNum > 0) && (action->verb != kNoAction))
    {
        switch ( action->verb)
        {
            case kCreateObject:
            case kCreateObjectSetDest:
                baseObject = mGetBaseObjectPtr( action->argument.createObject.whichBaseType);
                CheckBaseObjectMedia( baseObject, color);
                break;

            case kPlaySound:
                for (   count = action->argument.playSound.idMinimum;
                        count <= (action->argument.playSound.idMinimum +
                                    action->argument.playSound.idRange);
                        count++)
                {
                    KeepSound( count); // FIX to check for range of sounds
                }
                break;

            case kAlter:
                switch( action->argument.alterObject.alterType)
                {
                    case kAlterBaseType:
                        baseObject = mGetBaseObjectPtr( action->argument.alterObject.minimum);
                        CheckBaseObjectMedia( baseObject, color);
                        break;

                    case kAlterOwner:
                        baseObject = gBaseObjectData.get();
                        for ( count = 0; count < kMaxBaseObject; count++)
                        {
                            OKtoExecute = false;
                            if ( action->exclusiveFilter == 0xffffffff)
                            {
                                if (    (action->inclusiveFilter & kLevelKeyTagMask) ==
                                            ( baseObject->buildFlags & kLevelKeyTagMask)
                                    )
                                {
                                    OKtoExecute = true;
                                }
                            } else if ( ( action->inclusiveFilter & baseObject->attributes) == action->inclusiveFilter)
                            {
                                OKtoExecute = true;
                            }
                            if ( OKtoExecute)
                            {
                                baseObject->internalFlags |= kOwnerMayChangeFlag;
                            }
                            baseObject++;
                        }
                        break;

                    default:
                        break;

                }
                break;

            case kMakeSparks:
            case kNoAction:
            case kDie:
            default:
                break;
        }
        actionNum--;
        action++;
    }
}

void AddBaseObjectMedia( long whichBase, unsigned char color)

{
    baseObjectType      *aBase = mGetBaseObjectPtr( whichBase);

    mWriteDebugString("\pADDBASE:");
    WriteDebugLong( whichBase);
    if ( !(aBase->internalFlags & (0x00000001 << color)))
    {
        aBase->internalFlags |= (0x00000001 << color);
        if ( aBase->pixResID != kNoSpriteTable)
        {
            if ( aBase->attributes & kCanThink)
            {
                AddPixTable( aBase->pixResID +
                    (color << kSpriteTableColorShift));
            } else
            {
                AddPixTable( aBase->pixResID);      // moves mem
            }
            aBase = mGetBaseObjectPtr( whichBase);
        }

        AddBaseObjectActionMedia( whichBase, kDestroyActionType, color);
        AddBaseObjectActionMedia( whichBase, kExpireActionType, color);
        AddBaseObjectActionMedia( whichBase, kCreateActionType, color);
        AddBaseObjectActionMedia( whichBase, kCollideActionType, color);
        AddBaseObjectActionMedia( whichBase, kActivateActionType, color);
        AddBaseObjectActionMedia( whichBase, kArriveActionType, color);

        aBase = mGetBaseObjectPtr( whichBase);
        if ( aBase->pulse != kNoWeapon)
        {
            aBase = mGetBaseObjectPtr( whichBase);
            AddBaseObjectMedia( aBase->pulse, color);
        }
        aBase = mGetBaseObjectPtr( whichBase);
        if ( aBase->beam != kNoWeapon)
        {
            aBase = mGetBaseObjectPtr( whichBase);
            AddBaseObjectMedia( aBase->beam, color);
        }
        aBase = mGetBaseObjectPtr( whichBase);
        if ( aBase->special != kNoWeapon)
        {
            aBase = mGetBaseObjectPtr( whichBase);
            AddBaseObjectMedia( aBase->special, color);
        }
    }
}

void AddBaseObjectActionMedia( long whichBase, long whichType, unsigned char color)

{
    baseObjectType      *baseObject = nil;
    long                count = 0;
    objectActionType    *action = nil;

    do
    {
        baseObject = mGetBaseObjectPtr( whichBase);
        mGetActionFromBaseTypeNum( action, baseObject, whichType, count);

        AddActionMedia( action, color);
        count++;
    } while ( action != nil);
}

void AddActionMedia( objectActionType *action, unsigned char color)
{
    baseObjectType      *baseObject = nil;
    long                count = 0, l1, l2;
    bool             OKtoExecute;

    if ( action != nil)
    {
        switch ( action->verb)
        {
            case kCreateObject:
            case kCreateObjectSetDest:
                AddBaseObjectMedia( action->argument.createObject.whichBaseType, color);
                break;

            case kPlaySound:
                l1 = action->argument.playSound.idMinimum;
                l2 = action->argument.playSound.idMinimum +
                        action->argument.playSound.idRange;
                for ( count = l1; count <= l2; count++)
                {
                    mWriteDebugString("\pADD SOUND:");
                    WriteDebugLong( count);
                    AddSound( count); // moves mem
                }
                break;

            case kNoAction:
                action = nil;   // get us out of loop
                break;

            case kAlter:
                switch( action->argument.alterObject.alterType)
                {
                    case kAlterBaseType:
                        AddBaseObjectMedia( action->argument.alterObject.minimum, color);
                        break;

                    case kAlterOwner:
                        baseObject = gBaseObjectData.get();
                        for ( count = 0; count < kMaxBaseObject; count++)
                        {
                            OKtoExecute = false;
                            if ( action->exclusiveFilter == 0xffffffff)
                            {
                                if (    (action->inclusiveFilter & kLevelKeyTagMask) ==
                                            ( baseObject->buildFlags & kLevelKeyTagMask)
                                    )
                                {
                                    OKtoExecute = true;
                                }
                            } else if ( ( action->inclusiveFilter & baseObject->attributes) == action->inclusiveFilter)
                            {
                                OKtoExecute = true;
                            }
                            if ( OKtoExecute)
                            {
                                baseObject->internalFlags |= kOwnerMayChangeFlag;
                            }
                            baseObject++;
                        }
                        break;
                }
                break;

            case kMakeSparks:
            case kDie:
            default:
                break;
        }
    }
}

void CheckEndgame( void)

{
}

void GetInitialCoord( scenarioInitialType *initial, coordPointType *coord, long rotation)

{
    int32_t lcos, lsin, lscrap;
//  double  dcos, dsin, dscrap;

//  Debugger();
/*
    mGetRotPoint( lcos, lsin, rotation)
    lcos = -lcos;
    lsin = -lsin;
    dcos = lcos;
    dsin = lsin;
    dcos /= (double)kFixedWholeMultiplier;
    dsin /= (double)kFixedWholeMultiplier;

    dscrap = (double)initial->location.h * dcos;
    dscrap -= (double)initial->location.v * dsin;
    lscrap = dscrap;
    coord->h = kUniversalCenter;
    coord->h += lscrap;

    dscrap = (double)initial->location.h * dsin;
    dscrap += (double)initial->location.v * dcos;
    lscrap = dscrap;
    coord->v = kUniversalCenter;
    coord->v += lscrap;
*/
    mAddAngle( rotation, 90);
    GetRotPoint(&lcos, &lsin, rotation);
    lcos = -lcos;
    lsin = -lsin;

    lscrap = mMultiplyFixed(initial->location.h, lcos);
    lscrap -= mMultiplyFixed(initial->location.v, lsin);
    coord->h = kUniversalCenter;
    coord->h += lscrap;

    lscrap = mMultiplyFixed(initial->location.h, lsin);
    lscrap += mMultiplyFixed(initial->location.v, lcos);
    coord->v = kUniversalCenter;
    coord->v += lscrap;
}

void HackWinLoseMessage( bool win)

{
    if ( win)
    {
        StartMessage();
        AppendStringToMessage("\pYOU WIN");
        EndMessage();
    } else
    {
        StartMessage();
        AppendStringToMessage("\pYou lose");
        EndMessage();
    }
}

void CheckScenarioConditions( long timePass)

{
    scenarioConditionType   *condition = nil;
    spaceObjectType         *sObject = nil, *dObject = nil;
    long                    i, l, difference;
    unsigned long           distance, dcalc;
    scenarioInitialType     *initial;
    Point                   offset(0, 0);
    bool                 conditionTrue = false;

#pragma unused( timePass)

        condition = mGetScenarioCondition( gThisScenario, 0);
        for ( i = 0; i < gThisScenario->conditionNum; i++)
        {
            if ( (!(condition->flags & kTrueOnlyOnce)) || ( !(condition->flags & kHasBeenTrue)))
            {
                conditionTrue = false;
                switch( condition->condition)
                {
                    case kCounterCondition:
                        l = mGetRealAdmiralNum(condition->conditionArgument.counter.whichPlayer);
                        if ( GetAdmiralScore( l, condition->conditionArgument.counter.whichCounter) ==
                            condition->conditionArgument.counter.amount)
                        {
                            conditionTrue = true;
                        }
                        break;

                    case kCounterGreaterCondition:
                        l = mGetRealAdmiralNum(condition->conditionArgument.counter.whichPlayer);
                        if ( GetAdmiralScore( l, condition->conditionArgument.counter.whichCounter) >=
                            condition->conditionArgument.counter.amount)
                        {
                            conditionTrue = true;
                        }
                        break;

                    case kCounterNotCondition:
                        l = mGetRealAdmiralNum(condition->conditionArgument.counter.whichPlayer);
                        if ( GetAdmiralScore( l, condition->conditionArgument.counter.whichCounter) !=
                            condition->conditionArgument.counter.amount)
                        {
                            conditionTrue = true;
                        }
                        break;

                    case kDestructionCondition:
                        sObject = nil;
                        mGetRealObjectFromInitial( sObject, initial, condition->conditionArgument.longValue);
                        if ( sObject == nil)
                        {
                            conditionTrue = true;
                        }
                        break;

                    case kOwnerCondition:
                        sObject = nil;
                        mGetRealObjectFromInitial( sObject, initial, condition->subjectObject);
                        if ( sObject != nil)
                        {
                            l = mGetRealAdmiralNum(condition->conditionArgument.longValue);
                            if ( l == sObject->owner)
                            {
                                conditionTrue = true;
                            }

                        }
                        break;

                    case kTimeCondition:
                        if ( globals()->gGameTime >= condition->conditionArgument.longValue)
                        {
                            conditionTrue = true;
                        }// else WriteDebugLong( globals()->gGameTime);
                        break;

                    case kProximityCondition:
                        sObject = dObject = nil;
                        mWriteDebugString("\pkProximityCondition");
                        WriteDebugLong(i);
                        mGetRealObjectFromInitial( sObject, initial, condition->subjectObject);
                        if ( sObject != nil)
                        {
                            mGetRealObjectFromInitial( dObject, initial, condition->directObject);
                            if ( dObject != nil)
                            {
                                difference = ABS<int>( sObject->location.h - dObject->location.h);
                                dcalc = difference;
                                difference =  ABS<int>( sObject->location.v - dObject->location.v);
                                distance = difference;

                                if (( dcalc < kMaximumRelevantDistance) && ( distance < kMaximumRelevantDistance))
                                {
                                    distance = distance * distance + dcalc * dcalc;
                                    if ( distance < condition->conditionArgument.unsignedLongValue)
                                    {
                                        mWriteDebugString("\ptrue!");
                                        conditionTrue = true;
                                    } else
                                    {
                                    }
                                }
                            }
                        }
                        break;

                    case kDistanceGreaterCondition:
                        sObject = dObject = nil;
                        mGetRealObjectFromInitial( sObject, initial, condition->subjectObject);
                        if ( sObject != nil)
                        {
                            mGetRealObjectFromInitial( dObject, initial, condition->directObject);
                            if ( dObject != nil)
                            {
                                difference = ABS<int>( sObject->location.h - dObject->location.h);
                                dcalc = difference;
                                difference =  ABS<int>( sObject->location.v - dObject->location.v);
                                distance = difference;

                                if (( dcalc < kMaximumRelevantDistance) && ( distance < kMaximumRelevantDistance))
                                {
                                    distance = distance * distance + dcalc * dcalc;
                                    if ( distance >= condition->conditionArgument.unsignedLongValue)
                                    {
                                        conditionTrue = true;
                                    } else
                                    {
                                    }
                                }
                            }
                        }
                        break;

                    case kHalfHealthCondition:
                        sObject = nil;
                        mGetRealObjectFromInitial( sObject, initial, condition->subjectObject);
                        if ( sObject == nil)
                        {
                            conditionTrue = true;
                        } else if ( sObject->health <= ( sObject->baseType->health >> 1))
                        {
                            conditionTrue = true;
                            WriteDebugLine("\pHALFNESS!");
                            WriteDebugLong( condition->directObject);
                        } else
                        {
                            WriteDebugLong( sObject->health);
                        }
                        break;

                    case kIsAuxiliaryObject:
                        sObject = nil;
                        mGetRealObjectFromInitial( sObject, initial, condition->subjectObject);
                        if ( sObject != nil)
                        {
                            l = GetAdmiralConsiderObject( globals()->gPlayerAdmiralNumber);
                            if ( l >= 0)
                            {
                                dObject = gSpaceObjectData.get() + l;
                                if ( dObject == sObject)
                                {
                                    conditionTrue = true;
                                }
                            }
                        }
                        break;

                    case kIsTargetObject:
                        sObject = nil;
                        mGetRealObjectFromInitial( sObject, initial, condition->subjectObject);
                        if ( sObject != nil)
                        {
                            l = GetAdmiralDestinationObject( globals()->gPlayerAdmiralNumber);
                            if ( l >= 0)
                            {
                                dObject = gSpaceObjectData.get() + l;
                                if ( dObject == sObject)
                                {
                                    conditionTrue = true;
                                }
                            }
                        }
                        break;

                    case kVelocityLessThanEqualToCondition:
                        sObject = nil;
                        mGetRealObjectFromInitial( sObject, initial, condition->subjectObject);
                        if ( sObject != nil)
                        {
                            if (( (ABS(sObject->velocity.h)) < condition->conditionArgument.longValue) &&
                                ( (ABS(sObject->velocity.v)) < condition->conditionArgument.longValue))
                            {
                                conditionTrue = true;
                            }
                        }
                        break;

                    case kNoShipsLeftCondition:
                        if ( GetAdmiralShipsLeft( condition->conditionArgument.longValue) <= 0)
                        {
                            conditionTrue = true;
                        }
                        break;

                    case kCurrentMessageCondition:
                        {
                            longMessageType *tmessage;

                            tmessage = globals()->gLongMessageData.get();
                            if ( tmessage->currentResID == (condition->conditionArgument.location.h +
                                condition->conditionArgument.location.v - 1))
                            {
                                conditionTrue = true;
                            }

                        }
                        break;

                    case kCurrentComputerCondition:
                        if (( globals()->gMiniScreenData.currentScreen ==
                            condition->conditionArgument.location.h) &&
                            ((condition->conditionArgument.location.v < 0) ||
                                (globals()->gMiniScreenData.selectLine ==
                                    condition->conditionArgument.location.v)))
                        {
                            conditionTrue = true;
                        }
                        break;

                    case kZoomLevelCondition:
                        if ( globals()->gZoomMode ==
                            condition->conditionArgument.longValue)
                        {
                            conditionTrue = true;
                        }
                        break;

                    case kAutopilotCondition:
                        conditionTrue = IsPlayerShipOnAutoPilot();

                        break;

                    case kNotAutopilotCondition:
                        conditionTrue = !IsPlayerShipOnAutoPilot();
                        break;

                    case kObjectIsBeingBuilt:
                        {

                            destBalanceType     *buildAtObject = nil;

//                          if ( sObject != nil)
                            {
                                buildAtObject = mGetDestObjectBalancePtr( GetAdmiralBuildAtObject( globals()->gPlayerAdmiralNumber));
                                if ( buildAtObject != nil)
                                {
                                    if ( buildAtObject->totalBuildTime > 0)
                                    {
                                        conditionTrue = true;
                                    }
                                }
                            }
                        }
                        break;

                    case kDirectIsSubjectTarget:
                        mGetRealObjectFromInitial( sObject, initial, condition->subjectObject);
                        mGetRealObjectFromInitial( dObject, initial, condition->directObject);
                        if (( sObject != nil) && ( dObject != nil))
                        {
                            if ( sObject->destObjectID == dObject->id)
                                conditionTrue = true;
                        }
                        break;

                    case kSubjectIsPlayerCondition:
                        sObject = nil;
                        mGetRealObjectFromInitial( sObject, initial, condition->subjectObject);
                        if ( sObject != nil)
                        {
                            if ( sObject->entryNumber == globals()->gPlayerShipNumber)
                                conditionTrue = true;
                        }
                        break;

//                  case kVelocityLessThanEqualToCondition:
//                      mGetRealObjectFromInitial( sObject, initial, condition->subjectObject)
//                      if ( sObject != nil)
//                      {
//                          if ( sObject->velocity < condition->conditionArgument.longValue)
//                              conditionTrue = true;
//                      }
//                      break;

                    default:
                        break;

                }
                if ( conditionTrue)
                {
//                  WriteDebugLine((char *)"\ptrue!");
//                  WriteDebugLong( i);

                    condition->flags |= kHasBeenTrue;
                    sObject = nil;
                    dObject = nil;
                    mGetRealObjectFromInitial( sObject, initial, condition->subjectObject);
                    mGetRealObjectFromInitial( dObject, initial, condition->directObject);
                    ExecuteObjectActions( condition->startVerb, condition->verbNum,
                        sObject, dObject, &offset, true);
                }
            }
            condition++;
        }
}

void SetScenarioConditionTrueYet( long whichCondition, bool state)

{
    scenarioConditionType   *condition = nil;

    condition = mGetScenarioCondition( gThisScenario, whichCondition);
    if ( state)
    {
//      WriteDebugLine((char *)"\pSET:true");
//      WriteDebugLong( whichCondition);
        condition->flags |= kHasBeenTrue;
    } else
    {
//      WriteDebugLine((char *)"\pSET:NOTtrue");
//      WriteDebugLong( whichCondition);
        condition->flags &= ~kHasBeenTrue;
    }
}

bool GetScenarioConditionTrue( long whichCondition)
{
    scenarioConditionType   *condition = nil;

    condition = mGetScenarioCondition( gThisScenario, whichCondition);

    if ( condition->flags & kHasBeenTrue) return( true);
    else return( false);
}

long GetRealAdmiralNumber( long whichAdmiral)
{
    long result;

    result = mGetRealAdmiralNum( whichAdmiral);
    return( result);
}

void UnhideInitialObject( long whichInitial)

{
    scenarioInitialType     *initial;
    spaceObjectType         *anObject = nil;
    coordPointType          coord;
    fixedPointType          v = {0, 0};
    unsigned long           specialAttributes;
    long                    newShipNum, owner, type, saveDest, baseClass, race;
    baseObjectType          *baseObject;

    v.h = 0;
    v.v = 0;
    initial = mGetScenarioInitial( gThisScenario, 0);
    mGetRealObjectFromInitial( anObject, initial, whichInitial);
    if ( anObject == nil)
    {
//      if (initial->attributes & kInitiallyHidden)
        {
            GetInitialCoord( initial, &coord, globals()->gScenarioRotation);

            if ( initial->owner > kScenarioNoOwner)
                owner = gThisScenario->player[initial->owner].admiralNumber;
            else owner = kScenarioNoOwner;

            specialAttributes = initial->attributes & ( ~kInitialAttributesMask);
            if ( initial->attributes & kIsPlayerShip)
            {
                if ( GetAdmiralFlagship( owner) == nil)
                {
                    if ( owner == globals()->gPlayerAdmiralNumber)
                    {
    //                  specialAttributes &= (~( kCanThink | kCanEngage | kCanEvade | kHasDirectionGoal));
                        if ( globals()->gOptions & kOptionAutoPlay)
                        {
        //                  specialAttributes |= kIsPlayerShip;
                        } else
                        {
                            specialAttributes |= kIsHumanControlled;// | kIsPlayerShip;
                        }
                    } else
                    {
                        if ( globals()->gOptions & kOptionNetworkOn)
                            specialAttributes |= kIsRemote;
                        else
                            specialAttributes &= ~kIsPlayerShip;
                    }
                } else // we already have a flagship; this should not override
                {
                    specialAttributes &= ~kIsPlayerShip;
                }
            }


            type = initial->type;
            if ((globals()->gOptions & kOptionNetworkOn) && (GetAdmiralRace( initial->owner) >= 0) &&
                ( !(initial->attributes & kFixedRace)))
            {
                baseObject = mGetBaseObjectPtr( type);
                baseClass = baseObject->baseClass;
                race = GetAdmiralRace( initial->owner);
                mGetBaseObjectFromClassRace( baseObject, type, baseClass, race);
                if ( baseObject == nil) type = initial->type;
            }
/*
            initial->realObjectNumber = newShipNum = CreateAnySpaceObject( type, &v, &coord, 0, owner,
                                                specialAttributes,
                                                initial->canBuild,
                                                initial->nameResID,
                                                initial->nameStrNum,
                                                initial->spriteIDOverride);
*/
            initial->realObjectNumber = newShipNum = CreateAnySpaceObject( type, &v, &coord, 0, owner,
                                                specialAttributes,
                                                initial->spriteIDOverride);

            anObject = gSpaceObjectData.get() + newShipNum;
            initial = mGetScenarioInitial( gThisScenario, whichInitial);

            if ( anObject->attributes & kIsDestination)
            {
                anObject->destinationObject = MakeNewDestination( newShipNum, initial->canBuild,
                    initial->earning, initial->nameResID, initial->nameStrNum);

                if ( owner >= 0)
                {
                    if ( initial->canBuild[0] >= 0)
                    {
                        if ( GetAdmiralConsiderObject( owner) < 0)
                            SetAdmiralConsiderObject( owner, newShipNum);
                        if ( GetAdmiralBuildAtObject( owner) < 0)
                        {
                            SetAdmiralBuildAtObject( owner, newShipNum);
                        }
                        if ( GetAdmiralDestinationObject( owner) < 0)
                        {
                            SetAdmiralDestinationObject( owner, newShipNum, kObjectDestinationType);
                        }
                    }
                }
            }

            initial->realObjectID = anObject->id;
            if (( initial->attributes & kIsPlayerShip) &&
                ( GetAdmiralFlagship( owner) == nil))
            {
                SetAdmiralFlagship( owner, newShipNum);
                if ( owner == globals()->gPlayerAdmiralNumber)
                {
                    ResetPlayerShip( newShipNum);
                }
            }

            if ( initial->initialDestination >= 0)
            {
                // only objects controlled by an Admiral can have destinations
                if ( initial->owner > kScenarioNoOwner)
                {
                    // get the correct admiral #

                    owner = gThisScenario->player[initial->owner].admiralNumber;

                    // INITIAL IS BEING CHANGED HERE
                    initial = mGetScenarioInitial( gThisScenario, initial->initialDestination);
                    if ( initial->realObjectNumber >= 0)
                    {
                        saveDest = GetAdmiralDestinationObject( owner); // save the original dest

                        // set the admiral's dest object to the mapped initial dest object
                        SetAdmiralDestinationObject( owner,
                            initial->realObjectNumber,
                            kObjectDestinationType);

                        // now give the mapped initial object the admiral's destination

                        initial = mGetScenarioInitial( gThisScenario, whichInitial);
                        anObject = gSpaceObjectData.get() + initial->realObjectNumber;
                        specialAttributes = anObject->attributes; // preserve the attributes
                        anObject->attributes &= ~kStaticDestination; // we've got to force this off so we can set dest
                        SetObjectDestination( anObject, nil);
                        anObject->attributes = specialAttributes;

                        SetAdmiralDestinationObject( owner, saveDest, kObjectDestinationType);

                    }
                }
            }
        }/* else
        {
            initial->realObjectNumber = -1;
        }*/
    }
}

spaceObjectType *GetObjectFromInitialNumber( long initialNumber)

{
    scenarioInitialType     *initial;
    spaceObjectType         *anObject = nil;

    mGetRealObjectFromInitial( anObject, initial, initialNumber);
    return( anObject);
}

void DeclareWinner(long whichPlayer, long nextLevel, long textID) {
    if (whichPlayer < 0) {
        // if there's no winner, we want to exit immediately
        if (nextLevel >= 0) {
            globals()->gScenarioWinner.next = nextLevel;
        } else {
            globals()->gScenarioWinner.next = -1;
        }
        if (textID >= 0) {
            globals()->gScenarioWinner.text = textID;
        }
        globals()->gGameOver = 1;
    } else {
        if (globals()->gScenarioWinner.player == -1) {
            globals()->gScenarioWinner.player = whichPlayer;
            globals()->gScenarioWinner.text = textID;
            if (nextLevel >= 0) {
                globals()->gScenarioWinner.next = nextLevel;
            } else {
                globals()->gScenarioWinner.next = -1;
            }
            if (globals()->gGameOver >= 0) {
                globals()->gGameOver = -180;
            }
        }
    }
}

// GetScenarioFullScaleAndCorner:
//  This is really just for the mission briefing.  It calculates the best scale
//  at which to show the entire scenario.

void GetScenarioFullScaleAndCorner( long whichScenario, long rotation,
                            coordPointType *corner, long *scale, Rect *bounds)

{
    long            biggest, count, otherCount, mustFit;
    Point           coord, otherCoord, tempCoord;
    scenarioType    *scenario = globals()->gScenarioData.get() + whichScenario;
    scenarioInitialType     *initial;


#pragma unused( rotation)
    mustFit = bounds->bottom - bounds->top;
    if ( ( bounds->right - bounds->left) < mustFit) mustFit = bounds->right - bounds->left;

    biggest = 0;
    for ( count = 0; count < scenario->initialNum; count++)
    {
        initial = mGetScenarioInitial( scenario, count);
        if ( !(initial->attributes & kInitiallyHidden))
        {
            GetInitialCoord( initial, reinterpret_cast<coordPointType *>(&coord), globals()->gScenarioRotation);

            for ( otherCount = 0; otherCount < scenario->initialNum; otherCount++)
            {
                initial = mGetScenarioInitial( scenario, otherCount);
                GetInitialCoord( initial, reinterpret_cast<coordPointType *>(&otherCoord), globals()->gScenarioRotation);

                if ( ABS( otherCoord.h - coord.h) > biggest)
                    biggest = ABS( otherCoord.h - coord.h);
                if ( ABS( otherCoord.v - coord.v) > biggest)
                    biggest = ABS( otherCoord.v - coord.v);
            }
        }
    }

    biggest += biggest >> 2L;

    *scale = SCALE_SCALE * mustFit;
    *scale /= biggest;

    otherCoord.h = kUniversalCenter;
    otherCoord.v = kUniversalCenter;
    coord.h = kUniversalCenter;
    coord.v = kUniversalCenter;
    initial = mGetScenarioInitial( scenario, 0);
    for ( count = 0; count < scenario->initialNum; count++)
    {
        if ( !(initial->attributes & kInitiallyHidden))
        {
            GetInitialCoord( initial, reinterpret_cast<coordPointType *>(&tempCoord), globals()->gScenarioRotation);

            if ( (tempCoord.h) < coord.h)
                coord.h = tempCoord.h;
            if ( (tempCoord.v) < coord.v)
                coord.v = tempCoord.v;

            if ( (tempCoord.h) > otherCoord.h)
                otherCoord.h = tempCoord.h;
            if ( (tempCoord.v) > otherCoord.v)
                otherCoord.v = tempCoord.v;
        }
        initial++;
    }

    biggest = ( bounds->right - bounds->left);
    biggest *= SCALE_SCALE;
    biggest /= *scale;
    biggest /= 2;
    corner->h = ( coord.h + ( otherCoord.h - coord.h) / 2) - biggest;
    biggest = ( bounds->bottom - bounds->top);
    biggest *= SCALE_SCALE;
    biggest /= *scale;
    biggest /= 2;
    corner->v = ( coord.v + ( otherCoord.v - coord.v) / 2) - biggest;

}


long GetBriefPointNumber( long whichScenario)

{
    scenarioType    *scenario = globals()->gScenarioData.get() + whichScenario;

    return ( scenario->briefPointNum & kScenarioBriefMask);
}

long GetScenarioAngle( long whichScenario)
{
    scenarioType    *scenario = globals()->gScenarioData.get() + whichScenario;

    if ( scenario->briefPointNum & kScenarioAngleMask)
        return (( (scenario->briefPointNum & kScenarioAngleMask) >> kScenarioAngleShift) - 1) * 2;
    else
        return -1;
}

long GetScenarioNumberFromChapterNumber( long whichChapter)
{
    scenarioType    *aScenario = globals()->gScenarioData.get();
    long            whichScenario = 0;

    while (( whichScenario < kScenarioNum) && ( aScenario->levelNameStrNum != whichChapter))
    {
        whichScenario++;
        aScenario++;
    }

    if ( whichScenario == kScenarioNum) return ( -1);
    else return( whichScenario);
}

void GetScenarioStarMapPoint( long whichScenario, Point *starPoint)

{
    scenarioType    *scenario = globals()->gScenarioData.get() + whichScenario;

    starPoint->h = scenario->starMapH;
    starPoint->v = scenario->starMapV;
}

long GetChapterNumberFromScenarioNumber( long whichScenario)
{
    scenarioType    *aScenario = globals()->gScenarioData.get() + whichScenario;

    if ( whichScenario < 0) return ( -1);
    return( aScenario->levelNameStrNum);
}

scenarioType *GetScenarioPtrFromChapter( long whichChapter)
{
    scenarioType    *aScenario =
                        globals()->gScenarioData.get() +
                        GetScenarioNumberFromChapterNumber( whichChapter);

    if ( whichChapter < 0) return ( nil);
    else return( aScenario);

}

void GetScenarioName(long whichScenario, unsigned char* scenarioName) {
    scenarioType    *aScenario = globals()->gScenarioData.get() + whichScenario;
    GetIndString( scenarioName, kLevelNameID, aScenario->levelNameStrNum);
}

long GetScenarioNumber() {
    return globals()->scenarioNum;
}

long GetScenarioPlayerNum( long whichScenario)
{
    scenarioType    *aScenario = globals()->gScenarioData.get() + whichScenario;
    return ( aScenario->playerNum);
}

long GetScenarioPrologueID( long whichScenario)
{
    scenarioType    *aScenario = globals()->gScenarioData.get() + whichScenario;
    return ( aScenario->prologueID);
}

long GetScenarioEpilogueID( long whichScenario)
{
    scenarioType    *aScenario = globals()->gScenarioData.get() + whichScenario;
    return ( aScenario->epilogueID);
}

long GetNextScenarioChapter( long whichScenario)
{
    scenarioType    *aScenario = globals()->gScenarioData.get() + whichScenario, *newScenario = nil;
    long            newScenarioNum, newChapterNum = aScenario->levelNameStrNum + 1;

    newScenarioNum = GetScenarioNumberFromChapterNumber( newChapterNum);
    if ( newScenarioNum >= 0)
    {
        newScenario = globals()->gScenarioData.get() + newScenarioNum;
        while ( ( newScenario->playerNum <= 0) && ( newChapterNum < GetScenarioNumber()) && ( newScenario != nil))
        {
            newChapterNum++;
            newScenarioNum = GetScenarioNumberFromChapterNumber( newChapterNum);
            if ( newScenarioNum >= 0)
            {
                newScenario = globals()->gScenarioData.get() + newScenarioNum;
            } else newScenario = nil;
        }
    }
    if (( newChapterNum <= GetScenarioNumber()) && ( newScenario != nil)) return ( newScenarioNum);
    else return ( -1);

}

long GetFirstNetworkScenario( void)
{
    long            result = 1;

    // do something here to find first networkable scenario

    while ( (!ThisChapterIsNetworkable( result)) &&
        ( result <= GetScenarioNumber()))
        result++;
    if ( result > GetScenarioNumber()) return -1;
    return( result);
}

long GetNextNetworkScenario( long thisChapter)
{
    long            result = thisChapter;

    do
    {
        thisChapter++;

        if (( thisChapter <= GetScenarioNumber()) && ( ThisChapterIsNetworkable( thisChapter)))
        {
            result = thisChapter;
        }
    } while (( thisChapter <= GetScenarioNumber()) && ( !ThisChapterIsNetworkable( thisChapter))) ;

    return( result);
}

long GetPreviousNetworkScenario( long thisChapter)
{
    long            result = thisChapter;

    do
    {
        thisChapter--;

        if (( thisChapter > 0) && ( ThisChapterIsNetworkable( thisChapter)))
        {
            result = thisChapter;
        }
    } while (( thisChapter > 0) && ( !ThisChapterIsNetworkable( thisChapter)));

    return( result);
}

bool ThisChapterIsNetworkable( long whichChapter)

{
    scenarioType    *aScenario = globals()->gScenarioData.get() + GetScenarioNumberFromChapterNumber( whichChapter);
    long            i;

    if (( whichChapter < 0) || ( whichChapter > GetScenarioNumber()))
        return false;

    for ( i = 0; i < kScenarioPlayerNum; i++)
    {
        if ( aScenario->player[i].playerType == kNetworkHumanPlayer) return( true);
    }
    return( false);
}

}  // namespace antares

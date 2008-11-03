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

// Ares_External_File.c

#include "AresExternalFile.h"

#include "AmbrosiaSerial.h"
#include "AresGlobalType.h"
#include "Debug.h"
#include "Error.h"
#include "HandleHandling.h"
#include "Races.h"
#include "Scenario.h"
#include "ScenarioData.h"
#include "ScenarioMaker.h"
#include "SpaceObjectHandling.h"

extern  aresGlobalType          *gAresGlobal;
extern  Handle                  gBaseObjectData, gObjectActionData;

OSErr EF_OpenExternalFile( void)
{
    Handle  tempScenarioInfo = nil;
    short   oldResFile = CurResFile(), homeResFile;

    if ( gAresGlobal->externalFileRefNum != -1)
        CloseResFile( gAresGlobal->externalFileRefNum);

    if ( gAresGlobal->externalFileSpec.name[0] > 0)
    {
        gAresGlobal->externalFileRefNum =
            FSpOpenResFile( &gAresGlobal->externalFileSpec, fsRdPerm);

        if ( gAresGlobal->externalFileRefNum > 0)
            UseResFile( gAresGlobal->externalFileRefNum);
        else
        {
            UseResFile( oldResFile);
            gAresGlobal->externalFileRefNum = -1;
            return paramErr;
        }
    } else
    {
        gAresGlobal->externalFileRefNum = -1;
    }


    // scenario info
    tempScenarioInfo = GetResource( 'nlAG', 128);
    if ( tempScenarioInfo != nil)
    {
        homeResFile = HomeResFile( tempScenarioInfo);
        if (( homeResFile == gAresGlobal->externalFileRefNum) ||
            ( gAresGlobal->externalFileSpec.name[0] == 0))
        {
            BlockMove( *tempScenarioInfo, &gAresGlobal->scenarioFileInfo,
                sizeof( scenarioInfoType));
        } else
        {
            UseResFile( oldResFile);
            gAresGlobal->externalFileRefNum = -1;
            return resNotFound;
        }
        ReleaseResource( tempScenarioInfo);
    } else
    {
        UseResFile( oldResFile);
        gAresGlobal->externalFileRefNum = -1;
        return resNotFound;
    }

    // scenario stuff
    if ( gAresGlobal->gScenarioData != nil)
    {
        HHDeregisterHandle( &gAresGlobal->gScenarioData);
        DisposeHandle( gAresGlobal->gScenarioData);
        gAresGlobal->gScenarioData = nil;
    }
    if ( gAresGlobal->gScenarioInitialData != nil)
    {
        HHDeregisterHandle( &gAresGlobal->gScenarioInitialData);
        DisposeHandle( gAresGlobal->gScenarioInitialData);
        gAresGlobal->gScenarioInitialData = nil;
    }
    if ( gAresGlobal->gScenarioConditionData != nil)
    {
        HHDeregisterHandle( &gAresGlobal->gScenarioConditionData);
        DisposeHandle( gAresGlobal->gScenarioConditionData);
        gAresGlobal->gScenarioConditionData = nil;
    }
    if ( gAresGlobal->gScenarioBriefData != nil)
    {
        HHDeregisterHandle( &gAresGlobal->gScenarioBriefData);
        DisposeHandle( gAresGlobal->gScenarioBriefData);
        gAresGlobal->gScenarioBriefData = nil;
    }

    // races
    if ( gAresGlobal->gRaceData != nil)
    {
        HHDeregisterHandle( &gAresGlobal->gRaceData);
        DisposeHandle( gAresGlobal->gRaceData);
        gAresGlobal->gRaceData = nil;
    }

    // object stuff
    if ( gBaseObjectData != nil)
    {
        HHDeregisterHandle( &gBaseObjectData);
        DisposeHandle( gBaseObjectData);
        gBaseObjectData = nil;
    }

    if ( gObjectActionData != nil)
    {
        HHDeregisterHandle( &gObjectActionData);
        DisposeHandle( gObjectActionData);
        gObjectActionData = nil;
    }

    // load all the new stuff

    // scenario stuff
    gAresGlobal->gScenarioData = GetResource( kScenarioResType, kScenarioResID);
    if ( gAresGlobal->gScenarioData == nil)
    {
        ShowErrorAny( eQuitErr, kErrorStrID, nil, nil, nil, nil, kScenarioDataError, -1, -1, -1, __FILE__, 2);
        return( RESOURCE_ERROR);
    }

    if ( GetHandleSize( gAresGlobal->gScenarioData) <= 0)
    {
        ShowErrorAny( eQuitErr, kErrorStrID, nil, nil, nil, nil, kScenarioDataError, -1, -1, -1, __FILE__, 2);
        return( RESOURCE_ERROR);
    }

    DetachResource( gAresGlobal->gScenarioData);

    mDataHandleLockAndRegister( gAresGlobal->gScenarioData, nil, nil, CorrectThisScenarioPtr, "\pgAresGlobal->gScenarioData")

    gAresGlobal->scenarioNum = GetHandleSize( gAresGlobal->gScenarioData) /
        sizeof( scenarioType);

    gAresGlobal->gScenarioInitialData = GetResource( kScenarioInitialResType, kScenarioInitialResID);
    if ( gAresGlobal->gScenarioInitialData == nil)
    {
        ShowErrorAny( eQuitErr, kErrorStrID, nil, nil, nil, nil, kScenarioInitialDataError, -1, -1, -1, __FILE__, 3);
        return( RESOURCE_ERROR);
    }
    DetachResource( gAresGlobal->gScenarioInitialData);

    mDataHandleLockAndRegister( gAresGlobal->gScenarioInitialData, nil, nil, nil, "\pgAresGlobal->gScenarioInitialData")

    gAresGlobal->maxScenarioInitial = GetHandleSize(
        gAresGlobal->gScenarioInitialData) / sizeof( scenarioInitialType);

    gAresGlobal->gScenarioConditionData = GetResource( kScenarioConditionResType, kScenarioConditionResID);
    if ( gAresGlobal->gScenarioConditionData == nil)
    {
        ShowErrorAny( eQuitErr, kErrorStrID, nil, nil, nil, nil, kScenarioConditionDataError, -1, -1, -1, __FILE__, 4);
        return( RESOURCE_ERROR);
    }
    DetachResource( gAresGlobal->gScenarioConditionData);

    mDataHandleLockAndRegister( gAresGlobal->gScenarioConditionData, nil, nil, nil, "\pgAresGlobal->gScenarioConditionData")

    gAresGlobal->maxScenarioCondition = GetHandleSize(
        gAresGlobal->gScenarioConditionData) / sizeof( scenarioConditionType);

    gAresGlobal->gScenarioBriefData = GetResource( kScenarioBriefResType, kScenarioBriefResID);
    if ( gAresGlobal->gScenarioBriefData == nil)
    {
        ShowErrorAny( eQuitErr, kErrorStrID, nil, nil, nil, nil, kScenarioBriefDataError, -1, -1, -1, __FILE__, 5);
        return( RESOURCE_ERROR);
    }
    DetachResource( gAresGlobal->gScenarioBriefData);

    mDataHandleLockAndRegister( gAresGlobal->gScenarioBriefData, nil, nil, nil, "\pgAresGlobal->gScenarioBriefData")

    gAresGlobal->maxScenarioBrief = GetHandleSize(
        gAresGlobal->gScenarioBriefData) / sizeof( briefPointType);

    // races
    if ( gAresGlobal->gRaceData == nil)
    {
        gAresGlobal->gRaceData = GetResource( kRaceResType, kRaceResID);
        if ( gAresGlobal->gRaceData == nil)
        {
            ShowErrorAny( eQuitErr, kErrorStrID, nil, nil, nil, nil, kReadRaceDataError, -1, -1, -1, __FILE__, 1);
            return( RESOURCE_ERROR);
        }
        DetachResource( gAresGlobal->gRaceData);

        mDataHandleLockAndRegister( gAresGlobal->gRaceData, nil, nil, nil, "\pgAresGlobal->gRaceData")
    }

    // object stuff
    gBaseObjectData = GetResource( kBaseObjectResType, kBaseObjectResID);
    if ( gBaseObjectData == nil)
    {
        ShowErrorAny( eQuitErr, kErrorStrID, nil, nil, nil, nil, kReadBaseObjectDataError, -1, -1, -1, __FILE__, 2);
        return( MEMORY_ERROR);
    }

    DetachResource( gBaseObjectData);

    mDataHandleLockAndRegister( gBaseObjectData, nil, nil, nil, "\pgBaseObjectData")

    gAresGlobal->maxBaseObject = GetHandleSize( gBaseObjectData) /
        sizeof( baseObjectType);

    gObjectActionData = GetResource( kObjectActionResType, kObjectActionResID);
    if ( gObjectActionData == nil)
    {
        ShowErrorAny( eQuitErr, kErrorStrID, nil, nil, nil, nil, kReadObjectActionDataError, -1, -1, -1, __FILE__, 2);
        return( MEMORY_ERROR);
    }
    DetachResource( gObjectActionData);
    mDataHandleLockAndRegister( gObjectActionData, nil, nil, ResolveObjectActionData, "\pgObjectActionData")

    gAresGlobal->maxObjectAction = GetHandleSize( gObjectActionData)
        / sizeof( objectActionType);

    gAresGlobal->okToOpenFile = false;

    CorrectAllBaseObjectColor();
    ResetAllSpaceObjects();
    ResetActionQueueData();

    return noErr;
}

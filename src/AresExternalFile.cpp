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

#include "AresExternalFile.hpp"

#include <Resources.h>

#include "AmbrosiaSerial.h"
#include "AresGlobalType.hpp"
#include "Debug.hpp"
#include "Error.hpp"
#include "Races.hpp"
#include "Scenario.hpp"
#include "ScenarioData.hpp"
#include "ScenarioMaker.hpp"
#include "SpaceObjectHandling.hpp"

extern aresGlobalType*          gAresGlobal;
extern baseObjectType**         gBaseObjectData;
extern objectActionType**       gObjectActionData;

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
        DisposeHandle( reinterpret_cast<Handle>(gAresGlobal->gScenarioData));
        gAresGlobal->gScenarioData = nil;
    }
    if ( gAresGlobal->gScenarioInitialData != nil)
    {
        DisposeHandle( reinterpret_cast<Handle>(gAresGlobal->gScenarioInitialData));
        gAresGlobal->gScenarioInitialData = nil;
    }
    if ( gAresGlobal->gScenarioConditionData != nil)
    {
        DisposeHandle( reinterpret_cast<Handle>(gAresGlobal->gScenarioConditionData));
        gAresGlobal->gScenarioConditionData = nil;
    }
    if ( gAresGlobal->gScenarioBriefData != nil)
    {
        DisposeHandle( reinterpret_cast<Handle>(gAresGlobal->gScenarioBriefData));
        gAresGlobal->gScenarioBriefData = nil;
    }

    // races
    if ( gAresGlobal->gRaceData != nil)
    {
        DisposeHandle( reinterpret_cast<Handle>(gAresGlobal->gRaceData));
        gAresGlobal->gRaceData = nil;
    }

    // object stuff
    if ( gBaseObjectData != nil)
    {
        DisposeHandle( reinterpret_cast<Handle>(gBaseObjectData));
        gBaseObjectData = nil;
    }

    if ( gObjectActionData != nil)
    {
        DisposeHandle( reinterpret_cast<Handle>(gObjectActionData));
        gObjectActionData = nil;
    }

    // load all the new stuff

    // scenario stuff
    gAresGlobal->gScenarioData = reinterpret_cast<scenarioType**>(GetResource( kScenarioResType, kScenarioResID));
    if ( gAresGlobal->gScenarioData == nil)
    {
        ShowErrorAny( eQuitErr, kErrorStrID, nil, nil, nil, nil, kScenarioDataError, -1, -1, -1, __FILE__, 2);
        return( RESOURCE_ERROR);
    }

    if ( GetHandleSize( reinterpret_cast<Handle>(gAresGlobal->gScenarioData)) <= 0)
    {
        ShowErrorAny( eQuitErr, kErrorStrID, nil, nil, nil, nil, kScenarioDataError, -1, -1, -1, __FILE__, 2);
        return( RESOURCE_ERROR);
    }

    DetachResource( reinterpret_cast<Handle>(gAresGlobal->gScenarioData));

    gAresGlobal->scenarioNum = GetHandleSize( reinterpret_cast<Handle>(gAresGlobal->gScenarioData)) /
        sizeof( scenarioType);

    gAresGlobal->gScenarioInitialData = reinterpret_cast<scenarioInitialType**>(GetResource( kScenarioInitialResType, kScenarioInitialResID));
    if ( gAresGlobal->gScenarioInitialData == nil)
    {
        ShowErrorAny( eQuitErr, kErrorStrID, nil, nil, nil, nil, kScenarioInitialDataError, -1, -1, -1, __FILE__, 3);
        return( RESOURCE_ERROR);
    }
    DetachResource( reinterpret_cast<Handle>(gAresGlobal->gScenarioInitialData));

    gAresGlobal->maxScenarioInitial = GetHandleSize(
        reinterpret_cast<Handle>(gAresGlobal->gScenarioInitialData)) / sizeof( scenarioInitialType);

    gAresGlobal->gScenarioConditionData = reinterpret_cast<scenarioConditionType**>(GetResource( kScenarioConditionResType, kScenarioConditionResID));
    if ( gAresGlobal->gScenarioConditionData == nil)
    {
        ShowErrorAny( eQuitErr, kErrorStrID, nil, nil, nil, nil, kScenarioConditionDataError, -1, -1, -1, __FILE__, 4);
        return( RESOURCE_ERROR);
    }
    DetachResource( reinterpret_cast<Handle>(gAresGlobal->gScenarioConditionData));

    gAresGlobal->maxScenarioCondition = GetHandleSize(
        reinterpret_cast<Handle>(gAresGlobal->gScenarioConditionData)) / sizeof( scenarioConditionType);

    gAresGlobal->gScenarioBriefData = reinterpret_cast<briefPointType**>(GetResource( kScenarioBriefResType, kScenarioBriefResID));
    if ( gAresGlobal->gScenarioBriefData == nil)
    {
        ShowErrorAny( eQuitErr, kErrorStrID, nil, nil, nil, nil, kScenarioBriefDataError, -1, -1, -1, __FILE__, 5);
        return( RESOURCE_ERROR);
    }
    DetachResource( reinterpret_cast<Handle>(gAresGlobal->gScenarioBriefData));

    gAresGlobal->maxScenarioBrief = GetHandleSize(
        reinterpret_cast<Handle>(gAresGlobal->gScenarioBriefData)) / sizeof( briefPointType);

    // races
    if ( gAresGlobal->gRaceData == nil)
    {
        gAresGlobal->gRaceData = reinterpret_cast<raceType**>(GetResource( kRaceResType, kRaceResID));
        if ( gAresGlobal->gRaceData == nil)
        {
            ShowErrorAny( eQuitErr, kErrorStrID, nil, nil, nil, nil, kReadRaceDataError, -1, -1, -1, __FILE__, 1);
            return( RESOURCE_ERROR);
        }
        DetachResource( reinterpret_cast<Handle>(gAresGlobal->gRaceData));
    }

    // object stuff
    gBaseObjectData = reinterpret_cast<baseObjectType**>(GetResource( kBaseObjectResType, kBaseObjectResID));
    if ( gBaseObjectData == nil)
    {
        ShowErrorAny( eQuitErr, kErrorStrID, nil, nil, nil, nil, kReadBaseObjectDataError, -1, -1, -1, __FILE__, 2);
        return( MEMORY_ERROR);
    }

    DetachResource( reinterpret_cast<Handle>(gBaseObjectData));

    gAresGlobal->maxBaseObject = GetHandleSize( reinterpret_cast<Handle>(gBaseObjectData)) /
        sizeof( baseObjectType);

    gObjectActionData = reinterpret_cast<objectActionType**>(GetResource( kObjectActionResType, kObjectActionResID));
    if ( gObjectActionData == nil)
    {
        ShowErrorAny( eQuitErr, kErrorStrID, nil, nil, nil, nil, kReadObjectActionDataError, -1, -1, -1, __FILE__, 2);
        return( MEMORY_ERROR);
    }
    DetachResource( reinterpret_cast<Handle>(gObjectActionData));

    gAresGlobal->maxObjectAction = GetHandleSize( reinterpret_cast<Handle>(gObjectActionData))
        / sizeof( objectActionType);

    gAresGlobal->okToOpenFile = false;

    CorrectAllBaseObjectColor();
    ResetAllSpaceObjects();
    ResetActionQueueData();

    return noErr;
}

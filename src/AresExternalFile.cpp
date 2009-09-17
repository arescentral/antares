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
extern TypedHandle<baseObjectType>  gBaseObjectData;
extern TypedHandle<objectActionType>    gObjectActionData;

OSErr EF_OpenExternalFile( void)
{
    TypedHandle<scenarioInfoType> tempScenarioInfo;
    short oldResFile = CurResFile();

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
    tempScenarioInfo.load_resource('nlAG', 128);
    if (tempScenarioInfo.get() != nil) {
        gAresGlobal->scenarioFileInfo = **tempScenarioInfo;
        tempScenarioInfo.destroy();
    } else
    {
        UseResFile( oldResFile);
        gAresGlobal->externalFileRefNum = -1;
        return resNotFound;
    }

    // scenario stuff
    if (gAresGlobal->gScenarioData.get() != nil) {
        gAresGlobal->gScenarioData.destroy();
    }
    if (gAresGlobal->gScenarioInitialData.get() != nil) {
        gAresGlobal->gScenarioInitialData.destroy();
    }
    if (gAresGlobal->gScenarioConditionData.get() != nil) {
        gAresGlobal->gScenarioConditionData.destroy();
    }
    if (gAresGlobal->gScenarioBriefData.get() != nil) {
        gAresGlobal->gScenarioBriefData.destroy();
    }

    // races
    if (gAresGlobal->gRaceData.get() != nil)
    {
        gAresGlobal->gRaceData.destroy();
    }

    // object stuff
    if (gBaseObjectData.get() != nil)
    {
        gBaseObjectData.destroy();
    }

    if (gObjectActionData.get() != nil) {
        gObjectActionData.destroy();
    }

    // load all the new stuff

    // scenario stuff
    gAresGlobal->gScenarioData.load_resource('snro', kScenarioResID);
    if (gAresGlobal->gScenarioData.get() == nil) {
        ShowErrorAny( eQuitErr, kErrorStrID, nil, nil, nil, nil, kScenarioDataError, -1, -1, -1, __FILE__, 2);
        return( RESOURCE_ERROR);
    }

    if (gAresGlobal->gScenarioData.size() <= 0) {
        ShowErrorAny( eQuitErr, kErrorStrID, nil, nil, nil, nil, kScenarioDataError, -1, -1, -1, __FILE__, 2);
        return( RESOURCE_ERROR);
    }

    gAresGlobal->scenarioNum = gAresGlobal->gScenarioData.count();

    gAresGlobal->gScenarioInitialData.load_resource('snit', kScenarioInitialResID);
    if (gAresGlobal->gScenarioInitialData.get() == nil) {
        ShowErrorAny( eQuitErr, kErrorStrID, nil, nil, nil, nil, kScenarioInitialDataError, -1, -1, -1, __FILE__, 3);
        return( RESOURCE_ERROR);
    }

    gAresGlobal->maxScenarioInitial = gAresGlobal->gScenarioInitialData.size();

    gAresGlobal->gScenarioConditionData.load_resource('sncd', kScenarioConditionResID);
    if (gAresGlobal->gScenarioConditionData.get() == nil) {
        ShowErrorAny( eQuitErr, kErrorStrID, nil, nil, nil, nil, kScenarioConditionDataError, -1, -1, -1, __FILE__, 4);
        return( RESOURCE_ERROR);
    }

    gAresGlobal->maxScenarioCondition = gAresGlobal->gScenarioConditionData.size();

    gAresGlobal->gScenarioBriefData.load_resource('snbf', kScenarioBriefResID);
    if (gAresGlobal->gScenarioBriefData.get() == nil) {
        ShowErrorAny( eQuitErr, kErrorStrID, nil, nil, nil, nil, kScenarioBriefDataError, -1, -1, -1, __FILE__, 5);
        return( RESOURCE_ERROR);
    }

    gAresGlobal->maxScenarioBrief = gAresGlobal->gScenarioBriefData.count();

    // races
    if (gAresGlobal->gRaceData.get() == nil)
    {
        gAresGlobal->gRaceData.load_resource('race', kRaceResID);
        if (gAresGlobal->gRaceData.get() == nil)
        {
            ShowErrorAny( eQuitErr, kErrorStrID, nil, nil, nil, nil, kReadRaceDataError, -1, -1, -1, __FILE__, 1);
            return( RESOURCE_ERROR);
        }
    }

    // object stuff
    gBaseObjectData.load_resource('bsob', kBaseObjectResID);
    if (gBaseObjectData.get() == nil)
    {
        ShowErrorAny( eQuitErr, kErrorStrID, nil, nil, nil, nil, kReadBaseObjectDataError, -1, -1, -1, __FILE__, 2);
        return( MEMORY_ERROR);
    }

    gAresGlobal->maxBaseObject = gBaseObjectData.count();

    gObjectActionData.load_resource('obac', kObjectActionResID);
    if (gObjectActionData.get() == nil) {
        ShowErrorAny( eQuitErr, kErrorStrID, nil, nil, nil, nil, kReadObjectActionDataError, -1, -1, -1, __FILE__, 2);
        return( MEMORY_ERROR);
    }

    gAresGlobal->maxObjectAction = gObjectActionData.count();

    gAresGlobal->okToOpenFile = false;

    CorrectAllBaseObjectColor();
    ResetAllSpaceObjects();
    ResetActionQueueData();

    return noErr;
}

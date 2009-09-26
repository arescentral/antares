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

extern TypedHandle<baseObjectType>  gBaseObjectData;
extern TypedHandle<objectActionType>    gObjectActionData;

OSErr EF_OpenExternalFile( void)
{
    TypedHandle<scenarioInfoType> tempScenarioInfo;
    short oldResFile = CurResFile();

    if ( globals()->externalFileRefNum != -1)
        CloseResFile( globals()->externalFileRefNum);

    if ( globals()->externalFileSpec.name[0] > 0)
    {
        globals()->externalFileRefNum =
            FSpOpenResFile( &globals()->externalFileSpec, fsRdPerm);

        if ( globals()->externalFileRefNum > 0)
            UseResFile( globals()->externalFileRefNum);
        else
        {
            UseResFile( oldResFile);
            globals()->externalFileRefNum = -1;
            return paramErr;
        }
    } else
    {
        globals()->externalFileRefNum = -1;
    }


    // scenario info
    tempScenarioInfo.load_resource('nlAG', 128);
    if (tempScenarioInfo.get() != nil) {
        globals()->scenarioFileInfo = **tempScenarioInfo;
        tempScenarioInfo.destroy();
    } else
    {
        UseResFile( oldResFile);
        globals()->externalFileRefNum = -1;
        return resNotFound;
    }

    // scenario stuff
    if (globals()->gScenarioData.get() != nil) {
        globals()->gScenarioData.destroy();
    }
    if (globals()->gScenarioInitialData.get() != nil) {
        globals()->gScenarioInitialData.destroy();
    }
    if (globals()->gScenarioConditionData.get() != nil) {
        globals()->gScenarioConditionData.destroy();
    }
    if (globals()->gScenarioBriefData.get() != nil) {
        globals()->gScenarioBriefData.destroy();
    }

    // races
    if (globals()->gRaceData.get() != nil)
    {
        globals()->gRaceData.destroy();
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
    globals()->gScenarioData.load_resource('snro', kScenarioResID);
    if (globals()->gScenarioData.get() == nil) {
        ShowErrorAny( eQuitErr, kErrorStrID, nil, nil, nil, nil, kScenarioDataError, -1, -1, -1, __FILE__, 2);
        return( RESOURCE_ERROR);
    }

    if (globals()->gScenarioData.size() <= 0) {
        ShowErrorAny( eQuitErr, kErrorStrID, nil, nil, nil, nil, kScenarioDataError, -1, -1, -1, __FILE__, 2);
        return( RESOURCE_ERROR);
    }

    globals()->scenarioNum = globals()->gScenarioData.count();

    globals()->gScenarioInitialData.load_resource('snit', kScenarioInitialResID);
    if (globals()->gScenarioInitialData.get() == nil) {
        ShowErrorAny( eQuitErr, kErrorStrID, nil, nil, nil, nil, kScenarioInitialDataError, -1, -1, -1, __FILE__, 3);
        return( RESOURCE_ERROR);
    }

    globals()->maxScenarioInitial = globals()->gScenarioInitialData.size();

    globals()->gScenarioConditionData.load_resource('sncd', kScenarioConditionResID);
    if (globals()->gScenarioConditionData.get() == nil) {
        ShowErrorAny( eQuitErr, kErrorStrID, nil, nil, nil, nil, kScenarioConditionDataError, -1, -1, -1, __FILE__, 4);
        return( RESOURCE_ERROR);
    }

    globals()->maxScenarioCondition = globals()->gScenarioConditionData.size();

    globals()->gScenarioBriefData.load_resource('snbf', kScenarioBriefResID);
    if (globals()->gScenarioBriefData.get() == nil) {
        ShowErrorAny( eQuitErr, kErrorStrID, nil, nil, nil, nil, kScenarioBriefDataError, -1, -1, -1, __FILE__, 5);
        return( RESOURCE_ERROR);
    }

    globals()->maxScenarioBrief = globals()->gScenarioBriefData.count();

    // races
    if (globals()->gRaceData.get() == nil)
    {
        globals()->gRaceData.load_resource('race', kRaceResID);
        if (globals()->gRaceData.get() == nil)
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

    globals()->maxBaseObject = gBaseObjectData.count();

    gObjectActionData.load_resource('obac', kObjectActionResID);
    if (gObjectActionData.get() == nil) {
        ShowErrorAny( eQuitErr, kErrorStrID, nil, nil, nil, nil, kReadObjectActionDataError, -1, -1, -1, __FILE__, 2);
        return( MEMORY_ERROR);
    }

    globals()->maxObjectAction = gObjectActionData.count();

    globals()->okToOpenFile = false;

    CorrectAllBaseObjectColor();
    ResetAllSpaceObjects();
    ResetActionQueueData();

    return noErr;
}

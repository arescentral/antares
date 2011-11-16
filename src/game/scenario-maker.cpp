// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// Copyright (C) 2008-2011 Ares Central
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
// License along with this program.  If not, see
// <http://www.gnu.org/licenses/>.

#include "game/scenario-maker.hpp"

#include <vector>
#include <sfz/sfz.hpp>

#include "config/keys.hpp"
#include "data/races.hpp"
#include "data/resource.hpp"
#include "data/string-list.hpp"
#include "drawing/color.hpp"
#include "drawing/pix-table.hpp"
#include "game/admiral.hpp"
#include "game/beam.hpp"
#include "game/globals.hpp"
#include "game/instruments.hpp"
#include "game/labels.hpp"
#include "game/messages.hpp"
#include "game/minicomputer.hpp"
#include "game/motion.hpp"
#include "game/non-player-ship.hpp"
#include "game/player-ship.hpp"
#include "game/space-object.hpp"
#include "game/starfield.hpp"
#include "lang/casts.hpp"
#include "math/macros.hpp"
#include "math/random.hpp"
#include "math/rotation.hpp"
#include "math/units.hpp"
#include "ui/interface-handling.hpp"

using sfz::Bytes;
using sfz::BytesSlice;
using sfz::Exception;
using sfz::PrintTarget;
using sfz::String;
using sfz::StringSlice;
using sfz::range;
using sfz::read;
using sfz::scoped_array;
using std::vector;
namespace macroman = sfz::macroman;

namespace antares {

namespace {

const int16_t kScenarioResID            = 500;
const int16_t kScenarioInitialResID     = 500;
const int16_t kScenarioConditionResID   = 500;
const int16_t kScenarioBriefResID       = 500;

const bool NETWORK_ON = false;

const int32_t kOwnerMayChangeFlag   = 0x80000000;
const int32_t kAnyOwnerColorFlag    = 0x0000ffff;

const int16_t kLevelNameID = 4600;

vector<Scenario> gScenarioData;
vector<Scenario::InitialObject> gScenarioInitialData;
vector<Scenario::Condition> gScenarioConditionData;
vector<Scenario::BriefPoint> gScenarioBriefData;
int32_t gScenarioRotation = 0;
int32_t gAdmiralNumbers[kMaxPlayerNum];

void CheckActionMedia(int32_t whichAction, int32_t actionNum, uint8_t color);
void AddBaseObjectActionMedia(int32_t whichBase, int32_t whichType, uint8_t color);
void AddActionMedia(objectActionType *action, uint8_t color);

void SetAllBaseObjectsUnchecked() {
    baseObjectType  *aBase = gBaseObjectData.get();
    long            count;

    for ( count = 0; count < globals()->maxBaseObject; count++)
    {
        aBase->internalFlags = 0;
        aBase++;
    }
}

void CheckBaseObjectMedia(baseObjectType *aBase, uint8_t color) {
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

void CheckActionMedia(int32_t whichAction, int32_t actionNum, uint8_t color) {
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
                        for ( count = 0; count < globals()->maxBaseObject; count++)
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

void AddBaseObjectMedia(int32_t whichBase, uint8_t color) {
    baseObjectType      *aBase = mGetBaseObjectPtr( whichBase);

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

void mGetActionFromBaseTypeNum(
        objectActionType*& mactPtr, baseObjectType* mbaseObjPtr, long mactionType,
        long mactionNum) {
    if ( (mactionType) == kDestroyActionType)
    {
        if ( mactionNum >= ((mbaseObjPtr)->destroyActionNum & kDestroyActionNotMask)) mactPtr = NULL;
        else mactPtr = gObjectActionData.get() + (mbaseObjPtr)->destroyAction + implicit_cast<long>(mactionNum);
    } else if ( (mactionType) == kExpireActionType)
    {
        if ( mactionNum >= ((mbaseObjPtr)->expireActionNum  & kDestroyActionNotMask)) mactPtr = NULL;
        else mactPtr = gObjectActionData.get() + (mbaseObjPtr)->expireAction + implicit_cast<long>(mactionNum);
    } else if ( (mactionType) == kCreateActionType)
    {
        if ( mactionNum >= (mbaseObjPtr)->createActionNum) mactPtr = NULL;
        else mactPtr = gObjectActionData.get() + (mbaseObjPtr)->createAction + implicit_cast<long>(mactionNum);
    } else if ( (mactionType) == kCollideActionType)
    {
        if ( mactionNum >= (mbaseObjPtr)->collideActionNum) mactPtr = NULL;
        else mactPtr = gObjectActionData.get() + (mbaseObjPtr)->collideAction + implicit_cast<long>(mactionNum);
    } else if ( (mactionType) == kActivateActionType)
    {
        if ( mactionNum >= ((mbaseObjPtr)->activateActionNum & kPeriodicActionNotMask)) mactPtr = NULL;
        else mactPtr = gObjectActionData.get() + (mbaseObjPtr)->activateAction + implicit_cast<long>(mactionNum);
    } else if ( (mactionType) == kArriveActionType)
    {
        if ( mactionNum >= (mbaseObjPtr)->arriveActionNum) mactPtr = NULL;
        else mactPtr = gObjectActionData.get() + (mbaseObjPtr)->arriveAction + implicit_cast<long>(mactionNum);
    } else mactPtr = NULL;
}

void AddBaseObjectActionMedia(int32_t whichBase, int32_t whichType, uint8_t color) {
    baseObjectType      *baseObject = NULL;
    long                count = 0;
    objectActionType    *action = NULL;

    do
    {
        baseObject = mGetBaseObjectPtr( whichBase);
        mGetActionFromBaseTypeNum( action, baseObject, whichType, count);

        AddActionMedia( action, color);
        count++;
    } while ( action != NULL);
}

void AddActionMedia(objectActionType *action, uint8_t color) {
    baseObjectType      *baseObject = NULL;
    long                count = 0, l1, l2;
    bool             OKtoExecute;

    if ( action != NULL)
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
                    AddSound( count); // moves mem
                }
                break;

            case kNoAction:
                action = NULL;   // get us out of loop
                break;

            case kAlter:
                switch( action->argument.alterObject.alterType)
                {
                    case kAlterBaseType:
                        AddBaseObjectMedia( action->argument.alterObject.minimum, color);
                        break;

                    case kAlterOwner:
                        baseObject = gBaseObjectData.get();
                        for ( count = 0; count < globals()->maxBaseObject; count++)
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

void GetInitialCoord(Scenario::InitialObject *initial, coordPointType *coord, int32_t rotation) {
    int32_t lcos, lsin, lscrap;

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

}  // namespace

const Scenario* gThisScenario = NULL;

Scenario* mGetScenario(int32_t num) {
    return &gScenarioData[num];
}

int32_t mGetRealAdmiralNum(int32_t mplayernum) {
    return gAdmiralNumbers[mplayernum];
}

Scenario::InitialObject* Scenario::initial(size_t at) const {
    return &gScenarioInitialData[initialFirst + at];
}

Scenario::Condition* Scenario::condition(size_t at) const {
    return &gScenarioConditionData[conditionFirst + at];
}

Scenario::BriefPoint* Scenario::brief_point(size_t at) const {
    return &gScenarioBriefData[briefPointFirst + at];
}

size_t Scenario::brief_point_size() const {
    return briefPointNum & kScenarioBriefMask;
}

int32_t Scenario::angle() const {
    if (briefPointNum & kScenarioAngleMask) {
        return (((briefPointNum & kScenarioAngleMask) >> kScenarioAngleShift) - 1) * 2;
    } else {
        return -1;
    }
}

Point Scenario::star_map_point() const {
    return Point(starMapH, starMapV);
}

int32_t Scenario::chapter_number() const {
    return levelNameStrNum;
}

ScenarioName Scenario::name() const {
    ScenarioName name = {levelNameStrNum};
    return name;
}

void print_to(PrintTarget out, ScenarioName name) {
    StringList strings(kLevelNameID);
    print(out, strings.at(name.string_id - 1));
}

int32_t Scenario::prologue_id() const {
    return prologueID;
}

int32_t Scenario::epilogue_id() const {
    return epilogueID;
}

void Scenario::Condition::set_true_yet(bool state) {
    if (state) {
        flags |= kHasBeenTrue;
    } else {
        flags &= ~kHasBeenTrue;
    }
}

bool Scenario::Condition::true_yet() const {
    return flags & kHasBeenTrue;
}

void ScenarioMakerInit() {
    {
        Resource rsrc("scenario-info", "nlAG", 128);
        BytesSlice in(rsrc.data());
        read(in, globals()->scenarioFileInfo);
        if (!in.empty()) {
            throw Exception("didn't consume all of scenario file info data");
        }
    }

    {
        gScenarioData.clear();
        Resource rsrc("scenarios", "snro", kScenarioResID);
        BytesSlice in(rsrc.data());
        while (!in.empty()) {
            Scenario scenario;
            read(in, scenario);
            gScenarioData.push_back(scenario);
        }
        globals()->scenarioNum = gScenarioData.size();
    }

    {
        gScenarioInitialData.clear();
        Resource rsrc("scenario-initial-objects", "snit", kScenarioInitialResID);
        BytesSlice in(rsrc.data());
        while (!in.empty()) {
            Scenario::InitialObject initial;
            read(in, initial);
            gScenarioInitialData.push_back(initial);
        }
        globals()->maxScenarioInitial = gScenarioInitialData.size();
    }

    {
        gScenarioConditionData.clear();
        Resource rsrc("scenario-conditions", "sncd", kScenarioConditionResID);
        BytesSlice in(rsrc.data());
        while (!in.empty()) {
            Scenario::Condition condition;
            read(in, condition);
            gScenarioConditionData.push_back(condition);
        }
        globals()->maxScenarioCondition = gScenarioConditionData.size();
    }

    {
        gScenarioBriefData.clear();
        Resource rsrc("scenario-briefing-points", "snbf", kScenarioBriefResID);
        BytesSlice in(rsrc.data());
        while (!in.empty()) {
            Scenario::BriefPoint brief_point;
            read(in, brief_point);
            gScenarioBriefData.push_back(brief_point);
        }
        globals()->maxScenarioBrief = gScenarioBriefData.size();
    }

    InitRaces();
}

bool ConstructScenario(const Scenario* scenario) {
    long                count, owner, type, specialAttributes,
                        newShipNum, c2, c3, baseClass, race;
    coordPointType      coord;
    fixedPointType      v = {0, 0};
    baseObjectType      *baseObject;
    spaceObjectType     *anObject;
    Scenario::Condition     *condition;
    Scenario::InitialObject     *initial;
    objectActionType        *action;
    Rect                    loadingRect;
    long                    stepNumber, currentStep = 0;

    v.h = 0; v.v = 0;

    ResetAllSpaceObjects();
    ResetActionQueueData();
    ResetBeams();
    ResetAllSprites();
    ResetAllLabels();
    ResetInstruments();
    ResetAllAdmirals();
    ResetAllDestObjectData();
    ResetMotionGlobals();
    gAbsoluteScale = kTimesTwoScale;
    globals()->gSynchValue = 0;

    if (NETWORK_ON)
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
    }

    gThisScenario = scenario;

    count = gThisScenario->angle();
    if ( count < 0)
        gScenarioRotation = RandomSeeded( ROT_POS, &gRandomSeed, 'scm0', -1);
    else
        gScenarioRotation = count;

    globals()->gScenarioWinner.player = -1;
    globals()->gScenarioWinner.next = -1;
    globals()->gScenarioWinner.text = -1;

    SetMiniScreenStatusStrList( gThisScenario->scoreStringResID);

    // *** BEGIN INIT ADMIRALS ***
    for( count = 0; count < kMaxPlayerNum; count++)
    {
        gAdmiralNumbers[count] = -1;
    }

    for ( count = 0; count < gThisScenario->playerNum; count++)
    {
        if (NETWORK_ON)
        {
#ifdef NETSPROCKET_AVAILABLE
            if ( gThisScenario->player[count].playerType == kComputerPlayer)
            {
                gAdmiralNumbers[count] = MakeNewAdmiral(
                        kNoShip, kNoDestinationObject, kNoDestinationType,
                        kAIsComputer, gThisScenario->player[count].playerRace,
                        gThisScenario->player[count].nameResID,
                        gThisScenario->player[count].nameStrNum,
                        gThisScenario->player[count].earningPower);
                PayAdmiral(gAdmiralNumbers[count], mLongToFixed( 5000));
            } else if ( GetPlayerRace( count) >= 0)
            {
                if ( count == globals()->gPlayerAdmiralNumber)
                {
                    admiralType = 0;
                } else
                {
                    admiralType = kAIsRemote;
                }
                gAdmiralNumbers[count] = MakeNewAdmiral(
                        kNoShip, kNoDestinationObject,
                        kNoDestinationType, kAIsHuman | admiralType,
                        GetRaceIDFromNum( GetPlayerRace( count)),
                        gThisScenario->player[count].nameResID,
                        gThisScenario->player[count].nameStrNum,
                        gThisScenario->player[count].earningPower);
                PayAdmiral(gAdmiralNumbers[count], mLongToFixed(5000));
                SetAdmiralColor(gAdmiralNumbers[count], GetPlayerColor(count));
                SetAdmiralName(gAdmiralNumbers[count], (anyCharType *)GetPlayerName( count));
            }
#endif NETSPROCKET_AVAILABLE
        } else
        {
            if ( gThisScenario->player[count].playerType == kSingleHumanPlayer)
            {
                gAdmiralNumbers[count] = MakeNewAdmiral(
                        kNoShip, kNoDestinationObject, kNoDestinationType,
                        kAIsHuman, gThisScenario->player[count].playerRace,
                        gThisScenario->player[count].nameResID,
                        gThisScenario->player[count].nameStrNum,
                        gThisScenario->player[count].earningPower);
                PayAdmiral(gAdmiralNumbers[count], mLongToFixed(5000));
                globals()->gPlayerAdmiralNumber = gAdmiralNumbers[count];
            } else
            {
                gAdmiralNumbers[count] = MakeNewAdmiral(
                        kNoShip, kNoDestinationObject, kNoDestinationType,
                        kAIsComputer, gThisScenario->player[count].playerRace,
                        gThisScenario->player[count].nameResID,
                        gThisScenario->player[count].nameStrNum,
                        gThisScenario->player[count].earningPower);
                PayAdmiral(gAdmiralNumbers[count], mLongToFixed(5000));
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
    StringList strings(kLevelNameID);
    StringSlice level_name = strings.at(gThisScenario->levelNameStrNum - 1);
    DoLoadingInterface(&loadingRect, level_name);
    UpdateLoadingInterface( currentStep, stepNumber, &loadingRect);

    // for each initial object

    if (globals()->scenarioFileInfo.energyBlobID < 0) {
        throw Exception("No energy blob defined");
    }
    if (globals()->scenarioFileInfo.warpInFlareID < 0) {
        throw Exception("No warp in flare defined");
    }
    if (globals()->scenarioFileInfo.warpOutFlareID < 0) {
        throw Exception("No warp out flare defined");
    }
    if (globals()->scenarioFileInfo.playerBodyID < 0) {
        throw Exception("No player body defined");
    }

    for ( count = 0; count < gThisScenario->playerNum; count++)
    {
        baseObject = mGetBaseObjectPtr( globals()->scenarioFileInfo.energyBlobID);
        if ( baseObject != NULL)
            CheckBaseObjectMedia( baseObject, 0);   // special case; always neutral
        baseObject = mGetBaseObjectPtr( globals()->scenarioFileInfo.warpInFlareID);
        if ( baseObject != NULL)
            CheckBaseObjectMedia( baseObject, 0); // special case; always neutral
        baseObject = mGetBaseObjectPtr( globals()->scenarioFileInfo.warpOutFlareID);
        if ( baseObject != NULL)
            CheckBaseObjectMedia( baseObject, 0); // special case; always neutral
        baseObject = mGetBaseObjectPtr( globals()->scenarioFileInfo.playerBodyID);
        if ( baseObject != NULL)
            CheckBaseObjectMedia( baseObject, GetAdmiralColor( count));
    }

    for ( count = 0; count < gThisScenario->initialNum; count++)
    {
        initial = gThisScenario->initial(count);
        currentStep++;
        UpdateLoadingInterface( currentStep, stepNumber, &loadingRect);
        // get the base object equiv
        baseObject = mGetBaseObjectPtr( initial->type);
        if (NETWORK_ON && (GetAdmiralRace( initial->owner) >= 0) &&
            ( !(initial->attributes & kFixedRace)))
        {
            baseClass = baseObject->baseClass;
            race = GetAdmiralRace( initial->owner);
            mGetBaseObjectFromClassRace( baseObject, newShipNum, baseClass, race);
            if ( baseObject == NULL) baseObject = mGetBaseObjectPtr( initial->type);
        }
        // check the media for this object
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
                    mGetBaseObjectFromClassRace( baseObject, newShipNum, initial->canBuild[c2], GetAdmiralRace( c3));
                    if ( baseObject != NULL)
                        CheckBaseObjectMedia( baseObject, GetAdmiralColor( c3));
                }
            }
        }
    }

    // check media for all condition actions
    condition = gThisScenario->condition(0);
    for ( count = 0; count < gThisScenario->conditionNum; count++)
    {
        CheckActionMedia( condition->startVerb, condition->verbNum, 0);
        condition = gThisScenario->condition(count);
    }

    // make sure we check things whose owner may change
    for ( count = 0; count < globals()->maxBaseObject; count++)
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

    RemoveAllUnusedSounds();
    RemoveAllUnusedPixTables();

    for ( count = 0; count < gThisScenario->playerNum; count++)
    {
        baseObject = mGetBaseObjectPtr( globals()->scenarioFileInfo.energyBlobID);
        if ( baseObject != NULL)
        {
            AddBaseObjectMedia( globals()->scenarioFileInfo.energyBlobID, 0); // special case; always neutral
        }
        baseObject = mGetBaseObjectPtr( globals()->scenarioFileInfo.warpInFlareID);
        if ( baseObject != NULL)
        {
            AddBaseObjectMedia( globals()->scenarioFileInfo.warpInFlareID, 0); // special case; always neutral
        }
        baseObject = mGetBaseObjectPtr( globals()->scenarioFileInfo.warpOutFlareID);
        if ( baseObject != NULL)
        {
            AddBaseObjectMedia( globals()->scenarioFileInfo.warpOutFlareID, 0); // special case; always neutral
        }
        baseObject = mGetBaseObjectPtr( globals()->scenarioFileInfo.playerBodyID);
        if ( baseObject != NULL)
        {
            AddBaseObjectMedia( globals()->scenarioFileInfo.playerBodyID, GetAdmiralColor( count));
        }
    }

    for ( count = 0; count < gThisScenario->initialNum; count++)
    {
        currentStep++;
        UpdateLoadingInterface( currentStep, stepNumber, &loadingRect);

        initial = gThisScenario->initial(count);

        // get the base object equiv
        type = initial->type;
        baseObject = mGetBaseObjectPtr( type);
        if (NETWORK_ON && (GetAdmiralRace( initial->owner) >= 0) &&
            ( !(initial->attributes & kFixedRace)))
        {
            baseClass = baseObject->baseClass;
            race = GetAdmiralRace( initial->owner);
            mGetBaseObjectFromClassRace( baseObject, type, baseClass, race);
            if ( baseObject == NULL)
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
        initial = gThisScenario->initial(count);
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
        }

        // check any objects this object can build
        for ( c2 = 0; c2 < kMaxTypeBaseCanBuild; c2++)
        {
            initial = gThisScenario->initial(count);
            if ( initial->canBuild[c2] != kNoClass)
            {
                // check for each player
                for ( c3 = 0; c3 < gThisScenario->playerNum; c3++)
                {
                    initial = gThisScenario->initial(count);
                    baseObject = mGetBaseObjectPtr( type);
                    mGetBaseObjectFromClassRace( baseObject, newShipNum, initial->canBuild[c2], GetAdmiralRace( c3));
                    if ( baseObject != NULL)
                    {
                        AddBaseObjectMedia( newShipNum, GetAdmiralColor( c3));
                    }
                }
            }
        }
    }

    // add media for all condition actions
    condition = gThisScenario->condition(0);
    for ( count = 0; count < gThisScenario->conditionNum; count++)
    {
        condition = gThisScenario->condition(count);
        action = gObjectActionData.get() + condition->startVerb;
        for ( c2 = 0; c2 < condition->verbNum; c2++)
        {
            condition = gThisScenario->condition(count);
            action = gObjectActionData.get() + condition->startVerb + c2;
            AddActionMedia( action, 0);
        }
    }

    // make sure we check things whose owner may change
    for ( count = 0; count < globals()->maxBaseObject; count++)
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
    condition = gThisScenario->condition(0);
    for ( count = 0; count < gThisScenario->conditionNum; count++)
    {
        if ( condition->flags & kInitiallyTrue)
            condition->flags |= kHasBeenTrue;
        else
            condition->flags &= ~kHasBeenTrue;
        condition++;
    }

    initial = gThisScenario->initial(0);
    for ( count = 0; count < gThisScenario->initialNum; count++)
    {
        currentStep++;
        UpdateLoadingInterface( currentStep, stepNumber, &loadingRect);

        if ( !(initial->attributes & kInitiallyHidden))
        {
            GetInitialCoord( initial, &coord, gScenarioRotation);

            if ( initial->owner > kScenarioNoOwner)
                owner = gAdmiralNumbers[initial->owner];
            else owner = kScenarioNoOwner;

            specialAttributes = initial->attributes & ( ~kInitialAttributesMask);
            if ( initial->attributes & kIsPlayerShip)
            {
                if ( GetAdmiralFlagship( owner) == NULL)
                {
                    if ( owner == globals()->gPlayerAdmiralNumber)
                    {
                        specialAttributes |= kIsHumanControlled;
                    } else
                    {
                        if (NETWORK_ON)
                        {
                            specialAttributes |= kIsRemote;
                        } else
                        {
                            specialAttributes &= ~kIsPlayerShip;
                        }
                    }
                } else
                {
                    specialAttributes &= ~kIsPlayerShip;
                }
            }


            type = initial->type;
            if (NETWORK_ON && (GetAdmiralRace( initial->owner) >= 0) &&
                ( !(initial->attributes & kFixedRace)))
            {
                baseObject = mGetBaseObjectPtr( type);
                baseClass = baseObject->baseClass;
                race = GetAdmiralRace( initial->owner);
                mGetBaseObjectFromClassRace( baseObject, type, baseClass, race);
                if ( baseObject == NULL)
                {
                    baseObject = mGetBaseObjectPtr( initial->type);
                    type = initial->type;
                }
            }
            initial->realObjectNumber = newShipNum = CreateAnySpaceObject( type, &v, &coord,
                                                gScenarioRotation,
                                                owner,
                                                specialAttributes,
                                                initial->spriteIDOverride);

            anObject = gSpaceObjectData.get() + newShipNum;
            if ( anObject->attributes & kIsDestination)
            {
                anObject->destinationObject = MakeNewDestination( newShipNum, initial->canBuild,
                    initial->earning, initial->nameResID, initial->nameStrNum);
            }
            initial->realObjectID = anObject->id;
            if (( initial->attributes & kIsPlayerShip) && ( GetAdmiralFlagship( owner)
                == NULL))
            {
                SetAdmiralFlagship( owner, newShipNum);
                if ( owner == globals()->gPlayerAdmiralNumber)
                {
                    ResetPlayerShip( newShipNum);
                } else
                {
                    if (NETWORK_ON)
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

        initial = gThisScenario->initial(count);
        // if the initial object has an initial destination
        if (( initial->realObjectNumber >= 0) && ( initial->initialDestination >= 0))
        {
            // only objects controlled by an Admiral can have destinations
            if ( initial->owner > kScenarioNoOwner)
            {
                // get the correct admiral #

                owner = gAdmiralNumbers[initial->owner];
                initial = gThisScenario->initial(initial->initialDestination);

                // set the admiral's dest object to the mapped initial dest object
                SetAdmiralDestinationObject( owner,
                    initial->realObjectNumber,
                    kObjectDestinationType);

                // now give the mapped initial object the admiral's destination

                initial = gThisScenario->initial(count);
                anObject = gSpaceObjectData.get() + initial->realObjectNumber;
                specialAttributes = anObject->attributes; // preserve the attributes
                anObject->attributes &= ~kStaticDestination; // we've got to force this off so we can set dest
                SetObjectDestination( anObject, NULL);
                anObject->attributes = specialAttributes;
            }
        }
    }

    // set up all the admiral's destination objects
    RecalcAllAdmiralBuildData();

    if (NETWORK_ON)
    {
        for ( c2 = 0; c2 < gThisScenario->playerNum; c2++)
        {
            if ( GetAdmiralFlagship( c2) == NULL)
            {
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
                    SetAdmiralFlagship( c2, count);
                    anObject->attributes |= kIsPlayerShip;
                    if ( c2 != globals()->gPlayerAdmiralNumber)
                    {
                        anObject->attributes |= kIsRemote;
                    } else
                    {
                        anObject->attributes |= kIsHumanControlled;
                        ResetPlayerShip( count);
                    }
                }
            }
        }
    }
    ClearMessage();

    c2 = 0;
    const int64_t start_time = (gThisScenario->startTime & kScenario_StartTimeMask) * kScenarioTimeMultiple;
    for (int64_t t = 0; t < start_time; t += kTimeUnit) {
        globals()->gGameTime = usecs_to_ticks(t);
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
        if ((t % kScenarioTimeMultiple) == 0)
        {
            currentStep++;
            UpdateLoadingInterface( currentStep, stepNumber, &loadingRect);
        }
    }
    globals()->gGameTime = usecs_to_ticks(start_time);

    return( true);
}

void CheckScenarioConditions(int32_t timePass) {
    Scenario::Condition     *condition = NULL;
    spaceObjectType         *sObject = NULL, *dObject = NULL;
    long                    i, l, difference;
    unsigned long           distance, dcalc;
    Point                   offset(0, 0);
    bool                 conditionTrue = false;

#pragma unused( timePass)

        condition = gThisScenario->condition(0);
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
                        sObject = GetObjectFromInitialNumber(
                                condition->conditionArgument.longValue);
                        if (sObject == NULL) {
                            conditionTrue = true;
                        }
                        break;

                    case kOwnerCondition:
                        sObject = GetObjectFromInitialNumber(condition->subjectObject);
                        if (sObject != NULL) {
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
                        }
                        break;

                    case kProximityCondition:
                        sObject = GetObjectFromInitialNumber(condition->subjectObject);
                        if (sObject != NULL) {
                            dObject = GetObjectFromInitialNumber(condition->directObject);
                            if (dObject != NULL) {
                                difference = ABS<int>( sObject->location.h - dObject->location.h);
                                dcalc = difference;
                                difference =  ABS<int>( sObject->location.v - dObject->location.v);
                                distance = difference;

                                if (( dcalc < kMaximumRelevantDistance) && ( distance < kMaximumRelevantDistance))
                                {
                                    distance = distance * distance + dcalc * dcalc;
                                    if ( distance < condition->conditionArgument.unsignedLongValue)
                                    {
                                        conditionTrue = true;
                                    } else
                                    {
                                    }
                                }
                            }
                        }
                        break;

                    case kDistanceGreaterCondition:
                        sObject = GetObjectFromInitialNumber(condition->subjectObject);
                        if (sObject != NULL) {
                            dObject = GetObjectFromInitialNumber(condition->directObject);
                            if (dObject != NULL) {
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
                        sObject = GetObjectFromInitialNumber(condition->subjectObject);
                        if (sObject == NULL) {
                            conditionTrue = true;
                        } else if ( sObject->health <= ( sObject->baseType->health >> 1))
                        {
                            conditionTrue = true;
                        }
                        break;

                    case kIsAuxiliaryObject:
                        sObject = GetObjectFromInitialNumber(condition->subjectObject);
                        if (sObject != NULL) {
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
                        sObject = GetObjectFromInitialNumber(condition->subjectObject);
                        if (sObject != NULL) {
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
                        sObject = GetObjectFromInitialNumber(condition->subjectObject);
                        if (sObject != NULL) {
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

                            destBalanceType     *buildAtObject = NULL;

                            buildAtObject = mGetDestObjectBalancePtr( GetAdmiralBuildAtObject( globals()->gPlayerAdmiralNumber));
                            if ( buildAtObject != NULL)
                            {
                                if ( buildAtObject->totalBuildTime > 0)
                                {
                                    conditionTrue = true;
                                }
                            }
                        }
                        break;

                    case kDirectIsSubjectTarget:
                        sObject = GetObjectFromInitialNumber(condition->subjectObject);
                        dObject = GetObjectFromInitialNumber(condition->directObject);
                        if ((sObject != NULL) && (dObject != NULL)) {
                            if ( sObject->destObjectID == dObject->id)
                                conditionTrue = true;
                        }
                        break;

                    case kSubjectIsPlayerCondition:
                        sObject = GetObjectFromInitialNumber(condition->subjectObject);
                        if (sObject != NULL) {
                            if ( sObject->entryNumber == globals()->gPlayerShipNumber)
                                conditionTrue = true;
                        }
                        break;

                    default:
                        break;

                }
                if ( conditionTrue)
                {
                    condition->flags |= kHasBeenTrue;
                    sObject = GetObjectFromInitialNumber(condition->subjectObject);
                    dObject = GetObjectFromInitialNumber(condition->directObject);
                    ExecuteObjectActions( condition->startVerb, condition->verbNum,
                        sObject, dObject, &offset, true);
                }
            }
            condition++;
        }
}

int32_t GetRealAdmiralNumber(int32_t whichAdmiral) {
    long result;

    result = mGetRealAdmiralNum( whichAdmiral);
    return( result);
}

void UnhideInitialObject(int32_t whichInitial) {
    Scenario::InitialObject     *initial;
    spaceObjectType         *anObject = NULL;
    coordPointType          coord;
    fixedPointType          v = {0, 0};
    unsigned long           specialAttributes;
    long                    newShipNum, owner, type, saveDest, baseClass, race;
    baseObjectType          *baseObject;

    v.h = 0;
    v.v = 0;
    initial = gThisScenario->initial(whichInitial);
    anObject = GetObjectFromInitialNumber(whichInitial);
    if (anObject == NULL) {
        GetInitialCoord( initial, &coord, gScenarioRotation);

        if ( initial->owner > kScenarioNoOwner)
            owner = gAdmiralNumbers[initial->owner];
        else owner = kScenarioNoOwner;

        specialAttributes = initial->attributes & ( ~kInitialAttributesMask);
        if ( initial->attributes & kIsPlayerShip)
        {
            if ( GetAdmiralFlagship( owner) == NULL)
            {
                if ( owner == globals()->gPlayerAdmiralNumber)
                {
                    specialAttributes |= kIsHumanControlled;
                } else
                {
                    if (NETWORK_ON)
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
        if (NETWORK_ON && (GetAdmiralRace( initial->owner) >= 0) &&
            ( !(initial->attributes & kFixedRace)))
        {
            baseObject = mGetBaseObjectPtr( type);
            baseClass = baseObject->baseClass;
            race = GetAdmiralRace( initial->owner);
            mGetBaseObjectFromClassRace( baseObject, type, baseClass, race);
            if ( baseObject == NULL) type = initial->type;
        }
        initial->realObjectNumber = newShipNum = CreateAnySpaceObject( type, &v, &coord, 0, owner,
                                            specialAttributes,
                                            initial->spriteIDOverride);

        anObject = gSpaceObjectData.get() + newShipNum;
        initial = gThisScenario->initial(whichInitial);

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
            ( GetAdmiralFlagship( owner) == NULL))
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

                owner = gAdmiralNumbers[initial->owner];

                // INITIAL IS BEING CHANGED HERE
                initial = gThisScenario->initial(initial->initialDestination);
                if ( initial->realObjectNumber >= 0)
                {
                    saveDest = GetAdmiralDestinationObject( owner); // save the original dest

                    // set the admiral's dest object to the mapped initial dest object
                    SetAdmiralDestinationObject( owner,
                        initial->realObjectNumber,
                        kObjectDestinationType);

                    // now give the mapped initial object the admiral's destination

                    initial = gThisScenario->initial(whichInitial);
                    anObject = gSpaceObjectData.get() + initial->realObjectNumber;
                    specialAttributes = anObject->attributes; // preserve the attributes
                    anObject->attributes &= ~kStaticDestination; // we've got to force this off so we can set dest
                    SetObjectDestination( anObject, NULL);
                    anObject->attributes = specialAttributes;

                    SetAdmiralDestinationObject( owner, saveDest, kObjectDestinationType);

                }
            }
        }
    }
}

spaceObjectType *GetObjectFromInitialNumber(int32_t initialNumber) {
    if (initialNumber >= 0) {
        Scenario::InitialObject* initial = gThisScenario->initial(initialNumber);
        if (initial->realObjectNumber >= 0) {
            spaceObjectType& object = gSpaceObjectData[initial->realObjectNumber];
            if ((object.id != initial->realObjectID) || (object.active != kObjectInUse)) {
                return NULL;
            }
            return &object;
        }
        return NULL;
    } else if (initialNumber == -2) {
        spaceObjectType& object = gSpaceObjectData[globals()->gPlayerShipNumber];
        if ((!object.active) || (!(object.attributes & kCanThink))) {
            return NULL;
        }
        return &object;
    }
    return NULL;
}

void DeclareWinner(int32_t whichPlayer, int32_t nextLevel, int32_t textID) {
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

void GetScenarioFullScaleAndCorner(
        const Scenario* scenario, int32_t rotation, coordPointType *corner, int32_t *scale,
        Rect *bounds) {
    long            biggest, count, otherCount, mustFit;
    Point           coord, otherCoord, tempCoord;
    Scenario::InitialObject     *initial;


#pragma unused( rotation)
    mustFit = bounds->bottom - bounds->top;
    if ( ( bounds->right - bounds->left) < mustFit) mustFit = bounds->right - bounds->left;

    biggest = 0;
    for ( count = 0; count < scenario->initialNum; count++)
    {
        initial = scenario->initial(count);
        if ( !(initial->attributes & kInitiallyHidden))
        {
            GetInitialCoord( initial, reinterpret_cast<coordPointType *>(&coord), gScenarioRotation);

            for ( otherCount = 0; otherCount < scenario->initialNum; otherCount++)
            {
                initial = scenario->initial(otherCount);
                GetInitialCoord( initial, reinterpret_cast<coordPointType *>(&otherCoord), gScenarioRotation);

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
    initial = scenario->initial(0);
    for ( count = 0; count < scenario->initialNum; count++)
    {
        if ( !(initial->attributes & kInitiallyHidden))
        {
            GetInitialCoord( initial, reinterpret_cast<coordPointType *>(&tempCoord), gScenarioRotation);

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

const Scenario* GetScenarioPtrFromChapter(int32_t chapter) {
    SFZ_FOREACH(const Scenario& scenario, gScenarioData, {
        if (scenario.chapter_number() == chapter) {
            return &scenario;
        }
    });
    return NULL;
}

coordPointType Translate_Coord_To_Scenario_Rotation(int32_t h, int32_t v) {
    int32_t lcos, lsin, lscrap, angle = gScenarioRotation;
    coordPointType coord;

    mAddAngle( angle, 90);
    GetRotPoint(&lcos, &lsin, angle);
    lcos = -lcos;
    lsin = -lsin;

    lscrap = mMultiplyFixed(h, lcos);
    lscrap -= mMultiplyFixed(v, lsin);
    coord.h = kUniversalCenter;
    coord.h += lscrap;

    lscrap = mMultiplyFixed(h, lsin);
    lscrap += mMultiplyFixed(v, lcos);
    coord.v = kUniversalCenter;
    coord.v += lscrap;

    return coord;
}

}  // namespace antares

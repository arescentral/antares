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
using std::vector;

namespace antares {

namespace {

const int16_t kScenarioResID            = 500;
const int16_t kScenarioInitialResID     = 500;
const int16_t kScenarioConditionResID   = 500;
const int16_t kScenarioBriefResID       = 500;

const int32_t kOwnerMayChangeFlag   = 0x80000000;
const int32_t kAnyOwnerColorFlag    = 0x0000ffff;

const int16_t kLevelNameID = 4600;
static StringList* level_names;

vector<Scenario> gScenarioData;
vector<Scenario::InitialObject> gScenarioInitialData;
vector<Scenario::Condition> gScenarioConditionData;
vector<Scenario::BriefPoint> gScenarioBriefData;
int32_t gScenarioRotation = 0;
int32_t gAdmiralNumbers[kMaxPlayerNum];

void AddBaseObjectActionMedia(int32_t whichBase, int32_t whichType, uint8_t color);
void AddActionMedia(objectActionType *action, uint8_t color);

void SetAllBaseObjectsUnchecked() {
    baseObjectType  *aBase = mGetBaseObjectPtr(0);
    for (int32_t count = 0; count < globals()->maxBaseObject; count++) {
        aBase->internalFlags = 0;
        aBase++;
    }
}

void AddBaseObjectMedia(int32_t whichBase, uint8_t color) {
    baseObjectType      *aBase = mGetBaseObjectPtr(whichBase);

    if (!(aBase->internalFlags & (0x00000001 << color))) {
        aBase->internalFlags |= (0x00000001 << color);
        if (aBase->pixResID != kNoSpriteTable) {
            int16_t id = aBase->pixResID;
            if (aBase->attributes & kCanThink) {
                id += color << kSpriteTableColorShift;
            }
            AddPixTable(id);
        }

        AddBaseObjectActionMedia(whichBase, kDestroyActionType, color);
        AddBaseObjectActionMedia(whichBase, kExpireActionType, color);
        AddBaseObjectActionMedia(whichBase, kCreateActionType, color);
        AddBaseObjectActionMedia(whichBase, kCollideActionType, color);
        AddBaseObjectActionMedia(whichBase, kActivateActionType, color);
        AddBaseObjectActionMedia(whichBase, kArriveActionType, color);

        for (int32_t weapon : {aBase->pulse, aBase->beam, aBase->special}) {
            if (weapon != kNoWeapon) {
                AddBaseObjectMedia(weapon, color);
            }
        }
    }
}

objectActionType* mGetActionFromBaseTypeNum(
        const baseObjectType& mbaseObjPtr, int32_t mactionType, int32_t mactionNum) {
    if (mactionType == kDestroyActionType) {
        if (mactionNum < (mbaseObjPtr.destroyActionNum & kDestroyActionNotMask)) {
            return mGetObjectActionPtr(mbaseObjPtr.destroyAction + mactionNum);
        }
    } else if (mactionType == kExpireActionType) {
        if (mactionNum < (mbaseObjPtr.expireActionNum  & kDestroyActionNotMask)) {
            return mGetObjectActionPtr(mbaseObjPtr.expireAction + mactionNum);
        }
    } else if (mactionType == kCreateActionType) {
        if (mactionNum < mbaseObjPtr.createActionNum) {
            return mGetObjectActionPtr(mbaseObjPtr.createAction + mactionNum);
        }
    } else if (mactionType == kCollideActionType) {
        if (mactionNum < mbaseObjPtr.collideActionNum) {
            return mGetObjectActionPtr(mbaseObjPtr.collideAction + mactionNum);
        }
    } else if (mactionType == kActivateActionType) {
        if (mactionNum < (mbaseObjPtr.activateActionNum & kPeriodicActionNotMask)) {
            return mGetObjectActionPtr(mbaseObjPtr.activateAction + mactionNum);
        }
    } else if (mactionType == kArriveActionType) {
        if (mactionNum < mbaseObjPtr.arriveActionNum) {
            return mGetObjectActionPtr(mbaseObjPtr.arriveAction + mactionNum);
        }
    }
    return nullptr;
}

void AddBaseObjectActionMedia(int32_t whichBase, int32_t whichType, uint8_t color) {
    for (int count = 0; ; ++count) {
        const baseObjectType& baseObject = *mGetBaseObjectPtr(whichBase);
        auto* action = mGetActionFromBaseTypeNum(baseObject, whichType, count);
        if (!action) {
            break;
        }

        AddActionMedia(action, color);
    }
}

void AddActionMedia(objectActionType *action, uint8_t color) {
    baseObjectType      *baseObject = NULL;
    int32_t             count = 0, l1, l2;

    if (action == NULL) {
        return;
    }
    switch (action->verb) {
        case kCreateObject:
        case kCreateObjectSetDest:
            AddBaseObjectMedia(action->argument.createObject.whichBaseType, color);
            break;

        case kPlaySound:
            l1 = action->argument.playSound.idMinimum;
            l2 = action->argument.playSound.idMinimum +
                    action->argument.playSound.idRange;
            for (int32_t count = l1; count <= l2; count++) {
                AddSound(count); // moves mem
            }
            break;

        case kNoAction:
            action = NULL;   // get us out of loop
            break;

        case kAlter:
            switch(action->argument.alterObject.alterType) {
                case kAlterBaseType:
                    AddBaseObjectMedia(action->argument.alterObject.minimum, color);
                    break;

                case kAlterOwner:
                    baseObject = mGetBaseObjectPtr(0);
                    for (int32_t count = 0; count < globals()->maxBaseObject; count++) {
                        if (action_filter_applies_to(*action, *baseObject)) {
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

void GetInitialCoord(Scenario::InitialObject *initial, coordPointType *coord, int32_t rotation) {
    int32_t lcos, lsin, lscrap;

    mAddAngle(rotation, 90);
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
    print(out, level_names->at(name.string_id - 1));
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

    level_names = new StringList(kLevelNameID);
}

bool start_construct_scenario(const Scenario* scenario, int32_t* max) {
    ResetAllSpaceObjects();
    ResetActionQueueData();
    Beams::reset();
    ResetAllSprites();
    Labels::reset();
    ResetInstruments();
    ResetAllAdmirals();
    ResetAllDestObjectData();
    ResetMotionGlobals();
    gAbsoluteScale = kTimesTwoScale;
    globals()->gSynchValue = 0;

    gThisScenario = scenario;

    {
        int32_t angle = gThisScenario->angle();
        if (angle < 0) {
            gScenarioRotation = gRandomSeed.next(ROT_POS);
        } else {
            gScenarioRotation = angle;
        }
    }

    globals()->gScenarioWinner.player = -1;
    globals()->gScenarioWinner.next = -1;
    globals()->gScenarioWinner.text = -1;

    SetMiniScreenStatusStrList(gThisScenario->scoreStringResID);

    // *** BEGIN INIT ADMIRALS ***
    for (int i = 0; i < kMaxPlayerNum; i++) {
        gAdmiralNumbers[i] = -1;
    }

    for (int i = 0; i < gThisScenario->playerNum; i++) {
        if (gThisScenario->player[i].playerType == kSingleHumanPlayer) {
            gAdmiralNumbers[i] = MakeNewAdmiral(
                    kNoShip, kNoDestinationObject, kNoDestinationType,
                    kAIsHuman, gThisScenario->player[i].playerRace,
                    gThisScenario->player[i].nameResID,
                    gThisScenario->player[i].nameStrNum,
                    gThisScenario->player[i].earningPower);
            PayAdmiral(gAdmiralNumbers[i], mLongToFixed(5000));
            globals()->gPlayerAdmiralNumber = gAdmiralNumbers[i];
        } else {
            gAdmiralNumbers[i] = MakeNewAdmiral(
                    kNoShip, kNoDestinationObject, kNoDestinationType,
                    kAIsComputer, gThisScenario->player[i].playerRace,
                    gThisScenario->player[i].nameResID,
                    gThisScenario->player[i].nameStrNum,
                    gThisScenario->player[i].earningPower);
            PayAdmiral(gAdmiralNumbers[i], mLongToFixed(5000));
        }
    }

    // *** END INIT ADMIRALS ***

    ///// FIRST SELECT WHAT MEDIA WE NEED TO USE:

    // uncheck all base objects
    SetAllBaseObjectsUnchecked();
    // uncheck all sounds
    SetAllSoundsNoKeep();
    SetAllPixTablesNoKeep();

    RemoveAllUnusedSounds();
    RemoveAllUnusedPixTables();

    *max = gThisScenario->initialNum * 3L
         + 1
         + (gThisScenario->startTime & kScenario_StartTimeMask); // for each run through the initial num

    return true;
}

void construct_scenario(const Scenario* scenario, int32_t* current) {
    int32_t step = *current;
    if (step == 0) {
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

        // check media for all condition actions
        for (int i = 0; i < gThisScenario->playerNum; i++) {
            int32_t blessed[][2] = {
                {globals()->scenarioFileInfo.energyBlobID, 0},  // always neutral
                {globals()->scenarioFileInfo.warpInFlareID, 0},  // always neutral
                {globals()->scenarioFileInfo.warpOutFlareID, 0},  // always neutral
                {globals()->scenarioFileInfo.playerBodyID, GetAdmiralColor(i)},
            };
            for (auto id_color : blessed) {
                AddBaseObjectMedia(id_color[0], id_color[1]);
            }
        }
    }

    if ((0 <= step) && (step < gThisScenario->initialNum)) {
        int i = step;

        Scenario::InitialObject* initial = gThisScenario->initial(i);

        // get the base object equiv
        int32_t type = initial->type;
        baseObjectType* baseObject = mGetBaseObjectPtr(type);
        // TODO(sfiera): remap objects in networked games.
        // check the media for this object
        if (baseObject->attributes & kIsDestination) {
            for (int j = 0; j < gThisScenario->playerNum; j++) {
                AddBaseObjectMedia(type, GetAdmiralColor(j));
            }
        } else {
            AddBaseObjectMedia(type, GetAdmiralColor(initial->owner));
        }

        // we may have just moved memory, so let's make sure our ptrs are correct
        initial = gThisScenario->initial(i);
        baseObject = mGetBaseObjectPtr(type);

        // make sure we're not overriding the sprite
        if (initial->spriteIDOverride >= 0) {
            if (baseObject->attributes & kCanThink) {
                AddPixTable(
                        initial->spriteIDOverride +
                        (GetAdmiralColor(initial->owner) << kSpriteTableColorShift));
            } else {
                AddPixTable(initial->spriteIDOverride);
            }
        }

        // check any objects this object can build
        for (int j = 0; j < kMaxTypeBaseCanBuild; j++) {
            initial = gThisScenario->initial(i);
            if (initial->canBuild[j] != kNoClass) {
                // check for each player
                for (int k = 0; k < gThisScenario->playerNum; k++) {
                    initial = gThisScenario->initial(i);
                    baseObject = mGetBaseObjectPtr(type);
                    int32_t newShipNum;
                    mGetBaseObjectFromClassRace(
                            baseObject, newShipNum, initial->canBuild[j], GetAdmiralRace(k));
                    if (baseObject != NULL) {
                        AddBaseObjectMedia(newShipNum, GetAdmiralColor(k));
                    }
                }
            }
        }
        (*current)++;
        return;
    }
    step -= gThisScenario->initialNum;

    // add media for all condition actions
    if (step == 0) {
        {
            Scenario::Condition* condition = gThisScenario->condition(0);
            for (int i = 0; i < gThisScenario->conditionNum; i++) {
                condition = gThisScenario->condition(i);
                objectActionType* action = mGetObjectActionPtr(condition->startVerb);
                for (int j = 0; j < condition->verbNum; j++) {
                    condition = gThisScenario->condition(i);
                    action = mGetObjectActionPtr(condition->startVerb + j);
                    AddActionMedia(action, 0);
                }
            }
        }

        // make sure we check things whose owner may change
        for (int i = 0; i < globals()->maxBaseObject; i++) {
            baseObjectType* baseObject = mGetBaseObjectPtr(i);
            if ((baseObject->internalFlags & kOwnerMayChangeFlag) 
                    && (baseObject->internalFlags & kAnyOwnerColorFlag)) {
                for (int j = 0; j < gThisScenario->playerNum; j++) {
                    AddBaseObjectMedia(i, GetAdmiralColor(j));
                }
            }
        }

        SetAllBaseObjectsUnchecked();

        // begin init admirals used to be here
        {
            Scenario::Condition* condition = gThisScenario->condition(0);
            for (int i = 0; i < gThisScenario->conditionNum; i++) {
                if (condition->flags & kInitiallyTrue) {
                    condition->flags |= kHasBeenTrue;
                } else {
                    condition->flags &= ~kHasBeenTrue;
                }
                condition++;
            }
        }
    }

    if ((0 <= step) && (step < gThisScenario->initialNum)) {
        Scenario::InitialObject* initial = gThisScenario->initial(step);

        if (!(initial->attributes & kInitiallyHidden)) {
            coordPointType coord;
            GetInitialCoord(initial, &coord, gScenarioRotation);

            int32_t owner;
            if (initial->owner > kScenarioNoOwner) {
                owner = gAdmiralNumbers[initial->owner];
            } else {
                owner = kScenarioNoOwner;
            }

            int32_t specialAttributes = initial->attributes & (~kInitialAttributesMask);
            if (initial->attributes & kIsPlayerShip) {
                if (GetAdmiralFlagship(owner) == NULL) {
                    if (owner == globals()->gPlayerAdmiralNumber) {
                        specialAttributes |= kIsHumanControlled;
                    } else {
                        specialAttributes &= ~kIsPlayerShip;
                    }
                } else {
                    specialAttributes &= ~kIsPlayerShip;
                }
            }

            int32_t type = initial->type;
            // TODO(sfiera): remap object in networked games.
            fixedPointType v = {0, 0};
            int32_t newShipNum;
            initial->realObjectNumber = newShipNum = CreateAnySpaceObject(
                    type, &v, &coord, gScenarioRotation, owner, specialAttributes,
                    initial->spriteIDOverride);

            spaceObjectType* anObject = mGetSpaceObjectPtr(newShipNum);
            if (anObject->attributes & kIsDestination) {
                anObject->destinationObject = MakeNewDestination(
                        newShipNum, initial->canBuild, initial->earning, initial->nameResID,
                        initial->nameStrNum);
            }
            initial->realObjectID = anObject->id;
            if ((initial->attributes & kIsPlayerShip)
                    && (GetAdmiralFlagship(owner) == NULL)) {
                SetAdmiralFlagship(owner, newShipNum);
                if (owner == globals()->gPlayerAdmiralNumber) {
                    ResetPlayerShip(newShipNum);
                }
            }

            if (anObject->attributes & kIsDestination) {
                if (owner >= 0) {
                    if (initial->canBuild[0] >= 0) {
                        if (GetAdmiralBuildAtObject(owner) < 0) {
                            SetAdmiralConsiderObject(owner, newShipNum);
                            SetAdmiralDestinationObject(owner, newShipNum, kObjectDestinationType);
                        }
                    }
                }
            }
        } else {
            initial->realObjectNumber = -1;
        }

        (*current)++;
        return;
    }
    step -= gThisScenario->initialNum;

    // double back and set up any defined initial destinations
    if ((0 <= step) && (step < gThisScenario->initialNum)) {
        int i = step;

        Scenario::InitialObject* initial = gThisScenario->initial(i);
        // if the initial object has an initial destination
        if ((initial->realObjectNumber >= 0) && (initial->initialDestination >= 0)) {
            // only objects controlled by an Admiral can have destinations
            if (initial->owner > kScenarioNoOwner) {
                // get the correct admiral #

                int32_t owner = gAdmiralNumbers[initial->owner];
                initial = gThisScenario->initial(initial->initialDestination);

                // set the admiral's dest object to the mapped initial dest object
                SetAdmiralDestinationObject(
                        owner, initial->realObjectNumber, kObjectDestinationType);

                // now give the mapped initial object the admiral's destination

                initial = gThisScenario->initial(i);
                spaceObjectType* anObject = mGetSpaceObjectPtr(initial->realObjectNumber);
                int32_t specialAttributes = anObject->attributes; // preserve the attributes
                anObject->attributes &= ~kStaticDestination; // we've got to force this off so we can set dest
                SetObjectDestination(anObject, NULL);
                anObject->attributes = specialAttributes;
            }
        }
        (*current)++;
        return;
    }
    step -= gThisScenario->initialNum;

    if (step == 0) {
        // set up all the admiral's destination objects
        RecalcAllAdmiralBuildData();
        Messages::clear();

        int x = 0;
        const int64_t start_ticks
            = (gThisScenario->startTime & kScenario_StartTimeMask) * kScenarioTimeMultiple;
        const int64_t start_time = add_ticks(0, start_ticks);
        globals()->gGameTime = 0;
        for (int64_t i = 0; i < start_ticks; ++i) {
            globals()->gGameTime = add_ticks(globals()->gGameTime, 1);
            MoveSpaceObjects(kDecideEveryCycles);
            NonplayerShipThink(kDecideEveryCycles);
            AdmiralThink();
            ExecuteActionQueue(kDecideEveryCycles);
            CollideSpaceObjects();
            x++;
            if (x == 30) {
                x = 0;
                CheckScenarioConditions(0);
            }
            CullSprites();
            Beams::cull();
            if ((i % kScenarioTimeMultiple) == 0) {
                (*current)++;
            }
        }
        globals()->gGameTime = start_time;

        (*current)++;
        return;
    }
}

void CheckScenarioConditions(int32_t timePass) {
    Scenario::Condition     *condition = NULL;
    spaceObjectType         *sObject = NULL, *dObject = NULL;
    int32_t                 i, l, difference;
    uint32_t                distance, dcalc;
    Point                   offset(0, 0);
    bool                 conditionTrue = false;

    condition = gThisScenario->condition(0);
    for (int32_t i = 0; i < gThisScenario->conditionNum; i++) {
        if (!(condition->flags & kTrueOnlyOnce) || !(condition->flags & kHasBeenTrue)) {
            conditionTrue = false;
            switch(condition->condition) {
                case kCounterCondition:
                    l = mGetRealAdmiralNum(condition->conditionArgument.counter.whichPlayer);
                    if (GetAdmiralScore(l, condition->conditionArgument.counter.whichCounter) ==
                        condition->conditionArgument.counter.amount) {
                        conditionTrue = true;
                    }
                    break;

                case kCounterGreaterCondition:
                    l = mGetRealAdmiralNum(condition->conditionArgument.counter.whichPlayer);
                    if (GetAdmiralScore(l, condition->conditionArgument.counter.whichCounter) >=
                        condition->conditionArgument.counter.amount) {
                        conditionTrue = true;
                    }
                    break;

                case kCounterNotCondition:
                    l = mGetRealAdmiralNum(condition->conditionArgument.counter.whichPlayer);
                    if (GetAdmiralScore(l, condition->conditionArgument.counter.whichCounter) !=
                        condition->conditionArgument.counter.amount) {
                        conditionTrue = true;
                    }
                    break;

                case kDestructionCondition:
                    sObject = GetObjectFromInitialNumber(condition->conditionArgument.longValue);
                    if (sObject == NULL) {
                        conditionTrue = true;
                    }
                    break;

                case kOwnerCondition:
                    sObject = GetObjectFromInitialNumber(condition->subjectObject);
                    if (sObject != NULL) {
                        l = mGetRealAdmiralNum(condition->conditionArgument.longValue);
                        if (l == sObject->owner) {
                            conditionTrue = true;
                        }
                    }
                    break;

                case kTimeCondition:
                    if (globals()->gGameTime >=
                            ticks_to_usecs(condition->conditionArgument.longValue)) {
                        conditionTrue = true;
                    }
                    break;

                case kProximityCondition:
                    sObject = GetObjectFromInitialNumber(condition->subjectObject);
                    if (sObject != NULL) {
                        dObject = GetObjectFromInitialNumber(condition->directObject);
                        if (dObject != NULL) {
                            difference = ABS<int>(sObject->location.h - dObject->location.h);
                            dcalc = difference;
                            difference =  ABS<int>(sObject->location.v - dObject->location.v);
                            distance = difference;

                            if ((dcalc < kMaximumRelevantDistance) && (distance < kMaximumRelevantDistance)) {
                                distance = distance * distance + dcalc * dcalc;
                                if (distance < condition->conditionArgument.unsignedLongValue) {
                                    conditionTrue = true;
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
                            difference = ABS<int>(sObject->location.h - dObject->location.h);
                            dcalc = difference;
                            difference =  ABS<int>(sObject->location.v - dObject->location.v);
                            distance = difference;

                            if ((dcalc < kMaximumRelevantDistance)
                                    && (distance < kMaximumRelevantDistance)) {
                                distance = distance * distance + dcalc * dcalc;
                                if (distance >= condition->conditionArgument.unsignedLongValue) {
                                    conditionTrue = true;
                                }
                            }
                        }
                    }
                    break;

                case kHalfHealthCondition:
                    sObject = GetObjectFromInitialNumber(condition->subjectObject);
                    if (sObject == NULL) {
                        conditionTrue = true;
                    } else if (sObject->health <= (sObject->baseType->health >> 1)) {
                        conditionTrue = true;
                    }
                    break;

                case kIsAuxiliaryObject:
                    sObject = GetObjectFromInitialNumber(condition->subjectObject);
                    if (sObject != NULL) {
                        l = GetAdmiralConsiderObject(globals()->gPlayerAdmiralNumber);
                        if (l >= 0) {
                            dObject = mGetSpaceObjectPtr(l);
                            if (dObject == sObject) {
                                conditionTrue = true;
                            }
                        }
                    }
                    break;

                case kIsTargetObject:
                    sObject = GetObjectFromInitialNumber(condition->subjectObject);
                    if (sObject != NULL) {
                        l = GetAdmiralDestinationObject(globals()->gPlayerAdmiralNumber);
                        if (l >= 0) {
                            dObject = mGetSpaceObjectPtr(l);
                            if (dObject == sObject) {
                                conditionTrue = true;
                            }
                        }
                    }
                    break;

                case kVelocityLessThanEqualToCondition:
                    sObject = GetObjectFromInitialNumber(condition->subjectObject);
                    if (sObject != NULL) {
                        if (((ABS(sObject->velocity.h)) < condition->conditionArgument.longValue) &&
                            ((ABS(sObject->velocity.v)) < condition->conditionArgument.longValue)) {
                            conditionTrue = true;
                        }
                    }
                    break;

                case kNoShipsLeftCondition:
                    if (GetAdmiralShipsLeft(condition->conditionArgument.longValue) <= 0) {
                        conditionTrue = true;
                    }
                    break;

                case kCurrentMessageCondition:
                    if (Messages::current() == (condition->conditionArgument.location.h +
                        condition->conditionArgument.location.v - 1)) {
                        conditionTrue = true;
                    }
                    break;

                case kCurrentComputerCondition:
                    if ((globals()->gMiniScreenData.currentScreen ==
                        condition->conditionArgument.location.h) &&
                        ((condition->conditionArgument.location.v < 0) ||
                            (globals()->gMiniScreenData.selectLine ==
                                condition->conditionArgument.location.v))) {
                        conditionTrue = true;
                    }
                    break;

                case kZoomLevelCondition:
                    if (globals()->gZoomMode == condition->conditionArgument.longValue) {
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

                        buildAtObject = mGetDestObjectBalancePtr(GetAdmiralBuildAtObject(globals()->gPlayerAdmiralNumber));
                        if (buildAtObject != NULL) {
                            if (buildAtObject->totalBuildTime > 0) {
                                conditionTrue = true;
                            }
                        }
                    }
                    break;

                case kDirectIsSubjectTarget:
                    sObject = GetObjectFromInitialNumber(condition->subjectObject);
                    dObject = GetObjectFromInitialNumber(condition->directObject);
                    if ((sObject != NULL) && (dObject != NULL)) {
                        if (sObject->destObjectID == dObject->id) {
                            conditionTrue = true;
                        }
                    }
                    break;

                case kSubjectIsPlayerCondition:
                    sObject = GetObjectFromInitialNumber(condition->subjectObject);
                    if (sObject != NULL) {
                        if (sObject->entryNumber == globals()->gPlayerShipNumber) {
                            conditionTrue = true;
                        }
                    }
                    break;

                default:
                    break;

            }
            if (conditionTrue) {
                condition->flags |= kHasBeenTrue;
                sObject = GetObjectFromInitialNumber(condition->subjectObject);
                dObject = GetObjectFromInitialNumber(condition->directObject);
                ExecuteObjectActions(
                        condition->startVerb, condition->verbNum, sObject, dObject, &offset, true);
            }
        }
        condition++;
    }
}

void UnhideInitialObject(int32_t whichInitial) {
    Scenario::InitialObject     *initial;
    spaceObjectType         *anObject = NULL;
    coordPointType          coord;
    fixedPointType          v = {0, 0};
    uint32_t                specialAttributes;
    int32_t                 newShipNum, owner, type, saveDest, baseClass, race;
    baseObjectType          *baseObject;

    v.h = 0;
    v.v = 0;
    initial = gThisScenario->initial(whichInitial);
    anObject = GetObjectFromInitialNumber(whichInitial);
    if (anObject == NULL) {
        GetInitialCoord(initial, &coord, gScenarioRotation);

        if (initial->owner > kScenarioNoOwner) {
            owner = gAdmiralNumbers[initial->owner];
        } else {
            owner = kScenarioNoOwner;
        }

        specialAttributes = initial->attributes & ~kInitialAttributesMask;
        if (initial->attributes & kIsPlayerShip) {
            if (GetAdmiralFlagship(owner) == NULL) {
                if (owner == globals()->gPlayerAdmiralNumber) {
                    specialAttributes |= kIsHumanControlled;
                } else {
                    specialAttributes &= ~kIsPlayerShip;
                }
            } else { // we already have a flagship; this should not override
                specialAttributes &= ~kIsPlayerShip;
            }
        }


        type = initial->type;
        // TODO(sfiera): remap objects in networked games.
        initial->realObjectNumber = newShipNum = CreateAnySpaceObject(
                type, &v, &coord, 0, owner, specialAttributes, initial->spriteIDOverride);

        anObject = mGetSpaceObjectPtr(newShipNum);
        initial = gThisScenario->initial(whichInitial);

        if (anObject->attributes & kIsDestination) {
            anObject->destinationObject = MakeNewDestination(
                    newShipNum, initial->canBuild, initial->earning, initial->nameResID,
                    initial->nameStrNum);

            if (owner >= 0) {
                if (initial->canBuild[0] >= 0) {
                    if (GetAdmiralConsiderObject(owner) < 0) {
                        SetAdmiralConsiderObject(owner, newShipNum);
                    }
                    if (GetAdmiralBuildAtObject(owner) < 0) {
                        SetAdmiralBuildAtObject(owner, newShipNum);
                    }
                    if (GetAdmiralDestinationObject(owner) < 0) {
                        SetAdmiralDestinationObject(owner, newShipNum, kObjectDestinationType);
                    }
                }
            }
        }

        initial->realObjectID = anObject->id;
        if ((initial->attributes & kIsPlayerShip) && (GetAdmiralFlagship(owner) == NULL)) {
            SetAdmiralFlagship(owner, newShipNum);
            if (owner == globals()->gPlayerAdmiralNumber) {
                ResetPlayerShip(newShipNum);
            }
        }

        if (initial->initialDestination >= 0) {
            // only objects controlled by an Admiral can have destinations
            if (initial->owner > kScenarioNoOwner) {
                // get the correct admiral #

                owner = gAdmiralNumbers[initial->owner];

                // INITIAL IS BEING CHANGED HERE
                initial = gThisScenario->initial(initial->initialDestination);
                if (initial->realObjectNumber >= 0) {
                    saveDest = GetAdmiralDestinationObject(owner); // save the original dest

                    // set the admiral's dest object to the mapped initial dest object
                    SetAdmiralDestinationObject(
                            owner, initial->realObjectNumber, kObjectDestinationType);

                    // now give the mapped initial object the admiral's destination

                    initial = gThisScenario->initial(whichInitial);
                    anObject = mGetSpaceObjectPtr(initial->realObjectNumber);
                    specialAttributes = anObject->attributes; // preserve the attributes
                    anObject->attributes &= ~kStaticDestination; // we've got to force this off so we can set dest
                    SetObjectDestination(anObject, NULL);
                    anObject->attributes = specialAttributes;

                    SetAdmiralDestinationObject(owner, saveDest, kObjectDestinationType);
                }
            }
        }
    }
}

spaceObjectType *GetObjectFromInitialNumber(int32_t initialNumber) {
    if (initialNumber >= 0) {
        Scenario::InitialObject* initial = gThisScenario->initial(initialNumber);
        if (initial->realObjectNumber >= 0) {
            spaceObjectType& object = *mGetSpaceObjectPtr(initial->realObjectNumber);
            if ((object.id != initial->realObjectID) || (object.active != kObjectInUse)) {
                return NULL;
            }
            return &object;
        }
        return NULL;
    } else if (initialNumber == -2) {
        spaceObjectType& object = *mGetSpaceObjectPtr(globals()->gPlayerShipNumber);
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
    int32_t         biggest, count, otherCount, mustFit;
    Point           coord, otherCoord, tempCoord;
    Scenario::InitialObject     *initial;

    mustFit = bounds->bottom - bounds->top;
    if ((bounds->right - bounds->left) < mustFit) mustFit = bounds->right - bounds->left;

    biggest = 0;
    for (int32_t count = 0; count < scenario->initialNum; count++)
    {
        initial = scenario->initial(count);
        if (!(initial->attributes & kInitiallyHidden)) {
            GetInitialCoord(initial, reinterpret_cast<coordPointType *>(&coord), gScenarioRotation);

            for (int32_t otherCount = 0; otherCount < scenario->initialNum; otherCount++) {
                initial = scenario->initial(otherCount);
                GetInitialCoord(initial, reinterpret_cast<coordPointType *>(&otherCoord), gScenarioRotation);

                if (ABS(otherCoord.h - coord.h) > biggest) {
                    biggest = ABS(otherCoord.h - coord.h);
                }
                if (ABS(otherCoord.v - coord.v) > biggest) {
                    biggest = ABS(otherCoord.v - coord.v);
                }
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
    for (int32_t count = 0; count < scenario->initialNum; count++)
    {
        if (!(initial->attributes & kInitiallyHidden)) {
            GetInitialCoord(initial, reinterpret_cast<coordPointType *>(&tempCoord), gScenarioRotation);

            if (tempCoord.h < coord.h) {
                coord.h = tempCoord.h;
            }
            if (tempCoord.v < coord.v) {
                coord.v = tempCoord.v;
            }

            if (tempCoord.h > otherCoord.h) {
                otherCoord.h = tempCoord.h;
            }
            if (tempCoord.v > otherCoord.v) {
                otherCoord.v = tempCoord.v;
            }
        }
        initial++;
    }

    biggest = bounds->right - bounds->left;
    biggest *= SCALE_SCALE;
    biggest /= *scale;
    biggest /= 2;
    corner->h = (coord.h + (otherCoord.h - coord.h) / 2) - biggest;
    biggest = (bounds->bottom - bounds->top);
    biggest *= SCALE_SCALE;
    biggest /= *scale;
    biggest /= 2;
    corner->v = (coord.v + (otherCoord.v - coord.v) / 2) - biggest;

}

const Scenario* GetScenarioPtrFromChapter(int32_t chapter) {
    for (const Scenario& scenario: gScenarioData) {
        if (scenario.chapter_number() == chapter) {
            return &scenario;
        }
    }
    return NULL;
}

coordPointType Translate_Coord_To_Scenario_Rotation(int32_t h, int32_t v) {
    int32_t lcos, lsin, lscrap, angle = gScenarioRotation;
    coordPointType coord;

    mAddAngle(angle, 90);
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

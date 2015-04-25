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
#include <set>
#include <sfz/sfz.hpp>

#include "config/keys.hpp"
#include "data/races.hpp"
#include "data/resource.hpp"
#include "data/string-list.hpp"
#include "drawing/color.hpp"
#include "drawing/pix-table.hpp"
#include "game/action.hpp"
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
using std::set;
using std::vector;

namespace antares {

namespace {

const int16_t kScenarioResID            = 500;
const int16_t kScenarioInitialResID     = 500;
const int16_t kScenarioConditionResID   = 500;
const int16_t kScenarioBriefResID       = 500;

const uint32_t kNeutralColorNeededFlag   = 0x00010000u;
const uint32_t kAnyColorNeededFlag       = 0xffff0000u;
const uint32_t kNeutralColorLoadedFlag   = 0x00000001u;
const uint32_t kAnyColorLoadedFlag       = 0x0000ffffu;

const int16_t kLevelNameID = 4600;
static StringList* level_names;

vector<Scenario> gScenarioData;
vector<Scenario::InitialObject> gScenarioInitialData;
vector<Scenario::Condition> gScenarioConditionData;
vector<Scenario::BriefPoint> gScenarioBriefData;
int32_t gScenarioRotation = 0;

#ifdef DATA_COVERAGE
set<int32_t> possible_objects;
set<int32_t> possible_actions;
#endif  // DATA_COVERAGE

void AddBaseObjectActionMedia(
        Handle<BaseObject> base, int32_t whichType, uint8_t color, uint32_t all_colors);
void AddActionMedia(objectActionType *action, uint8_t color, uint32_t all_colors);

void SetAllBaseObjectsUnchecked() {
    for (int32_t i = 0; i < globals()->maxBaseObject; i++) {
        auto aBase = Handle<BaseObject>(i);
        aBase->internalFlags = 0;
    }
}

void AddBaseObjectMedia(Handle<BaseObject> base, uint8_t color, uint32_t all_colors) {
#ifdef DATA_COVERAGE
    possible_objects.insert(base.number());
#endif  // DATA_COVERAGE

    if (!(base->attributes & kCanThink)) {
        color = GRAY;
    }
    base->internalFlags |= (kNeutralColorNeededFlag << color);
    for (int i = 0; i < 16; ++i) {
        if (base->internalFlags & (kNeutralColorLoadedFlag << i)) {
            continue;  // color already loaded.
        } else if (!(base->internalFlags & (kNeutralColorNeededFlag << i))) {
            continue;  // color not needed.
        }
        base->internalFlags |= kNeutralColorLoadedFlag << i;

        if (base->pixResID != kNoSpriteTable) {
            int16_t id = base->pixResID + (i << kSpriteTableColorShift);
            AddPixTable(id);
        }

        AddBaseObjectActionMedia(base, kDestroyActionType, i, all_colors);
        AddBaseObjectActionMedia(base, kExpireActionType, i, all_colors);
        AddBaseObjectActionMedia(base, kCreateActionType, i, all_colors);
        AddBaseObjectActionMedia(base, kCollideActionType, i, all_colors);
        AddBaseObjectActionMedia(base, kActivateActionType, i, all_colors);
        AddBaseObjectActionMedia(base, kArriveActionType, i, all_colors);

        for (Handle<BaseObject> weapon: {base->pulse.base, base->beam.base, base->special.base}) {
            if (weapon.get()) {
                AddBaseObjectMedia(weapon, i, all_colors);
            }
        }
    }
}

objectActionType* mGetActionFromBaseTypeNum(
        Handle<BaseObject> base, int32_t mactionType, int32_t mactionNum) {
    if (mactionType == kDestroyActionType) {
        if (mactionNum < base->destroy.count) {
            return mGetObjectActionPtr(base->destroy.start + mactionNum);
        }
    } else if (mactionType == kExpireActionType) {
        if (mactionNum < base->expire.count) {
            return mGetObjectActionPtr(base->expire.start + mactionNum);
        }
    } else if (mactionType == kCreateActionType) {
        if (mactionNum < base->create.count) {
            return mGetObjectActionPtr(base->create.start + mactionNum);
        }
    } else if (mactionType == kCollideActionType) {
        if (mactionNum < base->collide.count) {
            return mGetObjectActionPtr(base->collide.start + mactionNum);
        }
    } else if (mactionType == kActivateActionType) {
        if (mactionNum < base->activate.count) {
            return mGetObjectActionPtr(base->activate.start + mactionNum);
        }
    } else if (mactionType == kArriveActionType) {
        if (mactionNum < base->arrive.count) {
            return mGetObjectActionPtr(base->arrive.start + mactionNum);
        }
    }
    return nullptr;
}

void AddBaseObjectActionMedia(
        Handle<BaseObject> base, int32_t whichType, uint8_t color, uint32_t all_colors) {
    for (int count = 0; ; ++count) {
        auto* action = mGetActionFromBaseTypeNum(base, whichType, count);
        if (!action) {
            break;
        }

        AddActionMedia(action, color, all_colors);
    }
}

void AddActionMedia(objectActionType *action, uint8_t color, uint32_t all_colors) {
    int32_t             count = 0, l1, l2;
#ifdef DATA_COVERAGE
        possible_actions.insert(action - mGetObjectActionPtr(0));
#endif  // DATA_COVERAGE

    if (action == NULL) {
        return;
    }
    switch (action->verb) {
        case kCreateObject:
        case kCreateObjectSetDest:
            AddBaseObjectMedia(action->argument.createObject.whichBaseType, color, all_colors);
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
                    AddBaseObjectMedia(
                            Handle<BaseObject>(action->argument.alterObject.minimum),
                            color, all_colors);
                    break;

                case kAlterOwner:
                    for (int32_t i = 0; i < globals()->maxBaseObject; i++) {
                        auto baseObject = Handle<BaseObject>(i);
                        if (action_filter_applies_to(*action, baseObject)) {
                            baseObject->internalFlags |= all_colors;
                        }
                        if (baseObject->internalFlags & kAnyColorLoadedFlag) {
                            AddBaseObjectMedia(baseObject, color, all_colors);
                        }
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

void set_initial_destination(const Scenario::InitialObject* initial, bool preserve) {
    if ((initial->realObjectNumber < 0)                 // hasn't been created yet
            || (initial->initialDestination < 0)        // doesn't have a target
            || (!initial->owner.get())) {               // doesn't have an owner
        return;
    }

    // get the correct admiral #
    Handle<Admiral> owner = initial->owner;

    auto target = gThisScenario->initial(initial->initialDestination);
    if (target->realObjectNumber >= 0) {
        auto saveDest = owner->target(); // save the original dest

        // set the admiral's dest object to the mapped initial dest object
        owner->set_target(target->realObjectNumber);

        // now give the mapped initial object the admiral's destination

        auto object = Handle<SpaceObject>(initial->realObjectNumber);
        uint32_t specialAttributes = object->attributes; // preserve the attributes
        object->attributes &= ~kStaticDestination; // we've got to force this off so we can set dest
        SetObjectDestination(object.get(), NULL);
        object->attributes = specialAttributes;

        if (preserve) {
            owner->set_target(saveDest);
        }
    }
}

}  // namespace

const Scenario* gThisScenario = NULL;

Scenario* mGetScenario(int32_t num) {
    return &gScenarioData[num];
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

bool Scenario::Condition::active() const {
    return !(flags & kTrueOnlyOnce)
        || !(flags & kHasBeenTrue);
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

bool Scenario::Condition::is_true() const {
    SpaceObject* sObject = nullptr;
    SpaceObject* dObject = nullptr;
    int32_t i, difference;
    Handle<Admiral> a;
    uint32_t distance, dcalc;

    switch (condition) {
        case kCounterCondition:
            a = conditionArgument.counter.whichPlayer;
            if (GetAdmiralScore(a, conditionArgument.counter.whichCounter) ==
                conditionArgument.counter.amount) {
                return true;
            }
            break;

        case kCounterGreaterCondition:
            a = conditionArgument.counter.whichPlayer;
            if (GetAdmiralScore(a, conditionArgument.counter.whichCounter) >=
                conditionArgument.counter.amount) {
                return true;
            }
            break;

        case kCounterNotCondition:
            a = conditionArgument.counter.whichPlayer;
            if (GetAdmiralScore(a, conditionArgument.counter.whichCounter) !=
                conditionArgument.counter.amount) {
                return true;
            }
            break;

        case kDestructionCondition:
            sObject = GetObjectFromInitialNumber(conditionArgument.longValue);
            if (sObject == NULL) {
                return true;
            }
            break;

        case kOwnerCondition:
            sObject = GetObjectFromInitialNumber(subjectObject);
            if (sObject != NULL) {
                a = Handle<Admiral>(conditionArgument.longValue);
                if (a == sObject->owner) {
                    return true;
                }
            }
            break;

        case kTimeCondition:
            if (globals()->gGameTime >=
                    ticks_to_usecs(conditionArgument.longValue)) {
                return true;
            }
            break;

        case kProximityCondition:
            sObject = GetObjectFromInitialNumber(subjectObject);
            if (sObject != NULL) {
                dObject = GetObjectFromInitialNumber(directObject);
                if (dObject != NULL) {
                    difference = ABS<int>(sObject->location.h - dObject->location.h);
                    dcalc = difference;
                    difference =  ABS<int>(sObject->location.v - dObject->location.v);
                    distance = difference;

                    if ((dcalc < kMaximumRelevantDistance) && (distance < kMaximumRelevantDistance)) {
                        distance = distance * distance + dcalc * dcalc;
                        if (distance < conditionArgument.unsignedLongValue) {
                            return true;
                        }
                    }
                }
            }
            break;

        case kDistanceGreaterCondition:
            sObject = GetObjectFromInitialNumber(subjectObject);
            if (sObject != NULL) {
                dObject = GetObjectFromInitialNumber(directObject);
                if (dObject != NULL) {
                    difference = ABS<int>(sObject->location.h - dObject->location.h);
                    dcalc = difference;
                    difference =  ABS<int>(sObject->location.v - dObject->location.v);
                    distance = difference;

                    if ((dcalc < kMaximumRelevantDistance)
                            && (distance < kMaximumRelevantDistance)) {
                        distance = distance * distance + dcalc * dcalc;
                        if (distance >= conditionArgument.unsignedLongValue) {
                            return true;
                        }
                    }
                }
            }
            break;

        case kHalfHealthCondition:
            sObject = GetObjectFromInitialNumber(subjectObject);
            if (sObject == NULL) {
                return true;
            } else if (sObject->health() <= (sObject->max_health() >> 1)) {
                return true;
            }
            break;

        case kIsAuxiliaryObject:
            sObject = GetObjectFromInitialNumber(subjectObject);
            if (sObject != NULL) {
                auto dObject = globals()->gPlayerAdmiral->control();
                if (dObject.get()) {
                    if (dObject.get() == sObject) {
                        return true;
                    }
                }
            }
            break;

        case kIsTargetObject:
            sObject = GetObjectFromInitialNumber(subjectObject);
            if (sObject != NULL) {
                auto dObject = globals()->gPlayerAdmiral->target();
                if (dObject.get()) {
                    if (dObject.get() == sObject) {
                        return true;
                    }
                }
            }
            break;

        case kVelocityLessThanEqualToCondition:
            sObject = GetObjectFromInitialNumber(subjectObject);
            if (sObject != NULL) {
                if (((ABS(sObject->velocity.h)) < conditionArgument.longValue) &&
                    ((ABS(sObject->velocity.v)) < conditionArgument.longValue)) {
                    return true;
                }
            }
            break;

        case kNoShipsLeftCondition:
            if (GetAdmiralShipsLeft(Handle<Admiral>(conditionArgument.longValue)) <= 0) {
                return true;
            }
            break;

        case kCurrentMessageCondition:
            if (Messages::current() == (conditionArgument.location.h +
                conditionArgument.location.v - 1)) {
                return true;
            }
            break;

        case kCurrentComputerCondition:
            if ((globals()->gMiniScreenData.currentScreen ==
                conditionArgument.location.h) &&
                ((conditionArgument.location.v < 0) ||
                    (globals()->gMiniScreenData.selectLine ==
                        conditionArgument.location.v))) {
                return true;
            }
            break;

        case kZoomLevelCondition:
            if (globals()->gZoomMode == conditionArgument.longValue) {
                return true;
            }
            break;

        case kAutopilotCondition:
            return IsPlayerShipOnAutoPilot();
            break;

        case kNotAutopilotCondition:
            return !IsPlayerShipOnAutoPilot();
            break;

        case kObjectIsBeingBuilt: {
            auto buildAtObject = Handle<Destination>(GetAdmiralBuildAtObject(globals()->gPlayerAdmiral));
            if (buildAtObject.get()) {
                if (buildAtObject->totalBuildTime > 0) {
                    return true;
                }
            }
            break;
        }

        case kDirectIsSubjectTarget:
            sObject = GetObjectFromInitialNumber(subjectObject);
            dObject = GetObjectFromInitialNumber(directObject);
            if ((sObject != NULL) && (dObject != NULL)) {
                if (sObject->destObjectID == dObject->id) {
                    return true;
                }
            }
            break;

        case kSubjectIsPlayerCondition:
            sObject = GetObjectFromInitialNumber(subjectObject);
            if (sObject != NULL) {
                if (Handle<SpaceObject>(sObject->number()) == globals()->gPlayerShip) {
                    return true;
                }
            }
            break;
    }
    return false;
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
    reset_action_queue();
    Beams::reset();
    ResetAllSprites();
    Labels::reset();
    ResetInstruments();
    Admiral::reset();
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

    globals()->gScenarioWinner.player = Admiral::none();
    globals()->gScenarioWinner.next = -1;
    globals()->gScenarioWinner.text = -1;

    SetMiniScreenStatusStrList(gThisScenario->scoreStringResID);

    for (int i = 0; i < gThisScenario->playerNum; i++) {
        if (gThisScenario->player[i].playerType == kSingleHumanPlayer) {
            auto admiral = Admiral::make(i, kAIsHuman, gThisScenario->player[i]);
            admiral->pay(mLongToFixed(5000));
            globals()->gPlayerAdmiral = Handle<Admiral>(admiral);
        } else {
            auto admiral = Admiral::make(i, kAIsComputer, gThisScenario->player[i]);
            admiral->pay(mLongToFixed(5000));
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
    uint32_t all_colors = kNeutralColorNeededFlag;
    for (int k = 0; k < gThisScenario->playerNum; k++) {
        all_colors |= kNeutralColorNeededFlag << GetAdmiralColor(Handle<Admiral>(k));
    }

    if (step == 0) {
        if (!globals()->scenarioFileInfo.energyBlobID.get()) {
            throw Exception("No energy blob defined");
        }
        if (!globals()->scenarioFileInfo.warpInFlareID.get()) {
            throw Exception("No warp in flare defined");
        }
        if (!globals()->scenarioFileInfo.warpOutFlareID.get()) {
            throw Exception("No warp out flare defined");
        }
        if (!globals()->scenarioFileInfo.playerBodyID.get()) {
            throw Exception("No player body defined");
        }

        // Load the four blessed objects.  The player's body is needed
        // in all colors; the other three are needed only as neutral
        // objects by default.
        globals()->scenarioFileInfo.playerBodyID->internalFlags |= all_colors;
        for (int i = 0; i < gThisScenario->playerNum; i++) {
            const auto& info = globals()->scenarioFileInfo;
            Handle<BaseObject> blessed[] = {
                info.energyBlobID, info.warpInFlareID, info.warpOutFlareID, info.playerBodyID,
            };
            for (auto id: blessed) {
                AddBaseObjectMedia(id, GRAY, all_colors);
            }
        }
    }

    if ((0 <= step) && (step < gThisScenario->initialNum)) {
        int i = step;

        Scenario::InitialObject* initial = gThisScenario->initial(i);
        Handle<Admiral> owner = initial->owner;
        auto baseObject = initial->type;
        // TODO(sfiera): remap objects in networked games.

        // Load the media for this object
        //
        // I don't think that it's necessary to treat kIsDestination
        // objects specially here.  If their ownership can change, there
        // will be a transport or something that does it, and we will
        // mark the necessity of having all colors through action
        // checking.
        if (baseObject->attributes & kIsDestination) {
            baseObject->internalFlags |= all_colors;
        }
        AddBaseObjectMedia(baseObject, GetAdmiralColor(owner), all_colors);

        // make sure we're not overriding the sprite
        if (initial->spriteIDOverride >= 0) {
            if (baseObject->attributes & kCanThink) {
                AddPixTable(
                        initial->spriteIDOverride +
                        (GetAdmiralColor(owner) << kSpriteTableColorShift));
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
                    auto a = Handle<Admiral>(k);
                    auto baseObject = mGetBaseObjectFromClassRace(
                            initial->canBuild[j], GetAdmiralRace(a));
                    if (baseObject.get()) {
                        AddBaseObjectMedia(baseObject, GetAdmiralColor(a), all_colors);
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
        for (int i = 0; i < gThisScenario->conditionNum; i++) {
            Scenario::Condition* condition = gThisScenario->condition(i);
            objectActionType* action = mGetObjectActionPtr(condition->action.start);
            for (int j = 0; j < condition->action.count; j++) {
                condition = gThisScenario->condition(i);
                action = mGetObjectActionPtr(condition->action.start + j);
                AddActionMedia(action, GRAY, all_colors);
            }
            condition->set_true_yet(condition->flags & kInitiallyTrue);
        }
    }

    if ((0 <= step) && (step < gThisScenario->initialNum)) {
        Scenario::InitialObject* initial = gThisScenario->initial(step);

        if (initial->attributes & kInitiallyHidden) {
            initial->realObjectNumber = -1;
            (*current)++;
            return;
        }

        coordPointType coord;
        GetInitialCoord(initial, &coord, gScenarioRotation);

        Handle<Admiral> owner = Admiral::none();
        if (initial->owner.get()) {
            owner = initial->owner;
        }

        int32_t specialAttributes = initial->attributes & (~kInitialAttributesMask);
        if (initial->attributes & kIsPlayerShip) {
            specialAttributes &= ~kIsPlayerShip;
            if ((owner == globals()->gPlayerAdmiral) && !owner->flagship().get()) {
                specialAttributes |= kIsHumanControlled | kIsPlayerShip;
            }
        }

        auto type = initial->type;
        // TODO(sfiera): remap object in networked games.
        fixedPointType v = {0, 0};
        int32_t newShipNum;
        initial->realObjectNumber = newShipNum = CreateAnySpaceObject(
                type, &v, &coord, gScenarioRotation, owner, specialAttributes,
                initial->spriteIDOverride)->number();

        auto anObject = Handle<SpaceObject>(newShipNum);
        if (anObject->attributes & kIsDestination) {
            anObject->asDestination = MakeNewDestination(
                    newShipNum, initial->canBuild, initial->earning, initial->nameResID,
                    initial->nameStrNum);
        }
        initial->realObjectID = anObject->id;

        if ((initial->attributes & kIsPlayerShip)
                && owner.get() && !owner->flagship().get()) {
            owner->set_flagship(newShipNum);
            if (owner == globals()->gPlayerAdmiral) {
                ResetPlayerShip(newShipNum);
            }
        }

        if (anObject->attributes & kIsDestination) {
            if (owner.get()) {
                if (initial->canBuild[0] >= 0) {
                    if (GetAdmiralBuildAtObject(owner) < 0) {
                        owner->set_control(newShipNum);
                        owner->set_target(newShipNum);
                    }
                }
            }
        }

        (*current)++;
        return;
    }
    step -= gThisScenario->initialNum;

    // double back and set up any defined initial destinations
    if ((0 <= step) && (step < gThisScenario->initialNum)) {
        set_initial_destination(gThisScenario->initial(step), false);
        (*current)++;
        return;
    }
    step -= gThisScenario->initialNum;

    if (step == 0) {
#ifdef DATA_COVERAGE
        {
            sfz::print(sfz::io::err, sfz::format("{{ \"level\": {0},\n", gThisScenario->chapter_number()));
            const char* sep = "";
            sfz::print(sfz::io::err, "  \"objects\": [");
            for (auto object: possible_objects) {
                sfz::print(sfz::io::err, sfz::format("{0}{1}", sep, object));
                sep = ", ";
            }
            sfz::print(sfz::io::err, "],\n");
            possible_objects.clear();

            sep = "";
            sfz::print(sfz::io::err, "  \"actions\": [");
            for (auto action: possible_actions) {
                sfz::print(sfz::io::err, sfz::format("{0}{1}", sep, action));
                sep = ", ";
            }
            sfz::print(sfz::io::err, "]\n");
            sfz::print(sfz::io::err, "}\n");
            possible_actions.clear();
        }
#endif  // DATA_COVERAGE

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
            execute_action_queue(kDecideEveryCycles);
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
    for (int32_t i = 0; i < gThisScenario->conditionNum; i++) {
        auto c = gThisScenario->condition(i);
        if (c->active() && c->is_true()) {
            c->set_true_yet(true);
            auto sObject = GetObjectFromInitialNumber(c->subjectObject);
            auto dObject = GetObjectFromInitialNumber(c->directObject);
            Point offset;
            c->action.run(sObject, dObject, &offset);
        }
    }
}

void UnhideInitialObject(int32_t whichInitial) {
    auto initial = gThisScenario->initial(whichInitial);
    if (GetObjectFromInitialNumber(whichInitial)) {
        return;  // Already visible.
    }

    coordPointType coord;
    GetInitialCoord(initial, &coord, gScenarioRotation);

    Handle<Admiral> owner = Admiral::none();
    if (initial->owner.get()) {
        owner = initial->owner;
    }

    uint32_t specialAttributes = initial->attributes & ~kInitialAttributesMask;
    if (initial->attributes & kIsPlayerShip) {
        if (owner.get() && !owner->flagship().get()) {
            if (owner == globals()->gPlayerAdmiral) {
                specialAttributes |= kIsHumanControlled;
            } else {
                specialAttributes &= ~kIsPlayerShip;
            }
        } else { // we already have a flagship; this should not override
            specialAttributes &= ~kIsPlayerShip;
        }
    }


    auto type = initial->type;
    // TODO(sfiera): remap objects in networked games.
    fixedPointType v = {0, 0};
    int32_t newShipNum = CreateAnySpaceObject(
            type, &v, &coord, 0, owner, specialAttributes, initial->spriteIDOverride)->number();
    initial->realObjectNumber = newShipNum;

    auto anObject = Handle<SpaceObject>(newShipNum);

    if (anObject->attributes & kIsDestination) {
        anObject->asDestination = MakeNewDestination(
                newShipNum, initial->canBuild, initial->earning, initial->nameResID,
                initial->nameStrNum);

        if (owner.get()) {
            if (initial->canBuild[0] >= 0) {
                if (!owner->control().get()) {
                    owner->set_control(newShipNum);
                }
                if (GetAdmiralBuildAtObject(owner) < 0) {
                    SetAdmiralBuildAtObject(owner, newShipNum);
                }
                if (!owner->target().get()) {
                    owner->set_target(newShipNum);
                }
            }
        }
    }

    initial->realObjectID = anObject->id;
    if ((initial->attributes & kIsPlayerShip) && owner.get() && !owner->flagship().get()) {
        owner->set_flagship(newShipNum);
        if (owner == globals()->gPlayerAdmiral) {
            ResetPlayerShip(newShipNum);
        }
    }

    set_initial_destination(initial, true);
}

SpaceObject *GetObjectFromInitialNumber(int32_t initialNumber) {
    if (initialNumber >= 0) {
        Scenario::InitialObject* initial = gThisScenario->initial(initialNumber);
        if (initial->realObjectNumber >= 0) {
            auto object = Handle<SpaceObject>(initial->realObjectNumber);
            if ((object->id != initial->realObjectID) || (object->active != kObjectInUse)) {
                return NULL;
            }
            return object.get();
        }
        return NULL;
    } else if (initialNumber == -2) {
        auto object = globals()->gPlayerShip;
        if (!object->active || !(object->attributes & kCanThink)) {
            return NULL;
        }
        return object.get();
    }
    return NULL;
}

void DeclareWinner(Handle<Admiral> whichPlayer, int32_t nextLevel, int32_t textID) {
    if (!whichPlayer.get()) {
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
        if (!globals()->gScenarioWinner.player.get()) {
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

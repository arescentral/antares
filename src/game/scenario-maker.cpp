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
#include "lang/defines.hpp"
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

ANTARES_GLOBAL int32_t gScenarioRotation = 0;

#ifdef DATA_COVERAGE
ANTARES_GLOBAL set<int32_t> possible_objects;
ANTARES_GLOBAL set<int32_t> possible_actions;
#endif  // DATA_COVERAGE

void AddBaseObjectActionMedia(
        Handle<BaseObject> base, int32_t whichType, uint8_t color, uint32_t all_colors);
void AddActionMedia(Handle<Action> action, uint8_t color, uint32_t all_colors);

void SetAllBaseObjectsUnchecked() {
    for (auto aBase: BaseObject::all()) {
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

HandleList<Action> action_list(Handle<BaseObject> base, int32_t type) {
    switch (type) {
        case kDestroyActionType: return base->destroy;
        case kExpireActionType: return base->expire;
        case kCreateActionType: return base->create;
        case kCollideActionType: return base->collide;
        case kActivateActionType: return base->activate;
        case kArriveActionType: return base->arrive;
        default: return {-1, -1};
    }
}

void AddBaseObjectActionMedia(
        Handle<BaseObject> base, int32_t whichType, uint8_t color, uint32_t all_colors) {
    for (auto action: action_list(base, whichType)) {
        if (action.get()) {
            AddActionMedia(action, color, all_colors);
        }
    }
}

void AddActionMedia(Handle<Action> action, uint8_t color, uint32_t all_colors) {
    int32_t             count = 0, l1, l2;
#ifdef DATA_COVERAGE
        possible_actions.insert(action.number());
#endif  // DATA_COVERAGE

    if (!action.get()) {
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

        case kAlter:
            switch(action->argument.alterObject.alterType) {
                case kAlterBaseType:
                    AddBaseObjectMedia(
                            Handle<BaseObject>(action->argument.alterObject.minimum),
                            color, all_colors);
                    break;

                case kAlterOwner:
                    for (auto baseObject: BaseObject::all()) {
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
    }
}

void GetInitialCoord(Scenario::InitialObject *initial, coordPointType *coord, int32_t rotation) {
    Fixed lcos, lsin, lscrap;

    mAddAngle(rotation, 90);
    GetRotPoint(&lcos, &lsin, rotation);
    lcos = -lcos;
    lsin = -lsin;

    lscrap = Fixed::from_val(initial->location.h) * lcos;
    lscrap -= Fixed::from_val(initial->location.v) * lsin;
    coord->h = kUniversalCenter;
    coord->h += lscrap.val();

    lscrap = Fixed::from_val(initial->location.h) * lsin;
    lscrap += Fixed::from_val(initial->location.v) * lcos;
    coord->v = kUniversalCenter;
    coord->v += lscrap.val();
}

void set_initial_destination(const Scenario::InitialObject* initial, bool preserve) {
    if (!initial->realObject.get()                      // hasn't been created yet
            || (initial->initialDestination < 0)        // doesn't have a target
            || (!initial->owner.get())) {               // doesn't have an owner
        return;
    }

    // get the correct admiral #
    Handle<Admiral> owner = initial->owner;

    auto target = g.level->initial(initial->initialDestination);
    if (target->realObject.get()) {
        auto saveDest = owner->target(); // save the original dest

        // set the admiral's dest object to the mapped initial dest object
        owner->set_target(target->realObject);

        // now give the mapped initial object the admiral's destination

        auto object = initial->realObject;
        uint32_t specialAttributes = object->attributes; // preserve the attributes
        object->attributes &= ~kStaticDestination; // we've got to force this off so we can set dest
        SetObjectDestination(object, SpaceObject::none());
        object->attributes = specialAttributes;

        if (preserve) {
            owner->set_target(saveDest);
        }
    }
}

}  // namespace

Scenario* mGetScenario(int32_t num) {
    return &plug.chapters[num];
}

Scenario::InitialObject* Scenario::initial(size_t at) const {
    return &plug.initials[initialFirst + at];
}

Scenario::Condition* Scenario::condition(size_t at) const {
    return &plug.conditions[conditionFirst + at];
}

Scenario::BriefPoint* Scenario::brief_point(size_t at) const {
    return &plug.briefings[briefPointFirst + at];
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

        case kDestructionCondition: {
            auto sObject = GetObjectFromInitialNumber(conditionArgument.longValue);
            return !sObject.get();
        }

        case kOwnerCondition: {
            auto sObject = GetObjectFromInitialNumber(subjectObject);
            auto a = Handle<Admiral>(conditionArgument.longValue);
            return sObject.get()
                && (a == sObject->owner);
        }

        case kTimeCondition:
            {
                // Tricky: the original code for handling startTime counted g.time in major ticks,
                // but new code uses minor ticks, as game/main.cpp does. So, time before the epoch
                // (game start) counts as 1/3 towards time conditions to preserve old behavior.
                ticks game_time = g.time.time_since_epoch();
                ticks start_time = g.level->startTime / 3;
                if (g.time < game_ticks()) {
                    game_time /= 3;
                }
                return (game_time + start_time) >= conditionArgument.timeValue;
            }

        case kProximityCondition: {
            auto sObject = GetObjectFromInitialNumber(subjectObject);
            auto dObject = GetObjectFromInitialNumber(directObject);
            if (sObject.get() && dObject.get()) {
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
            break;
        }

        case kDistanceGreaterCondition: {
            auto sObject = GetObjectFromInitialNumber(subjectObject);
            auto dObject = GetObjectFromInitialNumber(directObject);
            if (sObject.get() && dObject.get()) {
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
            break;
        }

        case kHalfHealthCondition: {
            auto sObject = GetObjectFromInitialNumber(subjectObject);
            return !sObject.get()
                || (sObject->health() <= (sObject->max_health() >> 1));
        }

        case kIsAuxiliaryObject: {
            auto sObject = GetObjectFromInitialNumber(subjectObject);
            auto dObject = g.admiral->control();
            return sObject.get()
                && dObject.get()
                && (dObject == sObject);
        }

        case kIsTargetObject: {
            auto sObject = GetObjectFromInitialNumber(subjectObject);
            auto dObject = g.admiral->target();
            return sObject.get()
                && dObject.get()
                && (dObject == sObject);
        }

        case kVelocityLessThanEqualToCondition: {
            auto sObject = GetObjectFromInitialNumber(subjectObject);
            return sObject.get()
                && ((ABS(sObject->velocity.h)) < Fixed::from_val(conditionArgument.longValue))
                && ((ABS(sObject->velocity.v)) < Fixed::from_val(conditionArgument.longValue));
        }

        case kNoShipsLeftCondition:
            return GetAdmiralShipsLeft(Handle<Admiral>(conditionArgument.longValue)) <= 0;

        case kCurrentMessageCondition:
            return Messages::current()
                == (conditionArgument.location.h + conditionArgument.location.v - 1);

        case kCurrentComputerCondition:
            return (globals()->gMiniScreenData.currentScreen == conditionArgument.location.h)
                && ((conditionArgument.location.v < 0)
                        || (globals()->gMiniScreenData.selectLine == conditionArgument.location.v));

        case kZoomLevelCondition:
            return globals()->gZoomMode == conditionArgument.longValue;

        case kAutopilotCondition:
            return IsPlayerShipOnAutoPilot();

        case kNotAutopilotCondition:
            return !IsPlayerShipOnAutoPilot();

        case kObjectIsBeingBuilt: {
            auto buildAtObject = GetAdmiralBuildAtObject(g.admiral);
            return buildAtObject.get() && (buildAtObject->totalBuildTime > ticks(0));
        }

        case kDirectIsSubjectTarget: {
            auto sObject = GetObjectFromInitialNumber(subjectObject);
            auto dObject = GetObjectFromInitialNumber(directObject);
            return sObject.get()
                && dObject.get()
                && (sObject->destObject == dObject)
                && (sObject->destObjectID == dObject->id);
        }

        case kSubjectIsPlayerCondition: {
            auto sObject = GetObjectFromInitialNumber(subjectObject);
            return sObject.get()
                && (sObject == g.ship);
        }
    }
    return false;
}

void ScenarioMakerInit() {
    {
        Resource rsrc("scenario-info", "nlAG", 128);
        BytesSlice in(rsrc.data());
        read(in, plug.meta);
        if (!in.empty()) {
            throw Exception("didn't consume all of scenario file info data");
        }
    }

    {
        StringList chapter_names(kLevelNameID);
        plug.chapters.clear();
        Resource rsrc("scenarios", "snro", kScenarioResID);
        BytesSlice in(rsrc.data());
        while (!in.empty()) {
            Scenario scenario;
            read(in, scenario);
            scenario.name.assign(chapter_names.at(scenario.levelNameStrNum - 1));
            plug.chapters.push_back(scenario);
        }
    }

    {
        plug.initials.clear();
        Resource rsrc("scenario-initial-objects", "snit", kScenarioInitialResID);
        BytesSlice in(rsrc.data());
        while (!in.empty()) {
            Scenario::InitialObject initial;
            read(in, initial);
            plug.initials.push_back(initial);
        }
    }

    {
        plug.conditions.clear();
        Resource rsrc("scenario-conditions", "sncd", kScenarioConditionResID);
        BytesSlice in(rsrc.data());
        while (!in.empty()) {
            Scenario::Condition condition;
            read(in, condition);
            plug.conditions.push_back(condition);
        }
    }

    {
        plug.briefings.clear();
        Resource rsrc("scenario-briefing-points", "snbf", kScenarioBriefResID);
        BytesSlice in(rsrc.data());
        while (!in.empty()) {
            Scenario::BriefPoint brief_point;
            read(in, brief_point);
            plug.briefings.push_back(brief_point);
        }
    }

    InitRaces();
}

bool start_construct_scenario(const Scenario* scenario, int32_t* max) {
    ResetAllSpaceObjects();
    reset_action_queue();
    Beams::reset();
    ResetAllSprites();
    Label::reset();
    ResetInstruments();
    Admiral::reset();
    ResetAllDestObjectData();
    ResetMotionGlobals();
    gAbsoluteScale = kTimesTwoScale;
    g.sync = 0;

    g.level = scenario;

    {
        int32_t angle = g.level->angle();
        if (angle < 0) {
            gScenarioRotation = g.random.next(ROT_POS);
        } else {
            gScenarioRotation = angle;
        }
    }

    g.victor = Admiral::none();
    g.next_level = -1;
    g.victory_text = -1;

    SetMiniScreenStatusStrList(g.level->scoreStringResID);

    for (int i = 0; i < g.level->playerNum; i++) {
        if (g.level->player[i].playerType == kSingleHumanPlayer) {
            auto admiral = Admiral::make(i, kAIsHuman, g.level->player[i]);
            admiral->pay(Fixed::from_long(5000));
            g.admiral = admiral;
        } else {
            auto admiral = Admiral::make(i, kAIsComputer, g.level->player[i]);
            admiral->pay(Fixed::from_long(5000));
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

    *max = g.level->initialNum * 3L
         + 1
         + g.level->startTime.count(); // for each run through the initial num

    return true;
}

static void load_blessed_objects(uint32_t all_colors) {
    if (!plug.meta.energyBlobID.get()) {
        throw Exception("No energy blob defined");
    }
    if (!plug.meta.warpInFlareID.get()) {
        throw Exception("No warp in flare defined");
    }
    if (!plug.meta.warpOutFlareID.get()) {
        throw Exception("No warp out flare defined");
    }
    if (!plug.meta.playerBodyID.get()) {
        throw Exception("No player body defined");
    }

    // Load the four blessed objects.  The player's body is needed
    // in all colors; the other three are needed only as neutral
    // objects by default.
    plug.meta.playerBodyID->internalFlags |= all_colors;
    for (int i = 0; i < g.level->playerNum; i++) {
        const auto& meta = plug.meta;
        Handle<BaseObject> blessed[] = {
            meta.energyBlobID, meta.warpInFlareID, meta.warpOutFlareID, meta.playerBodyID,
        };
        for (auto id: blessed) {
            AddBaseObjectMedia(id, GRAY, all_colors);
        }
    }
}

static void load_initial(int i, uint32_t all_colors) {
    Scenario::InitialObject* initial = g.level->initial(i);
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
        initial = g.level->initial(i);
        if (initial->canBuild[j] != kNoClass) {
            // check for each player
            for (auto a: Admiral::all()) {
                if (a->active()) {
                    auto baseObject = mGetBaseObjectFromClassRace(
                            initial->canBuild[j], GetAdmiralRace(a));
                    if (baseObject.get()) {
                        AddBaseObjectMedia(baseObject, GetAdmiralColor(a), all_colors);
                    }
                }
            }
        }
    }
}

static void load_condition(int i, uint32_t all_colors) {
    Scenario::Condition* condition = g.level->condition(i);
    for (auto action: condition->action) {
        AddActionMedia(action, GRAY, all_colors);
    }
    condition->set_true_yet(condition->flags & kInitiallyTrue);
}

static void create_initial(int i, uint32_t all_colors) {
    Scenario::InitialObject* initial = g.level->initial(i);

    if (initial->attributes & kInitiallyHidden) {
        initial->realObject = SpaceObject::none();
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
        if ((owner == g.admiral) && !owner->flagship().get()) {
            specialAttributes |= kIsHumanControlled | kIsPlayerShip;
        }
    }

    auto type = initial->type;
    // TODO(sfiera): remap object in networked games.
    fixedPointType v = {Fixed::zero(), Fixed::zero()};
    auto anObject = initial->realObject = CreateAnySpaceObject(
            type, &v, &coord, gScenarioRotation, owner, specialAttributes,
            initial->spriteIDOverride);

    if (anObject->attributes & kIsDestination) {
        anObject->asDestination = MakeNewDestination(
                anObject, initial->canBuild, initial->earning, initial->nameResID,
                initial->nameStrNum);
    }
    initial->realObjectID = anObject->id;

    if ((initial->attributes & kIsPlayerShip)
            && owner.get() && !owner->flagship().get()) {
        owner->set_flagship(anObject);
        if (owner == g.admiral) {
            ResetPlayerShip(anObject);
        }
    }

    if (anObject->attributes & kIsDestination) {
        if (owner.get()) {
            if (initial->canBuild[0] >= 0) {
                if (!GetAdmiralBuildAtObject(owner).get()) {
                    owner->set_control(anObject);
                    owner->set_target(anObject);
                }
            }
        }
    }
}

static void run_game_1s() {
    game_ticks start_time = game_ticks(-g.level->startTime);
    do {
        g.time += kMajorTick;
        MoveSpaceObjects(kMajorTick);
        NonplayerShipThink();
        AdmiralThink();
        execute_action_queue();
        CollideSpaceObjects();
        if (((g.time - start_time) % kConditionTick) == ticks(0)) {
            CheckScenarioConditions();
        }
        CullSprites();
        Beams::cull();
    } while ((g.time.time_since_epoch() % secs(1)) != ticks(0));
}

void construct_scenario(const Scenario* scenario, int32_t* current) {
    int32_t step = *current;
    uint32_t all_colors = kNeutralColorNeededFlag;
    for (auto adm: Admiral::all()) {
        if (adm->active()) {
            all_colors |= kNeutralColorNeededFlag << GetAdmiralColor(adm);
        }
    }

    if (step == 0) {
        load_blessed_objects(all_colors);
        load_initial(step, all_colors);
    } else if (step < g.level->initialNum) {
        load_initial(step, all_colors);
    } else if (step == g.level->initialNum) {
        // add media for all condition actions
        step -= g.level->initialNum;
        for (int i = 0; i < g.level->conditionNum; i++) {
            load_condition(i, all_colors);
        }
        create_initial(step, all_colors);
    } else if (step < (2 * g.level->initialNum)) {
        step -= g.level->initialNum;
        create_initial(step, all_colors);
    } else if (step < (3 * g.level->initialNum)) {
        // double back and set up any defined initial destinations
        step -= (2 * g.level->initialNum);
        set_initial_destination(g.level->initial(step), false);
    } else if (step == (3 * g.level->initialNum)) {
        RecalcAllAdmiralBuildData();  // set up all the admiral's destination objects
        Messages::clear();
        g.time = game_ticks(-g.level->startTime);
    } else {
        run_game_1s();
    }
    ++*current;
    return;
}

void CheckScenarioConditions() {
    for (int32_t i = 0; i < g.level->conditionNum; i++) {
        auto c = g.level->condition(i);
        if (c->active() && c->is_true()) {
            c->set_true_yet(true);
            auto sObject = GetObjectFromInitialNumber(c->subjectObject);
            auto dObject = GetObjectFromInitialNumber(c->directObject);
            Point offset;
            exec(c->action, sObject, dObject, &offset);
        }
    }
}

void UnhideInitialObject(int32_t whichInitial) {
    auto initial = g.level->initial(whichInitial);
    if (GetObjectFromInitialNumber(whichInitial).get()) {
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
            if (owner == g.admiral) {
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
    fixedPointType v = {Fixed::zero(), Fixed::zero()};
    auto anObject = initial->realObject = CreateAnySpaceObject(
            type, &v, &coord, 0, owner, specialAttributes, initial->spriteIDOverride);

    if (anObject->attributes & kIsDestination) {
        anObject->asDestination = MakeNewDestination(
                anObject, initial->canBuild, initial->earning, initial->nameResID,
                initial->nameStrNum);

        if (owner.get()) {
            if (initial->canBuild[0] >= 0) {
                if (!owner->control().get()) {
                    owner->set_control(anObject);
                }
                if (!GetAdmiralBuildAtObject(owner).get()) {
                    SetAdmiralBuildAtObject(owner, anObject);
                }
                if (!owner->target().get()) {
                    owner->set_target(anObject);
                }
            }
        }
    }

    initial->realObjectID = anObject->id;
    if ((initial->attributes & kIsPlayerShip) && owner.get() && !owner->flagship().get()) {
        owner->set_flagship(anObject);
        if (owner == g.admiral) {
            ResetPlayerShip(anObject);
        }
    }

    set_initial_destination(initial, true);
}

Handle<SpaceObject> GetObjectFromInitialNumber(int32_t initialNumber) {
    if (initialNumber >= 0) {
        Scenario::InitialObject* initial = g.level->initial(initialNumber);
        if (initial->realObject.get()) {
            auto object = initial->realObject;
            if ((object->id != initial->realObjectID) || (object->active != kObjectInUse)) {
                return SpaceObject::none();
            }
            return object;
        }
        return SpaceObject::none();
    } else if (initialNumber == -2) {
        auto object = g.ship;
        if (!object->active || !(object->attributes & kCanThink)) {
            return SpaceObject::none();
        }
        return object;
    }
    return SpaceObject::none();
}

void DeclareWinner(Handle<Admiral> whichPlayer, int32_t nextLevel, int32_t textID) {
    if (!whichPlayer.get()) {
        // if there's no winner, we want to exit immediately
        g.next_level = nextLevel;
        g.victory_text = textID;
        g.game_over = true;
        g.game_over_at = g.time;
    } else {
        if (!g.victor.get()) {
            g.victor = whichPlayer;
            g.victory_text = textID;
            g.next_level = nextLevel;
            if (!g.game_over) {
                g.game_over = true;
                g.game_over_at = g.time + secs(3);
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
    for (const Scenario& scenario: plug.chapters) {
        if (scenario.chapter_number() == chapter) {
            return &scenario;
        }
    }
    return NULL;
}

coordPointType Translate_Coord_To_Scenario_Rotation(int32_t h, int32_t v) {
    Fixed lcos, lsin, lscrap;
    int32_t angle = gScenarioRotation;
    coordPointType coord;

    mAddAngle(angle, 90);
    GetRotPoint(&lcos, &lsin, angle);
    lcos = -lcos;
    lsin = -lsin;

    lscrap = Fixed::from_val(h) * lcos;
    lscrap -= Fixed::from_val(v) * lsin;
    coord.h = kUniversalCenter;
    coord.h += lscrap.val();

    lscrap = Fixed::from_val(h) * lsin;
    lscrap += Fixed::from_val(v) * lcos;
    coord.v = kUniversalCenter;
    coord.v += lscrap.val();

    return coord;
}

}  // namespace antares

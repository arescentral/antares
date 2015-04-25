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

#include "game/admiral.hpp"

#include "data/space-object.hpp"
#include "data/string-list.hpp"
#include "game/cheat.hpp"
#include "game/globals.hpp"
#include "game/space-object.hpp"
#include "lang/casts.hpp"
#include "math/macros.hpp"
#include "math/random.hpp"
#include "math/units.hpp"
#include "sound/fx.hpp"

using sfz::Bytes;
using sfz::Exception;
using sfz::String;
using sfz::StringSlice;
using std::min;
using std::unique_ptr;

namespace antares {

namespace {

const int32_t kNoFreeAdmiral            = -1;
const int32_t kDestNoObject             = -1;

const int32_t kDestinationNameLen        = 17;
const int32_t kAdmiralNameLen            = 31;

const Fixed kUnimportantTarget          = 0x00000000;
const Fixed kMostImportantTarget        = 0x00000200;
const Fixed kLeastImportantTarget       = 0x00000100;
const Fixed kVeryImportantTarget        = 0x00000160;
const Fixed kImportantTarget            = 0x00000140;
const Fixed kSomewhatImportantTarget    = 0x00000120;
const Fixed kAbsolutelyEssential        = 0x00008000;

unique_ptr<destBalanceType[]> gDestBalanceData;
unique_ptr<Admiral[]> gAdmiralData;

}  // namespace

void Admiral::init() {
    gAdmiralData.reset(new Admiral[kMaxPlayerNum]);
    reset();
    gDestBalanceData.reset(new destBalanceType[kMaxDestObject]);
    ResetAllDestObjectData();
}

void Admiral::reset() {
    for (int i = 0; i < kMaxPlayerNum; ++i) {
        gAdmiralData[i] = Admiral();
        globals()->gActiveCheats[i] = 0;
    }
}

void ResetAllDestObjectData() {
    destBalanceType* d = mGetDestObjectBalancePtr(0);
    for (int i = 0; i < kMaxDestObject; ++i) {
        d->whichObject = kDestNoObject;
        d->name.clear();
        d->earn = d->totalBuildTime = d->buildTime = 0;
        d->buildObjectBaseNum = BaseObject::none();
        for (int j = 0; j < kMaxTypeBaseCanBuild; ++j) {
            d->canBuildType[j] = kNoShip;
        }
        for (int j = 0; j < kMaxPlayerNum; ++j) {
            d->occupied[j] = 0;
        }
        d++;
    }
}

destBalanceType* mGetDestObjectBalancePtr(int32_t whichObject) {
    return gDestBalanceData.get() + whichObject;
}

Admiral* Admiral::get(int i) {
    if ((0 <= i) && (i < kMaxPlayerNum)) {
        return &gAdmiralData[i];
    }
    return nullptr;
}

Handle<Admiral> Admiral::make(int index, uint32_t attributes, const Scenario::Player& player) {
    Handle<Admiral> a(index);
    if (a->_active) {
        return none();
    }

    a->_active = true;
    a->_attributes = attributes;
    a->_earning_power = player.earningPower;
    a->_race = player.playerRace;
    if ((player.nameResID >= 0)) {
        a->_name.assign(StringList(player.nameResID).at(player.nameStrNum - 1));
        if (a->_name.size() > kAdmiralNameLen) {
            a->_name.resize(kAdmiralNameLen);
        }
    }

    // for now set strategy balance to 0 -- we may want to calc this if player added on the fly?
    return a;
}

int32_t MakeNewDestination(
        int32_t whichObject, int32_t* canBuildType, Fixed earn, int16_t nameResID,
        int16_t nameStrNum) {
    int32_t i = 0;
    SpaceObject* object = mGetSpaceObjectPtr(whichObject);

    destBalanceType* d = mGetDestObjectBalancePtr(0);
    while ((i < kMaxDestObject) && (d->whichObject != kDestNoObject)) {
        i++;
        d++;
    }

    if (i == kMaxDestObject) {
        return -1;
    } else {
        d->whichObject = whichObject;
        d->earn = earn;
        d->totalBuildTime = d->buildTime = 0;

        if (canBuildType != NULL) {
            for (int j = 0; j < kMaxTypeBaseCanBuild; j++) {
                d->canBuildType[j] = *canBuildType;
                canBuildType++;
            }
        } else {
            for (int j = 0; j < kMaxTypeBaseCanBuild; j++) {
                d->canBuildType[j] = kNoShip;
            }
        }

        if ((nameResID >= 0)) {
            d->name.assign(StringList(nameResID).at(nameStrNum - 1));
            if (d->name.size() > kDestinationNameLen) {
                d->name.resize(kDestinationNameLen);
            }
        }

        if (object->attributes & kNeutralDeath) {
            for (int j = 0; j < kMaxPlayerNum; j++) {
                d->occupied[j] = 0;
            }

            if (object->owner.get()) {
                d->occupied[object->owner.number()] = object->baseType->initialAgeRange;
            }
        }

        return i;
    }
}

void RemoveDestination(int32_t whichDestination) {
    destBalanceType* d = mGetDestObjectBalancePtr(whichDestination);
    Admiral* a;

    if ((whichDestination >= 0) && (whichDestination < kMaxDestObject)) {
        a = gAdmiralData.get();

        for (int i = 0; i < kMaxPlayerNum; i++) {
            if (a->active()) {
                if (a->destinationObject() == d->whichObject) {
                    a->destinationObject() = kNoDestinationObject;
                    a->destinationObjectID() = -1;
                    a->has_destination() = false;
                }
                if (a->considerDestination() == whichDestination) {
                    a->considerDestination() = kNoDestinationObject;
                }

                if (a->buildAtObject() == whichDestination) {
                    a->buildAtObject() = kNoShip;
                }
            }
            a++;
        }

        d->whichObject = kDestNoObject;
        d->name.clear();
        d->earn = d->totalBuildTime = d->buildTime = 0;
        d->buildObjectBaseNum = BaseObject::none();
        for (int i = 0; i < kMaxTypeBaseCanBuild; i++) {
            d->canBuildType[i] = kNoShip;
        }

        for (int i = 0; i < kMaxPlayerNum; i++) {
            d->occupied[i] = 0;
        }
    }
}

void RecalcAllAdmiralBuildData() {
    Admiral* a = gAdmiralData.get();
    SpaceObject* anObject= NULL;
    destBalanceType* d = mGetDestObjectBalancePtr(0);

    // first clear all the data
    for (int i = 0; i < kMaxPlayerNum; i++) {
        for (int j = 0; j < kMaxNumAdmiralCanBuild; j++) {
            a->canBuildType()[j].baseNum = -1;
            a->canBuildType()[j].base = BaseObject::none();
            a->canBuildType()[j].chanceRange = -1;
        }
        a->totalBuildChance() = 0;
        a->hopeToBuild() = -1;
        a++;
    }

    for (int i = 0; i < kMaxDestObject; i++) {
        if (d->whichObject != kDestNoObject) {
            anObject = mGetSpaceObjectPtr(d->whichObject);
            if (anObject->owner.get()) {
                const auto& a = anObject->owner;
                for (int k = 0; k < kMaxTypeBaseCanBuild; k++) {
                    if (d->canBuildType[k] >= 0) {
                        int j = 0;
                        while ((a->canBuildType()[j].baseNum != d->canBuildType[k])
                                && (j < kMaxNumAdmiralCanBuild)) {
                            j++;
                        }
                        if (j == kMaxNumAdmiralCanBuild) {
                            auto baseObject = mGetBaseObjectFromClassRace(d->canBuildType[k], a->race());
                            j = 0;
                            while ((a->canBuildType()[j].baseNum != -1)
                                    && (j < kMaxNumAdmiralCanBuild)) {
                                j++;
                            }
                            if (j == kMaxNumAdmiralCanBuild) {
                                throw Exception("Too Many Types to Build!");
                            }
                            a->canBuildType()[j].baseNum = d->canBuildType[k];
                            a->canBuildType()[j].base = baseObject;
                            a->canBuildType()[j].chanceRange = a->totalBuildChance();
                            if (baseObject.get()) {
                                a->totalBuildChance() += baseObject->buildRatio;
                            }
                        }
                    }
                }
            }
        }
        d++;
    }
}

uint8_t GetAdmiralColor(Handle<Admiral> a) {
    if (!a.get()) {
        return 0;
    }
    return a->color();
}

int32_t GetAdmiralRace(Handle<Admiral> a) {
    if (!a.get()) {
        return -1;
    }
    return a->race();
}

SpaceObject* Admiral::flagship() {
    if (_flagship != kNoShip) {
        SpaceObject* anObject = mGetSpaceObjectPtr(_flagship);
        if (anObject->id == _flagshipID) {
            return anObject;
        }
    }
    return nullptr;
}

void Admiral::set_flagship(int32_t number) {
    if (number >= 0) {
        _flagship = number;
        _flagshipID = mGetSpaceObjectPtr(number)->id;
    } else {
        _flagship = kNoShip;
        _flagshipID = -1;
    }
}

void Admiral::set_target(int32_t whichObject) {
    _destinationObject = whichObject;
    if (whichObject >= 0) {
        SpaceObject* destObject = mGetSpaceObjectPtr(whichObject);
        _destinationObjectID = destObject->id;
    } else {
        _destinationObjectID = -1;
    }
    _has_destination = true;
}

int32_t Admiral::target() const {
    if (_destinationObject >= 0) {
        SpaceObject* destObject = mGetSpaceObjectPtr(_destinationObject);
        if ((destObject->id == _destinationObjectID)
                && (destObject->active == kObjectInUse)) {
            return _destinationObject;
        }
    }
    return -1;
}

void Admiral::set_control(int32_t whichObject) {
    SpaceObject* anObject = mGetSpaceObjectPtr(whichObject);
    destBalanceType* d = mGetDestObjectBalancePtr(0);

    _considerShip = whichObject;
    if (whichObject >= 0) {
        _considerShipID = anObject->id;
        if (anObject->attributes & kCanAcceptBuild) {
            int buildAtNum = 0;
            while ((d->whichObject != whichObject) && (buildAtNum < kMaxDestObject)) {
                buildAtNum++;
                d++;
            }
            if (buildAtNum < kMaxDestObject) {
                int l = 0;
                while ((l < kMaxShipCanBuild) && (d->canBuildType[l] == kNoShip)) {
                    l++;
                }
                if (l < kMaxShipCanBuild) {
                    _buildAtObject = buildAtNum;
                }
            }
        }
    } else {
        _considerShipID = -1;
    }
}

int32_t Admiral::control() const {
    if (_considerShip >= 0) {
        SpaceObject* anObject = mGetSpaceObjectPtr(_considerShip);
        if ((anObject->id == _considerShipID)
                && (anObject->active == kObjectInUse)
                && (anObject->owner.get() == this)) {
            return _considerShip;
        }
    }
    return -1;
}

bool BaseHasSomethingToBuild(int32_t whichObject) {
    destBalanceType* d = mGetDestObjectBalancePtr(0);
    SpaceObject* anObject= mGetSpaceObjectPtr(whichObject);

    if (anObject->attributes & kCanAcceptBuild) {
        int buildAtNum = 0;
        while ((d->whichObject != whichObject) && (buildAtNum < kMaxDestObject)) {
            buildAtNum++;
            d++;
        }
        if (buildAtNum < kMaxDestObject) {
            int l = 0;
            while ((l < kMaxShipCanBuild) && (d->canBuildType[l] == kNoShip)) {
                l++;
            }
            if (l < kMaxShipCanBuild) {
                return true;
            }
        }
    }
    return false;
}

int32_t GetAdmiralBuildAtObject(Handle<Admiral> a) {
    if (a->buildAtObject() >= 0) {
        destBalanceType* destBalance = mGetDestObjectBalancePtr(a->buildAtObject());
        if (destBalance->whichObject >= 0) {
            SpaceObject* anObject = mGetSpaceObjectPtr(destBalance->whichObject);
            if (anObject->owner != a) {
                a->buildAtObject() = kNoShip;
            }
        } else {
            a->buildAtObject() = kNoShip;
        }
    }
    return a->buildAtObject();
}

void SetAdmiralBuildAtObject(Handle<Admiral> a, int32_t whichObject) {
    SpaceObject* anObject = mGetSpaceObjectPtr(whichObject);
    destBalanceType* d = mGetDestObjectBalancePtr(0);

    if (!a.get()) {
        throw Exception("Can't set consider ship for -1 admiral.");
    }
    if (whichObject >= 0) {
        if (anObject->attributes & kCanAcceptBuild) {
            int buildAtNum = 0;
            while ((d->whichObject != whichObject) && (buildAtNum < kMaxDestObject)) {
                buildAtNum++;
                d++;
            }
            if (buildAtNum < kMaxDestObject) {
                int l = 0;
                while ((l < kMaxShipCanBuild) && (d->canBuildType[l] == kNoShip)) {
                    l++;
                }
                if (l < kMaxShipCanBuild) {
                    a->buildAtObject() = buildAtNum;
                }
            }
        }
    }
}

void SetAdmiralBuildAtName(Handle<Admiral> a, StringSlice name) {
    destBalanceType* destObject = mGetDestObjectBalancePtr(a->buildAtObject());
    destObject->name.assign(name.slice(0, min<size_t>(name.size(), kDestinationNameLen)));
}

StringSlice GetDestBalanceName(int32_t whichDestObject) {
    destBalanceType* destObject = mGetDestObjectBalancePtr(whichDestObject);
    return (destObject->name);
}

StringSlice GetAdmiralName(Handle<Admiral> a) {
    if (a.get()) {
        return a->name();
    } else {
        return NULL;
    }
}

void SetObjectLocationDestination(SpaceObject *o, coordPointType *where) {
    // if the object does not have an alliance, then something is wrong here--forget it
    if (o->owner.number() <= kNoOwner) {
        o->destinationObject = kNoDestinationObject;
        o->destObjectDest = kNoDestinationObject;
        o->destObjectID = -1;
        o->destObjectPtr = NULL;
        o->destinationLocation.h = o->destinationLocation.v = kNoDestinationCoord;
        o->timeFromOrigin = 0;
        o->idealLocationCalc.h = o->idealLocationCalc.v = 0;
        o->originLocation = o->location;
        return;
    }

    // if this object can't accept a destination, then forget it
    if (!(o->attributes & kCanAcceptDestination)) {
        return;
    }

    // if this object has a locked destination, then forget it
    if (o->attributes & kStaticDestination) {
        return;
    }

    // if the owner is not legal, something is very very wrong
    if (!o->owner.get()) {
        return;
    }

    const auto& a = o->owner;

    // if the admiral is not legal, or the admiral has no destination, then forget about it
    if (!a->active()) {
        o->destinationObject = kNoDestinationObject;
        o->destObjectDest = kNoDestinationObject;
        o->destObjectPtr = NULL;
        o->destinationLocation.h = o->destinationLocation.v = kNoDestinationCoord;
        o->timeFromOrigin = 0;
        o->idealLocationCalc.h = o->idealLocationCalc.v = 0;
        o->originLocation = o->location;
    } else {
        // the object is OK, the admiral is OK, then go about setting its destination
        if (o->attributes & kCanAcceptDestination) {
            o->timeFromOrigin = kTimeToCheckHome;
        } else {
            o->timeFromOrigin = 0;
        }

        // remove this object from its destination
        if (o->destinationObject != kNoDestinationObject) {
            RemoveObjectFromDestination(o);
        }

        o->destinationLocation = o->originLocation = *where;
        o->destinationObject = kNoDestinationObject;
        o->destObjectPtr = NULL;
        o->timeFromOrigin = 0;
        o->idealLocationCalc.h = o->idealLocationCalc.v = 0;
    }
}

void SetObjectDestination(SpaceObject* o, SpaceObject* overrideObject) {
    SpaceObject* dObject = overrideObject;

    // if the object does not have an alliance, then something is wrong here--forget it
    if (o->owner.number() <= kNoOwner) {
        o->destinationObject = kNoDestinationObject;
        o->destObjectDest = kNoDestinationObject;
        o->destObjectID = -1;
        o->destObjectPtr = NULL;
        o->destinationLocation.h = o->destinationLocation.v = kNoDestinationCoord;
        o->timeFromOrigin = 0;
        o->idealLocationCalc.h = o->idealLocationCalc.v = 0;
        o->originLocation = o->location;
        return;
    }

    // if this object can't accept a destination, then forget it
    if (!(o->attributes & kCanAcceptDestination)) {
        return;
    }

    // if this object has a locked destination, then forget it
    if ((o->attributes & kStaticDestination) && (overrideObject == NULL)) {
        return;
    }

    // if the owner is not legal, something is very very wrong
    if (!o->owner.get()) {
        return;
    }

    // get the admiral
    const auto& a = o->owner;

    // if the admiral is not legal, or the admiral has no destination, then forget about it
    if ((dObject == NULL) &&
            ((!a->active())
             || !a->has_destination()
             || (a->destinationObject() == kNoDestinationObject)
             || (a->destinationObjectID() == o->id))) {
        o->destinationObject = kNoDestinationObject;
        o->destObjectDest = kNoDestinationObject;
        o->destObjectPtr = NULL;
        o->destinationLocation.h = o->destinationLocation.v = kNoDestinationCoord;
        o->timeFromOrigin = 0;
        o->idealLocationCalc.h = o->idealLocationCalc.v = 0;
        o->originLocation = o->location;
    } else {
        // the object is OK, the admiral is OK, then go about setting its destination

        // first make sure we're still looking at the same object
        if (dObject == NULL) {
            dObject = mGetSpaceObjectPtr(a->destinationObject());
        }

        if ((dObject->active == kObjectInUse) &&
                ((dObject->id == a->destinationObjectID())
                 || (overrideObject != NULL))) {
            if (o->attributes & kCanAcceptDestination) {
                o->timeFromOrigin = kTimeToCheckHome;
            } else {
                o->timeFromOrigin = 0;
            }
            // remove this object from its destination
            if (o->destinationObject != kNoDestinationObject) {
                RemoveObjectFromDestination(o);
            }

            // add this object to its destination
            if (o != dObject) {
                o->runTimeFlags &= ~kHasArrived;
                o->destinationObject = dObject->number();
                o->destObjectPtr = dObject;
                o->destObjectDest = dObject->destinationObject;
                o->destObjectDestID = dObject->destObjectID;
                o->destObjectID = dObject->id;

                if (dObject->owner == o->owner) {
                    dObject->remoteFriendStrength += o->baseType->offenseValue;
                    dObject->escortStrength += o->baseType->offenseValue;
                    if (dObject->attributes & kIsDestination) {
                        if (dObject->escortStrength < dObject->baseType->friendDefecit) {
                            o->duty = eGuardDuty;
                        } else {
                            o->duty = eNoDuty;
                        }
                    } else {
                        if (dObject->escortStrength < dObject->baseType->friendDefecit) {
                            o->duty = eEscortDuty;
                        } else {
                            o->duty = eNoDuty;
                        }
                    }
                } else {
                    dObject->remoteFoeStrength += o->baseType->offenseValue;
                    if (dObject->attributes & kIsDestination) {
                        o->duty = eAssaultDuty;
                    } else {
                        o->duty = eAssaultDuty;
                    }
                }
            } else {
                o->destinationObject = kNoDestinationObject;
                o->destObjectDest = kNoDestinationObject;
                o->destObjectPtr = NULL;
                o->destinationLocation.h = o->destinationLocation.v = kNoDestinationCoord;
                o->timeFromOrigin = 0;
                o->idealLocationCalc.h = o->idealLocationCalc.v = 0;
                o->originLocation = o->location;
            }
        } else {
            o->destinationObject = kNoDestinationObject;
            o->destObjectDest = kNoDestinationObject;
            o->destObjectPtr = NULL;
            o->destinationLocation.h = o->destinationLocation.v = kNoDestinationCoord;
            o->timeFromOrigin = 0;
            o->idealLocationCalc.h = o->idealLocationCalc.v = 0;
            o->originLocation = o->location;
        }
    }
}

void RemoveObjectFromDestination(SpaceObject* o) {
    SpaceObject* dObject = o;
    if ((o->destinationObject != kNoDestinationObject) && (o->destObjectPtr != NULL)) {
        dObject = o->destObjectPtr;
        if (dObject->id == o->destObjectID) {
            if (dObject->owner == o->owner) {
                dObject->remoteFriendStrength -= o->baseType->offenseValue;
                dObject->escortStrength -= o->baseType->offenseValue;
            } else {
                dObject->remoteFoeStrength -= o->baseType->offenseValue;
            }
        }
    }

    o->destinationObject = kNoDestinationObject;
    o->destObjectDest = kNoDestinationObject;
    o->destObjectID = -1;
    o->destObjectPtr = NULL;
}

// assumes you can afford it & base has time
static void AdmiralBuildAtObject(
        Handle<Admiral> admiral, Handle<BaseObject> base, int32_t whichDestObject) {
    destBalanceType* buildAtDest = mGetDestObjectBalancePtr(whichDestObject);
    SpaceObject* buildAtObject = NULL;
    coordPointType  coord;
    fixedPointType  v = {0, 0};

    if (base.get() && (admiral->buildAtObject() >= 0)) {
        buildAtObject = mGetSpaceObjectPtr(buildAtDest->whichObject);
        coord = buildAtObject->location;

        SpaceObject* newObject = CreateAnySpaceObject(base, &v, &coord, 0, admiral, 0, -1);
        if (newObject) {
            SetObjectDestination(newObject, NULL);
            if (admiral == globals()->gPlayerAdmiral) {
                PlayVolumeSound(kComputerBeep2, kMediumVolume, kMediumPersistence,
                        kLowPrioritySound);
            }
        }
    }
}

void AdmiralThink() {
    Admiral* a =gAdmiralData.get();
    SpaceObject* anObject;
    destBalanceType* destBalance;

    destBalance = mGetDestObjectBalancePtr(0);
    for (int i = 0; i < kMaxDestObject; i++) {
        destBalance->buildTime -= 10;
        if (destBalance->buildTime <= 0) {
            destBalance->buildTime = 0;
            if (destBalance->buildObjectBaseNum.get()) {
                anObject = mGetSpaceObjectPtr(destBalance->whichObject);
                AdmiralBuildAtObject(anObject->owner, destBalance->buildObjectBaseNum, i);
                destBalance->buildObjectBaseNum = BaseObject::none();
            }
        }

        anObject = mGetSpaceObjectPtr(destBalance->whichObject);
        if (anObject && anObject->owner.get()) {
            anObject->owner->pay(destBalance->earn);
        }
        destBalance++;
    }

    for (int i = 0; i < kMaxPlayerNum; i++) {
        a->think();
        a++;
    }
}

void Admiral::think() {
    Handle<SpaceObject> anObject;
    Handle<SpaceObject> destObject;
    Handle<SpaceObject> otherDestObject;
    Handle<SpaceObject> stepObject;
    destBalanceType* destBalance;
    int32_t origObject, origDest, difference;
    Fixed  friendValue, foeValue, thisValue;
    Point gridLoc;

    if (!(_attributes & kAIsComputer) || (_attributes & kAIsRemote)) {
        return;
    }

    if (_blitzkrieg > 0) {
        _blitzkrieg--;
        if (_blitzkrieg <= 0) {
            // Really 48:
            _blitzkrieg = 0 - (gRandomSeed.next(1200) + 1200);
            for (int j = 0; j < kMaxSpaceObject; j++) {
                anObject = Handle<SpaceObject>(j);
                if (anObject->owner.get() == this) {
                    anObject->currentTargetValue = 0x00000000;
                }
            }
        }
    } else {
        _blitzkrieg++;
        if (_blitzkrieg >= 0) {
            // Really 48:
            _blitzkrieg = gRandomSeed.next(1200) + 1200;
            for (int j = 0; j < kMaxSpaceObject; j++) {
                anObject = Handle<SpaceObject>(j);
                if (anObject->owner.get() == this) {
                    anObject->currentTargetValue = 0x00000000;
                }
            }
        }
    }

    // get the current object
    if (_considerShip < 0) {
        _considerShip = gRootObjectNumber;
        anObject = Handle<SpaceObject>(_considerShip);
        _considerShipID = anObject->id;
    } else {
        anObject = Handle<SpaceObject>(_considerShip);
    }

    if (_destinationObject < 0) {
        _destinationObject = gRootObjectNumber;
    }

    if (anObject->active != kObjectInUse) {
        _considerShip = gRootObjectNumber;
        anObject = Handle<SpaceObject>(_considerShip);
        _considerShipID = anObject->id;
    }

    if (_destinationObject >= 0) {
        destObject = Handle<SpaceObject>(_destinationObject);
        if (destObject->active != kObjectInUse) {
            destObject = gRootObject;
            _destinationObject = gRootObjectNumber;
        }
        origDest = _destinationObject;
        do {
            _destinationObject = destObject->nextObjectNumber;

            // if we've gone through all of the objects
            if (_destinationObject < 0) {
                // ********************************
                // SHIP MUST DECIDE, THEN INCREASE CONSIDER SHIP
                // ********************************
                if ((anObject->duty != eEscortDuty)
                        && (anObject->duty != eHostileBaseDuty)
                        && (anObject->bestConsideredTargetValue >
                            anObject->currentTargetValue)) {
                    _destinationObject = anObject->bestConsideredTargetNumber;
                    _has_destination = true;
                    if (_destinationObject >= 0) {
                        destObject = Handle<SpaceObject>(_destinationObject);
                        if (destObject->active == kObjectInUse) {
                            _destinationObjectID = destObject->id;
                            anObject->currentTargetValue
                                = anObject->bestConsideredTargetValue;
                            thisValue = anObject->randomSeed.next(
                                    mFloatToFixed(0.5))
                                - mFloatToFixed(0.25);
                            thisValue = mMultiplyFixed(
                                    thisValue, anObject->currentTargetValue);
                            anObject->currentTargetValue += thisValue;
                            SetObjectDestination(anObject.get(), NULL);
                        }
                    }
                    _has_destination = false;
                }

                if ((anObject->duty != eEscortDuty)
                        && (anObject->duty != eHostileBaseDuty)) {
                    _thisFreeEscortStrength += anObject->baseType->offenseValue;
                }

                anObject->bestConsideredTargetValue = 0xffffffff;
                // start back with 1st ship
                _destinationObject = gRootObjectNumber;
                destObject = gRootObject;

                // >>> INCREASE CONSIDER SHIP
                origObject = _considerShip;
                anObject = Handle<SpaceObject>(_considerShip);
                if (anObject->active != kObjectInUse) {
                    anObject = gRootObject;
                    _considerShip = gRootObjectNumber;
                    _considerShipID = anObject->id;
                }
                do {
                    _considerShip = anObject->nextObjectNumber;
                    if (_considerShip < 0) {
                        _considerShip = gRootObjectNumber;
                        anObject = gRootObject;
                        _considerShipID = anObject->id;
                        _lastFreeEscortStrength = _thisFreeEscortStrength;
                        _thisFreeEscortStrength = 0;
                    } else {
                        anObject = anObject->nextObject;
                        _considerShipID = anObject->id;
                    }
                } while (((anObject->owner.get() != this)
                            || (!(anObject->attributes & kCanAcceptDestination))
                            || (anObject->active != kObjectInUse))
                        && (_considerShip != origObject));
            } else {
                destObject = destObject->nextObject;
            }
            _destinationObjectID = destObject->id;
        } while (((!(destObject->attributes & (kCanBeDestination)))
                    || (_destinationObject == _considerShip)
                    || (destObject->active != kObjectInUse)
                    || (!(destObject->attributes & kCanBeDestination)))
                && (_destinationObject != origDest));

        // if our object is legal and our destination is legal
        if ((anObject->owner.get() == this)
                && (anObject->attributes & kCanAcceptDestination)
                && (anObject->active == kObjectInUse)
                && (destObject->attributes & (kCanBeDestination))
                && (destObject->active == kObjectInUse)
                && ((anObject->owner != destObject->owner)
                    || (anObject->baseType->destinationClass <
                        destObject->baseType->destinationClass))) {
            gridLoc = destObject->distanceGrid;
            stepObject = otherDestObject = destObject;
            while (stepObject->nextFarObject != NULL) {
                if ((stepObject->distanceGrid.h == gridLoc.h)
                        && (stepObject->distanceGrid.v == gridLoc.v)) {
                    otherDestObject = stepObject;
                }
                stepObject = Handle<SpaceObject>(stepObject->nextFarObject->number());
            }
            if (otherDestObject->owner == anObject->owner) {
                friendValue = otherDestObject->localFriendStrength;
                foeValue = otherDestObject->localFoeStrength;
            } else {
                foeValue = otherDestObject->localFriendStrength;
                friendValue = otherDestObject->localFoeStrength;
            }


            thisValue = kUnimportantTarget;
            if (destObject->owner == anObject->owner) {
                if (destObject->attributes & kIsDestination) {
                    if (destObject->escortStrength < destObject->baseType->friendDefecit) {
                        thisValue = kAbsolutelyEssential;
                    } else if (foeValue) {
                        if (foeValue >= friendValue) {
                            thisValue = kMostImportantTarget;
                        } else if (foeValue > (friendValue >> 1)) {
                            thisValue = kVeryImportantTarget;
                        } else {
                            thisValue = kUnimportantTarget;
                        }
                    } else {
                        if ((_blitzkrieg > 0) && (anObject->duty == eGuardDuty)) {
                            thisValue = kUnimportantTarget;
                        } else {
                            if (foeValue > 0) {
                                thisValue = kSomewhatImportantTarget;
                            } else {
                                thisValue = kUnimportantTarget;
                            }
                        }
                    }
                    if (anObject->baseType->orderFlags & kTargetIsBase) {
                        thisValue <<= 3;
                    }
                    if (anObject->baseType->orderFlags & kHardTargetIsNotBase) {
                        thisValue = 0;
                    }
                } else {
                    if (destObject->baseType->destinationClass
                            > anObject->baseType->destinationClass) {
                        if (foeValue > friendValue) {
                            thisValue = kMostImportantTarget;
                        } else {
                            if (destObject->escortStrength
                                    < destObject->baseType->friendDefecit) {
                                thisValue = kMostImportantTarget;
                            } else {
                                thisValue = kUnimportantTarget;
                            }
                        }
                    } else {
                        thisValue = kUnimportantTarget;
                    }
                    if (anObject->baseType->orderFlags & kTargetIsNotBase) {
                        thisValue <<= 3;
                    }
                    if (anObject->baseType->orderFlags & kHardTargetIsBase) {
                        thisValue = 0;
                    }
                }
                if (anObject->baseType->orderFlags & kTargetIsFriend) {
                    thisValue <<= 3;
                }
                if (anObject->baseType->orderFlags & kHardTargetIsFoe) {
                    thisValue = 0;
                }
            } else if (destObject->owner.get()) {
                if ((anObject->duty == eGuardDuty) || (anObject->duty == eNoDuty)) {
                    if (destObject->attributes & kIsDestination) {
                        if (foeValue < friendValue) {
                            thisValue = kMostImportantTarget;
                        } else {
                            thisValue = kSomewhatImportantTarget;
                        }
                        if (_blitzkrieg > 0) {
                            thisValue <<= 2;
                        }
                        if (anObject->baseType->orderFlags & kTargetIsBase) {
                            thisValue <<= 3;
                        }

                        if (anObject->baseType->orderFlags & kHardTargetIsNotBase) {
                            thisValue = 0;
                        }
                    } else {
                        if (friendValue) {
                            if (friendValue < foeValue) {
                                thisValue = kSomewhatImportantTarget;
                            } else {
                                thisValue = kUnimportantTarget;
                            }
                        } else {
                            thisValue = kLeastImportantTarget;
                        }
                        if (anObject->baseType->orderFlags & kTargetIsNotBase) {
                            thisValue <<= 1;
                        }

                        if (anObject->baseType->orderFlags & kHardTargetIsBase) {
                            thisValue = 0;
                        }
                    }
                }
                if (anObject->baseType->orderFlags & kTargetIsFoe) {
                    thisValue <<= 3;
                }
                if (anObject->baseType->orderFlags & kHardTargetIsFriend) {
                    thisValue = 0;
                }
            } else {
                if (destObject->attributes & kIsDestination) {
                    thisValue = kVeryImportantTarget;
                    if (_blitzkrieg > 0) {
                        thisValue <<= 2;
                    }
                    if (anObject->baseType->orderFlags & kTargetIsBase) {
                        thisValue <<= 3;
                    }
                    if (anObject->baseType->orderFlags & kHardTargetIsNotBase) {
                        thisValue = 0;
                    }
                } else {
                    if (anObject->baseType->orderFlags & kTargetIsNotBase) {
                        thisValue <<= 3;
                    }
                    if (anObject->baseType->orderFlags & kHardTargetIsBase) {
                        thisValue = 0;
                    }
                }
                if (anObject->baseType->orderFlags & kTargetIsFoe) {
                    thisValue <<= 3;
                }
                if (anObject->baseType->orderFlags & kHardTargetIsFriend) {
                    thisValue = 0;
                }
            }

            difference = ABS(implicit_cast<int32_t>(destObject->location.h)
                    - implicit_cast<int32_t>(anObject->location.h));
            gridLoc.h = difference;
            difference =  ABS(implicit_cast<int32_t>(destObject->location.v)
                    - implicit_cast<int32_t>(anObject->location.v));
            gridLoc.v = difference;

            if ((gridLoc.h < kMaximumRelevantDistance)
                    && (gridLoc.v < kMaximumRelevantDistance)) {
                if (anObject->baseType->orderFlags & kTargetIsLocal) {
                    thisValue <<= 3;
                }
                if (anObject->baseType->orderFlags & kHardTargetIsRemote) {
                    thisValue = 0;
                }
            } else {
                if (anObject->baseType->orderFlags & kTargetIsRemote) {
                    thisValue <<= 3;
                }
                if (anObject->baseType->orderFlags & kHardTargetIsLocal) {
                    thisValue = 0;
                }
            }


            if (anObject->baseType->orderKeyTag
                    && (anObject->baseType->orderKeyTag == destObject->baseType->levelKeyTag)) {
                thisValue <<= 3;
            } else if (anObject->baseType->orderFlags & kHardMatchingFoe) {
                thisValue = 0;
            }

            if (thisValue > 0) {
                thisValue += anObject->randomSeed.next(thisValue >> 1) - (thisValue >> 2);
            }
            if (thisValue > anObject->bestConsideredTargetValue) {
                anObject->bestConsideredTargetValue = thisValue;
                anObject->bestConsideredTargetNumber = _destinationObject;
            }
        }
    }

    // if we've saved enough for our dreams
    if (_cash > _saveGoal) {
        _saveGoal = 0;

        // consider what ship to build
        if (_buildAtObject < 0) {
            _buildAtObject = 0;
        }
        origDest = _buildAtObject;
        destBalance = mGetDestObjectBalancePtr(_buildAtObject);

        // try to find the next destination object that we own & that can build
        do {
            _buildAtObject++;
            destBalance++;
            if (_buildAtObject >= kMaxDestObject) {
                _buildAtObject = 0;
                destBalance = mGetDestObjectBalancePtr(0);
            }
            if (destBalance->whichObject >= 0) {
                anObject = Handle<SpaceObject>(destBalance->whichObject);
                if ((anObject->owner.get() != this)
                        || (!(anObject->attributes & kCanAcceptBuild))) {
                    anObject = SpaceObject::none();
                }
            } else {
                anObject = SpaceObject::none();
            }
        } while (!anObject.get() && (_buildAtObject != origDest));

        // if we have a legal object
        if (anObject.get()) {
            if (destBalance->buildTime <= 0) {
                if (_hopeToBuild < 0) {
                    int k = 0;
                    while ((_hopeToBuild < 0) && (k < 7)) {
                        k++;
                        // choose something to build
                        thisValue = gRandomSeed.next(_totalBuildChance);
                        friendValue = 0xffffffff; // equals the highest qualifying object
                        for (int j = 0; j < kMaxNumAdmiralCanBuild; ++j) {
                            if ((_canBuildType[j].chanceRange <= thisValue)
                                    && (_canBuildType[j].chanceRange > friendValue)) {
                                friendValue = _canBuildType[j].chanceRange;
                                _hopeToBuild = _canBuildType[j].baseNum;
                            }
                        }
                        if (_hopeToBuild >= 0) {
                            auto baseObject = mGetBaseObjectFromClassRace(_hopeToBuild, _race);
                            if (baseObject->buildFlags & kSufficientEscortsExist) {
                                for (int j = 0; j < kMaxSpaceObject; ++j) {
                                    anObject = Handle<SpaceObject>(j);
                                    if ((anObject->active)
                                            && (anObject->owner.get() == this)
                                            && (anObject->base == baseObject)
                                            && (anObject->escortStrength <
                                                baseObject->friendDefecit)) {
                                        _hopeToBuild = -1;
                                        j = kMaxSpaceObject;
                                    }
                                }
                            }

                            if (baseObject->buildFlags & kMatchingFoeExists) {
                                thisValue = 0;
                                for (int j = 0; j < kMaxSpaceObject; j++) {
                                    anObject = Handle<SpaceObject>(j);
                                    if ((anObject->active)
                                            && (anObject->owner.get() != this)
                                            && (anObject->baseType->levelKeyTag
                                                == baseObject->orderKeyTag)) {
                                        thisValue = 1;
                                    }
                                }
                                if (!thisValue) {
                                    _hopeToBuild = -1;
                                }
                            }
                        }
                    }
                }
                int j = 0;
                while ((destBalance->canBuildType[j] != _hopeToBuild)
                        && (j < kMaxTypeBaseCanBuild)) {
                    j++;
                }
                if ((j < kMaxTypeBaseCanBuild) && (_hopeToBuild != kNoShip)) {
                    auto baseObject = mGetBaseObjectFromClassRace(_hopeToBuild, _race);
                    if (_cash >= mLongToFixed(baseObject->price)) {
                        Admiral::build(j);
                        _hopeToBuild = -1;
                        _saveGoal = 0;
                    } else {
                        _saveGoal = mLongToFixed(baseObject->price);
                    }
                } // otherwise just wait until we get to it
            }
        }
    }
}

bool Admiral::build(int32_t buildWhichType) {
    destBalanceType* buildAtDest = mGetDestObjectBalancePtr(_buildAtObject);

    Handle<Admiral> self(this - gAdmiralData.get());
    GetAdmiralBuildAtObject(self);
    if ((buildWhichType >= 0)
            && (buildWhichType < kMaxTypeBaseCanBuild)
            && (_buildAtObject >= 0) && (buildAtDest->buildTime <= 0)) {
        auto buildBaseObject = mGetBaseObjectFromClassRace(buildAtDest->canBuildType[buildWhichType], _race);
        if (buildBaseObject.get() && (buildBaseObject->price <= mFixedToLong(_cash))) {
            _cash -= (mLongToFixed(buildBaseObject->price));
            if (globals()->gActiveCheats[self.number()] & kBuildFastBit) {
                buildAtDest->buildTime = 9;
                buildAtDest->totalBuildTime = 9;
            } else {
                buildAtDest->buildTime = buildBaseObject->buildTime;
                buildAtDest->totalBuildTime = buildAtDest->buildTime;
            }
            buildAtDest->buildObjectBaseNum = buildBaseObject;
            return true;
        }
    }
    return false;
}

void StopBuilding(int32_t whichDestObject) {
    destBalanceType* destObject;

    destObject = mGetDestObjectBalancePtr(whichDestObject);
    destObject->totalBuildTime = destObject->buildTime = 0;
    destObject->buildObjectBaseNum = BaseObject::none();
}

void Admiral::pay(Fixed howMuch) {
    pay_absolute(mMultiplyFixed(howMuch, _earning_power));
}

void Admiral::pay_absolute(Fixed howMuch) {
    _cash += howMuch;
    if (_cash < 0) {
        _cash = 0;
    }
}

void AlterAdmiralScore(Handle<Admiral> admiral, int32_t whichScore, int32_t amount) {
    if (admiral.get() && (whichScore >= 0) && (whichScore < kAdmiralScoreNum)) {
        admiral->score()[whichScore] += amount;
    }
}

int32_t GetAdmiralScore(Handle<Admiral> admiral, int32_t whichScore) {
    if (admiral.get() && (whichScore >= 0) && (whichScore < kAdmiralScoreNum)) {
        return admiral->score()[whichScore];
    } else {
        return 0;
    }
}

int32_t GetAdmiralShipsLeft(Handle<Admiral> admiral) {
    if (admiral.get()) {
        return admiral->shipsLeft();
    } else {
        return 0;
    }
}

int32_t AlterDestinationObjectOccupation(int32_t whichDestination, Handle<Admiral> a, int32_t amount) {
    destBalanceType* d = mGetDestObjectBalancePtr(whichDestination);

    if (a.get()) {
        d->occupied[a.number()] += amount;
        return(d->occupied[a.number()]);
    } else {
        return -1;
    }
}

void ClearAllOccupants(int32_t whichDestination, Handle<Admiral> a, int32_t fullAmount) {
    destBalanceType* d = mGetDestObjectBalancePtr(whichDestination);

    for (int i = 0; i < kMaxPlayerNum; i++) {
        d->occupied[i] = 0;
    }
    if (a.get()) {
        d->occupied[a.number()] = fullAmount;
    }
}

void AddKillToAdmiral(SpaceObject* anObject) {
    // only for player
    const auto& admiral = globals()->gPlayerAdmiral;

    if (anObject->attributes & kCanAcceptDestination) {
        if (anObject->owner == globals()->gPlayerAdmiral) {
            admiral->losses()++;
        } else {
            admiral->kills()++;
        }
    }
}

int32_t GetAdmiralLoss(Handle<Admiral> admiral) {
    if (admiral.get()) {
        return admiral->losses();
    } else {
        return 0;
    }
}

int32_t GetAdmiralKill(Handle<Admiral> admiral) {
    if (admiral.get()) {
        return admiral->kills();
    } else {
        return 0;
    }
}

}  // namespace antares

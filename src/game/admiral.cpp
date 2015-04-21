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

void AdmiralInit() {
    gAdmiralData.reset(new Admiral[kMaxPlayerNum]);
    ResetAllAdmirals();
    gDestBalanceData.reset(new destBalanceType[kMaxDestObject]);
    ResetAllDestObjectData();
}

void AdmiralCleanup() {
    gAdmiralData.reset();
    gDestBalanceData.reset();
}

void ResetAllAdmirals() {
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
        d->buildObjectBaseNum = kNoShip;
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

Admiral* mGetAdmiralPtr(int32_t mwhichAdmiral) {
    return Admiral::get(mwhichAdmiral);
}

Admiral* Admiral::get(int i) {
    return &gAdmiralData[i];
}

int Admiral::number() const {
    return this - gAdmiralData.get();
}

static Admiral* next_free_admiral() {
    for (int i = 0; i < kMaxPlayerNum; ++i) {
        Admiral& a = gAdmiralData[i];
        if (!a.active()) {
            return &a;
        }
    }
    return nullptr;
}

Admiral* Admiral::make(uint32_t attributes, const Scenario::Player& player) {
    Admiral* a = next_free_admiral();
    if (!a) {
        return nullptr;
    }

    *a = Admiral();
    a->_active = true;
    a->_attributes = attributes;
    a->_earningPower = player.earningPower;
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

            if (object->owner >= 0) {
                d->occupied[object->owner] = object->baseType->initialAgeRange;
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
                    a->destType() = kNoDestinationType;
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
        d->buildObjectBaseNum = kNoShip;
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
    BaseObject* baseObject = NULL;
    destBalanceType* d = mGetDestObjectBalancePtr(0);
    int32_t l;

    // first clear all the data
    for (int i = 0; i < kMaxPlayerNum; i++) {
        for (int j = 0; j < kMaxNumAdmiralCanBuild; j++) {
            a->canBuildType()[j].baseNum = -1;
            a->canBuildType()[j].base = NULL;
            a->canBuildType()[j].chanceRange = -1;
        }
        a->totalBuildChance() = 0;
        a->hopeToBuild() = -1;
        a++;
    }

    for (int i = 0; i < kMaxDestObject; i++) {
        if (d->whichObject != kDestNoObject) {
            anObject = mGetSpaceObjectPtr(d->whichObject);
            if (anObject->owner >= 0) {
                a = gAdmiralData.get() + anObject->owner;
                for (int k = 0; k < kMaxTypeBaseCanBuild; k++) {
                    if (d->canBuildType[k] >= 0) {
                        int j = 0;
                        while ((a->canBuildType()[j].baseNum != d->canBuildType[k])
                                && (j < kMaxNumAdmiralCanBuild)) {
                            j++;
                        }
                        if (j == kMaxNumAdmiralCanBuild) {
                            mGetBaseObjectFromClassRace(
                                    baseObject, l, d->canBuildType[k], a->race());
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
                            if (baseObject != NULL) {
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

uint8_t GetAdmiralColor(int32_t whichAdmiral) {
    if (whichAdmiral < 0) {
        return 0;
    }
    Admiral* a = gAdmiralData.get() + whichAdmiral;
    return a->color();
}

int32_t GetAdmiralRace(int32_t whichAdmiral) {
    if (whichAdmiral < 0) {
        return -1;
    }
    Admiral* a = gAdmiralData.get() + whichAdmiral;
    return a->race();
}

void SetAdmiralFlagship(int32_t whichAdmiral, int32_t whichShip) {
    if (whichAdmiral < 0) {
        throw Exception ("Can't set flagship of -1 admiral.");
    }
    Admiral* a = gAdmiralData.get() + whichAdmiral;
    if (whichShip >= 0) {
        a->flagship() = whichShip;
        SpaceObject* anObject = mGetSpaceObjectPtr(whichShip);
        a->flagshipID() = anObject->id;
    } else {
        a->flagshipID() = -1;
    }
}

SpaceObject* GetAdmiralFlagship(int32_t whichAdmiral) {
    if (whichAdmiral < 0) {
        return NULL;
    }
    Admiral* a = gAdmiralData.get() + whichAdmiral;
    if (a->flagship() == kNoShip) {
        return NULL;
    }
    SpaceObject* anObject = mGetSpaceObjectPtr(a->flagship());
    if (anObject->id == a->flagshipID()) {
        return anObject;
    } else {
        return NULL;
    }
}

void SetAdmiralEarningPower(int32_t whichAdmiral, Fixed power) {
    if (whichAdmiral >= 0) {
        Admiral* a = gAdmiralData.get() + whichAdmiral;
        a->earningPower() = power;
    }
}

Fixed GetAdmiralEarningPower(int32_t whichAdmiral) {
    if (whichAdmiral >= 0) {
        Admiral* a = gAdmiralData.get() + whichAdmiral;
        return a->earningPower();
    } else {
        return 0;
    }
}

void SetAdmiralDestinationObject(int32_t whichAdmiral, int32_t whichObject, destinationType dType) {
    Admiral* a = gAdmiralData.get() + whichAdmiral;
    a->destinationObject() = whichObject;
    if (whichObject >= 0) {
        SpaceObject* destObject = mGetSpaceObjectPtr(whichObject);
        a->destinationObjectID() = destObject->id;
    } else {
        a->destinationObjectID() = -1;
    }
    a->destType() = dType;
}

int32_t GetAdmiralDestinationObject(int32_t whichAdmiral) {
    Admiral* a = gAdmiralData.get() + whichAdmiral;

    if (a->destinationObject() < 0) {
        return (a->destinationObject());
    }

    SpaceObject* destObject = mGetSpaceObjectPtr(a->destinationObject());
    if ((destObject->id == a->destinationObjectID())
            && (destObject->active == kObjectInUse)) {
        return a->destinationObject();
    } else {
        a->destinationObject() = -1;
        a->destinationObjectID() = -1;
        return -1;
    }
}

void SetAdmiralConsiderObject(int32_t whichAdmiral, int32_t whichObject) {
    SpaceObject* anObject= mGetSpaceObjectPtr(whichObject);
    destBalanceType* d = mGetDestObjectBalancePtr(0);

    if (whichAdmiral < 0) {
        throw Exception("Can't set consider ship for -1 admiral.");
    }
    Admiral* a = gAdmiralData.get() + whichAdmiral;
    a->considerShip() = whichObject;
    if (whichObject >= 0) {
        a->considerShipID() = anObject->id;
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
    } else {
        a->considerShipID() = -1;
    }
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

int32_t GetAdmiralConsiderObject(int32_t whichAdmiral) {
    Admiral* a = gAdmiralData.get() + whichAdmiral;
    SpaceObject* anObject;

    if (whichAdmiral < 0) {
        return -1;
    }
    if (a->considerShip() >= 0) {
        anObject = mGetSpaceObjectPtr(a->considerShip());
        if ((anObject->id == a->considerShipID())
                && (anObject->active == kObjectInUse)
                && (anObject->owner == whichAdmiral)) {
            return a->considerShip();
        } else {
            a->considerShip() = -1;
            a->considerShipID() = -1;
            return -1;
        }
    } else {
        if (a->considerShip() != -1) {
            throw Exception("Strange Admiral Consider Ship");
        }
        return a->considerShip();
    }
}

int32_t GetAdmiralBuildAtObject(int32_t whichAdmiral) {
    Admiral* a = gAdmiralData.get() + whichAdmiral;
    if (a->buildAtObject() >= 0) {
        destBalanceType* destBalance = mGetDestObjectBalancePtr(a->buildAtObject());
        if (destBalance->whichObject >= 0) {
            SpaceObject* anObject = mGetSpaceObjectPtr(destBalance->whichObject);
            if (anObject->owner != whichAdmiral) {
                a->buildAtObject() = kNoShip;
            }
        } else {
            a->buildAtObject() = kNoShip;
        }
    }
    return a->buildAtObject();
}

void SetAdmiralBuildAtObject(int32_t whichAdmiral, int32_t whichObject) {
    SpaceObject* anObject = mGetSpaceObjectPtr(whichObject);
    destBalanceType* d = mGetDestObjectBalancePtr(0);

    if (whichAdmiral < 0) {
        throw Exception("Can't set consider ship for -1 admiral.");
    }
    Admiral* a = gAdmiralData.get() + whichAdmiral;
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

void SetAdmiralBuildAtName(int32_t whichAdmiral, StringSlice name) {
    Admiral* a = gAdmiralData.get() + whichAdmiral;
    destBalanceType* destObject = mGetDestObjectBalancePtr(a->buildAtObject());
    destObject->name.assign(name.slice(0, min<size_t>(name.size(), kDestinationNameLen)));
}

StringSlice GetDestBalanceName(int32_t whichDestObject) {
    destBalanceType* destObject = mGetDestObjectBalancePtr(whichDestObject);
    return (destObject->name);
}

StringSlice GetAdmiralName(int32_t whichAdmiral) {
    if ((whichAdmiral >= 0) && (whichAdmiral < kMaxPlayerNum)) {
        Admiral* a = gAdmiralData.get() + whichAdmiral;
        return a->name();
    } else {
        return NULL;
    }
}

void SetObjectLocationDestination(SpaceObject *o, coordPointType *where) {
    // if the object does not have an alliance, then something is wrong here--forget it
    if (o->owner <= kNoOwner) {
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
    if ((o->owner < 0) || (o->owner >= kMaxPlayerNum)) {
        return;
    }

    Admiral* a = gAdmiralData.get() + o->owner;

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
    if (o->owner <= kNoOwner) {
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
    if ((o->owner < 0) || (o->owner >= kMaxPlayerNum)) {
        return;
    }

    // get the admiral
    Admiral* a = gAdmiralData.get() + o->owner;

    // if the admiral is not legal, or the admiral has no destination, then forget about it
    if ((dObject == NULL) &&
            ((!a->active())
             || (a->destType() == kNoDestinationType)
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

void AdmiralThink() {
    Admiral* a =gAdmiralData.get();
    SpaceObject* anObject;
    SpaceObject* destObject;
    SpaceObject* otherDestObject;
    SpaceObject* stepObject;
    destBalanceType* destBalance;
    int32_t origObject, origDest, baseNum, difference;
    Fixed  friendValue, foeValue, thisValue;
    BaseObject* baseObject;
    Point gridLoc;

    destBalance = mGetDestObjectBalancePtr(0);
    for (int i = 0; i < kMaxDestObject; i++) {
        destBalance->buildTime -= 10;
        if (destBalance->buildTime <= 0) {
            destBalance->buildTime = 0;
            if (destBalance->buildObjectBaseNum != kNoShip) {
                anObject = mGetSpaceObjectPtr(destBalance->whichObject);
                AdmiralBuildAtObject(anObject->owner, destBalance->buildObjectBaseNum, i);
                destBalance->buildObjectBaseNum = kNoShip;
            }
        }

        anObject = mGetSpaceObjectPtr(destBalance->whichObject);
        if (anObject && (anObject->owner >= 0)) {
            PayAdmiral(anObject->owner, destBalance->earn);
        }
        destBalance++;
    }

    for (int i = 0; i < kMaxPlayerNum; i++) {
        if ((a->attributes() & kAIsComputer) && (!(a->attributes() & kAIsRemote))) {
            if (a->blitzkrieg() > 0) {
                a->blitzkrieg()--;
                if (a->blitzkrieg() <= 0) {
                    // Really 48:
                    a->blitzkrieg() = 0 - (gRandomSeed.next(1200) + 1200);
                    for (int j = 0; j < kMaxSpaceObject; j++) {
                        anObject = mGetSpaceObjectPtr(j);
                        if (anObject->owner == i) {
                            anObject->currentTargetValue = 0x00000000;
                        }
                    }
                }
            } else {
                a->blitzkrieg()++;
                if (a->blitzkrieg() >= 0) {
                    // Really 48:
                    a->blitzkrieg() = gRandomSeed.next(1200) + 1200;
                    for (int j = 0; j < kMaxSpaceObject; j++) {
                        anObject = mGetSpaceObjectPtr(j);
                        if (anObject->owner == i) {
                            anObject->currentTargetValue = 0x00000000;
                        }
                    }
                }
            }

            // get the current object
            if (a->considerShip() < 0) {
                a->considerShip() = gRootObjectNumber;
                anObject = mGetSpaceObjectPtr(a->considerShip());
                a->considerShipID() = anObject->id;
            } else {
                anObject = mGetSpaceObjectPtr(a->considerShip());
            }

            if (a->destinationObject() < 0) {
                a->destinationObject() = gRootObjectNumber;
            }

            if (anObject->active != kObjectInUse) {
                a->considerShip() = gRootObjectNumber;
                anObject = mGetSpaceObjectPtr(a->considerShip());
                a->considerShipID() = anObject->id;
            }

            if (a->destinationObject() >= 0) {
                destObject = mGetSpaceObjectPtr(a->destinationObject());
                if (destObject->active != kObjectInUse) {
                    destObject = gRootObject;
                    a->destinationObject() = gRootObjectNumber;
                }
                origDest = a->destinationObject();
                do {
                    a->destinationObject() = destObject->nextObjectNumber;

                    // if we've gone through all of the objects
                    if (a->destinationObject() < 0) {
                        // ********************************
                        // SHIP MUST DECIDE, THEN INCREASE CONSIDER SHIP
                        // ********************************
                        if ((anObject->duty != eEscortDuty)
                                && (anObject->duty != eHostileBaseDuty)
                                && (anObject->bestConsideredTargetValue >
                                    anObject->currentTargetValue)) {
                            a->destinationObject() = anObject->bestConsideredTargetNumber;
                            a->destType() = kObjectDestinationType;
                            if (a->destinationObject() >= 0) {
                                destObject = mGetSpaceObjectPtr(a->destinationObject());
                                if (destObject->active == kObjectInUse) {
                                    a->destinationObjectID() = destObject->id;
                                    anObject->currentTargetValue
                                        = anObject->bestConsideredTargetValue;
                                    thisValue = anObject->randomSeed.next(
                                            mFloatToFixed(0.5))
                                        - mFloatToFixed(0.25);
                                    thisValue = mMultiplyFixed(
                                            thisValue, anObject->currentTargetValue);
                                    anObject->currentTargetValue += thisValue;
                                    SetObjectDestination(anObject, NULL);
                                }
                            }
                            a->destType() = kNoDestinationType;
                        }

                        if ((anObject->duty != eEscortDuty)
                                && (anObject->duty != eHostileBaseDuty)) {
                            a->thisFreeEscortStrength() += anObject->baseType->offenseValue;
                        }

                        anObject->bestConsideredTargetValue = 0xffffffff;
                        // start back with 1st ship
                        a->destinationObject() = gRootObjectNumber;
                        destObject = gRootObject;

                        // >>> INCREASE CONSIDER SHIP
                        origObject = a->considerShip();
                        anObject = mGetSpaceObjectPtr(a->considerShip());
                        if (anObject->active != kObjectInUse) {
                            anObject = gRootObject;
                            a->considerShip() = gRootObjectNumber;
                            a->considerShipID() = anObject->id;
                        }
                        do {
                            a->considerShip() = anObject->nextObjectNumber;
                            if (a->considerShip() < 0) {
                                a->considerShip() = gRootObjectNumber;
                                anObject = gRootObject;
                                a->considerShipID() = anObject->id;
                                a->lastFreeEscortStrength() = a->thisFreeEscortStrength();
                                a->thisFreeEscortStrength() = 0;
                            } else {
                                anObject = anObject->nextObject;
                                a->considerShipID() = anObject->id;
                            }
                        } while (((anObject->owner != i)
                                    || (!(anObject->attributes & kCanAcceptDestination))
                                    || (anObject->active != kObjectInUse))
                                && (a->considerShip() != origObject));
                    } else {
                        destObject = destObject->nextObject;
                    }
                    a->destinationObjectID() = destObject->id;
                } while (((!(destObject->attributes & (kCanBeDestination)))
                            || (a->destinationObject() == a->considerShip())
                            || (destObject->active != kObjectInUse)
                            || (!(destObject->attributes & kCanBeDestination)))
                        && (a->destinationObject() != origDest));

            // if our object is legal and our destination is legal
            if ((anObject->owner == i)
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
                    stepObject = stepObject->nextFarObject;
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
                            if ((a->blitzkrieg() > 0) && (anObject->duty == eGuardDuty)) {
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
                } else if (destObject->owner >= 0) {
                    if ((anObject->duty == eGuardDuty) || (anObject->duty == eNoDuty)) {
                        if (destObject->attributes & kIsDestination) {
                            if (foeValue < friendValue) {
                                thisValue = kMostImportantTarget;
                            } else {
                                thisValue = kSomewhatImportantTarget;
                            }
                            if (a->blitzkrieg() > 0) {
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
                        if (a->blitzkrieg() > 0) {
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
                    anObject->bestConsideredTargetNumber = a->destinationObject();
                }
            }
        }

        // if we've saved enough for our dreams
        if (a->cash() > a->saveGoal()) {
                a->saveGoal() = 0;

                // consider what ship to build
                if (a->buildAtObject() < 0) {
                    a->buildAtObject() = 0;
                }
                origDest = a->buildAtObject();
                destBalance = mGetDestObjectBalancePtr(a->buildAtObject());

                // try to find the next destination object that we own & that can build
                do {
                    a->buildAtObject()++;
                    destBalance++;
                    if (a->buildAtObject() >= kMaxDestObject) {
                        a->buildAtObject() = 0;
                        destBalance = mGetDestObjectBalancePtr(0);
                    }
                    if (destBalance->whichObject >= 0) {
                        anObject = mGetSpaceObjectPtr(destBalance->whichObject);
                        if ((anObject->owner != i)
                                || (!(anObject->attributes & kCanAcceptBuild))) {
                            anObject = NULL;
                        }
                    } else anObject = NULL;
                } while ((anObject == NULL) && (a->buildAtObject() != origDest));

                // if we have a legal object
                if (anObject != NULL) {
                    if (destBalance->buildTime <= 0) {
                        if (a->hopeToBuild() < 0) {
                            int k = 0;
                            while ((a->hopeToBuild() < 0) && (k < 7)) {
                                k++;
                                // choose something to build
                                thisValue = gRandomSeed.next(a->totalBuildChance());
                                friendValue = 0xffffffff; // equals the highest qualifying object
                                for (int j = 0; j < kMaxNumAdmiralCanBuild; ++j) {
                                    if ((a->canBuildType()[j].chanceRange <= thisValue)
                                            && (a->canBuildType()[j].chanceRange > friendValue)) {
                                        friendValue = a->canBuildType()[j].chanceRange;
                                        a->hopeToBuild() = a->canBuildType()[j].baseNum;
                                    }
                                }
                                if (a->hopeToBuild() >= 0) {
                                    mGetBaseObjectFromClassRace(
                                            baseObject, baseNum, a->hopeToBuild(), a->race());
                                    if (baseObject->buildFlags & kSufficientEscortsExist) {
                                        for (int j = 0; j < kMaxSpaceObject; ++j) {
                                            anObject = mGetSpaceObjectPtr(j);
                                            if ((anObject->active)
                                                    && (anObject->owner == i)
                                                    && (anObject->whichBaseObject == baseNum)
                                                    && (anObject->escortStrength <
                                                        baseObject->friendDefecit)) {
                                                a->hopeToBuild() = -1;
                                                j = kMaxSpaceObject;
                                            }
                                        }
                                    }

                                    if (baseObject->buildFlags & kMatchingFoeExists) {
                                        thisValue = 0;
                                        for (int j = 0; j < kMaxSpaceObject; j++) {
                                            anObject = mGetSpaceObjectPtr(j);
                                            if ((anObject->active)
                                                    && (anObject->owner != i)
                                                    && (anObject->baseType->levelKeyTag
                                                        == baseObject->orderKeyTag)) {
                                                thisValue = 1;
                                            }
                                        }
                                        if (!thisValue) {
                                            a->hopeToBuild() = -1;
                                        }
                                    }
                                }
                            }
                        }
                        int j = 0;
                        while ((destBalance->canBuildType[j] != a->hopeToBuild())
                                && (j < kMaxTypeBaseCanBuild)) {
                            j++;
                        }
                        if ((j < kMaxTypeBaseCanBuild) && (a->hopeToBuild() != kNoShip)) {
                            mGetBaseObjectFromClassRace(baseObject, baseNum, a->hopeToBuild(),
                                    a->race());
                            if (a->cash() >= mLongToFixed(baseObject->price)) {
                                AdmiralScheduleBuild(i, j);
                                a->hopeToBuild() = -1;
                                a->saveGoal() = 0;
                            } else {
                                a->saveGoal() = mLongToFixed(baseObject->price);
                            }
                        } // otherwise just wait until we get to it
                    }
                }
            }
        }
        a++;
    }
}

// assumes you can afford it & base has time
void AdmiralBuildAtObject(int32_t whichAdmiral, int32_t baseTypeNum, int32_t whichDestObject) {
    Admiral* admiral = gAdmiralData.get() + whichAdmiral;
    destBalanceType* buildAtDest = mGetDestObjectBalancePtr(whichDestObject);
    SpaceObject* buildAtObject = NULL;
    coordPointType  coord;
    fixedPointType  v = {0, 0};

    if ((baseTypeNum >= 0) && (admiral->buildAtObject() >= 0)) {
        buildAtObject = mGetSpaceObjectPtr(buildAtDest->whichObject);
        coord = buildAtObject->location;

        SpaceObject* newObject = CreateAnySpaceObject(
                baseTypeNum, &v, &coord, 0, whichAdmiral, 0, -1);
        if (newObject) {
            SetObjectDestination(newObject, NULL);
            if (whichAdmiral == globals()->gPlayerAdmiralNumber) {
                PlayVolumeSound(kComputerBeep2, kMediumVolume, kMediumPersistence,
                        kLowPrioritySound);
            }
        }
    }
}

bool AdmiralScheduleBuild(int32_t whichAdmiral, int32_t buildWhichType) {
    Admiral* admiral = gAdmiralData.get() + whichAdmiral;
    destBalanceType* buildAtDest = mGetDestObjectBalancePtr(admiral->buildAtObject());
    BaseObject* buildBaseObject = NULL;
    int32_t            baseNum;

    GetAdmiralBuildAtObject(whichAdmiral);
    if ((buildWhichType >= 0)
            && (buildWhichType < kMaxTypeBaseCanBuild)
            && (admiral->buildAtObject() >= 0) && (buildAtDest->buildTime <= 0)) {
        mGetBaseObjectFromClassRace(buildBaseObject, baseNum,
                buildAtDest->canBuildType[buildWhichType], admiral->race());
        if ((buildBaseObject != NULL) && (buildBaseObject->price <= mFixedToLong(admiral->cash()))) {
            admiral->cash() -= (mLongToFixed(buildBaseObject->price));
            if (globals()->gActiveCheats[whichAdmiral] & kBuildFastBit) {
                buildAtDest->buildTime = 9;
                buildAtDest->totalBuildTime = 9;
            } else {
                buildAtDest->buildTime = buildBaseObject->buildTime;
                buildAtDest->totalBuildTime = buildAtDest->buildTime;
            }
            buildAtDest->buildObjectBaseNum = baseNum;
            return true;
        }
    }
    return false;
}

void StopBuilding(int32_t whichDestObject) {
    destBalanceType* destObject;

    destObject = mGetDestObjectBalancePtr(whichDestObject);
    destObject->totalBuildTime = destObject->buildTime = 0;
    destObject->buildObjectBaseNum = kNoShip;
}

void PayAdmiral(int32_t whichAdmiral, Fixed howMuch) {
    Admiral* admiral = gAdmiralData.get() + whichAdmiral;

    if ((whichAdmiral >= 0) && (whichAdmiral < kMaxPlayerNum)) {
        admiral->cash() += mMultiplyFixed(howMuch, admiral->earningPower());
    }
}

void PayAdmiralAbsolute(int32_t whichAdmiral, Fixed howMuch) {
    Admiral* admiral = gAdmiralData.get() + whichAdmiral;

    if ((whichAdmiral >= 0) && (whichAdmiral < kMaxPlayerNum)) {
        admiral->cash() += howMuch;
        if (admiral->cash() < 0) {
            admiral->cash() = 0;
        }
    }
}

void AlterAdmiralScore(int32_t whichAdmiral, int32_t whichScore, int32_t amount) {
    Admiral* admiral = gAdmiralData.get() + whichAdmiral;

    if ((whichAdmiral >= 0) && (whichAdmiral < kMaxPlayerNum)
            && (whichScore >= 0) && (whichScore < kAdmiralScoreNum)) {
        admiral->score()[whichScore] += amount;
    }
}

int32_t GetAdmiralScore(int32_t whichAdmiral, int32_t whichScore) {
    Admiral* admiral = gAdmiralData.get() + whichAdmiral;

    if ((whichAdmiral >= 0) && (whichAdmiral < kMaxPlayerNum)
            && (whichScore >= 0) && (whichScore < kAdmiralScoreNum)) {
        return admiral->score()[whichScore];
    } else {
        return 0;
    }
}

int32_t GetAdmiralShipsLeft(int32_t whichAdmiral) {
    Admiral* admiral = gAdmiralData.get() + whichAdmiral;

    if ((whichAdmiral >= 0) && (whichAdmiral < kMaxPlayerNum)) {
        return admiral->shipsLeft();
    } else {
        return 0;
    }
}

int32_t AlterDestinationObjectOccupation(int32_t whichDestination, int32_t whichAdmiral, int32_t amount) {
    destBalanceType* d = mGetDestObjectBalancePtr(whichDestination);

    if (whichAdmiral >= 0) {
        d->occupied[whichAdmiral] += amount;
        return(d->occupied[whichAdmiral]);
    } else {
        return -1;
    }
}

void ClearAllOccupants(int32_t whichDestination, int32_t whichAdmiral, int32_t fullAmount) {
    destBalanceType* d = mGetDestObjectBalancePtr(whichDestination);

    for (int i = 0; i < kMaxPlayerNum; i++) {
        d->occupied[i] = 0;
    }
    if (whichAdmiral >= 0) {
        d->occupied[whichAdmiral] = fullAmount;
    }
}

void AddKillToAdmiral(SpaceObject* anObject) {
    // only for player
    Admiral* admiral = gAdmiralData.get() + globals()->gPlayerAdmiralNumber;

    if (anObject->attributes & kCanAcceptDestination) {
        if (anObject->owner == globals()->gPlayerAdmiralNumber) {
            admiral->losses()++;
        } else {
            admiral->kills()++;
        }
    }
}

int32_t GetAdmiralLoss(int32_t whichAdmiral) {
    Admiral* admiral = gAdmiralData.get() + whichAdmiral;

    if ((whichAdmiral >= 0) && (whichAdmiral < kMaxPlayerNum)) {
        return admiral->losses();
    } else {
        return 0;
    }
}

int32_t GetAdmiralKill(int32_t whichAdmiral) {
    Admiral* admiral = gAdmiralData.get() + whichAdmiral;

    if ((whichAdmiral >= 0) && (whichAdmiral < kMaxPlayerNum)) {
        return admiral->kills();
    } else {
        return 0;
    }
}

}  // namespace antares

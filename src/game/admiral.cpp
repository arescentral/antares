// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// Copyright (C) 2008-2017 The Antares Authors
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

#include "data/base-object.hpp"
#include "data/races.hpp"
#include "data/resource.hpp"
#include "game/cheat.hpp"
#include "game/globals.hpp"
#include "game/space-object.hpp"
#include "game/sys.hpp"
#include "lang/casts.hpp"
#include "math/macros.hpp"
#include "math/random.hpp"
#include "math/units.hpp"
#include "sound/fx.hpp"

using std::min;
using std::unique_ptr;

namespace antares {

static const int32_t kDestinationNameLen = 17;
static const int32_t kAdmiralNameLen     = 31;

static const Fixed kUnimportantTarget       = Fixed::from_float(0.000);
static const Fixed kMostImportantTarget     = Fixed::from_float(2.000);
static const Fixed kLeastImportantTarget    = Fixed::from_float(1.000);
static const Fixed kVeryImportantTarget     = Fixed::from_float(1.375);
static const Fixed kImportantTarget         = Fixed::from_float(1.250);
static const Fixed kSomewhatImportantTarget = Fixed::from_float(1.125);
static const Fixed kAbsolutelyEssential     = Fixed::from_float(128.0);

static bool could_target_per_destination_flag(const BaseObject& base, const SpaceObject& target) {
    return base.ai.target.force.base.has_value()
                   ? (!!(target.attributes & kIsDestination) == *base.ai.target.force.base)
                   : true;
}

static bool could_target_per_owner(
        const Admiral& a, const BaseObject& base, const SpaceObject& target) {
    switch (base.ai.target.force.owner) {
        case Owner::ANY: return true;
        case Owner::DIFFERENT: return target.owner.get() != &a;
        case Owner::SAME: return target.owner.get() == &a;
    }
}

static bool could_target(const Admiral& a, const BaseObject& base, const SpaceObject& target) {
    return could_target_per_destination_flag(base, target) &&
           could_target_per_owner(a, base, target) &&
           tags_match(*target.base, base.ai.target.force.tags);
}

void Admiral::init() {
    g.admirals.reset(new Admiral[kMaxPlayerNum]);
    reset();
    g.destinations.reset(new Destination[kMaxDestObject]);
    ResetAllDestObjectData();
}

void Admiral::reset() {
    for (auto a : Admiral::all()) {
        *a = Admiral();
    }
}

void ResetAllDestObjectData() {
    for (auto d : Destination::all()) {
        d->whichObject = SpaceObject::none();
        d->name.clear();
        d->earn           = Fixed::zero();
        d->totalBuildTime = d->buildTime = ticks(0);
        d->buildObjectBaseNum            = nullptr;
        d->canBuildType.clear();
        for (int j = 0; j < kMaxPlayerNum; ++j) {
            d->occupied[j] = 0;
        }
    }
}

Destination* Destination::get(int i) {
    if ((0 <= i) && (i < kMaxDestObject)) {
        return &g.destinations[i];
    }
    return nullptr;
}

bool Destination::can_build() const { return !canBuildType.empty(); }

Admiral* Admiral::get(int i) {
    if ((0 <= i) && (i < kMaxPlayerNum)) {
        return &g.admirals[i];
    }
    return nullptr;
}

Handle<Admiral> Admiral::make(int index, const DemoLevel::Player& player) {
    return make(index, kAIsComputer, player.name, player.earning_power, player.race, player.hue);
}

Handle<Admiral> Admiral::make(int index, const SoloLevel::Player& player) {
    return make(
            index, player.type == LevelBase::PlayerType::HUMAN ? kAIsHuman : kAIsComputer,
            player.name, player.earning_power, player.race, player.hue);
}

Handle<Admiral> Admiral::make(
        int index, uint32_t attributes, pn::string_view name, sfz::optional<Fixed> earning_power,
        const NamedHandle<const Race>& race, sfz::optional<Hue> hue) {
    Handle<Admiral> a(index);
    if (a->_active) {
        return none();
    }

    a->_active        = true;
    a->_attributes    = attributes;
    a->_earning_power = earning_power.value_or(Fixed::zero());
    a->_race          = race.copy();
    a->_hue           = hue.value_or(Hue::GRAY);

    if (!name.empty()) {
        if (pn::rune::count(name) > kAdmiralNameLen) {
            a->_name = pn::rune::slice(name, 0, kAdmiralNameLen).copy();
        } else {
            a->_name = name.copy();
        }
    }

    // for now set strategy balance to 0 -- we may want to calc this if player added on the fly?
    return a;
}

static Handle<Destination> next_free_destination() {
    for (auto d : Destination::all()) {
        if (!d->whichObject.get()) {
            return d;
        }
    }
    return Destination::none();
}

Handle<Destination> MakeNewDestination(
        Handle<SpaceObject> object, const std::vector<BuildableObject>& canBuildType, Fixed earn,
        const sfz::optional<pn::string>& name) {
    auto d = next_free_destination();
    if (!d.get()) {
        return Destination::none();
    }

    d->whichObject    = object;
    d->earn           = earn;
    d->totalBuildTime = d->buildTime = ticks(0);
    d->canBuildType.clear();
    for (const BuildableObject& o : canBuildType) {
        d->canBuildType.emplace_back(BuildableObject{o.name.copy()});
    }

    if (name.has_value()) {
        if (pn::rune::count(*name) > kAdmiralNameLen) {
            d->name = pn::rune::slice(*name, 0, kAdmiralNameLen).copy();
        } else {
            d->name = name->copy();
        }
    }

    if (object->attributes & kNeutralDeath) {
        for (int j = 0; j < kMaxPlayerNum; j++) {
            d->occupied[j] = 0;
        }

        if (object->owner.get()) {
            d->occupied[object->owner.number()] = object->base->occupy_count;
        }
    }

    return d;
}

void Admiral::remove_destination(Handle<Destination> d) {
    if (_active) {
        if (_destinationObject == d->whichObject) {
            _destinationObject   = SpaceObject::none();
            _destinationObjectID = -1;
            _has_destination     = false;
        }
        if (_considerDestination == d.number()) {
            _considerDestination = kNoDestinationObject;
        }

        if (_buildAtObject == d) {
            _buildAtObject = Destination::none();
        }
    }
}

void RemoveDestination(Handle<Destination> d) {
    if (!d.get()) {
        return;
    }
    for (auto a : Admiral::all()) {
        a->remove_destination(d);
    }

    d->whichObject = SpaceObject::none();
    d->name.clear();
    d->earn           = Fixed::zero();
    d->totalBuildTime = d->buildTime = ticks(0);
    d->buildObjectBaseNum            = nullptr;
    d->canBuildType.clear();

    for (int i = 0; i < kMaxPlayerNum; i++) {
        d->occupied[i] = 0;
    }
}

void RecalcAllAdmiralBuildData() {
    // first clear all the data
    for (Handle<Admiral> a : Admiral::all()) {
        a->canBuildType().clear();
        a->totalBuildChance() = Fixed::zero();
        a->hopeToBuild().reset();
    }

    for (Handle<Destination> d : Destination::all()) {
        Handle<SpaceObject> anObject = d->whichObject;
        if (!anObject.get() || !anObject->owner.get()) {
            continue;
        }
        Handle<Admiral> a = anObject->owner;
        for (const BuildableObject& buildable_class : d->canBuildType) {
            bool found = false;
            for (const admiralBuildType& admiral_build : a->canBuildType()) {
                if (admiral_build.buildable.name == buildable_class.name) {
                    found = true;
                    break;
                }
            }
            if (found) {
                continue;
            }
            auto baseObject = get_buildable_object(buildable_class, a->race());
            if (baseObject) {
                a->canBuildType().emplace_back();
                a->canBuildType().back().buildable = BuildableObject{buildable_class.name.copy()};
                a->canBuildType().back().base      = baseObject;
                a->canBuildType().back().chanceRange = a->totalBuildChance();
                a->totalBuildChance() += baseObject->ai.build.ratio;
            }
        }
    }
}

Hue GetAdmiralColor(Handle<Admiral> a) {
    if (!a.get()) {
        return Hue::GRAY;
    }
    return a->hue();
}

void Admiral::set_target(Handle<SpaceObject> obj) {
    _destinationObject = obj;
    if (obj.get()) {
        _destinationObjectID = obj->id;
    } else {
        _destinationObjectID = -1;
    }
    _has_destination = true;
}

Handle<SpaceObject> Admiral::target() const {
    if (_destinationObject.get() && (_destinationObject->id == _destinationObjectID) &&
        (_destinationObject->active == kObjectInUse)) {
        return _destinationObject;
    }
    return SpaceObject::none();
}

void Admiral::set_control(Handle<SpaceObject> obj) {
    _considerShip = obj;
    if (obj.get()) {
        _considerShipID = obj->id;
        auto d          = obj->asDestination;
        if (d.get() && d->can_build()) {
            _buildAtObject = d;
        }
    } else {
        _considerShipID = -1;
    }
}

Handle<SpaceObject> Admiral::control() const {
    if (_considerShip.get() && (_considerShip->id == _considerShipID) &&
        (_considerShip->active == kObjectInUse) && (_considerShip->owner.get() == this)) {
        return _considerShip;
    }
    return SpaceObject::none();
}

bool BaseHasSomethingToBuild(Handle<SpaceObject> obj) {
    auto d = obj->asDestination;
    return d.get() && d->can_build();
}

Handle<Destination> GetAdmiralBuildAtObject(Handle<Admiral> a) {
    auto destBalance = a->buildAtObject();
    if (destBalance.get()) {
        if (destBalance->whichObject.get()) {
            auto anObject = destBalance->whichObject;
            if (anObject->owner != a) {
                a->buildAtObject() = Destination::none();
            }
        } else {
            a->buildAtObject() = Destination::none();
        }
    }
    return a->buildAtObject();
}

void SetAdmiralBuildAtObject(Handle<Admiral> a, Handle<SpaceObject> obj) {
    if (!a.get()) {
        throw std::runtime_error("Can't set consider ship for -1 admiral.");
    }
    if (obj.get()) {
        auto d = obj->asDestination;
        if (d.get() && d->can_build()) {
            a->buildAtObject() = d;
        }
    }
}

void SetAdmiralBuildAtName(Handle<Admiral> a, pn::string_view name) {
    auto   destObject = a->buildAtObject();
    size_t rune_count = 0;
    for (pn::string_view::iterator it = name.begin(); it != name.end(); ++it) {
        if (rune_count++ == kDestinationNameLen) {
            destObject->name = name.substr(0, it.offset()).copy();
            return;
        }
    }
    destObject->name = name.copy();
}

pn::string_view GetDestBalanceName(Handle<Destination> whichDestObject) {
    return whichDestObject->name;
}

pn::string_view GetAdmiralName(Handle<Admiral> a) {
    if (a.get()) {
        return a->name();
    } else {
        return pn::string_view{};
    }
}

void SetObjectLocationDestination(Handle<SpaceObject> o, coordPointType* where) {
    // if the object does not have an alliance, then something is wrong here--forget it
    if (o->owner.number() <= kNoOwner) {
        o->destObject            = SpaceObject::none();
        o->destObjectDest        = SpaceObject::none();
        o->destObjectID          = -1;
        o->destinationLocation.h = o->destinationLocation.v = kNoDestinationCoord;
        o->timeFromOrigin                                   = ticks(0);
        o->idealLocationCalc.h = o->idealLocationCalc.v = Fixed::zero();
        o->originLocation                               = o->location;
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
        o->destObject            = SpaceObject::none();
        o->destObjectDest        = SpaceObject::none();
        o->destinationLocation.h = o->destinationLocation.v = kNoDestinationCoord;
        o->timeFromOrigin                                   = ticks(0);
        o->idealLocationCalc.h = o->idealLocationCalc.v = Fixed::zero();
        o->originLocation                               = o->location;
    } else {
        // the object is OK, the admiral is OK, then go about setting its destination
        if (o->attributes & kCanAcceptDestination) {
            o->timeFromOrigin = kTimeToCheckHome;
        } else {
            o->timeFromOrigin = ticks(0);
        }

        // remove this object from its destination
        if (o->destObject.get()) {
            RemoveObjectFromDestination(o);
        }

        o->destinationLocation = o->originLocation = *where;
        o->destObject                              = SpaceObject::none();
        o->timeFromOrigin                          = ticks(0);
        o->idealLocationCalc.h = o->idealLocationCalc.v = Fixed::zero();
    }
}

void SetObjectDestination(Handle<SpaceObject> o) {
    OverrideObjectDestination(o, SpaceObject::none());
}

void OverrideObjectDestination(Handle<SpaceObject> o, Handle<SpaceObject> overrideObject) {
    auto dObject = overrideObject;

    // if the object does not have an alliance, then something is wrong here--forget it
    if (o->owner.number() <= kNoOwner) {
        o->destObject            = SpaceObject::none();
        o->destObjectDest        = SpaceObject::none();
        o->destObjectID          = -1;
        o->destinationLocation.h = o->destinationLocation.v = kNoDestinationCoord;
        o->timeFromOrigin                                   = ticks(0);
        o->idealLocationCalc.h = o->idealLocationCalc.v = Fixed::zero();
        o->originLocation                               = o->location;
        return;
    }

    // if this object can't accept a destination, then forget it
    if (!(o->attributes & kCanAcceptDestination)) {
        return;
    }

    // if this object has a locked destination, then forget it
    if ((o->attributes & kStaticDestination) && !overrideObject.get()) {
        return;
    }

    // if the owner is not legal, something is very very wrong
    if (!o->owner.get()) {
        return;
    }

    // get the admiral
    const auto& a = o->owner;

    // if the admiral is not legal, or the admiral has no destination, then forget about it
    if (!dObject.get() && ((!a->active()) || !a->has_destination() ||
                           !a->destinationObject().get() || (a->destinationObjectID() == o->id))) {
        o->destObject            = SpaceObject::none();
        o->destObjectDest        = SpaceObject::none();
        o->destinationLocation.h = o->destinationLocation.v = kNoDestinationCoord;
        o->timeFromOrigin                                   = ticks(0);
        o->idealLocationCalc.h = o->idealLocationCalc.v = Fixed::zero();
        o->originLocation                               = o->location;
    } else {
        // the object is OK, the admiral is OK, then go about setting its destination

        // first make sure we're still looking at the same object
        if (!dObject.get()) {
            dObject = a->destinationObject();
        }

        if ((dObject->active == kObjectInUse) &&
            ((dObject->id == a->destinationObjectID()) || overrideObject.get())) {
            if (o->attributes & kCanAcceptDestination) {
                o->timeFromOrigin = kTimeToCheckHome;
            } else {
                o->timeFromOrigin = ticks(0);
            }
            // remove this object from its destination
            if (o->destObject.get()) {
                RemoveObjectFromDestination(o);
            }

            // add this object to its destination
            if (o != dObject) {
                o->runTimeFlags &= ~kHasArrived;
                o->destObject       = dObject;
                o->destObjectDest   = dObject->destObject;
                o->destObjectDestID = dObject->destObjectID;
                o->destObjectID     = dObject->id;

                if (dObject->owner == o->owner) {
                    dObject->remoteFriendStrength += o->base->ai.escort.power;
                    dObject->escortStrength += o->base->ai.escort.power;
                    if (dObject->attributes & kIsDestination) {
                        if (dObject->escortStrength < dObject->base->ai.escort.need) {
                            o->duty = eGuardDuty;
                        } else {
                            o->duty = eNoDuty;
                        }
                    } else {
                        if (dObject->escortStrength < dObject->base->ai.escort.need) {
                            o->duty = eEscortDuty;
                        } else {
                            o->duty = eNoDuty;
                        }
                    }
                } else {
                    dObject->remoteFoeStrength += o->base->ai.escort.power;
                    if (dObject->attributes & kIsDestination) {
                        o->duty = eAssaultDuty;
                    } else {
                        o->duty = eAssaultDuty;
                    }
                }
            } else {
                o->destObject            = SpaceObject::none();
                o->destObjectDest        = SpaceObject::none();
                o->destinationLocation.h = o->destinationLocation.v = kNoDestinationCoord;
                o->timeFromOrigin                                   = ticks(0);
                o->idealLocationCalc.h = o->idealLocationCalc.v = Fixed::zero();
                o->originLocation                               = o->location;
            }
        } else {
            o->destObject            = SpaceObject::none();
            o->destObjectDest        = SpaceObject::none();
            o->destinationLocation.h = o->destinationLocation.v = kNoDestinationCoord;
            o->timeFromOrigin                                   = ticks(0);
            o->idealLocationCalc.h = o->idealLocationCalc.v = Fixed::zero();
            o->originLocation                               = o->location;
        }
    }
}

void RemoveObjectFromDestination(Handle<SpaceObject> o) {
    if (o->destObject.get()) {
        auto dObject = o->destObject;
        if (dObject->id == o->destObjectID) {
            if (dObject->owner == o->owner) {
                dObject->remoteFriendStrength -= o->base->ai.escort.power;
                dObject->escortStrength -= o->base->ai.escort.power;
            } else {
                dObject->remoteFoeStrength -= o->base->ai.escort.power;
            }
        }
    }

    o->destObject     = SpaceObject::none();
    o->destObjectDest = SpaceObject::none();
    o->destObjectID   = -1;
}

// assumes you can afford it & base has time
static void AdmiralBuildAtObject(
        Handle<Admiral> admiral, const BaseObject* base, Handle<Destination> buildAtDest) {
    fixedPointType v = {Fixed::zero(), Fixed::zero()};
    if (base) {
        auto coord = buildAtDest->whichObject->location;

        auto newObject = CreateAnySpaceObject(*base, &v, &coord, 0, admiral, 0, sfz::nullopt);
        if (newObject.get()) {
            SetObjectDestination(newObject);
            if (admiral == g.admiral) {
                sys.sound.build();
            }
        }
    }
}

void AdmiralThink() {
    for (auto destBalance : Destination::all()) {
        destBalance->buildTime -= kMajorTick;
        if (destBalance->buildTime <= ticks(0)) {
            destBalance->buildTime = ticks(0);
            if (destBalance->buildObjectBaseNum) {
                auto anObject = destBalance->whichObject;
                AdmiralBuildAtObject(
                        anObject->owner, destBalance->buildObjectBaseNum, destBalance);
                destBalance->buildObjectBaseNum = nullptr;
            }
        }

        auto anObject = destBalance->whichObject;
        if (anObject.get() && anObject->owner.get()) {
            anObject->owner->pay(Cash{destBalance->earn});
        }
    }

    for (auto a : Admiral::all()) {
        a->think();
    }
}

void Admiral::think() {
    Handle<SpaceObject> anObject;
    Handle<SpaceObject> destObject;
    Handle<SpaceObject> otherDestObject;
    Handle<SpaceObject> stepObject;
    Handle<SpaceObject> origObject;
    int32_t             difference;
    Fixed               friendValue, foeValue, thisValue;
    Point               gridLoc;

    if (!(_attributes & kAIsComputer) || (_attributes & kAIsRemote)) {
        return;
    }

    if (_blitzkrieg > 0) {
        _blitzkrieg--;
        if (_blitzkrieg <= 0) {
            // Really 48:
            _blitzkrieg = 0 - (g.random.next(1200) + 1200);
            for (auto anObject : SpaceObject::all()) {
                if (anObject->owner.get() == this) {
                    anObject->currentTargetValue = Fixed::zero();
                }
            }
        }
    } else {
        _blitzkrieg++;
        if (_blitzkrieg >= 0) {
            // Really 48:
            _blitzkrieg = g.random.next(1200) + 1200;
            for (auto anObject : SpaceObject::all()) {
                if (anObject->owner.get() == this) {
                    anObject->currentTargetValue = Fixed::zero();
                }
            }
        }
    }

    // get the current object
    if (!_considerShip.get()) {
        _considerShip = anObject = g.root;
        _considerShipID          = anObject->id;
    } else {
        anObject = _considerShip;
    }

    if (!_destinationObject.get()) {
        _destinationObject = g.root;
    }

    if (anObject->active != kObjectInUse) {
        _considerShip = anObject = g.root;
        _considerShipID          = anObject->id;
    }

    if (_destinationObject.get()) {
        destObject = _destinationObject;
        if (destObject->active != kObjectInUse) {
            destObject = _destinationObject = g.root;
        }
        auto origDest = _destinationObject;
        do {
            _destinationObject = destObject->nextObject;

            // if we've gone through all of the objects
            if (!_destinationObject.get()) {
                // ********************************
                // SHIP MUST DECIDE, THEN INCREASE CONSIDER SHIP
                // ********************************
                if ((anObject->duty != eEscortDuty) && (anObject->duty != eHostileBaseDuty) &&
                    (anObject->bestConsideredTargetValue > anObject->currentTargetValue)) {
                    _destinationObject = anObject->bestConsideredTargetNumber;
                    _has_destination   = true;
                    if (_destinationObject.get()) {
                        destObject = _destinationObject;
                        if (destObject->active == kObjectInUse) {
                            _destinationObjectID         = destObject->id;
                            anObject->currentTargetValue = anObject->bestConsideredTargetValue;
                            thisValue = anObject->randomSeed.next(Fixed::from_float(0.5)) -
                                        Fixed::from_float(0.25);
                            thisValue = (thisValue * anObject->currentTargetValue);
                            anObject->currentTargetValue += thisValue;
                            SetObjectDestination(anObject);
                        }
                    }
                    _has_destination = false;
                }

                if ((anObject->duty != eEscortDuty) && (anObject->duty != eHostileBaseDuty)) {
                    _thisFreeEscortStrength += anObject->base->ai.escort.power;
                }

                anObject->bestConsideredTargetValue = kFixedNone;
                // start back with 1st ship
                _destinationObject = g.root;
                destObject         = g.root;

                // >>> INCREASE CONSIDER SHIP
                origObject = anObject = _considerShip;
                if (anObject->active != kObjectInUse) {
                    anObject        = g.root;
                    _considerShip   = g.root;
                    _considerShipID = anObject->id;
                }
                do {
                    _considerShip = anObject->nextObject;
                    if (!_considerShip.get()) {
                        _considerShip           = g.root;
                        anObject                = g.root;
                        _considerShipID         = anObject->id;
                        _lastFreeEscortStrength = _thisFreeEscortStrength;
                        _thisFreeEscortStrength = Fixed::zero();
                    } else {
                        anObject        = anObject->nextObject;
                        _considerShipID = anObject->id;
                    }
                } while (((anObject->owner.get() != this) ||
                          (!(anObject->attributes & kCanAcceptDestination)) ||
                          (anObject->active != kObjectInUse)) &&
                         (_considerShip != origObject));
            } else {
                destObject = destObject->nextObject;
            }
            _destinationObjectID = destObject->id;
        } while (((!(destObject->attributes & (kCanBeDestination))) ||
                  (_destinationObject == _considerShip) || (destObject->active != kObjectInUse) ||
                  (!(destObject->attributes & kCanBeDestination))) &&
                 (_destinationObject != origDest));

        // if our object is legal and our destination is legal
        if ((anObject->owner.get() == this) && (anObject->attributes & kCanAcceptDestination) &&
            (anObject->active == kObjectInUse) && (destObject->attributes & (kCanBeDestination)) &&
            (destObject->active == kObjectInUse) &&
            ((anObject->owner != destObject->owner) ||
             (anObject->base->ai.escort.class_ < destObject->base->ai.escort.class_))) {
            gridLoc    = destObject->distanceGrid;
            stepObject = otherDestObject = destObject;
            while (stepObject->nextFarObject.get()) {
                if ((stepObject->distanceGrid.h == gridLoc.h) &&
                    (stepObject->distanceGrid.v == gridLoc.v)) {
                    otherDestObject = stepObject;
                }
                stepObject = stepObject->nextFarObject;
            }
            if (otherDestObject->owner == anObject->owner) {
                friendValue = otherDestObject->localFriendStrength;
                foeValue    = otherDestObject->localFoeStrength;
            } else {
                foeValue    = otherDestObject->localFriendStrength;
                friendValue = otherDestObject->localFoeStrength;
            }

            thisValue = kUnimportantTarget;
            if (destObject->owner == anObject->owner) {
                if (destObject->attributes & kIsDestination) {
                    if (destObject->escortStrength < destObject->base->ai.escort.need) {
                        thisValue = kAbsolutelyEssential;
                    } else if (foeValue != Fixed::zero()) {
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
                            if (foeValue > Fixed::zero()) {
                                thisValue = kSomewhatImportantTarget;
                            } else {
                                thisValue = kUnimportantTarget;
                            }
                        }
                    }
                    if (anObject->base->orderFlags & kSoftTargetIsBase) {
                        thisValue <<= 3;
                    }
                    if (anObject->base->orderFlags & kHardTargetIsNotBase) {
                        thisValue = Fixed::zero();
                    }
                } else {
                    if (destObject->base->ai.escort.class_ > anObject->base->ai.escort.class_) {
                        if (foeValue > friendValue) {
                            thisValue = kMostImportantTarget;
                        } else {
                            if (destObject->escortStrength < destObject->base->ai.escort.need) {
                                thisValue = kMostImportantTarget;
                            } else {
                                thisValue = kUnimportantTarget;
                            }
                        }
                    } else {
                        thisValue = kUnimportantTarget;
                    }
                    if (anObject->base->orderFlags & kSoftTargetIsNotBase) {
                        thisValue <<= 3;
                    }
                    if (anObject->base->orderFlags & kHardTargetIsBase) {
                        thisValue = Fixed::zero();
                    }
                }
                if (anObject->base->orderFlags & kSoftTargetIsFriend) {
                    thisValue <<= 3;
                }
                if (anObject->base->orderFlags & kHardTargetIsFoe) {
                    thisValue = Fixed::zero();
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
                        if (anObject->base->orderFlags & kSoftTargetIsBase) {
                            thisValue <<= 3;
                        }

                        if (anObject->base->orderFlags & kHardTargetIsNotBase) {
                            thisValue = Fixed::zero();
                        }
                    } else {
                        if (friendValue != Fixed::zero()) {
                            if (friendValue < foeValue) {
                                thisValue = kSomewhatImportantTarget;
                            } else {
                                thisValue = kUnimportantTarget;
                            }
                        } else {
                            thisValue = kLeastImportantTarget;
                        }
                        if (anObject->base->orderFlags & kSoftTargetIsNotBase) {
                            thisValue <<= 1;
                        }

                        if (anObject->base->orderFlags & kHardTargetIsBase) {
                            thisValue = Fixed::zero();
                        }
                    }
                }
                if (anObject->base->orderFlags & kSoftTargetIsFoe) {
                    thisValue <<= 3;
                }
                if (anObject->base->orderFlags & kHardTargetIsFriend) {
                    thisValue = Fixed::zero();
                }
            } else {
                if (destObject->attributes & kIsDestination) {
                    thisValue = kVeryImportantTarget;
                    if (_blitzkrieg > 0) {
                        thisValue <<= 2;
                    }
                    if (anObject->base->orderFlags & kSoftTargetIsBase) {
                        thisValue <<= 3;
                    }
                    if (anObject->base->orderFlags & kHardTargetIsNotBase) {
                        thisValue = Fixed::zero();
                    }
                } else {
                    if (anObject->base->orderFlags & kSoftTargetIsNotBase) {
                        thisValue <<= 3;
                    }
                    if (anObject->base->orderFlags & kHardTargetIsBase) {
                        thisValue = Fixed::zero();
                    }
                }
                if (anObject->base->orderFlags & kSoftTargetIsFoe) {
                    thisValue <<= 3;
                }
                if (anObject->base->orderFlags & kHardTargetIsFriend) {
                    thisValue = Fixed::zero();
                }
            }

            difference =
                    ABS(implicit_cast<int32_t>(destObject->location.h) -
                        implicit_cast<int32_t>(anObject->location.h));
            gridLoc.h = difference;
            difference =
                    ABS(implicit_cast<int32_t>(destObject->location.v) -
                        implicit_cast<int32_t>(anObject->location.v));
            gridLoc.v = difference;

            if ((gridLoc.h < kMaximumRelevantDistance) && (gridLoc.v < kMaximumRelevantDistance)) {
                if (anObject->base->orderFlags & kSoftTargetIsLocal) {
                    thisValue <<= 3;
                }
                if (anObject->base->orderFlags & kHardTargetIsRemote) {
                    thisValue = Fixed::zero();
                }
            } else {
                if (anObject->base->orderFlags & kSoftTargetIsRemote) {
                    thisValue <<= 3;
                }
                if (anObject->base->orderFlags & kHardTargetIsLocal) {
                    thisValue = Fixed::zero();
                }
            }

            if (anObject->base->orderFlags & kSoftTargetMatchesTags) {
                if (tags_match(*destObject->base, anObject->base->ai.target.prefer.tags)) {
                    thisValue <<= 3;
                }
            }
            if (anObject->base->orderFlags & kHardTargetMatchesTags) {
                if (!tags_match(*destObject->base, anObject->base->ai.target.force.tags)) {
                    thisValue = Fixed::zero();
                }
            }

            if (thisValue > Fixed::zero()) {
                thisValue += anObject->randomSeed.next(thisValue >> 1) - (thisValue >> 2);
            }
            if (thisValue > anObject->bestConsideredTargetValue) {
                anObject->bestConsideredTargetValue  = thisValue;
                anObject->bestConsideredTargetNumber = _destinationObject;
            }
        }
    }
    think_build();
}

void Admiral::think_build() {
    // if we've saved enough for our dreams
    if (_cash <= _saveGoal) {
        return;
    }
    _saveGoal = Cash{Fixed::zero()};

    // consider what ship to build
    if (!_buildAtObject.get()) {
        _buildAtObject = Handle<Destination>(0);
    }

    // try to find the next destination object that we own & that can build
    auto anObject = SpaceObject::none();
    auto begin    = _buildAtObject.number() + 1;
    auto end      = begin + kMaxDestObject;
    for (int i = begin; i < end; ++i) {
        auto d = _buildAtObject = Handle<Destination>(i % kMaxDestObject);
        if (d->whichObject.get() && (d->whichObject->owner.get() == this) &&
            (d->whichObject->attributes & kCanAcceptBuild)) {
            anObject = d->whichObject;
            break;
        }
    }

    // if we have a legal object
    if (!anObject.get()) {
        return;
    }
    auto destBalance = anObject->asDestination;
    if (destBalance->buildTime > ticks(0)) {
        return;
    }

    if (!_hopeToBuild.has_value()) {
        int k = 0;
        while (!_hopeToBuild.has_value() && (k < 7)) {
            k++;
            // choose something to build
            Fixed thisValue   = g.random.next(_totalBuildChance);
            Fixed friendValue = kFixedNone;  // equals the highest qualifying object
            for (int j = 0; j < _canBuildType.size(); ++j) {
                if ((_canBuildType[j].chanceRange <= thisValue) &&
                    (_canBuildType[j].chanceRange > friendValue)) {
                    friendValue = _canBuildType[j].chanceRange;
                    _hopeToBuild.emplace(BuildableObject{_canBuildType[j].buildable.name.copy()});
                }
            }
            if (_hopeToBuild.has_value()) {
                // If “needs_escort” is set, don’t build a second object of this type
                // until the first one has sufficient escorts.
                auto baseObject = get_buildable_object(*_hopeToBuild, _race);
                if (baseObject->ai.build.needs_escort) {
                    for (auto anObject : SpaceObject::all()) {
                        if ((anObject->active) && (anObject->owner.get() == this) &&
                            (anObject->base == baseObject) &&
                            (anObject->escortStrength < baseObject->ai.escort.need)) {
                            _hopeToBuild.reset();
                            break;
                        }
                    }
                }

                // Don’t build an object if there are no valid targets for it.
                bool any_target = false;
                for (auto anObject : SpaceObject::all()) {
                    if (anObject->active && could_target(*this, *baseObject, *anObject)) {
                        any_target = true;
                        break;
                    }
                }
                if (!any_target) {
                    _hopeToBuild.reset();
                }
            }
        }
    }

    if (!_hopeToBuild.has_value()) {
        return;
    }

    int j = 0;
    for (; j < destBalance->canBuildType.size(); ++j) {
        if (destBalance->canBuildType[j].name == _hopeToBuild->name) {
            break;
        }
    }
    if (j >= destBalance->canBuildType.size()) {
        return;  // just wait until we get to it
    }

    auto baseObject = get_buildable_object(*_hopeToBuild, _race);
    if (_cash >= baseObject->price) {
        Admiral::build(j);
        _hopeToBuild.reset();
        _saveGoal = Cash{Fixed::zero()};
    } else {
        _saveGoal = baseObject->price;
    }
}

bool Admiral::build(int32_t buildWhichType) {
    auto dest = _buildAtObject;
    if ((buildWhichType >= 0) && (buildWhichType < dest->canBuildType.size()) && (dest.get()) &&
        (dest->buildTime <= ticks(0))) {
        auto buildBaseObject = get_buildable_object(dest->canBuildType[buildWhichType], _race);
        if (buildBaseObject && (buildBaseObject->price <= _cash)) {
            _cash.amount -= buildBaseObject->price.amount;
            if (_cheats & kBuildFastBit) {
                dest->buildTime      = kMinorTick;
                dest->totalBuildTime = kMinorTick;
            } else {
                dest->buildTime      = buildBaseObject->buildTime;
                dest->totalBuildTime = dest->buildTime;
            }
            dest->buildObjectBaseNum = buildBaseObject;
            return true;
        }
    }
    return false;
}

void StopBuilding(Handle<Destination> destObject) {
    destObject->totalBuildTime = destObject->buildTime = ticks(0);
    destObject->buildObjectBaseNum                     = nullptr;
}

void Admiral::pay(Cash howMuch) { pay_absolute(Cash{howMuch.amount * _earning_power}); }

void Admiral::pay_absolute(Cash howMuch) {
    _cash.amount += howMuch.amount;
    if (_cash < Cash{Fixed::zero()}) {
        _cash = Cash{Fixed::zero()};
    }
}

void AlterAdmiralScore(Counter counter, int32_t amount) {
    if (counter.player.get() && (counter.which >= 0) && (counter.which < kAdmiralScoreNum)) {
        counter.player->score()[counter.which] += amount;
    }
}

int32_t GetAdmiralScore(Counter counter) {
    if (counter.player.get() && (counter.which >= 0) && (counter.which < kAdmiralScoreNum)) {
        return counter.player->score()[counter.which];
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

int32_t AlterDestinationObjectOccupation(
        Handle<Destination> d, Handle<Admiral> a, int32_t amount) {
    if (a.get()) {
        d->occupied[a.number()] += amount;
        return d->occupied[a.number()];
    } else {
        return -1;
    }
}

void ClearAllOccupants(Handle<Destination> d, Handle<Admiral> a, int32_t fullAmount) {
    for (int i = 0; i < kMaxPlayerNum; i++) {
        d->occupied[i] = 0;
    }
    if (a.get()) {
        d->occupied[a.number()] = fullAmount;
    }
}

void AddKillToAdmiral(Handle<SpaceObject> anObject) {
    // only for player
    const auto& admiral = g.admiral;

    if (anObject->attributes & kCanAcceptDestination) {
        if (anObject->owner == g.admiral) {
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

// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// Copyright (C) 2016-2017 The Antares Authors
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

#include "game/initial.hpp"

#include "data/plugin.hpp"
#include "game/admiral.hpp"
#include "game/globals.hpp"
#include "game/level.hpp"
#include "game/player-ship.hpp"
#include "game/space-object.hpp"

namespace antares {

const Initial* Initial::get(int number) {
    if ((0 <= number) && (number < g.level->base.initials.size())) {
        return &g.level->base.initials[number];
    }
    return nullptr;
}

HandleList<const Initial> Initial::all() {
    return HandleList<const Initial>(0, g.level->base.initials.size());
}

void create_initial(Handle<const Initial> initial) {
    if (initial->hide.value_or(false)) {
        g.initials[initial.number()] = SpaceObject::none();
        return;
    }

    coordPointType coord = Translate_Coord_To_Level_Rotation(initial->at.h, initial->at.v);

    Handle<Admiral> owner = initial->owner.value_or(Admiral::none());

    int32_t attributes = 0;
    if (initial->flagship.value_or(false)) {
        if ((owner == g.admiral) && !owner->flagship().get()) {
            attributes |= kIsPlayerShip;
        }
    }
    if (initial->target.lock.value_or(false)) {
        attributes |= kStaticDestination;
    }

    const BaseObject* base =
            initial->owner.has_value()
                    ? get_buildable_object(initial->base, (*initial->owner)->race())
                    : BaseObject::get(initial->base.name);
    // TODO(sfiera): remap object in networked games.
    fixedPointType v        = {Fixed::zero(), Fixed::zero()};
    auto           anObject = g.initials[initial.number()] = CreateAnySpaceObject(
            *base, &v, &coord, g.angle, owner, attributes,
            initial->override_.sprite.has_value()
                    ? sfz::make_optional<pn::string_view>(*initial->override_.sprite)
                    : sfz::nullopt);

    if (anObject->attributes & kIsDestination) {
        anObject->asDestination = MakeNewDestination(
                anObject, initial->build, initial->earning.value_or(Fixed::zero()),
                initial->override_.name);
    }
    g.initial_ids[initial.number()] = anObject->id;

    if ((anObject->attributes & kIsPlayerShip) && owner.get() && !owner->flagship().get()) {
        owner->set_flagship(anObject);
        if (owner == g.admiral) {
            g.ship = anObject;
            ResetPlayerShip();
        }
    }

    if (anObject->attributes & kIsDestination) {
        if (owner.get()) {
            if (!initial->build.empty()) {
                if (!GetAdmiralBuildAtObject(owner).get()) {
                    owner->set_control(anObject);
                    owner->set_target(anObject);
                }
            }
        }
    }
}

void set_initial_destination(Handle<const Initial> initial, bool preserve) {
    auto object = g.initials[initial.number()];
    if (!object.get()                              // hasn't been created yet
        || (!initial->target.initial.has_value())  // doesn't have a target
        || (!initial->owner.has_value())) {        // doesn't have an owner
        return;
    }

    // get the correct admiral #
    Handle<Admiral> owner = initial->owner.value_or(Admiral::none());

    const auto& target = g.initials[initial->target.initial->number()];
    if (target.get()) {
        auto saveDest = owner->target();  // save the original dest

        // set the admiral's dest object to the mapped initial dest object
        owner->set_target(target);

        // now give the mapped initial object the admiral's destination

        uint32_t specialAttributes = object->attributes;  // preserve the attributes
        object->attributes &=
                ~kStaticDestination;  // we've got to force this off so we can set dest
        SetObjectDestination(object);
        object->attributes = specialAttributes;

        if (preserve) {
            owner->set_target(saveDest);
        }
    }
}

void UnhideInitialObject(Handle<const Initial> initial) {
    if (GetObjectFromInitialNumber(initial).get()) {
        return;  // Already visible.
    }

    coordPointType coord = Translate_Coord_To_Level_Rotation(initial->at.h, initial->at.v);

    Handle<Admiral> owner = initial->owner.value_or(Admiral::none());

    uint32_t attributes = 0;
    if (initial->flagship.value_or(false)) {
        if ((owner == g.admiral) && !owner->flagship().get()) {
            attributes |= kIsPlayerShip;
        }
    }
    if (initial->target.lock.value_or(false)) {
        attributes |= kStaticDestination;
    }

    const BaseObject* base =
            initial->owner.has_value()
                    ? get_buildable_object(initial->base, (*initial->owner)->race())
                    : BaseObject::get(initial->base.name);
    // TODO(sfiera): remap objects in networked games.
    fixedPointType v        = {Fixed::zero(), Fixed::zero()};
    auto           anObject = g.initials[initial.number()] = CreateAnySpaceObject(
            *base, &v, &coord, 0, owner, attributes,
            initial->override_.sprite.has_value()
                    ? sfz::make_optional<pn::string_view>(*initial->override_.sprite)
                    : sfz::nullopt);

    if (anObject->attributes & kIsDestination) {
        anObject->asDestination = MakeNewDestination(
                anObject, initial->build, initial->earning.value_or(Fixed::zero()),
                initial->override_.name);

        if (owner.get()) {
            if (initial->build.size() > 0) {
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

    g.initial_ids[initial.number()] = anObject->id;
    if ((anObject->attributes & kIsPlayerShip) && owner.get() && !owner->flagship().get()) {
        owner->set_flagship(anObject);
        if (owner == g.admiral) {
            g.ship = anObject;
            ResetPlayerShip();
        }
    }

    set_initial_destination(initial, true);
}

Handle<SpaceObject> GetObjectFromInitialNumber(Handle<const Initial> initial) {
    if (initial.number() >= 0) {
        auto object = g.initials[initial.number()];
        if (object.get()) {
            if ((object->id != g.initial_ids[initial.number()]) ||
                (object->active != kObjectInUse)) {
                return SpaceObject::none();
            }
            return object;
        }
        return SpaceObject::none();
    } else if (initial.number() == -2) {
        auto object = g.ship;
        if (!object->active || !(object->attributes & kCanThink)) {
            return SpaceObject::none();
        }
        return object;
    }
    return SpaceObject::none();
}

Handle<SpaceObject> resolve_object_ref(const ObjectRef& ref) {
    switch (ref.type) {
        case ObjectRef::Type::INITIAL: return GetObjectFromInitialNumber(ref.initial);
        case ObjectRef::Type::FLAGSHIP: return ref.admiral->flagship();
        case ObjectRef::Type::CONTROL: return ref.admiral->control();
        case ObjectRef::Type::TARGET: return ref.admiral->target();
    }
}

Handle<SpaceObject> resolve_object_ref(const sfz::optional<ObjectRef>& ref) {
    if (ref.has_value()) {
        return resolve_object_ref(*ref);
    } else {
        return SpaceObject::none();
    }
}

}  // namespace antares

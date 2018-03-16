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

Level::Initial* Level::Initial::get(int number) {
    if ((0 <= number) && (number < g.level->initials.size())) {
        return &g.level->initials[number];
    }
    return nullptr;
}

HandleList<Level::Initial> Level::Initial::all() {
    return HandleList<Level::Initial>(0, g.level->initials.size());
}

void create_initial(Handle<Level::Initial> initial) {
    if (initial->attributes.initially_hidden()) {
        g.initials[initial.number()] = SpaceObject::none();
        return;
    }

    coordPointType coord = Translate_Coord_To_Level_Rotation(initial->at.h, initial->at.v);

    Handle<Admiral> owner = Admiral::none();
    if (initial->owner.get()) {
        owner = initial->owner;
    }

    int32_t specialAttributes = initial->attributes.space_object_attributes();
    if (initial->attributes.is_player_ship()) {
        specialAttributes &= ~kIsPlayerShip;
        if ((owner == g.admiral) && !owner->flagship().get()) {
            specialAttributes |= kIsHumanControlled | kIsPlayerShip;
        }
    }

    auto base = initial->base;
    // TODO(sfiera): remap object in networked games.
    fixedPointType v        = {Fixed::zero(), Fixed::zero()};
    auto           anObject = g.initials[initial.number()] = CreateAnySpaceObject(
            *base, &v, &coord, g.angle, owner, specialAttributes, initial->sprite_override);

    if (anObject->attributes & kIsDestination) {
        anObject->asDestination = MakeNewDestination(
                anObject, initial->build, initial->earning, initial->name_override);
    }
    g.initial_ids[initial.number()] = anObject->id;

    if (initial->attributes.is_player_ship() && owner.get() && !owner->flagship().get()) {
        owner->set_flagship(anObject);
        if (owner == g.admiral) {
            ResetPlayerShip(anObject);
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

void set_initial_destination(Handle<Level::Initial> initial, bool preserve) {
    auto object = g.initials[initial.number()];
    if (!object.get()                      // hasn't been created yet
        || (initial->target.number() < 0)  // doesn't have a target
        || (!initial->owner.get())) {      // doesn't have an owner
        return;
    }

    // get the correct admiral #
    Handle<Admiral> owner = initial->owner;

    const auto& target = g.initials[initial->target.number()];
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

void UnhideInitialObject(Handle<Level::Initial> initial) {
    if (GetObjectFromInitialNumber(initial).get()) {
        return;  // Already visible.
    }

    coordPointType coord = Translate_Coord_To_Level_Rotation(initial->at.h, initial->at.v);

    Handle<Admiral> owner = Admiral::none();
    if (initial->owner.get()) {
        owner = initial->owner;
    }

    uint32_t specialAttributes = initial->attributes.space_object_attributes();
    if (initial->attributes.is_player_ship()) {
        if (owner.get() && !owner->flagship().get()) {
            if (owner == g.admiral) {
                specialAttributes |= kIsHumanControlled;
            } else {
                specialAttributes &= ~kIsPlayerShip;
            }
        } else {  // we already have a flagship; this should not override
            specialAttributes &= ~kIsPlayerShip;
        }
    }

    auto base = initial->base;
    // TODO(sfiera): remap objects in networked games.
    fixedPointType v        = {Fixed::zero(), Fixed::zero()};
    auto           anObject = g.initials[initial.number()] = CreateAnySpaceObject(
            *base, &v, &coord, 0, owner, specialAttributes, initial->sprite_override);

    if (anObject->attributes & kIsDestination) {
        anObject->asDestination = MakeNewDestination(
                anObject, initial->build, initial->earning, initial->name_override);

        if (owner.get()) {
            if (initial->build[0] >= 0) {
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
    if (initial->attributes.is_player_ship() && owner.get() && !owner->flagship().get()) {
        owner->set_flagship(anObject);
        if (owner == g.admiral) {
            ResetPlayerShip(anObject);
        }
    }

    set_initial_destination(initial, true);
}

Handle<SpaceObject> GetObjectFromInitialNumber(Handle<Level::Initial> initial) {
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

}  // namespace antares

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

#include "game/vector.hpp"

#include <algorithm>
#include <cmath>
#include <sfz/sfz.hpp>

#include "data/base-object.hpp"
#include "drawing/color.hpp"
#include "drawing/sprite-handling.hpp"
#include "game/globals.hpp"
#include "game/motion.hpp"
#include "game/space-object.hpp"
#include "lang/casts.hpp"
#include "math/random.hpp"
#include "math/rotation.hpp"
#include "math/units.hpp"
#include "video/driver.hpp"

using sfz::range;
using std::abs;
using std::max;

namespace antares {

namespace {

void DetermineVectorRelativeCoordFromAngle(Handle<SpaceObject> vectorObject, int16_t angle) {
    Fixed range = Fixed::from_long(vectorObject->frame.vector->range);

    mAddAngle(angle, -90);
    Fixed fcos, fsin;
    GetRotPoint(&fcos, &fsin, angle);

    // TODO(sfiera): archaeology. Did we always multiply by zero?
    vectorObject->frame.vector->toRelativeCoord =
            Point(mFixedToLong((Fixed::zero() * -fcos) - (range * -fsin)),
                  mFixedToLong((Fixed::zero() * -fsin) + (range * -fcos)));
}

template <typename T>
void clear(T& t) {
    using std::swap;
    T u;
    swap(t, u);
}

int32_t scale(int32_t value, int32_t scale) {
    return (value * scale) >> SHIFT_SCALE;
}

}  // namespace

Vector* Vector::get(int number) {
    if ((0 <= number) && (number <= size)) {
        return &g.vectors[number];
    }
    return nullptr;
}

Vector::Vector() : killMe(false), active(false) {}

void Vectors::init() {
    g.vectors.reset(new Vector[Vector::size]);
}

void Vectors::reset() {
    for (auto vector : Vector::all()) {
        clear(*vector);
    }
}

Handle<Vector> Vectors::add(
        coordPointType* location, uint8_t color, uint8_t kind, int32_t accuracy,
        int32_t vector_range) {
    for (auto vector : Vector::all()) {
        if (!vector->active) {
            vector->lastGlobalLocation   = *location;
            vector->objectLocation       = *location;
            vector->lastApparentLocation = *location;
            vector->killMe               = false;
            vector->active               = true;
            vector->color                = color;

            const int32_t h      = scale(location->h - gGlobalCorner.h, gAbsoluteScale);
            const int32_t v      = scale(location->v - gGlobalCorner.v, gAbsoluteScale);
            vector->thisLocation = Rect(0, 0, 0, 0);
            vector->thisLocation.offset(h + viewport().left, v + viewport().top);

            vector->vectorKind      = kind;
            vector->accuracy        = accuracy;
            vector->range           = vector_range;
            vector->fromObjectID    = -1;
            vector->fromObject      = SpaceObject::none();
            vector->toObjectID      = -1;
            vector->toObject        = SpaceObject::none();
            vector->toRelativeCoord = Point(0, 0);
            vector->boltState       = 0;

            return vector;
        }
    }

    return Vector::none();
}

void Vectors::set_attributes(Handle<SpaceObject> vectorObject, Handle<SpaceObject> sourceObject) {
    Vector& vector      = *vectorObject->frame.vector;
    vector.fromObjectID = sourceObject->id;
    vector.fromObject   = sourceObject;

    if (sourceObject->targetObject.get()) {
        auto target = sourceObject->targetObject;

        if ((target->active) && (target->id == sourceObject->targetObjectID)) {
            const int32_t h =
                    abs(implicit_cast<int32_t>(target->location.h - vectorObject->location.h));
            const int32_t v =
                    abs(implicit_cast<int32_t>(target->location.v - vectorObject->location.v));

            if ((((h * h) + (v * v)) > (vector.range * vector.range)) ||
                (h > kMaximumRelevantDistance) || (v > kMaximumRelevantDistance)) {
                if (vector.vectorKind == Vector::BEAM_TO_OBJECT) {
                    vector.vectorKind = Vector::BEAM_TO_COORD;
                } else if (vector.vectorKind == Vector::BEAM_TO_OBJECT_LIGHTNING) {
                    vector.vectorKind = Vector::BEAM_TO_COORD_LIGHTNING;
                }
                DetermineVectorRelativeCoordFromAngle(vectorObject, sourceObject->targetAngle);
            } else {
                if ((vector.vectorKind == Vector::BEAM_TO_COORD) ||
                    (vector.vectorKind == Vector::BEAM_TO_COORD_LIGHTNING)) {
                    vector.toRelativeCoord.h = target->location.h - sourceObject->location.h -
                                               vector.accuracy +
                                               vectorObject->randomSeed.next(vector.accuracy << 1);
                    vector.toRelativeCoord.v = target->location.v - sourceObject->location.v -
                                               vector.accuracy +
                                               vectorObject->randomSeed.next(vector.accuracy << 1);
                } else {
                    vector.toObjectID = target->id;
                    vector.toObject   = target;
                }
            }
        } else {  // target not valid
            if (vector.vectorKind == Vector::BEAM_TO_OBJECT) {
                vector.vectorKind = Vector::BEAM_TO_COORD;
            } else if (vector.vectorKind == Vector::BEAM_TO_OBJECT_LIGHTNING) {
                vector.vectorKind = Vector::BEAM_TO_COORD_LIGHTNING;
            }
            DetermineVectorRelativeCoordFromAngle(vectorObject, sourceObject->direction);
        }
    } else {  // target not valid
        if (vector.vectorKind == Vector::BEAM_TO_OBJECT) {
            vector.vectorKind = Vector::BEAM_TO_COORD;
        } else if (vector.vectorKind == Vector::BEAM_TO_OBJECT_LIGHTNING) {
            vector.vectorKind = Vector::BEAM_TO_COORD_LIGHTNING;
        }
        DetermineVectorRelativeCoordFromAngle(vectorObject, sourceObject->direction);
    }
}

void Vectors::update() {
    for (auto vector : Vector::all()) {
        if (vector->active) {
            if (vector->lastApparentLocation != vector->objectLocation) {
                vector->thisLocation = Rect(
                        scale(vector->objectLocation.h - gGlobalCorner.h, gAbsoluteScale),
                        scale(vector->objectLocation.v - gGlobalCorner.v, gAbsoluteScale),
                        scale(vector->lastApparentLocation.h - gGlobalCorner.h, gAbsoluteScale),
                        scale(vector->lastApparentLocation.v - gGlobalCorner.v, gAbsoluteScale));
                vector->thisLocation.offset(viewport().left, viewport().top);
                vector->lastApparentLocation = vector->objectLocation;
            }

            if (!vector->killMe) {
                if (vector->color) {
                    if (vector->vectorKind != Vector::BOLT) {
                        vector->boltState++;
                        if (vector->boltState > 24)
                            vector->boltState = -24;
                        uint8_t currentColor = vector->color;
                        currentColor &= 0xf0;
                        if (vector->boltState < 0)
                            currentColor += (-vector->boltState) >> 1;
                        else
                            currentColor += vector->boltState >> 1;
                        vector->color = currentColor;
                    }
                    if ((vector->vectorKind == Vector::BEAM_TO_OBJECT_LIGHTNING) ||
                        (vector->vectorKind == Vector::BEAM_TO_COORD_LIGHTNING)) {
                        vector->thisBoltPoint[0].h                 = vector->thisLocation.left;
                        vector->thisBoltPoint[0].v                 = vector->thisLocation.top;
                        vector->thisBoltPoint[kBoltPointNum - 1].h = vector->thisLocation.right;
                        vector->thisBoltPoint[kBoltPointNum - 1].v = vector->thisLocation.bottom;

                        int32_t inaccuracy = max(abs(vector->thisLocation.width()),
                                                 abs(vector->thisLocation.height())) /
                                             kBoltPointNum / 2;

                        for (int j : range(1, kBoltPointNum - 1)) {
                            vector->thisBoltPoint[j].h =
                                    vector->thisLocation.left +
                                    ((vector->thisLocation.width() * j) / kBoltPointNum) -
                                    inaccuracy + Randomize(inaccuracy * 2);
                            vector->thisBoltPoint[j].v =
                                    vector->thisLocation.top +
                                    ((vector->thisLocation.height() * j) / kBoltPointNum) -
                                    inaccuracy + Randomize(inaccuracy * 2);
                        }
                    }
                }
            }
        }
    }
}

void Vectors::draw() {
    Lines lines;
    for (auto vector : Vector::all()) {
        if (vector->active) {
            if (!vector->killMe) {
                if (vector->color) {
                    if ((vector->vectorKind == Vector::BEAM_TO_OBJECT_LIGHTNING) ||
                        (vector->vectorKind == Vector::BEAM_TO_COORD_LIGHTNING)) {
                        for (int j : range(1, kBoltPointNum)) {
                            lines.draw(
                                    vector->thisBoltPoint[j - 1], vector->thisBoltPoint[j],
                                    GetRGBTranslateColor(vector->color));
                        }
                    } else {
                        lines.draw(
                                Point(vector->thisLocation.left, vector->thisLocation.top),
                                Point(vector->thisLocation.right, vector->thisLocation.bottom),
                                GetRGBTranslateColor(vector->color));
                    }
                }
            }
        }
    }
}

void Vectors::show_all() {
    for (auto vector : Vector::all()) {
        if (vector->active) {
            if (vector->killMe) {
                vector->active = false;
            }
            if (vector->color) {
                if ((vector->vectorKind == Vector::BEAM_TO_OBJECT_LIGHTNING) ||
                    (vector->vectorKind == Vector::BEAM_TO_COORD_LIGHTNING)) {
                    for (int j : range(kBoltPointNum)) {
                        vector->lastBoltPoint[j] = vector->thisBoltPoint[j];
                    }
                }
            }
        }
    }
}

void Vectors::cull() {
    for (auto vector : Vector::all()) {
        if (vector->active) {
            if (vector->killMe) {
                vector->active = false;
            }
        }
    }
}

}  // namespace antares

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

}  // namespace

Vector* Vector::get(int number) {
    if ((0 <= number) && (number <= size)) {
        return &g.vectors[number];
    }
    return nullptr;
}

Vector::Vector() : killMe(false), active(false) {}

void Vectors::init() { g.vectors.reset(new Vector[Vector::size]); }

void Vectors::reset() {
    for (auto vector : Vector::all()) {
        clear(*vector);
    }
}

Handle<Vector> Vectors::add(Point* location, const BaseObject::Ray& r) {
    for (auto vector : Vector::all()) {
        if (!vector->active) {
            vector->lastGlobalLocation   = *location;
            vector->objectLocation       = *location;
            vector->lastApparentLocation = *location;
            vector->killMe               = false;
            vector->active               = true;
            vector->visible              = r.hue.has_value();
            vector->color                = RgbColor::clear();
            vector->hue                  = r.hue;

            vector->thisBoltPoint[0] = vector->thisBoltPoint[kBoltPointNum - 1] =
                    Point{scale_by(location->h - scaled_screen.bounds.left, scaled_screen.scale) +
                                  viewport().left,
                          scale_by(location->v - scaled_screen.bounds.top, scaled_screen.scale) +
                                  viewport().top};

            vector->is_ray          = true;
            vector->to_coord        = (r.to == BaseObject::Ray::To::COORD);
            vector->lightning       = r.lightning;
            vector->accuracy        = r.accuracy;
            vector->range           = r.range;
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

Handle<Vector> Vectors::add(Point* location, const BaseObject::Bolt& b) {
    for (auto vector : Vector::all()) {
        if (!vector->active) {
            vector->lastGlobalLocation   = *location;
            vector->objectLocation       = *location;
            vector->lastApparentLocation = *location;
            vector->killMe               = false;
            vector->active               = true;
            vector->visible              = (b.color != RgbColor::clear());
            vector->hue                  = sfz::nullopt;
            vector->color                = b.color;

            vector->thisBoltPoint[0] = vector->thisBoltPoint[kBoltPointNum - 1] =
                    Point{scale_by(location->h - scaled_screen.bounds.left, scaled_screen.scale) +
                                  viewport().left,
                          scale_by(location->v - scaled_screen.bounds.top, scaled_screen.scale) +
                                  viewport().top};

            vector->is_ray          = false;
            vector->to_coord        = false;
            vector->lightning       = false;
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
            const int32_t h = abs(target->location.h - vectorObject->location.h);
            const int32_t v = abs(target->location.v - vectorObject->location.v);

            if ((((h * h) + (v * v)) > (vector.range * vector.range)) ||
                (h > kMaximumRelevantDistance) || (v > kMaximumRelevantDistance)) {
                if (vector.is_ray) {
                    vector.to_coord = true;
                }
                DetermineVectorRelativeCoordFromAngle(vectorObject, sourceObject->targetAngle);
            } else {
                if (vector.to_coord) {
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
            if (vector.is_ray) {
                vector.to_coord = true;
            }
            DetermineVectorRelativeCoordFromAngle(vectorObject, sourceObject->direction);
        }
    } else {  // target not valid
        if (vector.is_ray) {
            vector.to_coord = true;
        }
        DetermineVectorRelativeCoordFromAngle(vectorObject, sourceObject->direction);
    }
}

void Vectors::update() {
    for (auto vector : Vector::all()) {
        if (vector->active) {
            if (vector->lastApparentLocation != vector->objectLocation) {
                vector->thisBoltPoint[0] =
                        Point(viewport().left +
                                      scale_by(
                                              vector->objectLocation.h - scaled_screen.bounds.left,
                                              scaled_screen.scale),
                              viewport().top +
                                      scale_by(
                                              vector->objectLocation.v - scaled_screen.bounds.top,
                                              scaled_screen.scale));
                vector->thisBoltPoint[kBoltPointNum - 1] = Point(
                        viewport().left +
                                scale_by(
                                        vector->lastApparentLocation.h - scaled_screen.bounds.left,
                                        scaled_screen.scale),
                        viewport().top +
                                scale_by(
                                        vector->lastApparentLocation.v - scaled_screen.bounds.top,
                                        scaled_screen.scale));
                vector->lastApparentLocation = vector->objectLocation;
            }

            if (!vector->killMe) {
                if (vector->visible) {
                    if (vector->hue.has_value()) {
                        vector->boltState++;
                        if (vector->boltState > 24)
                            vector->boltState = -24;
                        uint8_t currentColor = static_cast<int>(*vector->hue) << 4;
                        if (vector->boltState < 0)
                            currentColor += (-vector->boltState) >> 1;
                        else
                            currentColor += vector->boltState >> 1;
                        vector->color = GetRGBTranslateColor(currentColor);
                    }
                    if (vector->lightning) {
                        auto&   p     = vector->thisBoltPoint;
                        Point   begin = p[0];
                        Point   end   = p[kBoltPointNum - 1];
                        Size    span{end.h - begin.h, end.v - begin.v};
                        int32_t inaccuracy =
                                max(abs(span.width), abs(span.height)) / kBoltPointNum / 2;

                        for (int j : range(1, kBoltPointNum - 1)) {
                            p[j].h = begin.h + ((span.width * j) / kBoltPointNum) - inaccuracy +
                                     Randomize(inaccuracy * 2);
                            p[j].v = begin.v + ((span.height * j) / kBoltPointNum) - inaccuracy +
                                     Randomize(inaccuracy * 2);
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
        if (vector->active && !vector->killMe) {
            if (vector->visible) {
                const auto& p = vector->thisBoltPoint;
                if (vector->lightning) {
                    for (int j : range(0, kBoltPointNum - 1)) {
                        lines.draw(p[j], p[j + 1], vector->color);
                    }
                } else {
                    lines.draw(p[0], p[kBoltPointNum - 1], vector->color);
                }
            }
        }
    }
}

void Vectors::cull() {
    for (auto vector : Vector::all()) {
        vector->active = vector->active && !vector->killMe;
    }
}

}  // namespace antares

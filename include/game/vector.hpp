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

#ifndef ANTARES_GAME_VECTOR_HPP_
#define ANTARES_GAME_VECTOR_HPP_

#include <stdint.h>

#include "data/base-object.hpp"
#include "data/handle.hpp"
#include "math/geometry.hpp"

namespace antares {

class SpaceObject;

static const int kBoltPointNum = 10;

struct Vector {
    static Vector*            get(int number);
    static Handle<Vector>     none() { return Handle<Vector>(-1); }
    static HandleList<Vector> all() { return HandleList<Vector>(0, size); }

    Vector();

    bool                is_ray    = false;
    bool                to_coord  = false;
    bool                lightning = false;
    Rect                thisLocation;
    coordPointType      lastGlobalLocation;
    coordPointType      objectLocation;
    coordPointType      lastApparentLocation;
    bool                visible;
    RgbColor            color;
    sfz::optional<Hue>  hue;
    bool                killMe;
    bool                active;
    int32_t             fromObjectID;
    Handle<SpaceObject> fromObject;
    int32_t             toObjectID;
    Handle<SpaceObject> toObject;
    Point               toRelativeCoord;
    int32_t             boltState;
    int32_t             accuracy;
    int32_t             range;
    Point               thisBoltPoint[kBoltPointNum];
    Point               lastBoltPoint[kBoltPointNum];

  private:
    friend class Vectors;
    const static size_t size = 256;
};

class Vectors {
  public:
    static void           init();
    static void           reset();
    static Handle<Vector> add(coordPointType* location, const BaseObject::Ray& r);
    static Handle<Vector> add(coordPointType* location, const BaseObject::Bolt& b);
    static void set_attributes(Handle<SpaceObject> vectorObject, Handle<SpaceObject> sourceObject);
    static void update();
    static void draw();
    static void show_all();
    static void cull();
};

}  // namespace antares

#endif  // ANTARES_GAME_VECTOR_HPP_

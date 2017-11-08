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

#ifndef ANTARES_DRAWING_BRIEFING_HPP_
#define ANTARES_DRAWING_BRIEFING_HPP_

#include "data/handle.hpp"
#include "math/geometry.hpp"

namespace antares {

struct Level;
class PixMap;

struct BriefingSpriteBounds {
    Rect                bounds;
    Handle<SpaceObject> object;
};

void draw_briefing_objects(
        Point origin, int32_t maxSize, Rect bounds, coordPointType corner, int32_t scale);

void BriefPoint_Data_Get(
        int32_t whichPoint, const Level* level, int32_t* headerID, int32_t* headerNumber,
        int32_t* contentID, Rect* hiliteBounds, coordPointType* corner, int32_t scale,
        int32_t minSectorSize, int32_t maxSize, Rect* bounds);

}  // namespace antares

#endif  // ANTARES_DRAWING_BRIEFING_HPP_

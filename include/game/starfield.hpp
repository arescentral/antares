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

#ifndef ANTARES_GAME_STARFIELD_HPP_
#define ANTARES_GAME_STARFIELD_HPP_

#include "data/handle.hpp"
#include "math/fixed.hpp"
#include "math/geometry.hpp"
#include "math/units.hpp"

namespace antares {

class SpaceObject;

const int32_t kMaxSparkAge          = 1023;
const int32_t kSparkAgeToShadeShift = 6;

enum StarSpeed {
    kNoStar          = -1,
    kSlowStarSpeed   = 1,
    kMediumStarSpeed = 2,
    kFastStarSpeed   = 3,
};

struct scrollStarType {
    Point          oldLocation;
    Point          location;
    fixedPointType motionFraction;
    fixedPointType velocity;
    int32_t        age;
    int32_t        speed;
    uint8_t        color;
};

class Starfield {
  public:
    static const int32_t kScrollStarNum   = 125;
    static const int32_t kSparkStarNum    = 125;
    static const int32_t kAllStarNum      = kScrollStarNum + kSparkStarNum;
    static const int32_t kSparkStarOffset = kScrollStarNum;

    Starfield();
    void reset(Handle<SpaceObject> which_object);
    void make_sparks(
            int32_t sparkNum, int32_t sparkSpeed, Fixed maxVelocity, uint8_t color,
            Point* location);
    void prepare_to_move();
    void move(ticks by_units);
    void draw() const;
    void show();

  private:
    scrollStarType _stars[kScrollStarNum + kSparkStarNum];
    int32_t        _last_clip_bottom;
    bool           _warp_stars;

    DISALLOW_COPY_AND_ASSIGN(Starfield);
};

}  // namespace antares

#endif  // ANTARES_GAME_STARFIELD_HPP_

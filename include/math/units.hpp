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

#ifndef ANTARES_MATH_UNITS_HPP_
#define ANTARES_MATH_UNITS_HPP_

#include <stdint.h>
#include <chrono>

namespace antares {

// The three units of time in Antares are the microsecond, the second,
// and the tick.
//
// There are 60 ticks in a second. Also, every third tick is a major
// tick, so there are 20 major ticks in a second. Major ticks are when
// most things happen: ships collide, decisions are made, and actions
// are executed. During the two minor ticks between major ticks, ships
// and stars move, but don't collide. Minor ticks may be skipped if
// drawing is slow, but will make the game look smoother if they aren't.
//
// Antares actually uses a unit for seconds that is slightly longer than
// a second. There are exactly 16667 microseconds in a tick, and 60
// ticks in a second, which gives us 1000020 microseconds in a second.
// This makes it possible to implicitly convert seconds to ticks to
// microseconds.
typedef std::chrono::microseconds                                       usecs;
typedef std::chrono::duration<usecs::rep, std::ratio<1000020, 1000000>> secs;
typedef std::chrono::duration<usecs::rep, std::ratio<16667, 1000000>>   ticks;

const ticks kMajorTick     = ticks(3);
const ticks kMinorTick     = ticks(1);
const ticks kConditionTick = ticks(90);

// Time units
struct GameStart {
    typedef ticks duration;
};
struct Wall {
    typedef usecs duration;
};

typedef std::chrono::time_point<GameStart>   game_ticks;
typedef std::chrono::time_point<Wall>        wall_time;
typedef std::chrono::time_point<Wall, ticks> wall_ticks;

// Spatial units

const int32_t kUniversalCenter         = 0x3fffffff;
const int32_t kMaximumRelevantDistance = 46340;  // floor(sqrt(0x7fffffff)
const int32_t kMaximumRelevantDistanceSquared =
        kMaximumRelevantDistance * kMaximumRelevantDistance;  // Slightly < 0x7fffffff
const int32_t kMaximumAngleDistance = 32767;  // maximum distance we can calc angle for

enum {
    SUBSECTOR     = 128,     // 2^7 (near object grid)
    SECTOR_SMALL  = 512,     // 2^9 (blue lines)
    SECTOR_MEDIUM = 2048,    // 2^11 (sky blue lines, near object super-grid, far object grid)
    SECTOR_LARGE  = 8192,    // 2^13 (green lines)
    SECTOR_HUGE   = 32768,   // 2^15 (far object super-grid)
    SECTOR_MAX    = 262144,  // 2^18 (universe width)
};

struct Scale {
    int32_t factor;
};

const int32_t SHIFT_SCALE = 12;
const Scale   SCALE_SCALE{1 << SHIFT_SCALE};
const Scale   MIN_SCALE{256};

const Scale kOneEighthScale{SCALE_SCALE.factor / 8};
const Scale kOneQuarterScale{SCALE_SCALE.factor / 4};
const Scale kOneHalfScale{SCALE_SCALE.factor / 2};
const Scale kTimesTwoScale{SCALE_SCALE.factor * 2};

}  // namespace antares

#endif  // ANTARES_MATH_UNITS_HPP_

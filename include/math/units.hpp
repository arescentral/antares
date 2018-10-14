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

const int32_t kUniversalCenter         = 1073741823;
const int32_t kMaximumRelevantDistance = 46340;
const int32_t kMaximumRelevantDistanceSquared =
        kMaximumRelevantDistance * kMaximumRelevantDistance;
const int32_t kMaximumAngleDistance = 32767;  // maximum distance we can calc angle for

const int32_t kSubSectorSize  = 512;
const int32_t kSubSectorShift = 9;

const int32_t SCALE_SCALE   = 4096;
const int32_t MIN_SCALE     = 256;
const int32_t MAX_SCALE     = 32768;
const int32_t MAX_SCALE_PIX = 32;  // the maximum size a single scaled pixel can be
                                   // (should be 32)

const int32_t kOneEighthScale  = SCALE_SCALE / 8;
const int32_t kOneQuarterScale = SCALE_SCALE / 4;
const int32_t kOneHalfScale    = SCALE_SCALE / 2;
const int32_t kTimesTwoScale   = SCALE_SCALE * 2;

const int32_t SHIFT_SCALE = 12;

}  // namespace antares

#endif  // ANTARES_MATH_UNITS_HPP_

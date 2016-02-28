// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// Copyright (C) 2008-2012 The Antares Authors
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

#include <chrono>
#include <stdint.h>

namespace antares {

// Time units
struct GameStart {};
struct Wall {};
typedef std::chrono::duration<std::chrono::microseconds::rep, std::ratio<16667, 1000000>> ticks;
typedef std::chrono::time_point<GameStart, std::chrono::microseconds> game_time;
typedef std::chrono::time_point<GameStart, ticks> game_ticks;
typedef std::chrono::time_point<Wall, std::chrono::microseconds> wall_time;
typedef std::chrono::time_point<Wall, ticks> wall_ticks;

// in microseconds--essentially one tick (1/60th of second)
const int64_t kTimeUnit = 16667;

inline int64_t ticks_to_usecs(int64_t ticks) { return ticks * kTimeUnit; }
inline int64_t usecs_to_ticks(int64_t usecs) { return usecs / kTimeUnit; }
inline int64_t usecs_to_ticks(game_time t) { return t.time_since_epoch().count() / kTimeUnit; }

// every time this many cycles pass, we have to process player & computer decisions
const ticks kDecideEveryCycles = ticks(3);

// Spatial units

const int32_t kUniversalCenter = 1073741823;
const int32_t kMaximumRelevantDistance = 46340;
const int32_t kMaximumRelevantDistanceSquared = kMaximumRelevantDistance * kMaximumRelevantDistance;
const int32_t kMaximumAngleDistance = 32767;      // maximum distance we can calc angle for

const int32_t kSubSectorSize = 512;
const int32_t kSubSectorShift = 9;

}  // namespace antares

#endif // ANTARES_MATH_UNITS_HPP_

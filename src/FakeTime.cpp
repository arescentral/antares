// Ares, a tactical space combat game.
// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// 
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

#include "FakeTime.hpp"

#include <stdint.h>
#include <sys/time.h>
#include <Base.h>

int64_t TickTime() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return 60.0 * (tv.tv_sec + tv.tv_usec / 1000000.0);
}

int TickCount() {
    static const int kStartTime = TickTime();
    return TickTime() - kStartTime;
}

void Microseconds(UnsignedWide* wide) {
    uint64_t time;
    struct timeval tv;
    gettimeofday(&tv, NULL);
    time = tv.tv_sec * 1000000ull + tv.tv_usec * 1ull;
    wide->hi = time >> 32;
    wide->lo = time;
}

void FakeTimeInit() {
}

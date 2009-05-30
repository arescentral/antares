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

#include "FakeMath.hpp"

#include <stdint.h>
#include <limits>

#include <Base.h>

inline int64_t WideToInt64(wide w) {
    return *reinterpret_cast<int64_t*>(&w);
}

inline wide Int64ToWide(int64_t i) {
    return *reinterpret_cast<wide*>(&i);
}

void WideAdd(wide* value, wide* summand) {
    int64_t value64 = WideToInt64(*value);
    int64_t summand64 = WideToInt64(*summand);
    *value = Int64ToWide(value64 + summand64);
}

void WideSubtract(wide* value, wide* difference) {
    int64_t value64 = WideToInt64(*value);
    int64_t difference64 = WideToInt64(*difference);
    *value = Int64ToWide(value64 - difference64);
}

void WideMultiply(long a, long b, wide* c) {
    int64_t a64 = a;
    int64_t b64 = b;
    *c = Int64ToWide(a64 * b64);
}

long Random() {
    static bool seeded = false;
    if (!seeded) {
        srand(0x84744901);
        seeded = true;
    }
    return rand() & 0x7FFF;
}

struct AngleFromSlopeData {
    Fixed min_slope;
    int angle;
};

static const int angle_from_slope_data_count = 182;
static AngleFromSlopeData angle_from_slope_data[angle_from_slope_data_count] = {
    {std::numeric_limits<int>::min(), 90},
    {-3755044, 89},
    {-1877195, 88},
    {-1250993, 87},
    {-937705, 86},
    {-749579, 85},
    {-624033, 84},
    {-534247, 83},
    {-466812, 82},
    {-414277, 81},
    {-372173, 80},
    {-337653, 79},
    {-308822, 78},
    {-284367, 77},
    {-263350, 76},
    {-245084, 75},
    {-229051, 74},
    {-214858, 73},
    {-202199, 72},
    {-190830, 71},
    {-180559, 70},
    {-171227, 69},
    {-162707, 68},
    {-154893, 67},
    {-147696, 66},
    {-141042, 65},
    {-134869, 64},
    {-129122, 63},
    {-123755, 62},
    {-118730, 61},
    {-114012, 60},
    {-109570, 59},
    {-105379, 58},
    {-101417, 57},
    {-97661, 56},
    {-94095, 55},
    {-90703, 54},
    {-87469, 53},
    {-84382, 52},
    {-81430, 51},
    {-78603, 50},
    {-75891, 49},
    {-73285, 48},
    {-70779, 47},
    {-68365, 46},
    {-66036, 45},
    {-63787, 44},
    {-61613, 43},
    {-59509, 42},
    {-57470, 41},
    {-55491, 40},
    {-53570, 39},
    {-51702, 38},
    {-49885, 37},
    {-48115, 36},
    {-46389, 35},
    {-44705, 34},
    {-43060, 33},
    {-41451, 32},
    {-39878, 31},
    {-38337, 30},
    {-36827, 29},
    {-35346, 28},
    {-33892, 27},
    {-32464, 26},
    {-31060, 25},
    {-29679, 24},
    {-28318, 23},
    {-26978, 22},
    {-25657, 21},
    {-24353, 20},
    {-23066, 19},
    {-21794, 18},
    {-20536, 17},
    {-19292, 16},
    {-18060, 15},
    {-16840, 14},
    {-15630, 13},
    {-14430, 12},
    {-13239, 11},
    {-12056, 10},
    {-10880, 9},
    {-9710, 8},
    {-8547, 7},
    {-7388, 6},
    {-6234, 5},
    {-5083, 4},
    {-3935, 3},
    {-2789, 2},
    {-1644, 1},
    {-500, 0},
    {0, 180},
    {501, 179},
    {1645, 178},
    {2790, 177},
    {3936, 176},
    {5084, 175},
    {6235, 174},
    {7389, 173},
    {8548, 172},
    {9711, 171},
    {10881, 170},
    {12057, 169},
    {13240, 168},
    {14431, 167},
    {15631, 166},
    {16841, 165},
    {18061, 164},
    {19293, 163},
    {20537, 162},
    {21795, 161},
    {23067, 160},
    {24354, 159},
    {25658, 158},
    {26979, 157},
    {28319, 156},
    {29680, 155},
    {31061, 154},
    {32465, 153},
    {33893, 152},
    {35347, 151},
    {36828, 150},
    {38338, 149},
    {39879, 148},
    {41452, 147},
    {43061, 146},
    {44706, 145},
    {46390, 144},
    {48116, 143},
    {49886, 142},
    {51703, 141},
    {53571, 140},
    {55492, 139},
    {57471, 138},
    {59510, 137},
    {61614, 136},
    {63788, 135},
    {66037, 134},
    {68366, 133},
    {70780, 132},
    {73286, 131},
    {75892, 130},
    {78604, 129},
    {81431, 128},
    {84383, 127},
    {87470, 126},
    {90704, 125},
    {94096, 124},
    {97662, 123},
    {101418, 122},
    {105380, 121},
    {109571, 120},
    {114013, 119},
    {118731, 118},
    {123756, 117},
    {129123, 116},
    {134870, 115},
    {141043, 114},
    {147697, 113},
    {154894, 112},
    {162708, 111},
    {171228, 110},
    {180560, 109},
    {190831, 108},
    {202200, 107},
    {214859, 106},
    {229052, 105},
    {245085, 104},
    {263351, 103},
    {284368, 102},
    {308823, 101},
    {337654, 100},
    {372174, 99},
    {414278, 98},
    {466813, 97},
    {534248, 96},
    {624034, 95},
    {749580, 94},
    {937706, 93},
    {1250994, 92},
    {1877196, 91},
    {3755045, 90},
};

long AngleFromSlope(Fixed slope) {
    for (int i = 1; i < angle_from_slope_data_count; ++i) {
        if (angle_from_slope_data[i].min_slope > slope) {
            return angle_from_slope_data[i - 1].angle;
        }
    }
    return 90;
}

void FakeMathInit() {
}

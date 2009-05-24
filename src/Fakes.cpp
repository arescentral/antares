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

#include <Quickdraw.h>
#include <Resources.h>
#include <Sound.h>

#include <assert.h>
#include <fcntl.h>
#include <glob.h>
#include <math.h>
#include <png++/png.hpp>
#include <string.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <limits>
#include <algorithm>

#include "AresGlobalType.hpp"

static void* const kMmapFailed = (void*)-1;

struct Color24Bit {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
};

Color24Bit colors_24_bit[256] = {
    {255, 255, 255},
    {32, 0, 0},
    {224, 224, 224},
    {208, 208, 208},
    {192, 192, 192},
    {176, 176, 176},
    {160, 160, 160},
    {144, 144, 144},
    {128, 128, 128},
    {112, 112, 112},
    {96, 96, 96},
    {80, 80, 80},
    {64, 64, 64},
    {48, 48, 48},
    {32, 32, 32},
    {16, 16, 16},
    {8, 8, 8},
    {255, 127, 0},
    {240, 120, 0},
    {224, 112, 0},
    {208, 104, 0},
    {192, 96, 0},
    {176, 88, 0},
    {160, 80, 0},
    {144, 72, 0},
    {128, 64, 0},
    {112, 56, 0},
    {96, 48, 0},
    {80, 40, 0},
    {64, 32, 0},
    {48, 24, 0},
    {32, 16, 0},
    {16, 8, 0},
    {255, 255, 0},
    {240, 240, 0},
    {224, 224, 0},
    {208, 208, 0},
    {192, 192, 0},
    {176, 176, 0},
    {160, 160, 0},
    {144, 144, 0},
    {128, 128, 0},
    {112, 112, 0},
    {96, 96, 0},
    {80, 80, 0},
    {64, 64, 0},
    {48, 48, 0},
    {32, 32, 0},
    {16, 16, 0},
    {0, 0, 255},
    {0, 0, 240},
    {0, 0, 224},
    {0, 0, 208},
    {0, 0, 192},
    {0, 0, 176},
    {0, 0, 160},
    {0, 0, 144},
    {0, 0, 128},
    {0, 0, 112},
    {0, 0, 96},
    {0, 0, 80},
    {0, 0, 64},
    {0, 0, 48},
    {0, 0, 32},
    {0, 0, 16},
    {0, 255, 0},
    {0, 240, 0},
    {0, 224, 0},
    {0, 208, 0},
    {0, 192, 0},
    {0, 176, 0},
    {0, 160, 0},
    {0, 144, 0},
    {0, 128, 0},
    {0, 112, 0},
    {0, 96, 0},
    {0, 80, 0},
    {0, 64, 0},
    {0, 48, 0},
    {0, 32, 0},
    {0, 16, 0},
    {127, 0, 255},
    {120, 0, 240},
    {112, 0, 224},
    {104, 0, 208},
    {96, 0, 192},
    {88, 0, 176},
    {80, 0, 160},
    {72, 0, 144},
    {64, 0, 128},
    {56, 0, 112},
    {48, 0, 96},
    {40, 0, 80},
    {32, 0, 64},
    {24, 0, 48},
    {16, 0, 32},
    {8, 0, 16},
    {127, 127, 255},
    {120, 120, 240},
    {112, 112, 224},
    {104, 104, 208},
    {96, 96, 192},
    {88, 88, 176},
    {80, 80, 160},
    {72, 72, 144},
    {64, 64, 128},
    {56, 56, 112},
    {48, 48, 96},
    {40, 40, 80},
    {32, 32, 64},
    {24, 24, 48},
    {16, 16, 32},
    {8, 8, 16},
    {255, 127, 127},
    {240, 120, 120},
    {224, 112, 112},
    {208, 104, 104},
    {192, 96, 96},
    {176, 88, 88},
    {160, 80, 80},
    {144, 72, 72},
    {128, 64, 64},
    {112, 56, 56},
    {96, 48, 48},
    {80, 40, 40},
    {64, 32, 32},
    {48, 24, 24},
    {32, 16, 16},
    {16, 8, 8},
    {255, 255, 127},
    {240, 240, 120},
    {224, 224, 112},
    {208, 208, 104},
    {192, 192, 96},
    {176, 176, 88},
    {160, 160, 80},
    {144, 144, 72},
    {128, 128, 64},
    {112, 112, 56},
    {96, 96, 48},
    {80, 80, 40},
    {64, 64, 32},
    {48, 48, 24},
    {32, 32, 16},
    {16, 16, 8},
    {0, 255, 255},
    {0, 240, 240},
    {0, 224, 224},
    {0, 208, 208},
    {0, 192, 192},
    {0, 176, 176},
    {0, 160, 160},
    {0, 144, 144},
    {0, 128, 128},
    {0, 112, 112},
    {0, 96, 96},
    {0, 80, 80},
    {0, 64, 64},
    {0, 48, 48},
    {0, 32, 32},
    {0, 16, 16},
    {255, 0, 127},
    {240, 0, 120},
    {224, 0, 112},
    {208, 0, 104},
    {192, 0, 96},
    {176, 0, 88},
    {160, 0, 80},
    {144, 0, 72},
    {128, 0, 64},
    {112, 0, 56},
    {96, 0, 48},
    {80, 0, 40},
    {64, 0, 32},
    {48, 0, 24},
    {32, 0, 16},
    {16, 0, 8},
    {127, 255, 127},
    {120, 240, 120},
    {112, 224, 112},
    {104, 208, 104},
    {96, 192, 96},
    {88, 176, 88},
    {80, 160, 80},
    {72, 144, 72},
    {64, 128, 64},
    {56, 112, 56},
    {48, 96, 48},
    {40, 80, 40},
    {32, 64, 32},
    {24, 48, 24},
    {16, 32, 16},
    {8, 16, 8},
    {255, 127, 255},
    {240, 120, 240},
    {224, 112, 224},
    {208, 104, 208},
    {192, 96, 192},
    {176, 88, 176},
    {160, 80, 160},
    {144, 72, 143},
    {128, 64, 128},
    {112, 56, 112},
    {96, 48, 96},
    {80, 40, 80},
    {64, 32, 64},
    {48, 24, 48},
    {32, 16, 32},
    {16, 8, 16},
    {0, 127, 255},
    {0, 120, 240},
    {0, 112, 224},
    {0, 104, 208},
    {0, 96, 192},
    {0, 88, 176},
    {0, 80, 160},
    {0, 72, 143},
    {0, 64, 128},
    {0, 56, 112},
    {0, 48, 96},
    {0, 40, 80},
    {0, 32, 64},
    {0, 24, 48},
    {0, 16, 32},
    {0, 8, 16},
    {255, 249, 207},
    {240, 234, 195},
    {225, 220, 183},
    {210, 205, 171},
    {195, 190, 159},
    {180, 176, 146},
    {165, 161, 134},
    {150, 146, 122},
    {135, 132, 110},
    {120, 117, 97},
    {105, 102, 85},
    {90, 88, 73},
    {75, 73, 61},
    {60, 58, 48},
    {45, 44, 36},
    {30, 29, 24},
    {255, 0, 0},
    {240, 0, 0},
    {225, 0, 0},
    {208, 0, 0},
    {192, 0, 0},
    {176, 0, 0},
    {160, 0, 0},
    {144, 0, 0},
    {128, 0, 0},
    {112, 0, 0},
    {96, 0, 0},
    {80, 0, 0},
    {64, 0, 0},
    {48, 0, 0},
    {0, 0, 0},
};

CTab* fakeCTabPtr = NULL;

AuxWin fakeAuxWin = {
    &fakeCTabPtr,
};
AuxWin* fakeAuxWinPtr = &fakeAuxWin;

static uint8_t NearestColor(uint16_t red, uint16_t green, uint16_t blue);
static uint8_t GetPixel(int x, int y);
static void SetPixel(int x, int y, uint8_t c);
static void SetPixelRow(int x, int y, uint8_t* c, int count);

extern aresGlobalType* gAresGlobal;

void Dump() {
    std::string contents = "P6 640 480 255\n";
    contents.reserve(contents.size() + 640 * 480 * 3);
    for (int y = 0; y < 480; ++y) {
        for (int x = 0; x < 640; ++x) {
            int color = GetPixel(x, y);
            const char pixel[3] = {
                fakeCTabPtr->ctTable[color].rgb.red >> 8,
                fakeCTabPtr->ctTable[color].rgb.green >> 8,
                fakeCTabPtr->ctTable[color].rgb.blue >> 8,
            };
            contents.insert(contents.size(), pixel, 3);
        }
    }

    char filename[64];
    sprintf(filename, "dump-%05ld.pnm", gAresGlobal->gGameTime);
    int fd = open(filename, O_WRONLY | O_CREAT, 0644);
    write(fd, contents.c_str(), contents.size());
    close(fd);
}

class RealHandle {
  public:
    void* data;
    size_t size;

    RealHandle() : data(NULL), size(0) { }
};

class AllocatedHandle : public RealHandle {
  public:
    AllocatedHandle(size_t size_) {
        size = size_;
        data = new char[size];
    }

    ~AllocatedHandle() {
        delete[] static_cast<char*>(data);
    }
};

class ResourceHandle : public RealHandle {
  public:
    int fd;

    ResourceHandle() : fd(-1) { }

    ~ResourceHandle() {
        if (data != NULL && data != kMmapFailed) {
            munmap(data, size);
        }
        if (fd >= 0) {
            close(fd);
        }
    }
};

template <typename T>
class scoped_ptr {
  public:
    scoped_ptr() : _t(NULL) { }
    explicit scoped_ptr(T* t) : _t(t) { }

    ~scoped_ptr() { if (_t) delete _t; }

    T* operator->() { return _t; }
    T& operator*() { return _t; }

    void reset() { if (_t) { delete _t; _t = NULL; } }
    T* release() { T* t = _t; _t = NULL; return t; }

  private:
    T* _t;
};

Handle GetResource(FourCharCode code, int id) {
    char filename[64];
    sprintf(filename, "data/original/rsrc/%4s/r.%d", reinterpret_cast<char*>(&code), id);
    scoped_ptr<ResourceHandle> result(new ResourceHandle);

    result->fd = open(filename, O_RDONLY);
    if (result->fd < 0) {
        perror("open");
        return NULL;
    }

    struct stat st;
    if (fstat(result->fd, &st) < 0) {
        perror("fstat");
        return NULL;
    }
    result->size = st.st_size;

    result->data = mmap(NULL, result->size, PROT_READ | PROT_WRITE, MAP_PRIVATE, result->fd, 0);
    if (result->data == kMmapFailed) {
        perror("mmap");
        return NULL;
    }

    return reinterpret_cast<Handle>(result.release());
}

Handle NewHandle(size_t size) {
    return reinterpret_cast<Handle>(new AllocatedHandle(size));
}

int GetHandleSize(Handle handle) {
    RealHandle* real = reinterpret_cast<RealHandle*>(handle);
    return real->size;
}

void GetIndString(unsigned char* result, int id, int index) {
    if (index <= 0) {
        *result = '\0';
        return;
    }
    Handle resource = GetResource('STR#', id);
    assert(resource);
    uint16_t count = *reinterpret_cast<uint16_t*>(*resource);
    assert(index <= count);
    char* pstr = *resource + 2;
    uint8_t size = *pstr;
    while (index > 1) {
        pstr += size + 1;
        size = *pstr;
        --index;
    }
    memcpy(result, pstr, size + 1);
}

void BlockMove(void* src, void* dst, size_t size) {
    memcpy(dst, src, size);
}

OSErr PtrToHand(void* ptr, Handle* handle, int len) {
    *handle = new char*;
    **handle = new char[len];
    BlockMove(ptr, **handle, len);
    return noErr;
}

int64_t TickTime() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return 60.0 * (tv.tv_sec + tv.tv_usec / 1000000.0);
}

int TickCount() {
    static const int kStartTime = TickTime();
    return TickTime() - kStartTime;
}

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

void Microseconds(UnsignedWide* wide) {
    uint64_t time;
    struct timeval tv;
    gettimeofday(&tv, NULL);
    time = tv.tv_sec * 1000000ull + tv.tv_usec * 1ull;
    wide->hi = time >> 32;
    wide->lo = time;
}

void ModalDialog(void*, short* item) {
    *item = 1;
}

bool WaitNextEvent(long mask, EventRecord* evt, unsigned long sleep, Rgn** mouseRgn) {
    evt->what = 0;
    return true;
}

long Random() {
    static bool seeded = false;
    if (!seeded) {
        srand(0x84744901);
        seeded = true;
    }
    return rand() & 0x7FFF;
}

// Perform this many clicks in succession at the start of the game.
const int kClickCount = 2;

bool Button() {
    static int current_click = 0;
    static bool clicked = false;
    if (current_click < kClickCount) {
        if (clicked) {
            ++current_click;
        }
        clicked = !clicked;
    }
    return clicked;
}

void GetKeys(KeyMap keys) {
    bzero(keys, sizeof(KeyMap));
}

void SetRect(Rect* rect, int left, int top, int right, int bottom) {
    rect->left = left;
    rect->top = top;
    rect->right = right;
    rect->bottom = bottom;
}

void MacSetRect(Rect* rect, int left, int top, int right, int bottom) {
    SetRect(rect, left, top, right, bottom);
}

void OffsetRect(Rect* rect, int x, int y) {
    rect->left += x;
    rect->right += x;
    rect->top += y;
    rect->bottom += y;
}

void MacOffsetRect(Rect* rect, int x, int y) {
    OffsetRect(rect, x, y);
}

bool MacPtInRect(Point p, Rect* rect) {
    return (rect->left <= p.h && p.h <= rect->right)
        && (rect->top <= p.v && p.v <= rect->bottom);
}

void MacInsetRect(Rect* rect, int x, int y) {
    rect->left += x;
    rect->right -= x;
    rect->top += y;
    rect->bottom -= y;
}

void GetAuxWin(Window*, AuxWinHandle* handle) {
    *handle = &fakeAuxWinPtr;
}

struct AngleFromSlopeData {
    Fixed min_slope;
    int angle;
};

static const int angle_from_slope_data_count = 182;
static AngleFromSlopeData angle_from_slope_data[angle_from_slope_data_count] = {
    {INT_MIN, 90},
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

extern GWorld* gOffWorld;
extern GWorld* gRealWorld;
extern GWorld* gSaveWorld;

struct GWorld {
    char pixels[640 * 480];
    PixMap pixMap;
    PixMap* pixMapPtr;
};

GWorld fakeOffGWorld = {
    { },
    {
        { 0, 0, 640, 480 },
        &fakeCTabPtr,
        640 | 0x8000,
        fakeOffGWorld.pixels,
        1,
    },
    &fakeOffGWorld.pixMap,
};

GWorld fakeRealGWorld = {
    { },
    {
        { 0, 0, 640, 480 },
        &fakeCTabPtr,
        640 | 0x8000,
        fakeRealGWorld.pixels,
        1,
    },
    &fakeRealGWorld.pixMap,
};

GWorld fakeSaveGWorld = {
    { },
    {
        { 0, 0, 640, 480 },
        &fakeCTabPtr,
        640 | 0x8000,
        fakeSaveGWorld.pixels,
        1,
    },
    &fakeSaveGWorld.pixMap,
};

Window fakeWindow = {
    { 0, 0, 640, 480 },
    fakeRealGWorld.pixMap,
};

GDevice fakeGDevice = {
    &fakeRealGWorld.pixMapPtr,
    { 0, 0, 640, 480 },
    &fakeRealGWorld,
};
GDevice* fakeGDevicePtr = &fakeGDevice;

void GetGWorld(GWorld** world, GDevice*** device) {
    *world = fakeGDevice.world;
    *device = &fakeGDevicePtr;
}

void SetGWorld(GWorld* world, GDevice***) {
    fakeGDevice.world = world;
}

OSErr NewGWorld(GWorld** world, int, Rect*, CTabHandle, GDHandle device, int) {
    assert(device == &fakeGDevicePtr);
    if (world == &gOffWorld) {
        *world = &fakeOffGWorld;
    } else if (world == &gRealWorld) {
        *world = &fakeRealGWorld;
    } else if (world == &gSaveWorld) {
        *world = &fakeSaveGWorld;
    } else {
        assert(false);
    }
    return noErr;
}

void DisposeGWorld(GWorld*) {
    assert(false);
}

PixMap** GetGWorldPixMap(GWorld* world) {
    if (world == gOffWorld) {
        return fakeGDevice.gdPMap;
    } else if (world == gRealWorld) {
        return fakeGDevice.gdPMap;
    } else if (world == gSaveWorld) {
        return fakeGDevice.gdPMap;
    } else {
        assert(false);
    }
}

static uint8_t NearestColor(uint16_t red, uint16_t green, uint16_t blue) {
    uint8_t best_color = 0;
    int min_distance = std::numeric_limits<int>::max();
    for (int i = 0; i < 256; ++i) {
        int distance = abs(fakeCTabPtr->ctTable[i].rgb.red - red)
            + abs(fakeCTabPtr->ctTable[i].rgb.green - green)
            + abs(fakeCTabPtr->ctTable[i].rgb.blue - blue);
        if (distance == 0) {
            return i;
        } else if (distance < min_distance) {
            min_distance = distance;
            best_color = i;
        }
    }
    return best_color;
}

static uint8_t GetPixel(int x, int y) {
    const PixMap* p = &fakeWindow.portBits;
    return p->baseAddr[x + y * (p->rowBytes & 0x7fff)];
}

static void SetPixel(int x, int y, uint8_t c) {
    const PixMap* p = *fakeGDevice.gdPMap;
    p->baseAddr[x + y * (p->rowBytes & 0x7fff)] = c;
}

static void SetPixelRow(int x, int y, uint8_t* c, int count) {
    const PixMap* p = *fakeGDevice.gdPMap;
    memcpy(&p->baseAddr[x + y * (p->rowBytes & 0x7fff)], c, count);
}

void CopyBits(BitMap* source, BitMap* dest, Rect* source_rect, Rect* dest_rect, int mode, void*) {
    if (source == dest) {
        return;
    }

    int width = source_rect->right - source_rect->left;
    int height = source_rect->bottom - source_rect->top;
    assert(width == dest_rect->right - dest_rect->left);
    assert(height == dest_rect->bottom - dest_rect->top);

    for (int i = 0; i < height; ++i) {
        int source_y = source_rect->top + i;
        int dest_y = dest_rect->top + i;
        char* sourceBytes = source->baseAddr + source_rect->left + source_y * (source->rowBytes & 0x7fff);
        char* destBytes = dest->baseAddr + dest_rect->left + dest_y * (dest->rowBytes & 0x7fff);
        memcpy(destBytes, sourceBytes, width);
    }
}

struct PicData {
    png::image<png::rgba_pixel> image;
    int height;
    int width;
    uint8_t** data;
    PicData(const std::string& filename)
            : image(filename), height(image.get_height()), width(image.get_width()) {
        data = new uint8_t*[height];
        for (int i = 0; i < height; ++i) {
            data[i] = new uint8_t[width];
            for (int j = 0; j < width; ++j) {
                const png::rgba_pixel& p = image[i][j];
                uint16_t red = p.red | (uint16_t)p.red << 8;
                uint16_t green = p.green | (uint16_t)p.green << 8;
                uint16_t blue = p.blue | (uint16_t)p.blue << 8;
                data[i][j] = NearestColor(red, green, blue);
            }
        }
    }
    ~PicData() {
        for (int i = 0; i < height; ++i) {
            delete[] data[i];
        }
        delete[] data;
    }
};

Pic** GetPicture(int id) {
    char fileglob[64];
    sprintf(fileglob, "data/derived/Pictures/%d*.png", id);
    glob_t g;
    g.gl_offs = 0;
    glob(fileglob, 0, NULL, &g);
    assert(g.gl_matchc == 1);
    std::string filename = g.gl_pathv[0];
    globfree(&g);

    Pic* p = new Pic;
    p->data = new PicData(filename);
    SetRect(&p->picFrame, 0, 0, p->data->image.get_width(), p->data->image.get_height());
    return new Pic*(p);
}

Pic** OpenPicture(Rect* source) {
    assert(false);
}

void KillPicture(Pic** pic) {
    delete *pic;
    delete pic;
}

Rect ClipRectToRect(const Rect& src, const Rect& clip) {
    Rect result = {
        std::max(src.left, clip.left),
        std::max(src.top, clip.top),
        std::min(src.right, clip.right),
        std::min(src.bottom, clip.bottom),
    };
    return result;
}

void DrawPicture(Pic** pic, Rect* rect) {
    Rect clipped = ClipRectToRect(*rect, (*fakeGDevice.gdPMap)->bounds);
    for (int y = clipped.top; y < clipped.bottom; ++y) {
        SetPixelRow(clipped.left, y, (*pic)->data->data[y - rect->top], clipped.right - clipped.left);
    }
}

void ClosePicture() {
    assert(false);
}

int currentForeColor;
int currentBackColor;

void RGBForeColor(RGBColor* color) {
    currentForeColor = NearestColor(color->red, color->green, color->blue);
}

void RGBBackColor(RGBColor* color) {
    currentBackColor = NearestColor(color->red, color->green, color->blue);
}

void PaintRect(Rect* rect) {
    Rect clipped = ClipRectToRect(*rect, (*fakeGDevice.gdPMap)->bounds);
    for (int y = clipped.top; y < clipped.bottom; ++y) {
        for (int x = clipped.left; x < clipped.right; ++x) {
            SetPixel(x, y, currentForeColor);
        }
    }
}

void EraseRect(Rect* rect) {
    Rect clipped = ClipRectToRect(*rect, (*fakeGDevice.gdPMap)->bounds);
    for (int y = clipped.top; y < clipped.bottom; ++y) {
        for (int x = clipped.left; x < clipped.right; ++x) {
            SetPixel(x, y, currentBackColor);
        }
    }
}

void FrameRect(Rect* rect) {
    Rect clipped = ClipRectToRect(*rect, (*fakeGDevice.gdPMap)->bounds);
    if (clipped.left == clipped.right || clipped.top == clipped.bottom) {
        return;
    }
    for (int x = clipped.left; x < clipped.right; ++x) {
        if (rect->top == clipped.top) {
            SetPixel(x, rect->top, currentForeColor);
        }
        if (rect->bottom == clipped.bottom) {
            SetPixel(x, rect->bottom - 1, currentForeColor);
        }
    }
    for (int y = clipped.top; y < clipped.bottom; ++y) {
        if (rect->left == clipped.left) {
            SetPixel(rect->left, y, currentForeColor);
        }
        if (rect->right == clipped.right) {
            SetPixel(rect->right - 1, y, currentForeColor);
        }
    }
}

void MacFrameRect(Rect* rect) {
    FrameRect(rect);
}

void Index2Color(long index, RGBColor* color) {
    color->red = fakeCTabPtr->ctTable[index].rgb.red;
    color->green = fakeCTabPtr->ctTable[index].rgb.green;
    color->blue = fakeCTabPtr->ctTable[index].rgb.blue;
}

Point currentPen = { 0, 0 };

void MoveTo(int x, int y) {
    currentPen.h = x;
    currentPen.v = y;
}

void GetPen(Point* pen) {
    *pen = currentPen;
}

void GetMouse(Point* point) {
    point->h = 320;
    point->v = 240;
}

uint16_t DoubleBits(uint8_t in) {
    uint16_t result = in;
    result <<= 8;
    result |= in;
    return result;
}

CTab* NewColorTable() {
    CTab* ctab = new CTab;
    ctab->ctSize = 256;
    ctab->ctTable = new ColorSpec[256];
    for (int i = 0; i < 256; ++i) {
        ctab->ctTable[i].value = i;
        ctab->ctTable[i].rgb.red = DoubleBits(colors_24_bit[i].red);
        ctab->ctTable[i].rgb.green = DoubleBits(colors_24_bit[i].green);
        ctab->ctTable[i].rgb.blue = DoubleBits(colors_24_bit[i].blue);
    }
    return ctab;
}

void FakeInit(int argc, const char** argv) {
    (void)argc;
    (void)argv;
    fakeCTabPtr = NewColorTable();
}

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

ColorSpec colors[256] = {
    {0, {65535, 65535, 65535}},
    {1, {8224, 0, 0}},
    {2, {57568, 57568, 57568}},
    {3, {53456, 53456, 53456}},
    {4, {49344, 49344, 49344}},
    {5, {45232, 45232, 45232}},
    {6, {41120, 41120, 41120}},
    {7, {37008, 37008, 37008}},
    {8, {32896, 32896, 32896}},
    {9, {28784, 28784, 28784}},
    {10, {24672, 24672, 24672}},
    {11, {20560, 20560, 20560}},
    {12, {16448, 16448, 16448}},
    {13, {12336, 12336, 12336}},
    {14, {8224, 8224, 8224}},
    {15, {4112, 4112, 4112}},
    {16, {2056, 2056, 2056}},
    {17, {65535, 32639, 0}},
    {18, {61680, 30840, 0}},
    {19, {57568, 28784, 0}},
    {20, {53456, 26728, 0}},
    {21, {49344, 24672, 0}},
    {22, {45232, 22616, 0}},
    {23, {41120, 20560, 0}},
    {24, {37008, 18504, 0}},
    {25, {32896, 16448, 0}},
    {26, {28784, 14392, 0}},
    {27, {24672, 12336, 0}},
    {28, {20560, 10280, 0}},
    {29, {16448, 8224, 0}},
    {30, {12336, 6168, 0}},
    {31, {8224, 4112, 0}},
    {32, {4112, 2056, 0}},
    {33, {65535, 65535, 0}},
    {34, {61680, 61680, 0}},
    {35, {57568, 57568, 0}},
    {36, {53456, 53456, 0}},
    {37, {49344, 49344, 0}},
    {38, {45232, 45232, 0}},
    {39, {41120, 41120, 0}},
    {40, {37008, 37008, 0}},
    {41, {32896, 32896, 0}},
    {42, {28784, 28784, 0}},
    {43, {24672, 24672, 0}},
    {44, {20560, 20560, 0}},
    {45, {16448, 16448, 0}},
    {46, {12336, 12336, 0}},
    {47, {8224, 8224, 0}},
    {48, {4112, 4112, 0}},
    {49, {0, 0, 65535}},
    {50, {0, 0, 61680}},
    {51, {0, 0, 57568}},
    {52, {0, 0, 53456}},
    {53, {0, 0, 49344}},
    {54, {0, 0, 45232}},
    {55, {0, 0, 41120}},
    {56, {0, 0, 37008}},
    {57, {0, 0, 32896}},
    {58, {0, 0, 28784}},
    {59, {0, 0, 24672}},
    {60, {0, 0, 20560}},
    {61, {0, 0, 16448}},
    {62, {0, 0, 12336}},
    {63, {0, 0, 8224}},
    {64, {0, 0, 4112}},
    {65, {0, 65535, 0}},
    {66, {0, 61680, 0}},
    {67, {0, 57568, 0}},
    {68, {0, 53456, 0}},
    {69, {0, 49344, 0}},
    {70, {0, 45232, 0}},
    {71, {0, 41120, 0}},
    {72, {0, 37008, 0}},
    {73, {0, 32896, 0}},
    {74, {0, 28784, 0}},
    {75, {0, 24672, 0}},
    {76, {0, 20560, 0}},
    {77, {0, 16448, 0}},
    {78, {0, 12336, 0}},
    {79, {0, 8224, 0}},
    {80, {0, 4112, 0}},
    {81, {32639, 0, 65535}},
    {82, {30840, 0, 61680}},
    {83, {28784, 0, 57568}},
    {84, {26728, 0, 53456}},
    {85, {24672, 0, 49344}},
    {86, {22616, 0, 45232}},
    {87, {20560, 0, 41120}},
    {88, {18504, 0, 37008}},
    {89, {16448, 0, 32896}},
    {90, {14392, 0, 28784}},
    {91, {12336, 0, 24672}},
    {92, {10280, 0, 20560}},
    {93, {8224, 0, 16448}},
    {94, {6168, 0, 12336}},
    {95, {4112, 0, 8224}},
    {96, {2056, 0, 4112}},
    {97, {32639, 32639, 65535}},
    {98, {30840, 30840, 61680}},
    {99, {28784, 28784, 57568}},
    {100, {26728, 26728, 53456}},
    {101, {24672, 24672, 49344}},
    {102, {22616, 22616, 45232}},
    {103, {20560, 20560, 41120}},
    {104, {18504, 18504, 37008}},
    {105, {16448, 16448, 32896}},
    {106, {14392, 14392, 28784}},
    {107, {12336, 12336, 24672}},
    {108, {10280, 10280, 20560}},
    {109, {8224, 8224, 16448}},
    {110, {6168, 6168, 12336}},
    {111, {4112, 4112, 8224}},
    {112, {2056, 2056, 4112}},
    {113, {65535, 32639, 32639}},
    {114, {61680, 30840, 30840}},
    {115, {57568, 28784, 28784}},
    {116, {53456, 26728, 26728}},
    {117, {49344, 24672, 24672}},
    {118, {45232, 22616, 22616}},
    {119, {41120, 20560, 20560}},
    {120, {37008, 18504, 18504}},
    {121, {32896, 16448, 16448}},
    {122, {28784, 14392, 14392}},
    {123, {24672, 12336, 12336}},
    {124, {20560, 10280, 10280}},
    {125, {16448, 8224, 8224}},
    {126, {12336, 6168, 6168}},
    {127, {8224, 4112, 4112}},
    {128, {4112, 2056, 2056}},
    {129, {65535, 65535, 32639}},
    {130, {61680, 61680, 30840}},
    {131, {57568, 57568, 28784}},
    {132, {53456, 53456, 26728}},
    {133, {49344, 49344, 24672}},
    {134, {45232, 45232, 22616}},
    {135, {41120, 41120, 20560}},
    {136, {37008, 37008, 18504}},
    {137, {32896, 32896, 16448}},
    {138, {28784, 28784, 14392}},
    {139, {24672, 24672, 12336}},
    {140, {20560, 20560, 10280}},
    {141, {16448, 16448, 8224}},
    {142, {12336, 12336, 6168}},
    {143, {8224, 8224, 4112}},
    {144, {4112, 4112, 2056}},
    {145, {0, 65535, 65535}},
    {146, {0, 61680, 61680}},
    {147, {0, 57568, 57568}},
    {148, {0, 53456, 53456}},
    {149, {0, 49344, 49344}},
    {150, {0, 45232, 45232}},
    {151, {0, 41120, 41120}},
    {152, {0, 37008, 37008}},
    {153, {0, 32896, 32896}},
    {154, {0, 28784, 28784}},
    {155, {0, 24672, 24672}},
    {156, {0, 20560, 20560}},
    {157, {0, 16448, 16448}},
    {158, {0, 12336, 12336}},
    {159, {0, 8224, 8224}},
    {160, {0, 4112, 4112}},
    {161, {65535, 0, 32639}},
    {162, {61680, 0, 30840}},
    {163, {57568, 0, 28784}},
    {164, {53456, 0, 26728}},
    {165, {49344, 0, 24672}},
    {166, {45232, 0, 22616}},
    {167, {41120, 0, 20560}},
    {168, {37008, 0, 18504}},
    {169, {32896, 0, 16448}},
    {170, {28784, 0, 14392}},
    {171, {24672, 0, 12336}},
    {172, {20560, 0, 10280}},
    {173, {16448, 0, 8224}},
    {174, {12336, 0, 6168}},
    {175, {8224, 0, 4112}},
    {176, {4112, 0, 2056}},
    {177, {32639, 65535, 32639}},
    {178, {30840, 61680, 30840}},
    {179, {28784, 57568, 28784}},
    {180, {26728, 53456, 26728}},
    {181, {24672, 49344, 24672}},
    {182, {22616, 45232, 22616}},
    {183, {20560, 41120, 20560}},
    {184, {18504, 37008, 18504}},
    {185, {16448, 32896, 16448}},
    {186, {14392, 28784, 14392}},
    {187, {12336, 24672, 12336}},
    {188, {10280, 20560, 10280}},
    {189, {8224, 16448, 8224}},
    {190, {6168, 12336, 6168}},
    {191, {4112, 8224, 4112}},
    {192, {2056, 4112, 2056}},
    {193, {65535, 32639, 65535}},
    {194, {61680, 30840, 61680}},
    {195, {57568, 28784, 57568}},
    {196, {53456, 26728, 53456}},
    {197, {49344, 24672, 49344}},
    {198, {45232, 22616, 45232}},
    {199, {41120, 20560, 41120}},
    {200, {37008, 18504, 36751}},
    {201, {32896, 16448, 32896}},
    {202, {28784, 14392, 28784}},
    {203, {24672, 12336, 24672}},
    {204, {20560, 10280, 20560}},
    {205, {16448, 8224, 16448}},
    {206, {12336, 6168, 12336}},
    {207, {8224, 4112, 8224}},
    {208, {4112, 2056, 4112}},
    {209, {0, 32639, 65535}},
    {210, {0, 30840, 61680}},
    {211, {0, 28784, 57568}},
    {212, {0, 26728, 53456}},
    {213, {0, 24672, 49344}},
    {214, {0, 22616, 45232}},
    {215, {0, 20560, 41120}},
    {216, {0, 18504, 36751}},
    {217, {0, 16448, 32896}},
    {218, {0, 14392, 28784}},
    {219, {0, 12336, 24672}},
    {220, {0, 10280, 20560}},
    {221, {0, 8224, 16448}},
    {222, {0, 6168, 12336}},
    {223, {0, 4112, 8224}},
    {224, {0, 2056, 4112}},
    {225, {65535, 63993, 53199}},
    {226, {61680, 60138, 50115}},
    {227, {57825, 56540, 47031}},
    {228, {53970, 52685, 43947}},
    {229, {50115, 48830, 40863}},
    {230, {46260, 45232, 37522}},
    {231, {42405, 41377, 34438}},
    {232, {38550, 37522, 31354}},
    {233, {34695, 33924, 28270}},
    {234, {30840, 30069, 24929}},
    {235, {26985, 26214, 21845}},
    {236, {23130, 22616, 18761}},
    {237, {19275, 18761, 15677}},
    {238, {15420, 14906, 12336}},
    {239, {11565, 11308, 9252}},
    {240, {7710, 7453, 6168}},
    {241, {65535, 0, 0}},
    {242, {61680, 0, 0}},
    {243, {57825, 0, 0}},
    {244, {53456, 0, 0}},
    {245, {49344, 0, 0}},
    {246, {45232, 0, 0}},
    {247, {41120, 0, 0}},
    {248, {37008, 0, 0}},
    {249, {32896, 0, 0}},
    {250, {28784, 0, 0}},
    {251, {24672, 0, 0}},
    {252, {20560, 0, 0}},
    {253, {16448, 0, 0}},
    {254, {12336, 0, 0}},
    {255, {0, 0, 0}},
};

CTab fakeCTab = {
    colors,
    255,
};
CTab* fakeCTabPtr = &fakeCTab;

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
    char filename[64];
    sprintf(filename, "dump-%05ld.pnm", gAresGlobal->gGameTime);
    FILE* f = fopen(filename, "w");
    fprintf(f, "P3\n");
    fprintf(f, "640 480\n");
    fprintf(f, "255\n");
    for (int y = 0; y < 480; ++y) {
        for (int x = 0; x < 640; ++x) {
            const RGBColor& c = colors[GetPixel(x, y)].rgb;
            fprintf(f, "%d %d %d\n", c.red >> 8, c.green >> 8, c.blue >> 8);
        }
    }
    fclose(f);
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
        int distance = abs(colors[i].rgb.red - red)
            + abs(colors[i].rgb.green - green)
            + abs(colors[i].rgb.blue - blue);
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
    color->red = colors[index].rgb.red;
    color->green = colors[index].rgb.green;
    color->blue = colors[index].rgb.blue;
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

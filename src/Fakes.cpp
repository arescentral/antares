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

static void* const kMmapFailed = (void*)-1;

ColorSpec colors[256] = {
    {0, {0, 0, 0}},
    {1, {224, 224, 224}},
    {2, {208, 208, 208}},
    {3, {192, 192, 192}},
    {4, {176, 176, 176}},
    {5, {160, 160, 160}},
    {6, {144, 144, 144}},
    {7, {128, 128, 128}},
    {8, {112, 112, 112}},
    {9, {96, 96, 96}},
    {10, {80, 80, 80}},
    {11, {64, 64, 64}},
    {12, {48, 48, 48}},
    {13, {32, 32, 32}},
    {14, {16, 16, 16}},
    {15, {8, 8, 8}},
    {16, {255, 127, 0}},
    {17, {240, 120, 0}},
    {18, {224, 112, 0}},
    {19, {208, 104, 0}},
    {20, {192, 96, 0}},
    {21, {176, 88, 0}},
    {22, {160, 80, 0}},
    {23, {144, 72, 0}},
    {24, {128, 64, 0}},
    {25, {112, 56, 0}},
    {26, {96, 48, 0}},
    {27, {80, 40, 0}},
    {28, {64, 32, 0}},
    {29, {48, 24, 0}},
    {30, {32, 16, 0}},
    {31, {16, 8, 0}},
    {32, {255, 255, 0}},
    {33, {240, 240, 0}},
    {34, {224, 224, 0}},
    {35, {208, 208, 0}},
    {36, {192, 192, 0}},
    {37, {176, 176, 0}},
    {38, {160, 160, 0}},
    {39, {144, 144, 0}},
    {40, {128, 128, 0}},
    {41, {112, 112, 0}},
    {42, {96, 96, 0}},
    {43, {80, 80, 0}},
    {44, {64, 64, 0}},
    {45, {48, 48, 0}},
    {46, {32, 32, 0}},
    {47, {16, 16, 0}},
    {48, {0, 0, 255}},
    {49, {0, 0, 240}},
    {50, {0, 0, 224}},
    {51, {0, 0, 208}},
    {52, {0, 0, 192}},
    {53, {0, 0, 176}},
    {54, {0, 0, 160}},
    {55, {0, 0, 144}},
    {56, {0, 0, 128}},
    {57, {0, 0, 112}},
    {58, {0, 0, 96}},
    {59, {0, 0, 80}},
    {60, {0, 0, 64}},
    {61, {0, 0, 48}},
    {62, {0, 0, 32}},
    {63, {0, 0, 16}},
    {64, {0, 255, 0}},
    {65, {0, 240, 0}},
    {66, {0, 224, 0}},
    {67, {0, 208, 0}},
    {68, {0, 192, 0}},
    {69, {0, 176, 0}},
    {70, {0, 160, 0}},
    {71, {0, 144, 0}},
    {72, {0, 128, 0}},
    {73, {0, 112, 0}},
    {74, {0, 96, 0}},
    {75, {0, 80, 0}},
    {76, {0, 64, 0}},
    {77, {0, 48, 0}},
    {78, {0, 32, 0}},
    {79, {0, 16, 0}},
    {80, {127, 0, 255}},
    {81, {120, 0, 240}},
    {82, {112, 0, 224}},
    {83, {104, 0, 208}},
    {84, {96, 0, 192}},
    {85, {88, 0, 176}},
    {86, {80, 0, 160}},
    {87, {72, 0, 144}},
    {88, {64, 0, 128}},
    {89, {56, 0, 112}},
    {90, {48, 0, 96}},
    {91, {40, 0, 80}},
    {92, {32, 0, 64}},
    {93, {24, 0, 48}},
    {94, {16, 0, 32}},
    {95, {8, 0, 16}},
    {96, {127, 127, 255}},
    {97, {120, 120, 240}},
    {98, {112, 112, 224}},
    {99, {104, 104, 208}},
    {100, {96, 96, 192}},
    {101, {88, 88, 176}},
    {102, {80, 80, 160}},
    {103, {72, 72, 144}},
    {104, {64, 64, 128}},
    {105, {56, 56, 112}},
    {106, {48, 48, 96}},
    {107, {40, 40, 80}},
    {108, {32, 32, 64}},
    {109, {24, 24, 48}},
    {110, {16, 16, 32}},
    {111, {8, 8, 16}},
    {112, {255, 127, 127}},
    {113, {240, 120, 120}},
    {114, {224, 112, 112}},
    {115, {208, 104, 104}},
    {116, {192, 96, 96}},
    {117, {176, 88, 88}},
    {118, {160, 80, 80}},
    {119, {144, 72, 72}},
    {120, {128, 64, 64}},
    {121, {112, 56, 56}},
    {122, {96, 48, 48}},
    {123, {80, 40, 40}},
    {124, {64, 32, 32}},
    {125, {48, 24, 24}},
    {126, {32, 16, 16}},
    {127, {16, 8, 8}},
    {128, {255, 255, 127}},
    {129, {240, 240, 120}},
    {130, {224, 224, 112}},
    {131, {208, 208, 104}},
    {132, {192, 192, 96}},
    {133, {176, 176, 88}},
    {134, {160, 160, 80}},
    {135, {144, 144, 72}},
    {136, {128, 128, 64}},
    {137, {112, 112, 56}},
    {138, {96, 96, 48}},
    {139, {80, 80, 40}},
    {140, {64, 64, 32}},
    {141, {48, 48, 24}},
    {142, {32, 32, 16}},
    {143, {16, 16, 8}},
    {144, {0, 255, 255}},
    {145, {0, 240, 240}},
    {146, {0, 224, 224}},
    {147, {0, 208, 208}},
    {148, {0, 192, 192}},
    {149, {0, 176, 176}},
    {150, {0, 160, 160}},
    {151, {0, 144, 144}},
    {152, {0, 128, 128}},
    {153, {0, 112, 112}},
    {154, {0, 96, 96}},
    {155, {0, 80, 80}},
    {156, {0, 64, 64}},
    {157, {0, 48, 48}},
    {158, {0, 32, 32}},
    {159, {0, 16, 16}},
    {160, {255, 0, 127}},
    {161, {240, 0, 120}},
    {162, {224, 0, 112}},
    {163, {208, 0, 104}},
    {164, {192, 0, 96}},
    {165, {176, 0, 88}},
    {166, {160, 0, 80}},
    {167, {144, 0, 72}},
    {168, {128, 0, 64}},
    {169, {112, 0, 56}},
    {170, {96, 0, 48}},
    {171, {80, 0, 40}},
    {172, {64, 0, 32}},
    {173, {48, 0, 24}},
    {174, {32, 0, 16}},
    {175, {16, 0, 8}},
    {176, {127, 255, 127}},
    {177, {120, 240, 120}},
    {178, {112, 224, 112}},
    {179, {104, 208, 104}},
    {180, {96, 192, 96}},
    {181, {88, 176, 88}},
    {182, {80, 160, 80}},
    {183, {72, 144, 72}},
    {184, {64, 128, 64}},
    {185, {56, 112, 56}},
    {186, {48, 96, 48}},
    {187, {40, 80, 40}},
    {188, {32, 64, 32}},
    {189, {24, 48, 24}},
    {190, {16, 32, 16}},
    {191, {8, 16, 8}},
    {192, {255, 127, 255}},
    {193, {240, 120, 240}},
    {194, {224, 112, 224}},
    {195, {208, 104, 208}},
    {196, {192, 96, 192}},
    {197, {176, 88, 176}},
    {198, {160, 80, 160}},
    {199, {144, 72, 143}},
    {200, {128, 64, 128}},
    {201, {112, 56, 112}},
    {202, {96, 48, 96}},
    {203, {80, 40, 80}},
    {204, {64, 32, 64}},
    {205, {48, 24, 48}},
    {206, {32, 16, 32}},
    {207, {16, 8, 16}},
    {208, {0, 127, 255}},
    {209, {0, 120, 240}},
    {210, {0, 112, 224}},
    {211, {0, 104, 208}},
    {212, {0, 96, 192}},
    {213, {0, 88, 176}},
    {214, {0, 80, 160}},
    {215, {0, 72, 143}},
    {216, {0, 64, 128}},
    {217, {0, 56, 112}},
    {218, {0, 48, 96}},
    {219, {0, 40, 80}},
    {220, {0, 32, 64}},
    {221, {0, 24, 48}},
    {222, {0, 16, 32}},
    {223, {0, 8, 16}},
    {224, {255, 249, 207}},
    {225, {240, 234, 195}},
    {226, {225, 220, 183}},
    {227, {210, 205, 171}},
    {228, {195, 190, 159}},
    {229, {180, 176, 146}},
    {230, {165, 161, 134}},
    {231, {150, 146, 122}},
    {232, {135, 132, 110}},
    {233, {120, 117, 97}},
    {234, {105, 102, 85}},
    {235, {90, 88, 73}},
    {236, {75, 73, 61}},
    {237, {60, 58, 48}},
    {238, {45, 44, 36}},
    {239, {30, 29, 24}},
    {240, {255, 0, 0}},
    {241, {240, 0, 0}},
    {242, {225, 0, 0}},
    {243, {208, 0, 0}},
    {244, {192, 0, 0}},
    {245, {176, 0, 0}},
    {246, {160, 0, 0}},
    {247, {144, 0, 0}},
    {248, {128, 0, 0}},
    {249, {112, 0, 0}},
    {250, {96, 0, 0}},
    {251, {80, 0, 0}},
    {252, {64, 0, 0}},
    {253, {48, 0, 0}},
    {254, {0, 0, 0}},
    {255, {255, 255, 255}},
};

CTab fakeCTab = {
    colors,
    255,
};
CTab* fakeCTabPtr = &fakeCTab;

Window fakeWindow = {
    { 0, 0, 640, 480 },
    { },
};

CWindow fakeCWindow = {
    { 0, 0, 640, 480 },
};

AuxWin fakeAuxWin = {
    &fakeCTabPtr,
};
AuxWin* fakeAuxWinPtr = &fakeAuxWin;

static uint8_t GetPixel(int x, int y);
static void SetPixel(int x, int y, uint8_t c);

void Dump() {
    FILE* f = fopen("dump.pnm", "w");
    fprintf(f, "P2\n");
    fprintf(f, "640 480\n");
    fprintf(f, "15\n");
    for (int y = 0; y < 480; ++y) {
        for (int x = 0; x < 640; ++x) {
            fprintf(f, "%d\n", GetPixel(x, y) & 0xF);
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
        delete[] (char*)data;
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
    return tv.tv_sec * 60.0 + tv.tv_usec / (60.0 * 1000000.0);
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
    return 0;
}

// Perform this many clicks in succession at the start of the game.
const int kClickCount = 12;

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

long AngleFromSlope(Fixed slope) {
    double d = slope / 256.0;
    return atan(d) * 180.0 / M_PI;
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
        640,
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
        640,
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
        640,
        fakeSaveGWorld.pixels,
        1,
    },
    &fakeSaveGWorld.pixMap,
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

static uint8_t GetPixel(int x, int y) {
    const PixMap* p = *fakeGDevice.gdPMap;
    return p->baseAddr[x + y * p->rowBytes];
}

static void SetPixel(int x, int y, uint8_t c) {
    const PixMap* p = *fakeGDevice.gdPMap;
    p->baseAddr[x + y * p->rowBytes] = c;
}

void CopyBits(BitMap* source, BitMap* dest, Rect* source_rect, Rect* dest_rect, int mode, void*) {
    PixMap* source_pix = reinterpret_cast<PixMap*>(source);
    PixMap* dest_pix = reinterpret_cast<PixMap*>(dest);
}

struct PicData {
    png::image<png::rgba_pixel> image;
    PicData(const std::string& filename) : image(filename) { }
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
        for (int x = clipped.left; x < clipped.right; ++x) {
            const png::rgba_pixel& p = (*pic)->data->image[y - rect->top][x - rect->left];
            SetPixel(x, y, p.red / 16);
        }
    }
}

void ClosePicture() {
    assert(false);
}

int currentForeColor;
int currentBackColor;

void RGBForeColor(RGBColor* color) { currentForeColor = color->red / 16; }
void RGBBackColor(RGBColor* color) { currentBackColor = color->red / 16; }

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
    for (int x = clipped.left; x < clipped.right; ++x) {
        if (rect->top == clipped.top) {
            SetPixel(x, rect->top, currentForeColor);
        }
        if (rect->bottom == clipped.bottom) {
            SetPixel(x, rect->bottom, currentForeColor);
        }
    }
    for (int y = clipped.top; y < clipped.bottom; ++y) {
        if (rect->left == clipped.left) {
            SetPixel(rect->left, y, currentForeColor);
        }
        if (rect->right == clipped.right) {
            SetPixel(rect->right, y, currentForeColor);
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

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

// Races.c

#include <Quickdraw.h>
#include <Resources.h>
#include <Sound.h>

#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <limits>

static void* const kMmapFailed = (void*)-1;

CTab fakeCTab = {
    new ColorSpec[256],
    256,
};
CTab* fakeCTabPtr = &fakeCTab;

PixMap fakePixMap = {
    { 0, 0, 480, 640 },
    &fakeCTabPtr,
    640,
    new char[640 * 480],
    1,
};
PixMap* fakePixMapPtr = &fakePixMap;

GDevice fakeGDevice = {
    &fakePixMapPtr,
    { 0, 0, 480, 640 },
};
GDevice* fakeGDevicePtr = &fakeGDevice;

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

void MacSetRect(Rect* rect, int left, int top, int right, int bottom) {
    rect->left = left;
    rect->top = top;
    rect->right = right;
    rect->bottom = bottom;
}

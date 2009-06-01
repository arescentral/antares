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

#include "FakeDrawing.hpp"

#include <assert.h>
#include <fcntl.h>
#include <glob.h>
#include <algorithm>
#include <limits>
#include <string>
#include <png++/png.hpp>

#include "FakeHandles.hpp"
#include "Fakes.hpp"

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

CTabHandle fakeCTabHandle;

void Dump() {
    std::string contents = "P6 640 480 255\n";
    contents.reserve(contents.size() + 640 * 480 * 3);
    for (int y = 0; y < 480; ++y) {
        for (int x = 0; x < 640; ++x) {
            int color = GetPixel(x, y);
            const char pixel[3] = {
                (*fakeCTabHandle)->ctTable[color].rgb.red >> 8,
                (*fakeCTabHandle)->ctTable[color].rgb.green >> 8,
                (*fakeCTabHandle)->ctTable[color].rgb.blue >> 8,
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

GWorld fakeOffGWorld(640, 480);
GWorld fakeRealGWorld(640, 480);
GWorld fakeSaveGWorld(640, 480);
FakeWindow fakeWindow(640, 480, &fakeRealGWorld);
FakeGDevice fakeGDevice(640, 480, &fakeRealGWorld);

GDevice* fakeGDevicePtr = &fakeGDevice;

Window* NewWindow(
        void*, Rect* rect, const unsigned char* title, bool, int, Window* behind, bool, int id) {
    (void)rect;
    (void)title;
    (void)behind;
    (void)id;
    return &fakeWindow;
}

CWindow* NewCWindow(
        void*, Rect* rect, const unsigned char* title, bool, int, Window* behind, bool, int id) {
    (void)rect;
    (void)title;
    (void)behind;
    (void)id;
    return &fakeWindow;
}

void GetPort(Window** port) {
    *port = &fakeWindow;
}

void MacSetPort(Window* port) {
    (void)port;
}

void GetGWorld(GWorld** world, GDevice*** device) {
    *world = fakeGDevice.world;
    *device = &fakeGDevicePtr;
}

void SetGWorld(GWorld* world, GDevice***) {
    fakeGDevice.world = world;
    fakeGDevice.gdPMap = &world->pixMapPtr;
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
    return &world->pixMapPtr;
}

uint8_t NearestColor(uint16_t red, uint16_t green, uint16_t blue) {
    uint8_t best_color = 0;
    int min_distance = std::numeric_limits<int>::max();
    for (int i = 0; i < 256; ++i) {
        int distance = abs((*fakeCTabHandle)->ctTable[i].rgb.red - red)
            + abs((*fakeCTabHandle)->ctTable[i].rgb.green - green)
            + abs((*fakeCTabHandle)->ctTable[i].rgb.blue - blue);
        if (distance == 0) {
            return i;
        } else if (distance < min_distance) {
            min_distance = distance;
            best_color = i;
        }
    }
    return best_color;
}

uint8_t GetPixel(int x, int y) {
    const PixMap* p = &fakeWindow.portBits;
    return p->baseAddr[x + y * (p->rowBytes & 0x7fff)];
}

void SetPixel(int x, int y, uint8_t c) {
    const PixMap* p = *fakeGDevice.gdPMap;
    p->baseAddr[x + y * (p->rowBytes & 0x7fff)] = c;
}

void SetPixelRow(int x, int y, uint8_t* c, int count) {
    const PixMap* p = *fakeGDevice.gdPMap;
    memcpy(&p->baseAddr[x + y * (p->rowBytes & 0x7fff)], c, count);
}

Point MakePoint(int x, int y) {
    Point result = { x, y };
    return result;
}

class ClippedTransfer {
  public:
    ClippedTransfer(const Rect& from, const Rect& to)
            : _from(from),
              _to(to) {
        // Rects must be the same size.
        assert(_from.right - _from.left == _to.right - _to.left);
        assert(_from.bottom - _from.top == _to.bottom - _to.top);
    }

    void ClipSourceTo(const Rect& clip) {
        ClipFirstToSecond(_from, clip);
    }

    void ClipDestTo(const Rect& clip) {
        ClipFirstToSecond(_to, clip);
    }

    int Height() const { return _from.bottom - _from.top; }
    int Width() const { return _from.right - _from.left; }

    int SourceRow(int i) const { return _from.top + i; }
    int SourceColumn(int i) const { return _from.left + i; }

    int DestRow(int i) const { return _to.top + i; }
    int DestColumn(int i) const { return _to.left + i; }

  private:
    inline void ClipFirstToSecond(const Rect& rect, const Rect& clip) {
        if (clip.left > rect.left) {
            int diff = clip.left - _from.left;
            _to.left += diff;
            _from.left += diff;
        }
        if (clip.top > rect.top) {
            int diff = clip.top - _from.top;
            _to.top += diff;
            _from.top += diff;
        }
        if (clip.right < rect.right) {
            int diff = clip.right - _from.right;
            _to.right += diff;
            _from.right += diff;
        }
        if (clip.bottom < rect.bottom) {
            int diff = clip.bottom - _from.bottom;
            _to.bottom += diff;
            _from.bottom += diff;
        }
    }

    Rect _from;
    Rect _to;
};

void CopyBits(BitMap* source, BitMap* dest, Rect* source_rect, Rect* dest_rect, int mode, void*) {
    if (source == dest) {
        return;
    }

    ClippedTransfer transfer(*source_rect, *dest_rect);
    transfer.ClipSourceTo(source->bounds);
    transfer.ClipDestTo(dest->bounds);

    for (int i = 0; i < transfer.Height(); ++i) {
        char* sourceBytes
            = source->baseAddr
            + transfer.SourceColumn(0)
            + transfer.SourceRow(i) * (source->rowBytes & 0x7fff);

        char* destBytes
            = dest->baseAddr
            + transfer.DestColumn(0)
            + transfer.DestRow(i) * (dest->rowBytes & 0x7fff);

        memcpy(destBytes, sourceBytes, transfer.Width());
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
    color->red = (*fakeCTabHandle)->ctTable[index].rgb.red;
    color->green = (*fakeCTabHandle)->ctTable[index].rgb.green;
    color->blue = (*fakeCTabHandle)->ctTable[index].rgb.blue;
}

Point currentPen = { 0, 0 };

void MoveTo(int x, int y) {
    currentPen.h = x;
    currentPen.v = y;
}

void MacLineTo(int h, int v) {
    assert(h == currentPen.h || v == currentPen.v);  // no diagonal lines yet.
    if (h == currentPen.h) {
        int step = 1;
        if (v < currentPen.v) {
            step = -1;
        }
        for (int i = currentPen.v; i != v; i += step) {
            SetPixel(currentPen.h, i, currentForeColor);
        }
        currentPen.v = v;
    } else {
        int step = 1;
        if (h < currentPen.h) {
            step = -1;
        }
        for (int i = currentPen.h; i != h; i += step) {
            SetPixel(i, currentPen.v, currentForeColor);
        }
        currentPen.h = h;
    }
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

CTab** NewColorTable() {
    HandleData<CTab>* handle = new HandleData<CTab>;
    CTab* ctab = handle->TypedData();
    ctab->ctSize = 256;
    ctab->ctTable = new ColorSpec[256];
    for (int i = 0; i < 256; ++i) {
        ctab->ctTable[i].value = i;
        ctab->ctTable[i].rgb.red = DoubleBits(colors_24_bit[i].red);
        ctab->ctTable[i].rgb.green = DoubleBits(colors_24_bit[i].green);
        ctab->ctTable[i].rgb.blue = DoubleBits(colors_24_bit[i].blue);
    }
    return handle->ToTypedHandle();
}

CTab** GetCTable(int id) {
    assert(id == 256);
    return NewColorTable();
}

void FakeDrawingInit() {
    fakeCTabHandle = NewColorTable();
    fakeOffGWorld.pixMap.pmTable = NewColorTable();
    fakeRealGWorld.pixMap.pmTable = NewColorTable();
    fakeSaveGWorld.pixMap.pmTable = NewColorTable();
}

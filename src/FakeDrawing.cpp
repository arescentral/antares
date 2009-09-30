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
#include <libkern/OSByteOrder.h>
#include <algorithm>
#include <limits>
#include <string>

#include "BinaryStream.hpp"
#include "ColorTable.hpp"
#include "FakeHandles.hpp"
#include "Fakes.hpp"
#include "File.hpp"

Window fakeWindow(640, 480);

extern PixMap* gActiveWorld;

namespace {

class StringBinaryWriter : public BinaryWriter {
public:
    StringBinaryWriter(std::string* out)
        : _out(out) { }

  protected:
    virtual void write_bytes(const char* bytes, size_t count) {
        _out->append(bytes, count);
    }

  private:
    std::string* _out;
};

}  // namespace

PixMap::PixMap(int width, int height) {
    SetRect(&bounds, 0, 0, width, height);
    rowBytes = width | 0x8000;
    baseAddr = new unsigned char[width * height];
    pixelSize = 1;
    colors = new ColorTable(256);
}

PixMap::~PixMap() {
    delete[] baseAddr;
}

Window::Window(int width, int height)
        : portBits(width, height) {
    SetRect(&portRect, 0, 0, width, height);
}

Window::~Window() { }

void DumpTo(const std::string& path) {
    std::string contents;
    StringBinaryWriter bin(&contents);

    bin.write<uint32_t>(640);
    bin.write<uint32_t>(480);

    for (size_t i = 0; i < 256; ++i) {
        RGBColor color = fakeWindow.portBits.colors->color(i);
        bin.write<uint32_t>(i);
        bin.write(color.red);
        bin.write(color.green);
        bin.write(color.blue);
    }

    const PixMap* p = &fakeWindow.portBits;
    bin.write(p->baseAddr, 640 * 480);

    MakeDirs(DirName(path), 0755);
    int fd = open(path.c_str(), O_WRONLY | O_CREAT, 0644);
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

uint8_t NearestColor(uint16_t red, uint16_t green, uint16_t blue) {
    uint8_t best_color = 0;
    int min_distance = std::numeric_limits<int>::max();
    for (int i = 0; i < 256; ++i) {
        int distance = abs(fakeWindow.portBits.colors->color(i).red - red)
            + abs(fakeWindow.portBits.colors->color(i).green - green)
            + abs(fakeWindow.portBits.colors->color(i).blue - blue);
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
    const PixMap* p = gActiveWorld;
    p->baseAddr[x + y * (p->rowBytes & 0x7fff)] = c;
}

void SetPixelRow(int x, int y, uint8_t* c, int count) {
    const PixMap* p = gActiveWorld;
    memcpy(&p->baseAddr[x + y * (p->rowBytes & 0x7fff)], c, count);
}

Point MakePoint(int x, int y) {
    Point result = { x, y };
    return result;
}

void CopyBits(PixMap* source, PixMap* dest, Rect* source_rect, Rect* dest_rect, int mode, void*) {
    static_cast<void>(mode);
    if (source == dest) {
        return;
    }

    ClippedTransfer transfer(*source_rect, *dest_rect);
    transfer.ClipSourceTo(source->bounds);
    transfer.ClipDestTo(dest->bounds);

    for (int i = 0; i < transfer.Height(); ++i) {
        unsigned char* sourceBytes
            = source->baseAddr
            + transfer.SourceColumn(0)
            + transfer.SourceRow(i) * (source->rowBytes & 0x7fff);

        unsigned char* destBytes
            = dest->baseAddr
            + transfer.DestColumn(0)
            + transfer.DestRow(i) * (dest->rowBytes & 0x7fff);

        memcpy(destBytes, sourceBytes, transfer.Width());
    }
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

int currentForeColor;
int currentBackColor;

void RGBForeColor(RGBColor* color) {
    currentForeColor = NearestColor(color->red, color->green, color->blue);
}

void RGBBackColor(RGBColor* color) {
    currentBackColor = NearestColor(color->red, color->green, color->blue);
}

void PaintRect(Rect* rect) {
    Rect clipped = ClipRectToRect(*rect, gActiveWorld->bounds);
    for (int y = clipped.top; y < clipped.bottom; ++y) {
        for (int x = clipped.left; x < clipped.right; ++x) {
            SetPixel(x, y, currentForeColor);
        }
    }
}

void MacFillRect(Rect* rect, Pattern* pattern) {
    static_cast<void>(pattern);
    Rect clipped = ClipRectToRect(*rect, gActiveWorld->bounds);
    for (int y = clipped.top; y < clipped.bottom; ++y) {
        for (int x = clipped.left; x < clipped.right; ++x) {
            SetPixel(x, y, 255);
        }
    }
}

void EraseRect(Rect* rect) {
    Rect clipped = ClipRectToRect(*rect, gActiveWorld->bounds);
    for (int y = clipped.top; y < clipped.bottom; ++y) {
        for (int x = clipped.left; x < clipped.right; ++x) {
            SetPixel(x, y, currentBackColor);
        }
    }
}

void FrameRect(Rect* rect) {
    Rect clipped = ClipRectToRect(*rect, gActiveWorld->bounds);
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
    color->red = fakeWindow.portBits.colors->color(index).red;
    color->green = fakeWindow.portBits.colors->color(index).green;
    color->blue = fakeWindow.portBits.colors->color(index).blue;
}

Point currentPen = { 0, 0 };

void MoveTo(int x, int y) {
    currentPen.h = x;
    currentPen.v = y;
}

bool IsOnScreen(int x, int y) {
    return 0 <= x && x < 640
        && 0 <= y && y < 480;
}

void MacLineTo(int h, int v) {
    assert(h == currentPen.h || v == currentPen.v);  // no diagonal lines yet.
    if (h == currentPen.h) {
        int step = 1;
        if (v < currentPen.v) {
            step = -1;
        }
        for (int i = currentPen.v; i != v; i += step) {
            if (IsOnScreen(currentPen.h, i)) {
                SetPixel(currentPen.h, i, currentForeColor);
            }
        }
        currentPen.v = v;
    } else {
        int step = 1;
        if (h < currentPen.h) {
            step = -1;
        }
        for (int i = currentPen.h; i != h; i += step) {
            if (IsOnScreen(i, currentPen.v)) {
                SetPixel(i, currentPen.v, currentForeColor);
            }
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

void RestoreEntries(const ColorTable& table) {
    for (size_t i = 0; i < table.size(); ++i) {
        fakeWindow.portBits.colors->set_color(i, table.color(i));
    }
}

void FakeDrawingInit() {
}

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
#include "Fakes.hpp"
#include "File.hpp"

namespace antares {

extern PixMap* gRealWorld;
extern PixMap* gActiveWorld;

namespace {

scoped_ptr<ColorTable> colors;

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

void DumpTo(const std::string& path) {
    std::string contents;
    StringBinaryWriter bin(&contents);

    uint32_t width = gRealWorld->bounds().right;
    uint32_t height = gRealWorld->bounds().bottom;

    bin.write<uint32_t>(width);
    bin.write<uint32_t>(height);

    for (size_t i = 0; i < 256; ++i) {
        RGBColor color = gRealWorld->colors().color(i);
        bin.write<uint32_t>(i);
        bin.write(color.red);
        bin.write(color.green);
        bin.write(color.blue);
    }

    bin.write(gRealWorld->bytes(), width * height);

    MakeDirs(DirName(path), 0755);
    int fd = open(path.c_str(), O_WRONLY | O_CREAT, 0644);
    write(fd, contents.c_str(), contents.size());
    close(fd);
}

void ScrollRect(const Rect& rect, int x, int y, const Rect& clip) {
    assert(x == 0 && y == -1);
    int rowBytes = gActiveWorld->row_bytes();
    for (int i = std::min(rect.top - 1, clip.top); i < rect.bottom - 1; ++i) {
        uint8_t* base = gActiveWorld->mutable_bytes() + i * rowBytes + rect.left;
        memcpy(base, base + rowBytes, rect.right - rect.left);
    }
}

uint8_t NearestColor(uint16_t red, uint16_t green, uint16_t blue) {
    uint8_t best_color = 0;
    int min_distance = std::numeric_limits<int>::max();
    for (int i = 0; i < 256; ++i) {
        int distance = abs(colors->color(i).red - red)
            + abs(colors->color(i).green - green)
            + abs(colors->color(i).blue - blue);
        if (distance == 0) {
            return i;
        } else if (distance < min_distance) {
            min_distance = distance;
            best_color = i;
        }
    }
    return best_color;
}

void ClearScreen() {
    gActiveWorld->fill(0xFF);
}

void CopyBits(PixMap* source, PixMap* dest, const Rect& source_rect, const Rect& dest_rect) {
    if (source == dest) {
        return;
    }

    ClippedTransfer transfer(source_rect, dest_rect);
    transfer.ClipSourceTo(source->bounds());
    transfer.ClipDestTo(dest->bounds());

    ViewPixMap clipped_src(source, transfer.from());
    ViewPixMap clipped_dst(dest, transfer.to());
    clipped_dst.copy(clipped_src);
}

Rect ClipRectToRect(const Rect& src, const Rect& clip) {
    return Rect(
        std::max(src.left, clip.left),
        std::max(src.top, clip.top),
        std::min(src.right, clip.right),
        std::min(src.bottom, clip.bottom));
}

int currentForeColor;
int currentBackColor;

void RGBForeColor(RGBColor* color) {
    currentForeColor = NearestColor(color->red, color->green, color->blue);
}

void RGBBackColor(RGBColor* color) {
    currentBackColor = NearestColor(color->red, color->green, color->blue);
}

void PaintRect(const Rect& rect) {
    ViewPixMap view(gActiveWorld, rect);
    view.fill(currentForeColor);
}

void EraseRect(const Rect& rect) {
    ViewPixMap view(gActiveWorld, rect);
    view.fill(currentBackColor);
}

void FrameRect(const Rect& rect) {
    Rect clipped = ClipRectToRect(rect, gActiveWorld->bounds());
    if (clipped.left == clipped.right || clipped.top == clipped.bottom) {
        return;
    }
    for (int x = clipped.left; x < clipped.right; ++x) {
        if (rect.top == clipped.top) {
            gActiveWorld->set(x, rect.top, currentForeColor);
        }
        if (rect.bottom == clipped.bottom) {
            gActiveWorld->set(x, rect.bottom - 1, currentForeColor);
        }
    }
    for (int y = clipped.top; y < clipped.bottom; ++y) {
        if (rect.left == clipped.left) {
            gActiveWorld->set(rect.left, y, currentForeColor);
        }
        if (rect.right == clipped.right) {
            gActiveWorld->set(rect.right - 1, y, currentForeColor);
        }
    }
}

void MacFrameRect(const Rect& rect) {
    FrameRect(rect);
}

void Index2Color(long index, RGBColor* color) {
    color->red = colors->color(index).red;
    color->green = colors->color(index).green;
    color->blue = colors->color(index).blue;
}

Point currentPen;

void MoveTo(int x, int y) {
    currentPen.h = x;
    currentPen.v = y;
}

bool IsOnScreen(int x, int y) {
    int width = gActiveWorld->bounds().right;
    int height = gActiveWorld->bounds().bottom;
    return 0 <= x && x < width
        && 0 <= y && y < height;
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
                gActiveWorld->set(currentPen.h, i, currentForeColor);
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
                gActiveWorld->set(i, currentPen.v, currentForeColor);
            }
        }
        currentPen.h = h;
    }
}

void GetPen(Point* pen) {
    *pen = currentPen;
}

uint16_t DoubleBits(uint8_t in) {
    uint16_t result = in;
    result <<= 8;
    result |= in;
    return result;
}

void RestoreEntries(const ColorTable& table) {
    for (size_t i = 0; i < table.size(); ++i) {
        gRealWorld->mutable_colors()->set_color(i, table.color(i));
    }
}

void FakeDrawingInit(int width, int height) {
    gRealWorld = new ArrayPixMap(width, height);
    colors.reset(new ColorTable(256));
}

}  // namespace antares

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
    StringBinaryWriter(&contents).write(*gRealWorld);

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

    const Rect& from() const { return _from; }
    const Rect& to() const { return _to; }

  private:
    inline void ClipFirstToSecond(const Rect& rect, const Rect& clip) {
        if (clip.left > rect.left) {
            int diff = clip.left - rect.left;
            _to.left += diff;
            _from.left += diff;
        }
        if (clip.top > rect.top) {
            int diff = clip.top - rect.top;
            _to.top += diff;
            _from.top += diff;
        }
        if (clip.right < rect.right) {
            int diff = clip.right - rect.right;
            _to.right += diff;
            _from.right += diff;
        }
        if (clip.bottom < rect.bottom) {
            int diff = clip.bottom - rect.bottom;
            _to.bottom += diff;
            _from.bottom += diff;
        }
    }

    Rect _from;
    Rect _to;
};

void CopyBits(PixMap* source, PixMap* dest, const Rect& source_rect, const Rect& dest_rect) {
    if (source == dest) {
        return;
    }

    ClippedTransfer transfer(source_rect, dest_rect);
    transfer.ClipSourceTo(source->bounds());
    transfer.ClipDestTo(dest->bounds());

    dest->view(transfer.to()).copy(source->view(transfer.from()));
}

int currentForeColor;
int currentBackColor;

void RGBForeColor(const RgbColor& color) {
    currentForeColor = NearestColor(color.red, color.green, color.blue);
}

void RGBBackColor(const RgbColor& color) {
    currentBackColor = NearestColor(color.red, color.green, color.blue);
}

void DrawLine(PixMap* pix, const Point& from, const Point& to) {
    assert(to.h == from.h || to.v == from.v);  // no diagonal lines yet.
    if (to.h == from.h) {
        int step = 1;
        if (to.v < from.v) {
            step = -1;
        }
        for (int i = from.v; i != to.v; i += step) {
            if (pix->bounds().contains(Point(from.h, i))) {
                pix->set(from.h, i, currentForeColor);
            }
        }
    } else {
        int step = 1;
        if (to.h < from.h) {
            step = -1;
        }
        for (int i = from.h; i != to.h; i += step) {
            if (pix->bounds().contains(Point(i, from.v))) {
                pix->set(i, from.v, currentForeColor);
            }
        }
    }
}

void FrameRect(const Rect& r) {
    DrawLine(gActiveWorld, Point(r.left, r.top), Point(r.left, r.bottom - 1));
    DrawLine(gActiveWorld, Point(r.left, r.bottom - 1), Point(r.right - 1, r.bottom - 1));
    DrawLine(gActiveWorld, Point(r.right - 1, r.bottom - 1), Point(r.right - 1, r.top));
    DrawLine(gActiveWorld, Point(r.right - 1, r.top), Point(r.left, r.top));
}

void MacFrameRect(const Rect& rect) {
    FrameRect(rect);
}

void Index2Color(long index, RgbColor* color) {
    color->red = colors->color(index).red;
    color->green = colors->color(index).green;
    color->blue = colors->color(index).blue;
}

Point currentPen;

void MoveTo(int x, int y) {
    currentPen.h = x;
    currentPen.v = y;
}

void MacLineTo(int h, int v) {
    DrawLine(gActiveWorld, currentPen, Point(h, v));
    MoveTo(h, v);
}

void GetPen(Point* pen) {
    *pen = currentPen;
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

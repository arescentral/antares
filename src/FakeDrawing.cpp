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

#include <fcntl.h>
#include <algorithm>
#include <limits>

#include <sfz/sfz.hpp>
#include "ColorTable.hpp"
#include "Error.hpp"
#include "Fakes.hpp"

using sfz::Bytes;
using sfz::makedirs;
using sfz::open;
using sfz::scoped_ptr;
using sfz::write;

namespace path = sfz::path;

namespace antares {

extern PixMap* gRealWorld;

namespace {

scoped_ptr<ColorTable> colors;

}  // namespace

void DumpTo(const sfz::StringPiece& path) {
    Bytes contents;
    write(&contents, *gRealWorld);

    makedirs(path::dirname(path), 0755);
    int fd = open(path, O_WRONLY | O_CREAT, 0644);
    write(fd, contents.data(), contents.size());
    close(fd);
}

void ScrollRect(PixMap* pix, const Rect& rect, int x, int y, const Rect& clip) {
    check(x == 0 && y == -1, "ScrollRect only supports shifting up by one pixel");
    int rowBytes = pix->row_bytes();
    for (int i = std::min(rect.top - 1, clip.top); i < rect.bottom - 1; ++i) {
        RgbColor* base = pix->mutable_bytes() + i * rowBytes + rect.left;
        memcpy(base, base + rowBytes, rect.width() * sizeof(RgbColor));
    }
}

class ClippedTransfer {
  public:
    ClippedTransfer(const Rect& from, const Rect& to)
            : _from(from),
              _to(to) {
        // Rects must be the same size.
        check(_from.width() == _to.width(), "rects have unequal width");
        check(_from.height() == _to.height(), "rects have unequal height");
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

RgbColor currentForeColor;
RgbColor currentBackColor;

void RGBForeColor(const RgbColor& color) {
    currentForeColor = color;
}

void RGBBackColor(const RgbColor& color) {
    currentBackColor = color;
}

void DrawLine(PixMap* pix, const Point& from, const Point& to) {
    check(to.h == from.h || to.v == from.v, "DrawLine() doesn't do diagonal lines");
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

void FrameRect(PixMap* pix, const Rect& r) {
    DrawLine(pix, Point(r.left, r.top), Point(r.left, r.bottom - 1));
    DrawLine(pix, Point(r.left, r.bottom - 1), Point(r.right - 1, r.bottom - 1));
    DrawLine(pix, Point(r.right - 1, r.bottom - 1), Point(r.right - 1, r.top));
    DrawLine(pix, Point(r.right - 1, r.top), Point(r.left, r.top));
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

void MacLineTo(PixMap* pix, int h, int v) {
    DrawLine(pix, currentPen, Point(h, v));
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

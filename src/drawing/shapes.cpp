// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// Copyright (C) 2008-2012 The Antares Authors
//
// This file is part of Antares, a tactical space combat game.
//
// Antares is free software: you can redistribute it and/or modify it
// under the terms of the Lesser GNU General Public License as published
// by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Antares is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with Antares.  If not, see http://www.gnu.org/licenses/

#include "drawing/shapes.hpp"

#include <algorithm>

#include "drawing/color.hpp"
#include "drawing/pix-map.hpp"
#include "lang/casts.hpp"
#include "math/macros.hpp"
#include "video/driver.hpp"

using sfz::Exception;

namespace antares {

inline void mHBlitz(RgbColor*& mdbyte, long mrunLen, const RgbColor& mcolor, long& mcount) {
    mcount = mrunLen;
    while ( mcount-- > 0)
    {
        *(mdbyte++) =mcolor;
    }
}

inline void mDrawHorizontalRun(
        RgbColor*& dbyte, long xAdvance, long runLen, const RgbColor& color, long drowPlus, long count) {
    count = runLen;
    while ( count-- > 0)
    {
        *dbyte = color;
        dbyte += xAdvance;
    }
    dbyte += drowPlus;
}

inline void mDrawVerticalRun(
        RgbColor*& dbyte, long xAdvance, long runLen, const RgbColor& color, long drowPlus, long count) {
    count = runLen;
    while ( count-- > 0)
    {
        *dbyte = color;
        dbyte += drowPlus;
    }
    dbyte += xAdvance;
}

inline void mCopyHorizontalRun(
        RgbColor*& dbyte, RgbColor*& sbyte, long xAdvance, long runLen, long drowPlus,
        long srowPlus, long count) {
    count = runLen;
    while ( count-- > 0)
    {
        *dbyte = *sbyte;
        dbyte += xAdvance;
        sbyte += xAdvance;
    }
    dbyte += drowPlus;
    sbyte += srowPlus;
}

inline void mCopyVerticalRun(
        RgbColor*& dbyte, RgbColor*& sbyte, long xAdvance, long runLen, long drowPlus,
        long srowPlus, long count) {
    count = runLen;
    while ( count-- > 0)
    {
        *dbyte = *sbyte;
        dbyte += drowPlus;
        sbyte += srowPlus;
    }
    dbyte += xAdvance;
    sbyte += xAdvance;
}

namespace {

Rect from_origin(const Rect& r) {
    return Rect(0, 0, r.width(), r.height());
}

bool intersects(const Rect& contained, const Rect& container) {
    return ((contained.right > container.left)
            && (contained.left < container.right)
            && (contained.bottom > container.top)
            && (contained.top < container.bottom));
}

void clip_rect(Rect* contained, const Rect& container) {
    contained->left = std::max(contained->left, container.left);
    contained->top = std::max(contained->top, container.top);
    contained->right = std::min(contained->right, container.right);
    contained->bottom = std::min(contained->bottom, container.bottom);
}

}  // namespace

void DrawLine(PixMap* pix, const Point& from, const Point& to, const RgbColor& color) {
    if (to.h != from.h && to.v != from.v) {
        throw Exception("DrawLine() doesn't do diagonal lines");
    }
    if (to.h == from.h) {
        int step = 1;
        if (to.v < from.v) {
            step = -1;
        }
        for (int i = from.v; i != to.v; i += step) {
            if (pix->size().as_rect().contains(Point(from.h, i))) {
                pix->set(from.h, i, color);
            }
        }
    } else {
        int step = 1;
        if (to.h < from.h) {
            step = -1;
        }
        for (int i = from.h; i != to.h; i += step) {
            if (pix->size().as_rect().contains(Point(i, from.v))) {
                pix->set(i, from.v, color);
            }
        }
    }
}

void FrameRect(PixMap* pix, const Rect& r, const RgbColor& color) {
    DrawLine(pix, Point(r.left, r.top), Point(r.left, r.bottom - 1), color);
    DrawLine(pix, Point(r.left, r.bottom - 1), Point(r.right - 1, r.bottom - 1), color);
    DrawLine(pix, Point(r.right - 1, r.bottom - 1), Point(r.right - 1, r.top), color);
    DrawLine(pix, Point(r.right - 1, r.top), Point(r.left, r.top), color);
}

// DrawNateRect: Direct-draws a rectangle
// CLIPS to destPix->bounds().

void DrawNateRect(PixMap* destPix, Rect* destRect, const RgbColor& color) {
    if (!intersects(*destRect, from_origin(destPix->size().as_rect()))) {
        destRect->left = destRect->right = destRect->top = destRect->bottom = 0;
        return;
    }
    clip_rect(destRect, from_origin(destPix->size().as_rect()));

    int32_t width = destRect->right - destRect->left;
    if (width < 0) {
        return;
    }

    destPix->view(*destRect).fill(color);
}

void DrawNateRectVScan(PixMap* pix, Rect bounds, const RgbColor& color, bool invert) {
    clip_rect(&bounds, from_origin(pix->size().as_rect()));
    if (bounds.width() < 0 || bounds.height() < 0) {
        return;
    }

    for (int i = bounds.top; i < bounds.bottom; ++i) {
        RgbColor* bytes = pix->mutable_row(i);
        for (int j = bounds.left; j < bounds.right; ++j) {
            if (implicit_cast<bool>((i ^ j) & 0x1) != invert) {
                bytes[j] = color;
            }
        }
    }
}

void DrawNateRectClipped(
        PixMap* destPix, Rect* destRect, const Rect& clipRect, const RgbColor& color) {
    if (!intersects(*destRect, from_origin(destPix->size().as_rect()))) {
        destRect->left = destRect->right = destRect->top = destRect->bottom = 0;
        return;
    }
    clip_rect(destRect, clipRect);

    DrawNateRect(destPix, destRect, color);
}

// must be square
void DrawNateTriangleUpClipped(PixMap *destPix, const RgbColor& color) {
    long count;

    int32_t trueWidth = destPix->size().width;
    if (trueWidth == 0) {
        destPix->set(0, 0, color);
        return;
    }

    int32_t rightPlus = trueWidth - 1;
    int32_t drowPlus = destPix->row_bytes() - rightPlus;
    RgbColor* dbyte = destPix->mutable_row(0);

    for (int x = 0; x < trueWidth; ++x) {
        *(dbyte++) = color;
    }

    if (rightPlus > 0) {
        dbyte += drowPlus - 1;
        mHBlitz(dbyte, rightPlus + 1, color, count);
        dbyte--;
    }
    drowPlus += 1;
    rightPlus -= 2;
    dbyte += drowPlus;
    drowPlus += 1;

    while (rightPlus >= 0) {
        mHBlitz(dbyte, rightPlus + 1, color, count);
        dbyte--;

        if (rightPlus > 0) {
            dbyte += drowPlus;
            mHBlitz(dbyte, rightPlus + 1, color, count);
            dbyte--;
        }

        drowPlus += 1;
        rightPlus -= 2;
        dbyte += drowPlus;
        drowPlus += 1;
    }
}

void DrawNatePlusClipped(PixMap *destPix, const RgbColor& color) {
    long count;

    int32_t drowPlus = destPix->row_bytes();
    int32_t trueWidth = destPix->size().width - 1;

    if (trueWidth == 0) {
        destPix->set(0, 0, color);
        return;
    }

    if ((trueWidth % 2) == 0) {
        trueWidth--;
    }
    if (trueWidth < 3) {
        int32_t half = (trueWidth >> 1) + 1;

        RgbColor* dbyte = destPix->mutable_row(0) + half;

        for (int x = 1; x < half; ++x) {
            *dbyte = color;
            dbyte += drowPlus;
        }
        dbyte -= half - 1;
        for (int x = 0; x < trueWidth; ++x) {
            *(dbyte++) = color;
        }
        dbyte += drowPlus - trueWidth + half - 1;
        for (int x = 1; x < half; ++x) {
            *dbyte = color;
            dbyte += drowPlus;
        }
    } else {
        int32_t half = (trueWidth >> 1) + 1;

        RgbColor* dbyte = destPix->mutable_row(0) + half - 1;

        for (int x = 2; x < half; ++x) {
            mHBlitz(dbyte, 3, color, count);
            dbyte += drowPlus - 3;
        }
        dbyte -= half - 2;
        for (int x = 0; x < 3; ++x) {
            mHBlitz(dbyte, trueWidth, color, count);
            dbyte += drowPlus - trueWidth;
        }
        dbyte += half - 2;
        for (int x = 2; x < half; ++x) {
            mHBlitz(dbyte, 3, color, count);
            dbyte += drowPlus - 3;
        }
    }
}

void DrawNateDiamondClipped(PixMap *destPix, const RgbColor& color) {
    long count;

    int32_t trueWidth = destPix->size().width - 1;
    if (trueWidth == 0) {
        destPix->set(0, 0, color);
        return;
    }

    int32_t leftEdge = (trueWidth >> 1) + (trueWidth & 1);
    int32_t rightPlus = ((trueWidth >> 1) + 1) - leftEdge;

    RgbColor* dbyte = destPix->mutable_bytes() + leftEdge;
    int32_t drowPlus = destPix->row_bytes() - (rightPlus + 1);
    while (leftEdge > 0) {
        mHBlitz(dbyte, rightPlus + 1, color, count);
        dbyte--;

        dbyte += drowPlus;
        drowPlus -= 2;
        rightPlus += 2;
        leftEdge--;
    }
    dbyte++;
    leftEdge = (trueWidth >> 1) + (trueWidth & 1);
    drowPlus += 4;
    rightPlus -= 2;
    if (trueWidth & 0x1) {
        dbyte++;
        drowPlus += 2;
        rightPlus -= 2;
        leftEdge--;
    }
    while (leftEdge > 0) {
        mHBlitz(dbyte, rightPlus + 1, color, count);
        dbyte--;

        dbyte += drowPlus;
        drowPlus += 2;
        rightPlus -= 2;
        leftEdge--;
    }
}

void DrawNateVBracket(
        PixMap *destPix, const Rect& destRect, const Rect& clipRect,
        const RgbColor& color) {
    destPix->view(
            Rect(destRect.left, destRect.top, destRect.right, destRect.top + 1)).fill(color);
    destPix->view(
            Rect(destRect.left, destRect.bottom - 1, destRect.right, destRect.bottom)).fill(color);

    destPix->set(destRect.left, destRect.top + 1, color);
    destPix->set(destRect.right - 1, destRect.top + 1, color);

    destPix->set(destRect.left, destRect.bottom - 2, color);
    destPix->set(destRect.right - 1, destRect.bottom - 2, color);
}

void draw_vbracket(const Rect& rect, const RgbColor& color) {
    Point ul(rect.left, rect.top);
    Point ur(rect.right - 1, rect.top);
    Point ll(rect.left, rect.bottom - 1);
    Point lr(rect.right - 1, rect.bottom - 1);

    VideoDriver::driver()->draw_line(ul, ur, color);
    VideoDriver::driver()->draw_line(ul, Point(ul.h, ul.v + 1), color);
    VideoDriver::driver()->draw_line(ur, Point(ur.h, ur.v + 1), color);

    VideoDriver::driver()->draw_line(ll, lr, color);
    VideoDriver::driver()->draw_line(ll, Point(ll.h, ll.v - 1), color);
    VideoDriver::driver()->draw_line(lr, Point(lr.h, lr.v - 1), color);
}

void DrawNateShadedRect(
        PixMap *destPix, Rect *destRect, const Rect& clipRect,
        const RgbColor& fillcolor, const RgbColor& lightcolor, const RgbColor& darkcolor) {
    Rect    tRect = *destRect;

    destPix->view(Rect(tRect.left, tRect.top, tRect.left + 1, tRect.bottom)).fill(lightcolor);
    destPix->view(Rect(tRect.left, tRect.top, tRect.right, tRect.top + 1)).fill(lightcolor);

    destPix->view(Rect(tRect.right - 1, tRect.top, tRect.right, tRect.bottom)).fill(darkcolor);
    destPix->view(Rect(tRect.left, tRect.bottom - 1, tRect.right, tRect.bottom)).fill(darkcolor);

    tRect.right--;
    tRect.bottom--;
    tRect.left++;
    tRect.top++;

    DrawNateRectClipped( destPix, &tRect, clipRect, fillcolor);
}

void draw_shaded_rect(
        Rect rect,
        const RgbColor& fill_color, const RgbColor& light_color, const RgbColor& dark_color) {
    rect.right--;
    rect.bottom--;

    VideoDriver::driver()->draw_line(
            Point(rect.left, rect.bottom), Point(rect.left, rect.top), light_color);
    VideoDriver::driver()->draw_line(
        Point(rect.left, rect.top), Point(rect.right, rect.top), light_color);

    VideoDriver::driver()->draw_line(
        Point(rect.right, rect.top), Point(rect.right, rect.bottom), dark_color);
    VideoDriver::driver()->draw_line(
        Point(rect.right, rect.bottom), Point(rect.left, rect.bottom), dark_color);
    rect.left++;
    rect.top++;

    if ((rect.height() > 0) && (rect.width() > 0)) {
        VideoDriver::driver()->fill_rect(rect, fill_color);
    }
}

}  // namespace antares

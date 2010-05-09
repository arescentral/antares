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

#include "PixMap.hpp"

#include <algorithm>
#include "sfz/WriteItem.hpp"
#include "sfz/Exception.hpp"
#include "Quickdraw.h"
#include "Casts.hpp"
#include "ColorTable.hpp"
#include "Error.hpp"
#include "ImageDriver.hpp"

using sfz::Exception;
using sfz::WriteTarget;
using sfz::scoped_array;
using sfz::write;

namespace antares {

PixMap::~PixMap() { }

const RgbColor* PixMap::row(int y) const {
    return bytes() + y * row_bytes();
}

RgbColor* PixMap::mutable_row(int y) {
    return mutable_bytes() + y * row_bytes();
}

const RgbColor& PixMap::get(int x, int y) const {
    return row(y)[x];
}

void PixMap::set(int x, int y, const RgbColor& color) {
    mutable_row(y)[x] = color;
}

void PixMap::fill(const RgbColor& color) {
    if (bounds().height() > 0) {
        for (int x = 0; x < bounds().width(); ++x) {
            set(x, 0, color);
        }
        for (int y = 1; y < bounds().height(); ++y) {
            memcpy(mutable_row(y), row(y - 1), bounds().width() * sizeof(RgbColor));
        }
    }
}

void PixMap::copy(const PixMap& pix) {
    if (bounds().width() != pix.bounds().width()
            || bounds().height() != pix.bounds().height()) {
        throw Exception("Mismatch in PixMap sizes");
    }
    for (int i = 0; i < bounds().height(); ++i) {
        memcpy(mutable_row(i), pix.row(i), bounds().width() * sizeof(RgbColor));
    }
}

void write_to(WriteTarget out, const PixMap& image) {
    double f = image.transition_fraction();
    if (f == 0.0) {
        ImageDriver::driver()->write(out, image);
    } else {
        ArrayPixMap shaded(image.bounds().width(), image.bounds().height());
        double g = 1.0 - f;
        const RgbColor& to = image.transition_to();
        for (int y = 0; y < image.bounds().height(); ++y) {
            const RgbColor* src = image.row(y);
            RgbColor* dst = shaded.mutable_row(y);
            for (int x = 0; x < image.bounds().width(); ++x) {
                dst[x].alpha = 0xFF;
                dst[x].red = to.red * f + src[x].red * g;
                dst[x].green = to.green * f + src[x].green * g;
                dst[x].blue = to.blue * f + src[x].blue * g;
            }
        }
        write(out, shaded);
    }
}

ArrayPixMap::ArrayPixMap(int width, int height)
        : _transition_fraction(0.0),
          _bounds(0, 0, width, height),
          _colors(new ColorTable(256)),
          _bytes(new RgbColor[width * height]) { }

ArrayPixMap::~ArrayPixMap() { }

void ArrayPixMap::resize(const Rect& new_bounds) {
    ArrayPixMap new_pix_map(new_bounds.width(), new_bounds.height());
    Rect transfer = _bounds;
    transfer.clip_to(new_bounds);
    CopyBits(this, &new_pix_map, transfer, transfer);
    _bounds = new_bounds;
    _bytes.swap(&new_pix_map._bytes);
}

const Rect& ArrayPixMap::bounds() const {
    return _bounds;
}

const ColorTable& ArrayPixMap::colors() const {
    return *_colors;
}

int ArrayPixMap::row_bytes() const {
    return _bounds.right;
}

const RgbColor* ArrayPixMap::bytes() const {
    return _bytes.get();
}

RgbColor* ArrayPixMap::mutable_bytes() {
    return _bytes.get();
}

ColorTable* ArrayPixMap::mutable_colors() {
    return _colors.get();
}

double ArrayPixMap::transition_fraction() const {
    return _transition_fraction;
}

void ArrayPixMap::set_transition_fraction(double fraction) {
    _transition_fraction = fraction;
}

const RgbColor& ArrayPixMap::transition_to() const {
    return *_transition_to;
}

void ArrayPixMap::set_transition_to(const RgbColor& color) {
    _transition_to.reset(new RgbColor(color));
}

PixMap::View::View(PixMap* pix, const Rect& bounds)
        : _parent(pix),
          _offset(bounds.left, bounds.top),
          _bounds(0, 0, bounds.width(), bounds.height()) {
    check(pix->bounds().encloses(bounds), "tried to take view (%d, %d, %d, %d) outside of parent "
            "PixMap with bounds (%d, %d, %d, %d)", bounds.left, bounds.top, bounds.right,
            bounds.bottom, pix->bounds().left, pix->bounds().top, pix->bounds().right,
            pix->bounds().bottom);
    _bounds.clip_to(pix->bounds());
}

const Rect& PixMap::View::bounds() const {
    return _bounds;
}

const ColorTable& PixMap::View::colors() const {
    return _parent->colors();
}

int PixMap::View::row_bytes() const {
    return _parent->row_bytes();
}

const RgbColor* PixMap::View::bytes() const {
    return _parent->bytes() + _offset.v * row_bytes() + _offset.h;
}

RgbColor* PixMap::View::mutable_bytes() {
    return _parent->mutable_bytes() + _offset.v * row_bytes() + _offset.h;
}

ColorTable* PixMap::View::mutable_colors() {
    return _parent->mutable_colors();
}

double PixMap::View::transition_fraction() const {
    return _parent->transition_fraction();
}

void PixMap::View::set_transition_fraction(double fraction) {
    _parent->set_transition_fraction(fraction);
}

const RgbColor& PixMap::View::transition_to() const {
    return _parent->transition_to();
}

void PixMap::View::set_transition_to(const RgbColor& color) {
    _parent->set_transition_to(color);
}

PixMap::View PixMap::view(const Rect& bounds) {
    return View(this, bounds);
}

}  // namespace antares

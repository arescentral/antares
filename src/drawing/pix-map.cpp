// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// Copyright (C) 2008-2017 The Antares Authors
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

#include "drawing/pix-map.hpp"

#include <algorithm>
#include <sfz/sfz.hpp>

#include "lang/casts.hpp"

using sfz::Exception;
using sfz::WriteTarget;
using sfz::format;
using sfz::write;

namespace antares {

PixMap::~PixMap() {}

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
    if (size().height > 0) {
        for (int x = 0; x < size().width; ++x) {
            set(x, 0, color);
        }
        for (int y = 1; y < size().height; ++y) {
            memcpy(mutable_row(y), row(y - 1), size().width * sizeof(RgbColor));
        }
    }
}

void PixMap::copy(const PixMap& pix) {
    if (size() != pix.size()) {
        throw Exception("Mismatch in PixMap sizes");
    }
    for (int i = 0; i < size().height; ++i) {
        memcpy(mutable_row(i), pix.row(i), size().width * sizeof(RgbColor));
    }
}

void PixMap::composite(const PixMap& pix) {
    if (size() != pix.size()) {
        throw Exception("Mismatch in PixMap sizes");
    }
    for (int y = 0; y < size().height; ++y) {
        for (int x = 0; x < size().width; ++x) {
            const RgbColor& over  = pix.get(x, y);
            const double    oa    = over.alpha / 255.0;
            const RgbColor& under = get(x, y);
            const double    ua    = under.alpha / 255.0;

            // TODO(sfiera): if we're going to do anything like this in the long run, we should
            // require that alpha be pre-multiplied with the color components.  We should probably
            // also use integral arithmetic.
            double red   = (over.red * oa) + ((under.red * ua) * (1.0 - oa));
            double green = (over.green * oa) + ((under.green * ua) * (1.0 - oa));
            double blue  = (over.blue * oa) + ((under.blue * ua) * (1.0 - oa));
            double alpha = oa + (ua * (1.0 - oa));
            set(x, y, rgba(red / alpha, green / alpha, blue / alpha, alpha * 255));
        }
    }
}

ArrayPixMap::ArrayPixMap(int32_t width, int32_t height)
        : _size(width, height), _bytes(new RgbColor[width * height]) {}

ArrayPixMap::ArrayPixMap(Size size)
        : _size(size), _bytes(new RgbColor[_size.width * _size.height]) {}

ArrayPixMap::~ArrayPixMap() {}

void ArrayPixMap::resize(Size new_size) {
    using sfz::swap;
    using std::min;
    ArrayPixMap new_pix_map(new_size.width, new_size.height);
    Size        min_size(min(size().width, new_size.width), min(size().height, new_size.height));
    Rect        transfer = min_size.as_rect();
    new_pix_map.view(transfer).copy(view(transfer));
    _size = new_size;
    swap(_bytes, new_pix_map._bytes);
}

const Size& ArrayPixMap::size() const {
    return _size;
}

int ArrayPixMap::row_bytes() const {
    return _size.width;
}

const RgbColor* ArrayPixMap::bytes() const {
    return _bytes.get();
}

RgbColor* ArrayPixMap::mutable_bytes() {
    return _bytes.get();
}

void ArrayPixMap::swap(ArrayPixMap& other) {
    using sfz::swap;
    using std::swap;
    swap(_size, other._size);
    swap(_bytes, other._bytes);
}

PixMap::View::View(PixMap* pix, const Rect& bounds)
        : _parent(pix), _offset(bounds.origin()), _size(bounds.size()) {
    Rect pix_bounds(Point(0, 0), pix->size());
    if (!pix_bounds.encloses(bounds)) {
        throw Exception(
                format("tried to take view {0} outside of parent PixMap with bounds {1}", bounds,
                       pix_bounds));
    }
}

const Size& PixMap::View::size() const {
    return _size;
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

PixMap::View PixMap::view(const Rect& bounds) {
    return View(this, bounds);
}

}  // namespace antares

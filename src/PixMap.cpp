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
#include <Quickdraw.h>
#include "ColorTable.hpp"

namespace antares {

const uint8_t* PixMap::row(int y) const {
    return bytes() + y * row_bytes();
}

uint8_t* PixMap::mutable_row(int y) {
    return mutable_bytes() + y * row_bytes();
}

uint8_t PixMap::get(int x, int y) const {
    return row(y)[x];
}

void PixMap::set(int x, int y, uint8_t color) {
    mutable_row(y)[x] = color;
}

void PixMap::fill(uint8_t color) {
    for (int i = 0; i < bounds().height(); ++i) {
        memset(mutable_row(i), color, bounds().width());
    }
}

void PixMap::copy(const PixMap& pix) {
    if (bounds().width() != pix.bounds().width()
            || bounds().height() != pix.bounds().height()) {
        throw PixMapException("Mismatch in PixMap sizes");
    }
    for (int i = 0; i < bounds().height(); ++i) {
        memcpy(mutable_row(i), pix.row(i), bounds().width());
    }
}

ArrayPixMap::ArrayPixMap(int width, int height)
        : _bounds(0, 0, width, height),
          _colors(new ColorTable(256)),
          _bytes(new unsigned char[width * height]) { }

ArrayPixMap::~ArrayPixMap() { }

const Rect& ArrayPixMap::bounds() const {
    return _bounds;
}

const ColorTable& ArrayPixMap::colors() const {
    return *_colors;
}

int ArrayPixMap::row_bytes() const {
    return _bounds.right;
}

const uint8_t* ArrayPixMap::bytes() const {
    return _bytes.get();
}

uint8_t* ArrayPixMap::mutable_bytes() {
    return _bytes.get();
}

ColorTable* ArrayPixMap::mutable_colors() {
    return _colors.get();
}

void ArrayPixMap::fill(uint8_t color) {
    memset(_bytes.get(), color, _bounds.width() * _bounds.height());
}

void ArrayPixMap::resize(const Rect& new_bounds) {
    ArrayPixMap new_pix_map(new_bounds.width(), new_bounds.height());
    Rect transfer = _bounds;
    transfer.clip_to(new_bounds);
    CopyBits(this, &new_pix_map, transfer, transfer);
    _bounds = new_bounds;
    _bytes.swap(&new_pix_map._bytes);
}

ViewPixMap::ViewPixMap(PixMap* pix, const Rect& r)
        : _parent(pix),
          _offset(r.left, r.top),
          _bounds(0, 0, r.width(), r.height()) {
    _bounds.clip_to(pix->bounds());
}

const Rect& ViewPixMap::bounds() const {
    return _bounds;
}

const ColorTable& ViewPixMap::colors() const {
    return _parent->colors();
}

int ViewPixMap::row_bytes() const {
    return _parent->row_bytes();
}

const uint8_t* ViewPixMap::bytes() const {
    return _parent->bytes() + _offset.v * row_bytes() + _offset.h;
}

uint8_t* ViewPixMap::mutable_bytes() {
    return _parent->mutable_bytes() + _offset.v * row_bytes() + _offset.h;
}

ColorTable* ViewPixMap::mutable_colors() {
    return _parent->mutable_colors();
}

}  // namespace antares

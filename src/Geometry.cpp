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

#include "Geometry.hpp"

#include "BinaryStream.hpp"

namespace antares {

Point::Point()
        : h(0),
          v(0) { }

Point::Point(int x, int y)
        : h(x),
          v(y) { }

void Point::read(BinaryReader* bin) {
    bin->read(&h);
    bin->read(&v);
}

Rect::Rect()
        : left(0),
          top(0),
          right(0),
          bottom(0) { }

Rect::Rect(int32_t left, int32_t top, int32_t right, int32_t bottom)
        : left(left),
          top(top),
          right(right),
          bottom(bottom) { }

bool Rect::contains(const Point& p) const {
    return left <= p.h && p.h < right
        && top <= p.v && p.v < bottom;
}

int32_t Rect::width() const {
    return right - left;
}

int32_t Rect::height() const {
    return bottom - top;
}

void Rect::offset(int32_t x, int32_t y) {
    left += x;
    right += x;
    top += y;
    bottom += y;
}

void Rect::inset(int32_t x, int32_t y) {
    left += x;
    right -= x;
    top += y;
    bottom -= y;
}

void Rect::center_in(const Rect& r) {
    int32_t offset_x = (r.left / 2 - left / 2) + (r.right / 2 - right / 2);
    int32_t offset_y = (r.top / 2 - top / 2) + (r.bottom / 2 - bottom / 2);
    offset(offset_x, offset_y);
}

void Rect::read(BinaryReader* bin) {
    bin->read(&left);
    bin->read(&top);
    bin->read(&right);
    bin->read(&bottom);
}

}  // namespace antares

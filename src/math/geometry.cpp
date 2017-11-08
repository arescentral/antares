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

#include "math/geometry.hpp"

#include <algorithm>
#include <sfz/sfz.hpp>

using sfz::ReadSource;
using sfz::format;
using sfz::read;

namespace antares {

Point::Point() : h(0), v(0) {}

Point::Point(int x, int y) : h(x), v(y) {}

void Point::offset(int32_t x, int32_t y) {
    h += x;
    v += y;
}

void Point::clamp_to(const Rect& rect) {
    h = std::max(h, rect.left);
    v = std::max(v, rect.top);
    h = std::min(h, rect.right - 1);
    v = std::min(v, rect.bottom - 1);
}

bool operator==(const Point& lhs, const Point& rhs) {
    return (lhs.h == rhs.h) && (lhs.v == rhs.v);
}

bool operator!=(const Point& lhs, const Point& rhs) {
    return !(lhs == rhs);
}

void read_from(ReadSource in, Point& p) {
    read(in, p.h);
    read(in, p.v);
}

Size::Size() : width(0), height(0) {}

Size::Size(int32_t width, int32_t height) : width(width), height(height) {}

Rect Size::as_rect() const {
    return Rect(0, 0, width, height);
}

bool operator==(Size x, Size y) {
    return (x.width == y.width) && (x.height == y.height);
}

bool operator!=(Size x, Size y) {
    return !(x == y);
}

Rect::Rect() : left(0), top(0), right(0), bottom(0) {}

Rect::Rect(int32_t left, int32_t top, int32_t right, int32_t bottom)
        : left(left), top(top), right(right), bottom(bottom) {}

Rect::Rect(Point origin, Size size)
        : left(origin.h), top(origin.v), right(left + size.width), bottom(top + size.height) {}

bool Rect::empty() const {
    return (width() <= 0) || (height() <= 0);
}

int32_t Rect::width() const {
    return right - left;
}

int32_t Rect::height() const {
    return bottom - top;
}

Point Rect::origin() const {
    return Point(left, top);
}

Point Rect::center() const {
    return Point((left + right) / 2, (top + bottom) / 2);
}

Size Rect::size() const {
    return Size(width(), height());
}

int32_t Rect::area() const {
    return width() * height();
}

bool Rect::contains(const Point& p) const {
    return left <= p.h && p.h < right && top <= p.v && p.v < bottom;
}

bool Rect::encloses(const Rect& r) const {
    return left <= r.left && r.right <= right && top <= r.top && r.bottom <= bottom;
}

bool Rect::intersects(const Rect& r) const {
    return left < r.right && r.left < right && top < r.bottom && r.top < bottom;
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
    int32_t offset_x = (r.left - left + r.right - right) / 2;
    int32_t offset_y = (r.top - top + r.bottom - bottom) / 2;
    offset(offset_x, offset_y);
}

void Rect::clip_to(const Rect& r) {
    left   = std::max(left, r.left);
    top    = std::max(top, r.top);
    right  = std::min(right, r.right);
    bottom = std::min(bottom, r.bottom);
}

void Rect::enlarge_to(const Rect& r) {
    left   = std::min(left, r.left);
    top    = std::min(top, r.top);
    right  = std::max(right, r.right);
    bottom = std::max(bottom, r.bottom);
}

void read_from(ReadSource in, Rect& r) {
    read(in, r.left);
    read(in, r.top);
    read(in, r.right);
    read(in, r.bottom);
}

void print_to(sfz::PrintTarget out, Rect r) {
    print(out, format("{{{0}, {1}, {2}, {3}}}", r.left, r.top, r.right, r.bottom));
}

}  // namespace antares

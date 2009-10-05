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

#ifndef ANTARES_GEOMETRY_HPP_
#define ANTARES_GEOMETRY_HPP_

#include <stdint.h>

class BinaryReader;

struct Point {
    int32_t h;
    int32_t v;

    Point();
    Point(int x, int y);
};

struct Rect {
    int32_t left;
    int32_t top;
    int32_t right;
    int32_t bottom;

    Rect();
    Rect(int32_t left, int32_t top, int32_t right, int32_t bottom);

    bool contains(const Point& p) const;
    int32_t width() const;
    int32_t height() const;

    void offset(int32_t x, int32_t y);
    void inset(int32_t x, int32_t y);
    void center_in(const Rect& r);

    void read(BinaryReader* bin);
};

#endif // ANTARES_GEOMETRY_HPP_

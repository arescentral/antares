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

#ifndef ANTARES_PIX_MAP_HPP_
#define ANTARES_PIX_MAP_HPP_

#include "Geometry.hpp"
#include "SmartPtr.hpp"

namespace antares {

class ColorTable;

struct PixMap {
    PixMap(int32_t width, int32_t height);
    ~PixMap();

    void resize(const Rect& r);

    void set(int x, int y, uint8_t color);
    uint8_t get(int x, int y) const;

    Rect bounds;
    ColorTable* colors;
    long rowBytes;
    uint8_t* baseAddr;
    int pixelSize;

    DISALLOW_COPY_AND_ASSIGN(PixMap);
};

}  // namespace antares

#endif  // ANTARES_PIX_MAP_HPP_

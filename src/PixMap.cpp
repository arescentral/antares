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

PixMap::PixMap(int width, int height)
        : bounds(0, 0, width, height),
          colors(new ColorTable(256)),
          rowBytes(width),
          baseAddr(new unsigned char[width * height]),
          pixelSize(1) { }

PixMap::~PixMap() {
    delete[] baseAddr;
}

void PixMap::resize(const Rect& new_bounds) {
    PixMap new_pix_map(new_bounds.width(), new_bounds.height());
    Rect transfer = bounds;
    transfer.clip_to(new_bounds);
    CopyBits(this, &new_pix_map, transfer, transfer);
    bounds = new_bounds;
    std::swap(baseAddr, new_pix_map.baseAddr);
}

void PixMap::set(int x, int y, uint8_t color) {
    int row_bytes = rowBytes;
    baseAddr[y * row_bytes + x] = color;
}

uint8_t PixMap::get(int x, int y) const {
    int row_bytes = rowBytes;
    return baseAddr[y * row_bytes + x];
}

}  // namespace antares

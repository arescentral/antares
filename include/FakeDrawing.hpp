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

#ifndef ANTARES_FAKE_DRAWING_HPP_
#define ANTARES_FAKE_DRAWING_HPP_

#include <stdint.h>
#include <Quickdraw.h>

#include "Fakes.hpp"

struct Color24Bit {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
};

class FakePixMap : public PixMap {
  public:
    FakePixMap(int width, int height) {
        bounds.left = 0;
        bounds.top = 0;
        bounds.right = width;
        bounds.bottom = height;
        pmTable = NULL;
        rowBytes = width | 0x8000;
        baseAddr = new char[width * height];
        pixelSize = 1;
    }

    ~FakePixMap() {
        delete[] baseAddr;
    }

    int width() const { return bounds.right - bounds.left; }
    int height() const { return bounds.bottom - bounds.top; }

    char& PixelAt(int x, int y) { return baseAddr[(y * rowBytes) + x]; }

  private:
    DISALLOW_COPY_AND_ASSIGN(FakePixMap);
};

uint8_t NearestColor(uint16_t red, uint16_t green, uint16_t blue);
uint8_t GetPixel(int x, int y);
void SetPixel(int x, int y, uint8_t c);
void SetPixelRow(int x, int y, uint8_t* c, int count);

class GWorld {
  public:
    GWorld(int width, int height)
            : pixMap(width, height),
              pixMapPtr(&pixMap) { }

    FakePixMap pixMap;
    PixMap* pixMapPtr;

  private:
    DISALLOW_COPY_AND_ASSIGN(GWorld);
};

extern GWorld* gOffWorld;
extern GWorld* gRealWorld;
extern GWorld* gSaveWorld;

Pic** GetPicture(int id);

void FakeDrawingInit();

#endif  // ANTARES_FAKE_DRAWING_HPP_

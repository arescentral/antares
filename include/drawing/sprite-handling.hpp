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

#ifndef ANTARES_DRAWING_SPRITE_HANDLING_HPP_
#define ANTARES_DRAWING_SPRITE_HANDLING_HPP_

#include "data/handle.hpp"
#include "drawing/color.hpp"
#include "drawing/pix-table.hpp"
#include "math/fixed.hpp"

namespace antares {

const int32_t kNoSpriteTable = -1;

const int16_t kSpriteTableColorShift  = 11;
const int16_t kSpriteTableColorIDMask = 0x7800;  // bits 11-14
// this makes the max legal sprite id 2047

enum {
    kNoSpriteLayer     = 0,
    kFirstSpriteLayer  = 1,
    kMiddleSpriteLayer = 2,
    kLastSpriteLayer   = 3,
};

const int32_t kSpriteMaxSize  = 2048;
const int32_t kBlipThreshhold = kOneQuarterScale;

const size_t MAX_PIX_SIZE = 480;

const size_t  kMaxPixTableEntry = 60;
const int32_t kNoSprite         = -1;

enum spriteStyleType { spriteNormal = 0, spriteColor = 2 };

typedef void (*draw_tiny_t)(const Rect& rect, const RgbColor& color);

class Sprite {
  public:
    static Sprite* get(int number);
    static Handle<Sprite>     none() { return Handle<Sprite>(-1); }
    static HandleList<Sprite> all() { return HandleList<Sprite>(0, size); }

    Sprite();

    Point           where;
    NatePixTable*   table;
    int16_t         resID;
    int             whichShape;
    int32_t         scale;
    spriteStyleType style;
    RgbColor        styleColor;
    int16_t         styleData;
    int32_t         tinySize;
    int16_t         whichLayer;
    RgbColor        tinyColor;
    bool            killMe;
    draw_tiny_t     draw_tiny;

  private:
    friend void         SpriteHandlingInit();
    static const size_t size = 500;
};

extern int32_t gAbsoluteScale;

// Scale `value` by `scale`.
//
// The regular variant calculates the final scale as ``(value * scale) / 4096``.  The evil variant
// calculates the final scale as ``(value * scale) >> 12``, which results in off-by-one errors when
// `value` is negative.
Fixed scale_by(Fixed value, int32_t scale);
Fixed evil_scale_by(Fixed value, int32_t scale);
int32_t evil_scale_by(int32_t value, int32_t scale);

class Pix {
  public:
    void          reset();
    NatePixTable* add(int16_t id);
    NatePixTable* get(int16_t id);

  private:
    std::map<int16_t, NatePixTable> pix;
};

void SpriteHandlingInit();
void ResetAllSprites();
Rect scale_sprite_rect(const NatePixTable::Frame& frame, Point where, int32_t scale);
Handle<Sprite> AddSprite(
        Point where, NatePixTable* table, int16_t resID, int16_t whichShape, int32_t scale,
        int32_t size, int16_t layer, const RgbColor& color);
void RemoveSprite(Handle<Sprite> sprite);
void draw_sprites();
void CullSprites();

}  // namespace antares

#endif  // ANTARES_DRAWING_SPRITE_HANDLING_HPP_

// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// Copyright (C) 2008-2012 The Antares Authors
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

#include "drawing/color.hpp"
#include "drawing/pix-table.hpp"

namespace antares {

const int32_t kNoSpriteTable = -1;

const int16_t kSpriteTableColorShift = 11;
const int16_t kSpriteTableColorIDMask = 0x7800;  // bits 11-14
// this makes the max legal sprite id 2047

const int32_t SCALE_SCALE   = 4096;
const int32_t MIN_SCALE     = 256;
const int32_t MAX_SCALE     = 32768;
const int32_t MAX_SCALE_PIX = 32;  // the maximum size a single scaled pixel can be
                                   // (should be 32)

const int32_t kOneEighthScale   = SCALE_SCALE / 8;
const int32_t kOneQuarterScale  = SCALE_SCALE / 4;
const int32_t kOneHalfScale     = SCALE_SCALE / 2;
const int32_t kTimesTwoScale    = SCALE_SCALE * 2;

const int32_t SHIFT_SCALE       = 12;

enum {
    kNoSpriteLayer      = 0,
    kFirstSpriteLayer   = 1,
    kMiddleSpriteLayer  = 2,
    kLastSpriteLayer    = 3,
};

const int32_t kSpriteMaxSize = 2048;
const int32_t kBlipThreshhold = kOneQuarterScale;

const size_t MAX_PIX_SIZE = 480;

const size_t kMaxPixTableEntry = 60;
const int32_t kNoSprite = -1;

enum spriteStyleType {
    spriteNormal = 0,
    spriteColor = 2
};

typedef void (*draw_tiny_t)(const Rect& rect, const RgbColor& color);

struct spriteType {
    Point           where;
    NatePixTable*   table;
    short           resID;
    int             whichShape;
    int32_t         scale;
    spriteStyleType style;
    RgbColor        styleColor;
    short           styleData;
    long            tinySize;
    short           whichLayer;
    RgbColor        tinyColor;
    bool            killMe;
    draw_tiny_t     draw_tiny;

    spriteType();
};

extern int32_t gAbsoluteScale;

// Scale `value` by `scale`.
//
// The regular variant calculates the final scale as ``(value * scale) / 4096``.  The evil variant
// calculates the final scale as ``(value * scale) >> 12``, which results in off-by-one errors when
// `value` is negative.
int32_t scale_by(int32_t value, int32_t scale);
int32_t evil_scale_by(int32_t value, int32_t scale);

void SpriteHandlingInit();
void ResetAllSprites();
Rect scale_sprite_rect(const NatePixTable::Frame& frame, Point where, int32_t scale);
void ResetAllPixTables();
void SetAllPixTablesNoKeep();
void KeepPixTable(int16_t resource_id);
void RemoveAllUnusedPixTables();
NatePixTable* AddPixTable(int16_t resource_id);
NatePixTable* GetPixTable(int16_t resource_id);
spriteType *AddSprite(
        Point where, NatePixTable* table, short resID, short whichShape, int32_t scale, long size,
        short layer, const RgbColor& color, long *whichSprite);
void RemoveSprite(spriteType *);
void draw_sprites();
void CullSprites();

}  // namespace antares

#endif // ANTARES_DRAWING_SPRITE_HANDLING_HPP_

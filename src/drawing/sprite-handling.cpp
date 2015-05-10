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

#include "drawing/sprite-handling.hpp"

#include <numeric>

#include "drawing/color.hpp"
#include "drawing/pix-table.hpp"
#include "drawing/shapes.hpp"
#include "drawing/text.hpp"
#include "game/globals.hpp"
#include "lang/defines.hpp"
#include "math/random.hpp"
#include "math/rotation.hpp"
#include "video/driver.hpp"

using sfz::Exception;
using sfz::Range;
using sfz::StringSlice;
using sfz::format;
using sfz::range;
using std::map;
using std::unique_ptr;

namespace antares {

namespace {

const size_t kMaxSpriteNum = 500;

const size_t kMinVolatilePixTable = 1;  // sound 0 is always there; 1+ is volatile

const uint32_t kSolidSquareBlip     = 0x00000000;
const uint32_t kTriangleUpBlip      = 0x00000010;
const uint32_t kDiamondBlip         = 0x00000020;
const uint32_t kPlusBlip            = 0x00000030;
const uint32_t kFramedSquareBlip    = 0x00000040;

const uint32_t kBlipSizeMask        = 0x0000000f;
const uint32_t kBlipTypeMask        = 0x000000f0;

template <typename T>
void zero(T* t) {
    *t = T();
}

template <typename T>
Range<T*> slice(T* array, size_t start, size_t end) {
    return Range<T*>(array + start, array + end);
}

static void draw_tiny_square(const Rect& rect, const RgbColor& color) {
    Rects().fill(rect, color);
}

static void draw_tiny_triangle(const Rect& rect, const RgbColor& color) {
    VideoDriver::driver()->draw_triangle(rect, color);
}

static void draw_tiny_diamond(const Rect& rect, const RgbColor& color) {
    VideoDriver::driver()->draw_diamond(rect, color);
}

static void draw_tiny_plus(const Rect& rect, const RgbColor& color) {
    VideoDriver::driver()->draw_plus(rect, color);
}

draw_tiny_t draw_tiny_function(uint8_t id) {
    uint8_t size = id & kBlipSizeMask;
    uint8_t type = id & kBlipTypeMask;
    if (size <= 0) {
        return NULL;
    }
    switch (type) {
        case kTriangleUpBlip: return draw_tiny_triangle;
        case kFramedSquareBlip:
        case kSolidSquareBlip: return draw_tiny_square;
        case kPlusBlip: return draw_tiny_plus;
        case kDiamondBlip: return draw_tiny_diamond;
        default: return NULL;
    }
}

}  // namespace

struct pixTableType {
    std::unique_ptr<NatePixTable>   resource;
    int                             resID;
    bool                            keepMe;
};
static ANTARES_GLOBAL pixTableType gPixTable[kMaxPixTableEntry];

int32_t ANTARES_GLOBAL gAbsoluteScale = MIN_SCALE;

void SpriteHandlingInit() {
    ResetAllPixTables();

    g.sprites.reset(new spriteType[kMaxSpriteNum]);
    ResetAllSprites();

    for (int i = 0; i < 4000; ++i) {
        Randomize(256);
    }
}

spriteType::spriteType()
        : table(NULL),
          resID(-1),
          style(spriteNormal),
          styleColor(RgbColor::kWhite),
          styleData(0),
          whichLayer(kNoSpriteLayer),
          killMe(false),
          draw_tiny(NULL) { }

void ResetAllSprites() {
    for (int i: range(kMaxSpriteNum)) {
        zero(&g.sprites[i]);
    }
}

void ResetAllPixTables() {
    for (pixTableType* entry: range(gPixTable, gPixTable + kMaxPixTableEntry)) {
        entry->resource.reset();
        entry->keepMe = false;
        entry->resID = -1;
    }
}

void SetAllPixTablesNoKeep() {
    for (pixTableType* entry:
            range(gPixTable + kMinVolatilePixTable, gPixTable + kMaxPixTableEntry)) {
        entry->keepMe = false;
    }
}

void KeepPixTable(int16_t resID) {
    for (pixTableType* entry: range(gPixTable, gPixTable + kMaxPixTableEntry)) {
        if (entry->resID == resID) {
            entry->keepMe = true;
            return;
        }
    }
}

void RemoveAllUnusedPixTables() {
    for (pixTableType* entry:
            range(gPixTable + kMinVolatilePixTable, gPixTable + kMaxPixTableEntry)) {
        if (!entry->keepMe) {
            entry->resource.reset();
            entry->resID = -1;
        }
    }
}

NatePixTable* AddPixTable(int16_t resource_id) {
    NatePixTable* result = GetPixTable(resource_id);
    if (result != NULL) {
        return result;
    }

    int16_t real_resource_id = resource_id & ~kSpriteTableColorIDMask;
    int16_t color = (resource_id & kSpriteTableColorIDMask) >> kSpriteTableColorShift;
    for (pixTableType* entry: range(gPixTable, gPixTable + kMaxPixTableEntry)) {
        if (entry->resource.get() == NULL) {
            entry->resID = resource_id;
            entry->resource.reset(new NatePixTable(real_resource_id, color));
            return entry->resource.get();
        }
    }

    throw Exception("Can't manage any more sprite tables");
}

NatePixTable* GetPixTable(int16_t resource_id) {
    for (pixTableType* entry: range(gPixTable, gPixTable + kMaxPixTableEntry)) {
        if (entry->resID == resource_id) {
            return entry->resource.get();
        }
    }
    return NULL;
}

spriteType *AddSprite(
        Point where, NatePixTable* table, int16_t resID, int16_t whichShape, int32_t scale, int32_t size,
        int16_t layer, const RgbColor& color) {
    for (spriteType* sprite: range(g.sprites.get(), g.sprites.get() + kMaxSpriteNum)) {
        if (sprite->table == NULL) {
            sprite->where = where;
            sprite->table = table;
            sprite->resID = resID;
            sprite->whichShape = whichShape;
            sprite->scale = scale;
            sprite->whichLayer = layer;
            sprite->tinySize = size;
            sprite->tinyColor = color;
            sprite->draw_tiny = draw_tiny_function(size);
            sprite->killMe = false;
            sprite->style = spriteNormal;
            sprite->styleColor = RgbColor::kWhite;
            sprite->styleData = 0;

            return sprite;
        }
    }

    return NULL;
}

void RemoveSprite(spriteType *aSprite) {
    aSprite->killMe = false;
    aSprite->table = NULL;
    aSprite->resID = -1;
}

int32_t scale_by(int32_t value, int32_t scale) {
    return (value * scale) / SCALE_SCALE;
}

int32_t evil_scale_by(int32_t value, int32_t scale) {
    return (value * scale) >> SHIFT_SCALE;
}

Rect scale_sprite_rect(const NatePixTable::Frame& frame, Point where, int32_t scale) {
    Rect draw_rect = Rect(
            0, 0, evil_scale_by(frame.width(), scale), evil_scale_by(frame.height(), scale));
    draw_rect.offset(
            where.h - evil_scale_by(frame.center().h, scale),
            where.v - evil_scale_by(frame.center().v, scale));
    return draw_rect;
}

void draw_sprites() {
    if (gAbsoluteScale >= kBlipThreshhold) {
        for (int layer: range<int>(kFirstSpriteLayer, kLastSpriteLayer + 1)) {
            for (int i: range(kMaxSpriteNum)) {
                spriteType* aSprite = &g.sprites[i];
                if ((aSprite->table != NULL)
                        && !aSprite->killMe
                        && (aSprite->whichLayer == layer)) {
                    int32_t trueScale = evil_scale_by(aSprite->scale, gAbsoluteScale);
                    const NatePixTable::Frame& frame = aSprite->table->at(aSprite->whichShape);

                    const int32_t map_width = evil_scale_by(frame.width(), trueScale);
                    const int32_t map_height = evil_scale_by(frame.height(), trueScale);
                    const int32_t scaled_h = evil_scale_by(frame.center().h, trueScale);
                    const int32_t scaled_v = evil_scale_by(frame.center().v, trueScale);
                    const Point scaled_center(scaled_h, scaled_v);

                    Rect draw_rect(0, 0, map_width, map_height);
                    draw_rect.offset(aSprite->where.h - scaled_h, aSprite->where.v - scaled_v);

                    switch (aSprite->style) {
                      case spriteNormal:
                        frame.sprite().draw(draw_rect);
                        break;

                      case spriteColor:
                        Randomize(63);
                        frame.sprite().draw_static(
                                draw_rect, aSprite->styleColor, aSprite->styleData);
                        break;
                    }
                }
            }
        }
    } else {
        for (int layer: range<int>(kFirstSpriteLayer, kLastSpriteLayer + 1)) {
            for (int i: range(kMaxSpriteNum)) {
                spriteType* aSprite = &g.sprites[i];
                int tinySize = aSprite->tinySize & kBlipSizeMask;
                if ((aSprite->table != NULL)
                        && !aSprite->killMe
                        && tinySize
                        && (aSprite->draw_tiny != NULL)
                        && (aSprite->whichLayer == layer)) {
                    Rect tiny_rect(-tinySize, -tinySize, tinySize, tinySize);
                    tiny_rect.offset(aSprite->where.h, aSprite->where.v);
                    aSprite->draw_tiny(tiny_rect, aSprite->tinyColor);
                }
            }
        }
    }
}

// CullSprites: if you're keeping track of sprites, but not showing them, use this to remove
// dead sprites. (Implemented for Asteroid level, where game is run to populate scenario with
// Asteroids before the player actually starts.

void CullSprites() {
    for (int i: range(kMaxSpriteNum)) {
        spriteType* aSprite = &g.sprites[i];
        if (aSprite->table != NULL) {
            if (aSprite->killMe) {
                RemoveSprite(aSprite);
            }
        }
    }
}

}  // namespace antares

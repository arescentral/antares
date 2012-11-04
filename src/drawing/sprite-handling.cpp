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
#include "math/random.hpp"
#include "math/rotation.hpp"
#include "video/driver.hpp"

using sfz::Exception;
using sfz::Range;
using sfz::StringSlice;
using sfz::format;
using sfz::range;
using sfz::scoped_array;
using sfz::scoped_ptr;
using std::map;

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

map<uint8_t, Sprite*> tiny_sprites;

template <typename T>
void zero(T* t) {
    *t = T();
}

template <typename T>
Range<T*> slice(T* array, size_t start, size_t end) {
    return Range<T*>(array + start, array + end);
}

Sprite* make_tiny_sprite(uint8_t id) {
    uint8_t size = id & kBlipSizeMask;
    uint8_t type = id & kBlipTypeMask;
    if (size <= 0) {
        return NULL;
    }
    if (tiny_sprites.find(id) == tiny_sprites.end()) {
        ArrayPixMap pix(size * 2, size * 2);
        pix.fill(RgbColor::kClear);
        StringSlice shape_name;
        switch (type) {
          case kTriangleUpBlip:
            DrawNateTriangleUpClipped(&pix, RgbColor::kWhite);
            shape_name = "triangle";
            break;

          case kFramedSquareBlip:
          case kSolidSquareBlip:
            pix.fill(RgbColor::kWhite);
            shape_name = "square";
            break;

          case kPlusBlip:
            DrawNatePlusClipped(&pix, RgbColor::kWhite);
            shape_name = "plus";
            break;

          case kDiamondBlip:
            DrawNateDiamondClipped(&pix, RgbColor::kWhite);
            shape_name = "diamond";
            break;

          default:
            return NULL;
        }
        tiny_sprites[id] = VideoDriver::driver()->new_sprite(
                format("/x/{0}/{1}", shape_name, size),
                pix);
    }
    return tiny_sprites[id];
}

}  // namespace

struct pixTableType {
    sfz::scoped_ptr<NatePixTable>   resource;
    int                             resID;
    bool                            keepMe;
};
pixTableType gPixTable[kMaxPixTableEntry];

int32_t gAbsoluteScale = MIN_SCALE;
scoped_array<spriteType> gSpriteTable;
const RgbColor& kNoTinyColor = RgbColor::kBlack;

void SpriteHandlingInit() {
    ResetAllPixTables();

    gSpriteTable.reset(new spriteType[kMaxSpriteNum]);
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
          tiny_sprite(NULL) { }

void ResetAllSprites() {
    SFZ_FOREACH(int i, range(kMaxSpriteNum), {
        zero(&gSpriteTable[i]);
    });
}

void ResetAllPixTables() {
    SFZ_FOREACH(pixTableType* entry, range(gPixTable, gPixTable + kMaxPixTableEntry), {
        entry->resource.reset();
        entry->keepMe = false;
        entry->resID = -1;
    });
}

void SetAllPixTablesNoKeep() {
    SFZ_FOREACH(
            pixTableType* entry,
            range(gPixTable + kMinVolatilePixTable, gPixTable + kMaxPixTableEntry), {
        entry->keepMe = false;
    });
}

void KeepPixTable(short resID) {
    SFZ_FOREACH(pixTableType* entry, range(gPixTable, gPixTable + kMaxPixTableEntry), {
        if (entry->resID == resID) {
            entry->keepMe = true;
            return;
        }
    });
}

void RemoveAllUnusedPixTables() {
    SFZ_FOREACH(
            pixTableType* entry,
            range(gPixTable + kMinVolatilePixTable, gPixTable + kMaxPixTableEntry), {
        if (!entry->keepMe) {
            entry->resource.reset();
            entry->resID = -1;
        }
    });
}

NatePixTable* AddPixTable(int16_t resource_id) {
    NatePixTable* result = GetPixTable(resource_id);
    if (result != NULL) {
        return result;
    }

    int16_t real_resource_id = resource_id & ~kSpriteTableColorIDMask;
    int16_t color = (resource_id & kSpriteTableColorIDMask) >> kSpriteTableColorShift;
    SFZ_FOREACH(pixTableType* entry, range(gPixTable, gPixTable + kMaxPixTableEntry), {
        if (entry->resource.get() == NULL) {
            entry->resID = resource_id;
            entry->resource.reset(new NatePixTable(real_resource_id, color));
            return entry->resource.get();
        }
    });

    throw Exception("Can't manage any more sprite tables");
}

NatePixTable* GetPixTable(int16_t resource_id) {
    SFZ_FOREACH(pixTableType* entry, range(gPixTable, gPixTable + kMaxPixTableEntry), {
        if (entry->resID == resource_id) {
            return entry->resource.get();
        }
    });
    return NULL;
}

spriteType *AddSprite(
        Point where, NatePixTable* table, short resID, short whichShape, int32_t scale, long size,
        short layer, const RgbColor& color, long *whichSprite) {
    SFZ_FOREACH(
            spriteType* sprite, range(gSpriteTable.get(), gSpriteTable.get() + kMaxSpriteNum), {
        if (sprite->table == NULL) {
            *whichSprite = sprite - gSpriteTable.get();

            sprite->where = where;
            sprite->table = table;
            sprite->resID = resID;
            sprite->whichShape = whichShape;
            sprite->scale = scale;
            sprite->whichLayer = layer;
            sprite->tinySize = size;
            sprite->tinyColor = color;
            sprite->tiny_sprite = make_tiny_sprite(size);
            sprite->killMe = false;
            sprite->style = spriteNormal;
            sprite->styleColor = RgbColor::kWhite;
            sprite->styleData = 0;

            return sprite;
        }
    });

    *whichSprite = kNoSprite;
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
        SFZ_FOREACH(int layer, range<int>(kFirstSpriteLayer, kLastSpriteLayer + 1), {
            SFZ_FOREACH(int i, range(kMaxSpriteNum), {
                spriteType* aSprite = &gSpriteTable[i];
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
            });
        });
    } else {
        SFZ_FOREACH(int layer, range<int>(kFirstSpriteLayer, kLastSpriteLayer + 1), {
            SFZ_FOREACH(int i, range(kMaxSpriteNum), {
                spriteType* aSprite = &gSpriteTable[i];
                int tinySize = aSprite->tinySize & kBlipSizeMask;
                if ((aSprite->table != NULL)
                        && !aSprite->killMe
                        && (aSprite->tinyColor != kNoTinyColor)
                        && tinySize
                        && (aSprite->tiny_sprite != NULL)
                        && (aSprite->whichLayer == layer)) {
                    aSprite->tiny_sprite->draw_shaded(
                        aSprite->where.h - tinySize, aSprite->where.v - tinySize,
                        aSprite->tinyColor);
                }
            });
        });
    }
}

// CullSprites: if you're keeping track of sprites, but not showing them, use this to remove
// dead sprites. (Implemented for Asteroid level, where game is run to populate scenario with
// Asteroids before the player actually starts.

void CullSprites() {
    SFZ_FOREACH(int i, range(kMaxSpriteNum), {
        spriteType* aSprite = &gSpriteTable[i];
        if (aSprite->table != NULL) {
            if (aSprite->killMe) {
                RemoveSprite(aSprite);
            }
        }
    });
}

}  // namespace antares

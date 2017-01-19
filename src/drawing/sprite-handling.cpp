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

#include "drawing/sprite-handling.hpp"

#include <numeric>

#include "drawing/color.hpp"
#include "drawing/pix-table.hpp"
#include "drawing/shapes.hpp"
#include "drawing/text.hpp"
#include "game/globals.hpp"
#include "game/sys.hpp"
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

static const uint32_t kSolidSquareBlip  = 0x00000000;
static const uint32_t kTriangleUpBlip   = 0x00000010;
static const uint32_t kDiamondBlip      = 0x00000020;
static const uint32_t kPlusBlip         = 0x00000030;
static const uint32_t kFramedSquareBlip = 0x00000040;

static const uint32_t kBlipSizeMask = 0x0000000f;
static const uint32_t kBlipTypeMask = 0x000000f0;

static void draw_tiny_square(const Rect& rect, const RgbColor& color) {
    Rects().fill(rect, color);
}

static void draw_tiny_triangle(const Rect& rect, const RgbColor& color) {
    sys.video->draw_triangle(rect, color);
}

static void draw_tiny_diamond(const Rect& rect, const RgbColor& color) {
    sys.video->draw_diamond(rect, color);
}

static void draw_tiny_plus(const Rect& rect, const RgbColor& color) {
    sys.video->draw_plus(rect, color);
}

static draw_tiny_t draw_tiny_function(uint8_t id) {
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

int32_t ANTARES_GLOBAL gAbsoluteScale = MIN_SCALE;

void SpriteHandlingInit() {
    g.sprites.reset(new Sprite[Sprite::size]);
    ResetAllSprites();

    for (int i = 0; i < 4000; ++i) {
        Randomize(256);
    }
}

Sprite* Sprite::get(int number) {
    if ((0 <= number) && (number < size)) {
        return &g.sprites[number];
    }
    return nullptr;
}

Sprite::Sprite()
        : table(NULL),
          resID(-1),
          style(spriteNormal),
          styleColor(RgbColor::white()),
          styleData(0),
          whichLayer(kNoSpriteLayer),
          killMe(false),
          draw_tiny(NULL) {}

void ResetAllSprites() {
    for (auto sprite : Sprite::all()) {
        *sprite = Sprite();
    }
}

void Pix::reset() {
    pix.clear();
}

NatePixTable* Pix::add(int16_t resource_id) {
    NatePixTable* result = get(resource_id);
    if (result) {
        return result;
    }

    int16_t real_resource_id = resource_id & ~kSpriteTableColorIDMask;
    int16_t color            = (resource_id & kSpriteTableColorIDMask) >> kSpriteTableColorShift;
    auto    it = pix.emplace(resource_id, NatePixTable(real_resource_id, color)).first;
    return &it->second;
}

NatePixTable* Pix::get(int16_t resource_id) {
    auto it = pix.find(resource_id);
    if (it != pix.end()) {
        return &it->second;
    }
    return nullptr;
}

Handle<Sprite> AddSprite(
        Point where, NatePixTable* table, int16_t resID, int16_t whichShape, int32_t scale,
        int32_t size, int16_t layer, const RgbColor& color) {
    for (Handle<Sprite> sprite : Sprite::all()) {
        if (sprite->table == NULL) {
            sprite->where      = where;
            sprite->table      = table;
            sprite->resID      = resID;
            sprite->whichShape = whichShape;
            sprite->scale      = scale;
            sprite->whichLayer = layer;
            sprite->tinySize   = size;
            sprite->tinyColor  = color;
            sprite->draw_tiny  = draw_tiny_function(size);
            sprite->killMe     = false;
            sprite->style      = spriteNormal;
            sprite->styleColor = RgbColor::white();
            sprite->styleData  = 0;

            return sprite;
        }
    }

    return Sprite::none();
}

void RemoveSprite(Handle<Sprite> sprite) {
    sprite->killMe = false;
    sprite->table  = NULL;
    sprite->resID  = -1;
}

Fixed scale_by(Fixed value, int32_t scale) {
    return (value * scale) / SCALE_SCALE;
}

Fixed evil_scale_by(Fixed value, int32_t scale) {
    return (value * scale) >> SHIFT_SCALE;
}

int32_t evil_scale_by(int32_t value, int32_t scale) {
    return (value * scale) >> SHIFT_SCALE;
}

Rect scale_sprite_rect(const NatePixTable::Frame& frame, Point where, int32_t scale) {
    Rect draw_rect =
            Rect(0, 0, evil_scale_by(frame.width(), scale), evil_scale_by(frame.height(), scale));
    draw_rect.offset(
            where.h - evil_scale_by(frame.center().h, scale),
            where.v - evil_scale_by(frame.center().v, scale));
    return draw_rect;
}

void draw_sprites() {
    if (gAbsoluteScale >= kBlipThreshhold) {
        for (int layer : range<int>(kFirstSpriteLayer, kLastSpriteLayer + 1)) {
            for (auto aSprite : Sprite::all()) {
                if ((aSprite->table != NULL) && !aSprite->killMe &&
                    (aSprite->whichLayer == layer)) {
                    int32_t trueScale = evil_scale_by(aSprite->scale, gAbsoluteScale);
                    const NatePixTable::Frame& frame = aSprite->table->at(aSprite->whichShape);

                    const int32_t map_width  = evil_scale_by(frame.width(), trueScale);
                    const int32_t map_height = evil_scale_by(frame.height(), trueScale);
                    const int32_t scaled_h   = evil_scale_by(frame.center().h, trueScale);
                    const int32_t scaled_v   = evil_scale_by(frame.center().v, trueScale);
                    const Point   scaled_center(scaled_h, scaled_v);

                    Rect draw_rect(0, 0, map_width, map_height);
                    draw_rect.offset(aSprite->where.h - scaled_h, aSprite->where.v - scaled_v);

                    switch (aSprite->style) {
                        case spriteNormal: frame.texture().draw(draw_rect); break;

                        case spriteColor:
                            Randomize(63);
                            frame.texture().draw_static(
                                    draw_rect, aSprite->styleColor, aSprite->styleData);
                            break;
                    }
                }
            }
        }
    } else {
        for (int layer : range<int>(kFirstSpriteLayer, kLastSpriteLayer + 1)) {
            for (auto aSprite : Sprite::all()) {
                int tinySize = aSprite->tinySize & kBlipSizeMask;
                if ((aSprite->table != NULL) && !aSprite->killMe && tinySize &&
                    (aSprite->draw_tiny != NULL) && (aSprite->whichLayer == layer)) {
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
    for (auto aSprite : Sprite::all()) {
        if (aSprite->table != NULL) {
            if (aSprite->killMe) {
                RemoveSprite(aSprite);
            }
        }
    }
}

}  // namespace antares

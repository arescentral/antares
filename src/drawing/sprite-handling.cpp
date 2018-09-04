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
#include <sfz/sfz.hpp>

#include "data/resource.hpp"
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

using sfz::range;
using std::map;
using std::unique_ptr;

namespace antares {

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

static draw_tiny_t draw_tiny_function(BaseObject::Icon::Shape shape, int size) {
    if (size <= 0) {
        return NULL;
    }
    switch (shape) {
        case BaseObject::Icon::Shape::TRIANGLE: return draw_tiny_triangle;
        case BaseObject::Icon::Shape::SQUARE: return draw_tiny_square;
        case BaseObject::Icon::Shape::PLUS: return draw_tiny_plus;
        case BaseObject::Icon::Shape::DIAMOND: return draw_tiny_diamond;
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
          style(spriteNormal),
          styleColor(RgbColor::white()),
          styleData(0),
          whichLayer(BaseObject::Layer::NONE),
          killMe(false),
          draw_tiny(NULL) {}

void ResetAllSprites() {
    for (auto sprite : Sprite::all()) {
        *sprite = Sprite();
    }
}

void Pix::reset() {
    _pix.clear();
    _cursor.reset(new NatePixTable("gui/cursor", Hue::GRAY));
}

NatePixTable* Pix::add(pn::string_view name, Hue hue) {
    NatePixTable* result = get(name, hue);
    if (result) {
        return result;
    }

    auto it = _pix.emplace(std::make_pair(name.copy(), hue), NatePixTable(name, hue)).first;
    return &it->second;
}

NatePixTable* Pix::get(pn::string_view id, Hue hue) {
    auto it = _pix.find({id.copy(), hue});
    if (it != _pix.end()) {
        return &it->second;
    }
    return nullptr;
}

const NatePixTable* Pix::cursor() { return _cursor.get(); }

Handle<Sprite> AddSprite(
        Point where, NatePixTable* table, pn::string_view name, Hue hue, int16_t whichShape,
        int32_t scale, sfz::optional<BaseObject::Icon> icon, BaseObject::Layer layer, Hue tiny_hue,
        uint8_t tiny_shade) {
    for (Handle<Sprite> sprite : Sprite::all()) {
        if (sprite->table == NULL) {
            sprite->where      = where;
            sprite->table      = table;
            sprite->whichShape = whichShape;
            sprite->scale      = scale;
            sprite->whichLayer = layer;
            sprite->icon = icon.value_or(BaseObject::Icon{BaseObject::Icon::Shape::SQUARE, 0});
            sprite->tinyColor  = {tiny_hue, tiny_shade};
            sprite->draw_tiny  = draw_tiny_function(sprite->icon.shape, sprite->icon.size);
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
}

Fixed scale_by(Fixed value, int32_t scale) { return (value * scale) / SCALE_SCALE; }

Fixed evil_scale_by(Fixed value, int32_t scale) { return (value * scale) >> SHIFT_SCALE; }

int32_t evil_scale_by(int32_t value, int32_t scale) { return (value * scale) >> SHIFT_SCALE; }

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
        for (BaseObject::Layer layer :
             {BaseObject::Layer::BASES, BaseObject::Layer::SHIPS, BaseObject::Layer::SHOTS}) {
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
        for (BaseObject::Layer layer :
             {BaseObject::Layer::BASES, BaseObject::Layer::SHIPS, BaseObject::Layer::SHOTS}) {
            for (auto aSprite : Sprite::all()) {
                int tinySize = aSprite->icon.size;
                if ((aSprite->table != NULL) && !aSprite->killMe && tinySize &&
                    (aSprite->draw_tiny != NULL) && (aSprite->whichLayer == layer)) {
                    Rect tiny_rect(-tinySize, -tinySize, tinySize, tinySize);
                    tiny_rect.offset(aSprite->where.h, aSprite->where.v);
                    aSprite->draw_tiny(
                            tiny_rect, GetRGBTranslateColorShade(
                                               aSprite->tinyColor.hue, aSprite->tinyColor.shade));
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

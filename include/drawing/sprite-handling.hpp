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

#include <map>

#include "data/base-object.hpp"
#include "data/handle.hpp"
#include "drawing/color.hpp"
#include "drawing/pix-table.hpp"
#include "math/fixed.hpp"

namespace antares {

const int32_t kNoSpriteTable = -1;

const int16_t kSpriteTableColorShift  = 11;
const int16_t kSpriteTableColorIDMask = 0x7800;  // bits 11-14
// this makes the max legal sprite id 2047

const int32_t kSpriteMaxSize  = 2048;
const int32_t kBlipThreshhold = kOneQuarterScale;

const size_t MAX_PIX_SIZE = 480;

const size_t  kMaxPixTableEntry = 60;
const int32_t kNoSprite         = -1;

enum spriteStyleType { spriteNormal = 0, spriteColor = 2 };

typedef void (*draw_tiny_t)(const Rect& rect, const RgbColor& color);

class Sprite {
  public:
    static Sprite*            get(int number);
    static Handle<Sprite>     none() { return Handle<Sprite>(-1); }
    static HandleList<Sprite> all() { return HandleList<Sprite>(0, size); }

    Sprite();

    Point             where;
    NatePixTable*     table;
    int               whichShape;
    int32_t           scale;
    spriteStyleType   style;
    RgbColor          styleColor;
    int16_t           styleData;
    BaseObject::Layer whichLayer;
    struct {
        Hue     hue;
        uint8_t shade;
    } tinyColor;
    bool        killMe;
    draw_tiny_t draw_tiny;

    BaseObject::Icon icon;

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
Fixed   scale_by(Fixed value, int32_t scale);
Fixed   evil_scale_by(Fixed value, int32_t scale);
int32_t evil_scale_by(int32_t value, int32_t scale);

class Pix {
  public:
    void                reset();
    NatePixTable*       add(pn::string_view id, Hue hue);
    NatePixTable*       get(pn::string_view id, Hue hue);
    const NatePixTable* cursor();

  private:
    std::map<std::pair<pn::string, Hue>, NatePixTable> _pix;
    std::unique_ptr<NatePixTable>                      _cursor;
};

void           SpriteHandlingInit();
void           ResetAllSprites();
Rect           scale_sprite_rect(const NatePixTable::Frame& frame, Point where, int32_t scale);
Handle<Sprite> AddSprite(
        Point where, NatePixTable* table, pn::string_view name, Hue hue, int16_t whichShape,
        int32_t scale, sfz::optional<BaseObject::Icon> icon, BaseObject::Layer layer, Hue tiny_hue,
        uint8_t tiny_shade);
void RemoveSprite(Handle<Sprite> sprite);
void draw_sprites();
void CullSprites();

}  // namespace antares

#endif  // ANTARES_DRAWING_SPRITE_HANDLING_HPP_

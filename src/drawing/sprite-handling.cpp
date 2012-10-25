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
    int16_t                         static_value;
};
pixTableType gPixTable[kMaxPixTableEntry];

int32_t gAbsoluteScale = MIN_SCALE;
scoped_array<spriteType> gSpriteTable;
const RgbColor& kNoTinyColor = RgbColor::kBlack;

const int32_t kStaticTableSize = 4000;
const int32_t kStaticTileWidth = 16;
const int32_t kStaticTileHeight = 16;
const int32_t kStaticTileCount = 61;
Sprite** gStaticTiles;

bool PixelInSprite_IsOutside(
        const PixMap& pix, long x, long y, const int32_t* hmap, const int32_t* vmap);

void SpriteHandlingInit() {
    ResetAllPixTables();

    gSpriteTable.reset(new spriteType[kMaxSpriteNum]);
    ResetAllSprites();

    uint8_t static_table[kStaticTableSize];
    SFZ_FOREACH(int i, range(kStaticTableSize), {
        static_table[i] = Randomize(256);
    });

    int static_index = 0;
    gStaticTiles = new Sprite*[kStaticTileCount];
    SFZ_FOREACH(Sprite** tile, slice(gStaticTiles, 0, kStaticTileCount), {
        ArrayPixMap pix(kStaticTileWidth, kStaticTileHeight);
        SFZ_FOREACH(int y, range(kStaticTileHeight), {
            SFZ_FOREACH(int x, range(kStaticTileWidth), {
                pix.set(x, y, RgbColor(static_table[static_index], 0xff, 0xff, 0xff));
                static_index = (static_index + 223) % 3989;
            });
        });
        *tile = VideoDriver::driver()->new_sprite("/x/static", pix);
    });
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

namespace {

void clip_transfer(Rect* from, Rect* to, const Rect& clip) {
    if (to->left < clip.left) {
        from->left += clip.left - to->left;
        to->left = clip.left;
    }
    if (to->right > clip.right) {
        from->right -= to->right - clip.right;
        to->right = clip.right;
    }
    if (to->top < clip.top) {
        from->top += clip.top - to->top;
        to->top = clip.top;
    }
    if (to->bottom > clip.bottom) {
        from->bottom -= to->bottom - clip.bottom;
        to->bottom = clip.bottom;
    }
}

void build_scale_down_map(int32_t* map, int32_t source_max, int32_t dest_max) {
    int32_t h = source_max - 1;
    int32_t v = (dest_max - 1) * 2;
    int32_t d = v - h;
    h = v - (h * 2);
    int32_t x = 0;
    int32_t y = 0;
    int32_t i = 0;
    int32_t last = 0;
    if (v == 0) {
        v = 1;
    }

    while ((x < source_max) || (y < dest_max)) {
        x++;
        i++;
        if (d > 0) {
            *map = (x - (i >> 1L)) - last;
            last += *map;
            i = 0;
            y++;
            map++;
            d += h;
        } else {
            d += v;
        }
    }
    *map = source_max - last;
}

void build_scale_up_map(int32_t* map, int32_t source_max, int32_t dest_max) {
    int32_t h = dest_max - 1;
    int32_t v = (source_max - 1) * 2;
    int32_t d = v - h;
    h = v - (h * 2);
    int32_t x = 0;
    int32_t y = 0;
    int32_t i = 0;
    while ((x < dest_max - 1) || (y < source_max - 1)) {
        x++;
        i++;
        if (d > 0) {
            *map = i;

            i = 0;
            y++;
            map++;
            d += h;
        } else {
            d += v;
        }
    }
    *map = i + 1;
}

void no_change(PixMap* pix) {
    static_cast<void>(pix);
}

class Outline {
  public:
    Outline(const RgbColor& inside, const RgbColor& outside):
            _inside(inside),
            _outside(outside) { }

    void operator()(PixMap* pix) const {
        SFZ_FOREACH(int32_t y, range(pix->size().height), {
            SFZ_FOREACH(int32_t x, range(pix->size().width), {
                if (!is_clear(*pix, x, y)) {
                    if (is_exterior(*pix, x, y)) {
                        pix->set(x, y, _outside);
                    } else {
                        pix->set(x, y, _inside);
                    }
                }
            });
        });
    }

  private:
    static bool is_clear(const PixMap& pix, int32_t x, int32_t y) {
        return (pix.get(x, y).alpha == 0);
    }

    static bool is_exterior(const PixMap& pix, int32_t x, int32_t y) {
        Rect interior = pix.size().as_rect();
        interior.inset(1, 1);
        if (!interior.contains(Point(x, y))) {
            return true;
        }
        return is_clear(pix, x-1, y-1) || is_clear(pix, x+0, y-1) || is_clear(pix, x+1, y-1)
            || is_clear(pix, x-1, y+0) || /* center pixel is here */ is_clear(pix, x+1, y+0)
            || is_clear(pix, x-1, y+1) || is_clear(pix, x+0, y+1) || is_clear(pix, x+1, y+1);
    }

    const RgbColor _inside;
    const RgbColor _outside;
};

class ScaledDownRow {
  public:
    ScaledDownRow(int32_t source_width, int32_t dest_width):
            _dest_width(dest_width) {
        build_scale_down_map(_hmap_storage, source_width, dest_width);
    }

    void draw_row(const RgbColor* source_row, RgbColor* dest_row) const {
        const RgbColor* source_pixel = source_row;
        RgbColor* dest_pixel = dest_row;
        SFZ_FOREACH(const int32_t* hmap, slice(_hmap_storage, 0, _dest_width), {
            *dest_pixel = *source_pixel;
            source_pixel += *hmap;
            ++dest_pixel;
        });
    }

  private:
    int32_t _hmap_storage[MAX_PIX_SIZE];
    int32_t _dest_width;
};

template <class ScaledRow>
class ScaledDownColumn {
  public:
    ScaledDownColumn(int32_t source_height, int32_t dest_height):
            _dest_height(dest_height) {
        build_scale_down_map(_vmap_storage, source_height, dest_height);
    }

    void draw_pix_map(const ScaledRow& scaled_row, const PixMap& source, PixMap* dest) const {
        int32_t v = 0;
        const RgbColor* source_row = source.row(0);
        RgbColor* dest_row = dest->mutable_row(0);
        SFZ_FOREACH(const int32_t* vmap, slice(_vmap_storage, 0, _dest_height), {
            scaled_row.draw_row(source_row, dest_row);
            v += *vmap;
            source_row += (*vmap) * source.row_bytes();
            dest_row += dest->row_bytes();
        });
    }

  private:
    int32_t _vmap_storage[MAX_PIX_SIZE];
    int32_t _dest_height;
};

class ScaledUpRow {
  public:
    ScaledUpRow(int32_t source_width, int32_t dest_width):
            _source_width(source_width) {
        build_scale_up_map(_hmap_storage, source_width, dest_width);
    }

    void draw_row(const RgbColor* source_row, RgbColor* dest_row) const {
        const RgbColor* source_pixel = source_row;
        RgbColor* dest_pixel = dest_row;
        SFZ_FOREACH(const int32_t* hmap, slice(_hmap_storage, 0, _source_width), {
            SFZ_FOREACH(int32_t j, range(*hmap), {
                *dest_pixel = *source_pixel;
                ++dest_pixel;
            });
            ++source_pixel;
        });
    }

  private:
    int32_t _hmap_storage[MAX_PIX_SIZE];
    const int32_t _source_width;

    DISALLOW_COPY_AND_ASSIGN(ScaledUpRow);
};

template <class ScaledRow>
class ScaledUpColumn {
  public:
    ScaledUpColumn(int32_t source_height, int32_t dest_height):
            _source_height(source_height) {
        build_scale_up_map(_vmap_storage, source_height, dest_height);
    }

    void draw_pix_map(const ScaledRow& scaled_row, const PixMap& source, PixMap* dest) const {
        const RgbColor* source_row = source.row(0);
        RgbColor* dest_row = dest->mutable_row(0);
        SFZ_FOREACH(const int32_t* vmap, slice(_vmap_storage, 0, _source_height), {
            SFZ_FOREACH(int32_t i, range(*vmap), {
                scaled_row.draw_row(source_row, dest_row);
                dest_row += dest->row_bytes();
            });
            source_row += source.row_bytes();
        });
    }

  private:
    int32_t _vmap_storage[MAX_PIX_SIZE];
    const int32_t _source_height;

    DISALLOW_COPY_AND_ASSIGN(ScaledUpColumn);
};

template <typename ScaledRow, template <typename ScaledRow> class ScaledColumn>
void internal_scale_pix_map(const PixMap& source, PixMap* dest) {
    const ScaledRow scaled_row(
            source.size().width, dest->size().width);
    const ScaledColumn<ScaledRow> scaled_column(
            source.size().height, dest->size().height);
    scaled_column.draw_pix_map(scaled_row, source, dest);
}

template <typename PostProcess>
Rect DrawSpriteInPixMap(
        const PostProcess& post_process, const NatePixTable::Frame& frame,
        Point where, int32_t scale, const Rect& clip_rect, PixMap* dest) {
    Rect draw_rect = Rect(
            0, 0, evil_scale_by(frame.width(), scale), evil_scale_by(frame.height(), scale));
    draw_rect.offset(
            where.h - evil_scale_by(frame.center().h, scale),
            where.v - evil_scale_by(frame.center().v, scale));
    const PixMap& source = frame.pix_map();

    ArrayPixMap intermediate(draw_rect.width(), draw_rect.height());

    static const int32_t kMaxScaleFactor = MAX_SCALE / SCALE_SCALE;
    if ((!draw_rect.intersects(clip_rect))
            || (draw_rect.width() > (source.size().width * kMaxScaleFactor))
            || (draw_rect.height() > (source.size().height * kMaxScaleFactor))) {
        return Rect();
    }

    scale_pix_map(source, &intermediate);
    post_process(&intermediate);

    Rect map_rect = draw_rect;
    map_rect.offset(-map_rect.left, -map_rect.top);
    clip_transfer(&map_rect, &draw_rect, clip_rect);
    dest->view(draw_rect).composite(intermediate.view(map_rect));
    return draw_rect;
}

void draw_static(const Rect& draw_rect) {
    const int32_t hsize = (draw_rect.width() + kStaticTileWidth - 1) / kStaticTileWidth;
    const int32_t vsize = (draw_rect.height() + kStaticTileHeight - 1) / kStaticTileHeight;

    int32_t static_index = Randomize(kStaticTileCount);
    int32_t y = draw_rect.top;
    SFZ_FOREACH(int32_t i, range(vsize), {
        int32_t x = draw_rect.left;
        SFZ_FOREACH(int32_t j, range(hsize), {
            gStaticTiles[static_index]->draw(x, y);
            static_index = (static_index + 1) % kStaticTileCount;
            x += kStaticTileWidth;
        });
        y += kStaticTileHeight;
    });
}

}  // namespace

int32_t scale_by(int32_t value, int32_t scale) {
    return (value * scale) / SCALE_SCALE;
}

int32_t evil_scale_by(int32_t value, int32_t scale) {
    return (value * scale) >> SHIFT_SCALE;
}

void scale_pix_map(const PixMap& source, PixMap* dest) {
    const bool scale_down_row = dest->size().width <= source.size().width;
    const bool scale_down_column = dest->size().height <= source.size().height;
    if (scale_down_row && scale_down_column) {
        internal_scale_pix_map<ScaledDownRow, ScaledDownColumn>(source, dest);
    } else if (scale_down_row) {
        internal_scale_pix_map<ScaledDownRow, ScaledUpColumn>(source, dest);
    } else if (scale_down_column) {
        internal_scale_pix_map<ScaledUpRow, ScaledDownColumn>(source, dest);
    } else {
        internal_scale_pix_map<ScaledUpRow, ScaledUpColumn>(source, dest);
    }
}

void OptScaleSpritePixInPixMap(
        const NatePixTable::Frame& frame, Point where, int32_t scale, Rect *draw_rect,
        const Rect& clip_rect, PixMap* pix) {
    *draw_rect = DrawSpriteInPixMap(no_change, frame, where, scale, clip_rect, pix);
}

void OutlineScaleSpritePixInPixMap(
        const NatePixTable::Frame& frame, Point where, int32_t scale, Rect *draw_rect,
        const Rect& clip_rect, PixMap* pix, const RgbColor& color_out, const RgbColor& color_in) {
    Outline outline(color_in, color_out);
    *draw_rect = DrawSpriteInPixMap(outline, frame, where, scale, clip_rect, pix);
}

bool PixelInSprite_IsOutside(
        const PixMap& pix, long x, long y, const int32_t* hmap, const int32_t* vmap) {
    const RgbColor* pixel;
    int32_t rowPlus = pix.size().width;
    const int32_t* hmapStart = hmap;

    if ((x == 0) || (x >= (pix.size().width - *hmap))
            || (y == 0) || (y >= (pix.size().height - *vmap))) {
        return true;
    }

    vmap--;
    hmapStart--;
    rowPlus -= (*hmapStart + (*(hmapStart + 1)) + (*(hmapStart + 2)));
    pixel = pix.row(y - *vmap) + (x - *hmapStart);
    SFZ_FOREACH(int32_t j, range(y - 1, y + 2), {
        hmap = hmapStart;
        SFZ_FOREACH(int32_t i, range(x - 1, x + 2), {
            if (((j != y) || (x != i)) && (!pixel->alpha)) {
                return true;
            }
            pixel += *hmap++;
        });
        pixel += ((*vmap++ - 1) * pix.size().width) + rowPlus;
    });
    return false;
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
                        {
                            Stencil stencil(VideoDriver::driver());
                            frame.sprite().draw(draw_rect);
                            stencil.apply();
                            if (aSprite->styleColor != RgbColor::kBlack) {
                                VideoDriver::driver()->fill_rect(draw_rect, aSprite->styleColor);
                            }

                            Stencil stencil2(VideoDriver::driver());
                            stencil2.set_threshold(aSprite->styleData);
                            draw_static(draw_rect);
                            stencil2.apply();
                            frame.sprite().draw(draw_rect);
                        }
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
                    aSprite->tiny_sprite->draw(
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

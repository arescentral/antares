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

#include "drawing/briefing.hpp"

#include "data/level.hpp"
#include "drawing/color.hpp"
#include "drawing/pix-table.hpp"
#include "drawing/sprite-handling.hpp"
#include "game/admiral.hpp"
#include "game/globals.hpp"
#include "game/initial.hpp"
#include "game/level.hpp"
#include "game/space-object.hpp"
#include "game/sys.hpp"
#include "lang/casts.hpp"
#include "lang/defines.hpp"
#include "video/driver.hpp"

using std::vector;

namespace antares {

const int32_t kBriefing_Grid_Size = 16;

static ANTARES_GLOBAL vector<Rect> gBriefingSpriteBounds;

Point BriefingSprite_GetBestLocation(
        const NatePixTable::Frame& frame, int32_t scale, Point fromWhere, bool* grid,
        int32_t gridWidth, int32_t gridHeight, const Rect& bounds);

void GetInitialObjectSpriteData(
        Handle<const Initial> whichObject, int32_t maxSize, const Rect& bounds,
        const coordPointType& corner, int32_t scale, int32_t* thisScale, Point* where,
        Rect* spriteRect);

void GetRealObjectSpriteData(
        const coordPointType& realCoord, const BaseObject& baseObject, Handle<Admiral> owner,
        const SpaceObject::PixID& sprite, int32_t maxSize, const Rect& bounds,
        const coordPointType& corner, int32_t scale, int32_t* thisScale,
        const NatePixTable::Frame** frame, Point* where, Rect* spriteRect);

Rect SpriteBounds_Get(const NatePixTable::Frame& frame, Point where, int32_t scale);

bool BriefingSprite_IsLocationLegal(
        const NatePixTable::Frame& frame, int32_t scale, Point where, bool* grid,
        int32_t gridWidth, int32_t gridHeight, const Rect& bounds);

void BriefingSprite_UseLocation(
        const NatePixTable::Frame& frame, int32_t scale, Point where, bool* grid,
        int32_t gridWidth, int32_t gridHeight, const Rect& bounds);

bool Briefing_Grid_Get(bool* grid, int32_t x, int32_t y, int32_t gridWidth, int32_t gridHeight);

void Briefing_Grid_Set(
        bool* grid, int32_t x, int32_t y, int32_t gridWidth, int32_t gridHeight, bool value);

Point BriefingSprite_GetBestLocation(
        const NatePixTable::Frame& frame, int32_t scale, Point fromWhere, bool* grid,
        int32_t gridWidth, int32_t gridHeight, const Rect& bounds) {
    int32_t offsetSize = 1, i;
    Point   result     = fromWhere;

    if (BriefingSprite_IsLocationLegal(
                frame, scale, result, grid, gridWidth, gridHeight, bounds)) {
        return result;
    }

    while (offsetSize < gridWidth) {
        for (i = -offsetSize; i <= offsetSize; i++) {
            // try left
            result.h = fromWhere.h - (offsetSize * kBriefing_Grid_Size);
            result.v = fromWhere.v + (i * kBriefing_Grid_Size);
            if (BriefingSprite_IsLocationLegal(
                        frame, scale, result, grid, gridWidth, gridHeight, bounds)) {
                return result;
            }

            // try right
            result.h = fromWhere.h + (offsetSize * kBriefing_Grid_Size);
            result.v = fromWhere.v + (i * kBriefing_Grid_Size);
            if (BriefingSprite_IsLocationLegal(
                        frame, scale, result, grid, gridWidth, gridHeight, bounds)) {
                return result;
            }

            // try top
            result.h = fromWhere.h + (i * kBriefing_Grid_Size);
            result.v = fromWhere.v - (offsetSize * kBriefing_Grid_Size);
            if (BriefingSprite_IsLocationLegal(
                        frame, scale, result, grid, gridWidth, gridHeight, bounds)) {
                return result;
            }

            // try bottom
            result.h = fromWhere.h + (i * kBriefing_Grid_Size);
            result.v = fromWhere.v + (offsetSize * kBriefing_Grid_Size);
            if (BriefingSprite_IsLocationLegal(
                        frame, scale, result, grid, gridWidth, gridHeight, bounds)) {
                return result;
            }
        }
        offsetSize++;
    }
    result = fromWhere;
    return result;
}

bool BriefingSprite_IsLocationLegal(
        const NatePixTable::Frame& frame, int32_t scale, Point where, bool* grid,
        int32_t gridWidth, int32_t gridHeight, const Rect& bounds) {
    Rect    spriteBounds;
    int32_t i, j;

    spriteBounds = SpriteBounds_Get(frame, where, scale);
    spriteBounds.offset(-bounds.left, -bounds.top);
    spriteBounds.left /= kBriefing_Grid_Size;
    spriteBounds.right /= kBriefing_Grid_Size;
    spriteBounds.top /= kBriefing_Grid_Size;
    spriteBounds.bottom /= kBriefing_Grid_Size;
    if (spriteBounds.left < 1)
        return false;
    if (spriteBounds.right >= gridWidth)
        return false;
    if (spriteBounds.top < 1)
        return false;
    if (spriteBounds.bottom >= gridHeight)
        return false;

    for (j = spriteBounds.top; j <= spriteBounds.bottom; j++) {
        for (i = spriteBounds.left; i <= spriteBounds.right; i++) {
            if (Briefing_Grid_Get(grid, i, j, gridWidth, gridHeight))
                return false;
        }
    }
    return true;
}

void BriefingSprite_UseLocation(
        const NatePixTable::Frame& frame, int32_t scale, Point where, bool* grid,
        int32_t gridWidth, int32_t gridHeight, const Rect& bounds) {
    Rect    spriteBounds;
    int32_t i, j;

    spriteBounds = SpriteBounds_Get(frame, where, scale);
    spriteBounds.offset(-bounds.left, -bounds.top);
    spriteBounds.left /= kBriefing_Grid_Size;
    spriteBounds.right /= kBriefing_Grid_Size;
    spriteBounds.top /= kBriefing_Grid_Size;
    spriteBounds.bottom /= kBriefing_Grid_Size;
    if (spriteBounds.left < 1)
        return;
    if (spriteBounds.right >= gridWidth)
        return;
    if (spriteBounds.top < 1)
        return;
    if (spriteBounds.bottom >= gridHeight)
        return;

    for (j = spriteBounds.top; j <= spriteBounds.bottom; j++) {
        for (i = spriteBounds.left; i <= spriteBounds.right; i++) {
            Briefing_Grid_Set(grid, i, j, gridWidth, gridHeight, true);
        }
    }
}

bool Briefing_Grid_Get(bool* grid, int32_t x, int32_t y, int32_t gridWidth, int32_t gridHeight) {
    if (grid == NULL)
        return true;
    if (x < 1)
        return true;
    if (x >= gridWidth)
        return true;
    if (y < 1)
        return true;
    if (y >= gridHeight)
        return true;

    grid = grid + (y * gridWidth) + x;
    return *grid;
}

void Briefing_Grid_Set(
        bool* grid, int32_t x, int32_t y, int32_t gridWidth, int32_t gridHeight, bool value) {
    if (grid == NULL)
        return;
    if (x < 1)
        return;
    if (x >= gridWidth)
        return;
    if (y < 1)
        return;
    if (y >= gridHeight)
        return;

    grid  = grid + (y * gridWidth) + x;
    *grid = value;
}

void GetInitialObjectSpriteData(
        Handle<const Initial> whichObject, int32_t maxSize, const Rect& bounds,
        const coordPointType& corner, int32_t scale, int32_t* thisScale, Point* where,
        Rect* spriteRect) {
    spriteRect->right = spriteRect->left = -1;

    auto sObject = GetObjectFromInitialNumber(whichObject);

    if (sObject.get() && sObject->active) {
        const NatePixTable::Frame* frame = NULL;
        GetRealObjectSpriteData(
                sObject->location, *sObject->base, sObject->owner, *sObject->pix_id, maxSize,
                bounds, corner, scale, thisScale, &frame, where, spriteRect);

        *spriteRect = gBriefingSpriteBounds[sObject.number()];
    }
}

void GetRealObjectSpriteData(
        const coordPointType& realCoord, const BaseObject& baseObject, Handle<Admiral> owner,
        const SpaceObject::PixID& sprite, int32_t maxSize, const Rect& bounds,
        const coordPointType& corner, int32_t scale, int32_t* thisScale,
        const NatePixTable::Frame** frame, Point* where, Rect* spriteRect) {
    int            whichShape;
    coordPointType coord = realCoord;

    NatePixTable* pixTable = sys.pix.get(sprite.name, sprite.hue);
    if (pixTable == NULL) {
        throw std::runtime_error("Couldn't load a requested sprite");
    }

    if (baseObject.attributes & kIsSelfAnimated)
        whichShape = more_evil_fixed_to_long(baseObject.animation->frames.begin);
    else
        whichShape = 0;

    *frame = &pixTable->at(whichShape);

    int32_t tlong;
    tlong = *thisScale = implicit_cast<int32_t>(maxSize) * SCALE_SCALE;
    *thisScale /= (*frame)->width();
    tlong /= (*frame)->height();

    if (tlong < *thisScale)
        *thisScale = tlong;

    coord.h = evil_scale_by(coord.h - corner.h, scale);
    coord.h += bounds.left;

    coord.v = evil_scale_by(coord.v - corner.v, scale);
    coord.v += bounds.top;

    where->h = coord.h;
    where->v = coord.v;

    spriteRect->left = evil_scale_by((*frame)->center().h, *thisScale);
    spriteRect->left = where->h - spriteRect->left;

    spriteRect->right = evil_scale_by((*frame)->width(), *thisScale);
    spriteRect->right = spriteRect->left + spriteRect->right;

    spriteRect->top = evil_scale_by((*frame)->center().v, *thisScale);
    spriteRect->top = where->v - spriteRect->top;

    spriteRect->bottom = evil_scale_by((*frame)->height(), *thisScale);
    spriteRect->bottom = spriteRect->top + spriteRect->bottom;
}

Rect SpriteBounds_Get(const NatePixTable::Frame& frame, Point where, int32_t scale) {
    Rect    bounds;
    int32_t tlong;

    tlong       = evil_scale_by(frame.center().h, scale);
    tlong       = where.h - tlong;
    bounds.left = tlong;

    tlong        = evil_scale_by(frame.width(), scale);
    tlong        = bounds.left + tlong;
    bounds.right = tlong;

    tlong      = evil_scale_by(frame.center().v, scale);
    tlong      = where.v - tlong;
    bounds.top = tlong;

    tlong         = evil_scale_by(frame.height(), scale);
    tlong         = bounds.top + tlong;
    bounds.bottom = tlong;

    return bounds;
}

template <typename Renderer>
static void render_briefing_with(
        const Renderer& renderer, int32_t maxSize, const Rect& bounds,
        const coordPointType& corner, int32_t scale) {
    int32_t thisScale, gridWidth, gridHeight, i, j;
    Point   where;
    Rect    clipRect;
    bool*   gridCells = NULL;

    gridWidth  = (bounds.right - bounds.left) / kBriefing_Grid_Size;
    gridHeight = (bounds.bottom - bounds.top) / kBriefing_Grid_Size;

    gridCells = new bool[gridWidth * gridHeight];

    if (gridCells == NULL)
        return;
    for (j = 0; j < gridHeight; j++) {
        for (i = 0; i < gridWidth; i++) {
            Briefing_Grid_Set(gridCells, i, j, gridWidth, gridHeight, false);
        }
    }

    gBriefingSpriteBounds.resize(kMaxSpaceObject);

    for (auto anObject : SpaceObject::all()) {
        Rect& rect = gBriefingSpriteBounds[anObject.number()];
        rect       = Rect(0, 0, 0, 0);
        Rect spriteRect;
        if (!((anObject->active == kObjectInUse) && anObject->sprite.get())) {
            continue;
        }

        auto baseObject = anObject->base;
        if (baseObject->maxVelocity == Fixed::zero()) {
            const NatePixTable::Frame* frame = NULL;
            GetRealObjectSpriteData(
                    anObject->location, *anObject->base, anObject->owner, *anObject->pix_id,
                    maxSize, bounds, corner, scale, &thisScale, &frame, &where, &spriteRect);
            if (frame != NULL) {
                thisScale = evil_scale_by(kOneQuarterScale, sprite_scale(*baseObject));
                clipRect  = bounds;

                clipRect.left = clipRect.top = 0;
                clipRect.right -= 1;
                clipRect.bottom -= 1;

                where = BriefingSprite_GetBestLocation(
                        *frame, thisScale, where, gridCells, gridWidth, gridHeight, bounds);

                BriefingSprite_UseLocation(
                        *frame, thisScale, where, gridCells, gridWidth, gridHeight, bounds);

                renderer.draw(*frame, where, thisScale, &spriteRect, clipRect);

                rect = spriteRect;
            }
        } else {
            const NatePixTable::Frame* frame = NULL;
            GetRealObjectSpriteData(
                    anObject->location, *anObject->base, anObject->owner, *anObject->pix_id,
                    maxSize / 2, bounds, corner, scale, &thisScale, &frame, &where, &spriteRect);
            if (frame != NULL) {
                thisScale = evil_scale_by(kOneQuarterScale, sprite_scale(*baseObject));

                clipRect = bounds;

                clipRect.left = clipRect.top = 0;
                clipRect.right -= 1;
                clipRect.bottom -= 1;
                where = BriefingSprite_GetBestLocation(
                        *frame, thisScale, where, gridCells, gridWidth, gridHeight, bounds);
                BriefingSprite_UseLocation(
                        *frame, thisScale, where, gridCells, gridWidth, gridHeight, bounds);

                Hue hue = Hue::BLUE;
                if (anObject->owner.number() >= 0) {
                    hue = anObject->sprite->tinyColor.hue;
                }

                const RgbColor light_color = GetRGBTranslateColorShade(hue, LIGHT);
                const RgbColor dark_color  = GetRGBTranslateColorShade(hue, DARK);

                renderer.outline(
                        *frame, where, thisScale, &spriteRect, clipRect, light_color, dark_color);

                rect = spriteRect;
            }
        }
    }
    if (gridCells != NULL) {
        delete[] gridCells;
    }
}

struct DriverRenderer {
    Point origin;
    void  outline(
             const NatePixTable::Frame& frame, Point where, int32_t scale, Rect* sprite_rect,
             Rect clip_rect, RgbColor outline_color, RgbColor fill_color) const {
        *sprite_rect   = scale_sprite_rect(frame, where, scale);
        Rect draw_rect = *sprite_rect;
        draw_rect.offset(origin.h, origin.v);
        frame.texture().draw_outlined(draw_rect, outline_color, fill_color);
    }
    void draw(
            const NatePixTable::Frame& frame, Point where, int32_t scale, Rect* sprite_rect,
            Rect clip_rect) const {
        *sprite_rect   = scale_sprite_rect(frame, where, scale);
        Rect draw_rect = *sprite_rect;
        draw_rect.offset(origin.h, origin.v);
        frame.texture().draw(draw_rect);
    }
};

void draw_briefing_objects(
        Point origin, int32_t maxSize, Rect bounds, coordPointType corner, int32_t scale) {
    DriverRenderer renderer = {origin};
    render_briefing_with(renderer, maxSize, bounds, corner, scale);
}

BriefPointInfo BriefPoint_Data_Get(
        int32_t whichPoint, const Level& level, const coordPointType& corner, int32_t scale,
        int32_t maxSize, const Rect& bounds) {
    const Briefing& brief = level.base.briefings[whichPoint];

    BriefPointInfo info;
    info.header    = brief.title.copy();
    info.content   = brief.content.copy();
    info.highlight = Rect{};

    if (brief.initial.has_value()) {
        Point   where;
        Rect    spriteRect;
        int32_t thisScale;
        GetInitialObjectSpriteData(
                brief.initial.value_or(Initial::none()), maxSize, bounds, corner, scale,
                &thisScale, &where, &spriteRect);
        info.highlight = spriteRect;
        info.highlight.inset(-2, -2);
    }

    return info;
}

}  // namespace antares

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

#include "game/cursor.hpp"

#include <sfz/sfz.hpp>

#include "drawing/color.hpp"
#include "drawing/offscreen-gworld.hpp"
#include "drawing/pix-table.hpp"
#include "game/globals.hpp"
#include "video/driver.hpp"

using sfz::Exception;
using sfz::format;
using sfz::scoped_ptr;

namespace antares {

namespace {

const int kCursorBoundsSize = 16;

spriteCursorType* gSpriteCursor = NULL;

}  // namespace

void InitSpriteCursor() {
    gSpriteCursor = new spriteCursorType;
}

spriteCursorType::spriteCursorType() {
    show = false;

    thisShowLine = false;
    thisLineStart = Point(-1, -1);
    thisLineEnd = Point(-1, -1);
}

void ShowSpriteCursor() {
    gSpriteCursor->show = true;
}

void HideSpriteCursor() {
    gSpriteCursor->show = false;
}

bool SpriteCursorVisible() {
    return gSpriteCursor->show;
}

void SetSpriteCursorTable(short resource_id) {
    gSpriteCursor->sprite.reset(new NatePixTable(resource_id, 0));
}

void MoveSpriteCursor(Point where) {
    gSpriteCursor->where = where;
}

void ShowHintLine(Point fromWhere, Point toWhere, unsigned char color, unsigned char brightness) {
    gSpriteCursor->thisLineStart = fromWhere;
    gSpriteCursor->thisLineEnd = toWhere;
    gSpriteCursor->thisShowLine = true;

    gSpriteCursor->thisLineColor = GetRGBTranslateColorShade(color, brightness);
    gSpriteCursor->thisLineColorDark = GetRGBTranslateColorShade(color, VERY_DARK);
}

void HideHintLine() {
    gSpriteCursor->thisShowLine = false;
}

void ResetHintLine() {
    gSpriteCursor->thisShowLine = false;
    gSpriteCursor->thisLineStart.h = gSpriteCursor->thisLineStart.v = -1;
    gSpriteCursor->thisLineEnd.h = gSpriteCursor->thisLineEnd.v = -1;
    gSpriteCursor->thisLineColor = gSpriteCursor->thisLineColorDark = RgbColor::kBlack;
}

void draw_cursor() {
    if (globals()->gMouseActive) {
        const Rect clip_rect = viewport;
        Stencil stencil(VideoDriver::driver());
        VideoDriver::driver()->fill_rect(clip_rect, RgbColor::kWhite);
        stencil.apply();

        Point where = globals()->cursor_coord;
        const RgbColor color = GetRGBTranslateColorShade(SKY_BLUE, MEDIUM);
        VideoDriver::driver()->draw_line(
                Point(where.h, clip_rect.top),
                Point(where.h, (where.v - kCursorBoundsSize)), color);
        VideoDriver::driver()->draw_line(
                Point(where.h, (where.v + kCursorBoundsSize)),
                Point(where.h, clip_rect.bottom - 1), color);
        VideoDriver::driver()->draw_line(
                Point(clip_rect.left, where.v),
                Point((where.h - kCursorBoundsSize), where.v), color);
        VideoDriver::driver()->draw_line(
                Point((where.h + kCursorBoundsSize), where.v),
                Point(clip_rect.right - 1, where.v), color);
    }

    if (gSpriteCursor->show) {
        Point where = gSpriteCursor->where;
        where.offset(-gSpriteCursor->sprite->at(0).center().h, -gSpriteCursor->sprite->at(0).center().v);
        gSpriteCursor->sprite->at(0).sprite().draw(where.h, where.v);
    }
}

void draw_hint_line() {
    if (gSpriteCursor->thisShowLine) {
        Point start = gSpriteCursor->thisLineStart;
        Point end = gSpriteCursor->thisLineEnd;

        start.offset(0, 2);
        end.offset(0, 2);
        VideoDriver::driver()->draw_line(start, end, gSpriteCursor->thisLineColorDark);

        start.offset(0, -1);
        end.offset(0, -1);
        VideoDriver::driver()->draw_line(start, end, gSpriteCursor->thisLineColor);

        start.offset(0, -1);
        end.offset(0, -1);
        VideoDriver::driver()->draw_line(start, end, gSpriteCursor->thisLineColor);
    }
}

}  // namespace antares

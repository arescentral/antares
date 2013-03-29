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
#include "drawing/pix-table.hpp"
#include "game/globals.hpp"
#include "video/driver.hpp"

using sfz::Exception;
using sfz::format;
using std::unique_ptr;

namespace antares {

static const int kCursorBoundsSize = 16;

Cursor* cursor = nullptr;

void InitSpriteCursor() {
    cursor = new Cursor;
}

Cursor::Cursor() {
    show = false;

    thisShowLine = false;
    thisLineStart = Point(-1, -1);
    thisLineEnd = Point(-1, -1);
}

void ShowSpriteCursor() {
    cursor->show = true;
}

void HideSpriteCursor() {
    cursor->show = false;
}

bool SpriteCursorVisible() {
    return cursor->show;
}

void SetSpriteCursorTable(short resource_id) {
    cursor->sprite.reset(new NatePixTable(resource_id, 0));
}

void MoveSpriteCursor(Point where) {
    cursor->where = where;
}

void ShowHintLine(Point fromWhere, Point toWhere, unsigned char color, unsigned char brightness) {
    cursor->thisLineStart = fromWhere;
    cursor->thisLineEnd = toWhere;
    cursor->thisShowLine = true;

    cursor->thisLineColor = GetRGBTranslateColorShade(color, brightness);
    cursor->thisLineColorDark = GetRGBTranslateColorShade(color, VERY_DARK);
}

void HideHintLine() {
    cursor->thisShowLine = false;
}

void ResetHintLine() {
    cursor->thisShowLine = false;
    cursor->thisLineStart.h = cursor->thisLineStart.v = -1;
    cursor->thisLineEnd.h = cursor->thisLineEnd.v = -1;
    cursor->thisLineColor = cursor->thisLineColorDark = RgbColor::kBlack;
}

void draw_cursor() {
    if (globals()->gMouseActive) {
        const Rect clip_rect = viewport;
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
}

void draw_sprite_cursor() {
    if (cursor->show) {
        Point where = cursor->where;
        where.offset(-cursor->sprite->at(0).center().h, -cursor->sprite->at(0).center().v);
        cursor->sprite->at(0).sprite().draw(where.h, where.v);
    }
}

void draw_hint_line() {
    if (cursor->thisShowLine) {
        Point start = cursor->thisLineStart;
        Point end = cursor->thisLineEnd;

        start.offset(0, 2);
        end.offset(0, 2);
        VideoDriver::driver()->draw_line(start, end, cursor->thisLineColorDark);

        start.offset(0, -1);
        end.offset(0, -1);
        VideoDriver::driver()->draw_line(start, end, cursor->thisLineColor);

        start.offset(0, -1);
        end.offset(0, -1);
        VideoDriver::driver()->draw_line(start, end, cursor->thisLineColor);
    }
}

}  // namespace antares

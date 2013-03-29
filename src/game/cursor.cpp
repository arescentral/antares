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
#include "game/time.hpp"
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

Cursor::Cursor():
        show(true),
        _show_crosshairs_until(0) {
    thisShowLine = false;
    thisLineStart = Point(-1, -1);
    thisLineEnd = Point(-1, -1);
}

bool Cursor::active() const {
    return _show_crosshairs_until > now_usecs();
}

Point Cursor::clamped_location() const {
    return clamp(VideoDriver::driver()->get_mouse());
}

Point Cursor::clamp(Point p) {
    // Do the cursor, too, unless this is a replay.
    if (p.h > (viewport.right - kCursorBoundsSize - 1)) {
        p.h = viewport.right - kCursorBoundsSize - 1;
    }
    if (p.v < (viewport.top + kCursorBoundsSize)) {
        p.v = viewport.top + kCursorBoundsSize;
    } else if (p.v > (play_screen.bottom - kCursorBoundsSize - 1)) {
        p.v = play_screen.bottom - kCursorBoundsSize - 1;
    }
    return p;
}

void Cursor::mouse_down(const MouseDownEvent& event) {
    if (event.where().h >= viewport.left) {
        wake();
    }
}

void Cursor::mouse_up(const MouseUpEvent& event) {
    if (event.where().h >= viewport.left) {
        wake();
    }
}

void Cursor::mouse_move(const MouseMoveEvent& event) {
    if (event.where().h >= viewport.left) {
        wake();
    }
}

void Cursor::wake() {
    _show_crosshairs_until = now_usecs() + 1000000;
}

void SetSpriteCursorTable(short resource_id) {
    cursor->sprite.reset(new NatePixTable(resource_id, 0));
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

void Cursor::draw() const {
    if (!show) {
        return;
    }

    Point where = VideoDriver::driver()->get_mouse();

    where = clamp(where);
    if (active()) {
        const Rect clip_rect = viewport;
        const RgbColor color = GetRGBTranslateColorShade(SKY_BLUE, MEDIUM);

        Point top_a = Point(where.h, clip_rect.top);
        Point top_b = Point(where.h, (where.v - kCursorBoundsSize));
        Point bottom_a = Point(where.h, (where.v + kCursorBoundsSize));
        Point bottom_b = Point(where.h, clip_rect.bottom - 1);
        Point left_a = Point(clip_rect.left, where.v);
        Point left_b = Point((where.h - kCursorBoundsSize), where.v);
        Point right_a = Point(std::max(viewport.left, where.h + kCursorBoundsSize), where.v);
        Point right_b = Point(clip_rect.right - 1, where.v);

        if (top_a.h >= viewport.left) {
            VideoDriver::driver()->draw_line(top_a, top_b, color);
            VideoDriver::driver()->draw_line(bottom_a, bottom_b, color);
        }
        VideoDriver::driver()->draw_line(right_a, right_b, color);
        if (left_b.h >= viewport.left) {
            VideoDriver::driver()->draw_line(left_a, left_b, color);
        }
    }

    if (where.h < viewport.left) {
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

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

#ifndef ANTARES_GAME_CURSOR_HPP_
#define ANTARES_GAME_CURSOR_HPP_

#include <sfz/sfz.hpp>

#include "drawing/color.hpp"
#include "drawing/pix-table.hpp"
#include "math/geometry.hpp"
#include "math/units.hpp"
#include "ui/event.hpp"

namespace antares {

class NatePixTable;

class Cursor {
  public:
    Cursor();
    Cursor(const Cursor&) = delete;

    void draw() const;
    void draw_at(Point where) const;

  private:
    NatePixTable _sprite;
};

class GameCursor : public Cursor, public EventReceiver {
  public:
    GameCursor();
    GameCursor(const GameCursor&) = delete;

    bool show;

    bool         active() const;
    void         draw() const;
    static Point clamped_location();

    virtual void mouse_down(const MouseDownEvent& event);
    virtual void mouse_up(const MouseUpEvent& event);
    virtual void mouse_move(const MouseMoveEvent& event);

  private:
    static Point clamp(Point p);
    void wake();

    wall_time _show_crosshairs_until;
};

class HintLine {
  public:
    static void show(Point fromWhere, Point toWhere, uint8_t color, uint8_t brightness);
    static void hide();
    static void reset();
    static void draw();

  private:
    static bool     show_hint_line;
    static Point    hint_line_start;
    static Point    hint_line_end;
    static RgbColor hint_line_color;
    static RgbColor hint_line_color_dark;
};

}  // namespace antares

#endif  // ANTARES_GAME_CURSOR_HPP_

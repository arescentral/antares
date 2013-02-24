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

#ifndef ANTARES_GAME_CURSOR_HPP_
#define ANTARES_GAME_CURSOR_HPP_

#include <sfz/sfz.hpp>

#include "drawing/color.hpp"
#include "math/geometry.hpp"

namespace antares {

class NatePixTable;

struct spriteCursorType {
    Point                   where;
    bool                    show;
    std::unique_ptr<NatePixTable> sprite;

    bool        thisShowLine;
    Point       thisLineStart;
    Point       thisLineEnd;
    RgbColor    thisLineColor;
    RgbColor    thisLineColorDark;

    spriteCursorType();
};

void InitSpriteCursor();
void CleanupSpriteCursor();
void ShowSpriteCursor();
void HideSpriteCursor();
bool SpriteCursorVisible();
void SetSpriteCursorTable(short resource_id);
void MoveSpriteCursor( Point);
void ShowHintLine(Point fromWhere, Point toWhere, unsigned char color, unsigned char brightness);
void HideHintLine();
void ResetHintLine();

void draw_cursor();
void draw_sprite_cursor();
void draw_hint_line();

}  // namespace antares

#endif // ANTARES_GAME_CURSOR_HPP_

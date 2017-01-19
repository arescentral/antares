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

#ifndef ANTARES_DRAWING_SHAPES_HPP_
#define ANTARES_DRAWING_SHAPES_HPP_

#include <stdint.h>

namespace antares {

class PixMap;
struct Rect;
class Rects;
class RgbColor;

void draw_triangle_up(PixMap* destPix, const RgbColor& color);
void draw_compat_plus(PixMap* destPix, const RgbColor& color);
void draw_compat_diamond(PixMap* destPix, const RgbColor& color);
void draw_vbracket(const Rects& rects, const Rect& rect, const RgbColor& color);
void draw_shaded_rect(
        const Rects& rects, Rect rect, const RgbColor& fill_color, const RgbColor& light_color,
        const RgbColor& dark_color);
void draw_shaded_rect(
        const Rects& rects, Rect rect, uint8_t hue, uint8_t fill_color, uint8_t light_color,
        uint8_t dark_color);

}  // namespace antares

#endif  // ANTARES_DRAWING_SHAPES_HPP_

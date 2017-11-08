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

#ifndef ANTARES_DRAWING_TEXT_HPP_
#define ANTARES_DRAWING_TEXT_HPP_

#include <sfz/sfz.hpp>

#include "drawing/sprite-handling.hpp"
#include "lang/casts.hpp"
#include "video/driver.hpp"

namespace antares {

class Quads;

class Font {
  public:
    Font(sfz::StringSlice name);
    ~Font();

    uint8_t char_width(sfz::Rune mchar) const;
    int32_t string_width(sfz::StringSlice s) const;

    void draw(Point cursor, sfz::StringSlice string, RgbColor color) const;
    void draw(const Quads& quads, Point cursor, sfz::StringSlice string, RgbColor color) const;

    Texture texture;
    int32_t logicalWidth;
    int32_t height;
    int32_t ascent;

  private:
    Rect glyph_rect(sfz::Rune r) const;

    int _scale;
    std::map<sfz::Rune, Rect> _glyphs;

    DISALLOW_COPY_AND_ASSIGN(Font);
};

}  // namespace antares

#endif  // ANTARES_DRAWING_TEXT_HPP_

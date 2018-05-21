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

#include <pn/string>

#include "drawing/sprite-handling.hpp"
#include "lang/casts.hpp"
#include "video/driver.hpp"

namespace antares {

class Quads;

class Font {
  public:
    Font();
    Font(Texture texture, int logical_width, int height, int ascent,
         std::map<pn::rune, Rect> glyphs);
    Font(const Font&) = delete;
    Font& operator=(const Font&) = delete;
    Font(Font&&)                 = default;
    Font& operator=(Font&&) = default;
    ~Font();

    uint8_t char_width(pn::rune rune) const;
    int32_t string_width(pn::string_view s) const;

    void draw(Point cursor, pn::string_view string, RgbColor color) const;
    void draw(const Quads& quads, Point cursor, pn::string_view string, RgbColor color) const;

    Texture texture;
    int32_t logicalWidth = 0;
    int32_t height       = 0;
    int32_t ascent       = 0;

  private:
    Rect glyph_rect(pn::rune rune) const;

    std::map<pn::rune, Rect> _glyphs;
};

Font font(pn::string_view name);

}  // namespace antares

#endif  // ANTARES_DRAWING_TEXT_HPP_

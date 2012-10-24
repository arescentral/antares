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

#ifndef ANTARES_DRAWING_RETRO_TEXT_HPP_
#define ANTARES_DRAWING_RETRO_TEXT_HPP_

#include <vector>
#include <sfz/sfz.hpp>

#include "drawing/color.hpp"
#include "math/geometry.hpp"

namespace antares {

class Font;
class Picture;
class PixMap;

class RetroText {
  public:
    RetroText(
            const sfz::StringSlice& text, const Font* font,
            RgbColor fore_color, RgbColor back_color);
    ~RetroText();

    void set_tab_width(int tab_width);
    void wrap_to(int width, int side_margin, int line_spacing);

    int size() const;
    int tab_width() const;
    int width() const;
    int height() const;
    int auto_width() const;

    void draw(const Rect& bounds) const;
    void draw(PixMap* pix, const Rect& bounds) const;
    void draw_char(const Rect& bounds, int index) const;
    void draw_char(PixMap* pix, const Rect& bounds, int index) const;

    void draw_cursor(const Rect& bounds, int index) const;

  private:
    enum SpecialChar {
        NONE,
        TAB,
        WORD_BREAK,
        LINE_BREAK,
        DELAY,
    };

    struct RetroChar {
        RetroChar(
                uint32_t character, SpecialChar special, const RgbColor& fore_color,
                const RgbColor& back_color);

        uint32_t character;
        SpecialChar special;
        RgbColor fore_color;
        RgbColor back_color;
        int h;
        int v;
    };

    void color_cursor(const Rect& bounds, int index, const RgbColor& color) const;
    int move_word_down(int index, int v);

    RgbColor _fore_color;
    std::vector<RetroChar> _chars;
    int _tab_width;
    int _width;
    int _height;
    int _auto_width;
    int _side_margin;
    int _line_spacing;
    const Font* const _font;

    DISALLOW_COPY_AND_ASSIGN(RetroText);
};

}  // namespace antares

#endif  // ANTARES_DRAWING_RETRO_TEXT_HPP_

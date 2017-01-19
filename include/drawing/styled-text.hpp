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

#ifndef ANTARES_DRAWING_STYLED_TEXT_HPP_
#define ANTARES_DRAWING_STYLED_TEXT_HPP_

#include <sfz/sfz.hpp>
#include <vector>

#include "drawing/color.hpp"
#include "drawing/interface.hpp"
#include "math/geometry.hpp"

namespace antares {

class Font;
class Picture;

// the inline pictType struct is for keeping track of picts included in my text boxes.
struct inlinePictType {
    Rect    bounds;
    int16_t id;
};

class StyledText {
  public:
    StyledText(const Font* font);
    ~StyledText();

    void set_fore_color(RgbColor fore_color);
    void set_back_color(RgbColor back_color);
    void set_tab_width(int tab_width);
    void set_retro_text(sfz::StringSlice text);
    void set_interface_text(sfz::StringSlice text);
    void wrap_to(int width, int side_margin, int line_spacing);

    int                                size() const;
    int                                tab_width() const;
    int                                width() const;
    int                                height() const;
    int                                auto_width() const;
    const std::vector<inlinePictType>& inline_picts() const;

    void draw(const Rect& bounds) const;
    void draw_range(const Rect& bounds, int begin, int end) const;
    void draw_char(const Rect& bounds, int index) const;

    void draw_cursor(const Rect& bounds, int index) const;

  private:
    enum SpecialChar {
        NONE,
        TAB,
        WORD_BREAK,
        LINE_BREAK,
        PICTURE,
        DELAY,
    };

    struct StyledChar {
        StyledChar(
                uint32_t character, SpecialChar special, const RgbColor& fore_color,
                const RgbColor& back_color);

        sfz::Rune   character;
        SpecialChar special;
        RgbColor    fore_color;
        RgbColor    back_color;
        int         h;
        int         v;
    };

    void color_cursor(const Rect& bounds, int index, const RgbColor& color) const;
    int move_word_down(int index, int v);

    RgbColor                    _fore_color;
    RgbColor                    _back_color;
    std::vector<StyledChar>     _chars;
    std::vector<inlinePictType> _inline_picts;
    std::vector<Texture>        _textures;
    int                         _tab_width;
    int                         _width;
    int                         _height;
    int                         _auto_width;
    int                         _side_margin;
    int                         _line_spacing;
    const Font* const           _font;

    DISALLOW_COPY_AND_ASSIGN(StyledText);
};

}  // namespace antares

#endif  // ANTARES_DRAWING_STYLED_TEXT_HPP_

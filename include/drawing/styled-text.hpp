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

#include <pn/string>
#include <vector>

#include "data/handle.hpp"
#include "drawing/color.hpp"
#include "drawing/interface.hpp"
#include "math/geometry.hpp"
#include "video/driver.hpp"

namespace antares {

class Font;
class Picture;

// the inline pictType struct is for keeping track of picts included in my text boxes.
struct inlinePictType {
    Rect              bounds;
    pn::string        picture;
    const BaseObject* object;  // May be null.
};

class StyledText {
  public:
    StyledText();
    StyledText(const StyledText&) = delete;
    StyledText(StyledText&&)      = default;
    StyledText& operator=(const StyledText&) = delete;
    StyledText& operator=(StyledText&&) = default;
    ~StyledText();

    void set_plain_text(
            pn::string_view text, RgbColor fore_color = RgbColor::white(),
            RgbColor back_color = RgbColor::black());
    void set_retro_text(
            pn::string_view text, RgbColor fore_color = RgbColor::white(),
            RgbColor back_color = RgbColor::black());
    void set_interface_text(
            pn::string_view text, RgbColor fore_color = RgbColor::white(),
            RgbColor back_color = RgbColor::black());
    void select(int from, int to);
    void wrap_to(
            const Font& font, int width, int side_margin, int line_spacing, int tab_width = 0);
    void clear();

    bool                               empty() const;
    int                                size() const;
    int                                width() const;
    int                                height() const;
    int                                auto_width() const;
    const std::vector<inlinePictType>& inline_picts() const;

    void draw(const Rect& bounds) const;
    void draw_range(const Rect& bounds, int begin, int end) const;
    void draw_char(const Rect& bounds, int index) const;

    void draw_cursor(const Rect& bounds, int index, const RgbColor& color) const;

  private:
    enum SpecialChar {
        NONE,
        TAB,
        WORD_BREAK,
        LINE_BREAK,
        NO_BREAK,
        PICTURE,
        DELAY,
    };

    struct StyledChar {
        StyledChar(
                uint32_t character, SpecialChar special, const RgbColor& fore_color,
                const RgbColor& back_color);

        pn::rune    character;
        SpecialChar special;
        RgbColor    fore_color;
        RgbColor    back_color;
        Rect        bounds;
    };

    int  move_word_down(int index, int v);
    bool is_selected(int index) const;

    std::vector<StyledChar>     _chars;
    std::vector<inlinePictType> _inline_picts;
    std::vector<Texture>        _textures;
    int                         _width;
    int                         _height;
    int                         _auto_width;
    int                         _side_margin;
    int                         _line_spacing;
    int                         _select_begin = -1;
    int                         _select_end   = -1;
    const Font*                 _font;
};

}  // namespace antares

#endif  // ANTARES_DRAWING_STYLED_TEXT_HPP_

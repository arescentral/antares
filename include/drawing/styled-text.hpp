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

#include <map>
#include <pn/string>
#include <utility>
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

struct WrapMetrics {
    const Font* font;
    int         width;
    int         side_margin;
    int         line_spacing;
    int         tab_width;

    WrapMetrics(
            const Font& font, int width = std::numeric_limits<int>::max(), int side_margin = 0,
            int line_spacing = 0, int tab_width = 0)
            : font{&font},
              width{width},
              side_margin{side_margin},
              line_spacing{line_spacing},
              tab_width{tab_width} {}
};

class StyledText {
  public:
    StyledText();
    StyledText(const StyledText&) = delete;
    StyledText(StyledText&&)      = default;
    StyledText& operator=(const StyledText&) = delete;
    StyledText& operator=(StyledText&&) = default;
    ~StyledText();

    static StyledText plain(
            pn::string_view text, WrapMetrics metrics, RgbColor fore_color = RgbColor::white(),
            RgbColor back_color = RgbColor::black());
    static StyledText retro(
            pn::string_view text, WrapMetrics metrics, RgbColor fore_color = RgbColor::white(),
            RgbColor back_color = RgbColor::black());
    static StyledText interface(
            pn::string_view text, WrapMetrics metrics, RgbColor fore_color = RgbColor::white(),
            RgbColor back_color = RgbColor::black());

    bool                               empty() const;
    int                                height() const;
    int                                auto_width() const;
    const std::vector<inlinePictType>& inline_picts() const;

    void hide();
    void advance();
    bool done() const;

    pn::string_view     text() const;
    void                select(int from, int to);
    std::pair<int, int> selection() const;
    void                mark(int from, int to);
    std::pair<int, int> mark() const;

    void draw(const Rect& bounds) const;

    void draw_cursor(const Rect& bounds, const RgbColor& color, bool ends = true) const;

    int offset(int origin, TextReceiver::Offset offset, TextReceiver::OffsetUnit unit) const;

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
                SpecialChar special, int pict_index, const RgbColor& fore_color,
                const RgbColor& back_color);

        SpecialChar special;
        int         pict_index;
        RgbColor    fore_color;
        RgbColor    back_color;
        Rect        bounds;
    };

    void rewrap();
    int  move_word_down(std::map<pn::string::iterator, StyledChar>::iterator it, int v);
    bool is_selected(std::map<pn::string::iterator, StyledChar>::const_iterator it) const;

    bool is_line_start(
            pn::string::iterator begin, pn::string::iterator end, pn::string::iterator it) const;
    bool is_line_end(
            pn::string::iterator begin, pn::string::iterator end, pn::string::iterator it) const;
    bool is_start(
            pn::string::iterator begin, pn::string::iterator end, pn::string::iterator it,
            TextReceiver::OffsetUnit unit) const;
    bool is_end(
            pn::string::iterator begin, pn::string::iterator end, pn::string::iterator it,
            TextReceiver::OffsetUnit unit) const;
    pn::string::iterator line_up(pn::string::iterator it) const;
    pn::string::iterator line_down(pn::string::iterator it) const;

    pn::string                                                  _text;
    std::unique_ptr<std::map<pn::string::iterator, StyledChar>> _chars;
    std::vector<inlinePictType>                                 _inline_picts;
    std::vector<Texture>                                        _textures;
    WrapMetrics                                                 _wrap_metrics;
    std::map<pn::string::iterator, StyledChar>::const_iterator  _until;
    Size                                                        _auto_size;
    std::pair<int, int>                                         _selection = {-1, -1};
    std::pair<int, int>                                         _mark      = {-1, -1};
};

}  // namespace antares

#endif  // ANTARES_DRAWING_STYLED_TEXT_HPP_

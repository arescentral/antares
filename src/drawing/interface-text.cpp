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

#include "drawing/interface-text.hpp"

#include <algorithm>
#include <limits>
#include <sfz/sfz.hpp>

#include "data/picture.hpp"
#include "drawing/color.hpp"
#include "drawing/text.hpp"
#include "video/driver.hpp"

using sfz::Bytes;
using sfz::Exception;
using sfz::String;
using sfz::StringSlice;
using sfz::format;
using sfz::scoped_ptr;
using sfz::string_to_int;

namespace antares {
namespace interface {

StyledText::StyledText(
        const sfz::StringSlice& text, const Font* font,
        RgbColor fore_color, RgbColor back_color):
        _tab_width(0),
        _font(font) {
    for (size_t i = 0; i < text.size(); ++i) {
        switch (text.at(i)) {
          case '\r':
            _chars.push_back(StyledChar('\r', LINE_BREAK, fore_color, back_color));
            break;

          case ' ':
            _chars.push_back(StyledChar(' ', WORD_BREAK, fore_color, back_color));
            break;

          case '^':
            {
                bool found_code = false;
                if (i + 1 >= text.size()) {
                    throw Exception(format("not enough input for inline code."));
                }
                if ((text.at(i + 1) != 'P') && (text.at(i + 1) != 'p')) {
                    throw Exception(format("found bad inline pict code {0}", text.at(i)));
                }
                String id_string;
                for (size_t j = i + 2; j < text.size(); ++j) {
                    if (text.at(j) == '^') {
                        inlinePictType inline_pict;
                        int32_t id;
                        if (!string_to_int(id_string, id, 10)) {
                            throw Exception(format("invalid numeric literal {0}", id_string));
                        }
                        // TODO(sfiera): save the picture somewhere so we only have to load it once
                        // here, and not additionally each time we draw it.
                        inline_pict.id = id;
                        // TODO(sfiera): report an error if the picture is not loadable, instead of
                        // silently ignoring it.
                        try {
                            inline_pict.bounds = Picture(id).size().as_rect();
                            _inline_picts.push_back(inline_pict);
                            _chars.push_back(StyledChar(
                                        _inline_picts.size() - 1, PICTURE, fore_color,
                                        back_color));
                        } catch (sfz::Exception& e) { }
                        found_code = true;
                        i = j;
                        break;
                    }
                    id_string.push(1, text.at(j));
                }
                if (!found_code) {
                    throw Exception(format("malformed inline code"));
                }
            }
            break;

          default:
            _chars.push_back(StyledChar(text.at(i), NONE, fore_color, back_color));
            break;
        }
    }
    _chars.push_back(StyledChar('\r', LINE_BREAK, fore_color, back_color));

    wrap_to(std::numeric_limits<int>::max(), 0, 0);
}

StyledText::~StyledText() {
}

void StyledText::set_tab_width(int tab_width) {
    _tab_width = tab_width;
}

void StyledText::wrap_to(int width, int side_margin, int line_spacing) {
    _width = width;
    _side_margin = side_margin;
    _line_spacing = line_spacing;
    _auto_width = 0;
    int h = _side_margin;
    int v = 0;

    int wrap_distance = width - side_margin;

    for (size_t i = 0; i < _chars.size(); ++i) {
        _chars[i].h = h;
        _chars[i].v = v;
        switch (_chars[i].special) {
          case NONE:
            h += _font->char_width(_chars[i].character);
            if (h >= wrap_distance) {
                v += _font->height + _line_spacing;
                h = move_word_down(i, v);
            }
            _auto_width = std::max(_auto_width, h);
            break;

          case TAB:
            h += tab_width() - (h % tab_width());
            _auto_width = std::max(_auto_width, h);
            break;

          case LINE_BREAK:
            h = _side_margin;
            v += _font->height + _line_spacing;
            break;

          case WORD_BREAK:
            h += _font->char_width(_chars[i].character);
            break;

          case PICTURE:
            {
                inlinePictType* pict = &_inline_picts[_chars[i].character];
                if (h != _side_margin) {
                    v += _font->height + _line_spacing;
                }
                h = _side_margin;
                pict->bounds.offset(0, v - pict->bounds.top);
                v += pict->bounds.height() + _line_spacing + 3;
                if (_chars[i + 1].special == LINE_BREAK) {
                    v -= (_font->height + _line_spacing);
                }
            }
            break;
        }
    }
    _height = v;
}

int StyledText::size() const {
    return _chars.size();
}

int StyledText::tab_width() const {
    if (_tab_width > 0) {
        return _tab_width;
    } else {
        return _width / 2;
    }
}

int StyledText::width() const {
    return _width;
}

int StyledText::height() const {
    return _height;
}

int StyledText::auto_width() const {
    return _auto_width;
}

const std::vector<inlinePictType>& StyledText::inline_picts() const {
    return _inline_picts;
}

void StyledText::draw(const Rect& bounds) const {
    for (size_t i = 0; i < _chars.size(); ++i) {
        draw_char(bounds, i);
    }
}

void StyledText::draw(PixMap* pix, const Rect& bounds) const {
    for (size_t i = 0; i < _chars.size(); ++i) {
        draw_char(pix, bounds, i);
    }
}

void StyledText::draw_char(const Rect& bounds, int index) const {
    const int line_height = _font->height + _line_spacing;
    const int char_adjust = _font->ascent;
    const StyledChar& ch = _chars[index];
    Point corner(bounds.left, bounds.top);

    switch (ch.special) {
      case NONE:
      case WORD_BREAK:
        {
            corner.offset(ch.h, ch.v);
            if (ch.back_color != RgbColor::kBlack) {
                Rect char_rect(0, 0, _font->char_width(ch.character), line_height);
                char_rect.offset(corner.h, corner.v);
                VideoDriver::driver()->fill_rect(char_rect, ch.back_color);
            }
            String str(1, ch.character);
            _font->draw_sprite(Point(corner.h, corner.v + char_adjust), str, ch.fore_color);
        }
        break;

      case TAB:
        if (ch.back_color != RgbColor::kBlack) {
            Rect tab_rect(0, 0, tab_width() - (ch.h % tab_width()), line_height);
            tab_rect.offset(corner.h, corner.v);
            VideoDriver::driver()->fill_rect(tab_rect, ch.back_color);
        }
        break;

      case PICTURE:
        {
            const inlinePictType& inline_pict = _inline_picts[ch.character];
            corner.offset(inline_pict.bounds.left, inline_pict.bounds.top);
            Picture pict(inline_pict.id);
            scoped_ptr<Sprite> sprite(VideoDriver::driver()->new_sprite(
                        format("/pict/{0}"), pict));
            sprite->draw(corner.h, corner.v);
        }
        break;

      case LINE_BREAK:
        corner.offset(ch.h, ch.v);
        if (ch.back_color != RgbColor::kBlack) {
            Rect line_rect(0, 0, bounds.width() - ch.h, line_height);
            line_rect.offset(corner.h, corner.v);
            VideoDriver::driver()->fill_rect(line_rect, ch.back_color);
        }
        break;
    }
}

void StyledText::draw_char(PixMap* pix, const Rect& bounds, int index) const {
    const int line_height = _font->height + _line_spacing;
    const int char_adjust = _font->ascent;
    const StyledChar& ch = _chars[index];
    Point corner(bounds.left + ch.h, bounds.top + ch.v);

    switch (ch.special) {
      case NONE:
      case WORD_BREAK:
        {
            if (ch.back_color != RgbColor::kBlack) {
                Rect char_rect(0, 0, _font->char_width(ch.character), line_height);
                char_rect.offset(corner.h, corner.v);
                pix->view(char_rect).fill(ch.back_color);
            }
            String str(1, ch.character);
            _font->draw(Point(corner.h, corner.v + char_adjust), str, ch.fore_color, pix, bounds);
        }
        break;

      case TAB:
        if (ch.back_color != RgbColor::kBlack) {
            Rect tab_rect(0, 0, tab_width() - (ch.h % tab_width()), line_height);
            tab_rect.offset(corner.h, corner.v);
            pix->view(tab_rect).fill(ch.back_color);
        }
        break;

      case PICTURE:
        {
            const inlinePictType& inline_pict = _inline_picts[ch.character];
            Rect pict_bounds = inline_pict.bounds;
            pict_bounds.offset(bounds.left, bounds.top);
            Picture pict(inline_pict.id);
            pix->view(pict_bounds).copy(pict);
        }
        break;

      case LINE_BREAK:
        if (ch.back_color != RgbColor::kBlack) {
            Rect line_rect(0, 0, bounds.width() - ch.h, line_height);
            line_rect.offset(corner.h, corner.v);
            pix->view(line_rect).fill(ch.back_color);
        }
        break;
    }
}

int StyledText::move_word_down(int index, int v) {
    for (int i = index; i >= 0; --i) {
        switch (_chars[i].special) {
          case LINE_BREAK:
          case PICTURE:
            return _side_margin;

          case WORD_BREAK:
          case TAB:
            {
                if (_chars[i + 1].h <= _side_margin) {
                    return _side_margin;
                }

                int h = _side_margin;
                for (int j = i + 1; j <= index; ++j) {
                    _chars[j].h = h;
                    _chars[j].v = v;
                    h += _font->char_width(_chars[j].character);
                }
                return h;
            }

          case NONE:
            break;
        }
    }
    return _side_margin;
}

StyledText::StyledChar::StyledChar(
        uint32_t character, SpecialChar special, const RgbColor& fore_color,
        const RgbColor& back_color):
        character(character),
        special(special),
        fore_color(fore_color),
        back_color(back_color),
        h(0),
        v(0) { }

}  // namespace interface
}  // namespace antares

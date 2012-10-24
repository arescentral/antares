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

#include "drawing/retro-text.hpp"

#include <algorithm>
#include <limits>
#include <sfz/sfz.hpp>

#include "drawing/color.hpp"
#include "drawing/text.hpp"
#include "video/driver.hpp"

using sfz::Bytes;
using sfz::Exception;
using sfz::String;
using sfz::StringSlice;
using sfz::format;

namespace antares {

namespace {

int hex_digit(uint32_t c) {
    if ('0' <= c && c <= '9') {
        return c - '0';
    } else if ('A' <= c && c <= 'Z') {
        return c - 'A' + 10;
    } else if ('a' <= c && c <= 'z') {
        return c - 'a' + 10;
    }
    throw Exception(format("{0} is not a valid hex digit", c));
}

}  // namespace

RetroText::RetroText(
        const StringSlice& text, const Font* font,
        RgbColor fore_color, RgbColor back_color):
        _fore_color(fore_color),
        _tab_width(0),
        _font(font) {
    const RgbColor original_fore_color = fore_color;
    const RgbColor original_back_color = back_color;

    for (size_t i = 0; i < text.size(); ++i) {
        switch (text.at(i)) {
          case '\r':
            _chars.push_back(RetroChar('\r', LINE_BREAK, fore_color, back_color));
            break;

          case '_':
            // TODO(sfiera): replace use of "_" with e.g. "\_".
            _chars.push_back(RetroChar(' ', NONE, fore_color, back_color));
            break;

          case ' ':
            _chars.push_back(RetroChar(' ', WORD_BREAK, fore_color, back_color));
            break;

          case '\\':
            if (i + 1 >= text.size()) {
                throw Exception(format("not enough input for special code."));
            }
            ++i;
            switch (text.at(i)) {
              case 'i':
                std::swap(fore_color, back_color);
                _chars.push_back(RetroChar('\\', DELAY, fore_color, back_color));
                _chars.push_back(RetroChar('i', DELAY, fore_color, back_color));
                break;

              case 'f':
                if (i + 2 >= text.size()) {
                    throw Exception(format("not enough input for foreground code."));
                }
                fore_color = GetRGBTranslateColorShade(
                        hex_digit(text.at(i + 1)), hex_digit(text.at(i + 2)));
                i += 2;
                break;

              case 'b':
                if (i + 2 >= text.size()) {
                    throw Exception(format("not enough input for foreground code."));
                }
                back_color = GetRGBTranslateColorShade(
                        hex_digit(text.at(i + 1)), hex_digit(text.at(i + 2)));
                i += 2;
                break;

              case 'r':
                fore_color = original_fore_color;
                back_color = original_back_color;
                _chars.push_back(RetroChar('\\', DELAY, fore_color, back_color));
                _chars.push_back(RetroChar('r', DELAY, fore_color, back_color));
                break;

              case 't':
                _chars.push_back(RetroChar('\\', TAB, fore_color, back_color));
                break;

              case '\\':
                _chars.push_back(RetroChar('\\', NONE, fore_color, back_color));
                break;

              default:
                throw Exception(format("found bad special character {0}.", text.at(i)));
            }
            break;

          default:
            _chars.push_back(RetroChar(text.at(i), NONE, fore_color, back_color));
            break;
        }
    }
    _chars.push_back(RetroChar('\r', LINE_BREAK, fore_color, back_color));

    wrap_to(std::numeric_limits<int>::max(), 0, 0);
}

RetroText::~RetroText() {
}

void RetroText::set_tab_width(int tab_width) {
    _tab_width = tab_width;
}

void RetroText::wrap_to(int width, int side_margin, int line_spacing) {
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

          case DELAY:
            break;
        }
    }
    _height = v;
}

int RetroText::size() const {
    return _chars.size();
}

int RetroText::tab_width() const {
    if (_tab_width > 0) {
        return _tab_width;
    } else {
        return _width / 2;
    }
}

int RetroText::width() const {
    return _width;
}

int RetroText::height() const {
    return _height;
}

int RetroText::auto_width() const {
    return _auto_width;
}

void RetroText::draw(const Rect& bounds) const {
    for (size_t i = 0; i < _chars.size(); ++i) {
        draw_char(bounds, i);
    }
}

void RetroText::draw(PixMap* pix, const Rect& bounds) const {
    for (size_t i = 0; i < _chars.size(); ++i) {
        draw_char(pix, bounds, i);
    }
}

void RetroText::draw_char(const Rect& bounds, int index) const {
    const int line_height = _font->height + _line_spacing;
    const int char_adjust = _font->ascent + _line_spacing;
    const RetroChar& ch = _chars[index];
    Point corner(bounds.left + ch.h, bounds.top + ch.v);
    switch (ch.special) {
      case NONE:
      case WORD_BREAK:
        {
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

      case LINE_BREAK:
        if (ch.back_color != RgbColor::kBlack) {
            Rect line_rect(0, 0, bounds.width() - ch.h, line_height);
            line_rect.offset(corner.h, corner.v);
            VideoDriver::driver()->fill_rect(line_rect, ch.back_color);
        }
        break;

      case DELAY:
        break;
    }
}

void RetroText::draw_char(PixMap* pix, const Rect& bounds, int index) const {
    const int line_height = _font->height + _line_spacing;
    const int char_adjust = _font->ascent + _line_spacing;
    const RetroChar& ch = _chars[index];
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

      case LINE_BREAK:
        if (ch.back_color != RgbColor::kBlack) {
            Rect line_rect(0, 0, bounds.width() - ch.h, line_height);
            line_rect.offset(corner.h, corner.v);
            pix->view(line_rect).fill(ch.back_color);
        }
        break;

      case DELAY:
        break;
    }
}

void RetroText::draw_cursor(const Rect& bounds, int index) const {
    color_cursor(bounds, index, _fore_color);
}

void RetroText::color_cursor(const Rect& bounds, int index, const RgbColor& color) const {
    const int line_height = _font->height + _line_spacing;
    const RetroChar& ch = _chars[index];
    Point corner(bounds.left + ch.h, bounds.top + ch.v);
    Rect char_rect(0, 0, _font->logicalWidth, line_height);
    char_rect.offset(corner.h, corner.v);
    char_rect.clip_to(bounds);
    if ((char_rect.width() > 0) && (char_rect.height() > 0)) {
        VideoDriver::driver()->fill_rect(char_rect, color);
    }
}

int RetroText::move_word_down(int index, int v) {
    for (int i = index; i >= 0; --i) {
        switch (_chars[i].special) {
          case LINE_BREAK:
            return _side_margin;

          case WORD_BREAK:
          case TAB:
          case DELAY:
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

RetroText::RetroChar::RetroChar(
        uint32_t character, SpecialChar special, const RgbColor& fore_color,
        const RgbColor& back_color):
        character(character),
        special(special),
        fore_color(fore_color),
        back_color(back_color),
        h(0),
        v(0) { }

}  // namespace antares

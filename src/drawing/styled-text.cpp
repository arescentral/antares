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

#include "drawing/styled-text.hpp"

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
using std::unique_ptr;

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

StyledText::StyledText(const Font* font)
        : _fore_color(RgbColor::white()),
          _back_color(RgbColor::black()),
          _tab_width(0),
          _font(font) {}

StyledText::~StyledText() {}

void StyledText::set_fore_color(RgbColor fore_color) {
    _fore_color = fore_color;
}

void StyledText::set_back_color(RgbColor back_color) {
    _back_color = back_color;
}

void StyledText::set_tab_width(int tab_width) {
    _tab_width = tab_width;
}

void StyledText::set_retro_text(sfz::StringSlice text) {
    const RgbColor original_fore_color = _fore_color;
    const RgbColor original_back_color = _back_color;
    RgbColor       fore_color          = _fore_color;
    RgbColor       back_color          = _back_color;

    for (size_t i = 0; i < text.size(); ++i) {
        switch (text.at(i)) {
            case '\n':
                _chars.push_back(StyledChar('\n', LINE_BREAK, fore_color, back_color));
                break;

            case '_':
                // TODO(sfiera): replace use of "_" with e.g. "\_".
                _chars.push_back(StyledChar(' ', NONE, fore_color, back_color));
                break;

            case ' ': _chars.push_back(StyledChar(' ', WORD_BREAK, fore_color, back_color)); break;

            case '\\':
                if (i + 1 >= text.size()) {
                    throw Exception(format("not enough input for special code."));
                }
                ++i;
                switch (text.at(i)) {
                    case 'i':
                        std::swap(fore_color, back_color);
                        _chars.push_back(StyledChar('\\', DELAY, fore_color, back_color));
                        _chars.push_back(StyledChar('i', DELAY, fore_color, back_color));
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
                        _chars.push_back(StyledChar('\\', DELAY, fore_color, back_color));
                        _chars.push_back(StyledChar('r', DELAY, fore_color, back_color));
                        break;

                    case 't':
                        _chars.push_back(StyledChar('\\', TAB, fore_color, back_color));
                        break;

                    case '\\':
                        _chars.push_back(StyledChar('\\', NONE, fore_color, back_color));
                        break;

                    default:
                        throw Exception(format("found bad special character {0}.", text.at(i)));
                }
                break;

            default: _chars.push_back(StyledChar(text.at(i), NONE, fore_color, back_color)); break;
        }
    }
    _chars.push_back(StyledChar('\n', LINE_BREAK, fore_color, back_color));

    wrap_to(std::numeric_limits<int>::max(), 0, 0);
}

void StyledText::set_interface_text(sfz::StringSlice text) {
    for (size_t i = 0; i < text.size(); ++i) {
        switch (text.at(i)) {
            case '\n':
                _chars.push_back(StyledChar('\n', LINE_BREAK, _fore_color, _back_color));
                break;

            case ' ':
                _chars.push_back(StyledChar(' ', WORD_BREAK, _fore_color, _back_color));
                break;

            case '^': {
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
                        int32_t        id;
                        if (!string_to_int(id_string, id, 10)) {
                            throw Exception(format("invalid numeric literal {0}", id_string));
                        }
                        inline_pict.id = id;
                        // TODO(sfiera): report an error if the picture is not loadable, instead of
                        // silently ignoring it.
                        try {
                            Picture pict(id);
                            inline_pict.bounds = pict.size().as_rect();
                            _inline_picts.push_back(inline_pict);
                            _textures.push_back(pict.texture());
                            _chars.push_back(StyledChar(
                                    _inline_picts.size() - 1, PICTURE, _fore_color, _back_color));
                        } catch (sfz::Exception& e) {
                        }
                        found_code = true;
                        i          = j;
                        break;
                    }
                    id_string.push(1, text.at(j));
                }
                if (!found_code) {
                    throw Exception(format("malformed inline code"));
                }
            } break;

            default:
                _chars.push_back(StyledChar(text.at(i), NONE, _fore_color, _back_color));
                break;
        }
    }
    _chars.push_back(StyledChar('\n', LINE_BREAK, _fore_color, _back_color));

    wrap_to(std::numeric_limits<int>::max(), 0, 0);
}

void StyledText::wrap_to(int width, int side_margin, int line_spacing) {
    _width        = width;
    _side_margin  = side_margin;
    _line_spacing = line_spacing;
    _auto_width   = 0;
    int h         = _side_margin;
    int v         = 0;

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

            case WORD_BREAK: h += _font->char_width(_chars[i].character); break;

            case PICTURE: {
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
            } break;

            case DELAY: break;
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
    draw_range(bounds, 0, _chars.size());
}

void StyledText::draw_range(const Rect& bounds, int begin, int end) const {
    const int line_height = _font->height + _line_spacing;
    const int char_adjust = _font->ascent + _line_spacing;
    {
        Rects rects;
        for (size_t i = begin; i < end; ++i) {
            const StyledChar& ch     = _chars[i];
            Point             corner = bounds.origin();

            switch (ch.special) {
                case NONE:
                case WORD_BREAK:
                    corner.offset(ch.h, ch.v);
                    if (ch.back_color != RgbColor::black()) {
                        Rect char_rect(0, 0, _font->char_width(ch.character), line_height);
                        char_rect.offset(corner.h, corner.v);
                        rects.fill(char_rect, ch.back_color);
                    }
                    break;

                case TAB:
                    corner.offset(ch.h, ch.v);
                    if (ch.back_color != RgbColor::black()) {
                        Rect tab_rect(0, 0, tab_width() - (ch.h % tab_width()), line_height);
                        tab_rect.offset(corner.h, corner.v);
                        rects.fill(tab_rect, ch.back_color);
                    }
                    break;

                case LINE_BREAK:
                    corner.offset(ch.h, ch.v);
                    if (ch.back_color != RgbColor::black()) {
                        Rect line_rect(0, 0, bounds.width() - ch.h, line_height);
                        line_rect.offset(corner.h, corner.v);
                        rects.fill(line_rect, ch.back_color);
                    }
                    break;

                default: break;
            }
        }
    }

    {
        Quads quads(_font->texture);
        for (size_t i = begin; i < end; ++i) {
            const StyledChar& ch = _chars[i];
            if (ch.special == NONE) {
                _font->draw(
                        quads, Point(bounds.left + ch.h, bounds.top + ch.v + char_adjust),
                        String(1, ch.character), ch.fore_color);
            }
        }
    }

    for (size_t i = begin; i < end; ++i) {
        const StyledChar& ch     = _chars[i];
        Point             corner = bounds.origin();
        if (ch.special == PICTURE) {
            const inlinePictType& inline_pict = _inline_picts[ch.character];
            const Texture&        texture     = _textures[_chars[i].character];
            corner.offset(inline_pict.bounds.left, inline_pict.bounds.top + _line_spacing);
            texture.draw(corner.h, corner.v);
        }
    }
}

void StyledText::draw_char(const Rect& bounds, int index) const {
    draw_range(bounds, index, index + 1);
}

void StyledText::draw_cursor(const Rect& bounds, int index) const {
    color_cursor(bounds, index, _fore_color);
}

void StyledText::color_cursor(const Rect& bounds, int index, const RgbColor& color) const {
    const int         line_height = _font->height + _line_spacing;
    const StyledChar& ch          = _chars[index];
    Point             corner(bounds.left + ch.h, bounds.top + ch.v);
    Rect              char_rect(0, 0, _font->logicalWidth, line_height);
    char_rect.offset(corner.h, corner.v);
    char_rect.clip_to(bounds);
    if ((char_rect.width() > 0) && (char_rect.height() > 0)) {
        Rects().fill(char_rect, color);
    }
}

int StyledText::move_word_down(int index, int v) {
    for (int i = index; i >= 0; --i) {
        switch (_chars[i].special) {
            case LINE_BREAK:
            case PICTURE: return _side_margin;

            case WORD_BREAK:
            case TAB:
            case DELAY: {
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

            case NONE: break;
        }
    }
    return _side_margin;
}

StyledText::StyledChar::StyledChar(
        uint32_t character, SpecialChar special, const RgbColor& fore_color,
        const RgbColor& back_color)
        : character(character),
          special(special),
          fore_color(fore_color),
          back_color(back_color),
          h(0),
          v(0) {}

}  // namespace antares

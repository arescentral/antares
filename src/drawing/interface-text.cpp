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

namespace {

int char_width(uint32_t ch) {
    uint8_t w;
    mDirectCharWidth(w, ch);
    return w;
}

const directTextType* font_for_style(interfaceStyleType style) {
    switch (style) {
      case kLarge:
        return button_font;
      case kSmall:
        return small_button_font;
    }
    throw Exception(format("invalid style {0}", style));
}

}  // namespace

InterfaceText::InterfaceText(
        const StringSlice& text, interfaceStyleType style, const RgbColor& color)
        : _color(color),
          _font(font_for_style(style)) {
    for (size_t i = 0; i < text.size(); ++i) {
        switch (text.at(i)) {
          case '\r':
            _chars.push_back(InterfaceChar('\r', LINE_BREAK));
            break;

          case ' ':
            _chars.push_back(InterfaceChar(' ', WORD_BREAK));
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
                            _chars.push_back(InterfaceChar(_inline_picts.size() - 1, PICTURE));
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
            _chars.push_back(InterfaceChar(text.at(i), NONE));
            break;
        }
    }
    _chars.push_back(InterfaceChar('\r', LINE_BREAK));

    wrap_to(std::numeric_limits<int>::max(), 0, 0);
}

InterfaceText::~InterfaceText() {
}

void InterfaceText::wrap_to(int width, int h_buffer, int v_buffer) {
    mSetDirectFont(_font);
    _width = width;
    _h_buffer = h_buffer;
    _v_buffer = v_buffer;
    int h = _h_buffer;
    int v = 0;

    int wrap_distance = width - h_buffer;

    for (size_t i = 0; i < _chars.size(); ++i) {
        _chars[i].h = h;
        _chars[i].v = v;
        switch (_chars[i].special) {
          case NONE:
            h += char_width(_chars[i].character);
            if (h >= wrap_distance) {
                v += mDirectFontHeight() + _v_buffer;
                h = move_word_down(i, v);
            }
            break;

          case LINE_BREAK:
            h = _h_buffer;
            v += mDirectFontHeight() + _v_buffer;
            break;

          case WORD_BREAK:
            h += char_width(_chars[i].character);
            break;

          case PICTURE:
            {
                inlinePictType* pict = &_inline_picts[_chars[i].character];
                if (h != _h_buffer) {
                    v += mDirectFontHeight() + _v_buffer;
                }
                h = _h_buffer;
                pict->bounds.offset(0, v - pict->bounds.top);
                v += pict->bounds.height() + _v_buffer + 3;
                if (_chars[i + 1].special == LINE_BREAK) {
                    v -= (mDirectFontHeight() + _v_buffer);
                }
            }
            break;
        }
    }
    _height = v;
}

int InterfaceText::width() const {
    return _width;
}

int InterfaceText::height() const {
    return _height;
}

const std::vector<inlinePictType>& InterfaceText::inline_picts() const {
    return _inline_picts;
}

void InterfaceText::draw(const Rect& bounds) const {
    mSetDirectFont(_font);
    for (size_t i = 0; i < _chars.size(); ++i) {
        draw_char(bounds, i);
    }
}

void InterfaceText::draw(PixMap* pix, const Rect& bounds) const {
    mSetDirectFont(_font);
    for (size_t i = 0; i < _chars.size(); ++i) {
        draw_char(pix, bounds, i);
    }
}

void InterfaceText::draw_char(const Rect& bounds, int index) const {
    const int char_adjust = mDirectFontAscent();
    const InterfaceChar& ch = _chars[index];
    Point corner(bounds.left, bounds.top);

    switch (ch.special) {
      case NONE:
      case WORD_BREAK:
        {
            corner.offset(ch.h, ch.v);
            String str(1, ch.character);
            gDirectText->draw_sprite(Point(corner.h, corner.v + char_adjust), str, _color);
        }
        break;

      case LINE_BREAK:
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
    }
}

void InterfaceText::draw_char(PixMap* pix, const Rect& bounds, int index) const {
    const int char_adjust = mDirectFontAscent();
    const InterfaceChar& ch = _chars[index];
    Point corner(bounds.left + ch.h, bounds.top + ch.v);

    switch (ch.special) {
      case NONE:
      case WORD_BREAK:
        {
            String str(1, ch.character);
            DrawDirectTextStringClipped(
                    Point(corner.h, corner.v + char_adjust), str, _color, pix, bounds);
        }
        break;

      case LINE_BREAK:
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
    }
}

int InterfaceText::move_word_down(int index, int v) {
    for (int i = index; i >= 0; --i) {
        switch (_chars[i].special) {
          case LINE_BREAK:
          case PICTURE:
            return _h_buffer;

          case WORD_BREAK:
            {
                if (_chars[i + 1].h <= _h_buffer) {
                    return _h_buffer;
                }

                int h = _h_buffer;
                for (int j = i + 1; j <= index; ++j) {
                    _chars[j].h = h;
                    _chars[j].v = v;
                    h += char_width(_chars[j].character);
                }
                return h;
            }

          case NONE:
            break;
        }
    }
    return _h_buffer;
}

InterfaceText::InterfaceChar::InterfaceChar(uint32_t character, SpecialChar special)
        : character(character),
          special(special),
          h(0),
          v(0) { }

}  // namespace antares

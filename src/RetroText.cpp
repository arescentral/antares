// Ares, a tactical space combat game.
// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// 
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

#include "RetroText.hpp"

#include <assert.h>
#include <algorithm>
#include <limits>
#include "Quickdraw.h"
#include "ColorTranslation.hpp"
#include "DirectText.hpp"
#include "Error.hpp"

namespace antares {

namespace {

int char_width(uint8_t ch) {
    uint8_t w;
    unsigned char* p;
    mDirectCharWidth(w, ch, p);
    return w;
}

int hex_digit(char c) {
    assert(isxdigit(c));
    if (c >= 'a') {
        return c - 'a' + 10;
    } else if (c >= 'A') {
        return c - 'A' + 10;
    } else {
        return c - '0';
    }
}

}  // namespace

RetroText::RetroText(const char* data, size_t len, int font, uint8_t fore_color, uint8_t back_color)
        : _font(font) {
    const uint8_t original_fore_color = fore_color;
    const uint8_t original_back_color = back_color;
    transColorType* trans_color;

    for (size_t i = 0; i < len; ++i) {
        switch (data[i]) {
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
            if (i >= len) {
                fail("not enough input for special code.");
            }
            ++i;
            switch (data[i]) {
              case 'i':
                std::swap(fore_color, back_color);
                break;

              case 'f':
                if (i + 2 >= len) {
                    fail("not enough input for foreground code.");
                }
                mGetTranslateColorShade(
                        hex_digit(data[i + 1]), hex_digit(data[i + 2]), fore_color, trans_color);
                i += 2;
                break;

              case 'b':
                if (i + 2 >= len) {
                    fail("not enough input for foreground code.");
                }
                mGetTranslateColorShade(
                        hex_digit(data[i + 1]), hex_digit(data[i + 2]), back_color, trans_color);
                i += 2;
                break;

              case 'r':
                fore_color = original_fore_color;
                back_color = original_back_color;
                break;

              case 't':
                _chars.push_back(RetroChar('\t', TAB, fore_color, back_color));
                break;

              case '\\':
                _chars.push_back(RetroChar('\\', NONE, fore_color, back_color));
                break;

              default:
                fail("found bad special character '%c'.", data[i]);
            }
            break;

          default:
            _chars.push_back(RetroChar(data[i], NONE, fore_color, back_color));
            break;
        }
    }

    wrap_to(std::numeric_limits<int>::max(), 0);
}

RetroText::~RetroText() {
}

void RetroText::wrap_to(int width, int line_spacing) {
    mSetDirectFont(_font);
    _width = width;
    _line_spacing = line_spacing;
    _auto_width = 0;
    int h = 0;
    int v = 0;

    for (size_t i = 0; i < _chars.size(); ++i) {
        _chars[i].h = h;
        _chars[i].v = v;
        switch (_chars[i].special) {
          case NONE:
            h += char_width(_chars[i].character);
            if (h > _width) {
                v += mDirectFontHeight() + _line_spacing;
                h = move_word_down(i, v);
            }
            _auto_width = std::max(_auto_width, h);
            break;

          case TAB:
            if (h >= _width / 2) {
                // If we're already at or past the midpoint of the line, shift a line down.
                v += mDirectFontHeight() + _line_spacing;
            }
            h = _width / 2;
            break;

          case LINE_BREAK:
            h = 0;
            v += mDirectFontHeight() + _line_spacing;
            break;

          case WORD_BREAK:
            h += char_width(_chars[i].character);
            break;
        }
    }
    _height = v + mDirectFontHeight() + _line_spacing;
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

void RetroText::draw(PixMap* pix, const Rect& bounds) const {
    mSetDirectFont(_font);
    const int line_height = mDirectFontHeight() + _line_spacing;
    const int char_adjust = mDirectFontAscent() + _line_spacing;
    for (std::vector<RetroChar>::const_iterator it = _chars.begin(); it != _chars.end(); ++it) {
        Point corner(bounds.left + it->h, bounds.top + it->v);
        switch (it->special) {
          case NONE:
          case WORD_BREAK:
            {
                if (it->back_color != 0xFF) {
                    Rect char_rect(0, 0, char_width(it->character), line_height);
                    char_rect.offset(corner.h, corner.v);
                    DrawNateRect(pix, &char_rect, 0, 0, it->back_color);
                }
                MoveTo(corner.h, corner.v + char_adjust);
                unsigned char pstr[2] = {1, it->character};
                DrawDirectTextStringClipped(pstr, it->fore_color, pix, bounds, 0, 0);
            }
            break;

          case TAB:
            if (it->back_color != 0xFF) {
                if (it->h >= _width) {
                    // If the tab starts at or past the midpoint of the line, first draw one box to
                    // the end of the line.
                    Rect line_rect(0, 0, bounds.width() - it->h, line_height);
                    line_rect.offset(corner.h, corner.v);
                    DrawNateRect(pix, &line_rect, 0, 0, it->back_color);

                    // Then draw a box from the start of the next line to its midpoint.
                    Rect tab_rect(0, 0, bounds.width() / 2, line_height);
                    tab_rect.offset(bounds.left, corner.v + line_height);
                    DrawNateRect(pix, &tab_rect, 0, 0, it->back_color);
                } else {
                    // Otherwise, we just have to draw a box to the midpoint of the line.
                    Rect tab_rect(0, 0, bounds.width() / 2 - it->h, line_height);
                    tab_rect.offset(corner.h, corner.v);
                    DrawNateRect(pix, &tab_rect, 0, 0, it->back_color);
                }
            }
            break;

          case LINE_BREAK:
            if (it->back_color != 0xFF) {
                Rect line_rect(0, 0, bounds.width() - it->h, line_height);
                line_rect.offset(corner.h, corner.v);
                DrawNateRect(pix, &line_rect, 0, 0, it->back_color);
            }
            break;
        }
    }
}

int RetroText::move_word_down(int index, int v) {
    for (int i = index; i >= 0; --i) {
        switch (_chars[i].special) {
          case LINE_BREAK:
            return 0;

          case WORD_BREAK:
          case TAB:
            {
                if (_chars[i + 1].h == 0) {
                    return 0;
                }

                int h = 0;
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
    return 0;
}

RetroText::RetroChar::RetroChar(
        char character, SpecialChar special, uint8_t fore_color, uint8_t back_color)
        : character(character),
          special(special),
          fore_color(fore_color),
          back_color(back_color),
          h(0),
          v(0) { }

}  // namespace antares

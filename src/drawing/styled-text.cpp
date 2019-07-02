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
#include <pn/output>

#include "data/resource.hpp"
#include "drawing/color.hpp"
#include "drawing/text.hpp"
#include "game/sys.hpp"
#include "video/driver.hpp"

using std::unique_ptr;

namespace antares {

namespace {

int hex_digit(pn::rune r) {
    int32_t c = r.value();
    if ('0' <= c && c <= '9') {
        return c - '0';
    } else if ('A' <= c && c <= 'Z') {
        return c - 'A' + 10;
    } else if ('a' <= c && c <= 'z') {
        return c - 'a' + 10;
    }
    throw std::runtime_error(pn::format("{0} is not a valid hex digit", c).c_str());
}

}  // namespace

StyledText::StyledText() : StyledText{sys.fonts.tactical} {}

StyledText::StyledText(const Font& font)
        : _fore_color(RgbColor::white()), _back_color(RgbColor::black()), _font(&font) {}

StyledText::~StyledText() {}

void StyledText::set_font(const Font& font) { _font = &font; }

void StyledText::set_fore_color(RgbColor fore_color) { _fore_color = fore_color; }

void StyledText::set_back_color(RgbColor back_color) { _back_color = back_color; }

void StyledText::set_plain_text(pn::string_view text) {
    _chars.clear();

    for (auto r : text) {
        switch (r.value()) {
            case '\n':
                _chars.push_back(StyledChar('\n', LINE_BREAK, _fore_color, _back_color));
                break;
            case ' ':
                _chars.push_back(StyledChar(' ', WORD_BREAK, _fore_color, _back_color));
                break;
            case 0xA0:
                _chars.push_back(StyledChar(0xA0, NO_BREAK, _fore_color, _back_color));
                break;
            default:
                _chars.push_back(StyledChar(r.value(), NONE, _fore_color, _back_color));
                break;
        }
    }
    if (_chars.empty() || (_chars.back().special != LINE_BREAK)) {
        _chars.push_back(StyledChar('\n', LINE_BREAK, _fore_color, _back_color));
    }
    wrap_to(std::numeric_limits<int>::max(), 0, 0);
}

void StyledText::set_retro_text(pn::string_view text) {
    _chars.clear();

    const RgbColor original_fore_color = _fore_color;
    const RgbColor original_back_color = _back_color;
    RgbColor       fore_color          = _fore_color;
    RgbColor       back_color          = _back_color;
    pn::rune       r1;

    enum { START, SLASH, FG1, FG2, BG1, BG2 } state = START;

    for (pn::rune r : text) {
        switch (state) {
            case START:
                switch (r.value()) {
                    case '\n':
                        _chars.push_back(StyledChar('\n', LINE_BREAK, fore_color, back_color));
                        break;

                    case '_':
                        // TODO(sfiera): replace use of "_" with e.g. "\_".
                        _chars.push_back(StyledChar(0xA0, NO_BREAK, fore_color, back_color));
                        break;

                    case ' ':
                        _chars.push_back(StyledChar(' ', WORD_BREAK, fore_color, back_color));
                        break;

                    case '\\': state = SLASH; break;

                    default:
                        _chars.push_back(StyledChar(r.value(), NONE, fore_color, back_color));
                        break;
                }
                break;

            case SLASH:
                switch (r.value()) {
                    case 'i':
                        std::swap(fore_color, back_color);
                        _chars.push_back(StyledChar('\\', DELAY, fore_color, back_color));
                        _chars.push_back(StyledChar('i', DELAY, fore_color, back_color));
                        state = START;
                        break;

                    case 'r':
                        fore_color = original_fore_color;
                        back_color = original_back_color;
                        _chars.push_back(StyledChar('\\', DELAY, fore_color, back_color));
                        _chars.push_back(StyledChar('r', DELAY, fore_color, back_color));
                        state = START;
                        break;

                    case 't':
                        _chars.push_back(StyledChar('\\', TAB, fore_color, back_color));
                        state = START;
                        break;

                    case '\\':
                        _chars.push_back(StyledChar('\\', NONE, fore_color, back_color));
                        state = START;
                        break;

                    case 'f': state = FG1; break;
                    case 'b': state = BG1; break;

                    default:
                        throw std::runtime_error(
                                pn::format("found bad special character {0}.", r.value()).c_str());
                }
                break;

            case FG1: r1 = r, state = FG2; break;
            case FG2:
                fore_color =
                        GetRGBTranslateColorShade(static_cast<Hue>(hex_digit(r1)), hex_digit(r));
                state = START;
                break;

            case BG1: r1 = r, state = BG2; break;
            case BG2:
                back_color =
                        GetRGBTranslateColorShade(static_cast<Hue>(hex_digit(r1)), hex_digit(r));
                state = START;
                break;
        }
    }

    if (state != START) {
        throw std::runtime_error(pn::format("not enough input for special code.").c_str());
    }

    if (_chars.empty() || (_chars.back().special != LINE_BREAK)) {
        _chars.push_back(StyledChar('\n', LINE_BREAK, fore_color, back_color));
    }

    wrap_to(std::numeric_limits<int>::max(), 0, 0);
}

void StyledText::set_interface_text(pn::string_view text) {
    _chars.clear();

    const auto f = _fore_color;
    const auto b = _back_color;
    pn::string id;
    enum { START, CODE, ID } state = START;

    for (auto r : text) {
        switch (state) {
            case START:
                switch (r.value()) {
                    case '\n': _chars.push_back(StyledChar('\n', LINE_BREAK, f, b)); break;
                    case ' ': _chars.push_back(StyledChar(' ', WORD_BREAK, f, b)); break;
                    default: _chars.push_back(StyledChar(r.value(), NONE, f, b)); break;
                    case '^': state = CODE; break;
                }
                break;

            case CODE:
                if ((r != pn::rune{'P'}) && (r != pn::rune{'p'})) {
                    throw std::runtime_error(
                            pn::format(
                                    "found bad inline pict code {0}", pn::dump(r, pn::dump_short))
                                    .c_str());
                }
                state = ID;
                break;

            case ID:
                if (r != pn::rune{'^'}) {
                    id += r;
                    continue;
                }
                inlinePictType inline_pict;
                if ((inline_pict.object = BaseObject::get(id)) &&
                    inline_pict.object->portrait.has_value()) {
                    inline_pict.picture = inline_pict.object->portrait->copy();
                } else {
                    inline_pict.picture = std::move(id);
                }

                _textures.push_back(Resource::texture(inline_pict.picture));
                inline_pict.bounds = _textures.back().size().as_rect();
                _inline_picts.emplace_back(std::move(inline_pict));
                _chars.push_back(StyledChar(_inline_picts.size() - 1, PICTURE, f, b));
                id.clear();
                state = START;
                break;
        }
    }

    if (_chars.empty() || (_chars.back().special != LINE_BREAK)) {
        _chars.push_back(StyledChar('\n', LINE_BREAK, f, b));
    }

    wrap_to(std::numeric_limits<int>::max(), 0, 0);
}

void StyledText::select(int from, int to) {
    _select_begin = from;
    _select_end   = to;
}

void StyledText::wrap_to(int width, int side_margin, int line_spacing, int tab_width) {
    _width        = width;
    _side_margin  = side_margin;
    _line_spacing = line_spacing;
    _auto_width   = 0;
    int h         = _side_margin;
    int v         = 0;

    if (tab_width <= 0) {
        tab_width = width / 2;
    }
    const int line_height   = _font->height + _line_spacing;
    const int wrap_distance = width - side_margin;

    for (size_t i = 0; i < _chars.size(); ++i) {
        _chars[i].bounds = Rect{h, v, h, v + line_height};
        switch (_chars[i].special) {
            case NONE:
            case NO_BREAK:
                h += _font->char_width(_chars[i].character);
                if (h >= wrap_distance) {
                    v += _font->height + _line_spacing;
                    h = move_word_down(i, v);
                }
                _auto_width = std::max(_auto_width, h);
                break;

            case TAB:
                h += tab_width - (h % tab_width);
                _auto_width = std::max(_auto_width, h);
                break;

            case LINE_BREAK:
                h = _side_margin;
                v += _font->height + _line_spacing;
                break;

            case WORD_BREAK: h += _font->char_width(_chars[i].character); break;

            case PICTURE: {
                inlinePictType* pict = &_inline_picts[_chars[i].character.value()];
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
        _chars[i].bounds.right = h;
    }
    _height = v;
}

void StyledText::clear() { _chars.clear(); }

bool StyledText::empty() const {
    return _chars.size() <= 1;  // Always have \n at the end.
}

int StyledText::size() const { return _chars.size(); }

int StyledText::width() const { return _width; }

int StyledText::height() const { return _height; }

int StyledText::auto_width() const { return _auto_width; }

const std::vector<inlinePictType>& StyledText::inline_picts() const { return _inline_picts; }

void StyledText::draw(const Rect& bounds) const { draw_range(bounds, 0, _chars.size()); }

void StyledText::draw_range(const Rect& bounds, int begin, int end) const {
    const Point char_adjust = {bounds.left, bounds.top + _font->ascent + _line_spacing};

    {
        Rects rects;
        for (size_t i = begin; i < end; ++i) {
            const StyledChar& ch = _chars[i];
            Rect              r  = ch.bounds;
            r.offset(bounds.left, bounds.top);
            const RgbColor color = is_selected(i) ? ch.fore_color : ch.back_color;

            switch (ch.special) {
                case NONE:
                case NO_BREAK:
                case WORD_BREAK:
                case TAB:
                    if (color == RgbColor::black()) {
                        continue;
                    }
                    break;

                case LINE_BREAK:
                    if (color == RgbColor::black()) {
                        continue;
                    }
                    r.right = bounds.right;
                    break;

                case PICTURE:
                case DELAY: continue;
            }

            rects.fill(r, color);
        }

        if ((0 <= _select_begin) && (_select_begin == _select_end) &&
            (_select_end < _chars.size())) {
            const int      i = _select_begin;
            Rect           r;
            const RgbColor color = _chars[i].fore_color;
            if ((i == 0) || (_chars[i - 1].special == LINE_BREAK)) {
                r = _chars[i].bounds;
            } else {
                r      = _chars[i - 1].bounds;
                r.left = r.right;
            }
            r.right = r.left + 1;
            r.offset(bounds.left, bounds.top);
            rects.fill(r, color);
        }
    }

    {
        Quads quads(_font->texture);

        for (size_t i = begin; i < end; ++i) {
            const StyledChar& ch = _chars[i];
            if (ch.special == NONE) {
                RgbColor color = is_selected(i) ? ch.back_color : ch.fore_color;
                Point    p = Point{ch.bounds.left + char_adjust.h, ch.bounds.top + char_adjust.v};
                _font->draw(quads, p, ch.character, color);
            }
        }
    }

    for (size_t i = begin; i < end; ++i) {
        const StyledChar& ch     = _chars[i];
        Point             corner = bounds.origin();
        if (ch.special == PICTURE) {
            const inlinePictType& inline_pict = _inline_picts[ch.character.value()];
            const Texture&        texture     = _textures[_chars[i].character.value()];
            corner.offset(inline_pict.bounds.left, inline_pict.bounds.top + _line_spacing);
            texture.draw(corner.h, corner.v);
        }
    }
}

void StyledText::draw_char(const Rect& bounds, int index) const {
    draw_range(bounds, index, index + 1);
}

void StyledText::draw_cursor(const Rect& bounds, int index, const RgbColor& color) const {
    const int         line_height = _font->height + _line_spacing;
    const StyledChar& ch          = _chars[index];
    Rect              char_rect(0, 0, _font->logicalWidth, line_height);
    char_rect.offset(bounds.left + ch.bounds.left, bounds.top + ch.bounds.top);
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
                if (_chars[i + 1].bounds.left <= _side_margin) {
                    return _side_margin;
                }

                int h = _side_margin;
                for (int j = i + 1; j <= index; ++j) {
                    _chars[j].bounds = Rect{Point{h, v}, _chars[j].bounds.size()};
                    h += _font->char_width(_chars[j].character);
                }
                return h;
            }

            case NO_BREAK:
            case NONE: break;
        }
    }
    return _side_margin;
}

bool StyledText::is_selected(int index) const {
    return (_select_begin <= index) && (index < _select_end);
}

StyledText::StyledChar::StyledChar(
        uint32_t character, SpecialChar special, const RgbColor& fore_color,
        const RgbColor& back_color)
        : character(character),
          special(special),
          fore_color(fore_color),
          back_color(back_color),
          bounds{0, 0, 0, 0} {}

}  // namespace antares

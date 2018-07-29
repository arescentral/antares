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

#include "drawing/text.hpp"

#include <algorithm>
#include <pn/file>
#include <pn/map>
#include <pn/string>
#include <pn/value>

#include "data/font-data.hpp"
#include "data/resource.hpp"
#include "drawing/color.hpp"
#include "game/globals.hpp"
#include "game/sys.hpp"
#include "lang/defines.hpp"
#include "video/driver.hpp"

namespace antares {

namespace {

enum {
    kTacticalFontResID    = 5000,
    kComputerFontResID    = 5001,
    kButtonFontResID      = 5002,
    kTitleFontResID       = 5004,
    kButtonSmallFontResID = 5005,
};

}  // namespace

Font::Font() {}

Font::Font(
        Texture texture, int logical_width, int height, int ascent,
        std::map<pn::rune, Rect> glyphs)
        : texture(std::move(texture)),
          logicalWidth(logical_width),
          height(height),
          ascent(ascent),
          _glyphs(glyphs) {}

Font font(pn::string_view name) {
    FontData d       = Resource::font(name);
    Texture  texture = Resource::font_image(name);
    return Font(std::move(texture), d.logical_width, d.height, d.ascent, d.glyphs.map);
}

Font::~Font() {}

Rect Font::glyph_rect(pn::rune rune) const {
    auto it = _glyphs.find(rune);
    if (it == _glyphs.end()) {
        return Rect();
    }
    return it->second;
}

void Font::draw(Point cursor, pn::string_view string, RgbColor color) const {
    draw(Quads(texture), cursor, string, color);
}

void Font::draw(const Quads& quads, Point cursor, pn::string_view string, RgbColor color) const {
    cursor.offset(0, -ascent);
    for (pn::rune rune : string) {
        auto glyph = glyph_rect(rune);
        quads.draw(Rect(cursor, glyph.size()), glyph, color);
        cursor.offset(glyph.width(), 0);
    }
}

uint8_t Font::char_width(pn::rune mchar) const { return glyph_rect(mchar).width(); }

int32_t Font::string_width(pn::string_view s) const {
    int32_t sum = 0;
    for (pn::rune rune : s) {
        sum += char_width(rune);
    }
    return sum;
}

}  // namespace antares

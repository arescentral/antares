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

#include "data/picture.hpp"
#include "data/pn.hpp"
#include "data/resource.hpp"
#include "drawing/color.hpp"
#include "game/globals.hpp"
#include "lang/defines.hpp"
#include "video/driver.hpp"

namespace utf8 = sfz::utf8;

namespace antares {

namespace {

enum {
    kTacticalFontResID    = 5000,
    kComputerFontResID    = 5001,
    kButtonFontResID      = 5002,
    kTitleFontResID       = 5004,
    kButtonSmallFontResID = 5005,
};

void recolor(PixMap& glyph_table) {
    for (size_t y = 0; y < glyph_table.size().height; ++y) {
        for (size_t x = 0; x < glyph_table.size().width; ++x) {
            if (glyph_table.get(x, y).red < 255) {
                glyph_table.set(x, y, RgbColor::white());
            } else {
                glyph_table.set(x, y, RgbColor::clear());
            }
        }
    }
}

}  // namespace

Font::Font(pn::string_view name) {
    pn::string path = pn::format("fonts/{0}.pn", name);
    Resource   rsrc(path);
    pn::value  x;
    pn_error_t err;
    if (!pn::parse(rsrc.string().open(), x, &err)) {
        throw std::runtime_error(
                pn::format("{0}:{1}:{2}: {3}", path, err.lineno, err.column, pn_strerror(err.code))
                        .c_str());
    }

    pn::map_cref    m     = x.as_map();
    pn::string_view image = m.get("image").as_string();
    logicalWidth          = m.get("logical-width").as_int();
    height                = m.get("height").as_int();
    ascent                = m.get("ascent").as_int();
    pn::map_cref glyphs   = m.get("glyphs").as_map();

    Picture glyph_table(image, true);
    recolor(glyph_table);
    texture = glyph_table.texture();
    _scale  = glyph_table.scale();

    for (pn::key_value_cref kv : glyphs) {
        pn::string_view glyph    = kv.key();
        pn::map_cref    rect_map = kv.value().as_map();

        _glyphs[(*glyph.begin()).value()] =
                Rect(rect_map.get("left").as_int(), rect_map.get("top").as_int(),
                     rect_map.get("right").as_int(), rect_map.get("bottom").as_int());
    }
}

Font::~Font() {}

Rect Font::glyph_rect(uint32_t rune) const {
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
        auto glyph = glyph_rect(rune.value());
        Rect scaled(
                glyph.left * _scale, glyph.top * _scale, glyph.right * _scale,
                glyph.bottom * _scale);
        quads.draw(Rect(cursor, glyph.size()), scaled, color);
        cursor.offset(glyph.width(), 0);
    }
}

uint8_t Font::char_width(uint32_t mchar) const { return glyph_rect(mchar).width(); }

int32_t Font::string_width(pn::string_view s) const {
    int32_t sum = 0;
    for (pn::rune rune : s) {
        sum += char_width(rune.value());
    }
    return sum;
}

}  // namespace antares

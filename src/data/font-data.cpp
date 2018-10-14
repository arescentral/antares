// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// Copyright (C) 2008-2018 The Antares Authors
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

#include "data/font-data.hpp"

#include "data/field.hpp"

namespace antares {

FIELD_READER(FontData::Glyphs) {
    if (x.value().is_map()) {
        FontData::Glyphs glyphs;
        for (pn::key_value_cref kv : x.value().as_map()) {
            pn::string_view glyph      = kv.key();
            path_value      rect       = x.get(glyph);
            glyphs.map[*glyph.begin()] = read_field<Rect>(rect);
        }
        return glyphs;
    } else {
        throw std::runtime_error(pn::format("{0}must be map", x.prefix()).c_str());
    }
}

FontData font_data(pn::value_cref x) {
    return required_struct<FontData>(
            path_value{x}, {{"logical-width", &FontData::logical_width},
                            {"height", &FontData::height},
                            {"ascent", &FontData::ascent},
                            {"glyphs", &FontData::glyphs}});
}

}  // namespace antares

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
#include <sfz/sfz.hpp>

#include "data/picture.hpp"
#include "data/resource.hpp"
#include "drawing/color.hpp"
#include "game/globals.hpp"
#include "lang/defines.hpp"
#include "video/driver.hpp"

using sfz::Bytes;
using sfz::BytesSlice;
using sfz::Exception;
using sfz::Json;
using sfz::JsonDefaultVisitor;
using sfz::Rune;
using sfz::String;
using sfz::StringMap;
using sfz::StringSlice;
using sfz::format;
using sfz::hex;
using sfz::read;
using sfz::string_to_json;
using std::map;
using std::unique_ptr;

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

struct FontVisitor : public JsonDefaultVisitor {
    enum StateEnum {
        NEW,
        IMAGE,
        LOGICAL_WIDTH,
        HEIGHT,
        ASCENT,
        GLYPHS,
        GLYPH,
        GLYPH_LEFT,
        GLYPH_TOP,
        GLYPH_RIGHT,
        GLYPH_BOTTOM,
        DONE,
    };
    struct State {
        StateEnum state;
        Rune      glyph;
        Rect      glyph_rect;
        State() : state(NEW) {}
    };
    State&   state;
    Texture& texture;
    int&     scale;
    int32_t& logical_width;
    int32_t& height;
    int32_t& ascent;
    map<Rune, Rect>& glyphs;

    FontVisitor(
            State& state, Texture& texture, int& scale, int32_t& logical_width, int32_t& height,
            int32_t& ascent, map<Rune, Rect>& glyphs)
            : state(state),
              texture(texture),
              scale(scale),
              logical_width(logical_width),
              height(height),
              ascent(ascent),
              glyphs(glyphs) {}

    bool descend(StateEnum state, const StringMap<Json>& value, StringSlice key) const {
        auto it = value.find(key);
        if (it == value.end()) {
            return false;
        }
        StateEnum saved_state = this->state.state;
        this->state.state     = state;
        it->second.accept(*this);
        this->state.state = saved_state;
        return true;
    }

    virtual void visit_object(const StringMap<Json>& value) const {
        switch (state.state) {
            case NEW:
                if (!descend(IMAGE, value, "image")) {
                    throw Exception("missing image in sprite json");
                }
                if (!descend(LOGICAL_WIDTH, value, "logical-width")) {
                    throw Exception("missing logical-width in sprite json");
                }
                if (!descend(HEIGHT, value, "height")) {
                    throw Exception("missing height in sprite json");
                }
                if (!descend(ASCENT, value, "ascent")) {
                    throw Exception("missing ascent in sprite json");
                }
                descend(GLYPHS, value, "glyphs");
                break;

            case GLYPHS:
                this->state.state = GLYPH;
                for (auto kv : value) {
                    state.glyph = kv.first.at(0);
                    kv.second.accept(*this);
                }
                break;

            case GLYPH:
                if (descend(GLYPH_LEFT, value, "left") && descend(GLYPH_TOP, value, "top") &&
                    descend(GLYPH_RIGHT, value, "right") &&
                    descend(GLYPH_BOTTOM, value, "bottom")) {
                    glyphs[state.glyph] = state.glyph_rect;
                } else {
                    throw Exception("bad glyph rect");
                }
                break;

            default: return visit_default("object");
        }
    }

    virtual void visit_string(const StringSlice& value) const {
        switch (state.state) {
            case IMAGE: {
                Picture glyph_table(value, true);
                recolor(glyph_table);
                texture = glyph_table.texture();
                scale   = glyph_table.scale();
                break;
            }
            default: return visit_default("string");
        }
    }

    virtual void visit_number(double value) const {
        switch (state.state) {
            case LOGICAL_WIDTH: logical_width          = value; break;
            case HEIGHT: height                        = value; break;
            case ASCENT: ascent                        = value; break;
            case GLYPH_LEFT: state.glyph_rect.left     = value; break;
            case GLYPH_TOP: state.glyph_rect.top       = value; break;
            case GLYPH_RIGHT: state.glyph_rect.right   = value; break;
            case GLYPH_BOTTOM: state.glyph_rect.bottom = value; break;
            default: return visit_default("number");
        }
    }

    virtual void visit_default(const char* type) const {
        throw Exception(format("unexpected {0} in sprite json", type));
    }
};

}  // namespace

Font::Font(StringSlice name) {
    String   path(format("fonts/{0}.json", name));
    Resource rsrc(path);
    String   rsrc_string(utf8::decode(rsrc.data()));
    Json     json;
    if (!string_to_json(rsrc_string, json)) {
        throw Exception("invalid JSON");
    }
    FontVisitor::State state;
    json.accept(FontVisitor(state, texture, _scale, logicalWidth, height, ascent, _glyphs));
}

Font::~Font() {}

Rect Font::glyph_rect(Rune r) const {
    auto it = _glyphs.find(r);
    if (it == _glyphs.end()) {
        return Rect();
    }
    return it->second;
}

void Font::draw(Point cursor, sfz::StringSlice string, RgbColor color) const {
    draw(Quads(texture), cursor, string, color);
}

void Font::draw(const Quads& quads, Point cursor, sfz::StringSlice string, RgbColor color) const {
    cursor.offset(0, -ascent);
    for (size_t i = 0; i < string.size(); ++i) {
        auto glyph = glyph_rect(string.at(i));
        Rect scaled(
                glyph.left * _scale, glyph.top * _scale, glyph.right * _scale,
                glyph.bottom * _scale);
        quads.draw(Rect(cursor, glyph.size()), scaled, color);
        cursor.offset(glyph.width(), 0);
    }
}

uint8_t Font::char_width(Rune mchar) const {
    return glyph_rect(mchar).width();
}

int32_t Font::string_width(sfz::StringSlice s) const {
    int32_t sum = 0;
    for (int i = 0; i < s.size(); ++i) {
        sum += char_width(s.at(i));
    }
    return sum;
}

}  // namespace antares

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

#include "drawing/text.hpp"

#include <algorithm>
#include <sfz/sfz.hpp>

#include "data/resource.hpp"
#include "drawing/color.hpp"
#include "game/globals.hpp"
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

namespace utf8 = sfz::utf8;

namespace antares {

namespace {

static const int kDirectFontNum = 6;

enum {
    kTacticalFontResID      = 5000,
    kComputerFontResID      = 5001,
    kButtonFontResID        = 5002,
    kTitleFontResID         = 5004,
    kButtonSmallFontResID   = 5005,
};

struct FontVisitor : public JsonDefaultVisitor {
    enum StateEnum {
        NEW,
        IMAGE,
        LOGICAL_WIDTH, HEIGHT, ASCENT,
        GLYPHS, GLYPH, GLYPH_LEFT, GLYPH_TOP, GLYPH_RIGHT, GLYPH_BOTTOM,
        DONE,
    };
    struct State {
        StateEnum state;
        Rune glyph;
        Rect glyph_rect;
        State(): state(NEW) { }
    };
    State& state;
    ArrayPixMap& image;
    int32_t& logical_width;
    int32_t& height;
    int32_t& ascent;
    map<Rune, Rect>& glyphs;

    FontVisitor(State& state, ArrayPixMap& image,
                int32_t& logical_width, int32_t& height, int32_t& ascent,
                map<Rune, Rect>& glyphs):
            state(state),
            image(image),
            logical_width(logical_width),
            height(height),
            ascent(ascent),
            glyphs(glyphs) { }

    bool descend(StateEnum state, const StringMap<Json>& value, StringSlice key) const {
        auto it = value.find(key);
        if (it == value.end()) {
            return false;
        }
        StateEnum saved_state = this->state.state;
        this->state.state = state;
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
            for (auto kv: value) {
                state.glyph = kv.first.at(0);
                kv.second.accept(*this);
            }
            break;

          case GLYPH:
            if (descend(GLYPH_LEFT, value, "left") &&
                    descend(GLYPH_TOP, value, "top") &&
                    descend(GLYPH_RIGHT, value, "right") &&
                    descend(GLYPH_BOTTOM, value, "bottom")) {
                glyphs[state.glyph] = state.glyph_rect;
            } else {
                throw Exception("bad glyph rect");
            }
            break;

          default:
            return visit_default("object");
        }
    }

    virtual void visit_string(const StringSlice& value) const {
        switch (state.state) {
          case IMAGE:
            {
                Resource rsrc(value);
                BytesSlice data = rsrc.data();
                read(data, image);
            }
            break;
          default:
            return visit_default("string");
        }
    }

    virtual void visit_number(double value) const {
        switch (state.state) {
          case LOGICAL_WIDTH: logical_width = value; break;
          case HEIGHT: height = value; break;
          case ASCENT: ascent = value; break;
          case GLYPH_LEFT: state.glyph_rect.left = value; break;
          case GLYPH_TOP: state.glyph_rect.top = value; break;
          case GLYPH_RIGHT: state.glyph_rect.right = value; break;
          case GLYPH_BOTTOM: state.glyph_rect.bottom = value; break;
          default:
            return visit_default("number");
        }
    }

    virtual void visit_default(const char* type) const {
        throw Exception(format("unexpected {0} in sprite json", type));
    }
};

}  // namespace

const Font* gDirectTextData[kDirectFontNum];
const Font* tactical_font;
const Font* computer_font;
const Font* button_font;
const Font* title_font;
const Font* small_button_font;

Font::Font(StringSlice name):
        _glyph_table(0, 0) {
    String path(format("fonts/{0}.json", name));
    Resource rsrc(path);
    String rsrc_string(utf8::decode(rsrc.data()));
    Json json;
    if (!string_to_json(rsrc_string, json)) {
        throw Exception("invalid JSON");
    }
    FontVisitor::State state;
    json.accept(FontVisitor(state, _glyph_table, logicalWidth, height, ascent, _glyphs));

    if (VideoDriver::driver()) {
        for (const auto& kv: _glyphs) {
            ArrayPixMap pix(kv.second.width(), kv.second.height());
            pix.fill(RgbColor::kClear);
            draw_internal(Point(0, ascent), kv.first, RgbColor::kWhite, &pix);
            _sprites[kv.first] = VideoDriver::driver()->new_sprite(
                    format("/fonts/{0}/{1}", name, hex(kv.first, 2)), pix);
        }
    }
}

Font::~Font() { }

Rect Font::glyph_rect(Rune r) const {
    auto it = _glyphs.find(r);
    if (it == _glyphs.end()) {
        return Rect();
    }
    return it->second;
}

void Font::draw(Point origin, Rune r, RgbColor color, PixMap* pix) const {
    draw_internal(origin, r, color, pix);
}

void Font::draw_internal(Point origin, Rune r, RgbColor color, PixMap* pix) const {
    origin.v -= ascent;
    Rect glyph = glyph_rect(r);
    for (size_t y = 0; y < glyph.height(); ++y) {
        for (size_t x = 0; x < glyph.width(); ++x) {
            if (_glyph_table.get(glyph.left + x, glyph.top + y).red < 255) {
                pix->set(origin.h + x, origin.v + y, color);
            }
        }
    }
}

void Font::draw_sprite(Point origin, sfz::StringSlice string, RgbColor color) const {
    origin.offset(0, -ascent);
    for (size_t i = 0; i < string.size(); ++i) {
        _sprites.find(string.at(i))->second->draw_shaded(origin.h, origin.v, color);
        origin.offset(char_width(string.at(i)), 0);
    }
}

void InitDirectText() {
    gDirectTextData[0] = tactical_font = new Font("tactical");
    gDirectTextData[1] = computer_font = new Font("computer");
    gDirectTextData[2] = button_font = new Font("button");
    gDirectTextData[4] = title_font = new Font("title");
    gDirectTextData[5] = small_button_font = new Font("button-small");
}

void DirectTextCleanup() {
    delete tactical_font;
    delete computer_font;
    delete button_font;
    delete title_font;
    delete small_button_font;
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

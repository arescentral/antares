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

#include "drawing/pix-table.hpp"

#include <sfz/sfz.hpp>

#include "data/resource.hpp"
#include "drawing/color.hpp"
#include "video/driver.hpp"

using sfz::BytesSlice;
using sfz::Exception;
using sfz::Json;
using sfz::JsonDefaultVisitor;
using sfz::ReadSource;
using sfz::String;
using sfz::StringMap;
using sfz::StringSlice;
using sfz::format;
using sfz::range;
using sfz::read;
using sfz::string_to_json;
using std::unique_ptr;
using std::vector;

namespace utf8 = sfz::utf8;

namespace antares {

namespace {

struct PixTableVisitor : public JsonDefaultVisitor {
    enum State {
        NEW,
        FRAME,
        IMAGE,
        OVERLAY,
        X_OFFSET,
        Y_OFFSET,
        DONE,
    };
    State& state;
    int16_t id;
    uint8_t color;
    vector<NatePixTable::Frame>& frames;

    PixTableVisitor(State& state, int16_t id, uint8_t color,
                    vector<NatePixTable::Frame>& frames):
            state(state),
            id(id),
            color(color),
            frames(frames) { }

    virtual void visit_array(const std::vector<Json>& value) const {
        switch (state) {
          case NEW:
            state = FRAME;
            for (const auto& v: value) {
                frames.emplace_back();
                v.accept(*this);
                frames.back().build(id, frames.size() - 1);
            }
            state = DONE;
            break;
          default:
            return visit_default("array");
        }
    }

    virtual void visit_object(const StringMap<Json>& value) const {
        switch (state) {
          case FRAME:
            if (value.find("image") == value.end()) {
                throw Exception("missing image in sprite json");
            }
            state = IMAGE;
            value.find("image")->second.accept(*this);

            if (color) {
                if (value.find("overlay") == value.end()) {
                    throw Exception("missing overlay in sprite json");
                }
                state = OVERLAY;
                value.find("overlay")->second.accept(*this);
            }

            if (value.find("x-offset") != value.end()) {
                state = X_OFFSET;
                value.find("x-offset")->second.accept(*this);
            }
            if (value.find("y-offset") != value.end()) {
                state = Y_OFFSET;
                value.find("y-offset")->second.accept(*this);
            }

            state = FRAME;
            break;

          default:
            return visit_default("object");
        }
    }

    virtual void visit_string(const StringSlice& value) const {
        switch (state) {
          case IMAGE:
            frames.back().load_image(value);
            break;
          case OVERLAY:
            frames.back().load_overlay(value, color);
            break;
          default:
            return visit_default("string");
        }
    }

    virtual void visit_number(double value) const {
        switch (state) {
          case X_OFFSET:
            frames.back().set_x_offset(value);
            break;
          case Y_OFFSET:
            frames.back().set_y_offset(value);
            break;
          default:
            return visit_default("number");
        }
    }

    virtual void visit_default(const char* type) const {
        throw Exception(format("unexpected {0} in sprite json", type));
    }
};

}  // namespace

NatePixTable::NatePixTable(int id, uint8_t color) {
    Resource rsrc("sprites", "json", id);
    String data(utf8::decode(rsrc.data()));
    Json json;
    if (!string_to_json(data, json)) {
        throw Exception("invalid sprite json");
    }
    PixTableVisitor::State state = PixTableVisitor::NEW;
    json.accept(PixTableVisitor(state, id, color, _frames));
}

NatePixTable::~NatePixTable() { }

const NatePixTable::Frame& NatePixTable::at(size_t index) const {
    return _frames[index];
}

size_t NatePixTable::size() const {
    return _size;
}

NatePixTable::Frame::Frame()
        : _h_offset(0),
          _v_offset(0),
          _pix_map(0, 0) { }

NatePixTable::Frame::~Frame() { }

void NatePixTable::Frame::load_image(StringSlice path) {
    Resource rsrc(path);
    BytesSlice data = rsrc.data();
    read(data, _pix_map);
}

void NatePixTable::Frame::load_overlay(StringSlice path, uint8_t color) {
    Resource rsrc(path);
    BytesSlice data = rsrc.data();
    ArrayPixMap overlay(0, 0);
    read(data, overlay);
    if ((overlay.size().width != width()) || (overlay.size().height != height())) {
        throw Exception("sprite overlay size mismatch");
    }
    for (auto x: range(width())) {
        for (auto y: range(height())) {
            RgbColor over = overlay.get(x, y);
            uint8_t value = over.red;
            uint8_t frac = over.alpha;
            over = RgbColor::tint(color, value);
            RgbColor under = _pix_map.get(x, y);
            RgbColor composite;
            composite.red = ((over.red * frac) + (under.red * (255 - frac))) / 255;
            composite.green = ((over.green * frac) + (under.green * (255 - frac))) / 255;
            composite.blue = ((over.blue * frac) + (under.blue * (255 - frac))) / 255;
            composite.alpha = under.alpha;
            _pix_map.set(x, y, composite);
        }
    }
}

void NatePixTable::Frame::set_x_offset(int32_t x) {
    _h_offset = x;
}

void NatePixTable::Frame::set_y_offset(int32_t y) {
    _v_offset = y;
}

uint16_t NatePixTable::Frame::width() const { return _pix_map.size().width; }
uint16_t NatePixTable::Frame::height() const { return _pix_map.size().height; }
Point NatePixTable::Frame::center() const { return Point(_h_offset, _v_offset); }
const PixMap& NatePixTable::Frame::pix_map() const { return _pix_map; }
const Sprite& NatePixTable::Frame::sprite() const { return *_sprite; }

void NatePixTable::Frame::build(int16_t id, int frame) {
    _sprite = VideoDriver::driver()->new_sprite(
            format("/sprites/{0}.SMIV/{1}", id, frame), _pix_map);
}

}  // namespace antares

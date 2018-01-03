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

#include "drawing/pix-table.hpp"

#include <pn/array>
#include <pn/file>
#include <pn/map>
#include <sfz/sfz.hpp>

#include "data/pn.hpp"
#include "data/resource.hpp"
#include "drawing/color.hpp"
#include "game/sys.hpp"
#include "video/driver.hpp"

using sfz::ReadSource;
using sfz::StringMap;
using sfz::range;
using sfz::read;
using std::unique_ptr;
using std::vector;

namespace utf8 = sfz::utf8;

namespace antares {

static void load_image(ArrayPixMap& image, pn::string_view path) {
    Resource        rsrc(path);
    sfz::BytesSlice data{rsrc.data().data(), static_cast<size_t>(rsrc.data().size())};
    read(data, image);
}

NatePixTable::NatePixTable(int id, uint8_t color) {
    Resource  rsrc("sprites", "pn", id);
    pn::value x;
    if (!pn::parse(rsrc.string().open(), x, nullptr)) {
        throw std::runtime_error("invalid sprite");
    }
    pn::map_cref m = x.as_map();

    struct State {
        int         rows, cols;
        Point       center;
        Rect        frame;
        ArrayPixMap image, overlay;
        State() : image(0, 0), overlay(0, 0) {}
    } state;

    pn::map_cref point = m.get("center").as_map();
    state.center       = Point(point.get("x").as_int(), point.get("y").as_int());
    state.rows         = m.get("rows").as_int();
    state.cols         = m.get("cols").as_int();
    load_image(state.image, m.get("image").as_string());
    load_image(state.overlay, m.get("overlay").as_string());

    if (state.image.size() != state.overlay.size()) {
        throw std::runtime_error("size mismatch between image and overlay");
    }
    if (state.image.size().width % state.cols) {
        throw std::runtime_error("sprite column count does not evenly split image");
    }
    if (state.image.size().height % state.rows) {
        throw std::runtime_error("sprite row count does not evenly split image");
    }

    pn::array_cref frames = m.get("frames").as_array();
    if (frames.size() != (state.rows * state.cols)) {
        throw std::runtime_error("frame count not equal to rows * cols");
    }
    for (pn::value_cref frame_value : frames) {
        pn::map_cref f = frame_value.as_map();
        state.frame =
                Rect(f.get("left").as_int(), f.get("top").as_int(), f.get("right").as_int(),
                     f.get("bottom").as_int());
        const int  frame       = _frames.size();
        const int  col         = frame % state.cols;
        const int  row         = frame / state.cols;
        const int  cell_width  = state.image.size().width / state.cols;
        const int  cell_height = state.image.size().height / state.rows;
        const Rect cell(Point(cell_width * col, cell_height * row), Size(cell_width, cell_height));
        Rect       sprite(state.frame);
        sprite.offset(state.center.h, state.center.v);
        Rect bounds(state.frame);
        bounds.offset(2 * -bounds.left, 2 * -bounds.top);
        if (color) {
            _frames.emplace_back(
                    bounds, state.image.view(cell).view(sprite), id, frame,
                    state.overlay.view(cell).view(sprite), color);
        } else {
            _frames.emplace_back(bounds, state.image.view(cell).view(sprite), id, frame);
        }
    }
}

NatePixTable::~NatePixTable() {}

const NatePixTable::Frame& NatePixTable::at(size_t index) const { return _frames[index]; }

size_t NatePixTable::size() const { return _size; }

NatePixTable::Frame::Frame(
        Rect bounds, const PixMap& image, int16_t id, int frame, const PixMap& overlay,
        uint8_t color)
        : _bounds(bounds), _pix_map(bounds.width(), bounds.height()) {
    load_image(image);
    load_overlay(overlay, color);
    build(id, frame);
}

NatePixTable::Frame::Frame(Rect bounds, const PixMap& image, int16_t id, int frame)
        : _bounds(bounds), _pix_map(bounds.width(), bounds.height()) {
    load_image(image);
    build(id, frame);
}

NatePixTable::Frame::~Frame() {}

void NatePixTable::Frame::load_image(const PixMap& pix) { _pix_map.copy(pix); }

void NatePixTable::Frame::load_overlay(const PixMap& pix, uint8_t color) {
    for (auto x : range(width())) {
        for (auto y : range(height())) {
            RgbColor over  = pix.get(x, y);
            uint8_t  value = over.red;
            uint8_t  frac  = over.alpha;
            over           = RgbColor::tint(color, value);
            RgbColor under = _pix_map.get(x, y);
            RgbColor composite;
            composite.red   = ((over.red * frac) + (under.red * (255 - frac))) / 255;
            composite.green = ((over.green * frac) + (under.green * (255 - frac))) / 255;
            composite.blue  = ((over.blue * frac) + (under.blue * (255 - frac))) / 255;
            composite.alpha = under.alpha;
            _pix_map.set(x, y, composite);
        }
    }
}

uint16_t       NatePixTable::Frame::width() const { return _bounds.width(); }
uint16_t       NatePixTable::Frame::height() const { return _bounds.height(); }
Point          NatePixTable::Frame::center() const { return _bounds.origin(); }
const PixMap&  NatePixTable::Frame::pix_map() const { return _pix_map; }
const Texture& NatePixTable::Frame::texture() const { return _texture; }

void NatePixTable::Frame::build(int16_t id, int frame) {
    _texture = sys.video->texture(pn::format("/sprites/{0}.SMIV/{1}", id, frame), _pix_map);
}

}  // namespace antares

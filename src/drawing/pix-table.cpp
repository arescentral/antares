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

#include "data/resource.hpp"
#include "data/sprite-data.hpp"
#include "drawing/color.hpp"
#include "game/sys.hpp"
#include "video/driver.hpp"

using sfz::StringMap;
using sfz::range;
using std::unique_ptr;
using std::vector;

namespace antares {

NatePixTable::NatePixTable(pn::string_view name, Hue hue) {
    SpriteData  data    = Resource::sprite_data(name);
    ArrayPixMap image   = Resource::sprite_image(name);
    ArrayPixMap overlay = Resource::sprite_overlay(name);

    if (image.size() != overlay.size()) {
        throw std::runtime_error("size mismatch between image and overlay");
    }
    for (SpriteData::Frame frame : data.frames) {
        const int i = _frames.size();
        Rect      sprite{frame.left, frame.top, frame.right, frame.bottom};
        Rect      bounds = sprite;
        bounds.offset(-frame.cx, -frame.cy);
        if (hue == Hue::GRAY) {
            _frames.emplace_back(bounds, image.view(sprite), name, i);
        } else {
            _frames.emplace_back(bounds, image.view(sprite), name, i, overlay.view(sprite), hue);
        }
    }
}

NatePixTable::~NatePixTable() {}

const NatePixTable::Frame& NatePixTable::at(size_t index) const { return _frames[index]; }

size_t NatePixTable::size() const { return _size; }

NatePixTable::Frame::Frame(
        Rect bounds, const PixMap& image, pn::string_view name, int frame, const PixMap& overlay,
        Hue hue)
        : _bounds(bounds), _pix_map(bounds.width(), bounds.height()) {
    load_image(image);
    load_overlay(overlay, hue);
    build(name, frame);
}

NatePixTable::Frame::Frame(Rect bounds, const PixMap& image, pn::string_view name, int frame)
        : _bounds(bounds), _pix_map(bounds.width(), bounds.height()) {
    load_image(image);
    build(name, frame);
}

NatePixTable::Frame::~Frame() {}

void NatePixTable::Frame::load_image(const PixMap& pix) { _pix_map.copy(pix); }

void NatePixTable::Frame::load_overlay(const PixMap& pix, Hue hue) {
    for (auto x : range(width())) {
        for (auto y : range(height())) {
            RgbColor over  = pix.get(x, y);
            uint8_t  value = over.red;
            uint8_t  frac  = over.alpha;
            over           = RgbColor::tint(hue, value);
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
Point          NatePixTable::Frame::center() const { return {-_bounds.left, -_bounds.top}; }
const PixMap&  NatePixTable::Frame::pix_map() const { return _pix_map; }
const Texture& NatePixTable::Frame::texture() const { return _texture; }

void NatePixTable::Frame::build(pn::string_view name, int frame) {
    _texture = sys.video->texture(pn::format("/sprites/{0}%{1}", name, frame), _pix_map, 1);
}

}  // namespace antares

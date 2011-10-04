// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// Copyright (C) 2008-2011 Ares Central
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
// License along with this program.  If not, see
// <http://www.gnu.org/licenses/>.

#include "drawing/pix-table.hpp"

#include <sfz/sfz.hpp>

#include "data/resource.hpp"
#include "drawing/color.hpp"
#include "video/driver.hpp"

using sfz::BytesSlice;
using sfz::ReadSource;
using sfz::format;
using sfz::range;
using sfz::read;
using sfz::scoped_ptr;

namespace antares {

NatePixTable::NatePixTable(int id, uint8_t color) {
    Resource rsrc("sprites", "SMIV", id);
    BytesSlice in(rsrc.data());

    in.shift(4);
    _size = read<uint32_t>(in);

    std::vector<uint32_t> offsets;
    for (size_t i = 0; i < _size; ++i) {
        offsets.push_back(read<uint32_t>(in));
    }

    _entries.reset(new Frame[_size]);
    for (size_t i = 0; i < _size; ++i) {
        BytesSlice entry_data(rsrc.data().slice(offsets[i]));
        _entries[i].build(entry_data, id, i, color);
    }
}

NatePixTable::~NatePixTable() { }

const NatePixTable::Frame& NatePixTable::at(size_t index) const {
    return _entries[index];
}

size_t NatePixTable::size() const {
    return _size;
}

NatePixTable::Frame::Frame()
        : _width(0),
          _height(0),
          _h_offset(0),
          _v_offset(0),
          _pix_map(0, 0) { }

NatePixTable::Frame::~Frame() { }

uint16_t NatePixTable::Frame::width() const { return _width; }
uint16_t NatePixTable::Frame::height() const { return _height; }
Point NatePixTable::Frame::center() const { return Point(_h_offset, _v_offset); }
const PixMap& NatePixTable::Frame::pix_map() const { return _pix_map; }
const Sprite& NatePixTable::Frame::sprite() const { return *_sprite; }

void NatePixTable::Frame::build(BytesSlice in, int32_t id, int32_t frame_number, uint8_t color) {
    read(in, _width);
    read(in, _height);
    read(in, _h_offset);
    read(in, _v_offset);
    in = in.slice(0, _width * _height);

    _pix_map.resize(Size(_width, _height));
    _pix_map.fill(RgbColor::kClear);
    if (color) {
        colorize_pix_map(in, color);
    } else {
        fill_pix_map(in);
    }
    _sprite.reset(VideoDriver::driver()->new_sprite(
                format("/sprites/{0}.SMIV/{1}", id, frame_number), _pix_map));
}

void NatePixTable::Frame::fill_pix_map(BytesSlice bytes) {
    SFZ_FOREACH(uint16_t y, range(_height), {
        SFZ_FOREACH(uint16_t x, range(_width), {
            uint8_t byte = read<uint8_t>(bytes);
            if (byte) {
                _pix_map.set(x, y, RgbColor::at(byte));
            }
        });
    });
}

void NatePixTable::Frame::colorize_pix_map(BytesSlice bytes, uint8_t color) {
    color <<= 4;

    // count the # of pixels, and # of pixels that are white
    int white_count = 0;
    int pixel_count = 0;
    SFZ_FOREACH(uint8_t byte, bytes, {
        if (byte) {
            ++pixel_count;
            if (byte <= 15) {
                ++white_count;
            }
        }
    });

    // If more than 1/3 of the opaque pixels in this sprite are in the 'white' band of the
    // color table, then colorize all opaque (non-0x00) pixels.  Otherwise, only colorize
    // pixels which are opaque and outside of the white band (which is 0x01..0x0F).
    const uint8_t color_mask = (white_count > (pixel_count / 3)) ? 0xFF : 0xF0;
    for (int y = 0; y < _height; ++y) {
        for (int x = 0; x < _width; ++x) {
            uint8_t byte = read<uint8_t>(bytes);
            if (byte & color_mask) {
                byte = (byte & 0x0F) | color;
                _pix_map.set(x, y, RgbColor::at(byte));
            } else if (byte) {
                _pix_map.set(x, y, RgbColor::at(byte));
            }
        }
    }
}

}  // namespace antares

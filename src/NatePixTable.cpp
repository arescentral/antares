// Ares, a tactical space combat game.
// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// 
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

#include "NatePixTable.hpp"

#include "sfz/ReadItem.hpp"
#include "ColorTranslation.hpp"
#include "Resource.hpp"

using sfz::BytesPiece;
using sfz::ReadSource;
using sfz::read;

namespace antares {

class natePixEntryType {
  public:
    natePixEntryType()
            : _width(0),
              _height(0),
              _h_offset(0),
              _v_offset(0),
              _data(NULL),
              _pixels(NULL) { }

    ~natePixEntryType() {
        clear();
    }

    uint16_t width() const { return _width; }
    uint16_t height() const { return _height; }
    int16_t h_offset() const { return _h_offset; }
    int16_t v_offset() const { return _v_offset; }
    uint8_t* data() const { return _data; }
    RgbColor* pixels() const { return _pixels; }

    void read_from(ReadSource in) {
        read(in, &_width);
        read(in, &_height);
        read(in, &_h_offset);
        read(in, &_v_offset);
        _data = new uint8_t[_width * _height];
        _pixels = new RgbColor[_width * _height];
        read(in, _data, _width * _height);

        ColorTable table(256);
        for (int i = 0; i < _width * _height; ++i) {
            if (_data[i]) {
                _pixels[i] = table.color(_data[i]);
            } else {
                _pixels[i] = RgbColor::kClear;
            }
        }
    }

    void clear() {
        delete[] _data;
        _data = NULL;
        delete[] _pixels;
        _pixels = NULL;
    }

  private:
    uint16_t _width;
    uint16_t _height;
    int16_t _h_offset;
    int16_t _v_offset;
    uint8_t* _data;
    RgbColor* _pixels;

    DISALLOW_COPY_AND_ASSIGN(natePixEntryType);
};
void read_from(ReadSource in, natePixEntryType* pix) { pix->read_from(in); }

natePixType::natePixType() { }

natePixType::natePixType(int id) {
    Resource rsrc('SMIV', id);
    BytesPiece in(rsrc.data());

    uint32_t size;
    int32_t pixnum;
    read(&in, &size);
    read(&in, &pixnum);

    std::vector<uint32_t> offsets;
    for (int i = 0; i < pixnum; ++i) {
        uint32_t offset;
        read(&in, &offset);
        offsets.push_back(offset);
    }

    for (int i = 0; i < pixnum; ++i) {
        BytesPiece entry(rsrc.data().substr(offsets[i]));
        _entries.push_back(new natePixEntryType);
        read(&entry, _entries.back());
    }
}

natePixType::~natePixType() {
    clear();
}

natePixEntryType* natePixType::at(size_t index) const {
    return _entries.at(index);
}

size_t natePixType::size() const {
    return _entries.size();
}

void natePixType::clear() {
    for (std::vector<natePixEntryType*>::iterator it = _entries.begin();
            it != _entries.end(); ++it) {
        delete *it;
    }
    _entries.clear();
}

long GetNatePixTablePixNum(const natePixType& table) {
    return table.size();
}

int GetNatePixTableNatePixWidth(const natePixType& table, long pixnum) {
    return table.at(pixnum)->width();
}

int GetNatePixTableNatePixHeight(const natePixType& table, long pixnum) {
    return table.at(pixnum)->height();
}

int GetNatePixTableNatePixHRef(const natePixType& table, long pixnum) {
    return table.at(pixnum)->h_offset();
}

int GetNatePixTableNatePixVRef(const natePixType& table, long pixnum) {
    return table.at(pixnum)->v_offset();
}

RgbColor* GetNatePixTableNatePixData(const natePixType& table, long pixnum) {
    return table.at(pixnum)->pixels();
}

// RemapNatePixTableColor:
//  given a NatePixTable, converts the raw pixel data based on custom color table, and translates
//  into device's color table, using Backbone Graphic's GetTranslateIndex().

void RemapNatePixTableColor(natePixType* table) {
    for (size_t l = 0; l < table->size(); ++l) {
        natePixEntryType* frame = table->at(l);
        int w = frame->width();
        int h = frame->height();
        uint8_t* p = frame->data();
        for (int i = 0; i < h; ++i) {
            for (int j = 0; j < w; ++j) {
                *p = GetTranslateIndex(*p);
                ++p;
            }
        }
    }
}

// ColorizeNatePixTableColor:
//  given a NatePixTable, converts the raw pixel data based on custom color table, and translates
//  into device's color table, but colorizes to color.

void ColorizeNatePixTableColor(natePixType* table, uint8_t color) {
    color <<= 4;

    for (size_t l = 0; l < table->size(); ++l) {
        natePixEntryType* frame = table->at(l);
        const int w = frame->width();
        const int h = frame->height();

        // count the # of pixels, and # of pixels that are white
        int white_count = 0;
        int pixel_count = 0;
        uint8_t* p = frame->data();
        for (int i = 0; i < h; ++i) {
            for (int j = 0; j < w; ++j) {
                if (*p != 0) {
                    ++pixel_count;
                    if (*p <= 15) {
                        ++white_count;
                    }
                }
                ++p;
            }
        }

        // If more than 1/3 of the opaque pixels in this sprite are in the 'white' band of the
        // color table, then colorize all opaque (non-0x00) pixels.  Otherwise, only colorize
        // pixels which are opaque and outside of the white band (which is 0x01..0x0F).
        const uint8_t color_mask = (white_count > (pixel_count / 3)) ? 0xFF : 0xF0;
        p = frame->data();
        for (int i = 0; i < h; ++i) {
            for (int j = 0; j < w; ++j) {
                if (*p & color_mask) {
                    *p = (*p & 0x0F) | color;
                }
                ++p;
            }
        }
    }
}

}  // namespace antares

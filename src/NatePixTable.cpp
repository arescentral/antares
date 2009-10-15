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

#include "BinaryStream.hpp"
#include "ColorTranslation.hpp"
#include "Sound.h"

namespace antares {

class natePixEntryType {
  public:
    natePixEntryType()
            : _width(0),
              _height(0),
              _h_offset(0),
              _v_offset(0),
              _data(NULL) { }

    natePixEntryType(const natePixEntryType& other)
            : _data(NULL) {
        copy_from(other);
    }

    natePixEntryType& operator=(const natePixEntryType& other) {
        copy_from(other);
        return *this;
    }

    ~natePixEntryType() {
        clear();
    }

    uint16_t width() const { return _width; }
    uint16_t height() const { return _height; }
    int16_t h_offset() const { return _h_offset; }
    int16_t v_offset() const { return _v_offset; }
    uint8_t* data() const { return _data; }

    void read(BinaryReader* bin) {
        bin->read(&_width);
        bin->read(&_height);
        bin->read(&_h_offset);
        bin->read(&_v_offset);
        _data = new uint8_t[_width * _height];
        bin->read(_data, _width * _height);
    }

    void copy_from(const natePixEntryType& other) {
        clear();
        _width = other._width;
        _height = other._height;
        _h_offset = other._h_offset;
        _v_offset = other._v_offset;
        _data = new uint8_t[_width * _height];
        memcpy(_data, other._data, _width * _height);
    }

    void clear() {
        delete[] _data;
        _data = NULL;
    }

  private:
    uint16_t _width;
    uint16_t _height;
    int16_t _h_offset;
    int16_t _v_offset;
    uint8_t* _data;
};

natePixType::natePixType() { }

natePixType::natePixType(const natePixType& other) {
    copy_from(other);
}

natePixType& natePixType::operator=(const natePixType& other) {
    copy_from(other);
    return *this;
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

size_t natePixType::load_data(const char* data, size_t len) {
    clear();

    BufferBinaryReader bin(data, len);

    uint32_t size;
    int32_t pixnum;
    bin.read(&size);
    bin.read(&pixnum);

    std::vector<uint32_t> offsets;
    for (int i = 0; i < pixnum; ++i) {
        uint32_t offset;
        bin.read(&offset);
        offsets.push_back(offset);
    }

    for (int i = 0; i < pixnum; ++i) {
        bin.discard(offsets[i] - bin.bytes_read());
        _entries.push_back(new natePixEntryType);
        bin.read(_entries.back());
    }
    bin.discard(len - bin.bytes_read());

    return bin.bytes_read();
}

void natePixType::copy_from(const natePixType& other) {
    clear();
    for (size_t i = 0; i < other.size(); ++i) {
        _entries.push_back(new natePixEntryType(*other.at(i)));
    }
}

void natePixType::clear() {
    for (std::vector<natePixEntryType*>::iterator it = _entries.begin();
            it != _entries.end(); ++it) {
        delete *it;
    }
    _entries.clear();
}

long GetNatePixTablePixNum(TypedHandle<natePixType> table) {
    return (*table)->size();
}

int GetNatePixTableNatePixWidth(TypedHandle<natePixType> table, long pixnum) {
    return (*table)->at(pixnum)->width();
}

int GetNatePixTableNatePixHeight(TypedHandle<natePixType> table, long pixnum) {
    return (*table)->at(pixnum)->height();
}

int GetNatePixTableNatePixHRef(TypedHandle<natePixType> table, long pixnum) {
    return (*table)->at(pixnum)->h_offset();
}

int GetNatePixTableNatePixVRef(TypedHandle<natePixType> table, long pixnum) {
    return (*table)->at(pixnum)->v_offset();
}

uint8_t* GetNatePixTableNatePixData(TypedHandle<natePixType> table, long pixnum) {
    return (*table)->at(pixnum)->data();
}

// RemapNatePixTableColor:
//  given a NatePixTable, converts the raw pixel data based on custom color table, and translates
//  into device's color table, using Backbone Graphic's GetTranslateIndex().

void RemapNatePixTableColor(TypedHandle<natePixType> table) {
    for (int l = 0; l < GetNatePixTablePixNum(table); ++l) {
        int w = GetNatePixTableNatePixWidth(table, l);
        int h = GetNatePixTableNatePixHeight(table, l);
        uint8_t* p = GetNatePixTableNatePixData(table, l);
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

void ColorizeNatePixTableColor(TypedHandle<natePixType> table, uint8_t color) {
    color <<= 4;

    for (int l = 0; l < GetNatePixTablePixNum(table); ++l) {
        const int w = GetNatePixTableNatePixWidth(table, l);
        const int h = GetNatePixTableNatePixHeight(table, l);

        // count the # of pixels, and # of pixels that are white
        int white_count = 0;
        int pixel_count = 0;
        uint8_t* p = GetNatePixTableNatePixData(table, l);
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
        p = GetNatePixTableNatePixData(table, l);
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

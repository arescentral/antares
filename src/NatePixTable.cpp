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

// NATEPIXTABLE.C

// ShapeMachine IV by Nathan Lamont

#include "NatePixTable.hpp"

#include "BinaryStream.hpp"
#include "ColorTranslation.hpp"
#include "Resources.h"
#include "Sound.h"

namespace antares {

#define EMPTY_NATE_PIX_SIZE         8L

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
    char* data() const { return _data; }

    void read(BinaryReader* bin) {
        bin->read(&_width);
        bin->read(&_height);
        bin->read(&_h_offset);
        bin->read(&_v_offset);
        _data = new char[_width * _height];
        bin->read(_data, _width * _height);
    }

    void copy_from(const natePixEntryType& other) {
        clear();
        _width = other._width;
        _height = other._height;
        _h_offset = other._h_offset;
        _v_offset = other._v_offset;
        _data = new char[_width * _height];
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
    char* _data;
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

unsigned char *GetNatePixTableNatePixData(TypedHandle<natePixType> table, long pixnum) {
    return reinterpret_cast<unsigned char*>((*table)->at(pixnum)->data());
}

unsigned char GetNatePixTableNatePixDataPixel(TypedHandle<natePixType> table, long pixnum, int x,
        int y) {
    natePixEntryType* entry = (*table)->at(pixnum);
    int width = entry->width();

    return entry->data()[y * width + x];
}

// GetNatePixTableNatePixPtr:
//  makes a new natePix structure with a baseAddr pointing into the natePixType, meaning
//  the original natePixType cannot be unlocked or disposed.

/*
void GetNatePixTableNatePixPtr( natePix *dPix, Handle table, int pixnum)

{

    if (NewNatePix( dPix, GetNatePixTableNatePixWidth( table, pixnum),
            GetNatePixTableNatePixHeight( table, pixnum), reinterpret_cast<char*>(*table) +
            GetNatePixTablePixOffset( table, pixnum) + 8L) != 0)
        SysBeep (20);
}
*/

/*
void GetNatePixTableNatePixDuplicate( natePix *dPix, Handle table, int pixnum)

{

    if (NewNatePix( dPix, GetNatePixTableNatePixWidth( table, pixnum),
            GetNatePixTableNatePixHeight( table, pixnum), reinterpret_cast<char*>(*table) +
            GetNatePixTablePixOffset( table, pixnum) + 8L) != 0)
        SysBeep (20);
    else
    {
        dPix->pixBase = GetNatePixTableNatePixDataCopy( table, pixnum);
    }
}
*/

// RemapNatePixTableColor:
//  given a NatePixTable, converts the raw pixel data based on custom color table, and translates
//  into device's color table, using Backbone Graphic's GetTranslateIndex().

void RemapNatePixTableColor(TypedHandle<natePixType> table) {
    long            l;
    unsigned char*  p;
    int             i, j, w, h;

//  WriteDebugLine((char *)"\pRemapSize:");
//  WriteDebugLong( GetNatePixTablePixNum( table));

    for ( l = 0; l < GetNatePixTablePixNum( table); l++)
    {
//      WriteDebugLong( l);
        w = GetNatePixTableNatePixWidth( table, l);
        h = GetNatePixTableNatePixHeight( table, l);
        p = GetNatePixTableNatePixData( table, l);
        for ( j = 0; j < h; j++)
        {
            for ( i = 0; i < w; i++)
            {
//              SetNatePixTableNatePixDataPixel( table, l, i, j,
//                  GetTranslateIndex(
//                  (int)GetNatePixTableNatePixDataPixel( table, l, i, j)));
                *p = GetTranslateIndex( *p);
                p++;
            }
        }
    }


}

// ColorizeNatePixTableColor:
//  given a NatePixTable, converts the raw pixel data based on custom color table, and translates
//  into device's color table, but colorizes to color.

void ColorizeNatePixTableColor(TypedHandle<natePixType> table, unsigned char color) {
    long            l, whiteCount, pixelCount;
    unsigned char   pixel, *p;
    int             i, j, w, h;

    color <<= 4;

    for ( l = 0; l < GetNatePixTablePixNum( table); l++)
    {
        w = GetNatePixTableNatePixWidth( table, l);
        h = GetNatePixTableNatePixHeight( table, l);

        // count the # of pixels, and # of pixels that are white
        whiteCount = pixelCount = 0;
        p = GetNatePixTableNatePixData( table, l);
        for ( j = 0; j < h; j++)
        {
            for ( i = 0; i < w; i++)
            {
                pixel = *p; //GetNatePixTableNatePixDataPixel( table, l, i, j);
                if ( pixel != 0)
                {
                    pixelCount++;
                    if ( pixel <= 15) whiteCount++;
                }
                p++;
            }
        }

        if ( whiteCount > ( pixelCount / 3)) whiteCount = 1;
        else whiteCount = 0;
        p = GetNatePixTableNatePixData( table, l);
        for ( j = 0; j < h; j++)
        {
            for ( i = 0; i < w; i++)
            {
                pixel = *p;//GetNatePixTableNatePixDataPixel( table, l, i, j);
                if ((pixel != 0) && ((pixel > 15) || ( whiteCount)))
                {
                    pixel &= 0x0f;
                    pixel += color;
                }
                *p = pixel; //SetNatePixTableNatePixDataPixel( table, l, i, j, pixel);
                p++;
            }
        }
    }
}

}  // namespace antares

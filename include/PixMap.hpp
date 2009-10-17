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

#ifndef ANTARES_PIX_MAP_HPP_
#define ANTARES_PIX_MAP_HPP_

#include <exception>
#include <string>
#include "Geometry.hpp"
#include "SmartPtr.hpp"

namespace antares {

class BinaryReader;
class BinaryWriter;
class ColorTable;

class PixMapException : public std::exception {
  public:
    PixMapException(const std::string& what) throw() : _what(what) { }
    ~PixMapException() throw() { }
    virtual const char* what() const throw() { return _what.c_str(); }
  private:
    const std::string _what;
};

class PixMap {
  public:
    virtual const Rect& bounds() const = 0;
    virtual const ColorTable& colors() const = 0;
    virtual int row_bytes() const = 0;
    virtual const uint8_t* bytes() const = 0;

    virtual uint8_t* mutable_bytes() = 0;
    virtual ColorTable* mutable_colors() = 0;

    virtual const uint8_t* row(int y) const;
    virtual uint8_t* mutable_row(int y);
    virtual uint8_t get(int x, int y) const;
    virtual void set(int x, int y, uint8_t color);
    virtual void fill(uint8_t color);
    virtual void copy(const PixMap& pix);

    virtual void write(BinaryWriter* bin) const;

    class View;
    View view(const Rect& bounds);
};

class ArrayPixMap : public PixMap {
  public:
    ArrayPixMap(int32_t width, int32_t height);
    ~ArrayPixMap();

    void resize(const Rect& r);
    void read(BinaryReader* bin);

    virtual const Rect& bounds() const;
    virtual const ColorTable& colors() const;
    virtual int row_bytes() const;
    virtual const uint8_t* bytes() const;

    virtual uint8_t* mutable_bytes();
    virtual ColorTable* mutable_colors();

    virtual void fill(uint8_t color);

  private:
    Rect _bounds;
    scoped_ptr<ColorTable> _colors;
    scoped_array<uint8_t> _bytes;

    DISALLOW_COPY_AND_ASSIGN(ArrayPixMap);
};

class PixMap::View : public PixMap {
  public:
    View(PixMap* pix, const Rect& r);

    virtual const Rect& bounds() const;
    virtual const ColorTable& colors() const;
    virtual int row_bytes() const;
    virtual const uint8_t* bytes() const;

    virtual uint8_t* mutable_bytes();
    virtual ColorTable* mutable_colors();

  private:
    PixMap* const _parent;
    const Point _offset;
    Rect _bounds;

    // ALLOW_COPY_AND_ASSIGN(View);
};

}  // namespace antares

#endif  // ANTARES_PIX_MAP_HPP_

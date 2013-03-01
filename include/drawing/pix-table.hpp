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

#ifndef ANTARES_DRAWING_PIX_TABLE_HPP_
#define ANTARES_DRAWING_PIX_TABLE_HPP_

#include <vector>
#include <sfz/sfz.hpp>

#include "drawing/pix-map.hpp"

namespace antares {

class Sprite;

class NatePixTable {
  public:
    class Frame;

    NatePixTable(int id, uint8_t color);
    ~NatePixTable();

    const Frame& at(size_t index) const;
    size_t size() const;

  private:
    size_t _size;
    std::vector<Frame> _frames;

    DISALLOW_COPY_AND_ASSIGN(NatePixTable);
};

class NatePixTable::Frame {
  public:
    Frame();
    Frame(Frame&&) = default;
    ~Frame();

    void load_image(sfz::StringSlice path);
    void load_overlay(sfz::StringSlice path, uint8_t color);
    void set_x_offset(int32_t x);
    void set_y_offset(int32_t y);
    void build(int16_t id, int frame);

    uint16_t width() const;
    uint16_t height() const;
    Point center() const;
    const PixMap& pix_map() const;
    const Sprite& sprite() const;

  private:
    int16_t _h_offset;
    int16_t _v_offset;
    ArrayPixMap _pix_map;
    std::unique_ptr<Sprite> _sprite;

    DISALLOW_COPY_AND_ASSIGN(Frame);
};

}  // namespace antares

#endif // ANTARES_DRAWING_PIX_TABLE_HPP_

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

#ifndef ANTARES_BINARY_STREAM_HPP_
#define ANTARES_BINARY_STREAM_HPP_

#include <stdint.h>
#include <stdlib.h>
#include "SmartPtr.hpp"

class BinaryStream {
  public:
    BinaryStream(const char* data, size_t len);

    template <typename T>
    void read(T* t, size_t count = 1);

  private:
    template <typename T>
    void read_primitive(T* t, size_t count = 1);

    const char* _data;
    size_t _len;
    size_t _pos;

    DISALLOW_COPY_AND_ASSIGN(BinaryStream);
};

template <> void BinaryStream::read<bool>(bool* b, size_t count);
template <> void BinaryStream::read<char>(char* c, size_t count);
template <> void BinaryStream::read<unsigned char>(unsigned char* uc, size_t count);
template <> void BinaryStream::read<int8_t>(int8_t* i8, size_t count);
template <> void BinaryStream::read<int16_t>(int16_t* i16, size_t count);
template <> void BinaryStream::read<uint16_t>(uint16_t* u16, size_t count);
template <> void BinaryStream::read<int32_t>(int32_t* i32, size_t count);
template <> void BinaryStream::read<uint32_t>(uint32_t* u32, size_t count);
template <> void BinaryStream::read<int64_t>(int64_t* i64, size_t count);
template <> void BinaryStream::read<uint64_t>(uint64_t* u64, size_t count);

template <typename T>
void BinaryStream::read(T* t, size_t count) {
    for (size_t i = 0; i < count; ++i) {
        t[i].read(this);
    }
}

#endif  // ANTARES_BINARY_STREAM_HPP_

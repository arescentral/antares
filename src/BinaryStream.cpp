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

#include "BinaryStream.hpp"

#include <assert.h>
#include <libkern/OSByteOrder.h>

BinaryStream::BinaryStream(const char* data, size_t len)
    : _data(data),
      _len(len),
      _pos(0) { }

namespace {

template <typename T, size_t bytes = sizeof(T)>
struct EndiannessConverter;

template <typename T>
struct EndiannessConverter<T, 1> {
    inline static T convert(const void* ptr) {
        return *reinterpret_cast<const T*>(ptr);
    }
};

template <typename T>
struct EndiannessConverter<T, 2> {
    inline static T convert(const void* ptr) {
        return OSSwapBigToHostInt16(*reinterpret_cast<const T*>(ptr));
    }
};

template <typename T>
struct EndiannessConverter<T, 4> {
    inline static T convert(const void* ptr) {
        return OSSwapBigToHostInt32(*reinterpret_cast<const T*>(ptr));
    }
};

template <typename T>
struct EndiannessConverter<T, 8> {
    inline static T convert(const void* ptr) {
        return OSSwapBigToHostInt64(*reinterpret_cast<const T*>(ptr));
    }
};

}  // namespace

template <typename T>
void BinaryStream::read_primitive(T* t, size_t count) {
    for (size_t i = 0; i < count; ++i) {
        assert(_pos + sizeof(T) <= _len);
        t[i] = EndiannessConverter<T>::convert(_data + _pos);
        _pos += sizeof(T);
    }
}

template <>
void BinaryStream::read<bool>(bool* b, size_t count) {
    read_primitive(b, count);
}

template <>
void BinaryStream::read<char>(char* c, size_t count) {
    read_primitive(c, count);
}

template <>
void BinaryStream::read<int8_t>(int8_t* i8, size_t count) {
    read_primitive(i8, count);
}

template <>
void BinaryStream::read<uint8_t>(uint8_t* u8, size_t count) {
    read_primitive(u8, count);
}

template <>
void BinaryStream::read<int16_t>(int16_t* i16, size_t count) {
    read_primitive(i16, count);
}

template <>
void BinaryStream::read<uint16_t>(uint16_t* u16, size_t count) {
    read_primitive(u16, count);
}

template <>
void BinaryStream::read<int32_t>(int32_t* i32, size_t count) {
    read_primitive(i32, count);
}

template <>
void BinaryStream::read<uint32_t>(uint32_t* u32, size_t count) {
    read_primitive(u32, count);
}

template <>
void BinaryStream::read<int64_t>(int64_t* i64, size_t count) {
    read_primitive(i64, count);
}

template <>
void BinaryStream::read<uint64_t>(uint64_t* u64, size_t count) {
    read_primitive(u64, count);
}

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
#include <string.h>
#include <libkern/OSByteOrder.h>

namespace {

template <typename T>
struct Bytes {
    Bytes() { }
    explicit Bytes(const T& t) { memcpy(value, &t, sizeof(T)); }
    char value[sizeof(T)];
};

template <typename T, size_t bytes = sizeof(T)>
struct EndiannessConverter;

template <typename T>
struct EndiannessConverter<T, 2> {
    inline static T big_to_host(const Bytes<T> bytes) {
        return OSSwapBigToHostInt16(*reinterpret_cast<const T*>(bytes.value));
    }
    inline static Bytes<T> host_to_big(const T& t) {
        return Bytes<T>(OSSwapHostToBigInt16(t));
    }
};

template <typename T>
struct EndiannessConverter<T, 4> {
    inline static T big_to_host(const Bytes<T> bytes) {
        return OSSwapBigToHostInt32(*reinterpret_cast<const T*>(bytes.value));
    }
    inline static Bytes<T> host_to_big(const T& t) {
        return Bytes<T>(OSSwapHostToBigInt32(t));
    }
};

template <typename T>
struct EndiannessConverter<T, 8> {
    inline static T big_to_host(const Bytes<T> bytes) {
        return OSSwapBigToHostInt64(*reinterpret_cast<const T*>(bytes.value));
    }
    inline static Bytes<T> host_to_big(const T& t) {
        return Bytes<T>(OSSwapHostToBigInt64(t));
    }
};

}  // namespace

BinaryReader::BinaryReader()
        : _bytes_read(0) { }

BinaryReader::~BinaryReader() { }

template <typename T>
void BinaryReader::read_primitive(T* t, size_t count) {
    for (size_t i = 0; i < count; ++i) {
        Bytes<T> bytes;
        read_bytes(bytes.value, sizeof(T));
        t[i] = EndiannessConverter<T>::big_to_host(bytes);
        _bytes_read += sizeof(T);
    }
}

template <>
void BinaryReader::read<bool>(bool* b, size_t count) {
    for (size_t i = 0; i < count; ++i) {
        char c;
        read_bytes(&c, 1);
        b[i] = c;
    }
}

template <>
void BinaryReader::read<char>(char* c, size_t count) {
    read_bytes(c, count);
    _bytes_read += count;
}

template <>
void BinaryReader::read<int8_t>(int8_t* i8, size_t count) {
    read_bytes(reinterpret_cast<char*>(i8), count);
    _bytes_read += count;
}

template <>
void BinaryReader::read<uint8_t>(uint8_t* u8, size_t count) {
    read_bytes(reinterpret_cast<char*>(u8), count);
    _bytes_read += count;
}

template <>
void BinaryReader::read<int16_t>(int16_t* i16, size_t count) {
    read_primitive(i16, count);
}

template <>
void BinaryReader::read<uint16_t>(uint16_t* u16, size_t count) {
    read_primitive(u16, count);
}

template <>
void BinaryReader::read<int32_t>(int32_t* i32, size_t count) {
    read_primitive(i32, count);
}

template <>
void BinaryReader::read<uint32_t>(uint32_t* u32, size_t count) {
    read_primitive(u32, count);
}

template <>
void BinaryReader::read<int64_t>(int64_t* i64, size_t count) {
    read_primitive(i64, count);
}

template <>
void BinaryReader::read<uint64_t>(uint64_t* u64, size_t count) {
    read_primitive(u64, count);
}

void BinaryReader::discard(size_t bytes) {
    for (size_t i = 0; i < bytes; ++i) {
        char c;
        read_bytes(&c, 1);
    }
    _bytes_read += bytes;
}

BufferBinaryReader::BufferBinaryReader(const char* data, size_t len)
    : _data(data),
      _len(len) { }

#include <stdio.h>

void BufferBinaryReader::read_bytes(char* bytes, size_t count) {
    assert(bytes_read() + count <= _len);
    memcpy(bytes, _data + bytes_read(), count);
}

BinaryWriter::BinaryWriter()
        : _bytes_written(0) { }

BinaryWriter::~BinaryWriter() { }

template <typename T>
void BinaryWriter::write_primitive(T* t, size_t count) {
    for (size_t i = 0; i < count; ++i) {
        write_bytes(EndiannessConverter<T>::host_to_big(t[i]).value, sizeof(T));
        _bytes_written += sizeof(T);
    }
}

template <> void BinaryWriter::write<bool>(const bool* b, size_t count) {
    for (size_t i = 0; i < count; ++i) {
        char c = b[i];
        write_bytes(&c, 1);
    }
}

template <> void BinaryWriter::write<char>(const char* c, size_t count) {
    write_bytes(c, count);
    _bytes_written += count;
}

template <> void BinaryWriter::write<int8_t>(const int8_t* i8, size_t count) {
    write_bytes(reinterpret_cast<const char*>(i8), count);
    _bytes_written += count;
}

template <> void BinaryWriter::write<uint8_t>(const uint8_t* uc, size_t count) {
    write_bytes(reinterpret_cast<const char*>(uc), count);
    _bytes_written += count;
}

template <> void BinaryWriter::write<int16_t>(const int16_t* i16, size_t count) {
    write_primitive(i16, count);
}

template <> void BinaryWriter::write<uint16_t>(const uint16_t* u16, size_t count) {
    write_primitive(u16, count);
}

template <> void BinaryWriter::write<int32_t>(const int32_t* i32, size_t count) {
    write_primitive(i32, count);
}

template <> void BinaryWriter::write<uint32_t>(const uint32_t* u32, size_t count) {
    write_primitive(u32, count);
}

template <> void BinaryWriter::write<int64_t>(const int64_t* i64, size_t count) {
    write_primitive(i64, count);
}

template <> void BinaryWriter::write<uint64_t>(const uint64_t* u64, size_t count) {
    write_primitive(u64, count);
}

void BinaryWriter::pad(size_t bytes) {
    char c = '\0';
    for (size_t i = 0; i < bytes; ++i) {
        write_bytes(&c, 1);
    }
    _bytes_written += bytes;
}

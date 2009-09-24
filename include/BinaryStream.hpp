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

class BinaryReader {
  public:
    virtual ~BinaryReader();

    template <typename T>
    void read(T* t, size_t count = 1);

    void discard(size_t bytes);

    size_t bytes_read() const { return _bytes_read; }

  protected:
    BinaryReader();
    virtual void read_bytes(char* bytes, size_t count) = 0;

  private:
    template <typename T>
    void read_primitive(T* t, size_t count = 1);

    size_t _bytes_read;
};

template <> void BinaryReader::read<bool>(bool* b, size_t count);
template <> void BinaryReader::read<char>(char* c, size_t count);
template <> void BinaryReader::read<unsigned char>(unsigned char* uc, size_t count);
template <> void BinaryReader::read<int8_t>(int8_t* i8, size_t count);
template <> void BinaryReader::read<int16_t>(int16_t* i16, size_t count);
template <> void BinaryReader::read<uint16_t>(uint16_t* u16, size_t count);
template <> void BinaryReader::read<int32_t>(int32_t* i32, size_t count);
template <> void BinaryReader::read<uint32_t>(uint32_t* u32, size_t count);
template <> void BinaryReader::read<int64_t>(int64_t* i64, size_t count);
template <> void BinaryReader::read<uint64_t>(uint64_t* u64, size_t count);

template <typename T>
void BinaryReader::read(T* t, size_t count) {
    for (size_t i = 0; i < count; ++i) {
        t[i].read(this);
    }
}

class BufferBinaryReader : public BinaryReader {
  public:
    BufferBinaryReader(const char* data, size_t len);

  protected:
    virtual void read_bytes(char* bytes, size_t count);

  private:
    const char* _data;
    const size_t _len;

    DISALLOW_COPY_AND_ASSIGN(BufferBinaryReader);
};

class BinaryWriter {
  public:
    virtual ~BinaryWriter();

    template <typename T>
    void write(const T& t);

    template <typename T>
    void write(const T* t, size_t count);

    void pad(size_t bytes);

    size_t bytes_written() const;

  protected:
    BinaryWriter();
    virtual void write_bytes(const char* bytes, size_t count) = 0;

    size_t _bytes_written;

  private:
    template <typename T>
    void write_primitive(T* t, size_t count);
};

template <> void BinaryWriter::write<bool>(const bool* b, size_t count);
template <> void BinaryWriter::write<char>(const char* c, size_t count);
template <> void BinaryWriter::write<unsigned char>(const unsigned char* uc, size_t count);
template <> void BinaryWriter::write<int8_t>(const int8_t* i8, size_t count);
template <> void BinaryWriter::write<int16_t>(const int16_t* i16, size_t count);
template <> void BinaryWriter::write<uint16_t>(const uint16_t* u16, size_t count);
template <> void BinaryWriter::write<int32_t>(const int32_t* i32, size_t count);
template <> void BinaryWriter::write<uint32_t>(const uint32_t* u32, size_t count);
template <> void BinaryWriter::write<int64_t>(const int64_t* i64, size_t count);
template <> void BinaryWriter::write<uint64_t>(const uint64_t* u64, size_t count);

template <typename T>
void BinaryWriter::write(const T& t) {
    write(&t, 1);
}

template <typename T>
void BinaryWriter::write(const T* t, size_t count) {
    for (size_t i = 0; i < count; ++i) {
        t[i].write(this);
    }
}

#endif  // ANTARES_BINARY_STREAM_HPP_

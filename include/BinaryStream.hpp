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

namespace antares {

// Interface for reading binary data.
//
// All primitive data read by BinaryReader subclasses is translated from network byte order to the
// host's byte order.
//
// In addition to being able to read primitive data types, BinaryReader objects can read data from
// complex types which conform to the binary-readable concept.  Conforming classes must expose a
// method `void read(BinaryReader* bin)`, which uses `bin` to read in a representation of the
// object.
class BinaryReader {
  public:
    BinaryReader();
    virtual ~BinaryReader();

    // Reads `count` values into `t`.
    //
    // The type of `t` must be one of the primitive types `bool`, `char`, or an integral type
    // `int<N>_t`, or a complex type which conforms to the binary-readable concept (see class
    // documentation).
    //
    // @param [out] t       An array of at least `count` objects of type `T` to read.
    // @param [in] count    The number of objects to read into `t`.
    template <typename T>
    void read(T* t, size_t count = 1);

    // Reads `bytes` bytes and discards them.
    //
    // @param [in] bytes    The number of bytes to discard.
    void discard(size_t bytes);

    // @returns             The number of bytes that have been read using this object.
    size_t bytes_read() const { return _bytes_read; }

  protected:
    // Reads `count` bytes into `bytes`.
    //
    // This is the method which must be implemented by subclasses.  
    //
    // @param [out] bytes   An array of at least `count` bytes to read.
    // @param [in] count    The number of bytes to read.
    virtual void read_bytes(char* bytes, size_t count) = 0;

  private:
    // Reads `count` primitives into `t`.
    //
    // Identical to `read()`, except that it only supports primitives in its selection of `T`.
    template <typename T>
    void read_primitive(T* t, size_t count = 1);

    // The number of bytes that have been read using this object.  Maintained internally by
    // `read_primitive()` so that subclasses don't have to.
    size_t _bytes_read;
};

// Reads binary data from an in-memory buffer.
//
// Uses an in-memory buffer of data as a source for BinaryReader.  The user of this object must not
// attempt to read more data from the buffer than it contains.
class BufferBinaryReader : public BinaryReader {
  public:
    // @param [in] data     The buffer to read from.
    // @param [in] len      The number of bytes in `data`.
    BufferBinaryReader(const char* data, size_t len);

  protected:
    // Implementation of virtual BinaryReader methods.
    virtual void read_bytes(char* bytes, size_t count);

  private:
    // The buffer to read form.  The next piece of data to be read is at position `bytes_read()`
    // within `_data`.
    const char* const _data;

    // The number of bytes in `data`.  When `bytes_read()` is equal to `_len`, the buffer is
    // exhausted.
    const size_t _len;

    DISALLOW_COPY_AND_ASSIGN(BufferBinaryReader);
};

// Interface for writing binary data.
//
// All primitive data written by BinaryWriter subclasses is translated from the host's byte order
// to network byte order.
//
// In addition to being able to write primitive data types, BinaryWriter objects can write data
// from complex types which conform to the binary-writable concept.  Conforming classes must expose
// a method `void write(BinaryWriter* bin)`, which uses `bin` to write in a representation of the
// object.
class BinaryWriter {
  public:
    BinaryWriter();
    virtual ~BinaryWriter();

    // Writes `t`.
    //
    // The type of `t` must be one of the primitive types `bool`, `char`, or an integral type
    // `int<N>_t`, or a complex type which conforms to the binary-writable concept (see class
    // documentation).
    //
    // @param [in] t        The value to write.
    template <typename T>
    void write(const T& t);

    // Writes `count` values from `t`.
    //
    // The type of `t` must be one of the primitive types `bool`, `char`, or an integral type
    // `int<N>_t`, or a complex type which conforms to the binary-writable concept (see class
    // documentation).
    //
    // @param [in] t        An array of at least `count` objects of type `T` to write.
    // @param [in] count    The number of objects to write from `t`.
    template <typename T>
    void write(const T* t, size_t count);

    // Writes `bytes` bytes of blank data.
    //
    // @param [in] bytes    The number of bytes to pad with.
    void pad(size_t bytes);

    // @returns             The number of bytes that have been written using this object.
    size_t bytes_written() const;

  protected:
    // Reads `count` bytes into `bytes`.
    //
    // This is the method which must be implemented by subclasses.  
    //
    // @param [in] bytes    An array of at least `count` bytes to write.
    // @param [in] count    The number of bytes to write.
    virtual void write_bytes(const char* bytes, size_t count) = 0;

  private:
    // Writes `count` primitives from `t`.
    //
    // Identical to `write()`, except that it only supports primitives in its selection of `T`.
    template <typename T>
    void write_primitive(const T* t, size_t count);

    // The number of bytes that have been written using this object.  Maintained internally by
    // `write_primitive()` so that subclasses don't have to.
    size_t _bytes_written;
};

// Implementation details follow.

// Specializations of `BinaryReader::read()` for primitive types.
template <> void BinaryReader::read<bool>(bool* b, size_t count);
template <> void BinaryReader::read<char>(char* c, size_t count);
template <> void BinaryReader::read<int8_t>(int8_t* i8, size_t count);
template <> void BinaryReader::read<uint8_t>(uint8_t* u8, size_t count);
template <> void BinaryReader::read<int16_t>(int16_t* i16, size_t count);
template <> void BinaryReader::read<uint16_t>(uint16_t* u16, size_t count);
template <> void BinaryReader::read<int32_t>(int32_t* i32, size_t count);
template <> void BinaryReader::read<uint32_t>(uint32_t* u32, size_t count);
template <> void BinaryReader::read<int64_t>(int64_t* i64, size_t count);
template <> void BinaryReader::read<uint64_t>(uint64_t* u64, size_t count);

// Default implementation of `BinaryReader::read()` for complex types.
template <typename T>
void BinaryReader::read(T* t, size_t count) {
    for (size_t i = 0; i < count; ++i) {
        t[i].read(this);
    }
}

// Specializations of `BinaryWriter::write()` for primitive types.
template <> void BinaryWriter::write<bool>(const bool* b, size_t count);
template <> void BinaryWriter::write<char>(const char* c, size_t count);
template <> void BinaryWriter::write<int8_t>(const int8_t* i8, size_t count);
template <> void BinaryWriter::write<uint8_t>(const uint8_t* u8, size_t count);
template <> void BinaryWriter::write<int16_t>(const int16_t* i16, size_t count);
template <> void BinaryWriter::write<uint16_t>(const uint16_t* u16, size_t count);
template <> void BinaryWriter::write<int32_t>(const int32_t* i32, size_t count);
template <> void BinaryWriter::write<uint32_t>(const uint32_t* u32, size_t count);
template <> void BinaryWriter::write<int64_t>(const int64_t* i64, size_t count);
template <> void BinaryWriter::write<uint64_t>(const uint64_t* u64, size_t count);

// Default implementation of `BinaryWriter::write()` for single instances of complex types.
// Forwards the call to the array case of `BinaryWriter::write()` as an array of size 1.
template <typename T>
void BinaryWriter::write(const T& t) {
    write(&t, 1);
}

// Default implementation of `BinaryWriter::write()` for arrays of complex types.
template <typename T>
void BinaryWriter::write(const T* t, size_t count) {
    for (size_t i = 0; i < count; ++i) {
        t[i].write(this);
    }
}

}  // namespace antares

#endif  // ANTARES_BINARY_STREAM_HPP_

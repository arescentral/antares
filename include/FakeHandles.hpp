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

#ifndef ANTARES_FAKE_HANDLES_HPP_
#define ANTARES_FAKE_HANDLES_HPP_

#include <string.h>
#include <Base.h>

#include "Fakes.hpp"

class HandleBase {
  public:
    virtual ~HandleBase() { }

    Handle ToHandle() {
        return reinterpret_cast<Handle>(this + 1);
    }

    static HandleBase* FromHandle(Handle h) {
        return reinterpret_cast<HandleBase*>(h - 1);
    }

    virtual HandleBase* Clone() = 0;
    virtual void* Data() const = 0;
    virtual int Size() const = 0;
};

template <typename T>
class HandleData : public HandleBase {
  public:
    explicit HandleData()
        : _data(new T) { }

    explicit HandleData(const T& t)
        : _data(new T(t)) { }

    ~HandleData() {
        delete _data;
    }

    virtual HandleData* Clone() {
        return new HandleData(*_data);
    }

    virtual void* Data() const { return _data; }
    virtual int Size() const { return sizeof(T); }

    T* TypedData() const { return _data; }

    T** ToTypedHandle() {
        return reinterpret_cast<T**>(ToHandle());
    }

    HandleData* FromTypedHandle(T** handle) {
        return reinterpret_cast<HandleData*>(FromHandle(handle));
    }

  private:
    T* _data;

    DISALLOW_COPY_AND_ASSIGN(HandleData);
};

template <>
class HandleData<void> : public HandleBase {
  public:
    explicit HandleData(int size)
        : _data(new char[size]),
          _size(size) { }

    explicit HandleData(int size, void* src)
        : _data(new char[size]),
          _size(size) {
        memcpy(_data, src, size);
    }

    ~HandleData() {
        delete[] _data;
    }

    virtual HandleData* Clone() {
        return new HandleData(_size, _data);
    }

    virtual void* Data() const { return _data; }
    virtual int Size() const { return _size; }

  private:
    char* _data;
    int _size;

    DISALLOW_COPY_AND_ASSIGN(HandleData);
};

void FakeHandlesInit();

#endif  // ANTARES_FAKE_HANDLES_HPP_

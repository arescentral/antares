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

#ifndef ANTARES_SMART_PTR_HPP_
#define ANTARES_SMART_PTR_HPP_

#include <stdlib.h>
#include <algorithm>

namespace antares {

#define DISALLOW_COPY_AND_ASSIGN(CLASS) \
  private: \
    CLASS(const CLASS&); \
    CLASS& operator=(const CLASS&);

template <typename T>
class scoped_ptr {
  public:
    explicit scoped_ptr(T* ptr = NULL) : _ptr(ptr) { }

    ~scoped_ptr() { reset(); }

    T* get() const { return _ptr; }
    T* operator->() const { return _ptr; }
    T& operator*() const { return *_ptr; }

    T* release() {
        T* ptr = _ptr;
        _ptr = NULL;
        return ptr;
    }

    void reset(T* new_ptr = NULL) {
        if (_ptr) {
            delete _ptr;
        }
        _ptr = new_ptr;
    }

    void swap(scoped_ptr<T>* s) {
        std::swap(_ptr, s->_ptr);
    }

  private:
    T* _ptr;

    DISALLOW_COPY_AND_ASSIGN(scoped_ptr);
};

template <typename T>
class scoped_array {
  public:
    explicit scoped_array(T* ptr = NULL) : _ptr(ptr) { }

    ~scoped_array() { reset(); }

    T* get() const { return _ptr; }
    T* operator->() const { return _ptr; }
    T& operator*() const { return *_ptr; }

    T* release() {
        T* ptr = _ptr;
        _ptr = NULL;
        return ptr;
    }

    void reset(T* new_ptr = NULL) {
        if (_ptr) {
            delete [] _ptr;
        }
        _ptr = new_ptr;
    }

    void swap(scoped_array<T>* s) {
        std::swap(_ptr, s->_ptr);
    }

  private:
    T* _ptr;

    DISALLOW_COPY_AND_ASSIGN(scoped_array);
};

}  // namespace antares

#endif  // ANTARES_SMART_PTR_HPP_

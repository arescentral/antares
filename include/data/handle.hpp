// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// Copyright (C) 2015-2017 The Antares Authors
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

#ifndef ANTARES_DATA_HANDLE_HPP_
#define ANTARES_DATA_HANDLE_HPP_

#include <stdlib.h>
#include <pn/string>

namespace antares {

class Admiral;
class BaseObject;
struct Destination;
class Label;
class SpaceObject;
class Sprite;
struct Vector;

template <typename T>
class Handle {
  public:
    Handle() : _number(-1) {}
    explicit Handle(int number) : _number(number) {}
    int number() const { return _number; }
    T*  get() const { return T::get(_number); }
    T&  operator*() const { return *get(); }
    T*  operator->() const { return get(); }

  private:
    int _number;
};
template <typename T>
inline bool operator==(Handle<T> x, Handle<T> y) {
    return x.number() == y.number();
}
template <typename T>
inline bool operator!=(Handle<T> x, Handle<T> y) {
    return !(x == y);
}

template <typename T>
class HandleList {
  public:
    HandleList() : HandleList(-1, -1) {}
    HandleList(int begin, int end) : _begin(begin), _end(end) {}
    size_t size() const { return _end - _begin; }
    class iterator {
        friend class HandleList;

      public:
        Handle<T> operator*() const { return Handle<T>(_number); }
        iterator& operator++() {
            ++_number;
            return *this;
        }
        iterator operator++(int) { return iterator(_number++); }
        bool     operator==(iterator other) const { return _number == other._number; }
        bool     operator!=(iterator other) const { return _number != other._number; }

      private:
        explicit iterator(int number) : _number(number) {}
        int _number;
    };
    iterator begin() const { return iterator(_begin); }
    iterator end() const { return iterator(_end); }

  private:
    int _begin;
    int _end;
};

template <typename T>
class NamedHandle {
  public:
    NamedHandle() : _name() {}
    explicit NamedHandle(pn::string_view name) : _name(name.copy()) {}
    NamedHandle     copy() const { return NamedHandle(_name.copy()); }
    pn::string_view name() const { return _name; }
    T*              get() const { return T::get(_name); }
    T&              operator*() const { return *get(); }
    T*              operator->() const { return get(); }

  private:
    pn::string _name;
};
template <typename T>
inline bool operator==(NamedHandle<T> x, NamedHandle<T> y) {
    return x.name() == y.name();
}
template <typename T>
inline bool operator!=(NamedHandle<T> x, NamedHandle<T> y) {
    return !(x == y);
}

}  // namespace antares

#endif  // ANTARES_DATA_HANDLE_HPP_

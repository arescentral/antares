// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// Copyright (C) 2015 The Antares Authors
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

namespace antares {

struct Admiral;

template <typename T>
class Handle {
  public:
    Handle(): _number(-1) { }
    Handle(int number): _number(number) { }
    int number() const { return _number; }
    T* get() const { return T::get(_number); }
    T& operator*() const { return *get(); }
    T* operator->() const { return get(); }
  private:
    int _number;
};
template <typename T>
inline bool operator==(Handle<T> x, Handle<T> y) { return x.number() == y.number(); }
template <typename T>
inline bool operator!=(Handle<T> x, Handle<T> y) { return !(x == y); }

}  // namespace antares

#endif // ANTARES_DATA_HANDLE_HPP_

// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// Copyright (C) 2008-2011 Ares Central
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
// License along with this program.  If not, see
// <http://www.gnu.org/licenses/>.

#ifndef ANTARES_COCOA_CORE_FOUNDATION_HPP_
#define ANTARES_COCOA_CORE_FOUNDATION_HPP_

#include <algorithm>
#include <CoreFoundation/CoreFoundation.h>
#include <sfz/sfz.hpp>

namespace antares {
namespace cf {

template <typename T>
class UnownedObject {
  public:
    typedef T type;

    UnownedObject(): _c_obj(NULL) { }
    UnownedObject(type c_obj): _c_obj(c_obj) { }

    type c_obj() const { return _c_obj; }
    type& c_obj() { return _c_obj; }

    type release() {
        using std::swap;
        type result = NULL;
        swap(_c_obj, result);
        return result;
    }

    void reset(type c_obj = NULL) {
        _c_obj = c_obj;
    }

  private:
    type _c_obj;

    DISALLOW_COPY_AND_ASSIGN(UnownedObject);
};

template <typename T>
class Object : public UnownedObject<T> {
  public:
    typedef T type;

    Object() { }
    Object(type c_obj): UnownedObject<T>(c_obj) { }
    ~Object() { reset(); }

    void reset(type c_obj = NULL) {
        if (UnownedObject<T>::c_obj()) {
            CFRelease(UnownedObject<T>::c_obj());
        }
        UnownedObject<T>::reset(c_obj);
    }
};

template <typename From, typename To>
bool move(To& to, From& from) {
    if (CFGetTypeID(from.c_obj()) == To::type_id()) {
        to.reset(reinterpret_cast<typename To::type>(from.release()));
        return true;
    }
    return false;
}

class Type : public Object<CFTypeRef> {
  public:
    Type();
    Type(type value);
};

class Boolean : public UnownedObject<CFBooleanRef> {
  public:
    static CFTypeID type_id();
    Boolean();
    Boolean(bool value);
    Boolean(type c_obj);
};

class Number : public Object<CFNumberRef> {
  public:
    static CFTypeID type_id();
    Number();
    Number(type c_obj);
};

class String : public Object<CFStringRef> {
  public:
    static CFTypeID type_id();
    String();
    String(type c_obj);
    String(const sfz::StringSlice& string);
};
void print_to(sfz::PrintTarget out, const String& string);

class Array : public Object<CFArrayRef> {
  public:
    static CFTypeID type_id();
    Array();
    Array(type c_obj);
};

class MutableArray : public Object<CFMutableArrayRef> {
  public:
    static CFTypeID type_id();
    MutableArray();
    MutableArray(type c_obj);
};

class Data : public Object<CFDataRef> {
  public:
    static CFTypeID type_id();
    Data();
    Data(type c_obj);

    sfz::BytesSlice data() const;
};
void write_to(sfz::WriteTarget out, const Data& data);

class PropertyList : public Object<CFPropertyListRef> {
  public:
    PropertyList();
    PropertyList(type c_obj);
};

class Url : public Object<CFURLRef> {
  public:
    static CFTypeID type_id();
    Url();
    Url(type c_obj);
    Url(const sfz::StringSlice& string);
};

}  // namespace cf
}  // namespace antares

#endif  // ANTARES_COCOA_CORE_FOUNDATION_HPP_

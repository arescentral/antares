// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// Copyright (C) 2008-2012 The Antares Authors
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

#ifndef ANTARES_COCOA_CORE_FOUNDATION_HPP_
#define ANTARES_COCOA_CORE_FOUNDATION_HPP_

#include <algorithm>
#include <initializer_list>
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
    UnownedObject(UnownedObject&& other): _c_obj(other.release()) { }
    UnownedObject& operator=(UnownedObject&& other) { reset(other.release()); return *this; }

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
    Object(Object&& other): UnownedObject<T>(other.release()) { }
    Object& operator=(Object&& other) { reset(other.release()); return *this; }
    ~Object() { reset(); }

    void reset(type c_obj = NULL) {
        if (UnownedObject<T>::c_obj()) {
            CFRelease(UnownedObject<T>::c_obj());
        }
        UnownedObject<T>::reset(c_obj);
    }
};

template <typename To, typename From>
To cast(From from) {
    if (CFGetTypeID(from.c_obj()) == To::type_id()) {
        return To(reinterpret_cast<typename To::type>(from.release()));
    }
    return nullptr;
}

class Type : public Object<CFTypeRef> {
  public:
    Type();
    Type(type value);
    Type(Type&&) = default;
    Type& operator=(Type&&) = default;
};

class Boolean : public UnownedObject<CFBooleanRef> {
  public:
    static CFTypeID type_id();
    Boolean();
    Boolean(bool value);
    Boolean(type c_obj);
    Boolean(Boolean&&) = default;
    Boolean& operator=(Boolean&&) = default;
};

class Number : public Object<CFNumberRef> {
  public:
    static CFTypeID type_id();
    Number();
    Number(type c_obj);
    Number(Number&&) = default;
    Number& operator=(Number&&) = default;

    static Number of_int(int i);
};

class String : public Object<CFStringRef> {
  public:
    static CFTypeID type_id();
    String();
    String(type c_obj);
    String(const sfz::StringSlice& string);
    String(String&&) = default;
    String& operator=(String&&) = default;
};
void print_to(sfz::PrintTarget out, const String& string);

class Array : public Object<CFArrayRef> {
  public:
    static CFTypeID type_id();
    Array();
    Array(type c_obj);
    Array(Array&&) = default;
    Array& operator=(Array&&) = default;
};

class MutableArray : public Object<CFMutableArrayRef> {
  public:
    static CFTypeID type_id();
    MutableArray();
    MutableArray(type c_obj);
    MutableArray(MutableArray&&) = default;
    MutableArray& operator=(MutableArray&&) = default;
};

class Dictionary : public Object<CFDictionaryRef> {
  public:
    static CFTypeID type_id();
    Dictionary();
    Dictionary(type c_obj);
    Dictionary(std::initializer_list<std::pair<cf::String, cf::Number>> items);
    Dictionary(Dictionary&&) = default;
    Dictionary& operator=(Dictionary&&) = default;
};

class Data : public Object<CFDataRef> {
  public:
    static CFTypeID type_id();
    Data();
    Data(type c_obj);
    Data(Data&&) = default;
    Data& operator=(Data&&) = default;

    sfz::BytesSlice data() const;
};
void write_to(sfz::WriteTarget out, const Data& data);

class PropertyList : public Object<CFPropertyListRef> {
  public:
    PropertyList();
    PropertyList(type c_obj);
    PropertyList(PropertyList&&) = default;
    PropertyList& operator=(PropertyList&&) = default;
};

class Url : public Object<CFURLRef> {
  public:
    static CFTypeID type_id();
    Url();
    Url(type c_obj);
    Url(const sfz::StringSlice& string);
    Url(Url&&) = default;
    Url& operator=(Url&&) = default;
};

}  // namespace cf
}  // namespace antares

#endif  // ANTARES_COCOA_CORE_FOUNDATION_HPP_

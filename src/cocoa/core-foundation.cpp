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

#include "cocoa/core-foundation.hpp"

#include <CoreFoundation/CoreFoundation.h>
#include <sfz/sfz.hpp>

using sfz::Bytes;
using sfz::BytesSlice;
using sfz::CString;
using sfz::StringSlice;

namespace utf8 = sfz::utf8;

namespace antares {
namespace cf {

///////////////////////////////////////////////////////////////////////////////////////////////////
// Type

Type::Type() { }

Type::Type(type c_obj):
    Object<CFTypeRef>(c_obj) { }

///////////////////////////////////////////////////////////////////////////////////////////////////
// Boolean

CFTypeID Boolean::type_id() { return CFBooleanGetTypeID(); }

Boolean::Boolean() { }

Boolean::Boolean(bool value):
    UnownedObject<CFBooleanRef>(value ? kCFBooleanTrue : kCFBooleanFalse) { }

Boolean::Boolean(type c_obj):
    UnownedObject<CFBooleanRef>(c_obj) { }

///////////////////////////////////////////////////////////////////////////////////////////////////
// Number

CFTypeID Number::type_id() { return CFNumberGetTypeID(); }

Number::Number() { }

Number::Number(type c_obj):
    Object<CFNumberRef>(c_obj) { }

///////////////////////////////////////////////////////////////////////////////////////////////////
// String

namespace {

CFStringRef create_string(const StringSlice& string) {
    Bytes bytes(utf8::encode(string));
    return CFStringCreateWithBytes(NULL, bytes.data(), bytes.size(), kCFStringEncodingUTF8, NULL);
}

}  // namespace

CFTypeID String::type_id() { return CFStringGetTypeID(); }

String::String() { }

String::String(type c_obj):
    Object<CFStringRef>(c_obj) { }

String::String(const sfz::StringSlice& string):
    Object<CFStringRef>(create_string(string)) { }

void print_to(sfz::PrintTarget out, const String& string) {
    cf::Data encoded(CFStringCreateExternalRepresentation(
                NULL, string.c_obj(), kCFStringEncodingUTF8, '?'));
    print(out, utf8::decode(encoded.data()));
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Array

CFTypeID Array::type_id() { return CFArrayGetTypeID(); }

Array::Array() { }

Array::Array(type c_obj):
    Object<CFArrayRef>(c_obj) { }

///////////////////////////////////////////////////////////////////////////////////////////////////
// MutableArray

CFTypeID MutableArray::type_id() { return CFArrayGetTypeID(); }

MutableArray::MutableArray() { }

MutableArray::MutableArray(type c_obj):
    Object<CFMutableArrayRef>(c_obj) { }

///////////////////////////////////////////////////////////////////////////////////////////////////
// Data

CFTypeID Data::type_id() { return CFDataGetTypeID(); }

Data::Data() { }

Data::Data(type c_obj):
    Object<CFDataRef>(c_obj) { }

sfz::BytesSlice Data::data() const {
    return sfz::BytesSlice(CFDataGetBytePtr(c_obj()), CFDataGetLength(c_obj()));
}

void write_to(sfz::WriteTarget out, const Data& data) {
    write(out, data.data());
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// PropertyList

PropertyList::PropertyList() { }

PropertyList::PropertyList(type c_obj):
    Object<CFPropertyListRef>(c_obj) { }

///////////////////////////////////////////////////////////////////////////////////////////////////
// Url

namespace {

CFURLRef create_url(const StringSlice& string) {
    Bytes bytes(utf8::encode(string));
    return CFURLCreateWithBytes(NULL, bytes.data(), bytes.size(), kCFStringEncodingUTF8, NULL);
}

}  // namespace

CFTypeID Url::type_id() { return CFURLGetTypeID(); }

Url::Url() { }

Url::Url(type c_obj):
    Object<CFURLRef>(c_obj) { }

Url::Url(const sfz::StringSlice& string):
    Object<CFURLRef>(create_url(string)) { }

}  // namespace cf
}  // namespace antares

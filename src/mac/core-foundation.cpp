// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// Copyright (C) 2008-2017 The Antares Authors
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

#include "mac/core-foundation.hpp"

#include <CoreFoundation/CoreFoundation.h>
#include <pn/data>

using std::unique_ptr;

namespace antares {
namespace cf {

///////////////////////////////////////////////////////////////////////////////////////////////////
// Type

Type::Type() {}

Type::Type(type c_obj) : Object<CFTypeRef>(c_obj) {}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Boolean

CFTypeID Boolean::type_id() { return CFBooleanGetTypeID(); }

Boolean::Boolean() {}

Boolean::Boolean(type c_obj) : UnownedObject<CFBooleanRef>(c_obj) {}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Number

CFTypeID Number::type_id() { return CFNumberGetTypeID(); }

Number::Number() {}

Number::Number(type c_obj) : Object<CFNumberRef>(c_obj) {}

///////////////////////////////////////////////////////////////////////////////////////////////////
// String

CFTypeID String::type_id() { return CFStringGetTypeID(); }

String::String() {}

String::String(type c_obj) : Object<CFStringRef>(c_obj) {}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Array

CFTypeID Array::type_id() { return CFArrayGetTypeID(); }

Array::Array() {}

Array::Array(type c_obj) : Object<CFArrayRef>(c_obj) {}

size_t Array::size() const { return CFArrayGetCount(c_obj()); }

const void* Array::get(size_t index) const { return CFArrayGetValueAtIndex(c_obj(), index); }

///////////////////////////////////////////////////////////////////////////////////////////////////
// MutableArray

CFTypeID MutableArray::type_id() { return CFArrayGetTypeID(); }

MutableArray::MutableArray() {}

MutableArray::MutableArray(type c_obj) : Object<CFMutableArrayRef>(c_obj) {}

size_t MutableArray::size() const { return CFArrayGetCount(c_obj()); }

const void* MutableArray::get(size_t index) const {
    return CFArrayGetValueAtIndex(c_obj(), index);
}

void MutableArray::append(const void* key) { CFArrayAppendValue(c_obj(), key); }

///////////////////////////////////////////////////////////////////////////////////////////////////
// Dictionary

CFTypeID Dictionary::type_id() { return CFDictionaryGetTypeID(); }

Dictionary::Dictionary() {}

Dictionary::Dictionary(type c_obj) : Object<CFDictionaryRef>(c_obj) {}

///////////////////////////////////////////////////////////////////////////////////////////////////
// MutableDictionary

CFTypeID MutableDictionary::type_id() { return CFDictionaryGetTypeID(); }

MutableDictionary::MutableDictionary() {}

MutableDictionary::MutableDictionary(type c_obj) : Object<CFMutableDictionaryRef>(c_obj) {}

void MutableDictionary::set(const void* key, const void* value) {
    CFDictionarySetValue(c_obj(), key, value);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Data

CFTypeID Data::type_id() { return CFDataGetTypeID(); }

Data::Data() {}

Data::Data(type c_obj) : Object<CFDataRef>(c_obj) {}

pn::data_view Data::data() const {
    return pn::data_view(CFDataGetBytePtr(c_obj()), CFDataGetLength(c_obj()));
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// PropertyList

PropertyList::PropertyList() {}

PropertyList::PropertyList(type c_obj) : Object<CFPropertyListRef>(c_obj) {}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Url

namespace {

CFURLRef create_url(pn::string_view string) {
    return CFURLCreateWithBytes(
            NULL, reinterpret_cast<const uint8_t*>(string.data()), string.size(),
            kCFStringEncodingUTF8, NULL);
}

}  // namespace

CFTypeID Url::type_id() { return CFURLGetTypeID(); }

Url::Url() {}

Url::Url(type c_obj) : Object<CFURLRef>(c_obj) {}

Url::Url(pn::string_view string) : Object<CFURLRef>(create_url(string)) {}

///////////////////////////////////////////////////////////////////////////////////////////////////
// wrap()

Boolean wrap(bool value) {
    if (value) {
        return Boolean(kCFBooleanTrue);
    } else {
        return Boolean(kCFBooleanFalse);
    }
}

Number wrap(short value) { return Number(CFNumberCreate(NULL, kCFNumberShortType, &value)); }

Number wrap(int value) { return Number(CFNumberCreate(NULL, kCFNumberIntType, &value)); }

Number wrap(long value) { return Number(CFNumberCreate(NULL, kCFNumberLongType, &value)); }

Number wrap(long long value) {
    return Number(CFNumberCreate(NULL, kCFNumberLongLongType, &value));
}

Number wrap(float value) { return Number(CFNumberCreate(NULL, kCFNumberFloatType, &value)); }

Number wrap(double value) { return Number(CFNumberCreate(NULL, kCFNumberDoubleType, &value)); }

String wrap(pn::string_view value) {
    return String(CFStringCreateWithBytes(
            NULL, reinterpret_cast<const uint8_t*>(value.data()), value.size(),
            kCFStringEncodingUTF8, false));
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// unwrap()

bool unwrap(const Boolean& cfvalue, bool& value) {
    if (!cfvalue.c_obj()) {
        return false;
    }
    value = (cfvalue.c_obj() == kCFBooleanTrue);
    return true;
}

bool unwrap(const Number& cfvalue, short& value) {
    if (!cfvalue.c_obj()) {
        return false;
    }
    return CFNumberGetValue(cfvalue.c_obj(), kCFNumberShortType, &value);
}

bool unwrap(const Number& cfvalue, int& value) {
    if (!cfvalue.c_obj()) {
        return false;
    }
    return CFNumberGetValue(cfvalue.c_obj(), kCFNumberIntType, &value);
}

bool unwrap(const Number& cfvalue, long& value) {
    if (!cfvalue.c_obj()) {
        return false;
    }
    return CFNumberGetValue(cfvalue.c_obj(), kCFNumberLongType, &value);
}

bool unwrap(const Number& cfvalue, long long& value) {
    if (!cfvalue.c_obj()) {
        return false;
    }
    return CFNumberGetValue(cfvalue.c_obj(), kCFNumberLongLongType, &value);
}

bool unwrap(const Number& cfvalue, float& value) {
    if (!cfvalue.c_obj()) {
        return false;
    }
    return CFNumberGetValue(cfvalue.c_obj(), kCFNumberFloatType, &value);
}

bool unwrap(const Number& cfvalue, double& value) {
    if (!cfvalue.c_obj()) {
        return false;
    }
    return CFNumberGetValue(cfvalue.c_obj(), kCFNumberDoubleType, &value);
}

bool unwrap(const String& cfvalue, pn::string& value) {
    if (!cfvalue.c_obj()) {
        return false;
    }
    Data encoded(CFStringCreateExternalRepresentation(
            NULL, cfvalue.c_obj(), kCFStringEncodingUTF8, '?'));
    value = pn::string(
            reinterpret_cast<const char*>(encoded.data().data()), encoded.data().size());
    return true;
}

}  // namespace cf
}  // namespace antares

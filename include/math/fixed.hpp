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

#ifndef ANTARES_MATH_FIXED_HPP_
#define ANTARES_MATH_FIXED_HPP_

#include <math.h>
#include <pn/file>
#include <pn/string>

namespace antares {

class Fixed {
  public:
    Fixed() = default;

    static constexpr Fixed from_long(int32_t x) { return Fixed(x << 8); }
    static constexpr Fixed from_float(double x) { return Fixed(round(x * 256.0)); }
    static constexpr Fixed from_val(int32_t value) { return Fixed(value); }
    static constexpr Fixed zero() { return Fixed(0); }

    int32_t val() const { return _value; }

    bool operator==(Fixed y) const { return _value == y._value; }
    bool operator!=(Fixed y) const { return _value != y._value; }
    bool operator<(Fixed y) const { return _value < y._value; }
    bool operator<=(Fixed y) const { return _value <= y._value; }
    bool operator>(Fixed y) const { return _value > y._value; }
    bool operator>=(Fixed y) const { return _value >= y._value; }

    Fixed operator+(Fixed y) const { return Fixed{_value + y._value}; }
    Fixed operator-(Fixed y) const { return Fixed{_value - y._value}; }
    Fixed operator%(Fixed y) const { return Fixed{_value % y._value}; }
    Fixed operator*(int32_t y) const { return Fixed{_value * y}; }
    Fixed operator/(int32_t y) const { return Fixed{_value / y}; }
    Fixed operator<<(int n) const { return Fixed{_value << n}; }
    Fixed operator>>(int n) const { return Fixed{_value >> n}; }

    // the max safe # we can do is 181 for signed multiply if we don't know other value
    // if -1 <= other value <= 1 then we can do 32767
    Fixed operator*(Fixed y) const { return Fixed{_value * y._value} >> 8; }
    Fixed operator/(Fixed y) const { return Fixed{_value << 8} / y._value; }
    Fixed operator%(int32_t y) const { return Fixed{*this % Fixed::from_long(y)}; }

    Fixed& operator+=(Fixed y) { return *this = *this + y; }
    Fixed& operator-=(Fixed y) { return *this = *this - y; }
    Fixed& operator*=(Fixed y) { return *this = *this * y; }
    Fixed& operator*=(int32_t y) { return *this = *this * y; }
    Fixed& operator/=(Fixed y) { return *this = *this / y; }
    Fixed& operator/=(int32_t y) { return *this = *this / y; }
    Fixed& operator<<=(int n) { return *this = *this << n; }
    Fixed& operator>>=(int n) { return *this = *this >> n; }

    Fixed operator-() const { return Fixed{-_value}; }

  private:
    explicit constexpr Fixed(int32_t value) : _value(value) {}

    int32_t _value;
};

inline Fixed operator*(int32_t x, Fixed y) { return y * x; }

static const Fixed kFixedNone = Fixed::from_val(-1);

//
//  MAX VALUE FOR SMALLFIXEDTYPE:
//
//  8,388,607       normal
//  4,194,303       addition
//  2,896           multiplication -- see below
//  32,768          division
//

// Convert a fixed-point number to int32_t.
//
// Both the evil and more evil variants return correct results when `value` is zero or positive
// (that is, ``value / 256``), but they differ in how wrong they are with negative values.  The
// evil variant returns ``(value / 256) + 1`` when 256 evenly divides `value`, returning correct
// results otherwise.  The more evil variant returns the evil result - 1, returning correct results
// only when 256 evenly divides `value`.
inline int32_t evil_fixed_to_long(Fixed value) {
    if (value < Fixed::zero()) {
        return (value.val() >> 8) + 1;
    } else {
        return value.val() >> 8;
    }
}
inline int32_t more_evil_fixed_to_long(Fixed value) { return (value >> 8).val(); }

inline float   mFixedToFloat(Fixed m_f) { return floorf(m_f.val() * 1e3 / 256.0) / 1e3; }
inline int32_t mFixedToLong(Fixed m_f) { return evil_fixed_to_long(m_f); }

pn::string stringify(Fixed fixed);

struct fixedPointType {
    Fixed h;
    Fixed v;
};

}  // namespace antares

#endif  // ANTARES_MATH_FIXED_HPP_

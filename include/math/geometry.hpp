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

#ifndef ANTARES_MATH_GEOMETRY_HPP_
#define ANTARES_MATH_GEOMETRY_HPP_

#include <stdint.h>
#include <pn/file>
#include <pn/string>

namespace antares {

struct Rect;

// A point (h, v) in two-dimensional space.
struct Point {
    int32_t h;
    int32_t v;

    // Creates a point at (0, 0).
    constexpr Point() : h{0}, v{0} {}

    // Creates a point at (x, y).
    // @param [in] x        The desired value of `h`.
    // @param [in] y        The desired value of `v`.
    constexpr Point(int x, int y) : h{x}, v{y} {}

    // Translates this Point by `(x, y)`.
    //
    // @param [in] x        Added to `h`.
    // @param [in] y        Added to `v`.
    void offset(int32_t x, int32_t y) { h += x, v += y; }

    // Move the point to the nearest point within `rect`.
    // @param [in] rect     The rectangle to clamp to.
    void clamp_to(const Rect& rect);
};

inline bool operator==(const Point& x, const Point& y) { return (x.h == y.h) && (x.v == y.v); }
inline bool operator!=(const Point& x, const Point& y) { return (x.h != y.h) || (x.v != y.v); }

// A size (width, height) in two-dimensional space.
struct Size {
    int32_t width;
    int32_t height;

    // Creates an empty size.
    constexpr Size() : width{0}, height{0} {}

    // Creates a size of (width, height).
    constexpr Size(int32_t width, int32_t height) : width{width}, height{height} {}

    // Returns a rect with origin (0, 0) and this size.
    Rect as_rect() const;
};

inline bool operator==(Size x, Size y) { return (x.width == y.width) && (x.height == y.height); }
inline bool operator!=(Size x, Size y) { return (x.width != y.width) || (x.height != y.height); }

// A rectangle in two-dimensional space.
//
// Rectangles are represented as 4-tuples of (left, top, right, bottom).  The four corners of the
// rectangle are therefore located at (left, top), (right, top), (left, bottom), and (right,
// bottom).  The width is `right - left` and the height is `bottom - top`.
//
// Rect generally assumes the invariant that `right >= left && bottom >= top`.  Methods on Rect
// will return undefined results when this invariant does not hold, and other users of the class
// will probably have unpredictable results as well.
struct Rect {
    int32_t left;
    int32_t top;
    int32_t right;
    int32_t bottom;

    // Creates a zero-area rect at (0, 0).
    constexpr Rect() : left{0}, top{0}, right{0}, bottom{0} {}

    // Creates a rect with the four given parameters.
    // @param [in] left     The desired value of `left`.
    // @param [in] top      The desired value of `top`.
    // @param [in] right    The desired value of `right`.
    // @param [in] bottom   The desired value of `bottom`.
    constexpr Rect(int32_t left, int32_t top, int32_t right, int32_t bottom)
            : left{left}, top{top}, right{right}, bottom{bottom} {}

    // Creates a rect width the two given parameters.
    constexpr Rect(Point origin, Size size)
            : left{origin.h}, top{origin.v}, right{left + size.width}, bottom{top + size.height} {}

    // @returns             True iff the rect is empty (width or height not positive).
    bool empty() const { return (width() <= 0) || (height() <= 0); }

    // @returns             The width of the rectangle, which is `right - left`.
    int32_t width() const { return right - left; }

    // @returns             The height of the rectangle, which is `bottom - top`.
    int32_t height() const { return bottom - top; }

    // @returns             The origin of the rectangle, ``(left, top)``.
    Point origin() const { return Point{left, top}; }

    // @returns             The center of the rectangle, ``((left - right)/2, (top + bottom)/2)``.
    Point center() const;

    // @returns             The size of the rectangle, ``(width, height)``.
    Size size() const { return Size{width(), height()}; }

    // @returns             The area of the rectangle, which is `width() * height()`.
    int32_t area() const { return width() * height(); }

    // Returns true if this Rect contains `p`.
    //
    // A point is contained by a rectangle if the point's x-position is in the range [left, right)
    // and its y-position is in the range [top, bottom).
    //
    // @param [in] p        A Point to test.
    // @returns             true iff `p` is contained within this rect.
    bool contains(const Point& p) const;

    // Returns true if this Rect encloses `r`.
    //
    // @param [in] r        A Rect to test.
    // @returns             true iff all points in `r` are contained within this rect.
    bool encloses(const Rect& r) const;

    // Returns true if this Rect intersects `r`.
    //
    // @param [in] r        A Rect to test.
    // @returns             true iff any point in `r` is contained within this rect.
    bool intersects(const Rect& r) const;

    // Translates this Rect by `(x, y)`.
    //
    // @param [in] x        Added to `left` and `right`.
    // @param [in] y        Added to `top` and `bottom`.
    void offset(int32_t x, int32_t y);

    // Multiplies all components by `(x, y)`
    void scale(int32_t x, int32_t y);

    // Shrinks this Rect by `(x, y)`.
    //
    // Either of `x` and `y` may be negative, which would correspond to enlarging the rectangle in
    // the horizontal or vertical dimensions.  `x` must not be greater than `this->width() / 2`,
    // and `y` must not be greater than `this->height() / 2`.
    //
    // @param [in] x        Added to `left` and subtracted from `right`.
    // @param [in] y        Added to `top` and subtracted from `bottom`.
    void inset(int32_t x, int32_t y);

    // Translate this Rect so that it shares its center with `r`.
    //
    // @param [in] r        The Rect to center this rectangle within.
    void center_in(const Rect& r);

    // Intersect this Rect with `r`.
    //
    // After calling `clip_to()`, this Rect will contain only the points which were contained both
    // by `this` and `r` before the call.  If `this` and `r` do not contain any points in common,
    // it is possible that this Rect will no longer satisfy the rectangle invariant given in the
    // class documentation.
    //
    // @param [in] r        The Rect to clip this rectangle to.
    void clip_to(const Rect& r);

    // Enlarge this Rect to contain `r`.
    //
    // After calling `enlarge_to()`, this Rect will contain all points which were contained by
    // either `this` or `r` before the call.  It may also contain points which were contained by
    // neither, since the union of two rectangles is not necessarily a rectangle.
    //
    // @param [in] r        The Rect to enlarge this one around.
    void enlarge_to(const Rect& r);
};

pn::string stringify(Rect r);

struct coordPointType {
    uint32_t h;
    uint32_t v;
};

inline bool operator==(coordPointType x, coordPointType y) { return (x.h == y.h) && (x.v == y.v); }
inline bool operator!=(coordPointType x, coordPointType y) { return (x.h != y.h) || (x.v != y.v); }

}  // namespace antares

#endif  // ANTARES_MATH_GEOMETRY_HPP_

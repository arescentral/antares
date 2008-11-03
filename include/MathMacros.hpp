/*
Ares, a tactical space combat game.
Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#ifndef ANTARES_MATH_MACROS_HPP_
#define ANTARES_MATH_MACROS_HPP_

/* Math Macros.h */

#define ABS( x) ((( x) >= 0) ? (x):(-(x)))

#define mClipCode( x, y, bounds) ( 0 | ((( (x) < (bounds).left) << 3) | \
    (( (x) > ( (bounds).right - 1)) << 2) | \
    (( (y) < (bounds).top) << 1) |  \
    ( (y) > ( (bounds).bottom - 1))))

#endif // ANTARES_MATH_MACROS_HPP_

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

#ifndef ANTARES_MACRO_COLORS_HPP_
#define ANTARES_MACRO_COLORS_HPP_

inline void BLACK_COLOR(RGBColor* color)  { (color)->red = (color)->blue = (color)->green = 0; }
inline void WHITE_COLOR(RGBColor* color)  { (color)->red = (color)->blue = (color)->green = 65535; }
inline void GRAY_13(RGBColor* color)      { (color)->red = (color)->blue = (color)->green = 85120; }
inline void GRAY_33(RGBColor* color)      { (color)->red = (color)->blue = (color)->green = 21627; }
inline void GRAY_53(RGBColor* color)      { (color)->red = (color)->blue = (color)->green = 34724; }
inline void GRAY_73(RGBColor* color)      { (color)->red = (color)->blue = (color)->green = 47841; }
inline void GRAY_86(RGBColor* color)      { (color)->red = (color)->blue = (color)->green = 56360; }
inline void HILITE_COLOR(RGBColor* color) { (color)->red = 65535; (color)->blue = 0; (color)->green = 52429; }

#endif // ANTARES_MACRO_COLORS_HPP_

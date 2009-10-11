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

#ifndef ANTARES_RETRO_TEXT_HPP_
#define ANTARES_RETRO_TEXT_HPP_

#include <string>
#include "Geometry.hpp"
#include "SmartPtr.hpp"

namespace antares {

class Picture;
class PixMap;

class RetroText {
  public:
    RetroText(const char* data, size_t len, int font, int fore_color, int back_color);
    ~RetroText();

    int height_for_width(int width);
    void draw(PixMap* pix, const Rect& bounds);

  private:
    const std::string _text;
    const int _font;
    const int _fore_color;
    const int _back_color;

    DISALLOW_COPY_AND_ASSIGN(RetroText);
};

}  // namespace antares

#endif  // ANTARES_RETRO_TEXT_HPP_

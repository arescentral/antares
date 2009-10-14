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

#include <vector>
#include "Geometry.hpp"
#include "SmartPtr.hpp"

namespace antares {

class Picture;
class PixMap;

class RetroText {
  public:
    RetroText(const char* data, size_t len, int font, uint8_t fore_color, uint8_t back_color);
    ~RetroText();

    void wrap_to(int width, int line_spacing);

    int width() const;
    int height() const;

    void draw(PixMap* pix, const Rect& bounds) const;

  private:
    enum SpecialChar {
        NONE,
        TAB,
        WORD_BREAK,
        LINE_BREAK,
    };

    struct RetroChar {
        RetroChar(char character, SpecialChar special, uint8_t fore_color, uint8_t back_color);

        char character;
        SpecialChar special;
        uint8_t fore_color;
        uint8_t back_color;
        int h;
        int v;
    };

    int move_word_down(int index, int v);

    std::vector<RetroChar> _chars;
    int _width;
    int _height;
    int _line_spacing;
    const int _font;

    DISALLOW_COPY_AND_ASSIGN(RetroText);
};

}  // namespace antares

#endif  // ANTARES_RETRO_TEXT_HPP_

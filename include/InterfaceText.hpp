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

#ifndef ANTARES_INTERFACE_TEXT_HPP_
#define ANTARES_INTERFACE_TEXT_HPP_

#include <vector>
#include "sfz/Macros.hpp"
#include "sfz/String.hpp"
#include "ColorTable.hpp"
#include "Geometry.hpp"
#include "PlayerInterfaceDrawing.hpp"
#include "PlayerInterfaceItems.hpp"

namespace antares {

class Picture;
class PixMap;

class InterfaceText {
  public:
    InterfaceText(const sfz::StringPiece& text, interfaceStyleType style, const RgbColor& color);
    ~InterfaceText();

    void wrap_to(int width, int h_buffer, int v_buffer);

    int width() const;
    int height() const;
    const std::vector<inlinePictType>& inline_picts() const;

    void draw(PixMap* pix, const Rect& bounds) const;
    void draw_char(PixMap* pix, const Rect& bounds, int index) const;

  private:
    enum SpecialChar {
        NONE,
        WORD_BREAK,
        LINE_BREAK,
        PICTURE,
    };

    struct InterfaceChar {
        InterfaceChar(uint32_t character, SpecialChar special);

        uint32_t character;
        SpecialChar special;
        int h;
        int v;
    };

    int move_word_down(int index, int v);

    const RgbColor _color;

    std::vector<InterfaceChar> _chars;
    std::vector<inlinePictType> _inline_picts;
    int _width;
    int _height;
    int _h_buffer;
    int _v_buffer;
    const int _font;

    DISALLOW_COPY_AND_ASSIGN(InterfaceText);
};

}  // namespace antares

#endif  // ANTARES_INTERFACE_TEXT_HPP_

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

#ifndef ANTARES_DRAWING_TEXT_HPP_
#define ANTARES_DRAWING_TEXT_HPP_

#include <sfz/sfz.hpp>

#include "drawing/sprite-handling.hpp"
#include "lang/casts.hpp"

namespace antares {

class Font {
  public:
    Font(int32_t id);
    ~Font();

    uint8_t char_width(sfz::Rune mchar) const;
    int32_t string_width(sfz::StringSlice s) const;

    void draw(Point origin, sfz::Rune r, RgbColor color, PixMap* pix, const Rect& clip) const;

    void draw_sprite(Point origin, sfz::StringSlice string, RgbColor color) const;

    int16_t resID;
    int32_t logicalWidth;
    int32_t physicalWidth;
    int32_t height;
    int32_t ascent;

  private:
    void draw_internal(
            Point origin, uint8_t ch, RgbColor color, PixMap* pix, const Rect& clip) const;

    sfz::Bytes charSet;
    std::vector<std::unique_ptr<ArrayPixMap>> _pix_maps;
    std::vector<std::unique_ptr<Sprite>> _sprites;

    DISALLOW_COPY_AND_ASSIGN(Font);
};

extern const Font* tactical_font;
extern const Font* computer_font;
extern const Font* button_font;
extern const Font* message_font;
extern const Font* title_font;
extern const Font* small_button_font;

void InitDirectText();
void DirectTextCleanup();

}  // namespace antares

#endif // ANTARES_DRAWING_TEXT_HPP_

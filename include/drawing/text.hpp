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

#ifndef ANTARES_DRAWING_TEXT_HPP_
#define ANTARES_DRAWING_TEXT_HPP_

#include <sfz/sfz.hpp>

#include "drawing/sprite-handling.hpp"
#include "lang/casts.hpp"

namespace antares {

enum {
    kTacticalFontNum        = 0,
    kComputerFontNum        = 1,
    kButtonFontNum          = 2,
    kMessageFontNum         = 3,
    kTitleFontNum           = 4,
    kButtonSmallFontNum     = 5,
};

class directTextType {
  public:
    directTextType(int32_t id);
    ~directTextType();

    uint8_t char_width(sfz::Rune mchar) const;

    void draw(
            Point origin, sfz::StringSlice string, RgbColor color, PixMap* pix,
            const Rect& clip) const;

    int16_t resID;
    int32_t logicalWidth;
    int32_t physicalWidth;
    int32_t height;
    int32_t ascent;

  private:
    sfz::Bytes charSet;

    DISALLOW_COPY_AND_ASSIGN(directTextType);
};

extern directTextType* gDirectText;

void InitDirectText();
void DirectTextCleanup();

void mDirectCharWidth(unsigned char& mwidth, uint32_t mchar);
void mSetDirectFont(long mwhichFont);
int mDirectFontHeight();
int mDirectFontAscent();
void mGetDirectStringDimensions(const sfz::StringSlice& string, long& width, long& height);
void DrawDirectTextStringClipped(
        Point origin, sfz::StringSlice string, RgbColor color, PixMap* pix, const Rect& clip);

}  // namespace antares

#endif // ANTARES_DRAWING_TEXT_HPP_

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

#include "drawing/text.hpp"

#include <algorithm>
#include <sfz/sfz.hpp>

#include "data/resource.hpp"
#include "drawing/color.hpp"
#include "game/globals.hpp"
#include "video/driver.hpp"

using sfz::Bytes;
using sfz::BytesSlice;
using sfz::Rune;
using sfz::String;
using sfz::StringSlice;
using sfz::format;
using sfz::hex;
using sfz::read;
using std::unique_ptr;

namespace macroman = sfz::macroman;

namespace antares {

namespace {

static const int kDirectFontNum = 6;

enum {
    kTacticalFontResID      = 5000,
    kComputerFontResID      = 5001,
    kButtonFontResID        = 5002,
    kMessageFontResID       = 5003,
    kTitleFontResID         = 5004,
    kButtonSmallFontResID   = 5005,
};

uint8_t to_mac_roman(Rune code) {
    String string(1, code);
    Bytes bytes(macroman::encode(string));
    return bytes.at(0);
}

Rune from_mac_roman(uint8_t byte) {
    BytesSlice bytes(&byte, 1);
    String string(macroman::decode(bytes));
    return string.at(0);
}

}  // namespace

const Font* gDirectTextData[kDirectFontNum];
const Font* tactical_font;
const Font* computer_font;
const Font* button_font;
const Font* message_font;
const Font* title_font;
const Font* small_button_font;

Font::Font(int32_t id) {
    Resource defn_rsrc("font-descriptions", "nlFD", id);
    BytesSlice in(defn_rsrc.data());

    in.shift(4);
    read(in, resID);
    in.shift(2);
    read(in, logicalWidth);
    read(in, physicalWidth);
    read(in, height);
    read(in, ascent);

    Resource data_rsrc("font-bitmaps", "nlFM", resID);
    charSet.assign(data_rsrc.data());

    if (VideoDriver::driver()) {
        _sprites.reset(new unique_ptr<Sprite>[256]);
        for (int i = 0; i < 256; i++) {
            ArrayPixMap pix(physicalWidth * 8, height + 1);
            pix.fill(RgbColor::kClear);
            String s(1, from_mac_roman(i));
            draw(Point(0, ascent), s, RgbColor::kWhite, &pix, pix.size().as_rect());
            _sprites[i] = VideoDriver::driver()->new_sprite(
                        format("/font/{0}/{1}", id, hex(i, 2)), pix);
        }
    }
}

Font::~Font() { }

void Font::draw(
        Point origin, sfz::StringSlice string, RgbColor color, PixMap* pix,
        const Rect& clip) const {
    // move the pen to the resulting location
    origin.v -= ascent;

    // Top and bottom boundaries of where we draw.
    int topEdge = std::max(0, clip.top - origin.v);
    int bottomEdge = height - std::max(0, origin.v + height - clip.bottom + 1);

    int rowBytes = pix->row_bytes();

    // set hchar = place holder for start of each char we draw
    RgbColor* hchar = pix->mutable_bytes() + (origin.v + topEdge) * rowBytes + origin.h;

    for (size_t i = 0; i < string.size(); ++i) {
        const uint8_t* sbyte = charSet.data()
            + height * physicalWidth * to_mac_roman(string.at(i))
            + to_mac_roman(string.at(i));

        int width = *sbyte;
        ++sbyte;

        if ((origin.h + width >= clip.left) || (origin.h < clip.right)) {
            // Left and right boundaries of where we draw.
            int leftEdge = std::max(0, clip.left - origin.h);
            int rightEdge = width - std::max(0, origin.h + width - clip.right);

            // skip over the clipped top rows
            sbyte += topEdge * physicalWidth;

            // dbyte = destination pixel
            RgbColor* dbyte = hchar;

            // repeat for every unclipped row
            for (int y = topEdge; y < bottomEdge; ++y) {
                // repeat for every byte of data
                for (int x = leftEdge; x < rightEdge; ++x) {
                    int byte = x / 8;
                    int bit = 0x80 >> (x & 0x7);
                    if (sbyte[byte] & bit) {
                        dbyte[x] = color;
                    }
                }
                sbyte += physicalWidth;
                dbyte += rowBytes;
            }
        }
        // else (not on screen) just increase the current character

        // for every char clipped or no:
        // increase our character pixel starting point by width of this character
        hchar += width;

        // increase our hposition (our position in pixels)
        origin.h += width;
    }
}

void Font::draw_sprite(Point origin, sfz::StringSlice string, RgbColor color) const {
    origin.offset(0, -ascent);
    for (size_t i = 0; i < string.size(); ++i) {
        uint8_t byte = to_mac_roman(string.at(i));
        _sprites[byte]->draw_shaded(origin.h, origin.v, color);
        origin.offset(char_width(string.at(i)), 0);
    }
}

void InitDirectText() {
    gDirectTextData[0] = tactical_font = new Font(kTacticalFontResID);
    gDirectTextData[1] = computer_font = new Font(kComputerFontResID);
    gDirectTextData[2] = button_font = new Font(kButtonFontResID);
    gDirectTextData[3] = message_font = new Font(kMessageFontResID);
    gDirectTextData[4] = title_font = new Font(kTitleFontResID);
    gDirectTextData[5] = small_button_font = new Font(kButtonSmallFontResID);
}

void DirectTextCleanup() {
    delete tactical_font;
    delete computer_font;
    delete button_font;
    delete message_font;
    delete title_font;
    delete small_button_font;
}

uint8_t Font::char_width(Rune mchar) const {
    const uint8_t* widptr =
        charSet.data() + height * physicalWidth * to_mac_roman(mchar) + to_mac_roman(mchar);
    return *widptr;
}

int32_t Font::string_width(sfz::StringSlice s) const {
    int32_t sum = 0;
    for (int i = 0; i < s.size(); ++i) {
        sum += char_width(s.at(i));
    }
    return sum;
}

}  // namespace antares

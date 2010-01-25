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

#include "BuildPix.hpp"

#include <string>
#include <vector>
#include "rezin/MacRoman.hpp"
#include "sfz/String.hpp"
#include "sfz/SmartPtr.hpp"
#include "ColorTranslation.hpp"
#include "DirectText.hpp"
#include "Error.hpp"
#include "Picture.hpp"
#include "Resource.hpp"
#include "RetroText.hpp"

using rezin::mac_roman_encoding;
using sfz::String;
using sfz::scoped_ptr;
using std::string;
using std::vector;

namespace antares {

namespace {

int string_to_int(const string& str) {
    if (str.size() > 0) {
        char* end;
        int value = strtol(str.c_str(), &end, 10);
        if (end == str.c_str() + str.size()) {
            return value;
        }
    }
    fail("Couldn't parse '%s' as an integer", str.c_str());
}

class PixBuilder {
  public:
    PixBuilder(ArrayPixMap* pix)
            : _pix(pix) { }

    void set_background(int id) {
        _background.reset(new Picture(id));
        _background_start = _pix->bounds().bottom;
    }

    void add_picture(int id) {
        Picture pict(id);
        extend(pict.bounds().bottom);
        Rect dest = pict.bounds();
        Rect surround(
                0, _pix->bounds().bottom - pict.bounds().height(),
                _pix->bounds().right, _pix->bounds().bottom);
        dest.center_in(surround);
        CopyBits(&pict, _pix, pict.bounds(), dest);
    }

    void add_text(const String& text) {
        RgbColor red;
        GetRGBTranslateColorShade(&red, RED, VERY_LIGHT);
        RetroText retro(text, kTitleFontNum, red, RgbColor::kBlack);
        retro.wrap_to(_pix->bounds().right - 12, 2);

        Rect dest(0, 0, _pix->bounds().right, retro.height());
        dest.offset(0, _pix->bounds().bottom);
        dest.inset(6, 0);

        extend(retro.height());
        retro.draw(_pix, dest);
    }

  private:
    void extend(int height) {
        const int old_height = _pix->bounds().bottom;
        const int new_height = old_height + height;
        _pix->resize(Rect(0, 0, _pix->bounds().right, new_height));

        if (_background.get()) {
            PixMap::View view(_pix, Rect(0, old_height, _pix->bounds().right, new_height));
            Rect dest = _background->bounds();
            dest.offset(0, -old_height);
            while (dest.top < height) {
                if (dest.bottom >= 0) {
                    CopyBits(_background.get(), &view, _background->bounds(), dest);
                }
                dest.offset(0, dest.height());
            }
        }
    }

    ArrayPixMap* _pix;

    scoped_ptr<Picture> _background;
    int _background_start;
};

}  // namespace

PixMap* build_pix(int text_id, int width) {
    scoped_ptr<ArrayPixMap> pix(new ArrayPixMap(width, 0));
    PixBuilder build(pix.get());
    Resource text('TEXT', text_id);

    vector<string> lines;
    const uint8_t* start = text.data().data();
    const uint8_t* const end = start + text.data().size();
    bool in_section_header = (start + 2 <= end) && (memcmp(start, "#+", 2) == 0);
    for (const uint8_t* p = start; p != end; ++p) {
        if (p + 3 <= end && memcmp(p, "\r#+", 3) == 0) {
            lines.push_back(string(reinterpret_cast<const char*>(start), p - start));
            start = p + 1;
            in_section_header = true;
        } else if (in_section_header && (*p == '\r')) {
            lines.push_back(string(reinterpret_cast<const char*>(start), p - start));
            start = p + 1;
            in_section_header = false;
        }
    }
    if (start != end) {
        lines.push_back(string(reinterpret_cast<const char*>(start), end - start));
    }

    for (vector<string>::iterator it = lines.begin(); it != lines.end(); ++it) {
        if (it->substr(0, 2) == "#+") {
            if (it->size() > 2) {
                if ((*it)[2] == 'B') {
                    int id = string_to_int(it->substr(3));
                    build.set_background(id);
                } else {
                    int id = string_to_int(it->substr(2));
                    build.add_picture(id);
                }
            }
        } else {
            String string(it->c_str(), mac_roman_encoding());
            build.add_text(string);
        }
    }

    return pix.release();
}

}  // namespace antares

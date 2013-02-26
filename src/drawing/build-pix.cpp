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

#include "drawing/build-pix.hpp"

#include <vector>
#include <sfz/sfz.hpp>

#include "data/picture.hpp"
#include "data/resource.hpp"
#include "drawing/color.hpp"
#include "drawing/styled-text.hpp"
#include "drawing/text.hpp"

using sfz::BytesSlice;
using sfz::Exception;
using sfz::String;
using sfz::StringSlice;
using sfz::format;
using sfz::string_to_int;
using std::unique_ptr;
using std::vector;

namespace macroman = sfz::macroman;
namespace utf8 = sfz::utf8;

namespace antares {

namespace {

class PixBuilder {
  public:
    PixBuilder(ArrayPixMap* pix)
            : _pix(pix) { }

    void set_background(int id) {
        _background.reset(new Picture(id));
        _background_start = _pix->size().height;
    }

    void add_picture(int id) {
        Picture pict(id);
        extend(pict.size().height);
        Rect dest = pict.size().as_rect();
        Rect surround(
                0, _pix->size().height - pict.size().height,
                _pix->size().width, _pix->size().height);
        dest.center_in(surround);
        _pix->view(dest).copy(pict);
    }

    void add_text(const StringSlice& text) {
        RgbColor red;
        red = GetRGBTranslateColorShade(RED, VERY_LIGHT);
        StyledText retro(title_font);
        retro.set_fore_color(red);
        retro.set_retro_text(text);
        retro.wrap_to(_pix->size().width - 11, 0, 2);

        Rect dest(0, 0, _pix->size().width, retro.height());
        dest.offset(0, _pix->size().height);
        dest.inset(6, 0);

        extend(retro.height());
        retro.draw(_pix, dest);
    }

  private:
    void extend(int height) {
        const int old_height = _pix->size().height;
        const int new_height = old_height + height;
        _pix->resize(Size(_pix->size().width, new_height));

        if (_background.get()) {
            Rect clip(0, old_height, _pix->size().width, new_height);
            Rect dest = _background->size().as_rect();
            while (dest.top < clip.bottom) {
                if (dest.bottom > clip.top) {
                    Rect clipped_dest = dest;
                    clipped_dest.clip_to(clip);
                    Rect source(
                            clipped_dest.left, clipped_dest.top % dest.height(),
                            clipped_dest.right, ((clipped_dest.bottom - 1) % dest.height()) + 1);
                    _pix->view(clipped_dest).copy(_background->view(source));
                }
                dest.offset(0, dest.height());
            }
        }
    }

    ArrayPixMap* _pix;

    unique_ptr<Picture> _background;
    int _background_start;
};

}  // namespace

unique_ptr<PixMap> build_pix(int text_id, int width) {
    unique_ptr<ArrayPixMap> pix(new ArrayPixMap(width, 0));
    PixBuilder build(pix.get());
    Resource rsrc("text", "txt", text_id);

    vector<String> lines;
    BytesSlice data = rsrc.data();
    String text(utf8::decode(data));
    bool in_section_header = (text.size() >= 2) && (text.slice(0, 2) == "#+");
    size_t start = 0;
    const size_t end = text.size();
    for (size_t i = start; i != end; ++i) {
        if (((end - i) >= 3) && (text.slice(i, 3) == "\n#+")) {
            lines.emplace_back(text.slice(start, i - start));
            start = i + 1;
            in_section_header = true;
        } else if (in_section_header && (text.at(i) == '\n')) {
            lines.emplace_back(text.slice(start, i - start));
            start = i + 1;
            in_section_header = false;
        }
    }
    if (start != end) {
        lines.emplace_back(text.slice(start));
    }

    for (const auto& line: lines) {
        if (line.size() >= 2 && line.slice(0, 2) == "#+") {
            if (line.size() > 2) {
                if (line.at(2) == 'B') {
                    int32_t id = 2005;
                    if (line.size() > 3) {
                        if (!string_to_int(line.slice(3), id)) {
                            throw Exception(format("malformed header line {0}", quote(line)));
                        }
                    }
                    build.set_background(id);
                } else {
                    int32_t id;
                    if (!string_to_int(line.slice(2), id)) {
                        throw Exception(format("malformed header line {0}", quote(line)));
                    }
                    build.add_picture(id);
                }
            }
        } else {
            build.add_text(line);
        }
    }

    return unique_ptr<PixMap>(pix.release());
}

}  // namespace antares

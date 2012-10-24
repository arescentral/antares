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
#include "drawing/retro-text.hpp"
#include "drawing/text.hpp"

using sfz::BytesSlice;
using sfz::Exception;
using sfz::String;
using sfz::StringSlice;
using sfz::format;
using sfz::linked_ptr;
using sfz::make_linked_ptr;
using sfz::scoped_ptr;
using sfz::string_to_int;
using std::vector;

namespace macroman = sfz::macroman;

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
        retro::StyledText retro(title_font);
        retro.set_fore_color(red);
        retro.set_text(text);
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

    scoped_ptr<Picture> _background;
    int _background_start;
};

}  // namespace

PixMap* build_pix(int text_id, int width) {
    scoped_ptr<ArrayPixMap> pix(new ArrayPixMap(width, 0));
    PixBuilder build(pix.get());
    Resource text("text", "txt", text_id);

    vector<linked_ptr<String> > lines;
    BytesSlice data = text.data();
    const uint8_t* start = data.data();
    const uint8_t* const end = start + data.size();
    bool in_section_header = (start + 2 <= end) && (memcmp(start, "#+", 2) == 0);
    for (const uint8_t* p = start; p != end; ++p) {
        if (p + 3 <= end && memcmp(p, "\r#+", 3) == 0) {
            linked_ptr<String> line(
                    new String(macroman::decode(data.slice(start - data.data(), p - start))));
            lines.push_back(line);
            start = p + 1;
            in_section_header = true;
        } else if (in_section_header && (*p == '\r')) {
            linked_ptr<String> line(
                    new String(macroman::decode(data.slice(start - data.data(), p - start))));
            lines.push_back(line);
            start = p + 1;
            in_section_header = false;
        }
    }
    if (start != end) {
        linked_ptr<String> line(
                new String(macroman::decode(data.slice(start - data.data(), end - start))));
        lines.push_back(line);
    }

    for (vector<linked_ptr<String> >::const_iterator it = lines.begin(); it != lines.end(); ++it) {
        if ((*it)->size() >= 2 && (*it)->at(0) == '#' && (*it)->at(1) == '+') {
            if ((*it)->size() > 2) {
                if ((*it)->at(2) == 'B') {
                    int32_t id = 2005;
                    if ((*it)->size() > 3) {
                        if (!string_to_int((*it)->slice(3), id)) {
                            throw Exception(format("malformed header line {0}", quote(**it)));
                        }
                    }
                    build.set_background(id);
                } else {
                    int32_t id;
                    if (!string_to_int((*it)->slice(2), id)) {
                        throw Exception(format("malformed header line {0}", quote(**it)));
                    }
                    build.add_picture(id);
                }
            }
        } else {
            build.add_text(**it);
        }
    }

    return pix.release();
}

}  // namespace antares

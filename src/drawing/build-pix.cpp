// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// Copyright (C) 2008-2017 The Antares Authors
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

#include <sfz/sfz.hpp>
#include <vector>

#include "data/picture.hpp"
#include "data/resource.hpp"
#include "drawing/color.hpp"
#include "drawing/text.hpp"
#include "game/sys.hpp"

using sfz::BytesSlice;
using sfz::Exception;
using sfz::String;
using sfz::StringSlice;
using sfz::format;
using sfz::string_to_int;
using std::unique_ptr;
using std::vector;

namespace utf8 = sfz::utf8;

namespace antares {

namespace {

class PixDraw {
  public:
    PixDraw(Point origin, int32_t width) : _bounds(origin, {width, 0}) {}

    void set_background(const Texture& texture) {
        _background       = &texture;
        _background_start = _bounds.height();
    }

    void add_picture(const Texture& texture) {
        Rect surround(
                _bounds.left, _bounds.bottom, _bounds.right,
                _bounds.bottom + texture.size().height);
        extend(texture.size().height);
        Rect dest = texture.size().as_rect();
        dest.center_in(surround);
        texture.draw(dest);
    }

    void add_text(const StyledText& text) {
        Rect dest(_bounds.origin(), {_bounds.width(), text.height()});
        dest.offset(0, _bounds.height());
        dest.inset(6, 0);
        extend(text.height());
        text.draw(dest);
    }

  private:
    void extend(int height) {
        const int top = _bounds.bottom;
        _bounds.bottom += height;
        const int bottom = _bounds.bottom;

        if (_background) {
            Rect clip(_bounds.left, top, _bounds.right, bottom);
            Rect dest(_bounds.origin(), _background->size());
            while (dest.top < clip.bottom) {
                if (dest.bottom > clip.top) {
                    Rect clipped_dest = dest;
                    clipped_dest.clip_to(clip);
                    Point origin(
                            clipped_dest.left - _bounds.left,
                            (clipped_dest.top - _bounds.top) % dest.height());
                    _background->draw_cropped(clipped_dest, Rect(origin, clipped_dest.size()));
                }
                dest.offset(0, dest.height());
            }
        }
    }

    Rect _bounds;

    const Texture* _background       = nullptr;
    int            _background_start = 0;
};

}  // namespace

BuildPix::BuildPix(int text_id, int width) : _size({width, 0}) {
    Resource rsrc("text", "txt", text_id);

    BytesSlice     data = rsrc.data();
    String         text(utf8::decode(data));
    bool           in_section_header = (text.size() >= 2) && (text.slice(0, 2) == "#+");
    size_t         start             = 0;
    const size_t   end               = text.size();
    vector<String> raw_lines;
    for (size_t i = start; i != end; ++i) {
        if (((end - i) >= 3) && (text.slice(i, 3) == "\n#+")) {
            raw_lines.emplace_back(text.slice(start, i - start));
            start             = i + 1;
            in_section_header = true;
        } else if (in_section_header && (text.at(i) == '\n')) {
            raw_lines.emplace_back(text.slice(start, i - start));
            start             = i + 1;
            in_section_header = false;
        }
    }
    if (start != end) {
        raw_lines.emplace_back(text.slice(start));
    }

    for (const auto& line : raw_lines) {
        if (line.size() >= 2 && line.slice(0, 2) == "#+") {
            if (line.size() > 2) {
                if (line.at(2) == 'B') {
                    int32_t id = 2005;
                    if (line.size() > 3) {
                        if (!string_to_int(line.slice(3), id)) {
                            throw Exception(format("malformed header line {0}", quote(line)));
                        }
                    }
                    Picture pict(id);
                    _lines.push_back(Line{
                            Line::BACKGROUND, Picture(id).texture(), nullptr,
                    });
                } else {
                    int32_t id;
                    if (!string_to_int(line.slice(2), id)) {
                        throw Exception(format("malformed header line {0}", quote(line)));
                    }
                    Picture pict(id);
                    _lines.push_back(Line{
                            Line::PICTURE, Picture(id).texture(), nullptr,
                    });
                }
            }
        } else {
            unique_ptr<StyledText> styled(new StyledText(sys.fonts.title));
            auto                   red = GetRGBTranslateColorShade(RED, VERY_LIGHT);
            styled->set_fore_color(red);
            styled->set_retro_text(line);
            styled->wrap_to(_size.width - 11, 0, 2);
            _lines.push_back(Line{Line::TEXT, nullptr, std::move(styled)});
        }
    }

    for (const auto& line : _lines) {
        switch (line.type) {
            case Line::PICTURE: _size.height += line.texture.size().height; break;
            case Line::TEXT: _size.height += line.text->height(); break;
            case Line::BACKGROUND: break;
        }
    }
}

void BuildPix::draw(Point origin) const {
    PixDraw draw(origin, _size.width);
    for (const auto& line : _lines) {
        switch (line.type) {
            case Line::PICTURE: draw.add_picture(line.texture); break;
            case Line::TEXT: draw.add_text(*line.text); break;
            case Line::BACKGROUND: draw.set_background(line.texture); break;
        }
    }
}

}  // namespace antares

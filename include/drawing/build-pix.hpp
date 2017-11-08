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

#ifndef ANTARES_DRAWING_BUILD_PIX_HPP_
#define ANTARES_DRAWING_BUILD_PIX_HPP_

#include <sfz/sfz.hpp>
#include <vector>
#include "drawing/styled-text.hpp"
#include "math/geometry.hpp"
#include "video/driver.hpp"

namespace antares {

class BuildPix {
  public:
    BuildPix(int text_id, int width);

    Size size() const { return _size; }
    void draw(Point origin) const;

  private:
    struct Line {
        enum Type {
            PICTURE,
            BACKGROUND,
            TEXT,
        } type;
        Texture                     texture;
        std::unique_ptr<StyledText> text;
    };
    std::vector<Line> _lines;
    Size              _size;
};

}  // namespace antares

#endif  // ANTARES_DRAWING_BUILD_PIX_HPP_

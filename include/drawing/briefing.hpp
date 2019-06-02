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

#ifndef ANTARES_DRAWING_BRIEFING_HPP_
#define ANTARES_DRAWING_BRIEFING_HPP_

#include <sfz/sfz.hpp>

#include "data/handle.hpp"
#include "drawing/pix-table.hpp"
#include "math/geometry.hpp"
#include "math/scale.hpp"

namespace antares {

union Level;
class PixMap;

struct BriefingSprite {
    const NatePixTable::Frame& frame;
    Rect                       sprite_rect;
    bool                       outline;
    RgbColor                   outline_color, fill_color;
};

std::vector<sfz::optional<BriefingSprite>> render_briefing(
        int32_t maxSize, const Rect& bounds, const Point& corner, Scale scale);

struct BriefPointInfo {
    pn::string header;
    pn::string content;
    Rect       highlight;
};
BriefPointInfo BriefPoint_Data_Get(
        int32_t whichPoint, const Level& level, const Point& corner, Scale scale, int32_t maxSize,
        const Rect& bounds, const std::vector<sfz::optional<BriefingSprite>>& sprites);

}  // namespace antares

#endif  // ANTARES_DRAWING_BRIEFING_HPP_

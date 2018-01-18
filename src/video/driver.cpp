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

#include "video/driver.hpp"

#include "game/sys.hpp"
#include "lang/defines.hpp"

namespace antares {

VideoDriver::VideoDriver() {
    if (sys.video) {
        throw std::runtime_error("VideoDriver is a singleton");
    }
    sys.video = this;
}

VideoDriver::~VideoDriver() { sys.video = NULL; }

Texture::Impl::~Impl() {}

Points::Points() { sys.video->begin_points(); }

Points::~Points() { sys.video->end_points(); }

void Points::draw(const Point& at, const RgbColor& color) const {
    sys.video->batch_point(at, color);
}

Lines::Lines() { sys.video->begin_lines(); }

Lines::~Lines() { sys.video->end_lines(); }

void Lines::draw(const Point& from, const Point& to, const RgbColor& color) const {
    sys.video->batch_line(from, to, color);
}

Rects::Rects() { sys.video->begin_rects(); }

Rects::~Rects() { sys.video->end_rects(); }

void Rects::fill(const Rect& rect, const RgbColor& color) const {
    sys.video->batch_rect(rect, color);
}

Quads::Quads(const Texture& sprite) : _sprite(sprite) { _sprite._impl->begin_quads(); }

Quads::~Quads() { _sprite._impl->end_quads(); }

void Quads::draw(const Rect& dest, const Rect& source, const RgbColor& tint) const {
    _sprite._impl->draw_quad(dest, source, tint);
}

}  // namespace antares

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

#include "video/driver.hpp"

#include <sfz/sfz.hpp>

#include "lang/defines.hpp"

using sfz::Exception;

namespace antares {

static ANTARES_GLOBAL VideoDriver* video_driver = NULL;

VideoDriver::VideoDriver() {
    if (video_driver) {
        throw Exception("VideoDriver is a singleton");
    }
    antares::video_driver = this;
}

VideoDriver::~VideoDriver() {
    antares::video_driver = NULL;
}

VideoDriver* VideoDriver::driver() {
    return antares::video_driver;
}

Texture::Impl::~Impl() { }

Points::Points() {
    VideoDriver::driver()->begin_points();
}

Points::~Points() {
    VideoDriver::driver()->end_points();
}

void Points::draw(const Point& at, const RgbColor& color) const {
    VideoDriver::driver()->batch_point(at, color);
}

Lines::Lines() {
    VideoDriver::driver()->begin_lines();
}

Lines::~Lines() {
    VideoDriver::driver()->end_lines();
}

void Lines::draw(const Point& from, const Point& to, const RgbColor& color) const {
    VideoDriver::driver()->batch_line(from, to, color);
}

Rects::Rects() {
    VideoDriver::driver()->begin_rects();
}

Rects::~Rects() {
    VideoDriver::driver()->end_rects();
}

void Rects::fill(const Rect& rect, const RgbColor& color) const {
    VideoDriver::driver()->batch_rect(rect, color);
}

Quads::Quads(const Texture& sprite):
        _sprite(sprite) {
    _sprite._impl->begin_quads();
}

Quads::~Quads() {
    _sprite._impl->end_quads();
}

void Quads::draw(const Rect& draw_rect, Point origin, const RgbColor& tint) const {
    _sprite._impl->draw_quad(draw_rect, origin, tint);
}

}  // namespace antares

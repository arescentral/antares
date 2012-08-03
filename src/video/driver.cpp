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

using sfz::Exception;

namespace antares {

namespace {

VideoDriver* video_driver = NULL;

}  // namespace

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

Stencil::Stencil(VideoDriver* driver):
        _driver(driver) {
    _driver->start_stencil();
}

void Stencil::set_threshold(uint8_t alpha) {
    _driver->set_stencil_threshold(alpha);
}

void Stencil::apply() {
    _driver->apply_stencil();
}

Stencil::~Stencil() {
    _driver->end_stencil();
}

Sprite::~Sprite() { }

}  // namespace antares

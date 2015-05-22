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

#include "mac/windowed.hpp"

#include <ApplicationServices/ApplicationServices.h>
#include <OpenGL/OpenGL.h>
#include <OpenGL/gl.h>
#include <sfz/sfz.hpp>

#include "mac/c/CocoaVideoDriver.h"
#include "mac/core-opengl.hpp"

using sfz::Exception;

namespace antares {

CocoaWindowed::CocoaWindowed(
        const cgl::PixelFormat& pixel_format, const cgl::Context& context, Size screen_size,
        bool fullscreen, bool retina):
        _window(antares_window_create(
                    pixel_format.c_obj(), context.c_obj(),
                    screen_size.width, screen_size.height, fullscreen, retina)) { }

CocoaWindowed::~CocoaWindowed() {
    antares_window_destroy(_window);
}

Size CocoaWindowed::viewport_size() const {
    return {antares_window_viewport_width(_window), antares_window_viewport_height(_window)};
}

}  // namespace antares

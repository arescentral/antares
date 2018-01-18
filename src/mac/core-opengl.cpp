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

#include "mac/core-opengl.hpp"

#include <OpenGL/OpenGL.h>
#include <pn/file>

namespace antares {
namespace cgl {

void check(CGLError error) {
    if (error != kCGLNoError) {
        throw std::runtime_error(pn::format("CGL Error: {0}", CGLErrorString(error)).c_str());
    }
}

PixelFormat::PixelFormat(const CGLPixelFormatAttribute* attrs) {
    check(CGLChoosePixelFormat(attrs, &_pixel_format, &_npix));
    if (_pixel_format == NULL) {
        throw std::runtime_error("Couldn't create CGLPixelFormatObj.");
    }
}

PixelFormat::~PixelFormat() { CGLReleasePixelFormat(_pixel_format); }

CGLPixelFormatObj PixelFormat::c_obj() const { return _pixel_format; }

GLint PixelFormat::npix() const { return _npix; }

Context::Context(CGLPixelFormatObj pix, CGLContextObj share) {
    check(CGLCreateContext(pix, NULL, &_context));
}

Context::~Context() { CGLReleaseContext(_context); }

CGLContextObj Context::c_obj() const { return _context; }

}  // namespace cgl
}  // namespace antares

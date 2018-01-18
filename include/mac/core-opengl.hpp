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

#ifndef ANTARES_MAC_CORE_OPENGL_HPP_
#define ANTARES_MAC_CORE_OPENGL_HPP_

#include <OpenGL/OpenGL.h>
#include <OpenGL/gl.h>

namespace antares {
namespace cgl {

#ifdef check
#undef check
#endif
void check(CGLError error);

class PixelFormat {
  public:
    PixelFormat(const CGLPixelFormatAttribute* attrs);
    PixelFormat(const PixelFormat&) = delete;
    PixelFormat& operator=(const PixelFormat&) = delete;
    ~PixelFormat();

    CGLPixelFormatObj c_obj() const;
    GLint             npix() const;

  private:
    CGLPixelFormatObj _pixel_format;
    GLint             _npix;
};

class Context {
  public:
    Context(CGLPixelFormatObj pix, CGLContextObj share);
    Context(const Context&) = delete;
    Context& operator=(const Context&) = delete;

    ~Context();
    CGLContextObj c_obj() const;

  private:
    CGLContextObj _context;
};

}  // namespace cgl
}  // namespace antares

#endif  // ANTARES_MAC_CORE_OPENGL_HPP_

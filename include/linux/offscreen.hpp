// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// Copyright (C) 2015-2017 The Antares Authors
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

#ifndef ANTARES_LINUX_OFFSCREEN_HPP_
#define ANTARES_LINUX_OFFSCREEN_HPP_

#define GLX_GLXEXT_PROTOTYPES

#include <GL/glx.h>
#include <GL/glxext.h>
#include <X11/Xlib.h>
#include <memory>

#include "math/geometry.hpp"

namespace antares {

class Offscreen {
  public:
    Offscreen(Size size);
    ~Offscreen();

  private:
    struct ContextDestroyer {
        Display* display;
        void     operator()(GLXContext context) { glXDestroyContext(display, context); }
    };

    std::unique_ptr<Display, decltype(&XCloseDisplay)>                       _display;
    std::unique_ptr<GLXFBConfig[], decltype(&XFree)>                         _fb_configs;
    std::unique_ptr<std::remove_pointer<GLXContext>::type, ContextDestroyer> _context;
};

}  // namespace antares

#endif  // ANTARES_LINUX_OFFSCREEN_HPP_

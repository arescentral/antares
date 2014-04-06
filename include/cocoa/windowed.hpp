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

#ifndef ANTARES_COCOA_WINDOWED_HPP_
#define ANTARES_COCOA_WINDOWED_HPP_

#include <sfz/sfz.hpp>

#include "cocoa/c/CocoaVideoDriver.h"
#include "math/geometry.hpp"

namespace antares {
namespace cgl {
class Context;
class PixelFormat;
}  // namespace cgl

class CocoaWindowed {
  public:
    CocoaWindowed(
        const cgl::PixelFormat& pixel_format, const cgl::Context& context, Size screen_size,
        bool fullscreen, bool retina);
    ~CocoaWindowed();

    AntaresWindow* window() const { return _window; }
    Size viewport_size() const;

  private:
    AntaresWindow* _window;
    DISALLOW_COPY_AND_ASSIGN(CocoaWindowed);
};

}  // namespace antares

#endif  // ANTARES_COCOA_WINDOWED_HPP_

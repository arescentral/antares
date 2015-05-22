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

#ifndef ANTARES_MAC_FULLSCREEN_HPP_
#define ANTARES_MAC_FULLSCREEN_HPP_

#include <sfz/sfz.hpp>

#include <ApplicationServices/ApplicationServices.h>

#include "mac/windowed.hpp"
#include "math/geometry.hpp"

namespace antares {
namespace cgl {
class Context;
}  // namespace cgl

class CocoaFullscreen {
  public:
    CocoaFullscreen(
            const cgl::PixelFormat& pixel_format, const cgl::Context& context, Size screen_size);
    ~CocoaFullscreen();

    AntaresWindow* window() { return _windowed.window(); }
    Size viewport_size() const { return _screen_size; }

  private:
    const Size _screen_size;
    struct DisplayFader {
        DisplayFader(CocoaWindowed& windowed);
        void finish();
        CGDisplayFadeReservationToken token;
    };
    DisplayFader _fader;
    CocoaWindowed _windowed;
    struct MenuBarHider {
        MenuBarHider();
        ~MenuBarHider();
    };
    MenuBarHider _menu_bar_hider;
    struct MouseHider {
        MouseHider();
        ~MouseHider();
    };
    MouseHider _mouse_hider;

    DISALLOW_COPY_AND_ASSIGN(CocoaFullscreen);
};

}  // namespace antares

#endif  // ANTARES_MAC_FULLSCREEN_HPP_

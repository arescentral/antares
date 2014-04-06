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

#include "cocoa/fullscreen.hpp"

#include <ApplicationServices/ApplicationServices.h>
#include <OpenGL/OpenGL.h>
#include <OpenGL/gl.h>
#include <sfz/sfz.hpp>

#include "cocoa/c/CocoaVideoDriver.h"
#include "cocoa/core-opengl.hpp"

using sfz::Exception;

namespace antares {

CocoaFullscreen::DisplayFader::DisplayFader(CocoaWindowed& windowed) {
    if (CGAcquireDisplayFadeReservation(kCGMaxDisplayReservationInterval, &token)
            == kCGErrorSuccess) {
        CGDisplayFade(
                token, 1.0, kCGDisplayBlendNormal, kCGDisplayBlendSolidColor,
                0, 0, 0, true);
    }
}

void CocoaFullscreen::DisplayFader::finish() {
    if (CGAcquireDisplayFadeReservation(1.0, &token) == kCGErrorSuccess) {
        CGDisplayFade(
                token, 1.0, kCGDisplayBlendSolidColor, kCGDisplayBlendNormal,
                0, 0, 0, false);
    }
}

CocoaFullscreen::MenuBarHider::MenuBarHider() {
    antares_menu_bar_hide();
}

CocoaFullscreen::MenuBarHider::~MenuBarHider() {
    antares_menu_bar_show();
}

CocoaFullscreen::MouseHider::MouseHider() {
    antares_mouse_hide();
}

CocoaFullscreen::MouseHider::~MouseHider() {
    antares_mouse_show();
}

CocoaFullscreen::CocoaFullscreen(
        const cgl::PixelFormat& pixel_format, const cgl::Context& context, Size screen_size):
        _screen_size(screen_size),
        _windowed(pixel_format, context, screen_size, true, true),
        _fader(_windowed) {
    CGReleaseDisplayFadeReservation(_fader.token);
}

CocoaFullscreen::~CocoaFullscreen() {
    _fader.finish();
}

}  // namespace antares

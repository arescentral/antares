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

CocoaFullscreen::DisplayCapturer::DisplayCapturer(Size screen_size) {
    CGDisplayFadeReservationToken token;
    if (CGAcquireDisplayFadeReservation(1.0, &token) == kCGErrorSuccess) {
        CGDisplayFade(
                token, 1.0, kCGDisplayBlendNormal, kCGDisplayBlendSolidColor,
                0, 0, 0, true);
    }

    CGDisplayErr err = CGCaptureAllDisplays();
    if (err != CGDisplayNoErr) {
        throw Exception("CGCaptureAllDisplays() failed");
        return;
    }

    // TODO(sfiera): control the resolution of the OpenGL context by
    // setting the resolution of the backing store, rather than the
    // screen.  Setting the backing store's resolution independently
    // appears to have only been supported since 10.6, and since we
    // currently target 10.4, we need to control the screen resolution
    // directly for the time being.
    CGDisplaySwitchToMode(kCGDirectMainDisplay, CGDisplayBestModeForParameters(
                kCGDirectMainDisplay, 32, screen_size.width, screen_size.height,
                NULL));
}

CocoaFullscreen::DisplayCapturer::~DisplayCapturer() {
    CGDisplayFadeReservationToken token;
    if (CGAcquireDisplayFadeReservation(1.0, &token) == kCGErrorSuccess) {
        CGDisplayFade(
                token, 1.0, kCGDisplayBlendSolidColor, kCGDisplayBlendNormal,
                0, 0, 0, false);
    }
    CGReleaseAllDisplays();
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

CocoaFullscreen::SetFullscreen::SetFullscreen(
        const cgl::Context& context, uint32_t display_mask) {
    cgl::check(CGLSetFullScreenOnDisplay(context.c_obj(), display_mask));
    cgl::check(CGLSetCurrentContext(context.c_obj()));
}

CocoaFullscreen::CocoaFullscreen(const cgl::Context& context, Size screen_size, uint32_t display_mask):
        _capturer(screen_size),
        _set_fullscreen(context, display_mask) {
}

CocoaFullscreen::~CocoaFullscreen() { }

}  // namespace antares

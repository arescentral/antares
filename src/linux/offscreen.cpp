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

#include "linux/offscreen.hpp"

#include <sfz/sfz.hpp>

using sfz::Exception;
using sfz::format;

namespace antares {

static int kAttrs[] = {None};

template <typename T>
static T* check_nonnull(T* value, const char* name) {
    if (value) {
        return value;
    }
    throw Exception(format("{0} was null", name));
}

GLXFBConfig* fb_configs(Display* display) {
    int          count;
    GLXFBConfig* configs = check_nonnull(
            glXChooseFBConfig(display, DefaultScreen(display), kAttrs, &count),
            "glXChooseFBConfig()");
    return configs;
}

const int kContextAttrs[] = {GLX_CONTEXT_MAJOR_VERSION_ARB,
                             3,
                             GLX_CONTEXT_MINOR_VERSION_ARB,
                             2,
                             GLX_CONTEXT_PROFILE_MASK_ARB,
                             GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
                             None};

GLXContext new_context(Display* display, GLXFBConfig config) {
    typedef GLXContext (*glXCreateContextAttribsARBProc)(
            Display*, GLXFBConfig, GLXContext, Bool, const int*);
    glXCreateContextAttribsARBProc glXCreateContextAttribsARB =
            (glXCreateContextAttribsARBProc)glXGetProcAddressARB(
                    (const GLubyte*)"glXCreateContextAttribsARB");
    return check_nonnull(
            glXCreateContextAttribsARB(display, config, nullptr, true, kContextAttrs),
            "glXCreateContextAttribsARB()");
}

GLXPbuffer new_pbuffer(Display* display, GLXFBConfig config, Size size) {
    int attrs[] = {GLX_PBUFFER_WIDTH, size.width, GLX_PBUFFER_HEIGHT, size.height, None};
    return glXCreatePbuffer(display, config, attrs);
}

Offscreen::Offscreen(Size size)
        : _display(check_nonnull(XOpenDisplay(nullptr), "XOpenDisplay()"), XCloseDisplay),
          _fb_configs(fb_configs(_display.get()), XFree),
          _context(new_context(_display.get(), _fb_configs[0]), {_display.get()}) {
    typedef Bool (*glXMakeContextCurrentARBProc)(Display*, GLXDrawable, GLXDrawable, GLXContext);
    glXMakeContextCurrentARBProc glXMakeContextCurrentARB =
            (glXMakeContextCurrentARBProc)glXGetProcAddressARB(
                    (const GLubyte*)"glXMakeContextCurrent");
    GLXPbuffer pbuffer = new_pbuffer(_display.get(), _fb_configs[0], size);
    glXMakeContextCurrentARB(_display.get(), pbuffer, pbuffer, _context.get());
}

Offscreen::~Offscreen() {}

}  // namespace antares

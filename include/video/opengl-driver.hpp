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

#ifndef ANTARES_VIDEO_OPEN_GL_DRIVER_HPP_
#define ANTARES_VIDEO_OPEN_GL_DRIVER_HPP_

#include <stdint.h>
#include <sfz/sfz.hpp>

#include "drawing/color.hpp"
#include "math/geometry.hpp"
#include "ui/card.hpp"
#include "video/driver.hpp"

namespace antares {

class Event;

class OpenGlVideoDriver : public VideoDriver {
  public:
    OpenGlVideoDriver(Size screen_size);

    virtual Sprite* new_sprite(sfz::PrintItem name, const PixMap& content);
    virtual void fill_rect(const Rect& rect, const RgbColor& color);
    virtual void draw_point(const Point& at, const RgbColor& color);
    virtual void draw_line(const Point& from, const Point& to, const RgbColor& color);

    virtual void start_stencil();
    virtual void set_stencil_threshold(uint8_t alpha);
    virtual void apply_stencil();
    virtual void end_stencil();

  protected:
    class MainLoop {
      public:
        MainLoop(const OpenGlVideoDriver& driver, Card* initial);
        bool done();
        void draw();
        Card* top() const;

      private:
        struct Setup {
            Setup();
        };
        const Setup _setup;
        const OpenGlVideoDriver& _driver;
        CardStack _stack;

        DISALLOW_COPY_AND_ASSIGN(MainLoop);
    };

    Size screen_size() const { return _screen_size; }

  private:
    // Clamps all bytes in the stencil buffer to [0, _stencil_height].  This is done whenever a
    // transition is made from drawing the stencil buffer to drawing pixels, so that if another
    // stenciling operation is pushed, incrementing bytes in the stencil buffer is guaranteed to
    // yield new values.
    void normalize_stencil();

    const Size _screen_size;

    int8_t _stencil_height;

    DISALLOW_COPY_AND_ASSIGN(OpenGlVideoDriver);
};

}  // namespace antares

#endif  // ANTARES_VIDEO_OPEN_GL_DRIVER_HPP_

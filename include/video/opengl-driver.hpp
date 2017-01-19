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

#ifndef ANTARES_VIDEO_OPEN_GL_DRIVER_HPP_
#define ANTARES_VIDEO_OPEN_GL_DRIVER_HPP_

#include <stdint.h>
#include <sfz/sfz.hpp>

#include "drawing/color.hpp"
#include "math/geometry.hpp"
#include "math/random.hpp"
#include "ui/card.hpp"
#include "video/driver.hpp"

namespace antares {

class Event;

typedef int sampler2D;
typedef int sampler2DRect;
struct vec2 {
    float x, y;
};
struct vec4 {
    float x, y, z, w;
};

template <typename T>
struct Uniform {
    const char* name;
    int         location;

    void load(int program);
    void set(T value) const;
};

class OpenGlVideoDriver : public VideoDriver {
  public:
    OpenGlVideoDriver();

    virtual int scale() const;

    virtual Texture texture(sfz::PrintItem name, const PixMap& content);
    virtual void dither_rect(const Rect& rect, const RgbColor& color);
    virtual void draw_point(const Point& at, const RgbColor& color);
    virtual void draw_line(const Point& from, const Point& to, const RgbColor& color);
    virtual void draw_triangle(const Rect& rect, const RgbColor& color);
    virtual void draw_diamond(const Rect& rect, const RgbColor& color);
    virtual void draw_plus(const Rect& rect, const RgbColor& color);

    struct Uniforms {
        Uniform<vec2>          screen          = {"screen"};
        Uniform<int>           scale           = {"scale"};
        Uniform<int>           color_mode      = {"color_mode"};
        Uniform<sampler2DRect> sprite          = {"sprite"};
        Uniform<sampler2D>     static_image    = {"static_image"};
        Uniform<float>         static_fraction = {"static_fraction"};
        Uniform<vec2>          unit            = {"unit"};
        Uniform<vec4>          outline_color   = {"outline_color"};
        Uniform<int>           seed            = {"seed"};
    };

  protected:
    class MainLoop {
      public:
        MainLoop(OpenGlVideoDriver& driver, Card* initial);
        void  draw();
        bool  done() const;
        Card* top() const;

      private:
        struct Setup {
            Setup(OpenGlVideoDriver& driver);
        };
        const Setup        _setup;
        OpenGlVideoDriver& _driver;
        CardStack          _stack;

        DISALLOW_COPY_AND_ASSIGN(MainLoop);
    };

    virtual Size viewport_size() const = 0;

  private:
    virtual void begin_points();
    virtual void end_points();
    virtual void batch_point(const Point& at, const RgbColor& color);

    virtual void begin_lines();
    virtual void end_lines();
    virtual void batch_line(const Point& from, const Point& to, const RgbColor& color);

    virtual void begin_rects();
    virtual void end_rects();
    virtual void batch_rect(const Rect& rect, const RgbColor& color);

    Random _static_seed;

    Uniforms _uniforms;

    std::map<size_t, Texture> _triangles;
    std::map<size_t, Texture> _diamonds;
    std::map<size_t, Texture> _pluses;

    uint32_t _vbuf[3];

    DISALLOW_COPY_AND_ASSIGN(OpenGlVideoDriver);
};

}  // namespace antares

#endif  // ANTARES_VIDEO_OPEN_GL_DRIVER_HPP_

// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// Copyright (C) 2008-2011 Ares Central
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
// License along with this program.  If not, see
// <http://www.gnu.org/licenses/>.

#include "video/opengl-driver.hpp"

#include <stdint.h>
#include <algorithm>
#include <OpenGL/OpenGL.h>
#include <OpenGL/gl.h>
#include <sfz/sfz.hpp>

#include "drawing/color.hpp"
#include "drawing/pix-map.hpp"
#include "math/geometry.hpp"
#include "ui/card.hpp"

using sfz::Exception;
using sfz::PrintItem;
using sfz::String;
using sfz::StringSlice;
using std::min;
using std::max;

namespace antares {

namespace {

class OpenGlSprite : public Sprite {
  public:
    OpenGlSprite(PrintItem name, const PixMap& image)
            : _name(name),
              _size(image.size()) {
        glBindTexture(GL_TEXTURE_RECTANGLE_EXT, _texture.id);
        glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
#if defined(__LITTLE_ENDIAN__)
        GLenum type = GL_UNSIGNED_INT_8_8_8_8;
#elif defined(__BIG_ENDIAN__)
        GLenum type = GL_UNSIGNED_INT_8_8_8_8_REV;
#else
#error "Couldn't determine endianness of platform"
#endif
        glTexImage2D(
                GL_TEXTURE_RECTANGLE_EXT, 0, GL_RGBA, _size.width, _size.height,
                0, GL_BGRA, type, image.bytes());
    }

    virtual StringSlice name() const {
        return _name;
    }

    virtual void draw(int32_t x, int32_t y) const {
        const int32_t w = _size.width;
        const int32_t h = _size.height;
        draw(Rect(x, y, x + w, y + h));
    }

    virtual void draw(const Rect& draw_rect) const {
        const int32_t w = _size.width;
        const int32_t h = _size.height;
        glBindTexture(GL_TEXTURE_RECTANGLE_EXT, _texture.id);
        glColor4f(1, 1, 1, 1);
        glBegin(GL_QUADS);
        glTexCoord2f(0, 0);
        glVertex2f(draw_rect.left, draw_rect.top);
        glTexCoord2f(0, h);
        glVertex2f(draw_rect.left, draw_rect.bottom);
        glTexCoord2f(w, h);
        glVertex2f(draw_rect.right, draw_rect.bottom);
        glTexCoord2f(w, 0);
        glVertex2f(draw_rect.right, draw_rect.top);
        glEnd();
    }

    virtual const Size& size() const {
        return _size;
    }

  private:
    struct Texture {
        Texture() { glGenTextures(1, &id); }
        ~Texture() { glDeleteTextures(1, &id); }

        GLuint id;
        DISALLOW_COPY_AND_ASSIGN(Texture);
    };

    const String _name;
    Texture _texture;
    Size _size;

    DISALLOW_COPY_AND_ASSIGN(OpenGlSprite);
};

}  // namespace

OpenGlVideoDriver::OpenGlVideoDriver(Size screen_size)
        : _screen_size(screen_size),
          _transition_fraction(0.0),
          _transition_color(RgbColor::kBlack),
          _stencil_height(0) { }

int OpenGlVideoDriver::get_demo_scenario() {
    return -1;
}

Sprite* OpenGlVideoDriver::new_sprite(PrintItem name, const PixMap& content) {
    return new OpenGlSprite(name, content);
}

void OpenGlVideoDriver::fill_rect(const Rect& rect, const RgbColor& color) {
    glBindTexture(GL_TEXTURE_RECTANGLE_EXT, 0);
    glColor4ub(color.red, color.green, color.blue, color.alpha);
    glBegin(GL_QUADS);
    glVertex2f(rect.right, rect.top);
    glVertex2f(rect.left, rect.top);
    glVertex2f(rect.left, rect.bottom);
    glVertex2f(rect.right, rect.bottom);
    glEnd();
}

void OpenGlVideoDriver::draw_point(const Point& at, const RgbColor& color) {
    glBindTexture(GL_TEXTURE_RECTANGLE_EXT, 0);
    glColor4ub(color.red, color.green, color.blue, color.alpha);
    glBegin(GL_POINTS);
    glVertex2f(at.h + 0.5, at.v + 0.5);
    glEnd();
}

void OpenGlVideoDriver::draw_line(const Point& from, const Point& to, const RgbColor& color) {
    glBindTexture(GL_TEXTURE_RECTANGLE_EXT, 0);

    // Shortcut: when `from` == `to`, we can draw just a point.
    if (from == to) {
        draw_point(from, color);
        return;
    }

    // Shortcut: when one of the dimensions of `from` and `to` matches, we can draw a rect.  This
    // gives more predictable results than using GL_LINES: the OpenGL standard is not strict about
    // how to render lines, and may give bad results at the endpoints.
    if ((from.h == to.h) || (from.v == to.v)) {
        Rect rect(
                min(from.h, to.h), min(from.v, to.v),
                max(from.h, to.h) + 1, max(from.v, to.v) + 1);
        fill_rect(rect, color);
        return;
    }

    //
    // Adjust `from` and `to` points that we draw all of the pixels that we're supposed to.
    //
    // Q: Why isn't it just as simple as an OpenGL line from (from.h, from.v) to (to.h, to.v)?
    //
    // A: Well, according to the interface of this function, we are drawing a line which includes
    //    those two pixels and each pixel in-between.  In OpenGL, though, coordinates specify the
    //    corners of pixels, not the pixels themselves, so the right-most and bottom-most pixels
    //    would not be drawn (both ends in a bend sinister; one end in other cases).
    //
    //    In order to force the pixels at both ends to be drawn, we have to adjust the points so
    //    that they specify the far corners of the specified pixels.  That means, for each
    //    dimension, moving the end-point with the greater value one pixel further away.  If
    //    they're equal, we leave them unchanged.
    //

    float x1 = from.h;
    float x2 = to.h;
    if (x1 > x2) {
        x1 += 1.0f;
    } else if (x1 < x2) {
        x2 += 1.0f;
    } else {
        x1 += 0.5f;
        x2 += 0.5f;
    }

    float y1 = from.v;
    float y2 = to.v;
    if (y1 > y2) {
        y1 += 1.0f;
    } else if (y1 < y2) {
        y2 += 1.0f;
    } else {
        y1 += 0.5f;
        y2 += 0.5f;
    }

    glColor4ub(color.red, color.green, color.blue, color.alpha);
    glBegin(GL_LINES);
    glVertex2f(x1, y1);
    glVertex2f(x2, y2);
    glEnd();
}

void OpenGlVideoDriver::set_transition_fraction(double fraction) {
    _transition_fraction = fraction;
}

void OpenGlVideoDriver::set_transition_to(const RgbColor& color) {
    _transition_color = color;
}

void OpenGlVideoDriver::start_stencil() {
    glColorMask(0, 0, 0, 0);
    glAlphaFunc(GL_GREATER, 0);
    glStencilFunc(GL_GEQUAL, _stencil_height, 0xff);
    glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);
    ++_stencil_height;
}

void OpenGlVideoDriver::set_stencil_threshold(uint8_t alpha) {
    glAlphaFunc(GL_GREATER, alpha / 256.0);
}

void OpenGlVideoDriver::apply_stencil() {
    normalize_stencil();
}

void OpenGlVideoDriver::end_stencil() {
    --_stencil_height;
    normalize_stencil();
}

void OpenGlVideoDriver::normalize_stencil() {
    glColorMask(0, 0, 0, 0);
    glAlphaFunc(GL_ALWAYS, 0);
    glStencilFunc(GL_LEQUAL, _stencil_height, 0xff);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

    glBegin(GL_QUADS);
    glVertex2f(_screen_size.width, 0);
    glVertex2f(0, 0);
    glVertex2f(0, _screen_size.height);
    glVertex2f(_screen_size.width, _screen_size.height);
    glEnd();

    glColorMask(1, 1, 1, 1);
    glStencilFunc(GL_EQUAL, _stencil_height, 0xff);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
}

OpenGlVideoDriver::MainLoop::Setup::Setup() {
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glClearColor(0, 0, 0, 1);
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_TEXTURE_RECTANGLE_EXT);
    glBindTexture(GL_TEXTURE_RECTANGLE_EXT, 1);
    glEnable(GL_STENCIL_TEST);
    glEnable(GL_ALPHA_TEST);
    glClearStencil(0);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glTextureRangeAPPLE(GL_TEXTURE_RECTANGLE_EXT, 0, NULL);
    glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
}

OpenGlVideoDriver::MainLoop::MainLoop(const OpenGlVideoDriver& driver, Card* initial):
        _driver(driver),
        _stack(initial) { }

bool OpenGlVideoDriver::MainLoop::done() {
    return _stack.empty();
}

void OpenGlVideoDriver::MainLoop::draw() {
    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glLoadIdentity();
    glViewport(0, 0, _driver._screen_size.width, _driver._screen_size.height);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();

    glTranslatef(-1.0, 1.0, 0.0);
    glScalef(2.0, -2.0, 1.0);
    glScalef(1.0 / _driver._screen_size.width, 1.0 / _driver._screen_size.height, 1.0);

    _stack.top()->draw();

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    glBindTexture(GL_TEXTURE_RECTANGLE_EXT, 0);
    glColor4ub(
            _driver._transition_color.red, _driver._transition_color.green,
            _driver._transition_color.blue, 0xff * _driver._transition_fraction);
    glBegin(GL_QUADS);
    glVertex2f(1, 1);
    glVertex2f(-1, 1);
    glVertex2f(-1, -1);
    glVertex2f(1, -1);
    glEnd();

    glFinish();
}

Card* OpenGlVideoDriver::MainLoop::top() const {
    return _stack.top();
}

}  // namespace antares

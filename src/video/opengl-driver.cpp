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

#include "video/opengl-driver.hpp"

#include <stdint.h>
#include <algorithm>
#include <OpenGL/OpenGL.h>
#include <OpenGL/gl.h>
#include <sfz/sfz.hpp>

#include "drawing/color.hpp"
#include "drawing/pix-map.hpp"
#include "drawing/shapes.hpp"
#include "game/globals.hpp"
#include "math/geometry.hpp"
#include "math/random.hpp"
#include "ui/card.hpp"

#include "game/time.hpp"

using sfz::Exception;
using sfz::PrintItem;
using sfz::String;
using sfz::StringSlice;
using sfz::format;
using sfz::print;
using std::min;
using std::max;
using std::unique_ptr;

namespace io = sfz::io;

namespace antares {

namespace {

static const char kShaderColorModeUniform[] = "color_mode";
static const char kShaderSpriteUniform[] = "sprite";
static const char kShaderStaticImageUniform[] = "static_image";
static const char kShaderStaticFractionUniform[] = "static_fraction";
static const char kShaderUnitUniform[] = "unit";
static const char kShaderOutlineColorUniform[] = "outline_color";
static const char kShaderSeedUniform[] = "seed";
static const GLchar* kShaderSource =
    "#version 120\n"
    "uniform int color_mode;\n"
    "uniform sampler2DRect sprite;\n"
    "uniform sampler2D static_image;\n"
    "uniform float static_fraction;\n"
    "uniform vec2 unit;\n"
    "uniform vec4 outline_color;\n"
    "uniform int seed;\n"
    "\n"
    "void main() {\n"
    "    vec2 uv = gl_TexCoord[0].xy;\n"
    "    vec4 sprite_color = texture2DRect(sprite, uv);\n"
    "    if (color_mode == 0) {\n"
    "        gl_FragColor = gl_Color;\n"
    "    } else if (color_mode == 1) {\n"
    "        if (mod(floor(gl_TexCoord[1].s) + floor(gl_TexCoord[1].t), 2) == 1) {\n"
    "            gl_FragColor = gl_Color;\n"
    "        } else {\n"
    "            gl_FragColor = vec4(0, 0, 0, 0);\n"
    "        }\n"
    "    } else if (color_mode == 2) {\n"
    "        gl_FragColor = sprite_color;\n"
    "    } else if (color_mode == 3) {\n"
    "        gl_FragColor = gl_Color * sprite_color;\n"
    "    } else if (color_mode == 4) {\n"
    "        vec2 uv2 = (gl_TexCoord[1].xy + vec2(seed / 256, seed)) * vec2(1.0/256, 1.0/256);\n"
    "        vec4 static_color = texture2D(static_image, uv2);\n"
    "        if (static_color.w <= static_fraction) {\n"
    "            vec4 sprite_alpha = vec4(1, 1, 1, sprite_color.w);\n"
    "            gl_FragColor = gl_Color * sprite_alpha;\n"
    "        } else {\n"
    "            gl_FragColor = sprite_color;\n"
    "        }\n"
    "    } else if (color_mode == 5) {\n"
    "        float neighborhood =\n"
    "                texture2DRect(sprite, uv + vec2(-unit.s, -unit.t)).w +\n"
    "                texture2DRect(sprite, uv + vec2(-unit.s,       0)).w +\n"
    "                texture2DRect(sprite, uv + vec2(-unit.s,  unit.t)).w +\n"
    "                texture2DRect(sprite, uv + vec2(      0, -unit.t)).w +\n"
    "                texture2DRect(sprite, uv + vec2(      0,  unit.t)).w +\n"
    "                texture2DRect(sprite, uv + vec2( unit.s, -unit.t)).w +\n"
    "                texture2DRect(sprite, uv + vec2( unit.s,       0)).w +\n"
    "                texture2DRect(sprite, uv + vec2( unit.s,  unit.t)).w;\n"
    "        if (sprite_color.w > (neighborhood / 8)) {\n"
    "            gl_FragColor = outline_color;\n"
    "        } else if (sprite_color.w > 0) {\n"
    "            gl_FragColor = gl_Color;\n"
    "        } else {\n"
    "            gl_FragColor = vec4(0, 0, 0, 0);\n"
    "        }\n"
    "    }\n"
    "}\n";

void gl_check() {
    int error = glGetError();
    if (error != GL_NO_ERROR) {
        throw Exception(error);
    }
}

void gl_log(GLint object) {
    GLint log_size;
    if (glIsShader(object)) {
        glGetShaderiv(object, GL_INFO_LOG_LENGTH, &log_size);
    } else {
        glGetProgramiv(object, GL_INFO_LOG_LENGTH, &log_size);
    }
    if (log_size == 0) {
        return;
    }
    unique_ptr<GLchar[]> log(new GLchar[log_size + 1]);
    if (glIsShader(object)) {
        glGetShaderInfoLog(object, log_size, &log_size, log.get());
    } else {
        glGetProgramInfoLog(object, log_size, &log_size, log.get());
    }
    print(io::err, format("object {0} log: {1}\n", object, (const char*)log.get()));
}

class OpenGlSprite : public Sprite {
  public:
    OpenGlSprite(PrintItem name, const PixMap& image, const OpenGlVideoDriver::Uniforms& uniforms)
            : _name(name),
              _size(image.size()),
              _uniforms(uniforms) {
        glBindTexture(GL_TEXTURE_RECTANGLE_EXT, _texture.id);
        glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
#if defined(__LITTLE_ENDIAN__)
        GLenum type = GL_UNSIGNED_INT_8_8_8_8;
#elif defined(__BIG_ENDIAN__)
        GLenum type = GL_UNSIGNED_INT_8_8_8_8_REV;
#else
#error "Couldn't determine endianness of platform"
#endif

        // Add a 1-pixel clear border.  Color mode 5 (outline) won't work unless we do this.
        Size size = image.size();
        size.width += 2;
        size.height += 2;
        ArrayPixMap copy(size);
        copy.fill(RgbColor::kClear);
        copy.view(Rect(1, 1, size.width - 1, size.height - 1)).copy(image);
        glTexImage2D(
                GL_TEXTURE_RECTANGLE_EXT, 0, GL_RGBA, size.width, size.height,
                0, GL_BGRA, type, copy.bytes());
    }

    virtual StringSlice name() const {
        return _name;
    }

    virtual void draw(const Rect& draw_rect) const {
        glUniform1i(_uniforms.color_mode, 2);
        draw_internal(draw_rect);
    }

    virtual void draw_cropped(const Rect& draw_rect, Point origin) const {
        Rect texture_rect(origin, draw_rect.size());
        texture_rect.offset(1, 1);

        glUniform1i(_uniforms.color_mode, 2);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_RECTANGLE_EXT, _texture.id);
        gl_check();
        glBegin(GL_QUADS);
        glMultiTexCoord2f(GL_TEXTURE0, texture_rect.left, texture_rect.top);
        glVertex2f(draw_rect.left, draw_rect.top);
        glMultiTexCoord2f(GL_TEXTURE0, texture_rect.left, texture_rect.bottom);
        glVertex2f(draw_rect.left, draw_rect.bottom);
        glMultiTexCoord2f(GL_TEXTURE0, texture_rect.right, texture_rect.bottom);
        glVertex2f(draw_rect.right, draw_rect.bottom);
        glMultiTexCoord2f(GL_TEXTURE0, texture_rect.right, texture_rect.top);
        glVertex2f(draw_rect.right, draw_rect.top);
        glEnd();
        gl_check();
    }

    virtual void draw_shaded(const Rect& draw_rect, const RgbColor& tint) const {
        glColor4ub(tint.red, tint.green, tint.blue, 255);
        glUniform1i(_uniforms.color_mode, 3);
        draw_internal(draw_rect);
    }

    virtual void draw_static(const Rect& draw_rect, const RgbColor& color, uint8_t frac) const {
        glColor4ub(color.red, color.green, color.blue, color.alpha);
        glUniform1i(_uniforms.color_mode, 4);
        glUniform1f(_uniforms.static_fraction, frac / 255.0);
        draw_internal(draw_rect);
    }

    virtual void draw_outlined(
            const Rect& draw_rect, const RgbColor& outline_color,
            const RgbColor& fill_color) const {
        glUniform1i(_uniforms.color_mode, 5);
        glUniform2f(
                _uniforms.unit,
                double(_size.width) / draw_rect.width(),
                double(_size.height) / draw_rect.height());
        glColor4ub(fill_color.red, fill_color.green, fill_color.blue, fill_color.alpha);
        glUniform4f(
                _uniforms.outline_color, outline_color.red / 255.0, outline_color.green / 255.0,
                outline_color.blue / 255.0, outline_color.alpha / 255.0);
        draw_internal(draw_rect);
    }

    virtual const Size& size() const {
        return _size;
    }

  private:
    virtual void draw_internal(const Rect& draw_rect) const {
        const int32_t w = _size.width;
        const int32_t h = _size.height;
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_RECTANGLE_EXT, _texture.id);
        gl_check();
        glBegin(GL_QUADS);
        glMultiTexCoord2f(GL_TEXTURE0, 1, 1);
        glMultiTexCoord2f(GL_TEXTURE1, draw_rect.left, draw_rect.top);
        glVertex2f(draw_rect.left, draw_rect.top);
        glMultiTexCoord2f(GL_TEXTURE0, 1, h + 1);
        glMultiTexCoord2f(GL_TEXTURE1, draw_rect.left, draw_rect.bottom);
        glVertex2f(draw_rect.left, draw_rect.bottom);
        glMultiTexCoord2f(GL_TEXTURE0, w + 1, h + 1);
        glMultiTexCoord2f(GL_TEXTURE1, draw_rect.right, draw_rect.bottom);
        glVertex2f(draw_rect.right, draw_rect.bottom);
        glMultiTexCoord2f(GL_TEXTURE0, w + 1, 1);
        glMultiTexCoord2f(GL_TEXTURE1, draw_rect.right, draw_rect.top);
        glVertex2f(draw_rect.right, draw_rect.top);
        glEnd();
        gl_check();
    }

    struct Texture {
        Texture() { glGenTextures(1, &id); }
        ~Texture() { glDeleteTextures(1, &id); }

        GLuint id;
        DISALLOW_COPY_AND_ASSIGN(Texture);
    };

    const String _name;
    Texture _texture;
    Size _size;
    const OpenGlVideoDriver::Uniforms& _uniforms;

    DISALLOW_COPY_AND_ASSIGN(OpenGlSprite);
};

}  // namespace

OpenGlVideoDriver::OpenGlVideoDriver(Size screen_size)
        : _screen_size(screen_size),
          _static_seed(0) { }

unique_ptr<Sprite> OpenGlVideoDriver::new_sprite(PrintItem name, const PixMap& content) {
    return unique_ptr<Sprite>(new OpenGlSprite(name, content, _uniforms));
}

void OpenGlVideoDriver::fill_rect(const Rect& rect, const RgbColor& color) {
    glUniform1i(_uniforms.color_mode, 0);
    glColor4ub(color.red, color.green, color.blue, color.alpha);
    glBegin(GL_QUADS);
    glVertex2f(rect.right, rect.top);
    glVertex2f(rect.left, rect.top);
    glVertex2f(rect.left, rect.bottom);
    glVertex2f(rect.right, rect.bottom);
    glEnd();
}

void OpenGlVideoDriver::dither_rect(const Rect& rect, const RgbColor& color) {
    glUniform1i(_uniforms.color_mode, 1);
    glColor4ub(color.red, color.green, color.blue, color.alpha);
    glBegin(GL_QUADS);
    glMultiTexCoord2f(GL_TEXTURE1, rect.right, rect.top);
    glVertex2f(rect.right, rect.top);
    glMultiTexCoord2f(GL_TEXTURE1, rect.left, rect.top);
    glVertex2f(rect.left, rect.top);
    glMultiTexCoord2f(GL_TEXTURE1, rect.left, rect.bottom);
    glVertex2f(rect.left, rect.bottom);
    glMultiTexCoord2f(GL_TEXTURE1, rect.right, rect.bottom);
    glVertex2f(rect.right, rect.bottom);
    glEnd();
}

void OpenGlVideoDriver::draw_point(const Point& at, const RgbColor& color) {
    glUniform1i(_uniforms.color_mode, 0);
    glColor4ub(color.red, color.green, color.blue, color.alpha);
    glBegin(GL_POINTS);
    glVertex2f(at.h + 0.5, at.v + 0.5);
    glEnd();
}

void OpenGlVideoDriver::draw_line(const Point& from, const Point& to, const RgbColor& color) {
    glUniform1i(_uniforms.color_mode, 0);

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

void OpenGlVideoDriver::draw_triangle(const Rect& rect, const RgbColor& color) {
    size_t size = min(rect.width(), rect.height());
    Rect to(0, 0, size, size);
    to.offset(rect.left, rect.top);
    if (_triangles.find(size) == _triangles.end()) {
        ArrayPixMap pix(size, size);
        pix.fill(RgbColor::kClear);
        draw_triangle_up(&pix, RgbColor::kWhite);
        _triangles[size] = new_sprite("", pix);
    }
    _triangles[size]->draw_shaded(to, color);
}

void OpenGlVideoDriver::draw_diamond(const Rect& rect, const RgbColor& color) {
    size_t size = min(rect.width(), rect.height());
    Rect to(0, 0, size, size);
    to.offset(rect.left, rect.top);
    if (_diamonds.find(size) == _diamonds.end()) {
        ArrayPixMap pix(size, size);
        pix.fill(RgbColor::kClear);
        draw_compat_diamond(&pix, RgbColor::kWhite);
        _diamonds[size] = new_sprite("", pix);
    }
    _diamonds[size]->draw_shaded(to, color);
}

void OpenGlVideoDriver::draw_plus(const Rect& rect, const RgbColor& color) {
    size_t size = min(rect.width(), rect.height());
    Rect to(0, 0, size, size);
    to.offset(rect.left, rect.top);
    if (_pluses.find(size) == _pluses.end()) {
        ArrayPixMap pix(size, size);
        pix.fill(RgbColor::kClear);
        draw_compat_plus(&pix, RgbColor::kWhite);
        _pluses[size] = new_sprite("", pix);
    }
    _pluses[size]->draw_shaded(to, color);
}

OpenGlVideoDriver::MainLoop::Setup::Setup(OpenGlVideoDriver& driver) {
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glClearColor(0, 0, 0, 1);
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_TEXTURE_RECTANGLE_EXT);
    glBindTexture(GL_TEXTURE_RECTANGLE_EXT, 1);
    glEnable(GL_ALPHA_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glTextureRangeAPPLE(GL_TEXTURE_RECTANGLE_EXT, 0, NULL);
    glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);

    GLuint shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(shader, 1, &kShaderSource, NULL);
    glCompileShader(shader);
    GLint compiled;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (compiled == GL_FALSE) {
        gl_log(shader);
        throw Exception("compilation failed");
    }
    gl_check();

    GLuint program = glCreateProgram();
    glAttachShader(program, shader);
    glLinkProgram(program);
    glValidateProgram(program);
    GLint linked;
    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    if (linked == GL_FALSE) {
        gl_log(program);
        throw Exception("linking failed");
    }
    gl_check();

    driver._uniforms.color_mode = glGetUniformLocation(program, kShaderColorModeUniform);
    driver._uniforms.sprite = glGetUniformLocation(program, kShaderSpriteUniform);
    driver._uniforms.static_image = glGetUniformLocation(program, kShaderStaticImageUniform);
    driver._uniforms.static_fraction = glGetUniformLocation(program, kShaderStaticFractionUniform);
    driver._uniforms.unit = glGetUniformLocation(program, kShaderUnitUniform);
    driver._uniforms.outline_color = glGetUniformLocation(program, kShaderOutlineColorUniform);
    driver._uniforms.seed = glGetUniformLocation(program, kShaderSeedUniform);
    glUseProgram(program);
    gl_check();

    GLuint static_texture;
    glGenTextures(1, &static_texture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, static_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    size_t size = 256;
    unique_ptr<uint8_t[]> static_data(new uint8_t[size * size * 2]);
    int32_t static_index = 0;
    uint8_t* p = static_data.get();
    for (int i = 0; i < (size * size); ++i) {
        *(p++) = 255;
        *(p++) = XRandomSeeded(256, &static_index);
    }
    glTexImage2D(
            GL_TEXTURE_2D, 0, GL_LUMINANCE_ALPHA, size, size, 0, GL_LUMINANCE_ALPHA,
            GL_UNSIGNED_BYTE, static_data.get());
    gl_check();

    glUniform1i(driver._uniforms.sprite, 0);
    glUniform1i(driver._uniforms.static_image, 1);
    gl_check();
}

OpenGlVideoDriver::MainLoop::MainLoop(OpenGlVideoDriver& driver, Card* initial):
        _setup(driver),
        _driver(driver),
        _stack(initial) { }

void OpenGlVideoDriver::MainLoop::draw() {
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();
    glViewport(0, 0, _driver._screen_size.width, _driver._screen_size.height);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();

    glTranslatef(-1.0, 1.0, 0.0);
    glScalef(2.0, -2.0, 1.0);
    glScalef(1.0 / _driver._screen_size.width, 1.0 / _driver._screen_size.height, 1.0);
    int32_t seed = XRandomSeeded(256, &_driver._static_seed);
    seed <<= 8;
    seed += XRandomSeeded(256, &_driver._static_seed);
    glUniform1i(_driver._uniforms.seed, seed);

    gl_check();

    _stack.top()->draw();

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    glFinish();
}

bool OpenGlVideoDriver::MainLoop::done() const {
    return _stack.empty();
}

Card* OpenGlVideoDriver::MainLoop::top() const {
    return _stack.top();
}

}  // namespace antares

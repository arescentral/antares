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
#include "video/glsl/fragment.hpp"
#include "video/glsl/vertex.hpp"

#include "game/time.hpp"

using sfz::Exception;
using sfz::PrintItem;
using sfz::String;
using sfz::StringSlice;
using sfz::format;
using sfz::hex;
using sfz::print;
using std::min;
using std::max;
using std::unique_ptr;

namespace io = sfz::io;

namespace antares {

namespace {

static const char kShaderScreenUniform[]          = "screen";
static const char kShaderColorModeUniform[]       = "color_mode";
static const char kShaderSpriteUniform[]          = "sprite";
static const char kShaderStaticImageUniform[]     = "static_image";
static const char kShaderStaticFractionUniform[]  = "static_fraction";
static const char kShaderUnitUniform[]            = "unit";
static const char kShaderOutlineColorUniform[]    = "outline_color";
static const char kShaderSeedUniform[]            = "seed";

enum {
    FILL_MODE            = 0,
    DITHER_MODE          = 1,
    DRAW_SPRITE_MODE     = 2,
    TINT_SPRITE_MODE     = 3,
    STATIC_SPRITE_MODE   = 4,
    OUTLINE_SPRITE_MODE  = 5,
};

#ifndef NDEBUG

static const char* _gl_error_string(GLenum err) {
    switch (err) {
        case GL_NO_ERROR: return "GL_NO_ERROR";
        case GL_INVALID_ENUM: return "GL_INVALID_ENUM";
        case GL_INVALID_VALUE: return "GL_INVALID_VALUE";
        case GL_INVALID_OPERATION: return "GL_INVALID_OPERATION";
        case GL_STACK_OVERFLOW: return "GL_STACK_OVERFLOW";
        case GL_STACK_UNDERFLOW: return "GL_STACK_UNDERFLOW";
        case GL_OUT_OF_MEMORY: return "GL_OUT_OF_MEMORY";
        case GL_TABLE_TOO_LARGE: return "GL_TABLE_TOO_LARGE";
        default: return "?";
    }
}

static void _gl_check(const char* fn, const char* file, int line) {
    int error = glGetError();
    if (error != GL_NO_ERROR) {
        print(io::err, format("{0}: {1} ({2}:{3})\n", fn, _gl_error_string(error), file, line));
    }
}

template <typename T>
static T _gl_check(T t, const char* fn, const char* file, int line) {
    _gl_check(fn, file, line);
    return t;
}

#define _GL(FN, ...) \
    ( FN ( __VA_ARGS__ ) \
    , _gl_check( #FN , __FILE__ , __LINE__ ) \
    )
#define _GLV(FN, ...) _gl_check( FN ( __VA_ARGS__ ), #FN , __FILE__ , __LINE__ )

#define glActiveTexture(texture)                _GL(glActiveTexture, texture)
#define glAttachShader(program, shader)         _GL(glAttachShader, program, shader)
// Skip glBegin().
#define glBindTexture(target, texture)          _GL(glBindTexture, target, texture)
#define glBlendFunc(sfactor, dfactor)           _GL(glBlendFunc, sfactor, dfactor)
#define glClear(mask)                           _GL(glClear, mask)
#define glClearColor(red, green, blue, alpha)   _GL(glClearColor, red, green, blue, alpha)
// Skip glColor4ub().
#define glCompileShader(shader)                 _GL(glCompileShader, shader)
#define glCreateProgram()                       _GLV(glCreateProgram)
#define glCreateShader(shaderType)              _GLV(glCreateShader, shaderType)
#define glDeleteTextures(n, textures)           _GL(glDeleteTextures, n, textures)
#define glDisable(cap)                          _GL(glDisable, cap)
#define glEnable(cap)                           _GL(glEnable, cap)
#define glEnd()                                 _GL(glEnd)
#define glFinish()                              _GL(glFinish)
#define glGenTextures(n, textures)              _GL(glGenTextures, n, textures)
// Skip glGetError().
#define glGetProgramInfoLog(program, maxLength, length, infoLog) \
    _GL(glGetProgramInfoLog, program, maxLength, length, infoLog)
#define glGetProgramiv(program, pname, params)  _GL(glGetProgramiv, program, pname, params)
#define glGetShaderInfoLog(shader, maxLength, length, infoLog) \
    _GL(glGetShaderInfoLog, shader, maxLength, length, infoLog)
#define glGetShaderiv(shader, pname, params)    _GL(glGetShaderiv, shader, pname, params)
#define glGetUniformLocation(program, name)     _GLV(glGetUniformLocation, program, name)
// Skip glIsShader().
#define glLinkProgram(program)                  _GL(glLinkProgram, program)
#define glLoadIdentity()                        _GL(glLoadIdentity)
// Skip glMultiTexCoord2f().
#define glPixelStorei(pname, param)             _GL(glPixelStorei, pname, param)
#define glShaderSource(shader, count, string, length) \
    _GL(glShaderSource, shader, count, string, length)
#define glTexImage2D(target, level, internalformat, width, height, border, format, type, pixels) \
    _GL(glTexImage2D, target, level, internalformat, width, height, border, format, type, pixels)
#define glTextureRangeAPPLE(target, length, pointer) \
    _GL(glTextureRangeAPPLE, target, length, pointer)
#define glUniform1f(location, v0)               _GL(glUniform1f, location, v0)
#define glUniform1i(location, v0)               _GL(glUniform1i, location, v0)
#define glUniform2f(location, v0, v1)           _GL(glUniform2f, location, v0, v1)
#define glUniform4f(location, v0, v1, v2, v3)   _GL(glUniform4f, location, v0, v1, v2, v3)
#define glUseProgram(program)                   _GL(glUseProgram, program)
#define glValidateProgram(program)              _GL(glValidateProgram, program)
// glVertex2f().
#define glViewport(x, y, width, height)         _GL(glViewport, x, y, width, height)

#endif  // NDEBUG

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

class OpenGlTextureImpl : public Texture::Impl {
  public:
    OpenGlTextureImpl(PrintItem name, const PixMap& image, const OpenGlVideoDriver::Uniforms& uniforms)
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
        glUniform1i(_uniforms.color_mode, DRAW_SPRITE_MODE);
        draw_internal(draw_rect);
    }

    virtual void draw_cropped(const Rect& draw_rect, Point origin, const RgbColor& tint) const {
        begin_quads();
        draw_quad(draw_rect, origin, tint);
        end_quads();
    }

    virtual void draw_shaded(const Rect& draw_rect, const RgbColor& tint) const {
        glColor4ub(tint.red, tint.green, tint.blue, 255);
        glUniform1i(_uniforms.color_mode, TINT_SPRITE_MODE);
        draw_internal(draw_rect);
    }

    virtual void draw_static(const Rect& draw_rect, const RgbColor& color, uint8_t frac) const {
        glColor4ub(color.red, color.green, color.blue, color.alpha);
        glUniform1i(_uniforms.color_mode, STATIC_SPRITE_MODE);
        glUniform1f(_uniforms.static_fraction, frac / 255.0);
        draw_internal(draw_rect);
    }

    virtual void draw_outlined(
            const Rect& draw_rect, const RgbColor& outline_color,
            const RgbColor& fill_color) const {
        glUniform1i(_uniforms.color_mode, OUTLINE_SPRITE_MODE);
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
        glBegin(GL_QUADS);
        glMultiTexCoord2f(GL_TEXTURE0, 1, 1);
        glVertex2f(draw_rect.left, draw_rect.top);
        glMultiTexCoord2f(GL_TEXTURE0, 1, h + 1);
        glVertex2f(draw_rect.left, draw_rect.bottom);
        glMultiTexCoord2f(GL_TEXTURE0, w + 1, h + 1);
        glVertex2f(draw_rect.right, draw_rect.bottom);
        glMultiTexCoord2f(GL_TEXTURE0, w + 1, 1);
        glVertex2f(draw_rect.right, draw_rect.top);
        glEnd();
    }

    virtual void begin_quads() const {
        glUniform1i(_uniforms.color_mode, TINT_SPRITE_MODE);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_RECTANGLE_EXT, _texture.id);
        glBegin(GL_QUADS);
    }

    virtual void end_quads() const {
        glEnd();
    }

    virtual void draw_quad(const Rect& draw_rect, Point origin, const RgbColor& tint) const {
        Rect texture_rect(origin, draw_rect.size());
        texture_rect.offset(1, 1);

        glColor4ub(tint.red, tint.green, tint.blue, 255);
        glMultiTexCoord2f(GL_TEXTURE0, texture_rect.left, texture_rect.top);
        glVertex2f(draw_rect.left, draw_rect.top);
        glMultiTexCoord2f(GL_TEXTURE0, texture_rect.left, texture_rect.bottom);
        glVertex2f(draw_rect.left, draw_rect.bottom);
        glMultiTexCoord2f(GL_TEXTURE0, texture_rect.right, texture_rect.bottom);
        glVertex2f(draw_rect.right, draw_rect.bottom);
        glMultiTexCoord2f(GL_TEXTURE0, texture_rect.right, texture_rect.top);
        glVertex2f(draw_rect.right, draw_rect.top);
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

    DISALLOW_COPY_AND_ASSIGN(OpenGlTextureImpl);
};

}  // namespace

OpenGlVideoDriver::OpenGlVideoDriver()
        : _static_seed{0} { }

Texture OpenGlVideoDriver::texture(PrintItem name, const PixMap& content) {
    return unique_ptr<Texture::Impl>(new OpenGlTextureImpl(name, content, _uniforms));
}

void OpenGlVideoDriver::begin_rects() {
    glUniform1i(_uniforms.color_mode, FILL_MODE);
    glBegin(GL_QUADS);
}

void OpenGlVideoDriver::batch_rect(const Rect& rect, const RgbColor& color) {
    glColor4ub(color.red, color.green, color.blue, color.alpha);
    glVertex2f(rect.right, rect.top);
    glVertex2f(rect.left, rect.top);
    glVertex2f(rect.left, rect.bottom);
    glVertex2f(rect.right, rect.bottom);
}

void OpenGlVideoDriver::end_rects() {
    glEnd();
}

void OpenGlVideoDriver::dither_rect(const Rect& rect, const RgbColor& color) {
    glUniform1i(_uniforms.color_mode, DITHER_MODE);
    glColor4ub(color.red, color.green, color.blue, color.alpha);
    glBegin(GL_QUADS);
    glVertex2f(rect.right, rect.top);
    glVertex2f(rect.left, rect.top);
    glVertex2f(rect.left, rect.bottom);
    glVertex2f(rect.right, rect.bottom);
    glEnd();
}

void OpenGlVideoDriver::begin_points() {
    glUniform1i(_uniforms.color_mode, FILL_MODE);
    glBegin(GL_POINTS);
}

void OpenGlVideoDriver::end_points() {
    glEnd();
}

void OpenGlVideoDriver::batch_point(const Point& at, const RgbColor& color) {
    glColor4ub(color.red, color.green, color.blue, color.alpha);
    glVertex2f(at.h + 0.5, at.v + 0.5);
}

void OpenGlVideoDriver::draw_point(const Point& at, const RgbColor& color) {
    begin_points();
    batch_point(at, color);
    end_points();
}

void OpenGlVideoDriver::begin_lines() {
    glUniform1i(_uniforms.color_mode, FILL_MODE);
    glBegin(GL_LINES);
}

void OpenGlVideoDriver::end_lines() {
    glEnd();
}

void OpenGlVideoDriver::batch_line(
        const Point& from, const Point& to, const RgbColor& color) {
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
    glVertex2f(x1, y1);
    glVertex2f(x2, y2);
}

void OpenGlVideoDriver::draw_line(const Point& from, const Point& to, const RgbColor& color) {
    begin_lines();
    draw_line(from, to, color);
    end_lines();
}

void OpenGlVideoDriver::draw_triangle(const Rect& rect, const RgbColor& color) {
    size_t size = min(rect.width(), rect.height());
    Rect to(0, 0, size, size);
    to.offset(rect.left, rect.top);
    if (_triangles.find(size) == _triangles.end()) {
        ArrayPixMap pix(size, size);
        pix.fill(RgbColor::kClear);
        draw_triangle_up(&pix, RgbColor::kWhite);
        _triangles[size] = texture("", pix);
    }
    _triangles[size].draw_shaded(to, color);
}

void OpenGlVideoDriver::draw_diamond(const Rect& rect, const RgbColor& color) {
    size_t size = min(rect.width(), rect.height());
    Rect to(0, 0, size, size);
    to.offset(rect.left, rect.top);
    if (_diamonds.find(size) == _diamonds.end()) {
        ArrayPixMap pix(size, size);
        pix.fill(RgbColor::kClear);
        draw_compat_diamond(&pix, RgbColor::kWhite);
        _diamonds[size] = texture("", pix);
    }
    _diamonds[size].draw_shaded(to, color);
}

void OpenGlVideoDriver::draw_plus(const Rect& rect, const RgbColor& color) {
    size_t size = min(rect.width(), rect.height());
    Rect to(0, 0, size, size);
    to.offset(rect.left, rect.top);
    if (_pluses.find(size) == _pluses.end()) {
        ArrayPixMap pix(size, size);
        pix.fill(RgbColor::kClear);
        draw_compat_plus(&pix, RgbColor::kWhite);
        _pluses[size] = texture("", pix);
    }
    _pluses[size].draw_shaded(to, color);
}

static GLuint make_shader(GLenum shader_type, const GLchar* source) {
    GLuint shader = glCreateShader(shader_type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
    GLint compiled;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (compiled == GL_FALSE) {
        gl_log(shader);
        throw Exception("compilation failed");
    }
    return shader;
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

    GLuint fragment = make_shader(GL_FRAGMENT_SHADER, glsl::fragment);
    GLuint vertex = make_shader(GL_VERTEX_SHADER, glsl::vertex);

    GLuint program = glCreateProgram();
    glAttachShader(program, fragment);
    glAttachShader(program, vertex);
    glLinkProgram(program);
    glValidateProgram(program);
    GLint linked;
    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    if (linked == GL_FALSE) {
        gl_log(program);
        throw Exception("linking failed");
    }

    driver._uniforms.screen = glGetUniformLocation(program, kShaderScreenUniform);
    driver._uniforms.color_mode = glGetUniformLocation(program, kShaderColorModeUniform);
    driver._uniforms.sprite = glGetUniformLocation(program, kShaderSpriteUniform);
    driver._uniforms.static_image = glGetUniformLocation(program, kShaderStaticImageUniform);
    driver._uniforms.static_fraction = glGetUniformLocation(program, kShaderStaticFractionUniform);
    driver._uniforms.unit = glGetUniformLocation(program, kShaderUnitUniform);
    driver._uniforms.outline_color = glGetUniformLocation(program, kShaderOutlineColorUniform);
    driver._uniforms.seed = glGetUniformLocation(program, kShaderSeedUniform);
    glUseProgram(program);

    GLuint static_texture;
    glGenTextures(1, &static_texture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, static_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    size_t size = 256;
    unique_ptr<uint8_t[]> static_data(new uint8_t[size * size * 2]);
    Random static_index = {0};
    uint8_t* p = static_data.get();
    for (int i = 0; i < (size * size); ++i) {
        *(p++) = 255;
        *(p++) = static_index.next(256);
    }
    glTexImage2D(
            GL_TEXTURE_2D, 0, GL_LUMINANCE_ALPHA, size, size, 0, GL_LUMINANCE_ALPHA,
            GL_UNSIGNED_BYTE, static_data.get());

    auto screen = driver.viewport_size();
    glUniform2f(driver._uniforms.screen, screen.width, screen.height);
    glUniform1i(driver._uniforms.sprite, 0);
    glUniform1i(driver._uniforms.static_image, 1);
}

OpenGlVideoDriver::MainLoop::MainLoop(OpenGlVideoDriver& driver, Card* initial):
        _setup(driver),
        _driver(driver),
        _stack(initial) { }

void OpenGlVideoDriver::MainLoop::draw() {
    if (done()) {
        return;
    }

    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();
    glViewport(0, 0, _driver.viewport_size().width, _driver.viewport_size().height);

    int32_t seed = {_driver._static_seed.next(256)};
    seed <<= 8;
    seed += _driver._static_seed.next(256);
    glUniform1i(_driver._uniforms.seed, seed);

    _stack.top()->draw();

    glFinish();
}

bool OpenGlVideoDriver::MainLoop::done() const {
    return _stack.empty();
}

Card* OpenGlVideoDriver::MainLoop::top() const {
    return _stack.top();
}

}  // namespace antares

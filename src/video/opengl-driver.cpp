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

#include "video/opengl-driver.hpp"

#include <stdint.h>
#include <algorithm>
#include <pn/file>

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

#ifdef __APPLE__
#include <OpenGL/OpenGL.h>
#include <OpenGL/gl3.h>
#else
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glu.h>
#endif

using std::min;
using std::max;
using std::unique_ptr;

namespace antares {

template <typename T>
void Uniform<T>::load(int program) {
    location = glGetUniformLocation(program, name);
}

template <>
void Uniform<int>::set(int value) const {
    glUniform1i(location, value);
}

template <>
void Uniform<float>::set(float value) const {
    glUniform1f(location, value);
}

template <>
void Uniform<vec2>::set(vec2 value) const {
    glUniform2f(location, value.x, value.y);
}

template <>
void Uniform<vec4>::set(vec4 value) const {
    glUniform4f(location, value.x, value.y, value.z, value.w);
}

namespace {

enum {
    FILL_MODE           = 0,
    DITHER_MODE         = 1,
    DRAW_SPRITE_MODE    = 2,
    TINT_SPRITE_MODE    = 3,
    STATIC_SPRITE_MODE  = 4,
    OUTLINE_SPRITE_MODE = 5,
};

#ifndef NDEBUG

static const char* _gl_error_string(GLenum err) {
    switch (err) {
        case GL_NO_ERROR: return "GL_NO_ERROR";
        case GL_INVALID_ENUM: return "GL_INVALID_ENUM";
        case GL_INVALID_VALUE: return "GL_INVALID_VALUE";
        case GL_INVALID_OPERATION: return "GL_INVALID_OPERATION";
        case GL_OUT_OF_MEMORY: return "GL_OUT_OF_MEMORY";
        default: return "?";
    }
}

static void _gl_check(const char* fn, const char* file, int line) {
    int error = glGetError();
    if (error != GL_NO_ERROR) {
        pn::file_view{stderr}.format(
                "{0}: {1} ({2}:{3})\n", fn, _gl_error_string(error), file, line);
    }
}

template <typename T>
static T _gl_check(T t, const char* fn, const char* file, int line) {
    _gl_check(fn, file, line);
    return t;
}

#define _GL(FN, ...) (FN(__VA_ARGS__), _gl_check(#FN, __FILE__, __LINE__))
#define _GLV(FN, ...) _gl_check(FN(__VA_ARGS__), #FN, __FILE__, __LINE__)

#define glActiveTexture(texture) _GL(glActiveTexture, texture)
#define glAttachShader(program, shader) _GL(glAttachShader, program, shader)
#define glBindTexture(target, texture) _GL(glBindTexture, target, texture)
#define glBlendFunc(sfactor, dfactor) _GL(glBlendFunc, sfactor, dfactor)
#define glClear(mask) _GL(glClear, mask)
#define glClearColor(red, green, blue, alpha) _GL(glClearColor, red, green, blue, alpha)
#define glCompileShader(shader) _GL(glCompileShader, shader)
#define glCreateProgram() _GLV(glCreateProgram)
#define glCreateShader(shaderType) _GLV(glCreateShader, shaderType)
#define glDeleteTextures(n, textures) _GL(glDeleteTextures, n, textures)
#define glDisable(cap) _GL(glDisable, cap)
#define glEnable(cap) _GL(glEnable, cap)
#define glFinish() _GL(glFinish)
#define glGenTextures(n, textures) _GL(glGenTextures, n, textures)
// Skip glGetError().
#define glGetProgramInfoLog(program, maxLength, length, infoLog) \
    _GL(glGetProgramInfoLog, program, maxLength, length, infoLog)
#define glGetProgramiv(program, pname, params) _GL(glGetProgramiv, program, pname, params)
#define glGetShaderInfoLog(shader, maxLength, length, infoLog) \
    _GL(glGetShaderInfoLog, shader, maxLength, length, infoLog)
#define glGetShaderiv(shader, pname, params) _GL(glGetShaderiv, shader, pname, params)
#define glGetUniformLocation(program, name) _GLV(glGetUniformLocation, program, name)
// Skip glIsShader().
#define glLinkProgram(program) _GL(glLinkProgram, program)
#define glLoadIdentity() _GL(glLoadIdentity)
#define glPixelStorei(pname, param) _GL(glPixelStorei, pname, param)
#define glShaderSource(shader, count, string, length) \
    _GL(glShaderSource, shader, count, string, length)
#define glTexImage2D(target, level, internalformat, width, height, border, format, type, pixels) \
    _GL(glTexImage2D, target, level, internalformat, width, height, border, format, type, pixels)
#define glUniform1f(location, v0) _GL(glUniform1f, location, v0)
#define glUniform1i(location, v0) _GL(glUniform1i, location, v0)
#define glUniform2f(location, v0, v1) _GL(glUniform2f, location, v0, v1)
#define glUniform4f(location, v0, v1, v2, v3) _GL(glUniform4f, location, v0, v1, v2, v3)
#define glUseProgram(program) _GL(glUseProgram, program)
#define glValidateProgram(program) _GL(glValidateProgram, program)
#define glViewport(x, y, width, height) _GL(glViewport, x, y, width, height)

#define glEnableClientState(array) _GL(glEnableClientState, array)
#define glDisableClientState(array) _GL(glDisableClientState, array)
#define glDrawArrays(mode, first, count) _GL(glDrawArrays, mode, first, count)
#define glGenBuffers(n, buffers) _GL(glGenBuffers, n, buffers)
#define glBindBuffer(target, buffer) _GL(glBindBuffer, target, buffer)
#define glBufferData(target, size, data, usage) _GL(glBufferData, target, size, data, usage)
#define glVertexAttribPointer(index, size, type, normalized, stride, pointer) \
    _GL(glVertexAttribPointer, index, size, type, normalized, stride, pointer)
#define glEnableVertexAttribArray(index) _GL(glEnableVertexAttribArray, index)
#define glDisableVertexAttribArray(index) _GL(glDisableVertexAttribArray, index)

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
    pn::file_view{stderr}.format("object {0} log: {1}\n", object, (const char*)log.get());
}

class OpenGlTextureImpl : public Texture::Impl {
  public:
    OpenGlTextureImpl(
            pn::string_view name, const PixMap& image, int scale,
            const OpenGlVideoDriver::Uniforms& uniforms, GLuint vbuf[3])
            : _name(name.copy()),
              _size(image.size()),
              _scale(scale),
              _uniforms(uniforms),
              _vbuf(vbuf) {
        glBindTexture(GL_TEXTURE_RECTANGLE, _texture.id);
        glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
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
        copy.fill(RgbColor::clear());
        copy.view(Rect(1, 1, size.width - 1, size.height - 1)).copy(image);
        glTexImage2D(
                GL_TEXTURE_RECTANGLE, 0, GL_RGBA, size.width, size.height, 0, GL_BGRA, type,
                copy.bytes());
    }

    virtual pn::string_view name() const { return _name; }

    virtual void draw(const Rect& draw_rect) const {
        _uniforms.color_mode.set(DRAW_SPRITE_MODE);
        draw_internal(draw_rect, RgbColor::white());
    }

    virtual void draw_cropped(const Rect& dest, const Rect& source, const RgbColor& tint) const {
        begin_quads();
        draw_quad(dest, source, tint);
        end_quads();
    }

    virtual void draw_shaded(const Rect& draw_rect, const RgbColor& tint) const {
        _uniforms.color_mode.set(TINT_SPRITE_MODE);
        draw_internal(draw_rect, tint);
    }

    virtual void draw_static(const Rect& draw_rect, const RgbColor& color, uint8_t frac) const {
        _uniforms.color_mode.set(STATIC_SPRITE_MODE);
        _uniforms.static_fraction.set(frac / 255.0f);
        draw_internal(draw_rect, color);
    }

    virtual void draw_outlined(
            const Rect& draw_rect, const RgbColor& outline_color,
            const RgbColor& fill_color) const {
        _uniforms.color_mode.set(OUTLINE_SPRITE_MODE);
        _uniforms.unit.set({float(_size.width) / draw_rect.width(),
                            float(_size.height) / draw_rect.height()});
        _uniforms.outline_color.set({outline_color.red / 255.0f, outline_color.green / 255.0f,
                                     outline_color.blue / 255.0f, outline_color.alpha / 255.0f});
        draw_internal(draw_rect, fill_color);
    }

    virtual const Size& size() const { return _size; }

  private:
    virtual void draw_internal(const Rect& draw_rect, const RgbColor& tint) const {
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glEnableVertexAttribArray(2);

        glBindBuffer(GL_ARRAY_BUFFER, _vbuf[0]);
        GLshort vertices[] = {
                GLshort(draw_rect.left),   GLshort(draw_rect.top),   GLshort(draw_rect.left),
                GLshort(draw_rect.bottom), GLshort(draw_rect.right), GLshort(draw_rect.bottom),
                GLshort(draw_rect.right),  GLshort(draw_rect.top),
        };
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STREAM_DRAW);
        glVertexAttribPointer(0, 2, GL_SHORT, GL_FALSE, 0, nullptr);

        glBindBuffer(GL_ARRAY_BUFFER, _vbuf[1]);
        GLubyte colors[] = {
                tint.red,  tint.green, tint.blue, tint.alpha, tint.red,  tint.green,
                tint.blue, tint.alpha, tint.red,  tint.green, tint.blue, tint.alpha,
                tint.red,  tint.green, tint.blue, tint.alpha,
        };
        glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_STREAM_DRAW);
        glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, nullptr);

        glBindBuffer(GL_ARRAY_BUFFER, _vbuf[2]);
        const int32_t w            = _size.width / _scale;
        const int32_t h            = _size.height / _scale;
        GLshort       tex_coords[] = {
                GLshort(1),     GLshort(1),     GLshort(1),     GLshort(h + 1),
                GLshort(w + 1), GLshort(h + 1), GLshort(w + 1), GLshort(1),
        };
        glBufferData(GL_ARRAY_BUFFER, sizeof(tex_coords), tex_coords, GL_STREAM_DRAW);
        glVertexAttribPointer(2, 2, GL_SHORT, GL_FALSE, 0, nullptr);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_RECTANGLE, _texture.id);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

        glDisableVertexAttribArray(2);
        glDisableVertexAttribArray(1);
        glDisableVertexAttribArray(0);
    }

    virtual void begin_quads() const {
        _uniforms.color_mode.set(TINT_SPRITE_MODE);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_RECTANGLE, _texture.id);
    }

    virtual void end_quads() const {}

    virtual void draw_quad(const Rect& dest, const Rect& source, const RgbColor& tint) const {
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glEnableVertexAttribArray(2);

        glBindBuffer(GL_ARRAY_BUFFER, _vbuf[0]);
        GLshort vertices[] = {
                GLshort(dest.left),   GLshort(dest.top),   GLshort(dest.left),
                GLshort(dest.bottom), GLshort(dest.right), GLshort(dest.bottom),
                GLshort(dest.right),  GLshort(dest.top),
        };
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STREAM_DRAW);
        glVertexAttribPointer(0, 2, GL_SHORT, GL_FALSE, 0, nullptr);

        glBindBuffer(GL_ARRAY_BUFFER, _vbuf[1]);
        GLubyte colors[] = {
                tint.red,  tint.green, tint.blue, tint.alpha, tint.red,  tint.green,
                tint.blue, tint.alpha, tint.red,  tint.green, tint.blue, tint.alpha,
                tint.red,  tint.green, tint.blue, tint.alpha,
        };
        glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_STREAM_DRAW);
        glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, nullptr);

        Rect texture_rect = source;
        texture_rect.scale(_scale, _scale);
        texture_rect.offset(1, 1);
        glBindBuffer(GL_ARRAY_BUFFER, _vbuf[2]);
        GLshort tex_coords[] = {
                GLshort(texture_rect.left),  GLshort(texture_rect.top),
                GLshort(texture_rect.left),  GLshort(texture_rect.bottom),
                GLshort(texture_rect.right), GLshort(texture_rect.bottom),
                GLshort(texture_rect.right), GLshort(texture_rect.top),
        };
        glBufferData(GL_ARRAY_BUFFER, sizeof(tex_coords), tex_coords, GL_STREAM_DRAW);
        glVertexAttribPointer(2, 2, GL_SHORT, GL_FALSE, 0, nullptr);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_RECTANGLE, _texture.id);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

        glDisableVertexAttribArray(2);
        glDisableVertexAttribArray(1);
        glDisableVertexAttribArray(0);
    }

    struct Texture {
        Texture() { glGenTextures(1, &id); }
        Texture(const Texture&) = delete;
        Texture& operator=(const Texture&) = delete;
        ~Texture() { glDeleteTextures(1, &id); }

        GLuint id;
    };

    const pn::string                   _name;
    Texture                            _texture;
    Size                               _size;
    int                                _scale;
    const OpenGlVideoDriver::Uniforms& _uniforms;
    GLuint*                            _vbuf;
};

}  // namespace

OpenGlVideoDriver::OpenGlVideoDriver() : _static_seed{0} {}

int OpenGlVideoDriver::scale() const { return viewport_size().width / screen_size().width; }

Texture OpenGlVideoDriver::texture(pn::string_view name, const PixMap& content, int scale) {
    return unique_ptr<Texture::Impl>(
            new OpenGlTextureImpl(name, content, scale, _uniforms, _vbuf));
}

void OpenGlVideoDriver::begin_rects() { _uniforms.color_mode.set(FILL_MODE); }

void OpenGlVideoDriver::batch_rect(const Rect& rect, const RgbColor& color) {
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, _vbuf[0]);
    GLshort vertices[] = {
            GLshort(rect.right), GLshort(rect.top),    GLshort(rect.left),  GLshort(rect.top),
            GLshort(rect.left),  GLshort(rect.bottom), GLshort(rect.right), GLshort(rect.bottom),
    };
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STREAM_DRAW);
    glVertexAttribPointer(0, 2, GL_SHORT, GL_FALSE, 0, nullptr);

    glBindBuffer(GL_ARRAY_BUFFER, _vbuf[1]);
    GLubyte colors[] = {
            color.red,  color.green, color.blue, color.alpha, color.red,  color.green,
            color.blue, color.alpha, color.red,  color.green, color.blue, color.alpha,
            color.red,  color.green, color.blue, color.alpha,
    };
    glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_STREAM_DRAW);
    glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, nullptr);

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(0);
}

void OpenGlVideoDriver::end_rects() {}

void OpenGlVideoDriver::dither_rect(const Rect& rect, const RgbColor& color) {
    _uniforms.color_mode.set(DITHER_MODE);
    batch_rect(rect, color);
}

void OpenGlVideoDriver::begin_points() { _uniforms.color_mode.set(FILL_MODE); }

void OpenGlVideoDriver::end_points() {}

void OpenGlVideoDriver::batch_point(const Point& at, const RgbColor& color) {
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, _vbuf[0]);
    GLfloat vertices[] = {GLfloat(at.h + 0.5), GLfloat(at.v + 0.5)};
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STREAM_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

    glBindBuffer(GL_ARRAY_BUFFER, _vbuf[1]);
    GLubyte colors[] = {
            color.red, color.green, color.blue, color.alpha,
    };
    glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_STREAM_DRAW);
    glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, nullptr);

    glDrawArrays(GL_POINTS, 0, 1);

    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(0);
}

void OpenGlVideoDriver::draw_point(const Point& at, const RgbColor& color) {
    begin_points();
    batch_point(at, color);
    end_points();
}

void OpenGlVideoDriver::begin_lines() { _uniforms.color_mode.set(FILL_MODE); }

void OpenGlVideoDriver::end_lines() {}

void OpenGlVideoDriver::batch_line(const Point& from, const Point& to, const RgbColor& color) {
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
    } else {
        x2 += 1.0f;
    }

    float y1 = from.v;
    float y2 = to.v;
    if (y1 > y2) {
        y1 += 1.0f;
    } else {
        y2 += 1.0f;
    }

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, _vbuf[0]);
    GLfloat vertices[] = {x1, y1, x2, y2};
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STREAM_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

    glBindBuffer(GL_ARRAY_BUFFER, _vbuf[1]);
    GLubyte colors[] = {
            color.red, color.green, color.blue, color.alpha,
            color.red, color.green, color.blue, color.alpha,
    };
    glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_STREAM_DRAW);
    glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, nullptr);

    glDrawArrays(GL_LINES, 0, 2);

    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(0);
}

void OpenGlVideoDriver::draw_line(const Point& from, const Point& to, const RgbColor& color) {
    // begin_lines();
    // draw_line(from, to, color);
    // end_lines();
}

void OpenGlVideoDriver::draw_triangle(const Rect& rect, const RgbColor& color) {
    size_t size = min(rect.width(), rect.height());
    Rect   to(0, 0, size, size);
    to.offset(rect.left, rect.top);
    if (_triangles.find(size) == _triangles.end()) {
        ArrayPixMap pix(size, size);
        pix.fill(RgbColor::clear());
        draw_triangle_up(&pix, RgbColor::white());
        _triangles[size] = texture("", pix, 1);
    }
    _triangles[size].draw_shaded(to, color);
}

void OpenGlVideoDriver::draw_diamond(const Rect& rect, const RgbColor& color) {
    size_t size = min(rect.width(), rect.height());
    Rect   to(0, 0, size, size);
    to.offset(rect.left, rect.top);
    if (_diamonds.find(size) == _diamonds.end()) {
        ArrayPixMap pix(size, size);
        pix.fill(RgbColor::clear());
        draw_compat_diamond(&pix, RgbColor::white());
        _diamonds[size] = texture("", pix, 1);
    }
    _diamonds[size].draw_shaded(to, color);
}

void OpenGlVideoDriver::draw_plus(const Rect& rect, const RgbColor& color) {
    size_t size = min(rect.width(), rect.height());
    Rect   to(0, 0, size, size);
    to.offset(rect.left, rect.top);
    if (_pluses.find(size) == _pluses.end()) {
        ArrayPixMap pix(size, size);
        pix.fill(RgbColor::clear());
        draw_compat_plus(&pix, RgbColor::white());
        _pluses[size] = texture("", pix, 1);
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
        throw std::runtime_error("compilation failed");
    }
    return shader;
}

OpenGlVideoDriver::MainLoop::Setup::Setup(OpenGlVideoDriver& driver) {
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glClearColor(0, 0, 0, 1);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);

    GLuint fragment = make_shader(GL_FRAGMENT_SHADER, glsl::fragment);
    GLuint vertex   = make_shader(GL_VERTEX_SHADER, glsl::vertex);

    GLuint program = glCreateProgram();
    glAttachShader(program, fragment);
    glAttachShader(program, vertex);
    glBindAttribLocation(program, 0, "vertex");
    glBindAttribLocation(program, 1, "in_color");
    glBindAttribLocation(program, 2, "tex_coord");
    glLinkProgram(program);
    glValidateProgram(program);
    GLint linked;
    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    if (linked == GL_FALSE) {
        gl_log(program);
        throw std::runtime_error("linking failed");
    }

    GLuint array;
    glGenVertexArrays(1, &array);
    glBindVertexArray(array);

    glGenBuffers(3, driver._vbuf);

    driver._uniforms.screen.load(program);
    driver._uniforms.scale.load(program);
    driver._uniforms.color_mode.load(program);
    driver._uniforms.sprite.load(program);
    driver._uniforms.static_image.load(program);
    driver._uniforms.static_fraction.load(program);
    driver._uniforms.unit.load(program);
    driver._uniforms.outline_color.load(program);
    driver._uniforms.seed.load(program);
    glUseProgram(program);

    GLuint static_texture;
    glGenTextures(1, &static_texture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, static_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    size_t                size = 256;
    unique_ptr<uint8_t[]> static_data(new uint8_t[size * size * 2]);
    Random                static_index = {0};
    uint8_t*              p            = static_data.get();
    for (int i = 0; i < (size * size); ++i) {
        *(p++) = 255;
        *(p++) = static_index.next(256);
    }
    glTexImage2D(
            GL_TEXTURE_2D, 0, GL_RG, size, size, 0, GL_RG, GL_UNSIGNED_BYTE, static_data.get());

    driver._uniforms.sprite.set(0);
    driver._uniforms.static_image.set(1);
}

OpenGlVideoDriver::MainLoop::MainLoop(OpenGlVideoDriver& driver, Card* initial)
        : _setup(driver), _driver(driver), _stack(initial) {}

void OpenGlVideoDriver::MainLoop::draw() {
    if (done()) {
        return;
    }

    glClear(GL_COLOR_BUFFER_BIT);
    glViewport(0, 0, _driver.viewport_size().width, _driver.viewport_size().height);

    auto screen = _driver.screen_size();
    _driver._uniforms.screen.set({screen.width * 1.0f, screen.height * 1.0f});
    _driver._uniforms.scale.set(_driver.scale());

    int32_t seed = {_driver._static_seed.next(256)};
    seed <<= 8;
    seed += _driver._static_seed.next(256);
    _driver._uniforms.seed.set(seed);

    _stack.top()->draw();

    glFinish();
}

bool OpenGlVideoDriver::MainLoop::done() const { return _stack.empty(); }

Card* OpenGlVideoDriver::MainLoop::top() const { return _stack.top(); }

}  // namespace antares

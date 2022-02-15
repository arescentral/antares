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

#include <GL/gl.h>

#include "video/opengl-dynamic.hpp"
#include "video/opengl-driver.hpp"

namespace antares {

struct OpenGlDynLinkFunctions {
    void (APIENTRYP glClearColor)( GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha );
    void (APIENTRYP glClear)( GLbitfield mask );
    void (APIENTRYP glBlendFunc)( GLenum sfactor, GLenum dfactor );
    void (APIENTRYP glEnable)( GLenum cap );
    void (APIENTRYP glFinish)( void );
    void (APIENTRYP glViewport)( GLint x, GLint y, GLsizei width, GLsizei height );
    void (APIENTRYP glDrawArrays)( GLenum mode, GLint first, GLsizei count );
    void (APIENTRYP glPixelStorei)( GLenum pname, GLint param );
    void (APIENTRYP glTexParameteri)( GLenum target, GLenum pname, GLint param );
    void (APIENTRYP glTexImage2D)( GLenum target, GLint level, GLint internalFormat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels );
    void (APIENTRYP glGenTextures)( GLsizei n, GLuint *textures );
    void (APIENTRYP glDeleteTextures)( GLsizei n, const GLuint *textures);
    VOID (APIENTRYP glBindTexture)( GLenum target, GLuint texture );

    PFNGLACTIVETEXTUREPROC glActiveTexture;
    PFNGLBINDBUFFERPROC glBindBuffer;
    PFNGLGENBUFFERSPROC glGenBuffers;
    PFNGLBUFFERDATAPROC glBufferData;
    PFNGLATTACHSHADERPROC glAttachShader;

    PFNGLBINDATTRIBLOCATIONPROC glBindAttribLocation;
    PFNGLCOMPILESHADERPROC glCompileShader;
    PFNGLCREATEPROGRAMPROC glCreateProgram;
    PFNGLCREATESHADERPROC glCreateShader;
    PFNGLDISABLEVERTEXATTRIBARRAYPROC glDisableVertexAttribArray;
    PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
    PFNGLGETPROGRAMIVPROC glGetProgramiv;
    PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog;
    PFNGLGETSHADERIVPROC glGetShaderiv;
    PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;
    PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation;
    PFNGLISSHADERPROC glIsShader;
    PFNGLLINKPROGRAMPROC glLinkProgram;
    PFNGLSHADERSOURCEPROC glShaderSource;
    PFNGLUSEPROGRAMPROC glUseProgram;
    PFNGLUNIFORM1FPROC glUniform1f;
    PFNGLUNIFORM2FPROC glUniform2f;
    PFNGLUNIFORM4FPROC glUniform4f;
    PFNGLUNIFORM1IPROC glUniform1i;
    PFNGLVALIDATEPROGRAMPROC glValidateProgram;
    PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer;
    PFNGLBINDVERTEXARRAYPROC glBindVertexArray;
    PFNGLGENVERTEXARRAYSPROC glGenVertexArrays;
    
    template<typename pfn>
    static void set_one(pfn& field, OpenGlVideoDriver& driver, const char* proc_name);

    static OpenGlDynLinkFunctions instance;
};

template<typename pfn>
void OpenGlDynLinkFunctions::set_one(pfn& field, OpenGlVideoDriver& driver, const char* proc_name) {
    field = static_cast<pfn>(driver.get_proc_address(proc_name));
}

OpenGlDynLinkFunctions OpenGlDynLinkFunctions::instance;

#define LINK_FUNC(proc_name) \
    OpenGlDynLinkFunctions::set_one(OpenGlDynLinkFunctions::instance.proc_name, driver, #proc_name)

typedef void (APIENTRYP PFNGLACTIVETEXTUREPROC) (GLenum texture);

void OpenGlDynLink::init_funcs(OpenGlVideoDriver& driver) {
    LINK_FUNC(glActiveTexture);
    LINK_FUNC(glBindBuffer);
    LINK_FUNC(glGenBuffers);
    LINK_FUNC(glBufferData);
    LINK_FUNC(glAttachShader);
    LINK_FUNC(glClearColor);
    LINK_FUNC(glClear);
    LINK_FUNC(glBlendFunc);
    LINK_FUNC(glEnable);
    LINK_FUNC(glFinish);
    LINK_FUNC(glViewport);
    LINK_FUNC(glDrawArrays);
    LINK_FUNC(glPixelStorei);
    LINK_FUNC(glTexParameteri);
    LINK_FUNC(glTexImage2D);
    LINK_FUNC(glGenTextures);
    LINK_FUNC(glDeleteTextures);
    LINK_FUNC(glBindTexture);
    LINK_FUNC(glBindAttribLocation);
    LINK_FUNC(glCompileShader);
    LINK_FUNC(glCreateProgram);
    LINK_FUNC(glCreateShader);
    LINK_FUNC(glDisableVertexAttribArray);
    LINK_FUNC(glEnableVertexAttribArray);
    LINK_FUNC(glGetProgramiv);
    LINK_FUNC(glGetProgramInfoLog);
    LINK_FUNC(glGetShaderiv);
    LINK_FUNC(glGetShaderInfoLog);
    LINK_FUNC(glGetUniformLocation);
    LINK_FUNC(glIsShader);
    LINK_FUNC(glLinkProgram);
    LINK_FUNC(glShaderSource);
    LINK_FUNC(glUseProgram);
    LINK_FUNC(glUniform1f);
    LINK_FUNC(glUniform2f);
    LINK_FUNC(glUniform4f);
    LINK_FUNC(glUniform1i);
    LINK_FUNC(glValidateProgram);
    LINK_FUNC(glVertexAttribPointer);
    LINK_FUNC(glBindVertexArray);
    LINK_FUNC(glGenVertexArrays);
}

}  // namespace antares

#define DLF antares::OpenGlDynLinkFunctions::instance

extern "C" {

GLAPI void GLAPIENTRY glActiveTexture(GLenum texture) {
    DLF.glActiveTexture(texture);
}

GLAPI void APIENTRY glBindBuffer (GLenum target, GLuint buffer) {
    DLF.glBindBuffer(target, buffer);
}

GLAPI void APIENTRY glGenBuffers (GLsizei n, GLuint *buffers) {
    DLF.glGenBuffers(n, buffers);
}

GLAPI void APIENTRY glBufferData (GLenum target, GLsizeiptr size, const void *data, GLenum usage) {
    DLF.glBufferData(target, size, data, usage);
}

GLAPI void APIENTRY glAttachShader (GLuint program, GLuint shader) {
    DLF.glAttachShader(program, shader);
}

GLAPI void GLAPIENTRY glClearColor( GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha ) {
    DLF.glClearColor(red, green, blue, alpha);
}

GLAPI void GLAPIENTRY glClear( GLbitfield mask ) {
    DLF.glClear(mask);
}

GLAPI void GLAPIENTRY glBlendFunc( GLenum sfactor, GLenum dfactor ) {
    DLF.glBlendFunc(sfactor, dfactor);
}

GLAPI void GLAPIENTRY glEnable( GLenum cap ) {
    DLF.glEnable(cap);
}

GLAPI void GLAPIENTRY glFinish( void ) {
    DLF.glFinish();
}

GLAPI void GLAPIENTRY glViewport( GLint x, GLint y, GLsizei width, GLsizei height ) {
    DLF.glViewport(x, y, width, height);
}

GLAPI void GLAPIENTRY glDrawArrays( GLenum mode, GLint first, GLsizei count ) {
    DLF.glDrawArrays(mode, first, count);
}

GLAPI void GLAPIENTRY glPixelStorei( GLenum pname, GLint param ) {
    DLF.glPixelStorei(pname, param);
}

GLAPI void GLAPIENTRY glTexParameteri( GLenum target, GLenum pname, GLint param ) {
    DLF.glTexParameteri(target, pname, param);
}

GLAPI void GLAPIENTRY glTexImage2D( GLenum target, GLint level, GLint internalFormat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels ) {
    DLF.glTexImage2D(target, level, internalFormat, width, height, border, format, type, pixels);
}

GLAPI void GLAPIENTRY glGenTextures( GLsizei n, GLuint *textures ) {
    DLF.glGenTextures(n, textures);
}

GLAPI void GLAPIENTRY glDeleteTextures( GLsizei n, const GLuint *textures) {
    DLF.glDeleteTextures(n, textures);
}

GLAPI void GLAPIENTRY glBindTexture( GLenum target, GLuint texture ) {
    DLF.glBindTexture(target, texture);
}

GLAPI void APIENTRY glBindAttribLocation (GLuint program, GLuint index, const GLchar *name) {
    DLF.glBindAttribLocation(program, index, name);
}

GLAPI void APIENTRY glCompileShader (GLuint shader) {
    DLF.glCompileShader(shader);
}

GLAPI GLuint APIENTRY glCreateProgram (void) {
    return DLF.glCreateProgram();
}

GLAPI GLuint APIENTRY glCreateShader (GLenum type) {
    return DLF.glCreateShader(type);
}

GLAPI void APIENTRY glDisableVertexAttribArray (GLuint index) {
    DLF.glDisableVertexAttribArray(index);
}

GLAPI void APIENTRY glEnableVertexAttribArray (GLuint index) {
    DLF.glEnableVertexAttribArray(index);
}

GLAPI void APIENTRY glGetProgramiv (GLuint program, GLenum pname, GLint *params) {
    DLF.glGetProgramiv(program, pname, params);
}

GLAPI void APIENTRY glGetProgramInfoLog (GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog) {
    DLF.glGetProgramInfoLog(program, bufSize, length, infoLog);
}

GLAPI void APIENTRY glGetShaderiv (GLuint shader, GLenum pname, GLint *params) {
    DLF.glGetShaderiv(shader, pname, params);
}

GLAPI void APIENTRY glGetShaderInfoLog (GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog) {
    DLF.glGetShaderInfoLog(shader, bufSize, length, infoLog);
}

GLAPI GLint APIENTRY glGetUniformLocation (GLuint program, const GLchar *name) {
    return DLF.glGetUniformLocation(program, name);
}

GLAPI GLboolean APIENTRY glIsShader (GLuint shader) {
    return DLF.glIsShader(shader);
}

GLAPI void APIENTRY glLinkProgram (GLuint program) {
    DLF.glLinkProgram(program);
}

GLAPI void APIENTRY glShaderSource (GLuint shader, GLsizei count, const GLchar *const*string, const GLint *length) {
    DLF.glShaderSource(shader, count, string, length);
}

GLAPI void APIENTRY glUseProgram (GLuint program) {
    DLF.glUseProgram(program);
}

GLAPI void APIENTRY glUniform1f (GLint location, GLfloat v0) {
    DLF.glUniform1f(location, v0);
}

GLAPI void APIENTRY glUniform2f (GLint location, GLfloat v0, GLfloat v1) {
    DLF.glUniform2f(location, v0, v1);
}

GLAPI void APIENTRY glUniform4f (GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3) {
    DLF.glUniform4f(location, v0, v1, v2, v3);
}

GLAPI void APIENTRY glUniform1i (GLint location, GLint v0) {
    DLF.glUniform1i(location, v0);
}

GLAPI void APIENTRY glValidateProgram (GLuint program) {
    DLF.glValidateProgram(program);
}

GLAPI void APIENTRY glVertexAttribPointer (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer) {
    DLF.glVertexAttribPointer(index, size, type, normalized, stride, pointer);
}

GLAPI void APIENTRY glBindVertexArray (GLuint array) {
    DLF.glBindVertexArray(array);
}

GLAPI void APIENTRY glGenVertexArrays (GLsizei n, GLuint *arrays) {
    DLF.glGenVertexArrays(n, arrays);
}

}  // extern "C"

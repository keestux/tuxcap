/*
 * File:   GLExtensions.h
 * Author: W.P. van Paassen
 *
 * Created on March 16, 2011, 8:37 AM
 */

#ifndef GLEXTENSIONS_H
#define	GLEXTENSIONS_H

#include "Common.h"

#include <SDL.h>
#ifdef USE_OPENGLES
#include <SDL_opengles.h>
#else
#include <SDL_opengl.h>
#endif

typedef void (APIENTRY * glBindBuffer_Func) (GLenum, GLuint);
typedef void (APIENTRY * glBufferData_Func) (GLenum, GLsizeiptr, const GLvoid *, GLenum);
typedef void (APIENTRY * glBufferSubData_Func) (GLenum, GLintptr, GLsizeiptr, const GLvoid *);
typedef void (APIENTRY * glDeleteBuffers_Func) (GLsizei, const GLuint *);
typedef void (APIENTRY * glGenBuffers_Func) (GLsizei, GLuint *);
typedef GLvoid* (APIENTRY * glMapBuffer_Func) (GLenum, GLenum);
typedef GLboolean (APIENTRY * glUnmapBuffer_Func) (GLenum);

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

class GLExtensions {
 public:

    static bool glIsExtensionSupported(const char *extension);
    static bool glIsVersionOrHigher(const char* version);
    static bool glEnableVertexBufferObjects();
    static bool glIsCompressedTexImage2DSupported();

    //vertex buffer object functions part of opengl
    static glBindBuffer_Func glBindBuffer_ptr ;
    static glBufferData_Func glBufferData_ptr ;
    static glBufferSubData_Func glBufferSubData_ptr ;
    static glDeleteBuffers_Func glDeleteBuffers_ptr ;
    static glGenBuffers_Func glGenBuffers_ptr ;
    static glMapBuffer_Func glMapBuffer_ptr ;
    static glUnmapBuffer_Func glUnmapBuffer_ptr ;
};

#endif	/* GLEXTENSIONS_H */

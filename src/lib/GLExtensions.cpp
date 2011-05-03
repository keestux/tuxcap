#include "Common.h"
#include "GLExtensions.h"

#define major_number_opengl 0
#define minor_number_opengl 2
#define major_number_opengles 13
#define minor_number_opengles 15

glBindBuffer_Func GLExtensions::glBindBuffer_ptr = NULL ;
glBufferData_Func GLExtensions::glBufferData_ptr = NULL ;
glBufferSubData_Func GLExtensions::glBufferSubData_ptr = NULL ;
glDeleteBuffers_Func GLExtensions::glDeleteBuffers_ptr = NULL ;
glGenBuffers_Func GLExtensions::glGenBuffers_ptr = NULL ;
glMapBuffer_Func GLExtensions::glMapBuffer_ptr = NULL ;
glUnmapBuffer_Func GLExtensions::glUnmapBuffer_ptr = NULL ;

//Used for checking the opengl version
//Version should be a null-terminated string consisting of major_number.minor_number
bool GLExtensions::glIsVersionOrHigher(const char* version) {
    int major, minor;

    const char* glVersion = (const char*)glGetString(GL_VERSION);

#ifndef USE_OPENGLES
    major = major_number_opengl;
    minor = minor_number_opengl;
#else
    major = major_number_opengles;
    minor = minor_number_opengles;
#endif

    if (glVersion[major] > version[0])
        return true;
    if (glVersion[major] < version[0])
        return false;

    return glVersion[minor] >= version[2];
}

bool GLExtensions::glIsExtensionSupported(const char *extension) {
  const GLubyte *extensions = NULL;
  const GLubyte *start;

  GLubyte *where, *terminator;
  /* Extension names should not have spaces. */
  where = (GLubyte *) strchr(extension, ' ');

  if (where || *extension == '\0')
    return false;

  extensions = glGetString(GL_EXTENSIONS);

  /* It takes a bit of care to be fool-proof about parsing the

     OpenGL extensions string. Don't be fooled by sub-strings,

     etc. */
  start = extensions;

  for (;;) {
    where = (GLubyte *) strstr((const char *) start, extension);

    if (!where)
      break;

    terminator = where + strlen(extension);

    if (where == start || *(where - 1) == ' ')

      if (*terminator == ' ' || *terminator == '\0')
        return true;

    start = terminator;
  }
  return false;
}

bool GLExtensions::glEnableVertexBufferObjects() {
    bool isVertexBufferObjectsAnExtension;
#ifdef USE_OPENGLES
    isVertexBufferObjectsAnExtension = !glIsVersionOrHigher("1.1");
#else
    isVertexBufferObjectsAnExtension = !glIsVersionOrHigher("1.5");
#endif

    if (isVertexBufferObjectsAnExtension &&
        !glIsExtensionSupported("GL_ARB_vertex_buffer_object"))
        return false;

    if (!isVertexBufferObjectsAnExtension) {
#if TARGET_OS_IPHONE == 0
        glBindBuffer_ptr = (glBindBuffer_Func)SDL_GL_GetProcAddress("glBindBuffer");
        glBufferData_ptr = (glBufferData_Func)SDL_GL_GetProcAddress("glBufferData");
        glBufferSubData_ptr = (glBufferSubData_Func)SDL_GL_GetProcAddress("glBufferSubData");
        glDeleteBuffers_ptr = (glDeleteBuffers_Func)SDL_GL_GetProcAddress("glDeleteBuffers");
        glGenBuffers_ptr = (glGenBuffers_Func)SDL_GL_GetProcAddress("glGenBuffers");
        glMapBuffer_ptr = (glMapBuffer_Func)SDL_GL_GetProcAddress("glMapBuffer");
        glUnmapBuffer_ptr = (glUnmapBuffer_Func)SDL_GL_GetProcAddress("glUnmapBuffer");
#else
        glBindBuffer_ptr = &glBindBuffer;
        glBufferData_ptr = &glBufferData;
        glBufferSubData_ptr = &glBufferSubData;
        glDeleteBuffers_ptr = &glDeleteBuffers;
        glGenBuffers_ptr = &glGenBuffers;
        glMapBuffer_ptr = &glMapBufferOES;
        glUnmapBuffer_ptr = &glUnmapBufferOES;
#endif
        return glBindBuffer_ptr && glBufferData_ptr && glBufferSubData_ptr && glDeleteBuffers_ptr && glGenBuffers_ptr && glMapBuffer_ptr && glUnmapBuffer_ptr;
    }

    return false;
}

bool GLExtensions::glIsCompressedTexImage2DSupported() {
#ifndef USE_OPENGLES
    return glIsVersionOrHigher("1.3");
#endif
    return true;
}

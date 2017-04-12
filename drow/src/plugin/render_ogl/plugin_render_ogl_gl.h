#ifndef UI_RENDER_OGL_GL_H
#define UI_RENDER_OGL_GL_H

/* openGL header */
#if defined(IN_EXPORT)
#       define DROW_RENDER_GLDUMMY
#       include "GLES2/gl2dummy.h"
#else
#   if defined(_WIN32)
#       if defined(__MINGW32__)
#           define DROW_RENDER_OPENGLES
#           include "gl.h"
#           include "glext.h"
#       elif defined(_MSC_VER)
#           define DROW_RENDER_OPENGL
#           include "GLee.h"
#       else
#           error("unknown win32 sub system!")
#       endif
#   elif defined(_APPLE)
#       include <ConditionalMacros.h>
#       if TARGET_OS_IPHONE
#           define DROW_RENDER_OPENGLES
#           include <OpenGLES/ES1/gl.h>
#           include <OpenGLES/ES1/glext.h>
#           include <OpenGLES/ES2/gl.h>
#           include <OpenGLES/ES2/glext.h>
#       elif TARGET_OS_MAC
#           define DROW_RENDER_OPENGL
#           include <OpenGL/gl.h>
#       else
#           error("unknown apple sub system!")
#       endif
#   elif defined(ANDROID)
#       define DROW_RENDER_OPENGLES
#       include <GLES/gl.h>
#       include <GLES/glext.h>
#       include <GLES2/gl2.h>
#       include <GLES2/gl2ext.h>
#   elif defined(FLEX)
#       define DROW_RENDER_OPENGL
#       include <GL/gl.h>
#   elif defined(EMSCRIPTEN)
#       define DROW_RENDER_OPENGLES
#       include <GLES2/gl2.h>
#       include <GLES2/gl2ext.h>
#   else
#       error("unknown system!")
#   endif
#endif

#if defined TARGET_OS_IPHONE
#define DROW_RENDER_USE_VAO 1
#define DROW_RENDER_USE_MAP_BUFFER 1
#endif
    
#endif

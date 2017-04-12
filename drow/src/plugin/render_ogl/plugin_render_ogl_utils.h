#ifndef UI_RENDER_OGL_UTILS_H
#define UI_RENDER_OGL_UTILS_H
#include "plugin_render_ogl_gl.h"
#include "render/runtime/ui_runtime_module.h"

#ifdef __cplusplus
extern "C" {
#endif

const char* gl_utils_strerror(GLenum err);

ui_runtime_render_program_attr_id_t plugin_render_ogl_program_attr_id_from_str(const char * str);
    
GLint plugin_render_ogl_format_api_format(ui_cache_pixel_format_t format);
GLenum plugin_render_ogl_format_api_type(ui_cache_pixel_format_t format);

#if DEBUG
#define DROW_RENDER_OGL_ERROR_DEBUG(__module)                   \
    do {                                                        \
        GLenum __error = glGetError();                          \
        if(__error) {                                           \
            CPE_ERROR((__module)->m_em,                         \
                      "OpenGL error 0x%04X(%s) in %s %s %d\n",  \
                      __error, gl_utils_strerror(__error),      \
                      __FILE__, __FUNCTION__, __LINE__);        \
        }                                                       \
    } while (0)
#else
#define DROW_RENDER_OGL_ERROR_DEBUG(__module)
#endif

#ifdef __cplusplus
}
#endif
    
#endif

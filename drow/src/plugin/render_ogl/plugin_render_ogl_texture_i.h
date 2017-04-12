#ifndef UI_RUNTIME_OGL_TEXTURE_DATA_I_H
#define UI_RUNTIME_OGL_TEXTURE_DATA_I_H
#include "plugin_render_ogl_module_i.h"

struct plugin_render_ogl_texture {
    plugin_render_ogl_module_t m_module;
    GLuint m_texture_id;
    ui_runtime_render_texture_filter_t m_min_filter;
    ui_runtime_render_texture_filter_t m_mag_filter;
    ui_runtime_render_texture_wrapping_t m_wrap_s;
    ui_runtime_render_texture_wrapping_t m_wrap_t;
};

GLuint plugin_render_ogl_texture_gl_texture_id(ui_cache_res_t res);
int plugin_render_ogl_texture_upload(ui_cache_res_t res);

int plugin_render_ogl_module_init_res_plugin(plugin_render_ogl_module_t module);
void plugin_render_ogl_module_fini_res_plugin(plugin_render_ogl_module_t module);

#endif

#ifndef UI_RENDER_OGL_SHADER_I_H
#define UI_RENDER_OGL_SHADER_I_H
#include "plugin_render_ogl_module_i.h"

struct plugin_render_ogl_shader {
    plugin_render_ogl_module_t m_module;
    TAILQ_ENTRY(plugin_render_ogl_shader) m_next;
    char m_name[64];
    GLuint m_shader_type;
    const char * m_source;
    uint32_t m_ref_count;
    GLuint m_shader_id;
};

plugin_render_ogl_shader_t
plugin_render_ogl_shader_create_from_text(plugin_render_ogl_module_t module, const char * name, GLuint shader_type, const char * source);

void plugin_render_ogl_shader_free(plugin_render_ogl_shader_t shader);

void plugin_render_ogl_shader_free_all(plugin_render_ogl_module_t module);

const char * plugin_render_ogl_shader_name(plugin_render_ogl_shader_t shader);
plugin_render_ogl_shader_t plugin_render_ogl_shader_find_by_name(plugin_render_ogl_module_t runtime, const char * name);

int plugin_render_ogl_shader_compile(plugin_render_ogl_shader_t shader);

#endif

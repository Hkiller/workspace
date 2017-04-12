#ifndef UI_RENDER_OGL_PROGRAM_I_H
#define UI_RENDER_OGL_PROGRAM_I_H
#include "render/runtime/ui_runtime_render_program.h"
#include "plugin_render_ogl_module_i.h"

struct plugin_render_ogl_program {
    plugin_render_ogl_module_t m_module;
    plugin_render_ogl_shader_t m_v_shader;
    plugin_render_ogl_shader_t m_p_shader;
    GLuint m_program_id;
    uint32_t m_attr_mask;
};

int plugin_render_ogl_program_init(void * ctx, ui_runtime_render_program_t program);
void plugin_render_ogl_program_fini(void * ctx, ui_runtime_render_program_t program, uint8_t is_external_unloaded);

plugin_render_ogl_shader_t plugin_render_ogl_program_v_shader(plugin_render_ogl_program_t program);
int plugin_render_ogl_program_set_v_shader(plugin_render_ogl_program_t program, plugin_render_ogl_shader_t v_shader);

plugin_render_ogl_shader_t plugin_render_ogl_program_p_shader(plugin_render_ogl_program_t program);
int plugin_render_ogl_program_set_p_shader(plugin_render_ogl_program_t program, plugin_render_ogl_shader_t p_shader);

int plugin_render_ogl_program_link(ui_runtime_render_program_t i_program, plugin_render_ogl_program_t program);

int plugin_render_ogl_program_bind(plugin_render_ogl_program_t program);
void plugin_render_ogl_program_unbind(plugin_render_ogl_program_t program);

/*unif*/
struct plugin_render_ogl_program_unif {
    GLuint m_location;
};

int plugin_render_ogl_program_unif_init(void * ctx, ui_runtime_render_program_unif_t program_unif);
void plugin_render_ogl_program_unif_fini(void * ctx, ui_runtime_render_program_unif_t program_unif);

/*attr*/
struct plugin_render_ogl_program_attr {
    GLuint m_location;
};

int plugin_render_ogl_program_attr_init(void * ctx, ui_runtime_render_program_attr_t program_attr);
void plugin_render_ogl_program_attr_fini(void * ctx, ui_runtime_render_program_attr_t program_attr);

#endif

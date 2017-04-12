#ifndef UI_RENDER_OGL_CACHE_I_H
#define UI_RENDER_OGL_CACHE_I_H
#include "render/utils/ui_vector_2.h"
#include "render/utils/ui_color.h"
#include "render/runtime/ui_runtime_render_state.h"
#include "plugin_render_ogl_module_i.h"

typedef enum plugin_render_ogl_buffer_type {
    plugin_render_ogl_buffer_array,
    plugin_render_ogl_buffer_element,
    plugin_render_ogl_buffer_count,
} plugin_render_ogl_buffer_type_t;

struct plugin_render_ogl_cache {
    ui_vector_2 m_view_point;
    ui_color m_clear_color;
    uint8_t m_active_texture;
    GLuint m_textures[16];
    GLuint m_program;
    struct ui_runtime_render_state_data m_state;
    GLuint m_vao;
    uint32_t m_attr_flag;
    GLuint m_buffs[plugin_render_ogl_buffer_count];
};

int plugin_render_ogl_cache_init(plugin_render_ogl_module_t module);
void plugin_render_ogl_cache_fini(plugin_render_ogl_module_t module);

void plugin_render_ogl_state_save(void * ctx, ui_runtime_render_state_data_t state_data);
void plugin_render_ogl_state_restore(void * ctx, ui_runtime_render_state_data_t queue);

void plugin_render_ogl_clear(plugin_render_ogl_module_t module, ui_color_t c);
void plugin_render_ogl_set_view_point(plugin_render_ogl_module_t module, ui_vector_2_t sz);

void plugin_render_ogl_active_vao(plugin_render_ogl_module_t module, GLuint vao);
void plugin_render_ogl_bind_buffer(plugin_render_ogl_module_t module, plugin_render_ogl_buffer_type_t t, GLuint buffer);

void plugin_render_ogl_set_texture_filter(plugin_render_ogl_module_t module, GLenum pname, ui_runtime_render_texture_filter_t filter);
void plugin_render_ogl_set_texture_wrapping(plugin_render_ogl_module_t module, GLenum pname, ui_runtime_render_texture_wrapping_t wrap);

void plugin_render_ogl_bind_texture_for_manip(
    plugin_render_ogl_module_t module,
    ui_cache_res_t res, uint8_t index);

void plugin_render_ogl_bind_texture(
    plugin_render_ogl_module_t module,
    ui_cache_res_t res,
    ui_runtime_render_texture_filter_t min_filter,
    ui_runtime_render_texture_filter_t mag_filter,
    ui_runtime_render_texture_wrapping_t wrap_s,
    ui_runtime_render_texture_wrapping_t wrap_t,
    uint8_t index);

void plugin_render_ogl_set_scissor(plugin_render_ogl_module_t module, ui_rect_t scissor);
void plugin_render_ogl_set_blend(plugin_render_ogl_module_t module, ui_runtime_render_blend_t blend);
void plugin_render_ogl_set_cull_face(plugin_render_ogl_module_t module, ui_runtime_render_cull_face_t cull_face);

void plugin_render_ogl_use_program(plugin_render_ogl_module_t module, GLuint program);

void plugin_render_ogl_enable_program_attrs(plugin_render_ogl_module_t module, uint32_t flags);
void plugin_render_ogl_enable_client_states(plugin_render_ogl_module_t module, uint32_t flags);

void plugin_render_ogl_buff_copy_from_mem(
    plugin_render_ogl_module_t module,
    plugin_render_ogl_buffer_type_t type, uint32_t size, void const * data, GLenum usage);

#endif


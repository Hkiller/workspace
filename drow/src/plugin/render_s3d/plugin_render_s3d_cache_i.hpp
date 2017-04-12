#ifndef UI_RENDER_S3D_CACHE_I_H
#define UI_RENDER_S3D_CACHE_I_H
#include "render/utils/ui_vector_2.h"
#include "render/utils/ui_color.h"
#include "render/runtime/ui_runtime_render_state.h"
#include "plugin_render_s3d_module_i.hpp"

struct plugin_render_s3d_cache {
    ui_vector_2 m_view_point;
    plugin_render_s3d_texture_t m_textures[16];
    uint32_t m_using_vertexes;
    
    struct ui_runtime_render_state_data m_state;

    plugin_render_s3d_program_t m_program;
};

int plugin_render_s3d_cache_init(plugin_render_s3d_module_t module);
void plugin_render_s3d_cache_fini(plugin_render_s3d_module_t module);
void plugin_render_s3d_cache_clear(plugin_render_s3d_module_t module);

void plugin_render_s3d_state_save(void * ctx, ui_runtime_render_state_data_t state_data);
void plugin_render_s3d_state_restore(void * ctx, ui_runtime_render_state_data_t queue);

void plugin_render_s3d_clear(plugin_render_s3d_module_t module, ui_color_t color);
void plugin_render_s3d_set_view_point(plugin_render_s3d_module_t module, ui_vector_2_t sz);

void plugin_render_s3d_set_scissor(plugin_render_s3d_module_t module, ui_rect_t scissor);
void plugin_render_s3d_set_blend(plugin_render_s3d_module_t module, ui_runtime_render_blend_t blend);
void plugin_render_s3d_set_cull_face(plugin_render_s3d_module_t module, ui_runtime_render_cull_face_t cull_face);

void plugin_render_s3d_bind_texture(
    plugin_render_s3d_module_t module,
    ui_cache_res_t res,
    ui_runtime_render_texture_filter_t min_filter,
    ui_runtime_render_texture_filter_t mag_filter,
    ui_runtime_render_texture_wrapping_t wrap_s,
    ui_runtime_render_texture_wrapping_t wrap_t,
    uint8_t index);

void plugin_render_s3d_unbind_other_textures(
    plugin_render_s3d_module_t module, uint32_t using_texture);

void plugin_render_s3d_unbind_other_vertexes(
    plugin_render_s3d_module_t module, uint32_t using_vertexes);

void plugin_render_s3d_use_program(
    plugin_render_s3d_module_t module, plugin_render_s3d_program_t program);

#endif


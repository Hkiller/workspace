#ifndef UI_RUNTIME_S3D_TEXTURE_DATA_I_H
#define UI_RUNTIME_S3D_TEXTURE_DATA_I_H
#include "plugin_render_s3d_module_i.hpp"

struct plugin_render_s3d_texture {
    plugin_render_s3d_module_t m_module;
    flash::display3D::textures::Texture m_texture;
    int8_t m_active_at;
    ui_runtime_render_texture_filter_t m_min_filter;
    ui_runtime_render_texture_filter_t m_mag_filter;
    ui_runtime_render_texture_wrapping_t m_wrap_s;
    ui_runtime_render_texture_wrapping_t m_wrap_t;
};

int plugin_render_s3d_texture_upload(ui_cache_res_t res);

int plugin_render_s3d_module_init_res_plugin(plugin_render_s3d_module_t module);
void plugin_render_s3d_module_fini_res_plugin(plugin_render_s3d_module_t module);

#endif

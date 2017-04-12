#include <assert.h>
#include "cpe/pal/pal_stdlib.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/math_ex.h"
#include "render/utils/ui_vector_2.h"
#include "render/utils/ui_rect.h"
#include "render/cache/ui_cache_res.h"
#include "render/cache/ui_cache_res_plugin.h"
#include "render/cache/ui_cache_pixel_format.h"
#include "render/cache/ui_cache_pixel_buf.h"
#include "render/cache/ui_cache_texture.h"
#include "plugin_render_s3d_texture_i.hpp"
#include "plugin_render_s3d_utils.hpp"
#include "plugin_render_s3d_cache_i.hpp"

static int plugin_render_s3d_texture_convert_data(
    plugin_render_s3d_module_t module, ui_cache_res_t res, AS3::ui::String & o_format,
    void const * & data, size_t & data_size, uint32_t width, uint32_t height);

int plugin_render_s3d_texture_create(ui_cache_res_t res, void const * data, size_t data_size, uint32_t width, uint32_t height) {
    plugin_render_s3d_texture_t s3d_texture = (plugin_render_s3d_texture_t)ui_cache_res_plugin_data(res);
    plugin_render_s3d_module_t module = s3d_texture->m_module;

    try {
        assert(ui_cache_res_type(res) == ui_cache_res_type_texture);
    
        s3d_texture->m_min_filter = (ui_runtime_render_texture_filter_t)-1;
        s3d_texture->m_mag_filter = (ui_runtime_render_texture_filter_t)-1;
        s3d_texture->m_wrap_s = (ui_runtime_render_texture_wrapping_t)-1;
        s3d_texture->m_wrap_t = (ui_runtime_render_texture_wrapping_t)-1;
        s3d_texture->m_active_at = -1;
        
        AS3::ui::String format;

        if (plugin_render_s3d_texture_convert_data(module, res, format, data, data_size, width, height) != 0) return -1;
        
        s3d_texture->m_texture = module->m_ctx3d->createTexture(width, height, flash::display3D::Context3DTextureFormat::BGRA, false);

        if (!cpe_math_32_is_pow2(width) || !cpe_math_32_is_pow2(height)) {
            CPE_ERROR(
                module->m_em, "%s: texture %s:load to buf (%d-%d) fail (size error)!",
                plugin_render_s3d_module_name(module), ui_cache_res_path(res), width, height);
            return -1;
        }

        s3d_texture->m_texture->uploadFromByteArray(internal::get_ram(), (unsigned)data, 0, (void*)data);
        CPE_INFO(
            module->m_em, "%s: texture %s: load success, size=(%.2fM)",
            plugin_render_s3d_module_name(module), ui_cache_res_path(res), ((float)data_size) / 1024.0f / 1024.0f);

        return 0;
    }
    S3D_REPORT_EXCEPTION_1("texture %s: create: ", ui_cache_res_path(res), return -1);
}

static void plugin_render_s3d_texture_delete(plugin_render_s3d_module_t module, ui_cache_res_t res, uint8_t is_external_unloaded) {
    plugin_render_s3d_texture_t s3d_texture = (plugin_render_s3d_texture_t)ui_cache_res_plugin_data(res);
    
    assert(ui_cache_res_type(res) == ui_cache_res_type_texture);

    if (s3d_texture->m_active_at >= 0) {
        assert((uint32_t)s3d_texture->m_active_at < CPE_ARRAY_SIZE(module->m_cache->m_textures));
        assert(module->m_cache->m_textures[s3d_texture->m_active_at] == s3d_texture);
        module->m_cache->m_textures[s3d_texture->m_active_at] = NULL;
    }

    if (!is_external_unloaded) {
        CPE_ERROR(module->m_em, "plugin_render_s3d_texture_delete: texture %s dispose", ui_cache_res_path(res));
        try {
            s3d_texture->m_texture->dispose();
        }
        S3D_REPORT_EXCEPTION_1("texture %s: dispose: ", ui_cache_res_path(res), );
    }
    
    s3d_texture->m_texture.~Texture();
}

static int plugin_render_s3d_on_res_loaded(void * ctx, ui_cache_res_t res) {
    plugin_render_s3d_module_t module = (plugin_render_s3d_module_t)ctx;
    ui_cache_pixel_buf_t data_buff = ui_cache_texture_data_buf(res);
    plugin_render_s3d_texture_t s3d_texture = (plugin_render_s3d_texture_t)ui_cache_res_plugin_data(res);
    
    assert(ui_cache_res_type(res) == ui_cache_res_type_texture);

    s3d_texture->m_module = module;
    new (&s3d_texture->m_texture) flash::display3D::textures::Texture();

    if (data_buff) {
        ui_cache_pixel_level_info_t level_info = ui_cache_pixel_buf_level_info_at(data_buff, 0);
    
        if (plugin_render_s3d_texture_create(
                res,
                ui_cache_pixel_buf_level_buf(data_buff, 0),
                ui_cache_pixel_buf_level_buf_size(level_info),
                ui_cache_pixel_buf_level_width(level_info),
                ui_cache_pixel_buf_level_height(level_info))
            != 0)
        {
            CPE_ERROR(
                module->m_em, "%s: on res load: texture %s: s3d create fail",
                plugin_render_s3d_module_name(module), ui_cache_res_path(res));
            return -1;
        }
        
        if (!ui_cache_texture_keep_data_buf(res) && !ui_cache_texture_need_part_update(res)) {
            ui_cache_texture_attach_data_buf(res, NULL);
        }
    }
    else {
        if (plugin_render_s3d_texture_create(res, NULL, 0, ui_cache_texture_width(res), ui_cache_texture_height(res))) {
            CPE_ERROR(
                module->m_em, "%s: on res load: texture %s: s3d create(empty) fail",
                plugin_render_s3d_module_name(module), ui_cache_res_path(res));
            return -1;
        }
    }
    
    return 0;
}

static void plugin_render_s3d_on_res_unload(void * ctx, ui_cache_res_t res, uint8_t is_external_unloaded) {
    plugin_render_s3d_module_t module = (plugin_render_s3d_module_t)ctx;
    plugin_render_s3d_texture_delete(module, res, is_external_unloaded);
}

int plugin_render_s3d_texture_upload(ui_cache_res_t res) {
    ui_cache_pixel_buf_t data_buff = ui_cache_texture_data_buf(res);
    plugin_render_s3d_texture_t s3d_texture = (plugin_render_s3d_texture_t)ui_cache_res_plugin_data(res);
    plugin_render_s3d_module_t module = s3d_texture->m_module;

    if (data_buff == NULL) {
        CPE_ERROR(
            module->m_em, "%s: texture %s: upload: no data buf!",
            plugin_render_s3d_module_name(module), ui_cache_res_path(res));
        return -1;
    }
    
    ui_cache_pixel_level_info_t level_info = ui_cache_pixel_buf_level_info_at(data_buff, 0);
    if (level_info == NULL) {
        CPE_ERROR(
            module->m_em, "%s: texture %s: upload: data buf no level info!",
            plugin_render_s3d_module_name(module), ui_cache_res_path(res));
        return -1;
    }

    AS3::ui::String format;
    void const * data = ui_cache_pixel_buf_level_buf(data_buff, 0);
    size_t data_size = ui_cache_pixel_buf_level_buf_size(level_info);
        
    if (plugin_render_s3d_texture_convert_data(
        module, res, format, data, data_size,
        ui_cache_pixel_buf_level_width(level_info),
        ui_cache_pixel_buf_level_height(level_info)) != 0) return -1;

    s3d_texture->m_texture->uploadFromByteArray(internal::get_ram(), (unsigned)data, 0, (void*)data);
    CPE_INFO(module->m_em, "%s: texture %s: re-upload success", plugin_render_s3d_module_name(module), ui_cache_res_path(res));
    
    ui_cache_texture_set_is_dirty(res, 0);
    return 0;
}

static int plugin_render_s3d_texture_convert_data(
    plugin_render_s3d_module_t module, ui_cache_res_t res, AS3::ui::String & o_format,
    void const * & data, size_t & data_size, uint32_t width, uint32_t height)
{
    switch(ui_cache_texture_format(res)) {
    case ui_cache_pf_r8g8b8a8: {
        uint32_t pt_count = width * height;
        size_t new_data_size = sizeof(uint32_t) * pt_count;
        
        mem_buffer_clear_data(gd_app_tmp_buffer(module->m_app));
        void * new_data = mem_buffer_alloc(gd_app_tmp_buffer(module->m_app), new_data_size);
        if (new_data == NULL) {
            CPE_ERROR(
                module->m_em, "%s: texture %s: convert rgba ==> bgra tmp buffer fail!",
                plugin_render_s3d_module_name(module), ui_cache_res_path(res));
            return -1;
        }

        for(uint32_t i = 0; i < pt_count; ++i) {
            unsigned char * p = (unsigned char *)(((uint32_t *)new_data) + i);
            const unsigned char * i_p = (const unsigned char *)(((uint32_t*)data) + i);
            p[0] = i_p[2];
            p[1] = i_p[1];
            p[2] = i_p[0];
            p[3] = i_p[3];
        }

        o_format = flash::display3D::Context3DTextureFormat::BGRA;
        data = new_data;
        data_size = new_data_size;
        return 0;
    }
    case ui_cache_pf_a8: {
        uint32_t pt_count = width * height;
        size_t new_data_size = sizeof(uint32_t) * pt_count;
            
        mem_buffer_clear_data(gd_app_tmp_buffer(module->m_app));
        void * new_data = mem_buffer_alloc(gd_app_tmp_buffer(module->m_app), new_data_size);
        if (new_data == NULL) {
            CPE_ERROR(
                module->m_em, "%s: texture %s: convert a8 ==> rgba tmp buffer fail!",
                plugin_render_s3d_module_name(module), ui_cache_res_path(res));
            return NULL;
        }
        for(uint32_t i = 0; i < pt_count; ++i) {
            char * p = (char *)(((uint32_t*)new_data) + i);
            p[0] = p[1] = p[2] = 0;
            p[3] = ((const char*)data)[i];
        }

        o_format = flash::display3D::Context3DTextureFormat::BGRA;
        data = new_data;
        data_size = new_data_size;
        return 0;
    }
    default:
        CPE_ERROR(
            module->m_em, "%s: texture %s: not support format %d",
            plugin_render_s3d_module_name(module), ui_cache_res_path(res),
            ui_cache_texture_format(res));
        return -1;
    }
}

int plugin_render_s3d_module_init_res_plugin(plugin_render_s3d_module_t module) {
    struct ui_cache_res_plugin_addition_fun addition_funcs;
    ui_cache_res_plugin_t plugin;

    addition_funcs.m_texture.m_upload_part = NULL;
    
    plugin = 
        ui_cache_res_plugin_create(
            module->m_cache_mgr, ui_cache_res_type_texture,
            "render", sizeof(struct plugin_render_s3d_texture), module,
            plugin_render_s3d_on_res_loaded,
            plugin_render_s3d_on_res_unload,
            &addition_funcs);
    if (plugin == NULL) {
        CPE_ERROR(module->m_em, "plugin_render_s3d_module_init_res_plugin: create plugin fail!");
        return -1;
    }
    
    return 0;
}

void plugin_render_s3d_module_fini_res_plugin(plugin_render_s3d_module_t module) {
    ui_cache_res_plugin_t plugin = ui_cache_res_plugin_find_by_ctx(module->m_cache_mgr, module);
    if (plugin == NULL) {
        CPE_ERROR(module->m_em, "plugin_render_s3d_module_fini_render_res_plugin: plugin not exist!");
    }
    else {
        ui_cache_res_plugin_free(plugin);
    }
}

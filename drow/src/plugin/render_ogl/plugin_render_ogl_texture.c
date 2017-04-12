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
#include "plugin_render_ogl_texture_i.h"
#include "plugin_render_ogl_cache_i.h"
#include "plugin_render_ogl_utils.h"

static int plugin_render_ogl_texture_upload_part(void * ctx, ui_cache_res_t res, ui_rect_t rect, void const * data);

uint32_t plugin_render_ogl_texture_gl_texture_id(ui_cache_res_t res) {
    plugin_render_ogl_texture_t data = ui_cache_res_plugin_data(res);
    assert(ui_cache_res_type(res) == ui_cache_res_type_texture);
    return data->m_texture_id;
}

int plugin_render_ogl_texture_gl_create(ui_cache_res_t res, void const * data, size_t data_size, uint32_t width, uint32_t height) {
    plugin_render_ogl_texture_t ogl_texture = ui_cache_res_plugin_data(res);
    plugin_render_ogl_module_t module = ogl_texture->m_module;
    GLint old_aligment;
    GLenum err;

    assert(glGetError() == 0);
    assert(ui_cache_res_type(res) == ui_cache_res_type_texture);

    assert(ogl_texture->m_texture_id == 0);
    
    ogl_texture->m_min_filter = -1;
    ogl_texture->m_mag_filter = -1;
    ogl_texture->m_wrap_s = -1;
    ogl_texture->m_wrap_t = -1;
    
    glGenTextures(1, &ogl_texture->m_texture_id);
    if ((err = glGetError())) {
        CPE_ERROR(
            module->m_em, "%s: texture %s: gl create fail!",
            plugin_render_ogl_module_name(module), ui_cache_res_path(res));
        return -1;
    }
    
    plugin_render_ogl_bind_texture(
        module, res, 
        ui_runtime_render_filter_linear,
        ui_runtime_render_filter_linear,
        ui_runtime_render_texture_clamp_to_edge,
        ui_runtime_render_texture_clamp_to_edge,
        module->m_capacity_install_texture_index);
    
    glGetIntegerv(GL_UNPACK_ALIGNMENT, &old_aligment);
    if (old_aligment != 1) {
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    }

    if (ui_cache_pixel_format_compressed(ui_cache_texture_format(res))) {
        glCompressedTexImage2D(
            GL_TEXTURE_2D,
            0,
            plugin_render_ogl_format_api_format(ui_cache_texture_format(res)),
            width,
            height,
            0,
            data_size,
            data);

        if ((err = glGetError())) {
            CPE_ERROR(
                module->m_em, "%s: texture %s(%d): gl load(compressed) to buf fail(%s)!",
                plugin_render_ogl_module_name(module), ui_cache_res_path(res), ogl_texture->m_texture_id,
                gl_utils_strerror(err));
            return -1;
        }
    }
    else {
        if (!cpe_math_32_is_pow2(width) || !cpe_math_32_is_pow2(height)) {
            CPE_ERROR(
                module->m_em, "%s: texture %s(%d): gl load to buf (%d-%d) fail (size error)!",
                plugin_render_ogl_module_name(module), ui_cache_res_path(res), ogl_texture->m_texture_id,
                width, height);
            return -1;
        }
            
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            plugin_render_ogl_format_api_format(ui_cache_texture_format(res)),
            width,
            height,
            0,
            plugin_render_ogl_format_api_format(ui_cache_texture_format(res)),
            plugin_render_ogl_format_api_type(ui_cache_texture_format(res)),
            data);

        if ((err = glGetError())) {
            if (data) {
                CPE_ERROR(
                    module->m_em, "%s: texture %s(%d): gl load(part) to buf (%d-%d) fail(%s)!",
                    plugin_render_ogl_module_name(module), ui_cache_res_path(res), ogl_texture->m_texture_id,
                    width, height,
                    gl_utils_strerror(err));
                return -1;
            }
        }
    }

    return 0;
}

static void plugin_render_ogl_texture_gl_delete(plugin_render_ogl_module_t module, ui_cache_res_t res) {
    plugin_render_ogl_texture_t ogl_texture = ui_cache_res_plugin_data(res);
    
    assert(glGetError() == 0);
    assert(ui_cache_res_type(res) == ui_cache_res_type_texture);

    if (ogl_texture->m_texture_id == 0) {
        CPE_ERROR(
            module->m_em, "%s: texture %s: gl not create!",
            plugin_render_ogl_module_name(module), ui_cache_res_path(res));
    }
    else {
        glDeleteTextures(1, &ogl_texture->m_texture_id);
        ogl_texture->m_texture_id = 0;
        assert(glGetError() == 0);
    }
}

static int plugin_render_ogl_on_res_loaded(void * ctx, ui_cache_res_t res) {
    plugin_render_ogl_module_t module = ctx;
    ui_cache_pixel_buf_t data_buff = ui_cache_texture_data_buf(res);
    int8_t i, level_count;
    plugin_render_ogl_texture_t ogl_texture = ui_cache_res_plugin_data(res);
    
    assert(ui_cache_res_type(res) == ui_cache_res_type_texture);

    ogl_texture->m_module = module;
    ogl_texture->m_texture_id = 0;

    if (data_buff) {
        ui_cache_pixel_level_info_t level_info = ui_cache_pixel_buf_level_info_at(data_buff, 0);
    
        if (plugin_render_ogl_texture_gl_create(
                res,
                ui_cache_pixel_buf_level_buf(data_buff, 0),
                ui_cache_pixel_buf_level_buf_size(level_info),
                ui_cache_pixel_buf_level_width(level_info),
                ui_cache_pixel_buf_level_height(level_info))
            != 0)
        {
            CPE_ERROR(
                module->m_em, "%s: on res load: texture %s: gl create fail",
                plugin_render_ogl_module_name(module), ui_cache_res_path(res));
            return -1;
        }

        for(i = 1, level_count = ui_cache_pixel_buf_level_count(data_buff); i < level_count; ++i) {
            ui_cache_pixel_level_info_t level_info = ui_cache_pixel_buf_level_info_at(data_buff, i);
            ui_rect rect = UI_RECT_INITLIZER(0, 0, ui_cache_pixel_buf_level_width(level_info), ui_cache_pixel_buf_level_height(level_info));

            if (plugin_render_ogl_texture_upload_part(module, res, &rect, ui_cache_pixel_buf_level_buf(data_buff, i)) != 0) {
                CPE_ERROR(
                    module->m_em, "%s: on res load: texture %s: gl create fail",
                    plugin_render_ogl_module_name(module), ui_cache_res_path(res));
            
                CPE_ERROR(module->m_em, "texture %s: gl create level %d fail", ui_cache_res_path(res), i);
                plugin_render_ogl_texture_gl_delete(module, res);
                return -1;
            }
        }
    
        if (!ui_cache_texture_keep_data_buf(res)) {
            ui_cache_texture_attach_data_buf(res, NULL);
        }
    }
    else {
        if (plugin_render_ogl_texture_gl_create(res, NULL, 0, ui_cache_texture_width(res), ui_cache_texture_height(res))) {
            CPE_ERROR(
                module->m_em, "%s: on res load: texture %s: gl create(empty) fail",
                plugin_render_ogl_module_name(module), ui_cache_res_path(res));
            return -1;
        }
    }
    
    return 0;
}

static void plugin_render_ogl_on_res_unload(void * ctx, ui_cache_res_t res, uint8_t is_external_unloaded) {
    plugin_render_ogl_module_t module = ctx;
    
    assert(ui_cache_res_type(res) == ui_cache_res_type_texture);

    if (is_external_unloaded) {
        plugin_render_ogl_texture_t ogl_texture = ui_cache_res_plugin_data(res);
        ogl_texture->m_texture_id = 0;
    }
    else {
        plugin_render_ogl_texture_gl_delete(module, res);
    }
}

int plugin_render_ogl_texture_upload(ui_cache_res_t res) {
    ui_cache_pixel_buf_t data_buff = ui_cache_texture_data_buf(res);
    plugin_render_ogl_texture_t ogl_texture = (plugin_render_ogl_texture_t)ui_cache_res_plugin_data(res);
    plugin_render_ogl_module_t module = ogl_texture->m_module;
    ui_rect rect = UI_RECT_INITLIZER(0, 0, ui_cache_texture_width(res), ui_cache_texture_height(res));

    if (data_buff == NULL) {
        CPE_ERROR(
            module->m_em, "%s: texture %s: upload: no data buf!",
            plugin_render_ogl_module_name(module), ui_cache_res_path(res));
        return -1;
    }

    if (plugin_render_ogl_texture_upload_part(module, res, &rect, ui_cache_pixel_buf_level_buf(data_buff, 0)) != 0) {
        CPE_ERROR(
            module->m_em, "%s: on res load: texture %s: gl create fail",
            plugin_render_ogl_module_name(module), ui_cache_res_path(res));
        return -1;
    }
    
    CPE_INFO(module->m_em, "%s: texture %s: re-upload success", plugin_render_ogl_module_name(module), ui_cache_res_path(res));
    
    ui_cache_texture_set_is_dirty(res, 0);
    return 0;
}

static int plugin_render_ogl_texture_upload_part(void * ctx, ui_cache_res_t res, ui_rect_t rect, void const * data) {
    plugin_render_ogl_module_t module = ctx;
	GLint old_aligment;
    plugin_render_ogl_texture_t ogl_texture = ui_cache_res_plugin_data(res);
    uint32_t texture_width = ui_cache_texture_width(res);
    uint32_t texture_height = ui_cache_texture_height(res);
    GLenum err;

    assert(glGetError() == 0);
    assert(ui_cache_res_type(res) == ui_cache_res_type_texture);

    if (rect->lt.x < 0.0f || rect->lt.y < 0.0f || rect->rb.x > texture_width || rect->rb.y > texture_height) {
        CPE_ERROR(
            module->m_em, "%s: texture %s: rect (%f,%f)-(%f,%f) overflow, texture=(0,0)-(%d,%d)!",
            plugin_render_ogl_module_name(module), ui_cache_res_path(res),
            rect->lt.x, rect->lt.y, rect->rb.x, rect->rb.y, texture_width, texture_height);
        return -1;
    }

    if (ogl_texture->m_texture_id == 0) {
        CPE_ERROR(
            module->m_em, "%s: texture %s: gl not create!",
            plugin_render_ogl_module_name(module), ui_cache_res_path(res));
        return -1;
    }

    glGetIntegerv(GL_UNPACK_ALIGNMENT, &old_aligment);
    if (old_aligment != 1) {
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        assert(glGetError() == 0);
    }

    plugin_render_ogl_bind_texture_for_manip(module, res, module->m_capacity_install_texture_index);
    if ((err = glGetError())) {
        CPE_ERROR(
            module->m_em, "%s: texture %s: fail(%s)!",
            plugin_render_ogl_module_name(module), ui_cache_res_path(res),
            gl_utils_strerror(err));
        assert(0);
        return -1;
    }
    
    glTexSubImage2D(
        GL_TEXTURE_2D,
        0,
        (uint32_t)rect->lt.x,
        (uint32_t)rect->lt.y,
        (uint32_t)ui_rect_width(rect),
        (uint32_t)ui_rect_height(rect),
        plugin_render_ogl_format_api_format(ui_cache_texture_format(res)),
        plugin_render_ogl_format_api_type(ui_cache_texture_format(res)),
        data);
    if ((err = glGetError())) {
        CPE_ERROR(
            module->m_em, "%s: texture %s(%d): gl load to buf (%d,%d)-(%d,%d) data=%p fail(%s)!",
            plugin_render_ogl_module_name(module), ui_cache_res_path(res), ogl_texture->m_texture_id,
            (int)rect->lt.x, (int)rect->lt.y,
            (int)ui_rect_width(rect), (int)ui_rect_height(rect), data,
            gl_utils_strerror(err));
        //assert(0);
        return -1;
    }
    
	if (old_aligment != 1) {
		glPixelStorei(GL_UNPACK_ALIGNMENT, old_aligment);
        assert(glGetError() == 0);
    }

    return 0;
}

int plugin_render_ogl_module_init_res_plugin(plugin_render_ogl_module_t module) {
    struct ui_cache_res_plugin_addition_fun addition_funcs;
    ui_cache_res_plugin_t plugin;

    addition_funcs.m_texture.m_upload_part = plugin_render_ogl_texture_upload_part;
    
    plugin = 
        ui_cache_res_plugin_create(
            module->m_cache_mgr, ui_cache_res_type_texture,
            "render", sizeof(struct plugin_render_ogl_texture), module,
            plugin_render_ogl_on_res_loaded,
            plugin_render_ogl_on_res_unload,
            &addition_funcs);
    if (plugin == NULL) {
        CPE_ERROR(module->m_em, "plugin_render_ogl_module_init_res_plugin: create plugin fail!");
        return -1;
    }
    
    return 0;
}

void plugin_render_ogl_module_fini_res_plugin(plugin_render_ogl_module_t module) {
    ui_cache_res_plugin_t plugin = ui_cache_res_plugin_find_by_ctx(module->m_cache_mgr, module);
    if (plugin == NULL) {
        CPE_ERROR(module->m_em, "plugin_render_ogl_module_fini_render_res_plugin: plugin not exist!");
    }
    else {
        ui_cache_res_plugin_free(plugin);
    }
}



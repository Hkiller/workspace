#include <assert.h>
#include "cpe/pal/pal_stdio.h"
#include "cpe/utils/math_ex.h"
#include "render/utils/ui_rect.h"
#include "render/cache/ui_cache_texture.h"
#include "render/cache/ui_cache_pixel_buf_manip.h"
#include "ui_cache_res_i.h"
#include "ui_cache_pixel_buf_i.h"
#include "ui_cache_pixel_format_i.h"
#include "ui_cache_pixel_convert_i.h"
#include "ui_cache_res_plugin_i.h"

int ui_cache_texture_set_summary(
    ui_cache_res_t res, uint32_t width, uint32_t height, ui_cache_pixel_format_t format)
{
    ui_cache_manager_t mgr = res->m_mgr;

    assert(res->m_res_type == ui_cache_res_type_texture);

    if (res->m_load_state == ui_cache_res_loaded) {
        CPE_ERROR(
            mgr->m_em, "%s: res %s already bind!",
            ui_cache_manager_name(mgr), res->m_path);
        return -1;
    }

    res->m_texture.m_format = format;
    res->m_texture.m_height = height;
    res->m_texture.m_width = width;

    return 0;
}

ui_cache_pixel_format_t ui_cache_texture_format(ui_cache_res_t res) {
    assert(res->m_res_type == ui_cache_res_type_texture);
    return res->m_texture.m_format;
}

uint32_t ui_cache_texture_width(ui_cache_res_t res) {
    assert(res->m_res_type == ui_cache_res_type_texture);
    return res->m_texture.m_width;
}

uint32_t ui_cache_texture_height(ui_cache_res_t res) {
    assert(res->m_res_type == ui_cache_res_type_texture);
    return res->m_texture.m_height;
}

uint32_t ui_cache_texture_memory_size(ui_cache_res_t res) {
    assert(res->m_res_type == ui_cache_res_type_texture);
    return res->m_texture.m_width * res->m_texture.m_height * ui_cache_pixel_format_to_stride(res->m_texture.m_format);
}

int ui_cache_texture_load_from_buf(ui_cache_res_t res, ui_cache_pixel_buf_t buf) {
    ui_cache_manager_t mgr = res->m_mgr;
    ui_cache_pixel_level_info_t level_info;

    assert(res->m_res_type == ui_cache_res_type_texture);

    if (ui_cache_pixel_buf_level_count(buf) <= 0) {
        CPE_ERROR(mgr->m_em, "texture %s: buf no level", res->m_path);
        ui_cache_pixel_buf_free(res->m_texture.m_data_buff);
        res->m_texture.m_data_buff = NULL;
        return -1;
    }

    level_info = ui_cache_pixel_buf_level_info_at(buf, 0);
    assert(level_info);

    res->m_texture.m_format = ui_cache_pixel_buf_format(buf);
    if (res->m_texture.m_height == 0) {
        res->m_texture.m_height = ui_cache_pixel_buf_level_height(level_info);
    }
    if (res->m_texture.m_width == 0) {
        res->m_texture.m_width = ui_cache_pixel_buf_level_width(level_info);
    }
    
    return 0;
}

int ui_cache_texture_do_load(ui_cache_manager_t mgr, ui_cache_res_t res, const char * root) {
    char path_buf[256];
    const char * path;

    assert(res->m_res_type == ui_cache_res_type_texture);
    
    if (res->m_texture.m_data_buff == NULL) {
        res->m_texture.m_data_buff = ui_cache_pixel_buf_create(mgr);
        if (res->m_texture.m_data_buff == NULL) {
            CPE_ERROR(mgr->m_em, "texture %s: create pixel buf fail", res->m_path);
            res->m_load_result = ui_cache_res_internal_error;
            return -1;
        }
    }
    
    path = ui_cache_res_path(res);
    if (path[0] != '/' && root && root[0] != 0) {
        snprintf(path_buf, sizeof(path_buf), "%s/%s", root, path);
        path = path_buf;
    }

    if (ui_cache_pixel_buf_load_from_file(res->m_texture.m_data_buff, path, mgr->m_em, mgr->m_alloc) != 0) {
        CPE_ERROR(mgr->m_em, "texture %s: load from file %s fail", res->m_path, path);
        res->m_load_result = ui_cache_res_internal_error;
        return -1;
    }

    if (res->m_texture.m_scale) {
        ui_cache_pixel_buf_t new_buf = ui_cache_pixel_buf_resample(res->m_texture.m_data_buff, res->m_texture.m_scale);
        if (new_buf == NULL) {
            CPE_ERROR(mgr->m_em, "texture %s: load from file %s fail", res->m_path, path);
            res->m_load_result = ui_cache_res_internal_error;
            return -1;
        }

        ui_cache_pixel_buf_free(res->m_texture.m_data_buff);
        res->m_texture.m_data_buff = new_buf;
    }
    
    return 0;
}

uint8_t ui_cache_texture_keep_data_buf(ui_cache_res_t texture) {
    assert(texture->m_res_type == ui_cache_res_type_texture);
    return texture->m_texture.m_keep_data_buf;
}

uint8_t ui_cache_texture_is_dirty(ui_cache_res_t texture) {
    assert(texture->m_res_type == ui_cache_res_type_texture);
    return texture->m_texture.m_is_dirty;
}

void ui_cache_texture_set_is_dirty(ui_cache_res_t texture, uint8_t is_dirty) {
    assert(texture->m_res_type == ui_cache_res_type_texture);
    texture->m_texture.m_is_dirty = is_dirty;
}

uint8_t ui_cache_texture_need_part_update(ui_cache_res_t texture) {
    assert(texture->m_res_type == ui_cache_res_type_texture);
    return texture->m_texture.m_need_part_update;
}

void ui_cache_texture_set_need_part_update(ui_cache_res_t texture, uint8_t need_part_update) {
    assert(texture->m_res_type == ui_cache_res_type_texture);
    texture->m_texture.m_need_part_update = need_part_update;
}

void ui_cache_texture_set_keep_data_buf(ui_cache_res_t texture, uint8_t keep_data_buf) {
    assert(texture->m_res_type == ui_cache_res_type_texture);
    texture->m_texture.m_keep_data_buf = keep_data_buf;
}

ui_cache_pixel_buf_t ui_cache_texture_data_buf(ui_cache_res_t texture) {
    assert(texture->m_res_type == ui_cache_res_type_texture);
    return texture->m_texture.m_data_buff;
}

int ui_cache_texture_attach_data_buf(ui_cache_res_t texture, ui_cache_pixel_buf_t data_buf) {
    ui_cache_pixel_level_info_t level_info;

    if (data_buf == NULL) {
        if (texture->m_texture.m_data_buff) {
            ui_cache_pixel_buf_free(texture->m_texture.m_data_buff);
            texture->m_texture.m_data_buff = NULL;
        }
        return 0;
    }
    
    if (data_buf->m_level_count <= 0) {
        CPE_ERROR(
            texture->m_mgr->m_em, "texture %s: attach data buff: data buf no layer",
            texture->m_path);
        return -1;
    }
    
    level_info =  &data_buf->m_level_infos[0];
    
    assert(texture->m_res_type == ui_cache_res_type_texture);
    switch(texture->m_load_state) {
    case ui_cache_res_loaded:
        break;
    case ui_cache_res_not_load:
    case ui_cache_res_load_fail:
        texture->m_load_state = ui_cache_res_not_load;
        break;
    case ui_cache_res_wait_load:
    case ui_cache_res_data_loaded:
    case ui_cache_res_loading:
    case ui_cache_res_cancel_loading:
        CPE_ERROR(
            texture->m_mgr->m_em, "texture %s: attach data buff: texture is in async loading",
            texture->m_path);
        return -1;
    default:
        CPE_ERROR(
            texture->m_mgr->m_em, "texture %s: attach data buff: texture state unknown!",
            texture->m_path);
        return -1;
    }
    
    if (texture->m_texture.m_data_buff) {
        ui_cache_pixel_buf_free(texture->m_texture.m_data_buff);
    }
    texture->m_texture.m_data_buff = data_buf;
    texture->m_texture.m_format = data_buf->m_format;
    if (texture->m_texture.m_height == 0) texture->m_texture.m_height = level_info->m_height;
    if (texture->m_texture.m_width == 0) texture->m_texture.m_width = level_info->m_width;

    return 0;
}

ui_cache_res_t
ui_cache_texture_create_with_buff(
    ui_cache_manager_t cache_mgr, ui_cache_pixel_format_t format,
    uint32_t width, uint32_t height, void * data, size_t data_size)
{
    ui_cache_res_t texture;
    ui_cache_pixel_buf_t buf;

    buf = ui_cache_pixel_buf_create(cache_mgr);
    if (buf == NULL) {
        CPE_ERROR(cache_mgr->m_em, "ui_cache_texture_create_with_buff: create buf fail!");
        return NULL;
    }

    if (ui_cache_pixel_buf_pixel_buf_create(buf, width, height, format, 1) != 0) {
        CPE_ERROR(cache_mgr->m_em, "ui_cache_texture_create_with_buff: create pixel buf fail!");
        ui_cache_pixel_buf_free(buf);
        return NULL;
    }

    if (data) {
        if (data_size != ui_cache_pixel_buf_total_buf_size(buf)) {
            CPE_ERROR(
                cache_mgr->m_em, "ui_cache_texture_create_with_buff: data size mismatch, expect %d, bug %d!",
                ui_cache_pixel_buf_total_buf_size(buf), (int)data_size);
            ui_cache_pixel_buf_free(buf);
            return NULL;
        }

        memcpy(ui_cache_pixel_buf_pixel_buf(buf), data, data_size);
    }
    
    texture = ui_cache_res_create(cache_mgr, ui_cache_res_type_texture);
    if (texture == NULL) {
        CPE_ERROR(cache_mgr->m_em, "ui_cache_texture_create_with_buff: create texture fail!");
        ui_cache_pixel_buf_free(buf);
        return NULL;
    }

    ui_cache_texture_set_summary(texture, width, height, format);

    if (ui_cache_texture_attach_data_buf(texture, buf) != 0) {
        CPE_ERROR(cache_mgr->m_em, "ui_cache_texture_create_with_buff: set buf fail!");
        ui_cache_pixel_buf_free(buf);
        ui_cache_res_free(texture);
        return NULL;
    }

    ui_cache_texture_set_keep_data_buf(texture, 1);
    
    return texture;
}

int ui_cache_texture_upload(ui_cache_res_t res, ui_rect_t rect, ui_cache_pixel_format_t format, void const * data, size_t data_size) {
    ui_cache_res_plugin_t plugin;
    ui_cache_manager_t mgr = res->m_mgr;
    ui_cache_pixel_buf_t pix_buf;
    struct ui_cache_pixel_buf_rect to_rect;
    
    assert(res->m_res_type == ui_cache_res_type_texture);    

    if (!res->m_texture.m_need_part_update) {
        CPE_ERROR(mgr->m_em, "ui_cache_texture_upload: %s not support upload part!", res->m_path);
        return -1;
    }
    
    plugin = ui_cache_res_plugin_find_by_type(res->m_mgr, ui_cache_res_type_texture);
    if (plugin == NULL) {
        CPE_ERROR(mgr->m_em, "ui_cache_texture_upload: no plugin!");
        return -1;
    }

    if (plugin->m_addition_funcs.m_texture.m_upload_part) {
        if (res->m_texture.m_format == format) {
            return plugin->m_addition_funcs.m_texture.m_upload_part(plugin->m_ctx, res, rect, data);
        }
        else {
            ui_cache_pixel_format_convert_fun_t convert;
            uint32_t count = ui_rect_width(rect) * ui_rect_height(rect);
            void * buf;
        
            convert = ui_cache_pixel_find_convert(res->m_texture.m_format, format);
            if (convert == NULL) {
                CPE_ERROR(
                    mgr->m_em, "ui_cache_texture_upload: not support convert from %s to %s!",
                    ui_cache_pixel_format_name(format), ui_cache_pixel_format_name(res->m_texture.m_format));
                return -1;
            }

            mem_buffer_clear_data(&mgr->m_dump_buffer);
            buf = mem_buffer_alloc(&mgr->m_dump_buffer, ui_cache_pixel_format_to_stride(res->m_texture.m_format) * count);
            if (buf == NULL) {
                CPE_ERROR(
                    mgr->m_em, "ui_cache_texture_upload: convert from %s to %s: alloc buff fail, stride=%d, count=%d!",
                    ui_cache_pixel_format_name(format), ui_cache_pixel_format_name(res->m_texture.m_format),
                    ui_cache_pixel_format_to_stride(res->m_texture.m_format), count);
                return -1;
            }

            convert(buf, data, count);

            return plugin->m_addition_funcs.m_texture.m_upload_part(plugin->m_ctx, res, rect, buf);
        }
    }

    pix_buf = ui_cache_texture_data_buf(res);
    if (pix_buf == NULL) {
        pix_buf = ui_cache_pixel_buf_create(mgr);
        if (pix_buf == NULL) {
            CPE_ERROR(mgr->m_em, "ui_cache_texture_upload: %s: create pixel buf fail!", res->m_path);
            return -1;
        }

        if (ui_cache_pixel_buf_pixel_buf_create(
                pix_buf, ui_cache_texture_width(res), ui_cache_texture_height(res), ui_cache_texture_format(res), 1) != 0)
        {
            CPE_ERROR(mgr->m_em, "ui_cache_texture_upload: %s: create pixel buf data buf fail!", res->m_path);
            ui_cache_pixel_buf_free(pix_buf);
            return -1;
        }

        if (ui_cache_texture_attach_data_buf(res, pix_buf) != 0) {
            CPE_ERROR(mgr->m_em, "ui_cache_texture_upload: %s: attach pixel buf fail!", res->m_path);
            ui_cache_pixel_buf_free(pix_buf);
            return -1;
        }
    }
        
    if (res->m_texture.m_format != format) {
        ui_cache_pixel_format_convert_fun_t convert;
        uint32_t count = ui_rect_width(rect) * ui_rect_height(rect);
        void * buf;
        size_t buf_size;
        
        convert = ui_cache_pixel_find_convert(res->m_texture.m_format, format);
        if (convert == NULL) {
            CPE_ERROR(
                mgr->m_em, "ui_cache_texture_upload: not support convert from %s to %s!",
                ui_cache_pixel_format_name(format), ui_cache_pixel_format_name(res->m_texture.m_format));
            return -1;
        }

        buf_size = ui_cache_pixel_format_to_stride(res->m_texture.m_format) * count;
        mem_buffer_clear_data(&mgr->m_dump_buffer);
        buf = mem_buffer_alloc(&mgr->m_dump_buffer, buf_size);
        if (buf == NULL) {
            CPE_ERROR(
                mgr->m_em, "ui_cache_texture_upload: convert from %s to %s: alloc buff fail, stride=%d, count=%d!",
                ui_cache_pixel_format_name(format), ui_cache_pixel_format_name(res->m_texture.m_format),
                ui_cache_pixel_format_to_stride(res->m_texture.m_format), count);
            return -1;
        }

        convert(buf, data, count);

        data = buf;
        data_size = buf_size;
    }

    to_rect.level = 0;
    to_rect.boundary_lt = rect->lt.x;
    to_rect.boundary_tp = rect->lt.y;
    to_rect.boundary_bm = rect->rb.y;
    to_rect.boundary_rt = rect->rb.x;

    if (ui_cache_pixel_buf_rect_copy_from_data(pix_buf, &to_rect, data, data_size, mgr->m_em) != 0) {
        CPE_ERROR(mgr->m_em, "ui_cache_texture_upload: %s: copy data fail!", res->m_path);
        return -1;
    }
    
    res->m_texture.m_is_dirty = 1;
    return 0;
}


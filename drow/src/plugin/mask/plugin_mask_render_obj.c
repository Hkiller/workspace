#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/math_ex.h"
#include "cpe/utils/string_utils.h"
#include "cpe/utils/bitarry.h"
#include "gd/app/app_context.h"
#include "render/model/ui_data_src.h"
#include "render/utils/ui_vector_2.h"
#include "render/utils/ui_rect.h"
#include "render/utils/ui_transform.h"
#include "render/model/ui_data_src.h"
#include "render/cache/ui_cache_color.h"
#include "render/cache/ui_cache_texture.h"
#include "render/cache/ui_cache_pixel_buf.h"
#include "render/runtime/ui_runtime_module.h"
#include "render/runtime/ui_runtime_render.h"
#include "render/runtime/ui_runtime_render_obj.h"
#include "render/runtime/ui_runtime_render_obj_meta.h"
#include "render/runtime/ui_runtime_render_second_color.h"
#include "render/runtime/ui_runtime_render_cmd_utils_2d.h"
#include "plugin_mask_render_obj_i.h"
#include "plugin_mask_data_block_i.h"

plugin_mask_data_block_t plugin_mask_render_obj_data_block(plugin_mask_render_obj_t obj) {
    return obj->m_data_block;
}

void plugin_mask_render_obj_set_data_block(plugin_mask_render_obj_t obj, plugin_mask_data_block_t data_block) {
    obj->m_data_block = data_block;
}

ui_color_t plugin_mask_render_obj_color(plugin_mask_render_obj_t obj) {
    return &obj->m_color;
}

void plugin_mask_render_obj_set_color(plugin_mask_render_obj_t obj, ui_color_t color) {
    obj->m_color = *color;
}

static int plugin_mask_render_obj_do_init(void * ctx, ui_runtime_render_obj_t render_obj) {
    plugin_mask_render_obj_t obj = ui_runtime_render_obj_data(render_obj);
    obj->m_module = ctx;
    obj->m_data_block = NULL;
    obj->m_color = UI_COLOR_WHITE;
    obj->m_color.a = 0.5f;
    obj->m_res = NULL;
    return 0;
}

static void plugin_mask_render_obj_do_fini(void * ctx, ui_runtime_render_obj_t render_obj) {
    plugin_mask_render_obj_t obj = ui_runtime_render_obj_data(render_obj);
    if (obj->m_res) {
        ui_cache_res_tag_delete(obj->m_res);
        obj->m_res = NULL;
    }
}

static int plugin_mask_render_obj_build_texture(plugin_mask_module_t module, plugin_mask_render_obj_t obj) {
    ui_cache_pixel_buf_t pixel_buf;
    uint32_t i, j;
    ui_cache_pixel_format_t format;
    void * buf_data;

    switch(obj->m_data_block->m_data->m_format) {
    case plugin_mask_data_format_bit:
    case plugin_mask_data_format_1:
        format = ui_cache_pf_a8;
        break;
    case plugin_mask_data_format_2:
    case plugin_mask_data_format_4:
        format = ui_cache_pf_r8g8b8a8;
        break;
    default:
        CPE_ERROR(
            module->m_em, "%s: mask obj render: build texture: unknown data format %d!",
            plugin_mask_module_name(module), obj->m_data_block->m_data->m_format);
        return -1;
    }
    
    obj->m_res = ui_cache_texture_create_with_buff(
        ui_runtime_module_cache_mgr(module->m_runtime), 
        format,
        cpe_math_32_round_to_pow2(obj->m_data_block->m_buf_width),
        cpe_math_32_round_to_pow2(obj->m_data_block->m_buf_height),
        NULL, 0);
    if (obj->m_res == NULL) {
        CPE_ERROR(module->m_em, "%s: mask obj render: build texture: create texture fail!", plugin_mask_module_name(module));
        return -1;
    }

    pixel_buf = ui_cache_texture_data_buf(obj->m_res);
    assert(ui_cache_pixel_buf_stride(pixel_buf) == 1);
    
    buf_data = ui_cache_pixel_buf_level_buf(pixel_buf, 0);
    assert(buf_data);
    bzero(buf_data, ui_cache_pixel_buf_level_buf_size(ui_cache_pixel_buf_level_info_at(pixel_buf, 0)));

    assert(obj->m_data_block->m_buf_x >= 0);
    assert(obj->m_data_block->m_buf_y >= 0);
    assert(obj->m_data_block->m_buf_x + obj->m_data_block->m_buf_width <= obj->m_data_block->m_width);
    assert(obj->m_data_block->m_buf_y + obj->m_data_block->m_buf_height <= obj->m_data_block->m_height);

    switch(obj->m_data_block->m_data->m_format) {
    case plugin_mask_data_format_bit:
        for(i = 0; i < obj->m_data_block->m_buf_height; i++) {
            uint8_t * line_begin = ((uint8_t*)buf_data) + i * cpe_math_32_round_to_pow2(obj->m_data_block->m_buf_width);
            uint8_t const * mask_line = ((uint8_t*)obj->m_data_block->m_buf) + i * cpe_ba_bytes_from_bits_m(obj->m_data_block->m_buf_width);
            for(j = 0; j < obj->m_data_block->m_buf_width; j++) {
                line_begin[j] = cpe_ba_get(mask_line, j) ? 255 : 0;
            }
        }
        break;
    case plugin_mask_data_format_1:
        for(i = 0; i < obj->m_data_block->m_buf_height; i++) {
            uint8_t * line_begin = ((uint8_t*)buf_data) + i * cpe_math_32_round_to_pow2(obj->m_data_block->m_buf_width);
            uint8_t const * mask_line = ((uint8_t*)obj->m_data_block->m_buf) + i * obj->m_data_block->m_buf_width;
            for(j = 0; j < obj->m_data_block->m_buf_width; j++) {
                line_begin[j] = mask_line[j];
            }
        }
        break;
    case plugin_mask_data_format_2:
    case plugin_mask_data_format_4:
        break;
    };

    ui_cache_res_load_sync(obj->m_res, NULL);
    
    return 0;
}

static int plugin_mask_render_obj_do_render(
    void * ctx, ui_runtime_render_obj_t render_obj,
    ui_runtime_render_t render, ui_rect_t clip_rect,
    ui_runtime_render_second_color_t second_color, ui_transform_t transform)
{
    plugin_mask_module_t module = ctx;
    plugin_mask_render_obj_t obj = ui_runtime_render_obj_data(render_obj);
    uint32_t abgr = ui_color_make_abgr(&obj->m_color);
    struct ui_runtime_render_blend blend = { ui_runtime_render_src_alpha, ui_runtime_render_one_minus_src_alpha };
    struct ui_transform trans = UI_TRANSFORM_IDENTITY;
    ui_vector_2 uv_rb;
    ui_vector_2 pos_lt;
    
    if (obj->m_data_block == NULL) return -1;

    if (obj->m_res == NULL) {
        if (plugin_mask_render_obj_build_texture(module, obj) != 0) return -1;
    }

    blend.m_src_factor = ui_runtime_render_src_alpha;
    blend.m_dst_factor = ui_runtime_render_one_minus_src_alpha;

    uv_rb.x = (float)obj->m_data_block->m_buf_width / ui_cache_texture_width(obj->m_res);
    uv_rb.y = (float)obj->m_data_block->m_buf_height / ui_cache_texture_height(obj->m_res);

    pos_lt.x = obj->m_data_block->m_x + obj->m_data_block->m_buf_x;
    pos_lt.y = obj->m_data_block->m_y + obj->m_data_block->m_buf_y;
    ui_transform_set_pos_2(&trans, &pos_lt);
    ui_transform_adj_by_parent(&trans, transform);
    
    do {
        ui_runtime_vertex_v3f_t2f_c4ub vertexs[4] = {
            { UI_VECTOR_3_INITLIZER(0.0f                           , 0.0f                           , 0.0f), UI_VECTOR_2_INITLIZER(0.0f,    0.0f   ), abgr },
            { UI_VECTOR_3_INITLIZER(obj->m_data_block->m_buf_width , obj->m_data_block->m_buf_height, 0.0f), UI_VECTOR_2_INITLIZER(uv_rb.x, uv_rb.y), abgr },
            { UI_VECTOR_3_INITLIZER(0.0f                           , obj->m_data_block->m_buf_height, 0.0f), UI_VECTOR_2_INITLIZER(0.0f,    uv_rb.y), abgr },
            { UI_VECTOR_3_INITLIZER(obj->m_data_block->m_buf_width , 0.0f                           , 0.0f), UI_VECTOR_2_INITLIZER(uv_rb.x, 0.0f   ), abgr },
        };

        ui_runtime_render_cmd_quad_create_2d_buildin(
            render, 0, &trans, vertexs, obj->m_res,
            ui_runtime_render_filter_nearest,
            ui_runtime_render_program_buildin_add,
            &blend);
    } while(0);
    
    return 0;
}

static int plugin_mask_render_obj_do_set(void * ctx, ui_runtime_render_obj_t obj, UI_OBJECT_URL const * obj_url) {
    plugin_mask_module_t module = ctx;
    plugin_mask_render_obj_t mask_obj = ui_runtime_render_obj_data(obj);
    ui_data_src_t mask_src;
    plugin_mask_data_t mask_data;
    plugin_mask_data_block_t mask_data_block;
    UI_OBJECT_URL_DATA_MASK const * mask_url = &obj_url->data.mask;
    
    mask_src = ui_runtime_module_find_src(module->m_runtime, &mask_url->src, ui_data_src_type_mask);
    if (mask_src == NULL) {
        CPE_ERROR(module->m_em, "%s: mask obj init: find src fail!", plugin_mask_module_name(module));
        return -1;
    }

    mask_data = ui_data_src_product(mask_src);
    if (mask_data == NULL) {
        CPE_ERROR(module->m_em, "%s: mask obj init: data not loaded!", plugin_mask_module_name(module));
        return -1;
    }

    mask_data_block = plugin_mask_data_block_find(mask_data, mask_url->block);
    if (mask_data_block == NULL) {
        CPE_ERROR(
            module->m_em, "%s: mask obj init: mask data %s no block %s!",
            plugin_mask_module_name(module), ui_data_src_path_dump(gd_app_tmp_buffer(module->m_app), mask_src), mask_url->block);
        return -1;
    }
    
    plugin_mask_render_obj_set_data_block(mask_obj, mask_data_block);
    ui_runtime_render_obj_set_src(obj, mask_src);
        
    return 0;
}

static int plugin_mask_render_obj_do_setup(void * ctx, ui_runtime_render_obj_t render_obj, char * args) {
    plugin_mask_module_t module = ctx;
    plugin_mask_render_obj_t mask_obj = ui_runtime_render_obj_data(render_obj);
    ui_data_src_t src = ui_runtime_render_obj_src(render_obj);
    const char * str_value;
    int rv = 0;
    
    if ((str_value = cpe_str_read_and_remove_arg(args, "color", ',', '='))) {
        struct ui_color c;
        if (ui_cache_find_color(ui_runtime_module_cache_mgr(module->m_runtime), str_value, &c) != 0) {
            CPE_ERROR(
                module->m_em, "mask obj %s(%s): color %s not exist!",
                ui_runtime_render_obj_name(render_obj),
                src ? ui_data_src_path_dump(gd_app_tmp_buffer(module->m_app), src) : "",
                str_value);
            rv = -1;
        }

        mask_obj->m_color.r = c.r;
        mask_obj->m_color.g = c.g;
        mask_obj->m_color.b = c.b;
    }
    
    if ((str_value = cpe_str_read_and_remove_arg(args, "alpha", ',', '='))) {
        mask_obj->m_color.a = atof(str_value);
    }
    
    return rv;
}

static void plugin_mask_render_obj_bounding(void * ctx, ui_runtime_render_obj_t render_obj, ui_rect_t bounding) {
    plugin_mask_render_obj_t mask_obj = ui_runtime_render_obj_data(render_obj);
    if (mask_obj->m_data_block) {
        bounding->lt.x = mask_obj->m_data_block->m_x;
        bounding->lt.y = mask_obj->m_data_block->m_y;
        bounding->rb.x = bounding->lt.x + mask_obj->m_data_block->m_width;
        bounding->rb.y = bounding->lt.y + mask_obj->m_data_block->m_height;
    }
    else {
        *bounding = UI_RECT_ZERO;
    }
}

//ui_runtime_render_obj_free_fun_t free_fun,
int plugin_mask_render_obj_regist(plugin_mask_module_t module) {
    if (module->m_runtime) {
        ui_runtime_render_obj_meta_t obj_meta = NULL;

        obj_meta =
            ui_runtime_render_obj_meta_create(
                module->m_runtime, "mask", UI_OBJECT_TYPE_MASK,
                sizeof(struct plugin_mask_render_obj), module,
                plugin_mask_render_obj_do_init,
                plugin_mask_render_obj_do_set,
                plugin_mask_render_obj_do_setup,
                NULL,
                plugin_mask_render_obj_do_fini,
                plugin_mask_render_obj_do_render,
                NULL,
                plugin_mask_render_obj_bounding,
                NULL);
        if (obj_meta == NULL) {
            CPE_ERROR(module->m_em, "mask_render_regist: register render obj fail");
            return -1;
        }
    }

    return 0;
}

void plugin_mask_render_obj_unregist(plugin_mask_module_t module) {
    if (module->m_runtime) {
        ui_runtime_render_obj_meta_t obj_meta = NULL;

        if ((obj_meta = ui_runtime_render_obj_meta_find_by_name(module->m_runtime, "mask"))) {
            ui_runtime_render_obj_meta_free(obj_meta);
        }
    }
}


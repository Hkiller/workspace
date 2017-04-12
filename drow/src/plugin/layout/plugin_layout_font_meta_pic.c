#include <assert.h>
#include "cpe/pal/pal_stdio.h"
#include "cpe/utils/math_ex.h"
#include "render/utils/ui_vector_2.h"
#include "render/model/ui_data_mgr.h"
#include "render/model/ui_data_src.h"
#include "render/model/ui_data_src_user.h"
#include "render/cache/ui_cache_manager.h"
#include "render/cache/ui_cache_texture.h"
#include "plugin_layout_font_meta_pic_i.h"
#include "plugin_layout_font_cache_i.h"
#include "plugin_layout_font_meta_i.h"
#include "plugin_layout_font_face_i.h"
#include "plugin_layout_font_element_i.h"
#include "plugin_layout_render_i.h"

static int plugin_layout_font_meta_pic_init_meta(void * ctx, plugin_layout_font_meta_t meta) {
    return 0;
}

static void plugin_layout_font_meta_pic_fini_meta(void * ctx, plugin_layout_font_meta_t meta) {
}

static void plugin_layout_font_meta_pic_on_src_unload(void * ctx, ui_data_src_t src) {
    plugin_layout_font_face_free(plugin_layout_font_face_from_data(ctx));
}

static int plugin_layout_font_meta_pic_init_face(void * ctx, plugin_layout_font_face_t face) {
    plugin_layout_module_t module = ctx;
    plugin_layout_font_face_pic_t face_pic = plugin_layout_font_face_data(face);
    char path_buf[128];
    
	snprintf(path_buf, sizeof(path_buf), "ArtFont/artFont_%d", face->m_id.face + 1);

    face_pic->m_module_src = ui_data_src_find_by_path(module->m_data_mgr, path_buf, ui_data_src_type_module);
    if (face_pic->m_module_src == NULL) {
        CPE_ERROR(module->m_em, "plugin_layout_font_meta_pic_init_face: module %s not exist", path_buf);
        return -1;
    }

    face_pic->m_module = (ui_data_module_t)ui_data_src_product(face_pic->m_module_src);
    if (face_pic->m_module == NULL) {
        CPE_ERROR(module->m_em, "plugin_layout_font_meta_pic_init_face: module %s not loaded", path_buf);
        return -1;
    }

    ui_data_src_user_create(face_pic->m_module_src, face_pic, plugin_layout_font_meta_pic_on_src_unload);
    return 0;
}

static void plugin_layout_font_meta_pic_fini_face(void * ctx, plugin_layout_font_face_t face) {
    plugin_layout_font_face_pic_t face_pic = plugin_layout_font_face_data(face);
    if (face_pic->m_module_src) {
        ui_data_src_remove_user(face_pic->m_module_src, face_pic);
    }
}

static int plugin_layout_font_meta_pic_init_element(void * ctx, plugin_layout_font_element_t element) {
    plugin_layout_module_t module = ctx;
    plugin_layout_font_face_pic_t face_pic = plugin_layout_font_face_data(element->m_face);
    ui_data_img_block_t img_block;
    UI_IMG_BLOCK const * img_data;
    float scale;
    
    img_block = ui_data_img_block_find_by_id(face_pic->m_module, element->m_charter);
    if (img_block == NULL) {
        CPE_ERROR(
            module->m_em, "plugin_layout_font_meta_pic_init_element: module %s no block %d",
            ui_data_src_path_dump(&module->m_dump_buffer, face_pic->m_module_src), element->m_charter);
        return -1;
    }

    img_data = ui_data_img_block_data(img_block);
    assert(img_data);

    element->m_texture_rect.lt.x = (float)img_data->src_x;
    element->m_texture_rect.lt.y = (float)img_data->src_y;
    element->m_texture_rect.rb.x = (float)(img_data->src_x + img_data->src_w);
    element->m_texture_rect.rb.y = (float)(img_data->src_y + img_data->src_h);

    scale = (float)element->m_face->m_id.size / (float)img_data->src_h;
    
    element->m_render_width = (uint32_t)(scale * img_data->src_w);
    element->m_render_height = (uint32_t)(scale * img_data->src_h);
    element->m_bearing_x = 0;
    element->m_bearing_y = 0;

    return 0;
}

static void plugin_layout_font_meta_pic_fini_element(void * ctx, plugin_layout_font_element_t element) {
}

static void plugin_layout_font_meta_pic_render_element(
    void * ctx, plugin_layout_font_element_t element, plugin_layout_font_draw_t font_draw, ui_rect_t target_rect,
    ui_runtime_render_t render, ui_runtime_render_cmd_t * batch_cmd,
    ui_rect_t clip_rect, ui_runtime_render_second_color_t second_color, ui_transform_t transform)
{
    plugin_layout_font_face_pic_t face_pic = plugin_layout_font_face_data(element->m_face);
    ui_data_img_block_t img_block;
    ui_cache_res_t texture;
    
    img_block = ui_data_img_block_find_by_id(face_pic->m_module, element->m_charter);
    if (img_block == NULL) return;

    
    texture = ui_data_img_block_using_texture(img_block);
    if (texture == NULL) return;
    
    plugin_layout_render_render_element(
        target_rect, &element->m_texture_rect,
        texture, ui_runtime_render_filter_linear,
        ui_runtime_render_program_buildin_multiply, &font_draw->color,
        render, batch_cmd,
        clip_rect, second_color, transform);
}

int plugin_layout_font_meta_pic_register(plugin_layout_module_t module) {
    plugin_layout_font_meta_t meta;

    meta = plugin_layout_font_meta_create(
        module, plugin_layout_font_category_pic, "pic",
        module,
        0,
        plugin_layout_font_meta_pic_init_meta,
        plugin_layout_font_meta_pic_fini_meta,
        NULL,
        sizeof(struct plugin_layout_font_face_pic),
        plugin_layout_font_meta_pic_init_face,
        plugin_layout_font_meta_pic_fini_face,
        0,
        plugin_layout_font_meta_pic_init_element,
        plugin_layout_font_meta_pic_fini_element,
        plugin_layout_font_meta_pic_render_element,
        /*layout*/
        plugin_layout_font_meta_pic_basic_layout);

    return meta ? 0 : -1;
}

void plugin_layout_font_meta_pic_unregister(plugin_layout_module_t module) {
    plugin_layout_font_meta_t meta;

    meta = plugin_layout_font_meta_find_by_name(module, "pic");
    if (meta) {
        plugin_layout_font_meta_free(meta);
    }
}


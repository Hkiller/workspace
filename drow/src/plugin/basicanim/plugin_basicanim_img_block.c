#include <assert.h>
#include "cpe/utils/string_utils.h"
#include "render/utils/ui_rect.h"
#include "render/utils/ui_transform.h"
#include "render/model/ui_data_src.h"
#include "render/model/ui_data_module.h"
#include "render/cache/ui_cache_res.h"
#include "render/runtime/ui_runtime_module.h"
#include "render/runtime/ui_runtime_render_obj.h"
#include "render/runtime/ui_runtime_render_obj_meta.h"
#include "render/runtime/ui_runtime_render_utils.h"
#include "plugin_basicanim_img_block_i.h"
#include "plugin_basicanim_utils_i.h"

int plugin_basicanim_img_block_init(void * ctx, ui_runtime_render_obj_t render_obj) {
    struct plugin_basicanim_img_block * obj = ui_runtime_render_obj_data(render_obj);
    obj->m_img_block = NULL;
	obj->m_filter = ui_runtime_render_filter_nearest;
    return 0;
}

int plugin_basicanim_img_block_set(void * ctx, ui_runtime_render_obj_t render_obj, UI_OBJECT_URL const * obj_url) {
    plugin_basicanim_module_t module = ctx;
    struct plugin_basicanim_img_block * obj = ui_runtime_render_obj_data(render_obj);
    UI_OBJECT_URL_DATA_SRC_ID const * img_data = &obj_url->data.img_block;
    ui_data_src_t src;
    ui_data_module_t data_module;

    src = ui_runtime_module_find_src(module->m_runtime, &img_data->src, ui_data_src_type_module);
    if (src == NULL) return -1;

    data_module = ui_data_src_product(src);
    if (data_module == NULL) {
        CPE_ERROR(module->m_em, "src %s not loaded", ui_data_src_path_dump(gd_app_tmp_buffer(module->m_app), src));
        return -1;
    }

    if (img_data->id != (uint32_t)-1) {
        obj->m_img_block = ui_data_img_block_find_by_id(data_module, img_data->id);
        if (obj->m_img_block == NULL) {
            CPE_ERROR(module->m_em, "src %s no img block %d", ui_data_src_path_dump(gd_app_tmp_buffer(module->m_app), src), img_data->id);
            return -1;
        }
    }
    else if (img_data->name[0]) {
        obj->m_img_block = ui_data_img_block_find_by_name(data_module, img_data->name);
        if (obj->m_img_block == NULL) {
            CPE_ERROR(module->m_em, "src %s no img %s", ui_data_src_path_dump(gd_app_tmp_buffer(module->m_app), src), img_data->name);
            return -1;
        }
    }
    else {
        CPE_ERROR(module->m_em, "obj url only set src %s, no id or name", ui_data_src_path_dump(gd_app_tmp_buffer(module->m_app), src));
        return -1;
    }

    ui_runtime_render_obj_set_src(render_obj, src);
    
    return 0;
}

static int plugin_basicanim_img_block_setup(void * ctx, ui_runtime_render_obj_t render_obj, char * args) {
	plugin_basicanim_img_block_t obj = (plugin_basicanim_img_block_t)ui_runtime_render_obj_data(render_obj);
	ui_data_src_t src = ui_runtime_render_obj_src(render_obj);
	plugin_basicanim_module_t module = ctx;
	char * str_value;
	int rv = 0;

	if ((str_value = cpe_str_read_and_remove_arg(args, "img-block-filter", ',', '='))) {
		obj->m_filter = ui_runtime_render_texture_filter_from_str(str_value,  obj->m_filter);
	}

	return rv;
}

static void plugin_basicanim_img_block_bounding(void * ctx, ui_runtime_render_obj_t render_obj, ui_rect_t bounding) {
    struct plugin_basicanim_img_block * obj = ui_runtime_render_obj_data(render_obj);

    if (obj->m_img_block) {
        UI_IMG_BLOCK const * img_block_data = ui_data_img_block_data(obj->m_img_block);
        bounding->lt.x = bounding->lt.y = 0.0f;
        bounding->rb.x = img_block_data->src_w;
        bounding->rb.y = img_block_data->src_h;
    }
    else {
        bounding->lt.x = bounding->lt.y = bounding->rb.x = bounding->rb.y = 0.0f;
    }
}

int plugin_basicanim_img_block_render(
    void * ctx, ui_runtime_render_obj_t render_obj,
    ui_runtime_render_t context, ui_rect_t clip_rect, ui_runtime_render_second_color_t second_color, ui_transform_t t)
{
    struct plugin_basicanim_img_block * obj = ui_runtime_render_obj_data(render_obj);
    UI_IMG_BLOCK const * img_block_data;
    ui_rect src_rect;
    
    if (obj->m_img_block == NULL) return -1;
    
    img_block_data = ui_data_img_block_data(obj->m_img_block);

    if (img_block_data->flag & UI_IMG_BLOCK_FLAG_SCALESTYPE) {
        /*TODO: */
        return 0;
    }
    else {
        ui_cache_res_t texture;
        
        //ui_runtime_render_program_state_t
        src_rect.lt.x = img_block_data->src_x;
        src_rect.lt.y = img_block_data->src_y;    
        src_rect.rb.x = img_block_data->src_x + img_block_data->src_w;
        src_rect.rb.y = img_block_data->src_y + img_block_data->src_h;

        texture = ui_data_img_block_using_texture(obj->m_img_block);
        if (texture == NULL) return -1;

        plugin_basicanim_render_draw_rect(context, NULL, clip_rect, t, second_color, texture, &src_rect, obj->m_filter);
        
        return 0;
    }
}

int plugin_basicanim_img_block_register(plugin_basicanim_module_t module) {
    ui_runtime_render_obj_meta_t obj_meta;

    obj_meta =
        ui_runtime_render_obj_meta_create(
            module->m_runtime, "img-block", UI_OBJECT_TYPE_IMG_BLOCK, sizeof(struct plugin_basicanim_img_block), module,
            plugin_basicanim_img_block_init,
            plugin_basicanim_img_block_set,
            plugin_basicanim_img_block_setup,
            NULL,
            NULL,
            plugin_basicanim_img_block_render,
            NULL,
            plugin_basicanim_img_block_bounding,
            NULL);
    if (obj_meta == NULL) {
        CPE_ERROR(module->m_em, "%s: create: register render obj fail", plugin_basicanim_module_name(module));
        return -1;
    }

    return 0;
}

void plugin_basicanim_img_block_unregister(plugin_basicanim_module_t module) {
    ui_runtime_render_obj_meta_t obj_meta = ui_runtime_render_obj_meta_find_by_id(module->m_runtime, UI_OBJECT_TYPE_IMG_BLOCK);
    if (obj_meta) {
        ui_runtime_render_obj_meta_free(obj_meta);
    }
}

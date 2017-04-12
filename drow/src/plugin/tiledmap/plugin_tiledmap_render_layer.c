#include <assert.h>
#include "cpe/utils/math_ex.h"
#include "render/utils/ui_color.h"
#include "render/utils/ui_vector_2.h"
#include "render/utils/ui_rect.h"
#include "render/utils/ui_transform.h"
#include "render/model/ui_data_src.h"
#include "render/model/ui_data_module.h"
#include "render/model/ui_data_sprite.h"
#include "render/cache/ui_cache_texture.h"
#include "render/runtime/ui_runtime_module.h"
#include "render/runtime/ui_runtime_render.h"
#include "render/runtime/ui_runtime_render_buff_use.h"
#include "render/runtime/ui_runtime_render_second_color.h"
#include "render/runtime/ui_runtime_render_cmd_utils_2d.h"
#include "render/runtime/ui_runtime_render_obj.h"
#include "render/runtime/ui_runtime_render_obj_meta.h"
#include "render/runtime/ui_runtime_render_obj_ref.h"
#include "plugin/basicanim/plugin_basicanim_utils.h"
#include "plugin_tiledmap_render_layer_i.h"
#include "plugin_tiledmap_layer_i.h"
#include "plugin_tiledmap_tile_i.h"
#include "plugin_tiledmap_data_tile_i.h"

plugin_tiledmap_layer_t plugin_tiledmap_render_layer_layer(plugin_tiledmap_render_layer_t render) {
    return render->m_layer;
}

void plugin_tiledmap_render_layer_set_layer(plugin_tiledmap_render_layer_t render, plugin_tiledmap_layer_t layer) {
    render->m_layer = layer;
}

int plugin_tiledmap_render_layer_do_init(void * ctx, ui_runtime_render_obj_t render_obj) {
    plugin_tiledmap_render_layer_t obj = ui_runtime_render_obj_data(render_obj);
    obj->m_layer = NULL;
    return 0;
}

int plugin_tiledmap_render_layer_do_render(
    void * ctx, ui_runtime_render_obj_t render_obj,
    ui_runtime_render_t render, ui_rect_t clip_rect,
    ui_runtime_render_second_color_t second_color, ui_transform_t transform)
{
    plugin_tiledmap_module_t module = ctx;
    plugin_tiledmap_render_layer_t obj = ui_runtime_render_obj_data(render_obj);
    struct plugin_tiledmap_tile_it tile_it;
    plugin_tiledmap_tile_t tile;
    ui_color color;
    uint32_t abgr;
    ui_runtime_render_cmd_t batch_cmd;
    struct ui_transform layer_trans;
    int rv = 0;
    
    if (obj->m_layer == NULL) return -1;
    if (obj->m_layer->m_alpha == 0.0f) return 0;
    
    color = second_color ? second_color->m_color : UI_COLOR_WHITE;
    color.a *= obj->m_layer->m_alpha;
    abgr = ui_color_make_abgr(&color);

    batch_cmd = NULL;

    layer_trans = obj->m_layer->m_trans;
    if (transform) ui_transform_adj_by_parent(&layer_trans, transform);
    
    plugin_tiledmap_layer_tiles(&tile_it, obj->m_layer);

    while((tile = plugin_tiledmap_tile_it_next(&tile_it))) {
        plugin_tiledmap_data_tile_t data_tile = plugin_tiledmap_tile_data(tile);
        TILEDMAP_TILE const * tile_data = plugin_tiledmap_data_tile_data(data_tile);
        ui_vector_2_t pos = plugin_tiledmap_tile_pos(tile);
		ui_transform o = UI_TRANSFORM_IDENTITY;
		ui_vector_3 s;
        ui_vector_3 t = UI_VECTOR_3_INITLIZER(pos->x, pos->y, 0.0f);

        s.x = (tile_data->flip_type & 0x01) ? - tile_data->scale.x : tile_data->scale.x;
        s.y = (tile_data->flip_type & 0x02) ? - tile_data->scale.y : tile_data->scale.y;
        s.z = 1.0f;

        ui_transform_set_scale(&o, &s);
        ui_transform_set_pos_3(&o, &t);

        ui_transform_adj_by_parent(&o, &layer_trans);

        if (tile_data->ref_type == tiledmap_tile_ref_type_img) {
            UI_IMG_BLOCK const * img_block_data;
            ui_rect src_rect;

            if (tile->m_using_product == NULL) continue;

            img_block_data = ui_data_img_block_data(tile->m_using_product);
            src_rect.lt.x = img_block_data->src_x;
            src_rect.lt.y = img_block_data->src_y;    
            src_rect.rb.x = img_block_data->src_x + img_block_data->src_w;
            src_rect.rb.y = img_block_data->src_y + img_block_data->src_h;

            plugin_basicanim_render_draw_rect(render, &batch_cmd, NULL, &o, second_color, tile->m_using_texture, &src_rect, ui_runtime_render_filter_linear);
        }
        else if (tile_data->ref_type == tiledmap_tile_ref_type_frame) {
            if (tile->m_using_product == NULL) continue;
            plugin_basicanim_render_draw_frame(render, &batch_cmd, NULL, &o, tile->m_using_product, second_color, module->m_em);
        }
        else if (tile_data->ref_type == tiledmap_tile_ref_type_tag) {
            if (tile_data->name[0] == '+') {
                ui_runtime_render_obj_ref_t extern_obj = ui_runtime_render_obj_find_child(render_obj, tile_data);
                if (extern_obj == NULL) {
                    char * left_args = NULL;
                    extern_obj = ui_runtime_render_obj_ref_create_by_res(
                        ui_runtime_render_obj_module(render_obj), tile_data->name + 1, &left_args);
                    if (extern_obj == NULL) {
                        CPE_ERROR(
                            module->m_em, "plugin_tiledmap_render_obj_render: extern res %s create fail!",
                            tile_data->name + 1);
                        continue;
                    }

                    ui_runtime_render_obj_ref_set_is_updator(extern_obj, 0);
                        
                    if (ui_runtime_render_obj_add_child(render_obj, extern_obj, tile_data, 0) != 0) {
                        CPE_ERROR(module->m_em, "plugin_tiledmap_render_obj_render: add child render obj fail!");
                        ui_runtime_render_obj_ref_free(extern_obj);
                        continue;
                    }
                }

                if (extern_obj) {
                    ui_runtime_render_obj_ref_render(extern_obj, render, clip_rect, &o);
                }
            }
        }
    }

    if (ui_runtime_render_cmd_quad_batch_commit(&batch_cmd, render) != 0) {
        return rv = -1;
    }

    return rv;
}

int plugin_tiledmap_render_layer_regist(plugin_tiledmap_module_t module) {
    if (module->m_runtime) {
        ui_runtime_render_obj_meta_t obj_meta = NULL;

        obj_meta =
            ui_runtime_render_obj_meta_create(
                module->m_runtime, "tiledmap-layer", 0, sizeof(struct plugin_tiledmap_render_layer), module,
                plugin_tiledmap_render_layer_do_init,
                NULL,
                NULL,
                NULL,
                NULL,
                plugin_tiledmap_render_layer_do_render,
                NULL,
                NULL,
                NULL);
        if (obj_meta == NULL) {
            CPE_ERROR(module->m_em, "tiledmap_render_regist: register render obj fail");
            return -1;
        }
    }

    return 0;
}

void plugin_tiledmap_render_layer_unregist(plugin_tiledmap_module_t module) {
    if (module->m_runtime) {
        ui_runtime_render_obj_meta_t obj_meta = NULL;

        if ((obj_meta = ui_runtime_render_obj_meta_find_by_name(module->m_runtime, "tiledmap-layer"))) {
            ui_runtime_render_obj_meta_free(obj_meta);
        }
    }
}


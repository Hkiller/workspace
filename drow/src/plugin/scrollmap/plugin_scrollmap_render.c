#include <assert.h>
#include "cpe/utils/math_ex.h"
#include "render/utils/ui_color.h"
#include "render/utils/ui_vector_2.h"
#include "render/model/ui_data_src.h"
#include "render/model/ui_data_module.h"
#include "render/model/ui_data_sprite.h"
#include "render/runtime/ui_runtime_module.h"
#include "render/runtime/ui_runtime_render.h"
#include "render/runtime/ui_runtime_render_second_color.h"
#include "render/runtime/ui_runtime_render_buff_use.h"
#include "render/runtime/ui_runtime_render_obj.h"
#include "render/runtime/ui_runtime_render_obj_meta.h"
#include "render/runtime/ui_runtime_render_obj_ref.h"
#include "plugin_scrollmap_render_i.h"
#include "plugin_scrollmap_layer_i.h"
#include "plugin_scrollmap_block_i.h"

plugin_scrollmap_layer_t plugin_scrollmap_render_layer(plugin_scrollmap_render_t render) {
    return render->m_layer;
}

void plugin_scrollmap_render_set_layer(plugin_scrollmap_render_t render, plugin_scrollmap_layer_t layer) {
    render->m_layer = layer;
}

int plugin_scrollmap_render_do_init(void * ctx, ui_runtime_render_obj_t render_obj) {
    plugin_scrollmap_render_t obj = ui_runtime_render_obj_data(render_obj);
    obj->m_layer = NULL;
    return 0;
}

int plugin_scrollmap_render_do_render(
    void * ctx, ui_runtime_render_obj_t render_obj,
    ui_runtime_render_t context, ui_rect_t clip_rect,
    ui_runtime_render_second_color_t second_color, ui_transform_t transform)
{
    plugin_scrollmap_render_t obj = ui_runtime_render_obj_data(render_obj);
    plugin_scrollmap_block_t block;
    uint32_t abgr;

    if (obj->m_layer == NULL) return -1;

    assert(second_color);
    abgr = ui_color_make_abgr(&second_color->m_color);

    TAILQ_FOREACH(block, &obj->m_layer->m_blocks, m_next_for_layer) {
        plugin_scrollmap_tile_t tile = block->m_tile;
        SCROLLMAP_PAIR const * data_pos = plugin_scrollmap_block_pos(block);
        ui_transform o = tile->m_transform;
        ui_vector_2 pos = UI_VECTOR_2_INITLIZER( data_pos->x, data_pos->y );

        ui_transform_adj_by_pos_2(&o, &pos);
        if (transform) ui_transform_adj_by_parent(&o, transform);

        if (tile->m_data->res_type == scrollmap_tile_res_type_module) {
            float half_w = tile->m_data->res_w * 0.5f;
            float half_h = tile->m_data->res_h * 0.5f;
            ui_vector_2 pt1 = UI_VECTOR_2_INITLIZER(- half_w,   half_h );
            ui_vector_2 pt2 = UI_VECTOR_2_INITLIZER(- half_w, - half_h );
            ui_vector_2 pt3 = UI_VECTOR_2_INITLIZER(  half_w, - half_h );
            ui_vector_2 pt4 = UI_VECTOR_2_INITLIZER(  half_w,   half_h );

            ui_transform_inline_adj_vector_2(&o, &pt1);
            ui_transform_inline_adj_vector_2(&o, &pt2);
            ui_transform_inline_adj_vector_2(&o, &pt3);
            ui_transform_inline_adj_vector_2(&o, &pt4);

            do {
                ui_runtime_vertex_v3f_t2f_c4ub vertexs[4] = {
                    { UI_VECTOR_3_INITLIZER(pt1.x, pt1.y, 0.0f), UI_VECTOR_2_INITLIZER(tile->m_module.m_uv.lt.x, tile->m_module.m_uv.rb.y), abgr },
                    { UI_VECTOR_3_INITLIZER(pt3.x, pt3.y, 0.0f), UI_VECTOR_2_INITLIZER(tile->m_module.m_uv.rb.x, tile->m_module.m_uv.lt.y), abgr },
                    { UI_VECTOR_3_INITLIZER(pt2.x, pt2.y, 0.0f), UI_VECTOR_2_INITLIZER(tile->m_module.m_uv.lt.x, tile->m_module.m_uv.lt.y), abgr },
                    { UI_VECTOR_3_INITLIZER(pt4.x, pt4.y, 0.0f), UI_VECTOR_2_INITLIZER(tile->m_module.m_uv.rb.x, tile->m_module.m_uv.rb.y), abgr },
                };

                //printf("pt1.x=%f,pt1.y=%f,u=%f,v=%f\n", pt1.x, pt1.y, tile->m_module.m_uv.lt.x, tile->m_module.m_uv.rb.y);
	
                if(second_color->m_mix == ui_runtime_render_second_color_none) {
                    /*
                    ui_runtime_render_add_ha_rect(
                        context, NULL, tile->m_module.m_texture, vertexs, ui_runtime_render_program_type_modulate,
                        GL_NEAREST, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                     */
                }
                else {
                    /*
                    ui_runtime_render_add_ha_rect(
                        context, NULL, tile->m_module.m_texture, vertexs, ui_runtime_render_program_type_modulate,
                        GL_NEAREST, blend->src_factor, blend->des_factor);
                     */
                }
            } while(0);
        }
        else if (tile->m_data->res_type == scrollmap_tile_res_type_sprite) {
        }
    }
    
    return 0;
}

int plugin_scrollmap_render_regist(plugin_scrollmap_module_t module) {
    if (module->m_runtime) {
        ui_runtime_render_obj_meta_t obj_meta = NULL;

        obj_meta =
            ui_runtime_render_obj_meta_create(
                module->m_runtime, "scrollmap-layer", 0, sizeof(struct plugin_scrollmap_render), module,
                plugin_scrollmap_render_do_init,
                NULL,
                NULL,
                NULL,
                NULL,
                plugin_scrollmap_render_do_render,
                NULL,
                NULL,
                NULL);
        if (obj_meta == NULL) {
            CPE_ERROR(module->m_em, "scrollmap_render_regist: register render obj fail");
            return -1;
        }
    }

    return 0;
}

void plugin_scrollmap_render_unregist(plugin_scrollmap_module_t module) {
    if (module->m_runtime) {
        ui_runtime_render_obj_meta_t obj_meta = NULL;

        if ((obj_meta = ui_runtime_render_obj_meta_find_by_name(module->m_runtime, "scrollmap-layer"))) {
            ui_runtime_render_obj_meta_free(obj_meta);
        }
    }
}


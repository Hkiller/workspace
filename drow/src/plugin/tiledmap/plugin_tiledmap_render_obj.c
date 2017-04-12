#include <assert.h>
#include "cpe/utils/math_ex.h"
#include "cpe/pal/pal_stdio.h"
#include "render/model/ui_data_src.h"
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
#include "plugin_tiledmap_render_obj_i.h"
#include "plugin_tiledmap_data_tile_i.h"

plugin_tiledmap_data_scene_t plugin_tiledmap_render_obj_data(plugin_tiledmap_render_obj_t obj) {
    return obj->m_data_scene;
}

int plugin_tiledmap_render_obj_set_data(plugin_tiledmap_render_obj_t obj, plugin_tiledmap_data_scene_t data_scene) {
    obj->m_data_scene = data_scene;
    return 0;
}

int plugin_tiledmap_render_obj_do_init(void * ctx, ui_runtime_render_obj_t render_obj) {
    plugin_tiledmap_render_obj_t obj = ui_runtime_render_obj_data(render_obj);
    obj->m_module = ctx;
    obj->m_data_scene = NULL;
    ui_runtime_render_obj_clear_childs(render_obj);
    
    return 0;
}

int plugin_tiledmap_render_obj_do_render(
    void * ctx, ui_runtime_render_obj_t render_obj,
    ui_runtime_render_t render, ui_rect_t clip_rect,
    ui_runtime_render_second_color_t second_color, ui_transform_t transform)
{
    plugin_tiledmap_module_t module = ctx;
    plugin_tiledmap_render_obj_t obj = ui_runtime_render_obj_data(render_obj);
    plugin_tiledmap_data_layer_t data_layer;
    plugin_tiledmap_data_tile_t data_tile;
    uint32_t abgr;
    ui_runtime_render_cmd_t batch_cmd;
    int rv = 0;
    ui_data_src_t src_cache;
    
    if (obj->m_data_scene == NULL) return -1;

    assert(second_color);
    abgr = ui_color_make_abgr(&second_color->m_color);

    batch_cmd = NULL;
    src_cache = NULL;

    TAILQ_FOREACH(data_layer, &obj->m_data_scene->m_layer_list, m_next_for_scene) {
        TAILQ_FOREACH(data_tile, &data_layer->m_tile_list, m_next_for_layer) {
            TILEDMAP_TILE const * tile_data = &data_tile->m_data;
            ui_transform o = UI_TRANSFORM_IDENTITY;
            ui_vector_3 s;
            ui_vector_3 t = UI_VECTOR_3_INITLIZER(tile_data->pos.x, tile_data->pos.y, 0.0f);
            
            s.x = (tile_data->flip_type & 0x01) ? - tile_data->scale.x : tile_data->scale.x;
            s.y = (tile_data->flip_type & 0x02) ? - tile_data->scale.y : tile_data->scale.y;
            s.z = 1.0f;

            ui_transform_set_scale(&o, &s);
            ui_transform_set_pos_3(&o, &t);

            if (transform) ui_transform_adj_by_parent(&o, transform);

            if (tile_data->ref_type == tiledmap_tile_ref_type_img) {
                ui_data_module_t using_module;
                ui_data_img_block_t img_block;
                UI_IMG_BLOCK const * img_block_data;
                ui_cache_res_t using_texture;
                ui_rect src_rect;

                if (src_cache == NULL || ui_data_src_id(src_cache) != tile_data->ref_data.img.module_id) {
                    src_cache = ui_data_src_find_by_id(module->m_data_mgr, tile_data->ref_data.img.module_id);
                    if (src_cache == NULL) continue;
                }

                using_module = ui_data_src_product(src_cache);
                if (using_module == NULL) {
                    CPE_ERROR(
                        module->m_em,
                        "plugin_tiledmap_render_obj_render: module "FMT_UINT32_T" not load!",
                        tile_data->ref_data.img.module_id);
                    continue;
                }
        
                img_block = ui_data_img_block_find_by_id(using_module, tile_data->ref_data.img.img_block_id);
                if (img_block == NULL) {
                    CPE_ERROR(
                        module->m_em,
                        "plugin_tiledmap_render_obj_render: module "FMT_UINT32_T" img_block "FMT_UINT32_T" not exist!",
                        tile_data->ref_data.img.module_id, tile_data->ref_data.img.img_block_id);
                    continue;
                }

                using_texture = ui_data_img_block_using_texture(img_block);
                if (using_texture == NULL) {
                    CPE_ERROR(
                        module->m_em, "plugin_tiledmap_tile_create: module "FMT_UINT32_T" using img %s not in cache!",
                        ui_data_src_id(src_cache), ui_data_img_block_using_texture_path(img_block));
                    continue;
                }
                
                img_block_data = ui_data_img_block_data(img_block);
                
                src_rect.lt.x = img_block_data->src_x;
                src_rect.lt.y = img_block_data->src_y;
                src_rect.rb.x = img_block_data->src_x + img_block_data->src_w;
                src_rect.rb.y = img_block_data->src_y + img_block_data->src_h;

                plugin_basicanim_render_draw_rect(render, &batch_cmd, clip_rect, &o, second_color, using_texture, &src_rect, ui_runtime_render_filter_linear);
            }
            else if (tile_data->ref_type == tiledmap_tile_ref_type_frame) {
                ui_data_sprite_t using_sprite;
                ui_data_frame_t frame;
                
                if (src_cache == NULL || ui_data_src_id(src_cache) != tile_data->ref_data.frame.sprite_id) {
                    src_cache = ui_data_src_find_by_id(module->m_data_mgr, tile_data->ref_data.frame.sprite_id);
                    if (src_cache == NULL) continue;
                }

                using_sprite = ui_data_src_product(src_cache);
                if (using_sprite == NULL) {
                    CPE_ERROR(
                        module->m_em,
                        "plugin_tiledmap_render_obj_render: sprite "FMT_UINT32_T" not load!",
                        tile_data->ref_data.frame.sprite_id);
                    continue;
                }

                frame = ui_data_frame_find_by_id(using_sprite, tile_data->ref_data.frame.frame_id);
                if (frame == NULL) {
                    CPE_ERROR(
                        module->m_em,
                        "plugin_tiledmap_render_obj_render: sprite "FMT_UINT32_T" no frame %d!",
                        tile_data->ref_data.frame.sprite_id, tile_data->ref_data.frame.frame_id);
                    continue;
                }
                
                plugin_basicanim_render_draw_frame(render, &batch_cmd, clip_rect, &o, frame, second_color, module->m_em);
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
    }
    
    if (ui_runtime_render_cmd_quad_batch_commit(&batch_cmd, render) != 0) {
        return rv = -1;
    }

    return rv;
}

static int plugin_tiledmap_render_obj_do_set(void * ctx, ui_runtime_render_obj_t obj, UI_OBJECT_URL const * obj_url) {
    plugin_tiledmap_module_t module = ctx;
    plugin_tiledmap_render_obj_t tiledmap_obj = ui_runtime_render_obj_data(obj);
    ui_data_src_t tiledmap_src;
    plugin_tiledmap_data_scene_t tiledmap_data;
    UI_OBJECT_URL_DATA_TILEDMAP const * tiledmap_url = &obj_url->data.tiledmap;
    
    tiledmap_src = ui_runtime_module_find_src(module->m_runtime, &tiledmap_url->src, ui_data_src_type_tiledmap_scene);
    if (tiledmap_src == NULL) {
        CPE_ERROR(module->m_em, "%s: tiledmap obj init: find src fail!", plugin_tiledmap_module_name(module));
        return -1;
    }

    tiledmap_data = ui_data_src_product(tiledmap_src);
    if (tiledmap_data == NULL) {
        CPE_ERROR(module->m_em, "%s: tiledmap obj init: data not loaded!", plugin_tiledmap_module_name(module));
        return -1;
    }

    if (plugin_tiledmap_render_obj_set_data(tiledmap_obj, tiledmap_data) != 0) return -1;

    ui_runtime_render_obj_set_src(obj, tiledmap_src);
        
    return 0;
}

static int plugin_tiledmap_render_obj_do_setup(void * ctx, ui_runtime_render_obj_t render_obj, char * args) {
    return 0;
}

static void plugin_tiledmap_render_obj_bounding(void * ctx, ui_runtime_render_obj_t render_obj, ui_rect_t bounding) {
    plugin_tiledmap_render_obj_t tiledmap_obj = ui_runtime_render_obj_data(render_obj);
    if (tiledmap_obj->m_data_scene) {
        if (plugin_tiledmap_data_scene_rect(tiledmap_obj->m_data_scene, bounding) != 0) {
            *bounding = UI_RECT_ZERO;
        }
    }
    else {
        *bounding = UI_RECT_ZERO;
    }
}

int plugin_tiledmap_render_obj_regist(plugin_tiledmap_module_t module) {
    if (module->m_runtime) {
        ui_runtime_render_obj_meta_t obj_meta = NULL;

        obj_meta =
            ui_runtime_render_obj_meta_create(
                module->m_runtime, "tiledmap", UI_OBJECT_TYPE_TILEDMAP,
                sizeof(struct plugin_tiledmap_render_obj), module,
                plugin_tiledmap_render_obj_do_init,
                plugin_tiledmap_render_obj_do_set,
                plugin_tiledmap_render_obj_do_setup,
                NULL,
                NULL,
                plugin_tiledmap_render_obj_do_render,
                NULL,
                plugin_tiledmap_render_obj_bounding,
                NULL);
        if (obj_meta == NULL) {
            CPE_ERROR(module->m_em, "tiledmap_render_regist: register render obj fail");
            return -1;
        }
    }

    return 0;
}

void plugin_tiledmap_render_obj_unregist(plugin_tiledmap_module_t module) {
    if (module->m_runtime) {
        ui_runtime_render_obj_meta_t obj_meta = NULL;

        if ((obj_meta = ui_runtime_render_obj_meta_find_by_name(module->m_runtime, "tiledmap"))) {
            ui_runtime_render_obj_meta_free(obj_meta);
        }
    }
}


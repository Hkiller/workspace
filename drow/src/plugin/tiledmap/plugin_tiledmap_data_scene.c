#include <assert.h>
#include "cpe/pal/pal_stdio.h"
#include "render/utils/ui_rect.h"
#include "render/model/ui_data_src_src.h"
#include "render/model/ui_object_ref.h"
#include "plugin_tiledmap_data_scene_i.h"
#include "plugin_tiledmap_data_layer_i.h"
#include "plugin_tiledmap_data_tile_i.h"

plugin_tiledmap_data_scene_t
plugin_tiledmap_data_scene_create(plugin_tiledmap_module_t module, ui_data_src_t src) {
    plugin_tiledmap_data_scene_t scene;

    if (ui_data_src_type(src) != ui_data_src_type_tiledmap_scene) {
        CPE_ERROR(
            module->m_em, "create scene at %s: src not scene!",
            ui_data_src_path_dump(&module->m_dump_buffer, src));
        return NULL;
    }

    if (ui_data_src_product(src)) {
        CPE_ERROR(
            module->m_em, "create scene at %s: product already loaded!",
            ui_data_src_path_dump(&module->m_dump_buffer, src));
        return NULL;
    }

    scene = (plugin_tiledmap_data_scene_t)mem_alloc(module->m_alloc, sizeof(struct plugin_tiledmap_data_scene));
    if (scene == NULL) {
        CPE_ERROR(
            module->m_em, "create scene at %s: alloc scene fail!",
            ui_data_src_path_dump(&module->m_dump_buffer, src));
        return NULL;
    }

    scene->m_module = module;
    scene->m_src = src;
    scene->m_layer_count = 0;
    TAILQ_INIT(&scene->m_layer_list);

    ui_data_src_set_product(src, scene);
    return scene;
}

void plugin_tiledmap_data_scene_free(plugin_tiledmap_data_scene_t scene) {
    plugin_tiledmap_module_t module = scene->m_module;

    while(!TAILQ_EMPTY(&scene->m_layer_list)) {
        plugin_tiledmap_data_layer_free(TAILQ_FIRST(&scene->m_layer_list));
    }

    mem_free(module->m_alloc, scene);
}

plugin_tiledmap_module_t plugin_tiledmap_data_scene_module(plugin_tiledmap_data_scene_t scene) {
    return scene->m_module;
}

TILEDMAP_SCENE * plugin_tiledmap_data_scene_data(plugin_tiledmap_data_scene_t scene) {
    return &scene->m_data;
}

uint32_t plugin_tiledmap_data_scene_layer_count(plugin_tiledmap_data_scene_t scene) {
    return scene->m_layer_count;
}

void plugin_tiledmap_data_scene_config_rect(plugin_tiledmap_data_scene_t scene, ui_rect_t rect) {
    uint8_t have_data = 0;
    plugin_tiledmap_data_layer_t layer;

    TAILQ_FOREACH(layer, &scene->m_layer_list, m_next_for_scene) {
        ui_rect layer_rect;

        plugin_tiledmap_data_layer_config_rect(layer, &layer_rect);

        if (have_data) {
            if (layer_rect.lt.x < rect->lt.x) rect->lt.x = layer_rect.lt.x;
            if (layer_rect.lt.y < rect->lt.y) rect->lt.y = layer_rect.lt.y;
            if (layer_rect.rb.x > rect->rb.x) rect->rb.x = layer_rect.rb.x;
            if (layer_rect.rb.y > rect->rb.y) rect->rb.y = layer_rect.rb.y;
        }
        else {
            have_data = 1;
            *rect = layer_rect;
        }
    }

    if (!have_data) {
        rect->lt.x = 0.0f;
        rect->lt.y = 0.0f;
        rect->rb.x = 0.0f;
        rect->rb.y = 0.0f;
    }
}

int plugin_tiledmap_data_scene_rect(plugin_tiledmap_data_scene_t scene, ui_rect_t rect) {
    uint8_t have_data = 0;
    plugin_tiledmap_data_layer_t layer;

    TAILQ_FOREACH(layer, &scene->m_layer_list, m_next_for_scene) {
        ui_rect layer_rect;

        if (plugin_tiledmap_data_layer_rect(layer, &layer_rect) != 0) return -1;

        if (have_data) {
            if (layer_rect.lt.x < rect->lt.x) rect->lt.x = layer_rect.lt.x;
            if (layer_rect.lt.y < rect->lt.y) rect->lt.y = layer_rect.lt.y;
            if (layer_rect.rb.x > rect->rb.x) rect->rb.x = layer_rect.rb.x;
            if (layer_rect.rb.y > rect->rb.y) rect->rb.y = layer_rect.rb.y;
        }
        else {
            have_data = 1;
            *rect = layer_rect;
        }
    }

    if (!have_data) {
        rect->lt.x = 0.0f;
        rect->lt.y = 0.0f;
        rect->rb.x = 0.0f;
        rect->rb.y = 0.0f;
    }
    
    return 0;
}

int plugin_tiledmap_data_scene_update_usings(ui_data_src_t src) {
    plugin_tiledmap_data_scene_t scene = ui_data_src_product(src);
    plugin_tiledmap_module_t module = scene->m_module;
    plugin_tiledmap_data_layer_t layer;
    plugin_tiledmap_data_tile_t tile;
    int rv = 0;

    TAILQ_FOREACH(layer, &scene->m_layer_list, m_next_for_scene) {
        TAILQ_FOREACH(tile, &layer->m_tile_list, m_next_for_layer) {
            ui_data_src_t using_src = NULL;
            
            if (tile->m_data.ref_type == tiledmap_tile_ref_type_tag) {
                if (tile->m_data.name[0] == '+') {
                    UI_OBJECT_URL url_buf;
                    UI_OBJECT_URL * url = ui_object_ref_parse(tile->m_data.name + 1, &url_buf, module->m_em);
                    if (url == NULL) {
                        CPE_ERROR(module->m_em, "%s: using src %s format error!", ui_data_src_path_dump(gd_app_tmp_buffer(module->m_app), src), tile->m_data.name + 1);
                        rv = -1;
                        continue;
                    }
                    
                    using_src = ui_data_src_find_by_url(module->m_data_mgr, &url_buf);
                    if (using_src == NULL) {
                        CPE_ERROR(module->m_em, "%s: using src %s not exist!", ui_data_src_path_dump(gd_app_tmp_buffer(module->m_app), src), tile->m_data.name + 1);
                        rv = -1;
                        continue;
                    }
                }
                else {
                    continue;
                }
            }
            else {
                uint32_t src_id = plugin_tiledmap_data_tile_src_id(tile);

                if (src_id == 0) continue;
            
                using_src = ui_data_src_find_by_id(module->m_data_mgr, src_id);
                if (using_src == NULL) {
                    CPE_ERROR(module->m_em, "%s: using src "FMT_UINT32_T" not exist!", ui_data_src_path_dump(gd_app_tmp_buffer(module->m_app), src), src_id);
                    rv = -1;
                    continue;
                }
            }

            assert(using_src);
            
            if (ui_data_src_src_create(src, using_src) == NULL) {
                CPE_ERROR(module->m_em, "%s: using src create fail!", ui_data_src_path_dump(gd_app_tmp_buffer(module->m_app), src));
                rv = -1;
                continue;
            }
        }
    }

    return rv;
}
    
static plugin_tiledmap_data_layer_t plugin_tiledmap_data_scene_layer_next(struct plugin_tiledmap_data_layer_it * it) {
    plugin_tiledmap_data_layer_t * data = (plugin_tiledmap_data_layer_t *)(it->m_data);
    plugin_tiledmap_data_layer_t r;

    if (*data == NULL) return NULL;

    r = *data;

    *data = TAILQ_NEXT(r, m_next_for_scene);

    return r;
}

void plugin_tiledmap_data_scene_layers(plugin_tiledmap_data_layer_it_t layer_it, plugin_tiledmap_data_scene_t scene) {
    *(plugin_tiledmap_data_layer_t *)(layer_it->m_data) = TAILQ_FIRST(&scene->m_layer_list);
    layer_it->next = plugin_tiledmap_data_scene_layer_next;
}

#include <assert.h>
#include "plugin/tiledmap/plugin_tiledmap_data_scene.h"
#include "plugin/tiledmap/plugin_tiledmap_data_layer.h"
#include "plugin/tiledmap/plugin_tiledmap_data_tile.h"
#include "plugin_tiledmap_manip_scene_import_utils_i.h"
#include "plugin_tiledmap_manip_scene_import_scene_i.h"
#include "plugin_tiledmap_manip_scene_import_layer_i.h"
#include "plugin_tiledmap_manip_scene_import_tile_ref_i.h"
#include "plugin_tiledmap_manip_scene_import_tile_i.h"
#include "plugin_tiledmap_manip_scene_import_tiled_proj_i.h"
#include "plugin_tiledmap_manip_scene_import_tiled_layer_i.h"

int plugin_tiledmap_manip_import_scene_build_tiledmap(plugin_tiledmap_manip_import_scene_t scene) {
    plugin_tiledmap_manip_import_ctx_t ctx = scene->m_ctx;
    ui_ed_src_t scene_src;
    ui_ed_obj_t scene_obj;
    plugin_tiledmap_manip_import_layer_t layer;
    int32_t adj_x;
    int32_t adj_y;

    switch(scene->m_position) {
    case plugin_tiledmap_manip_import_bottom_left:
        adj_x = 0;
        adj_y = - (int32_t)scene->m_height;
        break;
    case plugin_tiledmap_manip_import_bottom_right:
        adj_x = - (int32_t)scene->m_width;
        adj_y = - (int32_t)scene->m_height;
        break;
    case plugin_tiledmap_manip_import_top_left:
        adj_x = 0;
        adj_y = 0;
        break;
    case plugin_tiledmap_manip_import_top_right:
        adj_x = - (int32_t)scene->m_width;
        adj_y = 0;
        break;
    default:
        CPE_ERROR(ctx->m_em, "scene import from %s: build scene %s: unknown scene position %d!", ctx->m_proj_path, scene->m_path, scene->m_position);
        return -1;
    }

    scene_src = ui_ed_src_check_create(ctx->m_ed_mgr, scene->m_path, ui_data_src_type_tiledmap_scene);
    if (scene_src == NULL) {
        CPE_ERROR(ctx->m_em, "scene import from %s: build scene %s: check create scene src at!", ctx->m_proj_path, scene->m_path);
        return -1;
    }

    scene_obj = ui_ed_obj_only_child(ui_ed_src_root_obj(scene_src));
    assert(scene_obj);

    TAILQ_FOREACH(layer, &scene->m_layers, m_next) {
        if (plugin_tiledmap_manip_import_build_layer(
                ctx, scene_obj, layer->m_layer_name, &layer->m_tile_refs, adj_x, adj_y, layer->m_tile_w, layer->m_tile_h))
        {
            CPE_ERROR(
                ctx->m_em, "scene import from %s: build scene %s: build layer %s fail!",
                ctx->m_proj_path, scene->m_path, layer->m_layer_name);
            return -1;
        }
    }

    if (scene->m_tiled_proj) {
        plugin_tiledmap_manip_import_tiled_layer_t tiled_layer;

        TAILQ_FOREACH(tiled_layer, &scene->m_tiled_proj->m_layers, m_next) {
            if (plugin_tiledmap_manip_import_build_layer(
                    ctx, scene_obj, tiled_layer->m_name, &tiled_layer->m_tile_refs, adj_x, adj_y,
                    scene->m_tiled_proj->m_tile_w, scene->m_tiled_proj->m_tile_h))
            {
                CPE_ERROR(
                    ctx->m_em, "scene import from %s: build scene %s: build tiled layer %s fail!",
                    ctx->m_proj_path, scene->m_path, tiled_layer->m_name);
                return -1;
            }
        }
    }

    return 0;
}

#include "cpe/utils/string_utils.h"
#include "plugin_tiledmap_manip_scene_import_tiled_layer_i.h"
#include "plugin_tiledmap_manip_scene_import_tiled_tileset_i.h"
#include "plugin_tiledmap_manip_scene_import_tile_ref_i.h"
#include "plugin_tiledmap_manip_scene_import_utils_i.h"

plugin_tiledmap_manip_import_tiled_layer_t
plugin_tiledmap_manip_import_tiled_layer_create(plugin_tiledmap_manip_import_tiled_proj_t proj, cfg_t cfg) {
    plugin_tiledmap_manip_import_ctx_t ctx = proj->m_scene->m_ctx;
    plugin_tiledmap_manip_import_tiled_layer_t layer;
    const char * name;

    name = cfg_get_string(cfg, "name", NULL);
    if (name == NULL) {
        CPE_ERROR(
            ctx->m_em, "scene import from %s: load scene: create tiled proj %s: name not configured!",
            ctx->m_proj_path, proj->m_path);
        return NULL;
    }
    
    layer = mem_calloc(ctx->m_alloc, sizeof(struct plugin_tiledmap_manip_import_ctx));
    if (layer == NULL) {
        CPE_ERROR(
            ctx->m_em, "scene import from %s: load scene: create tiled proj %s: layer %s: alloc fail!",
            ctx->m_proj_path, proj->m_path, name);
        return NULL;
    }

    cpe_str_dup(layer->m_name, sizeof(layer->m_name), name);
    layer->m_proj = proj;
    layer->m_col_count = cfg_get_uint32(cfg, "width", 0);
    layer->m_row_count = cfg_get_uint32(cfg, "height", 0);
    layer->m_cfg = cfg;
    TAILQ_INIT(&layer->m_tile_refs);

    if (layer->m_col_count == 0 || layer->m_row_count == 0) {
        CPE_ERROR(
            ctx->m_em, "scene import from %s: load scene: create tiled proj %s: layer %s: layer size %dx%d error!",
            ctx->m_proj_path, proj->m_path, name, layer->m_col_count, layer->m_row_count);
        return NULL;
    }

    TAILQ_INSERT_TAIL(&proj->m_layers, layer, m_next);

    return layer;
}

void plugin_tiledmap_manip_import_tiled_layer_free(plugin_tiledmap_manip_import_tiled_layer_t layer) {
    plugin_tiledmap_manip_import_ctx_t ctx = layer->m_proj->m_scene->m_ctx;

    while(!TAILQ_EMPTY(&layer->m_tile_refs)) {
        plugin_tiledmap_manip_import_tile_ref_free(TAILQ_FIRST(&layer->m_tile_refs));
    }
    
    TAILQ_REMOVE(&layer->m_proj->m_layers, layer, m_next);

    mem_free(ctx->m_alloc, layer);
}

int plugin_tiledmap_manip_import_tiled_layer_load(plugin_tiledmap_manip_import_tiled_layer_t layer) {
    plugin_tiledmap_manip_import_scene_t scene = layer->m_proj->m_scene;
    plugin_tiledmap_manip_import_ctx_t ctx = scene->m_ctx;
    struct cfg_it data_it;
    cfg_t data_cfg;
    uint32_t i;

    cfg_it_init(&data_it, cfg_find_cfg(layer->m_cfg, "data"));
    for(i = 0; (data_cfg = cfg_it_next(&data_it)); i++) {
        uint32_t tile_index = cfg_as_uint32(data_cfg, 0);
        plugin_tiledmap_manip_import_tile_ref_t tileset_tile;
        uint32_t col;
        uint32_t row;

        if (tile_index == 0) continue;

        tileset_tile = plugin_tiledmap_manip_import_tiled_proj_find_tile_by_index(layer->m_proj, tile_index);
        if (tileset_tile == NULL) continue;

        row = i / layer->m_col_count;
        col = i % layer->m_col_count;

        if (plugin_tiledmap_manip_import_tile_ref_create(
                &layer->m_tile_refs,
                tileset_tile->m_tile,
                col * layer->m_proj->m_tile_w, row * layer->m_proj->m_tile_h)
            == NULL)
        {
            CPE_ERROR(
                ctx->m_em, "scene import from %s: load scene: tiled layer %s load: tile_index %d create tile fail!",
                ctx->m_proj_path, layer->m_name, tile_index);
            return -1;
        }
    }

    if (layer->m_col_count * layer->m_proj->m_tile_w > scene->m_width) {
        scene->m_width = layer->m_col_count * layer->m_proj->m_tile_w;
    }

    if (layer->m_row_count * layer->m_proj->m_tile_h > scene->m_height) {
        scene->m_height = layer->m_row_count * layer->m_proj->m_tile_h;
    }
    
    return 0;
}

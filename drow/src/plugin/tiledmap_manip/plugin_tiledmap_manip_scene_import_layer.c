#include <assert.h>
#include "cpe/utils/string_utils.h"
#include "render/cache/ui_cache_pixel_buf.h"
#include "plugin_tiledmap_manip_scene_import_layer_i.h"
#include "plugin_tiledmap_manip_scene_import_tile_ref_i.h"
#include "plugin_tiledmap_manip_scene_import_utils_i.h"

plugin_tiledmap_manip_import_layer_t
plugin_tiledmap_manip_import_layer_create(plugin_tiledmap_manip_import_scene_t scene, cfg_t cfg) {
    plugin_tiledmap_manip_import_ctx_t ctx = scene->m_ctx;
    plugin_tiledmap_manip_import_layer_t layer;
    const char * layer_name;
    const char * input_path;

    layer = mem_calloc(ctx->m_alloc, sizeof(struct plugin_tiledmap_manip_import_layer));
    if (layer == NULL) {
        CPE_ERROR(ctx->m_em, "scene import from %s: alloc layer fail!", ctx->m_proj_path);
        return NULL;
    }

    layer->m_scene = scene;
    TAILQ_INIT(&layer->m_tile_refs);

    layer_name = cfg_get_string(cfg, "layer", NULL);
    if (layer_name == NULL) {
        CPE_ERROR(ctx->m_em, "scene import from %s: scene %s: layer not configured!", ctx->m_proj_path, scene->m_path);
        mem_free(ctx->m_alloc, layer);
        return NULL;
    }
    cpe_str_dup(layer->m_layer_name, sizeof(layer->m_layer_name), layer_name);

    input_path = cfg_get_string(cfg, "input", NULL);
    if (input_path == NULL) {
        CPE_ERROR(
            ctx->m_em, "scene import from %s: scene %s: layer %s: input not configured!",
            ctx->m_proj_path, scene->m_path, layer->m_layer_name);
        mem_free(ctx->m_alloc, layer);
        return NULL;
    }
    cpe_str_dup(layer->m_input_path, sizeof(layer->m_input_path), input_path);
    
    layer->m_tile_w = cfg_get_uint32(cfg, "tile.w", 0);
    layer->m_tile_h = cfg_get_uint32(cfg, "tile.h", 0);
    if (layer->m_tile_w <= 0 || layer->m_tile_h <= 0) {
        CPE_ERROR(
            ctx->m_em, "scene import from %s: scene %s: layer %s: tile (w=%u, h=%u) error!",
            ctx->m_proj_path, scene->m_path, layer->m_layer_name, layer->m_tile_w, layer->m_tile_h);
        mem_free(ctx->m_alloc, layer);
        return NULL;
    }

    TAILQ_INSERT_TAIL(&scene->m_layers, layer, m_next);
    return layer;
}

void plugin_tiledmap_manip_import_layer_free(plugin_tiledmap_manip_import_layer_t layer) {
    plugin_tiledmap_manip_import_ctx_t ctx = layer->m_scene->m_ctx;

    while(!TAILQ_EMPTY(&layer->m_tile_refs)) {
        plugin_tiledmap_manip_import_tile_ref_free(TAILQ_FIRST(&layer->m_tile_refs));
    }

    if (layer->m_pixel_buf) {
        ui_cache_pixel_buf_free(layer->m_pixel_buf);
        layer->m_pixel_buf = NULL;
    }
    
    TAILQ_REMOVE(&layer->m_scene->m_layers, layer, m_next);
    mem_free(ctx->m_alloc, layer);
}

int plugin_tiledmap_manip_import_layer_load(plugin_tiledmap_manip_import_layer_t layer) {
    plugin_tiledmap_manip_import_scene_t scene = layer->m_scene;
    plugin_tiledmap_manip_import_ctx_t ctx = scene->m_ctx;
    ui_cache_pixel_level_info_t pixel_level_info;

    if (plugin_tiledmap_manip_import_load_layer(
            ctx, &layer->m_pixel_buf, &layer->m_tile_refs,
            layer->m_input_path, layer->m_layer_name, layer->m_tile_w, layer->m_tile_h)
        != 0)
    {
        CPE_ERROR(
            ctx->m_em, "scene import from %s: load %s: layer %s: load layer fail!",
            ctx->m_proj_path, layer->m_scene->m_path, layer->m_layer_name);
        return -1;
    }


    assert(layer->m_pixel_buf);

    pixel_level_info = ui_cache_pixel_buf_level_info_at(layer->m_pixel_buf, 0);
    assert(pixel_level_info);

    if (ui_cache_pixel_buf_level_width(pixel_level_info) > scene->m_width) {
        scene->m_width = ui_cache_pixel_buf_level_width(pixel_level_info);
    }

    if (ui_cache_pixel_buf_level_height(pixel_level_info) > scene->m_height) {
        scene->m_height = ui_cache_pixel_buf_level_height(pixel_level_info);
    }
    
    return 0;
}

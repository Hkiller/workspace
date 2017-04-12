#include "cpe/utils/string_utils.h"
#include "cpe/utils/buffer.h"
#include "gd/app/app_context.h"
#include "render/model/ui_data_mgr.h"
#include "plugin_tiledmap_manip_scene_import_tiled_proj_i.h"
#include "plugin_tiledmap_manip_scene_import_tiled_tileset_i.h"
#include "plugin_tiledmap_manip_scene_import_tiled_layer_i.h"

plugin_tiledmap_manip_import_tiled_proj_t
plugin_tiledmap_manip_import_tiled_proj_create(plugin_tiledmap_manip_import_scene_t scene, const char * path) {
    plugin_tiledmap_manip_import_ctx_t ctx = scene->m_ctx;
    plugin_tiledmap_manip_import_tiled_proj_t proj;
    char full_path[128];
    struct cfg_it tileset_it;
    cfg_t tileset_cfg;
    struct cfg_it layer_it;
    cfg_t layer_cfg;
    
    proj = mem_calloc(ctx->m_alloc, sizeof(struct plugin_tiledmap_manip_import_tiled_proj));
    if (proj == NULL) {
        CPE_ERROR(ctx->m_em, "scene import from %s: load scene: create tiled proj %s: alloc fail!", ctx->m_proj_path, path);
        return NULL;
    }

    proj->m_scene = scene;
    cpe_str_dup(proj->m_path, sizeof(proj->m_path), path);
    TAILQ_INIT(&proj->m_layers);

    proj->m_proj_cfg = cfg_create(ctx->m_alloc);
    if (proj->m_proj_cfg == NULL) {
        CPE_ERROR(ctx->m_em, "scene import from %s: load scene: create tiled proj %s: alloc cfg fail!", ctx->m_proj_path, path);
        mem_free(ctx->m_alloc, proj);
        return NULL;
    }
    
    snprintf(full_path, sizeof(full_path), "%s/%s", ctx->m_proj_path, path);
    if (cfg_json_read_file(proj->m_proj_cfg, gd_app_vfs_mgr(ui_data_mgr_app(ctx->m_data_mgr)), full_path, cfg_replace, ctx->m_em) != 0) {
        CPE_ERROR(ctx->m_em, "scene import from %s: load scene: create tiled proj %s: read project fail!", ctx->m_proj_path, path);
        goto ERROR;
    }

    proj->m_tile_w = cfg_get_uint32(proj->m_proj_cfg, "tilewidth", 0);
    proj->m_tile_h = cfg_get_uint32(proj->m_proj_cfg, "tileheight", 0);
    if (proj->m_tile_w == 0 || proj->m_tile_h == 0) {
        CPE_ERROR(
            ctx->m_em, "scene import from %s: load scene: create tiled proj %s: tile size %dx%d error!",
            ctx->m_proj_path, path, proj->m_tile_w, proj->m_tile_h);
        goto ERROR;
    }

    cfg_it_init(&tileset_it, cfg_find_cfg(proj->m_proj_cfg, "tilesets"));
    while((tileset_cfg = cfg_it_next(&tileset_it))) {
        const char * image_path;
        plugin_tiledmap_manip_import_tiled_tileset_t tileset;

        image_path = cfg_get_string(tileset_cfg, "image", NULL);
        if (image_path == NULL) {
            CPE_ERROR(ctx->m_em, "scene import from %s: load scene: create tiled proj %s: tileset image not configured!", ctx->m_proj_path, path);
            goto ERROR;
        }
        
        tileset = plugin_tiledmap_manip_import_tiled_tileset_find(ctx, image_path);
        if (tileset == NULL) {
            tileset = plugin_tiledmap_manip_import_tiled_tileset_create(ctx, image_path, tileset_cfg);
            if (tileset == NULL) {
                CPE_ERROR(ctx->m_em, "scene import from %s: load scene: create tiled proj %s: create tileset %s fail!", ctx->m_proj_path, path, image_path);
                goto ERROR;
            }
        }

        if (tileset->m_tile_w != proj->m_tile_w || tileset->m_tile_h != proj->m_tile_h) {
            CPE_ERROR(
                ctx->m_em, "scene import from %s: load scene: create tiled proj %s: proj tile size %dx%d and tileset tile size %dx%d mismatch!",
                ctx->m_proj_path, path, proj->m_tile_w, proj->m_tile_h, tileset->m_tile_w, tileset->m_tile_h);
            goto ERROR;
        }

        proj->m_tilesets[proj->m_tileset_count].m_tileset = tileset;
        proj->m_tilesets[proj->m_tileset_count].m_start_id = cfg_get_uint32(tileset_cfg, "firstgid", 0);
        proj->m_tileset_count++;
    }
        
    cfg_it_init(&layer_it, cfg_find_cfg(proj->m_proj_cfg, "layers"));
    while((layer_cfg = cfg_it_next(&layer_it))) {
        if (plugin_tiledmap_manip_import_tiled_layer_create(proj, layer_cfg) == 0) {
            CPE_ERROR(ctx->m_em, "scene import from %s: load scene: create tiled proj %s: create layer fail!", ctx->m_proj_path, path);
            goto ERROR;
        }
    }
        
    return proj;

ERROR:
    while(!TAILQ_EMPTY(&proj->m_layers)) {
        plugin_tiledmap_manip_import_tiled_layer_free(TAILQ_FIRST(&proj->m_layers));
    }

    if(proj->m_proj_cfg) cfg_free(proj->m_proj_cfg);

    mem_free(ctx->m_alloc, proj);

    return NULL;
}

void plugin_tiledmap_manip_import_tiled_proj_free(plugin_tiledmap_manip_import_tiled_proj_t tiled_proj) {
    plugin_tiledmap_manip_import_ctx_t ctx = tiled_proj->m_scene->m_ctx;

    while(!TAILQ_EMPTY(&tiled_proj->m_layers)) {
        plugin_tiledmap_manip_import_tiled_layer_free(TAILQ_FIRST(&tiled_proj->m_layers));
    }

    cfg_free(tiled_proj->m_proj_cfg);

    mem_free(ctx->m_alloc, tiled_proj);
}

plugin_tiledmap_manip_import_tile_ref_t
plugin_tiledmap_manip_import_tiled_proj_find_tile_by_index(plugin_tiledmap_manip_import_tiled_proj_t proj, uint32_t index) {
    uint32_t i;

    for(i = 0; i < proj->m_tileset_count; ++i) {
        struct plugin_tiledmap_manip_import_tiled_proj_use_tileset * use_tileset = proj->m_tilesets + i;
        if (index >= use_tileset->m_start_id && (index - use_tileset->m_start_id) < use_tileset->m_tileset->m_tile_count) {
            return use_tileset->m_tileset->m_tile_refs_in_order[index - use_tileset->m_start_id];
        }
    }

    return NULL;
}

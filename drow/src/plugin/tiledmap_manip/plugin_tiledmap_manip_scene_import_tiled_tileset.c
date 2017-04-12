#include <assert.h>
#include "cpe/utils/string_utils.h"
#include "render/cache/ui_cache_pixel_buf_manip.h"
#include "plugin_tiledmap_manip_scene_import_tiled_tileset_i.h"
#include "plugin_tiledmap_manip_scene_import_tile_ref_i.h"
#include "plugin_tiledmap_manip_scene_import_utils_i.h"

plugin_tiledmap_manip_import_tiled_tileset_t
plugin_tiledmap_manip_import_tiled_tileset_create(
    plugin_tiledmap_manip_import_ctx_t ctx, const char * path, cfg_t cfg)
{
    plugin_tiledmap_manip_import_tiled_tileset_t tileset;
    const char * name;

    name = cfg_get_string(cfg, "name", NULL);
    if (name == NULL) {
        CPE_ERROR(ctx->m_em, "scene import from %s: create tileset %s: name not configured!", ctx->m_proj_path, path);
        return NULL;
    }

    tileset = mem_calloc(ctx->m_alloc, sizeof(struct plugin_tiledmap_manip_import_tiled_tileset));
    if (tileset == NULL) {
        CPE_ERROR(ctx->m_em, "scene import from %s: create tileset %s: alloc tileset fail!", ctx->m_proj_path, path);
        return NULL;
    }

    tileset->m_ctx = ctx;
    cpe_str_dup(tileset->m_path, sizeof(tileset->m_path), path);
    cpe_str_dup(tileset->m_name, sizeof(tileset->m_name), name);
    tileset->m_tile_w = cfg_get_uint32(cfg, "tileheight", 0);
    tileset->m_tile_h = cfg_get_uint32(cfg, "tileheight", 0);
    tileset->m_pixel_width = cfg_get_uint32(cfg, "imagewidth", 0);
    tileset->m_pixel_height = cfg_get_uint32(cfg, "imageheight", 0);
    TAILQ_INIT(&tileset->m_tile_refs);

    if (tileset->m_tile_w == 0 || tileset->m_tile_h == 0) {
        CPE_ERROR(
            ctx->m_em, "scene import from %s: create tileset %s: tile %dx%d size error!",
            ctx->m_proj_path, path, tileset->m_tile_w, tileset->m_tile_h);
        mem_free(ctx->m_alloc, tileset);
        return NULL;
    }

    if (tileset->m_pixel_width == 0 || tileset->m_pixel_height == 0) {
        CPE_ERROR(
            ctx->m_em, "scene import from %s: create tileset %s: pixel size %dx%d error!",
            ctx->m_proj_path, path, tileset->m_pixel_width, tileset->m_pixel_height);
        mem_free(ctx->m_alloc, tileset);
        return NULL;
    }

    tileset->m_row_count = tileset->m_pixel_height / tileset->m_tile_h;
    tileset->m_col_count = tileset->m_pixel_width / tileset->m_tile_w;
    tileset->m_tile_count = tileset->m_row_count * tileset->m_col_count;

    tileset->m_tile_refs_in_order = mem_calloc(ctx->m_alloc, sizeof(plugin_tiledmap_manip_import_tile_ref_t) * tileset->m_tile_count);
    if (tileset->m_tile_refs_in_order == NULL) {
        CPE_ERROR(
            ctx->m_em, "scene import from %s: build tilesets: tileset %s: alloc tile_refs_in_order fail!",
            ctx->m_proj_path, tileset->m_name);
        mem_free(ctx->m_alloc, tileset);
        return NULL;
    }

    TAILQ_INSERT_TAIL(&ctx->m_tilesets, tileset, m_next_for_ctx);

    return tileset;
}

plugin_tiledmap_manip_import_tiled_tileset_t
plugin_tiledmap_manip_import_tiled_tileset_find(
    plugin_tiledmap_manip_import_ctx_t ctx, const char * path)
{
    plugin_tiledmap_manip_import_tiled_tileset_t tileset;

    TAILQ_FOREACH(tileset, &ctx->m_tilesets, m_next_for_ctx) {
        if (strcmp(tileset->m_path, path) == 0) return tileset;
    }

    return NULL;
}

void plugin_tiledmap_manip_import_tiled_tileset_free(plugin_tiledmap_manip_import_tiled_tileset_t tiled_tileset) {
    plugin_tiledmap_manip_import_ctx_t ctx = tiled_tileset->m_ctx;
    
    while(!TAILQ_EMPTY(&tiled_tileset->m_tile_refs)) {
        plugin_tiledmap_manip_import_tile_ref_free(TAILQ_FIRST(&tiled_tileset->m_tile_refs));
    }

    if (tiled_tileset->m_pixel_buf) {
        ui_cache_pixel_buf_free(tiled_tileset->m_pixel_buf);
        tiled_tileset->m_pixel_buf = NULL;
    }

    if (tiled_tileset->m_tile_refs_in_order) {
        mem_free(ctx->m_alloc, tiled_tileset->m_tile_refs_in_order);
        tiled_tileset->m_tile_refs_in_order = NULL;
    }
    
    TAILQ_REMOVE(&ctx->m_tilesets, tiled_tileset, m_next_for_ctx);

    mem_free(ctx->m_alloc, tiled_tileset);
}

int plugin_tiledmap_manip_import_tiled_tileset_load(plugin_tiledmap_manip_import_tiled_tileset_t tileset) {
    plugin_tiledmap_manip_import_ctx_t ctx = tileset->m_ctx;
    ui_cache_pixel_level_info_t pixel_level_info;
    plugin_tiledmap_manip_import_tile_ref_t tile_ref;

    if (plugin_tiledmap_manip_import_load_layer(
        ctx, &tileset->m_pixel_buf, &tileset->m_tile_refs, tileset->m_path, tileset->m_name, tileset->m_tile_w, tileset->m_tile_h)
        != 0)
    {
        CPE_ERROR(
            ctx->m_em, "scene import from %s: load tileset %s: laod layer fail!",
            ctx->m_proj_path, tileset->m_name);
        return -1;
    }

    pixel_level_info = ui_cache_pixel_buf_level_info_at(tileset->m_pixel_buf, 0);
    assert(pixel_level_info);

    if (ui_cache_pixel_buf_level_width(pixel_level_info) != tileset->m_pixel_width
        || ui_cache_pixel_buf_level_height(pixel_level_info) != tileset->m_pixel_height)
    {
        CPE_ERROR(
            ctx->m_em, "scene import from %s: load tileset %s: pixel size error, expect %dx%d, but %dx%d!",
            ctx->m_proj_path, tileset->m_name, tileset->m_pixel_width, tileset->m_pixel_height,
            ui_cache_pixel_buf_level_width(pixel_level_info),
            ui_cache_pixel_buf_level_height(pixel_level_info));
        return -1;
    }

    TAILQ_FOREACH(tile_ref, &tileset->m_tile_refs, m_next_for_owner) {
        uint32_t col = tile_ref->m_x / tileset->m_tile_w;
        uint32_t row = tile_ref->m_y / tileset->m_tile_h;
        uint32_t index = row * tileset->m_col_count + col;

        if (index >= tileset->m_tile_count) {
            CPE_ERROR(
                ctx->m_em, "scene import from %s: build tilesets: tileset %s: role (%d,%d) overflow, tile_count=%d!",
                ctx->m_proj_path, tileset->m_name, row, col, tileset->m_tile_count);
            return -1;
        }

        tileset->m_tile_refs_in_order[index] = tile_ref;
    }
    
    return 0;
}

int plugin_tiledmap_manip_import_tiled_tileset_build(plugin_tiledmap_manip_import_ctx_t ctx) {
    ui_ed_src_t scene_src;
    ui_ed_obj_t scene_obj;
    plugin_tiledmap_manip_import_tiled_tileset_t tileset;

    if (TAILQ_EMPTY(&ctx->m_tilesets)) return 0;

    if (ctx->m_tileset_path[0] == 0) {
        CPE_ERROR(ctx->m_em, "scene import from %s: build tileset: tileset path not configured!", ctx->m_proj_path);
        return -1;
    }

    scene_src = ui_ed_src_check_create(ctx->m_ed_mgr, ctx->m_tileset_path, ui_data_src_type_tiledmap_scene);
    if (scene_src == NULL) {
        CPE_ERROR(ctx->m_em, "scene import from %s: build tileset: check create scene src at!", ctx->m_proj_path);
        return -1;
    }

    scene_obj = ui_ed_obj_only_child(ui_ed_src_root_obj(scene_src));
    assert(scene_obj);

    TAILQ_FOREACH(tileset, &ctx->m_tilesets, m_next_for_ctx) {
        if (plugin_tiledmap_manip_import_build_layer(
                ctx, scene_obj, tileset->m_name, &tileset->m_tile_refs, 0, 0, tileset->m_tile_w, tileset->m_tile_h))
        {
            CPE_ERROR(
                ctx->m_em, "scene import from %s: build tilesets: build tileset %s fail!",
                ctx->m_proj_path, tileset->m_name);
            return -1;
        }
    }

    return 0;
}

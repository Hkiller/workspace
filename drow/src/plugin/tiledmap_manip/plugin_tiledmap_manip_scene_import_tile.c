#include <assert.h>
#include "cpe/pal/pal_stdio.h"
#include "plugin_tiledmap_manip_scene_import_tile_i.h"
#include "plugin_tiledmap_manip_scene_import_tile_ref_i.h"

plugin_tiledmap_manip_import_tile_t
plugin_tiledmap_manip_import_tile_create(
    plugin_tiledmap_manip_import_ctx_t ctx, ui_cache_pixel_buf_t pixel_buf, ui_cache_pixel_buf_rect_t rect)
{
    plugin_tiledmap_manip_import_tile_t tile;

    tile = mem_calloc(ctx->m_alloc, sizeof(struct plugin_tiledmap_manip_import_tile));
    if (tile == NULL) {
        CPE_ERROR(ctx->m_em, "scene import from %s: create tile: alloc fail!", ctx->m_proj_path);
        return NULL; 
    }

    tile->m_ctx = ctx;
    TAILQ_INIT(&tile->m_refs);
    tile->m_ref_count = 0;
    tile->m_pixel_buf = pixel_buf;
    tile->m_rect = *rect;

    ctx->m_tile_count++;
    TAILQ_INSERT_TAIL(&ctx->m_tiles, tile, m_next_for_ctx);
    
    return tile;
}

void plugin_tiledmap_manip_import_tile_free(plugin_tiledmap_manip_import_tile_t tile) {
    plugin_tiledmap_manip_import_ctx_t ctx = tile->m_ctx;

    while(!TAILQ_EMPTY(&tile->m_refs)) {
        plugin_tiledmap_manip_import_tile_ref_free(TAILQ_FIRST(&tile->m_refs));
    }
    assert(tile->m_ref_count == 0);

    ctx->m_tile_count--;
    TAILQ_REMOVE(&ctx->m_tiles, tile, m_next_for_ctx);
    mem_free(ctx->m_alloc, tile);
}

int plugin_tiledmap_manip_import_tile_dump(plugin_tiledmap_manip_import_tile_t tile, uint8_t flip_type, uint8_t angle_type) {
    ui_cache_pixel_level_info_t level_info;
    struct ui_cache_pixel_buf_rect tmp_rect;
    char path_buf[128];
    ui_cache_pixel_buf_t pixel_buf;
    uint32_t width = tile->m_rect.boundary_rt - tile->m_rect.boundary_lt;
    uint32_t height = tile->m_rect.boundary_bm - tile->m_rect.boundary_tp;

    pixel_buf = ui_cache_pixel_buf_create(tile->m_ctx->m_cache_mgr);
    if (pixel_buf == NULL) return -1;
    
    if (ui_cache_pixel_buf_pixel_buf_create(pixel_buf, width, height, ui_cache_pf_r8g8b8a8, 1) != 0) {
        ui_cache_pixel_buf_free(pixel_buf);
        return -1;
    }

    level_info = ui_cache_pixel_buf_level_info_at(pixel_buf, 0);
    assert(level_info);

    if (ui_cache_pixel_buf_level_width(level_info) < width || ui_cache_pixel_buf_level_height(level_info) < height) {
        if (ui_cache_pixel_buf_pixel_buf_create(pixel_buf, width, height, ui_cache_pf_r8g8b8a8, 1) != 0) {
            ui_cache_pixel_buf_free(pixel_buf);
            return -1;
        }
    }

    tmp_rect.level = 0;
    tmp_rect.boundary_lt = 0;
    tmp_rect.boundary_tp = 0;
    tmp_rect.boundary_rt = width;
    tmp_rect.boundary_bm = height;

    if (ui_cache_pixel_buf_rect_op(
            pixel_buf, &tmp_rect, tile->m_pixel_buf, &tile->m_rect,
            (ui_cache_pixel_rect_flip_type_t)flip_type, (ui_cache_pixel_rect_angle_type_t)angle_type, tile->m_ctx->m_em)
        != 0)
    {
        ui_cache_pixel_buf_free(pixel_buf);
        return -1;
    }

    snprintf(
        path_buf, sizeof(path_buf), "%s/%d-%d_%d_%d.png", tile->m_ctx->m_proj_path,
        tile->m_rect.boundary_lt, tile->m_rect.boundary_tp, flip_type, angle_type);

    if (ui_cache_pixel_buf_save_to_file(pixel_buf, path_buf, tile->m_ctx->m_em, tile->m_ctx->m_alloc) != 0) {
        ui_cache_pixel_buf_free(pixel_buf);
        return -1;
    }
    
    ui_cache_pixel_buf_free(pixel_buf);
    return 0;
}
    

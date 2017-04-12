#ifndef UI_MODEL_MANIP_SCENE_IMPORT_TILE_H
#define UI_MODEL_MANIP_SCENE_IMPORT_TILE_H
#include "render/cache/ui_cache_pixel_buf_manip.h"
#include "plugin_tiledmap_manip_scene_import_layer_i.h"

struct plugin_tiledmap_manip_import_tile {
    plugin_tiledmap_manip_import_ctx_t m_ctx;
    TAILQ_ENTRY(plugin_tiledmap_manip_import_tile) m_next_for_ctx;
    plugin_tiledmap_manip_import_tile_ref_list_t m_refs;
    uint32_t m_ref_count;
    ui_cache_pixel_buf_t m_pixel_buf;
    struct ui_cache_pixel_buf_rect m_rect;
    uint32_t m_img_block_id;
};

plugin_tiledmap_manip_import_tile_t
plugin_tiledmap_manip_import_tile_create(
    plugin_tiledmap_manip_import_ctx_t ctx, ui_cache_pixel_buf_t pixel_buf, ui_cache_pixel_buf_rect_t rect);

void plugin_tiledmap_manip_import_tile_free(plugin_tiledmap_manip_import_tile_t tile);

int plugin_tiledmap_manip_import_tile_dump(plugin_tiledmap_manip_import_tile_t tile, uint8_t flip_type, uint8_t angle_type);

int plugin_tiledmap_manip_import_tile_build_module(plugin_tiledmap_manip_import_ctx_t ctx);

#endif

#ifndef UI_MODEL_MANIP_SCENE_IMPORT_TILED_TILESET_H
#define UI_MODEL_MANIP_SCENE_IMPORT_TILED_TILESET_H
#include "plugin_tiledmap_manip_scene_import_tiled_proj_i.h"

struct plugin_tiledmap_manip_import_tiled_tileset {
    plugin_tiledmap_manip_import_ctx_t  m_ctx;
    TAILQ_ENTRY(plugin_tiledmap_manip_import_tiled_tileset) m_next_for_ctx;
    char m_path[128];
    char m_name[64];    
    uint16_t m_tile_w;
    uint16_t m_tile_h;
    uint32_t m_pixel_width;
    uint32_t m_pixel_height;
    uint16_t m_row_count;
    uint16_t m_col_count;
    uint32_t m_tile_count;
    plugin_tiledmap_manip_import_tile_ref_t * m_tile_refs_in_order;
    ui_cache_pixel_buf_t m_pixel_buf;
    plugin_tiledmap_manip_import_tile_ref_list_t m_tile_refs;
};

plugin_tiledmap_manip_import_tiled_tileset_t
plugin_tiledmap_manip_import_tiled_tileset_create(
    plugin_tiledmap_manip_import_ctx_t ctx, const char * path, cfg_t cfg);

plugin_tiledmap_manip_import_tiled_tileset_t
plugin_tiledmap_manip_import_tiled_tileset_find(
    plugin_tiledmap_manip_import_ctx_t ctx, const char * path);

void plugin_tiledmap_manip_import_tiled_tileset_free(plugin_tiledmap_manip_import_tiled_tileset_t tiled_tileset);

int plugin_tiledmap_manip_import_tiled_tileset_load(plugin_tiledmap_manip_import_tiled_tileset_t tileset);

int plugin_tiledmap_manip_import_tiled_tileset_build(plugin_tiledmap_manip_import_ctx_t ctx);

#endif

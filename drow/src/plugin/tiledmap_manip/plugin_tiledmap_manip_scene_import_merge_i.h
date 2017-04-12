#ifndef UI_MODEL_MANIP_SCENE_IMPORT_TILE_MERGE_H
#define UI_MODEL_MANIP_SCENE_IMPORT_TILE_MERGE_H
#include "cpe/utils/hash.h"
#include "cpe/utils/md5.h"
#include "plugin_tiledmap_manip_scene_import_tile_i.h"

struct plugin_tiledmap_manip_import_merge_ctx {
    plugin_tiledmap_manip_import_ctx_t m_import_ctx;
    struct cpe_hash_table m_tiles;
    ui_cache_pixel_buf_t m_tmp_pixel_buf;
};

struct plugin_tiledmap_manip_import_merge_tile {
    plugin_tiledmap_manip_import_merge_ctx_t m_ctx;
    plugin_tiledmap_manip_import_tile_t m_tile;
    struct cpe_md5_value m_md5;
    struct cpe_hash_entry m_hh;
};

int plugin_tiledmap_manip_import_merge(plugin_tiledmap_manip_import_ctx_t ctx);

int plugin_tiledmap_manip_import_merge_ctx_init(plugin_tiledmap_manip_import_merge_ctx_t merge_ctx, plugin_tiledmap_manip_import_ctx_t import_ctx);
void plugin_tiledmap_manip_import_merge_ctx_fini(plugin_tiledmap_manip_import_merge_ctx_t merge_ctx);

plugin_tiledmap_manip_import_merge_tile_t
plugin_tiledmap_manip_import_merge_tile_create(
    plugin_tiledmap_manip_import_merge_ctx_t merge_ctx, cpe_md5_value_t md5, plugin_tiledmap_manip_import_tile_t tile);

plugin_tiledmap_manip_import_merge_tile_t
plugin_tiledmap_manip_import_merge_tile_find(plugin_tiledmap_manip_import_merge_ctx_t merge_ctx, cpe_md5_value_t md5);

void plugin_tiledmap_manip_import_merge_tile_free(plugin_tiledmap_manip_import_merge_tile_t merge_tile);

#endif

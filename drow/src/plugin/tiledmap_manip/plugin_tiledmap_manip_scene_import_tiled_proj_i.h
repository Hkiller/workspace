#ifndef UI_MODEL_MANIP_SCENE_IMPORT_TILED_PROJ_H
#define UI_MODEL_MANIP_SCENE_IMPORT_TILED_PROJ_H
#include "plugin_tiledmap_manip_scene_import_scene_i.h"

struct plugin_tiledmap_manip_import_tiled_proj_use_tileset {
    plugin_tiledmap_manip_import_tiled_tileset_t m_tileset;
    uint32_t m_start_id;
};

struct plugin_tiledmap_manip_import_tiled_proj {
    plugin_tiledmap_manip_import_scene_t m_scene;
    char m_path[128];
    cfg_t m_proj_cfg;
    uint32_t m_tile_w;
    uint32_t m_tile_h;
    uint8_t m_tileset_count;
    struct plugin_tiledmap_manip_import_tiled_proj_use_tileset m_tilesets[64];
    plugin_tiledmap_manip_import_tiled_layer_list_t m_layers;
};

plugin_tiledmap_manip_import_tiled_proj_t
plugin_tiledmap_manip_import_tiled_proj_create(plugin_tiledmap_manip_import_scene_t scene, const char * path);
void plugin_tiledmap_manip_import_tiled_proj_free(plugin_tiledmap_manip_import_tiled_proj_t tiled_proj);

plugin_tiledmap_manip_import_tile_ref_t
plugin_tiledmap_manip_import_tiled_proj_find_tile_by_index(plugin_tiledmap_manip_import_tiled_proj_t proj, uint32_t index);

#endif

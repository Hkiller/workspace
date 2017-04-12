#ifndef UI_MODEL_MANIP_SCENE_IMPORT_TILED_LAYER_H
#define UI_MODEL_MANIP_SCENE_IMPORT_TILED_LAYER_H
#include "plugin_tiledmap_manip_scene_import_tiled_proj_i.h"

struct plugin_tiledmap_manip_import_tiled_layer {
    plugin_tiledmap_manip_import_tiled_proj_t m_proj;
    TAILQ_ENTRY(plugin_tiledmap_manip_import_tiled_layer) m_next;
    char m_name[64];
    uint32_t m_col_count;
    uint32_t m_row_count;
    cfg_t m_cfg;
    plugin_tiledmap_manip_import_tile_ref_list_t m_tile_refs;
};

plugin_tiledmap_manip_import_tiled_layer_t
plugin_tiledmap_manip_import_tiled_layer_create(plugin_tiledmap_manip_import_tiled_proj_t proj, cfg_t cfg);
void plugin_tiledmap_manip_import_tiled_layer_free(plugin_tiledmap_manip_import_tiled_layer_t tiled_layer);

int plugin_tiledmap_manip_import_tiled_layer_load(plugin_tiledmap_manip_import_tiled_layer_t layer);

#endif

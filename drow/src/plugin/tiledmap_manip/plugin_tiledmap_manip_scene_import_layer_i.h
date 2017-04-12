#ifndef UI_MODEL_MANIP_SCENE_IMPORT_LAYER_H
#define UI_MODEL_MANIP_SCENE_IMPORT_LAYER_H
#include "plugin_tiledmap_manip_scene_import_scene_i.h"

struct plugin_tiledmap_manip_import_layer {
    plugin_tiledmap_manip_import_scene_t m_scene;
    TAILQ_ENTRY(plugin_tiledmap_manip_import_layer) m_next;
    char m_layer_name[64];
    char m_input_path[128];
    uint16_t m_tile_w;
    uint16_t m_tile_h;
    ui_cache_pixel_buf_t m_pixel_buf;
    plugin_tiledmap_manip_import_tile_ref_list_t m_tile_refs;
};

plugin_tiledmap_manip_import_layer_t
plugin_tiledmap_manip_import_layer_create(plugin_tiledmap_manip_import_scene_t scene, cfg_t cfg);
void plugin_tiledmap_manip_import_layer_free(plugin_tiledmap_manip_import_layer_t layer);

int plugin_tiledmap_manip_import_layer_load(plugin_tiledmap_manip_import_layer_t layer);

#endif

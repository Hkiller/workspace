#ifndef UI_MODEL_MANIP_SCENE_EXPORT_LAYER_H
#define UI_MODEL_MANIP_SCENE_EXPORT_LAYER_H
#include "plugin_tiledmap_manip_scene_export_scene_i.h"

struct plugin_tiledmap_scene_export_layer {
    plugin_tiledmap_scene_export_scene_t m_scene;
    TAILQ_ENTRY(plugin_tiledmap_scene_export_layer) m_next_for_scene;
    char m_layer_name[64];
    char m_png_name[128];
    plugin_tiledmap_data_layer_t m_data_layer;
};

plugin_tiledmap_scene_export_layer_t
plugin_tiledmap_scene_export_layer_create(plugin_tiledmap_scene_export_scene_t scene, cfg_t cfg);
void plugin_tiledmap_scene_export_layer_free(plugin_tiledmap_scene_export_layer_t layer);

int plugin_tiledmap_scene_export_layer_do_export(plugin_tiledmap_scene_export_layer_t layer);

#endif

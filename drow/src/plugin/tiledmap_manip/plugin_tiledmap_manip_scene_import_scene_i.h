#ifndef UI_MODEL_MANIP_SCENE_IMPORT_SCENE_H
#define UI_MODEL_MANIP_SCENE_IMPORT_SCENE_H
#include "plugin_tiledmap_manip_scene_import_ctx_i.h"

typedef enum plugin_tiledmap_manip_import_position {
    plugin_tiledmap_manip_import_bottom_left = 1
    , plugin_tiledmap_manip_import_bottom_right
    , plugin_tiledmap_manip_import_top_left
    , plugin_tiledmap_manip_import_top_right
} plugin_tiledmap_manip_import_position_t;

struct plugin_tiledmap_manip_import_scene {
    plugin_tiledmap_manip_import_ctx_t m_ctx;
    TAILQ_ENTRY(plugin_tiledmap_manip_import_scene) m_next;
    char m_path[128];
    plugin_tiledmap_manip_import_position_t m_position;
    uint32_t m_width;
    uint32_t m_height;
    plugin_tiledmap_manip_import_layer_list_t m_layers;
    plugin_tiledmap_manip_import_tiled_proj_t m_tiled_proj;
};

plugin_tiledmap_manip_import_scene_t
plugin_tiledmap_manip_import_scene_create(plugin_tiledmap_manip_import_ctx_t ctx, cfg_t cfg);
void plugin_tiledmap_manip_import_scene_free(plugin_tiledmap_manip_import_scene_t scene);

int plugin_tiledmap_manip_import_scene_build_tiledmap(plugin_tiledmap_manip_import_scene_t scene);

#endif

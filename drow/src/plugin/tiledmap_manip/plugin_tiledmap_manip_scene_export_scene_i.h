#ifndef UI_MODEL_MANIP_SCENE_EXPORT_SCENE_H
#define UI_MODEL_MANIP_SCENE_EXPORT_SCENE_H
#include "render/utils/ui_rect.h"
#include "plugin_tiledmap_manip_scene_export_ctx_i.h"

typedef enum plugin_tiledmap_manip_export_position {
    plugin_tiledmap_manip_export_bottom_left = 1
    , plugin_tiledmap_manip_export_bottom_right
    , plugin_tiledmap_manip_export_top_left
    , plugin_tiledmap_manip_export_top_right
} plugin_tiledmap_manip_export_position_t;

struct plugin_tiledmap_scene_export_scene {
    plugin_tiledmap_scene_export_ctx_t m_ctx;
    TAILQ_ENTRY(plugin_tiledmap_scene_export_scene) m_next_for_ctx;

    char m_scene_path[128];
    plugin_tiledmap_data_scene_t m_data_scene;
    ui_rect m_scene_rect;
    plugin_tiledmap_manip_export_position_t m_scene_export_position;
    plugin_tiledmap_scene_export_layer_list_t m_layers;
};

plugin_tiledmap_scene_export_scene_t
plugin_tiledmap_scene_export_scene_create(plugin_tiledmap_scene_export_ctx_t ctx, cfg_t cfg);
void plugin_tiledmap_scene_export_scene_free(plugin_tiledmap_scene_export_scene_t scene);

int plugin_tiledmap_scene_export_scene_do_export(plugin_tiledmap_scene_export_scene_t scene);

#endif

#ifndef PLUGIN_TILEDMAP_RENDER_OBJ_I_H
#define PLUGIN_TILEDMAP_RENDER_OBJ_I_H
#include "plugin/tiledmap/plugin_tiledmap_render_obj.h"
#include "plugin_tiledmap_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_tiledmap_render_obj {
    plugin_tiledmap_module_t m_module;
    plugin_tiledmap_data_scene_t m_data_scene;
};

int plugin_tiledmap_render_obj_regist(plugin_tiledmap_module_t module);
void plugin_tiledmap_render_obj_unregist(plugin_tiledmap_module_t module);

#ifdef __cplusplus
}
#endif

#endif

#ifndef PLUGIN_TILEDMAP_RENDER_OBJ_H
#define PLUGIN_TILEDMAP_RENDER_OBJ_H
#include "plugin_tiledmap_types.h"

#ifdef __cplusplus
extern "C" {
#endif

plugin_tiledmap_data_scene_t plugin_tiledmap_render_obj_data(plugin_tiledmap_render_obj_t obj);
int plugin_tiledmap_render_obj_set_data(plugin_tiledmap_render_obj_t obj, plugin_tiledmap_data_scene_t data_scene);
    
#ifdef __cplusplus
}
#endif

#endif

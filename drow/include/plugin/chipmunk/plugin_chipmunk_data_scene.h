#ifndef DROW_PLUGIN_CHIPMUNK_DATA_SCENE_H
#define DROW_PLUGIN_CHIPMUNK_DATA_SCENE_H
#include "plugin_chipmunk_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*scene */
plugin_chipmunk_data_scene_t plugin_chipmunk_data_scene_create(plugin_chipmunk_module_t module, ui_data_src_t src);
void plugin_chipmunk_data_scene_free(plugin_chipmunk_data_scene_t sprite);

CHIPMUNK_SCENE * plugin_chipmunk_data_scene_data(plugin_chipmunk_data_scene_t scene);

uint32_t plugin_chipmunk_data_scene_body_count(plugin_chipmunk_data_scene_t scene);
void plugin_chipmunk_data_scene_bodies(plugin_chipmunk_data_body_it_t body_it, plugin_chipmunk_data_scene_t scene);

uint32_t plugin_chipmunk_data_scene_constraint_count(plugin_chipmunk_data_scene_t scene);
void plugin_chipmunk_data_scene_constraints(plugin_chipmunk_data_constraint_it_t constraint_it, plugin_chipmunk_data_scene_t scene);
    
#ifdef __cplusplus
}
#endif

#endif

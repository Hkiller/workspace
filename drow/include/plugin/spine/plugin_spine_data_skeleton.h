#ifndef PLUGIN_SPINE_DATA_SKELETON_H
#define PLUGIN_SPINE_DATA_SKELETON_H
#include "plugin_spine_types.h"
#include "spine/spine.h"

#ifdef __cplusplus
extern "C" {
#endif

/*spine*/
plugin_spine_data_skeleton_t plugin_spine_data_skeleton_create(plugin_spine_module_t mgr, ui_data_src_t src);
void plugin_spine_data_skeleton_free(plugin_spine_data_skeleton_t spine);

spAtlas * plugin_spine_data_skeleton_atlas(plugin_spine_data_skeleton_t spine);
spSkeletonData * plugin_spine_data_skeleton_data(plugin_spine_data_skeleton_t skeleton_data);
void plugin_spine_data_skeleton_set_data(plugin_spine_data_skeleton_t spine, spSkeletonData * skeleton_data, spAtlas * atlas);

#ifdef __cplusplus
}
#endif

#endif

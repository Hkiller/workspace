#ifndef PLUGIN_SWF_DATA_H
#define PLUGIN_SWF_DATA_H
#include "plugin_swf_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*swf*/
plugin_swf_data_t plugin_swf_data_create(plugin_swf_module_t mgr, ui_data_src_t src);
void plugin_swf_data_free(plugin_swf_data_t swf);

/* spAtlas * plugin_swf_data_atlas(plugin_swf_data_t swf); */
/* spSkeletonData * plugin_swf_data_data(plugin_swf_data_t skeleton_data); */
/* void plugin_swf_data_set_data(plugin_swf_data_t swf, spSkeletonData * skeleton_data, spAtlas * atlas); */

#ifdef __cplusplus
}
#endif

#endif

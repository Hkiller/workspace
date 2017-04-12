#ifndef PLUGIN_SPINE_DATA_SKELETON_I_H
#define PLUGIN_SPINE_DATA_SKELETON_I_H
#include "spine/SkeletonData.h"
#include "plugin/spine/plugin_spine_data_skeleton.h"
#include "plugin_spine_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_spine_data_skeleton {
    plugin_spine_module_t m_module;
    ui_data_src_t m_src;
    spAtlas * m_atlas;
    spSkeletonData * m_skeleton_data;
};

/*int plugin_spine_data_skeleton_bin_save(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, const char * root, error_monitor_t em);*/
int plugin_spine_data_skeleton_bin_load(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, error_monitor_t em);

int plugin_spine_data_skeleton_update_usings(ui_data_src_t src);
    
#ifdef __cplusplus
}
#endif

#endif

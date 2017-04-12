#ifndef PLUGIN_CHIPMUNK_DATA_SCENE_I_H
#define PLUGIN_CHIPMUNK_DATA_SCENE_I_H
#include "render/model/ui_data_mgr.h"
#include "render/model/ui_data_src.h"
#include "plugin/chipmunk/plugin_chipmunk_data_scene.h"
#include "plugin_chipmunk_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_chipmunk_data_scene {
    plugin_chipmunk_module_t m_module;
    ui_data_src_t m_src;
    CHIPMUNK_SCENE m_data;
    uint32_t m_body_count;
    plugin_chipmunk_data_body_list_t m_body_list;
    uint32_t m_constraint_count;
    plugin_chipmunk_data_constraint_list_t m_constraint_list;
};

#ifdef __cplusplus
}
#endif

#endif

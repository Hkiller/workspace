#ifndef PLUGIN_SPINE_OBJ_ANIM_GROUP_I_H
#define PLUGIN_SPINE_OBJ_ANIM_GROUP_I_H
#include "plugin/spine/plugin_spine_obj_anim_group.h"
#include "plugin_spine_obj_anim_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_spine_obj_anim_group {
    plugin_spine_module_t m_module;
    plugin_spine_obj_anim_group_binding_list_t m_anims;    
};
    
#ifdef __cplusplus
}
#endif

#endif

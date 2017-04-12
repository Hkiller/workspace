#ifndef PLUGIN_SPINE_OBJ_ANIM_GROUP_BINDING_I_H
#define PLUGIN_SPINE_OBJ_ANIM_GROUP_BINDING_I_H
#include "plugin_spine_obj_anim_group_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_spine_obj_anim_group_binding {
    plugin_spine_obj_anim_group_t m_group;
    TAILQ_ENTRY(plugin_spine_obj_anim_group_binding) m_next_for_group;
    plugin_spine_obj_anim_t m_anim;
    TAILQ_ENTRY(plugin_spine_obj_anim_group_binding) m_next_for_anim;
};

plugin_spine_obj_anim_group_binding_t
plugin_spine_obj_anim_group_binding_create(plugin_spine_obj_anim_group_t group, plugin_spine_obj_anim_t anim);
void plugin_spine_obj_anim_group_binding_free(plugin_spine_obj_anim_group_binding_t binding);

void plugin_spine_obj_anim_group_binding_real_free_all(plugin_spine_module_t module);

#ifdef __cplusplus
}
#endif

#endif

#ifndef PLUGIN_SPINE_OBJ_ANIM_GROUP_H
#define PLUGIN_SPINE_OBJ_ANIM_GROUP_H
#include "render/model/ui_model_types.h"
#include "spine/AnimationState.h"
#include "plugin_spine_types.h"

#ifdef __cplusplus
extern "C" {
#endif
        
plugin_spine_obj_anim_group_t
plugin_spine_obj_anim_group_create(plugin_spine_module_t module);
void plugin_spine_obj_anim_group_free(plugin_spine_obj_anim_group_t anim_group);

int plugin_spine_obj_anim_group_add_anim(plugin_spine_obj_anim_group_t group, plugin_spine_obj_anim_t anim);
int plugin_spine_obj_anim_group_remove_anim(plugin_spine_obj_anim_group_t group, plugin_spine_obj_anim_t anim);

int plugin_spine_obj_anim_group_add_track_listener(plugin_spine_obj_anim_group_t anim_group, plugin_spine_anim_event_fun_t fun, void * ctx);
void plugin_spine_obj_anim_group_remove_track_listener(plugin_spine_obj_anim_group_t anim_group, void * ctx);

void plugin_spine_obj_anim_group_free_all_anims(plugin_spine_obj_anim_group_t anim_group);
uint8_t plugin_spine_obj_anim_group_is_playing(plugin_spine_obj_anim_group_t anim_group);
    
#ifdef __cplusplus
}
#endif

#endif

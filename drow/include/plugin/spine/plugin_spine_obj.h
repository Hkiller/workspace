#ifndef PLUGIN_SPINE_OBJ_H
#define PLUGIN_SPINE_OBJ_H
#include "render/model/ui_model_types.h"
#include "plugin_spine_types.h"

#ifdef __cplusplus
extern "C" {
#endif
    
plugin_spine_module_t  plugin_spine_obj_module(plugin_spine_obj_t obj);

void plugin_spine_obj_set_mix(plugin_spine_obj_t obj, const char * from_animation, const char * to_animation, float duration);

int plugin_spine_obj_set_data(plugin_spine_obj_t obj, plugin_spine_data_skeleton_t data);
    
uint8_t plugin_spine_obj_debug_slots(plugin_spine_obj_t obj);
void plugin_spine_obj_set_debug_slots(plugin_spine_obj_t obj, uint8_t debug_slots);

uint8_t plugin_spine_obj_debug_bones(plugin_spine_obj_t obj);
void plugin_spine_obj_set_debug_bones(plugin_spine_obj_t obj, uint8_t debug_bones);

int plugin_spine_obj_modify(plugin_spine_obj_t obj, const char * modification);
int plugin_spine_obj_modify_replace_atlas(plugin_spine_obj_t obj, const char * name, const char * model_url);

int plugin_spine_obj_set_ik_by_name(plugin_spine_obj_t obj, const char * ik_name, float x, float y);
    
struct spSkeleton * plugin_spine_obj_skeleton(plugin_spine_obj_t obj);
struct spBone* plugin_spine_obj_find_bone_by_name(plugin_spine_obj_t obj, const char * name);
    
int plugin_spine_obj_apply_anim(plugin_spine_obj_t obj, const char * anim_name);

int plugin_spine_obj_scane_parts(plugin_spine_obj_t obj);
    
#ifdef __cplusplus
}
#endif

#endif

#ifndef PLUGIN_SPINE_OBJ_PART_H
#define PLUGIN_SPINE_OBJ_PART_H
#include "plugin_spine_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*part*/    
plugin_spine_obj_part_t plugin_spine_obj_part_create(plugin_spine_obj_t obj, const char * name);
void plugin_spine_obj_part_free(plugin_spine_obj_part_t part);

plugin_spine_obj_part_t plugin_spine_obj_part_find(plugin_spine_obj_t obj, const char * name);

plugin_spine_obj_t  plugin_spine_obj_part_obj(plugin_spine_obj_part_t part);
const char * plugin_spine_obj_part_name(plugin_spine_obj_part_t part);
plugin_spine_obj_track_t plugin_spine_obj_part_track(plugin_spine_obj_part_t part);

uint8_t plugin_spine_obj_part_state_count(plugin_spine_obj_part_t part);
plugin_spine_obj_part_state_t plugin_spine_obj_part_cur_state(plugin_spine_obj_part_t part);
int plugin_spine_obj_part_set_cur_state(plugin_spine_obj_part_t part, plugin_spine_obj_part_state_t state);
int plugin_spine_obj_part_set_cur_state_by_name(plugin_spine_obj_part_t part, const char * name);
int plugin_spine_obj_part_apply_transition(plugin_spine_obj_part_t part, plugin_spine_obj_part_transition_t transition);
int plugin_spine_obj_part_apply_transition_by_name(plugin_spine_obj_part_t part, const char * transition_name);
int plugin_spine_obj_part_apply_transition_force(plugin_spine_obj_part_t part, const char * name);

int plugin_spine_obj_part_switch_or_set_to_state(plugin_spine_obj_part_t part, plugin_spine_obj_part_state_t to_state, uint8_t force_change);
int plugin_spine_obj_part_switch_or_set_to_state_by_name(plugin_spine_obj_part_t part, const char * name, uint8_t force_change);

uint8_t plugin_spine_obj_part_is_in_enter(plugin_spine_obj_part_t part);

plugin_spine_obj_anim_t plugin_spine_obj_part_state_anim(plugin_spine_obj_part_t part);
plugin_spine_obj_anim_t plugin_spine_obj_part_enter_anim(plugin_spine_obj_part_t part);    
    
int plugin_spine_obj_build_parts(plugin_spine_obj_t obj, plugin_spine_data_state_def_t state_def);


/*part_state*/
plugin_spine_obj_part_state_t plugin_spine_obj_part_state_create(plugin_spine_obj_part_t part, const char * name);
void plugin_spine_obj_part_state_free(plugin_spine_obj_part_state_t state);

plugin_spine_obj_part_state_t plugin_spine_obj_part_state_find(plugin_spine_obj_part_t part, const char * name);
int plugin_spine_obj_part_state_set_anim(plugin_spine_obj_part_state_t state, const char * anim_name);

/*part_transition*/
plugin_spine_obj_part_transition_t
plugin_spine_obj_part_transition_create(plugin_spine_obj_part_state_t from, plugin_spine_obj_part_state_t to, const char * name);
void plugin_spine_obj_part_transition_free(plugin_spine_obj_part_transition_t transition);

plugin_spine_obj_part_transition_t plugin_spine_obj_part_transition_find(plugin_spine_obj_part_state_t from, const char * name);
plugin_spine_obj_part_transition_t plugin_spine_obj_part_transition_find_by_target(plugin_spine_obj_part_state_t from, const char * target_state);

int plugin_spine_obj_part_transition_set_anim(plugin_spine_obj_part_transition_t transition, const char * anim_name);
int plugin_spine_obj_part_transition_add_anim_by_def(plugin_spine_obj_part_transition_t transition, const char * def);

const char * plugin_spine_obj_part_transition_name(plugin_spine_obj_part_transition_t transition);

#ifdef __cplusplus
}
#endif

#endif

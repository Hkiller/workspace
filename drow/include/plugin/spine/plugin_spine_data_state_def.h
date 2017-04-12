#ifndef PLUGIN_SPINE_DATA_STATE_DEF_H
#define PLUGIN_SPINE_DATA_STATE_DEF_H
#include "protocol/plugin/spine/spine_state_def.h"
#include "plugin_spine_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*spine*/
plugin_spine_data_state_def_t plugin_spine_data_state_def_create(plugin_spine_module_t mgr, ui_data_src_t src);
void plugin_spine_data_state_def_free(plugin_spine_data_state_def_t state_def);

/*part*/
plugin_spine_data_part_t plugin_spine_data_part_create(plugin_spine_data_state_def_t state_def);
void plugin_spine_data_part_free(plugin_spine_data_part_t part);
plugin_spine_data_part_t plugin_spine_data_part_find(plugin_spine_data_state_def_t state_def, const char * part_name);
SPINE_PART * plugin_spine_data_part_data(plugin_spine_data_part_t part);
const char * plugin_spine_data_part_name(plugin_spine_data_part_t part);
const char * plugin_spine_data_part_init_state(plugin_spine_data_part_t part);
const char * plugin_spine_data_part_init_anim(plugin_spine_data_part_t part);
    
/*state*/
plugin_spine_data_part_state_t plugin_spine_data_part_state_create(plugin_spine_data_part_t part);
void plugin_spine_data_part_state_free(plugin_spine_data_part_state_t state);
SPINE_PART_STATE * plugin_spine_data_part_state_data(plugin_spine_data_part_state_t state);
const char * plugin_spine_data_part_state_name(plugin_spine_data_part_state_t state);    
const char * plugin_spine_data_part_state_anim(plugin_spine_data_part_state_t state);    

/*transition*/
plugin_spine_data_part_transition_t plugin_spine_data_part_transition_create(plugin_spine_data_part_t part);
void plugin_spine_data_part_transition_free(plugin_spine_data_part_transition_t transition);
SPINE_PART_TRANSITION * plugin_spine_data_part_transition_data(plugin_spine_data_part_transition_t transition);
const char * plugin_spine_data_part_transition_from(plugin_spine_data_part_transition_t transition);
const char * plugin_spine_data_part_transition_to(plugin_spine_data_part_transition_t transition);    
const char * plugin_spine_data_part_transition_name(plugin_spine_data_part_transition_t transition);    
const char * plugin_spine_data_part_transition_anim(plugin_spine_data_part_transition_t transition);    

#ifdef __cplusplus
}
#endif

#endif

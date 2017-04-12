#ifndef PLUGIN_SPINE_OBJ_PART_STATE_H
#define PLUGIN_SPINE_OBJ_PART_STATE_H
#include "plugin_spine_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_spine_obj_part_state_it {
    plugin_spine_obj_part_state_t (*next)(struct plugin_spine_obj_part_state_it * it);
    char m_data[64];
};
    
plugin_spine_obj_part_state_t plugin_spine_obj_part_state_create(plugin_spine_obj_part_t part, const char * name);
void plugin_spine_obj_part_state_free(plugin_spine_obj_part_state_t state);
plugin_spine_obj_part_state_t plugin_spine_obj_part_state_find(plugin_spine_obj_part_t part, const char * name);
 
plugin_spine_obj_part_t  plugin_spine_obj_part_state_part(plugin_spine_obj_part_state_t state);
const char * plugin_spine_obj_part_state_name(plugin_spine_obj_part_state_t state);

void plugin_spine_obj_part_states(plugin_spine_obj_part_t part, plugin_spine_obj_part_state_it_t it);

#define plugin_spine_obj_part_state_it_next(it) ((it)->next ? (it)->next(it) : NULL)

#ifdef __cplusplus
}
#endif

#endif

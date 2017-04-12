#ifndef UI_PLUGIN_PARTICLE_OBJ_H
#define UI_PLUGIN_PARTICLE_OBJ_H
#include "plugin_particle_types.h"

#ifdef __cplusplus
extern "C" {
#endif

plugin_particle_data_t plugin_particle_obj_data(plugin_particle_obj_t obj);
int plugin_particle_obj_set_data(plugin_particle_obj_t obj, plugin_particle_data_t particle_data);
    
uint8_t plugin_particle_obj_is_enable(plugin_particle_obj_t obj);
void plugin_particle_obj_set_enable(plugin_particle_obj_t obj, uint8_t enable);
    
uint32_t plugin_particle_obj_particle_count(plugin_particle_obj_t obj);
uint32_t plugin_particle_obj_active_emitter_count(plugin_particle_obj_t obj);
    
#ifdef __cplusplus
}
#endif

#endif


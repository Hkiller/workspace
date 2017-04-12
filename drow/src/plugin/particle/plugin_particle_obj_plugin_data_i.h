#ifndef UI_PLUGIN_PARTICLE_OBJ_PLUGIN_DATA_I_H
#define UI_PLUGIN_PARTICLE_OBJ_PLUGIN_DATA_I_H
#include "plugin/particle/plugin_particle_obj_plugin_data.h"
#include "plugin_particle_obj_plugin_i.h"

#ifdef __cplusplus
extern "C" {
#endif
        
struct plugin_particle_obj_plugin_data {
    plugin_particle_obj_plugin_t m_plugin;
    TAILQ_ENTRY(plugin_particle_obj_plugin_data) m_next_for_plugin;
    plugin_particle_obj_particle_t m_particle;
    TAILQ_ENTRY(plugin_particle_obj_plugin_data) m_next_for_particle;
};

void plugin_particle_obj_plugin_data_real_free(
    plugin_particle_obj_plugin_t plugin, plugin_particle_obj_plugin_data_t plugin_data);
    
#ifdef __cplusplus
}
#endif

#endif

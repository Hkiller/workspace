#ifndef UI_PLUGIN_PARTICLE_OBJ_EMITTER_DATA_I_H
#define UI_PLUGIN_PARTICLE_OBJ_EMITTER_DATA_I_H
#include "plugin_particle_obj_emitter_i.h"

#ifdef __cplusplus
extern "C" {
#endif
        
struct plugin_particle_obj_emitter_data {
    plugin_particle_obj_emitter_t m_emitter;
    union {
        TAILQ_ENTRY(plugin_particle_obj_emitter_data) m_next;
        UI_PARTICLE_EMITTER m_data;
    };
};

plugin_particle_obj_emitter_data_t
plugin_particle_obj_emitter_data_create(plugin_particle_obj_emitter_t emitter);
void plugin_particle_obj_emitter_data_free(plugin_particle_obj_emitter_data_t data);
    
void plugin_particle_obj_emitter_data_real_free(
    plugin_particle_module_t module, plugin_particle_obj_emitter_data_t data);

#ifdef __cplusplus
}
#endif

#endif

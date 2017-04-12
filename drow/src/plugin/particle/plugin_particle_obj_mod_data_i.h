#ifndef UI_PLUGIN_PARTICLE_OBJ_PARTICLE_DATA_I_H
#define UI_PLUGIN_PARTICLE_OBJ_PARTICLE_DATA_I_H
#include "plugin_particle_obj_particle_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_particle_obj_mod_data {
    plugin_particle_obj_emitter_t m_emitter;
    TAILQ_ENTRY(plugin_particle_obj_mod_data) m_next;
    union {
        struct {
            float m_emitter_angle_theta;
        } m_uber_circle;
        struct {
            float m_emitter_angle_theta;
        } m_uber_ellipse;
    };
};

plugin_particle_obj_mod_data_t
plugin_particle_obj_mod_data_create(plugin_particle_obj_emitter_t emitter);
void plugin_particle_obj_mod_data_free(plugin_particle_obj_mod_data_t data);
    
void plugin_particle_obj_mod_data_real_free(
    plugin_particle_module_t module, plugin_particle_obj_mod_data_t data);
    
#ifdef __cplusplus
}
#endif

#endif

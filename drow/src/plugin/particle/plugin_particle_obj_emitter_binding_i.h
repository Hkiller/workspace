#ifndef UI_PLUGIN_PARTICLE_OBJ_EMITTER_BINDING_I_H
#define UI_PLUGIN_PARTICLE_OBJ_EMITTER_BINDING_I_H
#include "plugin_particle_obj_emitter_i.h"
#include "plugin_particle_obj_particle_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_particle_obj_emitter_binding {
    plugin_particle_obj_emitter_t m_emitter;
    TAILQ_ENTRY(plugin_particle_obj_emitter_binding) m_next_for_emitter;
    
    plugin_particle_obj_particle_t m_particle;
    TAILQ_ENTRY(plugin_particle_obj_emitter_binding) m_next_for_particle;

    struct plugin_particle_obj_emitter_runtime m_runtime;
    uint8_t m_is_tie;
    uint8_t m_accept_scale;
    uint8_t m_accept_angle;
};

plugin_particle_obj_emitter_binding_t
plugin_particle_obj_emitter_binding_create(plugin_particle_obj_emitter_t emitter, plugin_particle_obj_particle_t particle);

void plugin_particle_obj_emitter_binding_free(plugin_particle_obj_emitter_binding_t binding);

void plugin_particle_obj_emitter_binding_real_free(plugin_particle_module_t module, plugin_particle_obj_emitter_binding_t binding);    

#ifdef __cplusplus
}
#endif

#endif

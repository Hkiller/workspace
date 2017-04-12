#ifndef UI_PLUGIN_PARTICLE_OBJ_EMITTER_MOD_I_H
#define UI_PLUGIN_PARTICLE_OBJ_EMITTER_MOD_I_H
#include "plugin_particle_obj_emitter_i.h"

#ifdef __cplusplus
extern "C" {
#endif
        
struct plugin_particle_obj_emitter_mod {
    plugin_particle_obj_emitter_t m_emitter;
    TAILQ_ENTRY(plugin_particle_obj_emitter_mod) m_next;
    UI_PARTICLE_MOD m_data;
};

plugin_particle_obj_emitter_mod_t
plugin_particle_obj_emitter_mod_create(plugin_particle_obj_emitter_t emitter);
plugin_particle_obj_emitter_mod_t
plugin_particle_obj_emitter_mod_clone(plugin_particle_obj_emitter_t emitter, plugin_particle_obj_emitter_mod_t o);
void plugin_particle_obj_emitter_mod_free(plugin_particle_obj_emitter_mod_t mod);
    
void plugin_particle_obj_emitter_mod_real_free(
    plugin_particle_module_t module, plugin_particle_obj_emitter_mod_t mod);

#ifdef __cplusplus
}
#endif

#endif

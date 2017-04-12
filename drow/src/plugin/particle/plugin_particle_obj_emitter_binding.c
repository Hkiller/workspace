#include "plugin_particle_obj_emitter_binding_i.h"
#include "plugin_particle_obj_emitter_i.h"
#include "plugin_particle_obj_particle_i.h"

plugin_particle_obj_emitter_binding_t
plugin_particle_obj_emitter_binding_create(plugin_particle_obj_emitter_t emitter, plugin_particle_obj_particle_t particle) {
    plugin_particle_module_t module = emitter->m_obj->m_module;
    plugin_particle_obj_emitter_binding_t binding;

    binding = TAILQ_FIRST(&module->m_free_emitter_bindings);
    if (binding) {
        TAILQ_REMOVE(&module->m_free_emitter_bindings, binding, m_next_for_emitter);
    }
    else {
        binding = mem_alloc(module->m_alloc, sizeof(struct plugin_particle_obj_emitter_binding));
        if (binding == NULL) {
            CPE_ERROR(module->m_em, "plugin_particle_obj_emitter_binding_create: alloc fail!");
            return NULL;
        }
    }

    binding->m_emitter = emitter;
    binding->m_particle = particle;
    binding->m_is_tie = 0;
    binding->m_accept_scale = 0;
    binding->m_accept_angle = 1;
    
    TAILQ_INSERT_TAIL(&emitter->m_binding_particles, binding, m_next_for_emitter);
    TAILQ_INSERT_TAIL(&particle->m_binding_emitters, binding, m_next_for_particle);

    plugin_particle_obj_emitter_runtime_init(&binding->m_runtime);

    return binding;
}

void plugin_particle_obj_emitter_binding_free(plugin_particle_obj_emitter_binding_t binding) {
    plugin_particle_module_t module = binding->m_emitter->m_obj->m_module;

    TAILQ_REMOVE(&binding->m_emitter->m_binding_particles, binding, m_next_for_emitter);
    TAILQ_REMOVE(&binding->m_particle->m_binding_emitters, binding, m_next_for_particle);

    TAILQ_INSERT_TAIL(&module->m_free_emitter_bindings, binding, m_next_for_emitter);
}

void plugin_particle_obj_emitter_binding_real_free(plugin_particle_module_t module, plugin_particle_obj_emitter_binding_t binding) {
    TAILQ_REMOVE(&module->m_free_emitter_bindings, binding, m_next_for_emitter);
    mem_free(module->m_alloc, binding);
}



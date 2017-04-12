#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "render/model/ui_data_src.h"
#include "plugin_particle_data_i.h"

plugin_particle_data_mod_t plugin_particle_data_mod_create(plugin_particle_data_emitter_t emitter) {
    plugin_particle_module_t module = emitter->m_particle->m_module;
    plugin_particle_data_mod_t mod;

    mod = mem_alloc(module->m_alloc, sizeof(struct plugin_particle_data_mod));
    if (mod == NULL) {
        CPE_ERROR(
            module->m_em, "create particle mod in particle %s: alloc fail !",
            ui_data_src_path_dump(&module->m_dump_buffer, emitter->m_particle->m_src));
        return NULL;
    }

    mod->m_emitter = emitter;
    bzero(&mod->m_data, sizeof(mod->m_data));

    emitter->m_mod_count++;
    TAILQ_INSERT_TAIL(&emitter->m_mods, mod, m_next_for_emitter);

    return mod;
}

void plugin_particle_data_mod_free(plugin_particle_data_mod_t mod) {
    plugin_particle_data_emitter_t emitter = mod->m_emitter;
    plugin_particle_module_t module = emitter->m_particle->m_module;

    emitter->m_mod_count--;
    TAILQ_REMOVE(&emitter->m_mods, mod, m_next_for_emitter);

    mem_free(module->m_alloc, mod);
}

static plugin_particle_data_mod_t plugin_particle_data_mod_in_emitter_next(struct plugin_particle_data_mod_it * it) {
    plugin_particle_data_mod_t * data = (plugin_particle_data_mod_t *)(it->m_data);
    plugin_particle_data_mod_t r;

    if (*data == NULL) return NULL;

    r = *data;

    *data = TAILQ_NEXT(r, m_next_for_emitter);

    return r;
}

void plugin_particle_data_emitter_mods(plugin_particle_data_mod_it_t it, plugin_particle_data_emitter_t emitter) {
    *(plugin_particle_data_mod_t *)(it->m_data) = TAILQ_FIRST(&emitter->m_mods);
    it->next = plugin_particle_data_mod_in_emitter_next;
}

UI_PARTICLE_MOD * plugin_particle_data_mod_data(plugin_particle_data_mod_t particle_mod) {
    return &particle_mod->m_data;
}

LPDRMETA plugin_particle_data_mod_meta(plugin_particle_module_t module) {
    return module->m_meta_particle_mod;
}

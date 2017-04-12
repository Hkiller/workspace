#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "render/model/ui_data_src.h"
#include "plugin_particle_data_i.h"

plugin_particle_data_emitter_t plugin_particle_data_emitter_create(plugin_particle_data_t particle) {
    plugin_particle_module_t module = particle->m_module;
    plugin_particle_data_emitter_t emitter;

    emitter = mem_alloc(module->m_alloc, sizeof(struct plugin_particle_data_emitter));
    if (emitter == NULL) {
        CPE_ERROR(
            module->m_em, "create img in particle %s: alloc fail !",
            ui_data_src_path_dump(&module->m_dump_buffer, particle->m_src));
        return NULL;
    }

    emitter->m_particle = particle;
    bzero(&emitter->m_data, sizeof(emitter->m_data));

    emitter->m_mod_count = 0;
    TAILQ_INIT(&emitter->m_mods);

    particle->m_emitter_count++;
    TAILQ_INSERT_TAIL(&particle->m_emitters, emitter, m_next_for_particle);

    return emitter;
}

void plugin_particle_data_emitter_free(plugin_particle_data_emitter_t emitter) {
    plugin_particle_data_t particle = emitter->m_particle;
    plugin_particle_module_t module = particle->m_module;

    while(!TAILQ_EMPTY(&emitter->m_mods)) {
        plugin_particle_data_mod_free(TAILQ_FIRST(&emitter->m_mods));
    }
    assert(emitter->m_mod_count == 0);

    particle->m_emitter_count--;
    TAILQ_REMOVE(&particle->m_emitters, emitter, m_next_for_particle);

    mem_free(module->m_alloc, emitter);
}

plugin_particle_data_emitter_t
plugin_particle_data_emitter_find(plugin_particle_data_t particle, const char * emitter_name) {
    plugin_particle_data_emitter_t emitter;

    TAILQ_FOREACH(emitter, &particle->m_emitters, m_next_for_particle) {
        if (strcmp(plugin_particle_data_emitter_msg(emitter, emitter->m_data.name_id), emitter_name) == 0) return emitter;
    }

    return NULL;
}

uint32_t plugin_particle_data_emitter_mod_count(plugin_particle_data_emitter_t emitter) {
    return emitter->m_mod_count;
}

static plugin_particle_data_emitter_t plugin_particle_data_emitter_in_particle_next(struct plugin_particle_data_emitter_it * it) {
    plugin_particle_data_emitter_t * data = (plugin_particle_data_emitter_t *)(it->m_data);
    plugin_particle_data_emitter_t r;

    if (*data == NULL) return NULL;

    r = *data;

    *data = TAILQ_NEXT(r, m_next_for_particle);

    return r;
}

void plugin_particle_data_emitters(plugin_particle_data_emitter_it_t it, plugin_particle_data_t particle) {
    *(plugin_particle_data_emitter_t *)(it->m_data) = TAILQ_FIRST(&particle->m_emitters);
    it->next = plugin_particle_data_emitter_in_particle_next;
}

UI_PARTICLE_EMITTER * plugin_particle_data_emitter_data(plugin_particle_data_emitter_t particle_emitter) {
    return &particle_emitter->m_data;
}

LPDRMETA plugin_particle_data_emitter_meta(plugin_particle_module_t module) {
    return module->m_meta_particle_emitter;
}

const char * plugin_particle_data_emitter_msg(plugin_particle_data_emitter_t emitter, uint32_t msg_id) {
    return ui_data_src_msg(emitter->m_particle->m_src, msg_id);
}

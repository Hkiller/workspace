#include "plugin_particle_obj_plugin_i.h"
#include "plugin_particle_obj_plugin_data_i.h"
#include "plugin_particle_obj_particle_i.h"

plugin_particle_obj_plugin_t
plugin_particle_obj_plugin_create(
    plugin_particle_obj_emitter_t emitter,
    void * ctx, uint32_t data_capacity,
    plugin_particle_obj_plugin_init_fun_t init_fun,
    plugin_particle_obj_plugin_fini_fun_t fini_fun,
    plugin_particle_obj_plugin_update_fun_t update_fun)
{
    plugin_particle_module_t module = emitter->m_obj->m_module;
    plugin_particle_obj_plugin_t plugin;

    plugin = TAILQ_FIRST(&module->m_free_plugins);
    if (plugin == NULL) {
        plugin = mem_alloc(module->m_alloc, sizeof(struct plugin_particle_obj_plugin));
        if (plugin == NULL) {
            CPE_ERROR(module->m_em, "plugin_particle_obj_plugin_create: alloc fail!");
            return NULL;
        }
    }
    else {
        TAILQ_REMOVE(&module->m_free_plugins, plugin, m_next);
    }

    plugin->m_emitter = emitter;
    plugin->m_ctx = ctx;
    plugin->m_data_capacity = data_capacity;
    plugin->m_init_fun = init_fun;
    plugin->m_fini_fun = fini_fun;
    plugin->m_update_fun = update_fun;
    TAILQ_INIT(&plugin->m_datas);
    TAILQ_INIT(&plugin->m_free_datas);

    TAILQ_INSERT_TAIL(&emitter->m_plugins, plugin, m_next);

    if (!TAILQ_EMPTY(&emitter->m_particles)) {
        plugin_particle_obj_particle_t particle;
        TAILQ_FOREACH(particle, &emitter->m_particles, m_next) {
            plugin_particle_obj_plugin_data_create(plugin, particle);
        }
    }

    return plugin;
}

void plugin_particle_obj_plugin_free(plugin_particle_obj_plugin_t plugin) {
    plugin_particle_obj_emitter_t emitter = plugin->m_emitter;
    plugin_particle_module_t module = emitter->m_obj->m_module;

    while(!TAILQ_EMPTY(&plugin->m_datas)) {
        plugin_particle_obj_plugin_data_free(TAILQ_FIRST(&plugin->m_datas));
    }

    while(!TAILQ_EMPTY(&plugin->m_free_datas)) {
        plugin_particle_obj_plugin_data_real_free(plugin, TAILQ_FIRST(&plugin->m_free_datas));
    }
    
    TAILQ_REMOVE(&emitter->m_plugins, plugin, m_next);

    TAILQ_INSERT_TAIL(&module->m_free_plugins, plugin, m_next);
}

void plugin_particle_obj_plugin_real_free(plugin_particle_module_t module, plugin_particle_obj_plugin_t plugin) {
    TAILQ_REMOVE(&module->m_free_plugins, plugin, m_next);
    mem_free(module->m_alloc, plugin);
}

plugin_particle_obj_emitter_t plugin_particle_obj_plugin_emitter(plugin_particle_obj_plugin_t plugin) {
    return plugin->m_emitter;
}

plugin_particle_obj_plugin_t
plugin_particle_obj_plugin_find_by_ctx(plugin_particle_obj_emitter_t emitter, void * ctx) {
    plugin_particle_obj_plugin_t plugin;

    TAILQ_FOREACH(plugin, &emitter->m_plugins, m_next) {
        if (plugin->m_ctx == ctx) return plugin;
    }

    return NULL;
}
    
void * plugin_particle_obj_plugin_ctx(plugin_particle_obj_plugin_t plugin) {
    return plugin->m_ctx;
}

static plugin_particle_obj_plugin_t plugin_particle_obj_plugin_next(struct plugin_particle_obj_plugin_it * it) {
    plugin_particle_obj_plugin_t * data = (plugin_particle_obj_plugin_t *)(it->m_data);
    plugin_particle_obj_plugin_t r;

    if (*data == NULL) return NULL;

    r = *data;

    *data = TAILQ_NEXT(r, m_next);

    return r;
}

void plugin_particle_obj_plugins(plugin_particle_obj_plugin_it_t plugin_it, plugin_particle_obj_emitter_t emitter) {
    *(plugin_particle_obj_plugin_t *)(plugin_it->m_data) = TAILQ_FIRST(&emitter->m_plugins);
    plugin_it->next = plugin_particle_obj_plugin_next;
}

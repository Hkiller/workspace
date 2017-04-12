#include "plugin_particle_obj_plugin_data_i.h"
#include "plugin_particle_obj_particle_i.h"

plugin_particle_obj_plugin_data_t
plugin_particle_obj_plugin_data_create(
    plugin_particle_obj_plugin_t plugin, plugin_particle_obj_particle_t particle)
{
    plugin_particle_module_t module = plugin->m_emitter->m_obj->m_module;
    plugin_particle_obj_plugin_data_t plugin_data;

    plugin_data = TAILQ_FIRST(&plugin->m_free_datas);
    if (plugin_data == NULL) {
        plugin_data = mem_alloc(module->m_alloc, sizeof(struct plugin_particle_obj_plugin_data) + plugin->m_data_capacity);
        if (plugin_data == NULL) {
            CPE_ERROR(module->m_em, "plugin_particle_obj_plugin_data_create: alloc fail!");
            return NULL;
        }
    }
    else {
        TAILQ_REMOVE(&plugin->m_free_datas, plugin_data, m_next_for_plugin);
    }

    plugin_data->m_plugin = plugin;
    TAILQ_INSERT_TAIL(&plugin->m_datas, plugin_data, m_next_for_plugin);
    plugin_data->m_particle = particle;
    TAILQ_INSERT_TAIL(&particle->m_plugin_datas, plugin_data, m_next_for_particle);

    if (plugin->m_init_fun) {
        if (plugin->m_init_fun(plugin->m_ctx, plugin_data) != 0) {
            TAILQ_REMOVE(&plugin->m_datas, plugin_data, m_next_for_plugin);
            TAILQ_REMOVE(&particle->m_plugin_datas, plugin_data, m_next_for_particle);
            TAILQ_INSERT_TAIL(&plugin->m_free_datas, plugin_data, m_next_for_plugin);
            return NULL;
        }
    }
    
    return plugin_data;
}

void plugin_particle_obj_plugin_data_free(plugin_particle_obj_plugin_data_t plugin_data) {
    plugin_particle_obj_plugin_t plugin = plugin_data->m_plugin;
    
    if (plugin->m_fini_fun) {
        plugin->m_fini_fun(plugin->m_ctx, plugin_data);
    }
    
    TAILQ_REMOVE(&plugin_data->m_plugin->m_datas, plugin_data, m_next_for_plugin);
    TAILQ_REMOVE(&plugin_data->m_particle->m_plugin_datas, plugin_data, m_next_for_particle);
    
    TAILQ_INSERT_TAIL(&plugin_data->m_plugin->m_free_datas, plugin_data, m_next_for_plugin);
}

plugin_particle_obj_plugin_data_t
plugin_particle_obj_plugin_data_find_by_ctx(plugin_particle_obj_particle_t particle, void * ctx) {
    plugin_particle_obj_plugin_data_t data;

    TAILQ_FOREACH(data, &particle->m_plugin_datas, m_next_for_particle) {
        if (data->m_plugin->m_ctx == ctx) return data;
    }
    
    return NULL;
}

void plugin_particle_obj_plugin_data_real_free(plugin_particle_obj_plugin_t plugin, plugin_particle_obj_plugin_data_t plugin_data) {
    TAILQ_REMOVE(&plugin->m_free_datas, plugin_data, m_next_for_plugin);
    mem_free(plugin->m_emitter->m_obj->m_module->m_alloc, plugin_data);
}

plugin_particle_obj_t plugin_particle_obj_plugin_data_obj(plugin_particle_obj_plugin_data_t plugin_data) {
    return plugin_data->m_plugin->m_emitter->m_obj;
}

plugin_particle_obj_plugin_t plugin_particle_obj_plugin_data_plugin(plugin_particle_obj_plugin_data_t plugin_data) {
    return plugin_data->m_plugin;
}

plugin_particle_obj_particle_t plugin_particle_obj_plugin_data_particle(plugin_particle_obj_plugin_data_t plugin_data) {
    return plugin_data->m_particle;
}

void * plugin_particle_obj_plugin_data_data(plugin_particle_obj_plugin_data_t plugin_data) {
    return plugin_data + 1;
}

plugin_particle_obj_plugin_data_t plugin_particle_obj_plugin_data_from_data(void * data) {
    return ((plugin_particle_obj_plugin_data_t)data) - 1;
}

#include <assert.h>
#include "gd/app/app_log.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dr/dr_data.h"
#include "plugin_particle_obj_emitter_data_i.h"

plugin_particle_obj_emitter_data_t
plugin_particle_obj_emitter_data_create(plugin_particle_obj_emitter_t emitter) {
    plugin_particle_module_t dataule = emitter->m_obj->m_module;
    plugin_particle_obj_emitter_data_t data;

    assert(emitter->m_runtime_data == NULL);
    
    data = TAILQ_FIRST(&dataule->m_free_emitter_datas);
    if (data == NULL) {
        data = mem_alloc(dataule->m_alloc, sizeof(struct plugin_particle_obj_emitter_data));
        if (data == NULL) {
            CPE_ERROR(dataule->m_em, "plugin_particle_obj_emitter_data_create: alloc fail!");
            return NULL;
        }
    }
    else {
        TAILQ_REMOVE(&dataule->m_free_emitter_datas, data, m_next);
    }

    data->m_emitter = emitter;

    emitter->m_runtime_data = data;

    return data;
}

void plugin_particle_obj_emitter_data_free(plugin_particle_obj_emitter_data_t data) {
    plugin_particle_obj_emitter_t emitter = data->m_emitter;
    plugin_particle_module_t module = emitter->m_obj->m_module;

    assert(emitter->m_runtime_data == data);

    emitter->m_runtime_data = NULL;
    
    TAILQ_INSERT_TAIL(&module->m_free_emitter_datas, data, m_next);
}

void plugin_particle_obj_emitter_data_real_free(plugin_particle_module_t module, plugin_particle_obj_emitter_data_t data) {
    TAILQ_REMOVE(&module->m_free_emitter_datas, data, m_next);
    mem_free(module->m_alloc, data);
}

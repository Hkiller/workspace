#include <assert.h>
#include "plugin_barrage_data_i.h"

plugin_barrage_data_t plugin_barrage_data_create(plugin_barrage_env_t env, dr_data_t src) {
    plugin_barrage_data_t data;

    data = (plugin_barrage_data_t)mem_alloc(env->m_module->m_alloc, sizeof(struct plugin_barrage_data) + src->m_size);
    if (data == NULL) return NULL;

    data->m_ref_count = 1;
    data->m_data.m_meta = src->m_meta;
    data->m_data.m_data = (data + 1);
    data->m_data.m_size = src->m_size;
    memcpy(data->m_data.m_data, src->m_data, src->m_size);
    
    env->m_data_count++;

    return data;
}

void plugin_barrage_data_free(plugin_barrage_env_t env, plugin_barrage_data_t data) {
    assert(data->m_ref_count > 0);

    --data->m_ref_count;

    if (data->m_ref_count > 0) return;

    env->m_data_count--;

    mem_free(env->m_module->m_alloc, data);
}

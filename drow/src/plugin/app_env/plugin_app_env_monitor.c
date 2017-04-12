#include "plugin_app_env_monitor_i.h"

plugin_app_env_monitor_t
plugin_app_env_monitor_create(
    plugin_app_env_module_t module, const char * meta_name,
    void * ctx, plugin_app_env_monitor_exec_fun_t fun, plugin_app_env_monitor_clear_fun_t clear_fun)
{
    plugin_app_env_monitor_t monitor;
    size_t name_len = strlen(meta_name) + 1;
    
    monitor = mem_alloc(module->m_alloc, sizeof(struct plugin_app_env_monitor) + name_len);
    if (monitor == NULL) {
        CPE_ERROR(module->m_em, "plugin_app_env monitor alloc fail!");
        return NULL;
    }

    monitor->m_module = module;
    monitor->m_meta_name = (void*)(monitor + 1);
    monitor->m_ctx = ctx;
    monitor->m_fun = fun;
    monitor->m_clear_fun = clear_fun;
    
    memcpy((void*)monitor->m_meta_name, meta_name, name_len);
    
    cpe_hash_entry_init(&monitor->m_hh);
    if (cpe_hash_table_insert(&module->m_monitors, monitor) != 0) {
        CPE_ERROR(module->m_em, "plugin_app_env monitor %s insert fail!", meta_name);
        mem_free(module->m_alloc, monitor);
        return NULL;
    }

    return monitor;
}

void plugin_app_env_monitor_free(plugin_app_env_monitor_t monitor) {
    plugin_app_env_module_t module = monitor->m_module;

    if (monitor->m_clear_fun) {
        monitor->m_clear_fun(monitor->m_ctx);
    }
    
    cpe_hash_table_remove_by_ins(&module->m_monitors, monitor);

    mem_free(module->m_alloc, monitor);
}

void plugin_app_env_monitor_free_by_ctx(plugin_app_env_module_t module, void * ctx) {
    struct cpe_hash_it monitor_it;
    plugin_app_env_monitor_t monitor;

    cpe_hash_it_init(&monitor_it, &module->m_monitors);

    monitor = cpe_hash_it_next(&monitor_it);
    while (monitor) {
        plugin_app_env_monitor_t next = cpe_hash_it_next(&monitor_it);

        if (monitor->m_ctx == ctx) {
            plugin_app_env_monitor_free(monitor);
        }
        
        monitor = next;
    }
}

void plugin_app_env_monitor_free_all(plugin_app_env_module_t module) {
    struct cpe_hash_it monitor_it;
    plugin_app_env_monitor_t monitor;

    cpe_hash_it_init(&monitor_it, &module->m_monitors);

    monitor = cpe_hash_it_next(&monitor_it);
    while (monitor) {
        plugin_app_env_monitor_t next = cpe_hash_it_next(&monitor_it);
        plugin_app_env_monitor_free(monitor);
        monitor = next;
    }
}

uint32_t plugin_app_env_monitor_hash(plugin_app_env_monitor_t monitor) {
    return cpe_hash_str(monitor->m_meta_name, strlen(monitor->m_meta_name));
}

int plugin_app_env_monitor_eq(plugin_app_env_monitor_t l, plugin_app_env_monitor_t r) {
    return strcmp(l->m_meta_name, r->m_meta_name) == 0 ? 1 : 0;
}

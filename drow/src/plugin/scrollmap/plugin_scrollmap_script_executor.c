#include "plugin_scrollmap_script_executor_i.h"

plugin_scrollmap_script_executor_t
plugin_scrollmap_script_executor_create(
    plugin_scrollmap_env_t env, const char * type,
    uint32_t data_capacity, plugin_scrollmap_script_executor_fun_t fun)
{
    plugin_scrollmap_module_t module = env->m_module;
    plugin_scrollmap_script_executor_t script_executor;
    size_t type_len = strlen(type) + 1;
    char * type_name_buf;
    
    script_executor = mem_alloc(module->m_alloc, sizeof(struct plugin_scrollmap_script_executor) + data_capacity + type_len);
    if (script_executor == NULL) {
        CPE_ERROR(module->m_em, "scrollmap_script_executor_create: alloc fail!");
        return NULL;
    }

    type_name_buf = ((char *)(script_executor + 1)) + data_capacity;
    memcpy(type_name_buf, type, type_len);

    script_executor->m_env = env;
    script_executor->m_type = type_name_buf;
    script_executor->m_exec_fun = fun;
    
    cpe_hash_entry_init(&script_executor->m_hh);
    if (cpe_hash_table_insert(&env->m_script_executors, script_executor) != 0) {
        CPE_ERROR(module->m_em, "scrollmap_script_executor_create: script_executor %s duplicate!", type);
        mem_free(module->m_alloc, script_executor);
        return NULL;
    }

    return script_executor;
}
    
plugin_scrollmap_script_executor_t
plugin_scrollmap_script_executor_find(
    plugin_scrollmap_env_t env, const char * type)
{
    struct plugin_scrollmap_script_executor key;
    key.m_type = type;
    return cpe_hash_table_find(&env->m_script_executors, &key);
}
    
void plugin_scrollmap_script_executor_free(plugin_scrollmap_script_executor_t executor) {
    plugin_scrollmap_env_t env = executor->m_env;

    cpe_hash_table_remove_by_ins(&env->m_script_executors, executor);

    mem_free(env->m_module->m_alloc, executor);
}

void plugin_scrollmap_script_executor_free_all(plugin_scrollmap_env_t env) {
    struct cpe_hash_it executor_it;
    plugin_scrollmap_script_executor_t executor;

    cpe_hash_it_init(&executor_it, &env->m_script_executors);

    executor = (plugin_scrollmap_script_executor_t)cpe_hash_it_next(&executor_it);
    while (executor) {
        plugin_scrollmap_script_executor_t next = (plugin_scrollmap_script_executor_t)cpe_hash_it_next(&executor_it);
        plugin_scrollmap_script_executor_free(executor);
        executor = next;
    }
}

void * plugin_scrollmap_script_executor_data(plugin_scrollmap_script_executor_t executor) {
    return executor + 1;
}

uint32_t plugin_scrollmap_script_executor_hash(plugin_scrollmap_script_executor_t script_executor) {
    return cpe_hash_str(script_executor->m_type, strlen(script_executor->m_type));
}

int plugin_scrollmap_script_executor_eq(plugin_scrollmap_script_executor_t l, plugin_scrollmap_script_executor_t r) {
    return strcmp(l->m_type, r->m_type) == 0;
}

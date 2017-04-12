#include "cpe/dr/dr_metalib_manage.h"
#include "plugin_app_env_executor_i.h"

static plugin_app_env_executor_t plugin_app_env_executor_create_i(plugin_app_env_module_t module, LPDRMETA apply_to, plugin_app_env_executor_type_t type) {
    plugin_app_env_executor_t executor;

    executor = mem_alloc(module->m_alloc, sizeof(struct plugin_app_env_executor));
    if (executor == NULL) {
        CPE_ERROR(module->m_em, "plugin_app_env executor alloc fail!");
        return NULL;
    }

    executor->m_module = module;
    executor->m_type = type;
    executor->m_apply_to = apply_to;
        
    cpe_hash_entry_init(&executor->m_hh);
    if (cpe_hash_table_insert(&module->m_executors, executor) != 0) {
        CPE_ERROR(module->m_em, "plugin_app_env executor %s insert fail!", dr_meta_name(apply_to));
        mem_free(module->m_alloc, executor);
        return NULL;
    }

    return executor;
}

plugin_app_env_executor_t plugin_app_env_executor_create_oneway(plugin_app_env_module_t module, LPDRMETA apply_to, void * ctx, plugin_app_env_executor_oneway_exec_fun_t fun) {
    plugin_app_env_executor_t executor = plugin_app_env_executor_create_i(module, apply_to, plugin_app_env_executor_oneway);
    if (executor == NULL) return NULL;

    executor->m_ctx = ctx;
    executor->m_exec_oneway = fun;

    return executor;
}

plugin_app_env_executor_t
plugin_app_env_executor_create_sync(plugin_app_env_module_t module, LPDRMETA apply_to, void * ctx, plugin_app_env_executor_sync_exec_fun_t fun) {
    plugin_app_env_executor_t executor = plugin_app_env_executor_create_i(module, apply_to, plugin_app_env_executor_sync);
    if (executor == NULL) return NULL;

    executor->m_ctx = ctx;
    executor->m_exec_sync = fun;

    return executor;
}

plugin_app_env_executor_t plugin_app_env_executor_create_async(plugin_app_env_module_t module, LPDRMETA apply_to, void * ctx, plugin_app_env_executor_async_exec_fun_t fun) {
    plugin_app_env_executor_t executor = plugin_app_env_executor_create_i(module, apply_to, plugin_app_env_executor_async);
    if (executor == NULL) return executor;

    executor->m_ctx = ctx;
    executor->m_exec_async = fun;

    return executor;
}

void plugin_app_env_executor_free(plugin_app_env_executor_t executor) {
    plugin_app_env_module_t module = executor->m_module;

    cpe_hash_table_remove_by_ins(&module->m_executors, executor);

    mem_free(module->m_alloc, executor);
}

void plugin_app_env_executor_free_by_ctx(plugin_app_env_module_t module, void * ctx) {
    struct cpe_hash_it executor_it;
    plugin_app_env_executor_t executor;

    cpe_hash_it_init(&executor_it, &module->m_executors);

    executor = cpe_hash_it_next(&executor_it);
    while (executor) {
        plugin_app_env_executor_t next = cpe_hash_it_next(&executor_it);

        if (executor->m_ctx == ctx) {
            plugin_app_env_executor_free(executor);
        }
        
        executor = next;
    }
}

void plugin_app_env_executor_free_all(plugin_app_env_module_t module) {
    struct cpe_hash_it executor_it;
    plugin_app_env_executor_t executor;

    cpe_hash_it_init(&executor_it, &module->m_executors);

    executor = cpe_hash_it_next(&executor_it);
    while (executor) {
        plugin_app_env_executor_t next = cpe_hash_it_next(&executor_it);
        plugin_app_env_executor_free(executor);
        executor = next;
    }
}

plugin_app_env_executor_t plugin_app_env_executor_find(plugin_app_env_module_t module, LPDRMETA req_meta) {
    struct plugin_app_env_executor key;
    key.m_apply_to = req_meta;
    return cpe_hash_table_find(&module->m_executors, &key);
}

uint32_t plugin_app_env_executor_hash(plugin_app_env_executor_t executor) {
    const char * name = dr_meta_name(executor->m_apply_to);
    return cpe_hash_str(name, strlen(name));
}

int plugin_app_env_executor_eq(plugin_app_env_executor_t l, plugin_app_env_executor_t r) {
    return l->m_apply_to == r->m_apply_to ? 1 : 0;
}

int plugin_app_env_executor_bulck_create(
    plugin_app_env_module_t module, LPDRMETALIB metalib, void * ctx, plugin_app_env_executor_def_t defs, uint8_t def_count)
{
    plugin_app_env_executor_t created[128];
    uint8_t i;

    if (def_count > CPE_ARRAY_SIZE(created)) {
        CPE_ERROR(module->m_em, "plugin_app_env: bulck create executor: def_count %d overflow!!", def_count);
        return -1;
    }

    for(i = 0; i < def_count; ++i) {
        plugin_app_env_executor_def_t def = defs + i;
        LPDRMETA meta;

        meta = dr_lib_find_meta_by_name(metalib, def->meta);
        if (meta == NULL) {
            CPE_ERROR(module->m_em, "plugin_app_env: bulck create executor: meta %s not exist!", def->meta);
            goto BULCK_CREATE_ERROR;
        }
        
        switch(def->type) {
        case plugin_app_env_executor_oneway:
            created[i] = plugin_app_env_executor_create_oneway(module, meta, ctx, def->fun.oneway);
            break;
        case plugin_app_env_executor_sync:
            created[i] = plugin_app_env_executor_create_sync(module, meta, ctx, def->fun.sync);
            break;
        case plugin_app_env_executor_async:
            created[i] = plugin_app_env_executor_create_async(module, meta, ctx, def->fun.async);
            break;
        default:
            CPE_ERROR(module->m_em, "plugin_app_env: bulck create executor: type %d unknown!", def->type);
            goto BULCK_CREATE_ERROR;
        }

        if (created[i] == NULL) {
            CPE_ERROR(module->m_em, "plugin_app_env: bulck create executor: create executor of %s fail!", def->meta);
            goto BULCK_CREATE_ERROR;
        }
    }

    return 0;
    
BULCK_CREATE_ERROR:
    for(; i > 0; --i) {
        plugin_app_env_executor_free(created[i - 1]);
    }
    
    return -1;
}

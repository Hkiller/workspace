#include <assert.h>
#include "cpe/cfg/cfg_read.h"
#include "cpe/nm/nm_manage.h"
#include "cpe/nm/nm_read.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dr/dr_json.h"
#include "gd/app/app_module.h"
#include "gd/app/app_context.h"
#include "plugin_app_env_module_i.h"
#include "plugin_app_env_executor_i.h"
#include "plugin_app_env_monitor_i.h"
#include "plugin_app_env_request_i.h"

static void plugin_app_env_module_clear(nm_node_t node);

struct nm_node_type s_nm_node_type_render_app_env_module = {
    "plugin_app_env_module",
    plugin_app_env_module_clear
};

static struct {
    const char * name; 
    int (*init)(plugin_app_env_module_t module);
    void (*fini)(plugin_app_env_module_t module);
} s_auto_reg_products[] = {
    { "backend", plugin_app_env_backend_init, plugin_app_env_backend_fini }
    , { "request", plugin_app_env_request_init, plugin_app_env_request_fini }
};

plugin_app_env_module_t
plugin_app_env_module_create(gd_app_context_t app, mem_allocrator_t alloc, const char * name, error_monitor_t em) {
    struct plugin_app_env_module * module;
    nm_node_t module_node;
    int8_t component_pos = 0;

    assert(app);

    if (name == NULL) name = "plugin_app_env_module";

    module_node = nm_group_create(gd_app_nm_mgr(app), name, sizeof(struct plugin_app_env_module));
    if (module_node == NULL) return NULL;

    module = (plugin_app_env_module_t)nm_node_data(module_node);

    module->m_app = app;
    module->m_alloc = alloc;
    module->m_em = em;
    module->m_suspend = 0;
    module->m_max_request_id = 0;
    TAILQ_INIT(&module->m_processing_requests);

    module->m_max_request_id = 0;
    
    if (cpe_hash_table_init(
            &module->m_executors,
            module->m_alloc,
            (cpe_hash_fun_t) plugin_app_env_executor_hash,
            (cpe_hash_eq_t) plugin_app_env_executor_eq,
            CPE_HASH_OBJ2ENTRY(plugin_app_env_executor, m_hh),
            -1) != 0)
    {
        mem_free(alloc, module);
        return NULL;
    }

    if (cpe_hash_table_init(
            &module->m_monitors,
            module->m_alloc,
            (cpe_hash_fun_t) plugin_app_env_monitor_hash,
            (cpe_hash_eq_t) plugin_app_env_monitor_eq,
            CPE_HASH_OBJ2ENTRY(plugin_app_env_monitor, m_hh),
            -1) != 0)
    {
        cpe_hash_table_fini(&module->m_executors);
        mem_free(alloc, module);
        return NULL;
    }

    if (cpe_hash_table_init(
            &module->m_requests,
            module->m_alloc,
            (cpe_hash_fun_t) plugin_app_env_request_hash,
            (cpe_hash_eq_t) plugin_app_env_request_eq,
            CPE_HASH_OBJ2ENTRY(plugin_app_env_request, m_hh),
            -1) != 0)
    {
        cpe_hash_table_fini(&module->m_monitors);
        cpe_hash_table_fini(&module->m_executors);
        mem_free(alloc, module);
        return NULL;
    }
    
    for(component_pos = 0; component_pos < CPE_ARRAY_SIZE(s_auto_reg_products); ++component_pos) {
        if (s_auto_reg_products[component_pos].init(module) != 0) {
            CPE_ERROR(em, "%s: regist product %s fail!", name, s_auto_reg_products[component_pos].name);
            for(; component_pos > 0; component_pos--) {
                s_auto_reg_products[component_pos - 1].fini(module);
            }
            cpe_hash_table_fini(&module->m_monitors);
            cpe_hash_table_fini(&module->m_executors);
            cpe_hash_table_fini(&module->m_requests);
            return NULL;
        }
    }

    mem_buffer_init(&module->m_dump_buffer, alloc);
    
    nm_node_set_type(module_node, &s_nm_node_type_render_app_env_module);

    return module;
}

static void plugin_app_env_module_clear(nm_node_t node) {
    plugin_app_env_module_t module;
    int component_pos;

    module = (plugin_app_env_module_t)nm_node_data(node);

    for(component_pos = CPE_ARRAY_SIZE(s_auto_reg_products); component_pos > 0; component_pos--) {
        s_auto_reg_products[component_pos - 1].fini(module);
    }

    plugin_app_env_executor_free_all(module);
    cpe_hash_table_fini(&module->m_executors);

    plugin_app_env_monitor_free_all(module);
    cpe_hash_table_fini(&module->m_monitors);
    
    plugin_app_env_request_free_all(module);
    cpe_hash_table_fini(&module->m_requests);
    assert(TAILQ_EMPTY(&module->m_processing_requests));
           
    mem_buffer_clear(&module->m_dump_buffer);
}

gd_app_context_t plugin_app_env_module_app(plugin_app_env_module_t module) {
    return module->m_app;
}

void plugin_app_env_module_free(plugin_app_env_module_t module) {
    nm_node_t module_node;
    assert(module);

	module_node = nm_node_from_data(module);
    if (nm_node_type(module_node) != &s_nm_node_type_render_app_env_module) return;
    nm_node_free(module_node);
}

plugin_app_env_module_t
plugin_app_env_module_find(gd_app_context_t app, cpe_hash_string_t name) {
    nm_node_t node;

    node = nm_mgr_find_node(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_render_app_env_module) return NULL;
    return (plugin_app_env_module_t)nm_node_data(node);
}

plugin_app_env_module_t
plugin_app_env_module_find_nc(gd_app_context_t app, const char * name) {
    nm_node_t node;

	if (name == NULL) name = "plugin_app_env_module";

    node = nm_mgr_find_node_nc(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_render_app_env_module) return NULL;
    return (plugin_app_env_module_t)nm_node_data(node);
}

const char * plugin_app_env_module_name(plugin_app_env_module_t module) {
    return nm_node_name(nm_node_from_data(module));
}

void plugin_app_env_post_request(plugin_app_env_module_t module, LPDRMETA meta, void const * data, size_t data_size) {
    struct plugin_app_env_executor key;
    plugin_app_env_executor_t executor, executor_next;
    uint32_t processed_count;

    key.m_apply_to = meta;

    executor = cpe_hash_table_find(&module->m_executors, &key);

    processed_count = 0;
    while(executor) {
        executor_next = cpe_hash_table_find_next(&module->m_executors, executor);

        switch(executor->m_type) {
        case plugin_app_env_executor_oneway:
            executor->m_exec_oneway(executor->m_ctx, meta, data, data_size);
            processed_count++;
            break;
        case plugin_app_env_executor_sync: {
            if (executor->m_exec_sync(executor->m_ctx, meta, data, data_size, NULL, NULL) != 0) {
                CPE_ERROR(
                    module->m_em, "plugin_app_env_post_request: %s execute sync fail, req=%s",
                    dr_meta_name(meta), dr_json_dump_inline(&module->m_dump_buffer, data, data_size, meta));
            }
            else {
                processed_count++;
            }
            break;
        }
        case plugin_app_env_executor_async:
            if (executor->m_exec_async(executor->m_ctx, meta, data, data_size, 0) != 0) {
                CPE_ERROR(
                    module->m_em, "plugin_app_env_post_request: %s execute async fail, req=%s",
                    dr_meta_name(meta), dr_json_dump_inline(&module->m_dump_buffer, data, data_size, meta));
            }
            else {
                processed_count++;
            }
            break;
        default:
            CPE_ERROR(module->m_em, "plugin_app_env_post_request: unknown executor type %d", executor->m_type);
        }
        
        executor = executor_next;
    }

    if (module->m_debug) {
        CPE_INFO(
            module->m_em, "plugin_app_env_post_request: %s processd %d, req=%s",
            dr_meta_name(meta), processed_count,
            dr_json_dump_inline(&module->m_dump_buffer, data, data_size, meta));
    }
}

int plugin_app_env_exec_request_sync(
    plugin_app_env_module_t module,
    dr_data_t * result, mem_buffer_t result_alloc,
    LPDRMETA meta, void const * data, size_t data_size)
{
    struct plugin_app_env_executor key;
    plugin_app_env_executor_t executor, executor_next;
    uint8_t sync_processed = 0;
    int rv = 0;

    key.m_apply_to = meta;

    executor = cpe_hash_table_find(&module->m_executors, &key);

    while(executor) {
        executor_next = cpe_hash_table_find_next(&module->m_executors, executor);

        switch(executor->m_type) {
        case plugin_app_env_executor_oneway:
            executor->m_exec_oneway(executor->m_ctx, meta, data, data_size);
            break;
        case plugin_app_env_executor_sync:
            if (sync_processed) {
                CPE_ERROR(
                    module->m_em, "plugin_app_env_executor_async: %s execute skip for already processed",
                    dr_meta_name(meta));
                rv = -1;
            }
            else {
                sync_processed = 1;
                if (executor->m_exec_sync(executor->m_ctx, meta, data, data_size, result, result_alloc) != 0) {
                    CPE_ERROR(
                        module->m_em, "plugin_app_env_executor_async: %s execute sync fail, req=%s",
                        dr_meta_name(meta), dr_json_dump_inline(&module->m_dump_buffer, data, data_size, meta));
                    rv = -1;
                }
            }
            break;
        case plugin_app_env_executor_async:
            if (module->m_debug) {
                CPE_INFO(module->m_em, "plugin_app_env_executor_async: %s ignore asnync executor!", dr_meta_name(meta));
            }
            break;
        default:
            CPE_ERROR(module->m_em, "plugin_app_env_executor_async: unknown executor type %d", executor->m_type);
        }
        
        executor = executor_next;
    }

    if (!sync_processed) {
        CPE_ERROR(module->m_em, "plugin_app_env_exec_request_sync: %s no sync executor response", dr_meta_name(meta));
        rv = -1;
    }
    
    if (rv == 0 && module->m_debug) {
        CPE_INFO(
            module->m_em, "plugin_app_env_exec_request_sync: %s sync processed, req=%s",
            dr_meta_name(meta),
            dr_json_dump_inline(&module->m_dump_buffer, data, data_size, meta));
    }

    return rv;
}

int plugin_app_env_exec_request_asnyc(
    plugin_app_env_module_t module, uint32_t * id,
    void * receiver_ctx, plugin_app_env_request_receiver_fun_t receiver_fun, void (*ctx_free)(void *),
    LPDRMETA meta, void const * data, size_t data_size)
{
    struct plugin_app_env_executor key;
    plugin_app_env_executor_t executor, executor_next;
    uint8_t processed = 0;
    int rv = 0;
    plugin_app_env_request_t request;

    request = plugin_app_env_request_create(module, meta, receiver_ctx, receiver_fun, NULL);
    if (request == NULL) {
        CPE_ERROR(module->m_em, "plugin_app_env_exec_request_asnyc: %s: create request fail!", dr_meta_name(meta));
        return -1;
    }
    
    key.m_apply_to = meta;
    executor = cpe_hash_table_find(&module->m_executors, &key);

    while(executor) {
        executor_next = cpe_hash_table_find_next(&module->m_executors, executor);

        switch(executor->m_type) {
        case plugin_app_env_executor_oneway:
            executor->m_exec_oneway(executor->m_ctx, meta, data, data_size);
            break;
        case plugin_app_env_executor_sync:
            if (module->m_debug) {
                CPE_INFO(module->m_em, "plugin_app_env_executor_async: %s: execute skip for only support sync", dr_meta_name(meta));
            }
            break;
        case plugin_app_env_executor_async:
            if (!processed) {
                processed = 1;
                if (executor->m_exec_async(executor->m_ctx, meta, data, data_size, request->m_id) != 0) {
                    CPE_ERROR(
                        module->m_em, "plugin_app_env_executor_async: %s execute start fail, req=%s",
                        dr_meta_name(meta), dr_json_dump_inline(&module->m_dump_buffer, data, data_size, meta));
                    rv = -1;
                }
            }
            else {
                if (module->m_debug) {
                    CPE_INFO(module->m_em, "plugin_app_env_executor_async: %s: execute skip for already processed", dr_meta_name(meta));
                }
            }
            break;
        default:
            CPE_ERROR(module->m_em, "plugin_app_env_executor_async: unknown executor type %d", executor->m_type);
        }
        
        executor = executor_next;
    }

    if (rv == 0 && !processed) {
        CPE_ERROR(module->m_em, "plugin_app_env_executor_async: %s no sync or async executor response", dr_meta_name(meta));
        rv = -1;
    }

    if (rv == 0 && module->m_debug) {
        CPE_INFO(
            module->m_em, "plugin_app_env_exec_request_async: %s async begin id=%d, req=%s",
            dr_meta_name(meta), request->m_id,
            dr_json_dump_inline(&module->m_dump_buffer, data, data_size, meta));
    }
    
    if (rv != 0) {
        plugin_app_env_request_free(request);
    }
    else {
        request->m_receiver_ctx_free = ctx_free;
        if (id) *id = request->m_id;
    }
    
    return rv;
}

uint32_t plugin_app_env_send_notification(plugin_app_env_module_t module, LPDRMETA meta, void const * data, size_t data_size) {
    struct plugin_app_env_monitor key;
    plugin_app_env_monitor_t monitor, monitor_next;
    uint32_t processed_count = 0;
    int rv;
    
    key.m_meta_name = dr_meta_name(meta);
    monitor = cpe_hash_table_find(&module->m_monitors, &key);

    while(monitor) {
        monitor_next = cpe_hash_table_find_next(&module->m_monitors, monitor);

        rv = monitor->m_fun(monitor->m_ctx, meta, data, data_size);
        if (rv > 0) processed_count += (uint32_t)rv;
        
        monitor = monitor_next;
    }

    return processed_count;
}

EXPORT_DIRECTIVE
int plugin_app_env_module_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    plugin_app_env_module_t app_env_module;

    app_env_module =
        plugin_app_env_module_create(
            app, gd_app_alloc(app), gd_app_module_name(module), gd_app_em(app));
    if (app_env_module == NULL) return -1;

    app_env_module->m_debug = cfg_get_int32(cfg, "debug", 0);

    if (app_env_module->m_debug) {
        CPE_INFO(
            gd_app_em(app), "%s: create: done",
            plugin_app_env_module_name(app_env_module));
    }

    return 0;
}

EXPORT_DIRECTIVE
void plugin_app_env_module_app_fini(gd_app_context_t app, gd_app_module_t module) {
    plugin_app_env_module_t plugin_app_env_module;

    plugin_app_env_module = plugin_app_env_module_find_nc(app, gd_app_module_name(module));
    if (plugin_app_env_module) {
        plugin_app_env_module_free(plugin_app_env_module);
    }
}


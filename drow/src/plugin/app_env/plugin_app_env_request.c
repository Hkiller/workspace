#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dr/dr_json.h"
#include "gd/app/app_context.h"
#include "plugin_app_env_request_i.h"

plugin_app_env_request_t plugin_app_env_request_create(
    plugin_app_env_module_t module, LPDRMETA request_meta,
    void * receiver_ctx, plugin_app_env_request_receiver_fun_t receiver_fun, void (*receiver_ctx_free)(void *))
{
    plugin_app_env_request_t request;

    request = mem_alloc(module->m_alloc, sizeof(struct plugin_app_env_request));
    if (request == NULL) {
        CPE_ERROR(module->m_em, "plugin_app_env request alloc fail!");
        return NULL;
    }

    request->m_module = module;
    request->m_id = module->m_max_request_id + 1;
    request->m_request_meta = request_meta;
    request->m_receiver_ctx = receiver_ctx;
    request->m_receiver_fun = receiver_fun;
    request->m_receiver_ctx_free = receiver_ctx_free;
    request->m_is_runing = 1;
    request->m_rv = -1;
    request->m_result.m_data = NULL;

    cpe_hash_entry_init(&request->m_hh);
    if (cpe_hash_table_insert(&module->m_requests, request) != 0) {
        CPE_ERROR(module->m_em, "plugin_app_env request %d insert fail!", request->m_id);
        mem_free(module->m_alloc, request);
        return NULL;
    }

    module->m_max_request_id++;
    return request;
}

void plugin_app_env_request_free(plugin_app_env_request_t request) {
    plugin_app_env_module_t module = request->m_module;

    if (request->m_receiver_ctx && request->m_receiver_ctx_free) {
        request->m_receiver_ctx_free(request->m_receiver_ctx);
    }

    if (request->m_result.m_data) {
        mem_free(module->m_alloc, request->m_result.m_data);
        request->m_result.m_data = NULL;
    }

    if (!request->m_is_runing) {
        TAILQ_REMOVE(&module->m_processing_requests, request, m_next_for_processing);
    }
    
    cpe_hash_table_remove_by_ins(&module->m_requests, request);

    mem_free(module->m_alloc, request);
}

void plugin_app_env_request_free_all(plugin_app_env_module_t module) {
    struct cpe_hash_it request_it;
    plugin_app_env_request_t request;

    cpe_hash_it_init(&request_it, &module->m_requests);

    request = cpe_hash_it_next(&request_it);
    while (request) {
        plugin_app_env_request_t next = cpe_hash_it_next(&request_it);
        plugin_app_env_request_free(request);
        request = next;
    }
}

plugin_app_env_request_t plugin_app_env_request_find(plugin_app_env_module_t module, uint32_t id) {
    struct plugin_app_env_request key;
    key.m_id = id;
    return cpe_hash_table_find(&module->m_requests, &key);
}


int plugin_app_env_cancel_request_by_id(plugin_app_env_module_t module, uint32_t id) {
    plugin_app_env_request_t request;

    request = plugin_app_env_request_find(module, id);
    if (request == NULL) return -1;

    if (!request->m_is_runing) return -1;

    request->m_is_runing = 0;
    TAILQ_INSERT_TAIL(&module->m_processing_requests, request, m_next_for_processing);
    request->m_rv = -1;

    return 0;
}

uint32_t plugin_app_env_cancel_requests_by_req(plugin_app_env_module_t module, const char * req_name) {
    struct cpe_hash_it request_it;
    plugin_app_env_request_t request;
    uint32_t count = 0;
    
    cpe_hash_it_init(&request_it, &module->m_requests);

    request = cpe_hash_it_next(&request_it);
    while (request) {
        plugin_app_env_request_t next = cpe_hash_it_next(&request_it);

        if (strcmp(dr_meta_name(request->m_request_meta), req_name) == 0) {
            count++;

            if (request->m_is_runing) {
                request->m_is_runing = 0;
                TAILQ_INSERT_TAIL(&module->m_processing_requests, request, m_next_for_processing);
                request->m_rv = -1;
            }
        }
        
        request = next;
    }

    return count;
}

uint32_t plugin_app_env_clear_requests_by_ctx(plugin_app_env_module_t module, void * ctx) {
    struct cpe_hash_it request_it;
    plugin_app_env_request_t request;
    uint32_t count = 0;

    cpe_hash_it_init(&request_it, &module->m_requests);

    request = cpe_hash_it_next(&request_it);
    while (request) {
        plugin_app_env_request_t next = cpe_hash_it_next(&request_it);

        if (request->m_receiver_ctx == ctx) {
            count++;
            plugin_app_env_request_free(request);
        }
        
        request = next;
    }

    return count;
}

int plugin_app_env_request_set_result(
    plugin_app_env_request_t request, int rv, LPDRMETA meta, void const * data, size_t data_size)
{
    plugin_app_env_module_t module = request->m_module;
    
    if (!request->m_is_runing) {
        CPE_ERROR(module->m_em, "plugin_app_env: notify result: request %d is not runing!", request->m_id);
        return -1;
    }

    request->m_is_runing = 0;
    TAILQ_INSERT_TAIL(&module->m_processing_requests, request, m_next_for_processing);

    request->m_rv = rv;

    if (data) {
        request->m_result.m_data = mem_alloc(module->m_alloc, data_size);
        if (request->m_result.m_data == NULL) {
            CPE_ERROR(module->m_em, "plugin_app_env: notify result: alloc result data buf fail(size=%d)!", (int)data_size);
            return -1;
        }
        memcpy(request->m_result.m_data, data, data_size);
        request->m_result.m_meta = meta;
        request->m_result.m_size = data_size;
    }

    return 0;
}

int plugin_app_env_notify_request_result(
    plugin_app_env_module_t module, uint32_t id,
    int rv, LPDRMETA meta, void const * data, size_t data_size)
{
    plugin_app_env_request_t request;

    request = plugin_app_env_request_find(module, id);
    if (request == NULL) {
        CPE_ERROR(module->m_em, "plugin_app_env: notify result: request %d not exist!", id);
        return -1;
    }

    return plugin_app_env_request_set_result(request, rv, meta, data, data_size);
}

uint32_t plugin_app_env_request_hash(plugin_app_env_request_t request) {
    return request->m_id;
}

int plugin_app_env_request_eq(plugin_app_env_request_t l, plugin_app_env_request_t r) {
    return l->m_id == r->m_id ? 1 : 0;
}

static ptr_int_t plugin_app_env_request_process(void * ctx, ptr_int_t arg, float delta_s) {
    plugin_app_env_module_t module = ctx;
    
    while(!TAILQ_EMPTY(&module->m_processing_requests)) {
        plugin_app_env_request_t request = TAILQ_FIRST(&module->m_processing_requests);

        if (request->m_receiver_fun) {
            if (module->m_debug) {
                if (request->m_result.m_data) {
                    CPE_INFO(
                        module->m_em, "plugin_app_env: request %d(%s) notify result: rv=%d, res=%s",
                        request->m_id, dr_meta_name(request->m_request_meta),
                        request->m_rv,
                        dr_json_dump_inline(&module->m_dump_buffer, request->m_result.m_data, request->m_result.m_size, request->m_result.m_meta));
                }
                else {
                    CPE_INFO(
                        module->m_em, "plugin_app_env: request %d(%s) notify result: rv=%d",
                        request->m_id, dr_meta_name(request->m_request_meta), request->m_rv);
                }
            }
    
            request->m_receiver_fun(request->m_receiver_ctx, request->m_id, request->m_rv, request->m_result.m_data ? &request->m_result : NULL);
        }

        plugin_app_env_request_free(request);
    }

    return 0;
}

int plugin_app_env_request_init(plugin_app_env_module_t module) {
    if (gd_app_tick_add(module->m_app, plugin_app_env_request_process, module, 0) != 0) {
        CPE_ERROR(module->m_em, "plugin_app_env: add tick func fail");
        return -1;
    }
    
    return 0;
}

void plugin_app_env_request_fini(plugin_app_env_module_t module) {
    gd_app_tick_remove(module->m_app, plugin_app_env_request_process, module);
}


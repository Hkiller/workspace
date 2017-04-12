#include <assert.h>
#include "cpe/utils/string_utils.h"
#include "gd/app/app_context.h"
#include "app_attr_request_i.h"
#include "app_attr_formula_i.h"
#include "app_attr_provider_i.h"
#include "app_attr_synchronizer_i.h"
#include "app_attr_attr_binding_i.h"

app_attr_request_t
app_attr_request_create(app_attr_module_t module) {
    app_attr_request_t request;

    request = TAILQ_FIRST(&module->m_free_requests);
    if (request) {
        TAILQ_REMOVE(&module->m_free_requests, request, m_next_for_module);
    }
    else {
        request = mem_calloc(module->m_alloc, sizeof(struct app_attr_request));
        if (request == NULL) {
            CPE_ERROR(module->m_em, "ad: create request: alloc fail!");
            return NULL;
        }
    }

    request->m_module = module;
    request->m_state = app_attr_request_init;
    request->m_id = module->m_request_max_id + 1;
    request->m_result_ctx = NULL;
    request->m_result_processor = NULL;
    request->m_result_arg = NULL;
    request->m_result_arg_free = NULL;
    
    request->m_is_success = 0;

    TAILQ_INIT(&request->m_formulas);
    TAILQ_INIT(&request->m_attrs);

    module->m_request_count++;
    TAILQ_INSERT_TAIL(&module->m_requests, request, m_next_for_module);
    TAILQ_INSERT_TAIL(&module->m_requests_to_process, request, m_next_for_state);

    module->m_request_max_id++;

    return request;
}

void app_attr_request_free(app_attr_request_t request) {
    app_attr_module_t module;

    module = request->m_module;

    if (request->m_state != app_attr_request_waiting) {
        TAILQ_REMOVE(&module->m_requests_to_process, request, m_next_for_state);
    }

    if (request->m_result_arg && request->m_result_arg_free) {
        request->m_result_arg_free(request->m_result_arg);
    }

    while(!TAILQ_EMPTY(&request->m_formulas)) {
        app_attr_formula_free(TAILQ_FIRST(&request->m_formulas));
    }

    while(!TAILQ_EMPTY(&request->m_attrs)) {
        app_attr_attr_binding_free(TAILQ_FIRST(&request->m_attrs));
    }

    module->m_request_count--;
    TAILQ_REMOVE(&module->m_requests, request, m_next_for_module);

    TAILQ_INSERT_TAIL(&module->m_free_requests, request, m_next_for_module);
}

void app_attr_request_real_free(app_attr_request_t request) {
    TAILQ_REMOVE(&request->m_module->m_free_requests, request, m_next_for_module);
    mem_free(request->m_module->m_alloc, request);
}

app_attr_module_t app_attr_request_module(app_attr_request_t request) {
    return request->m_module;
}

app_attr_request_t
app_attr_request_find_by_id(app_attr_module_t module, uint32_t id) {
    app_attr_request_t request;

    TAILQ_FOREACH(request, &module->m_requests, m_next_for_module) {
        if (request->m_id == id) return request;
    }
    
    return NULL;
}

uint32_t app_attr_request_id(app_attr_request_t request) {
    return request->m_id;
}

void app_attr_request_set_state(app_attr_request_t request, app_attr_request_state_t state) {
    if (request->m_state == state) return;

    if (request->m_state == app_attr_request_waiting) {
        TAILQ_INSERT_TAIL(&request->m_module->m_requests_to_process, request, m_next_for_state);
    }
    else {
        if (state == app_attr_request_waiting) {
            assert(request->m_state != app_attr_request_waiting);
            TAILQ_REMOVE(&request->m_module->m_requests_to_process, request, m_next_for_state);
        }
    }

    request->m_state = state;
}

void app_attr_request_tick(app_attr_module_t module) {
    app_attr_request_t request, next_request;
    
    for(request = TAILQ_FIRST(&module->m_requests_to_process); request; request = next_request) {
        next_request = TAILQ_NEXT(request, m_next_for_state);

        if (request->m_state == app_attr_request_init) { /*首次启动 */
            app_attr_attr_binding_t attr_binding;
            uint32_t not_readable_count = 0;
            
            TAILQ_FOREACH(attr_binding, &request->m_attrs, m_next_for_product) {
                app_attr_attr_t attr = attr_binding->m_attr;

                if (!TAILQ_EMPTY(&attr->m_synchronizers)) {
                    app_attr_attr_binding_t synchronizer_binding;
                    TAILQ_FOREACH(synchronizer_binding, &attr->m_synchronizers, m_next_for_attr) {
                        app_attr_synchronizer_t synchronizer = synchronizer_binding->m_product;
                        if (synchronizer->m_state == app_attr_synchronizer_idle) {
                            app_attr_synchronizer_set_state(synchronizer, app_attr_synchronizer_trigger);
                        }
                    }
                    attr->m_state = app_attr_attr_waiting;
                }

                if (attr->m_state != app_attr_attr_waiting) continue;

                if (module->m_debug) {
                    CPE_INFO(module->m_em, "app_attr_request_tick: request %d: attr %s is waiting", request->m_id, attr->m_name);
                }
                not_readable_count++;
            }

            app_attr_request_set_state(request, not_readable_count > 0 ? app_attr_request_waiting : app_attr_request_done);
        }
        else if (request->m_state == app_attr_request_check) { /*后续检查 */
            app_attr_attr_binding_t attr_binding;
            uint32_t not_readable_count = 0;
            
            TAILQ_FOREACH(attr_binding, &request->m_attrs, m_next_for_product) {
                if (attr_binding->m_attr->m_state != app_attr_attr_waiting) continue;

                if (module->m_debug) {
                    CPE_INFO(module->m_em, "app_attr_request_tick: request %d: attr %s is waiting", request->m_id, attr_binding->m_attr->m_name);
                }
            }

            app_attr_request_set_state(request, not_readable_count > 0 ? app_attr_request_waiting : app_attr_request_done);
        }
        
        if (request->m_state == app_attr_request_done) {
            if (request->m_result_processor) {
                request->m_result_processor(request->m_result_ctx, request, request->m_result_arg);
            }
            app_attr_request_free(request);
        }
    }
}

int app_attr_request_set_result_processor(
    app_attr_request_t request,
    void * ctx, app_attr_request_result_process_fun_t on_result,
    void * arg, void (*arg_free)(void *))
{
    if (request->m_state != app_attr_request_init) {
        CPE_ERROR(request->m_module->m_em, "app_attr_request_set_result_processor: request is not in init!");
        return -1;
    }

    if (request->m_result_processor != NULL) {
        CPE_ERROR(request->m_module->m_em, "app_attr_request_set_result_processor: request result processor is already setted!");
        return -1;
    }
    
    request->m_result_ctx = ctx;
    request->m_result_processor = on_result;
    request->m_result_arg = arg;
    request->m_result_arg_free = arg_free;
    
    return 0;
}

uint8_t app_attr_request_is_success(app_attr_request_t request) {
    return request->m_is_success;
}
    
int app_attr_request_set_done(app_attr_request_t request, uint8_t is_success) {
    app_attr_module_t module = request->m_module;
    
    if (request->m_state == app_attr_request_done) {
        CPE_ERROR(module->m_em, "app_attr_request_set_done: request is already done!");
        return -1;
    }

    if (request->m_state != app_attr_request_init) {
        TAILQ_INSERT_TAIL(&module->m_requests_to_process, request, m_next_for_state);
    }

    request->m_state = app_attr_request_done;
    request->m_is_success = is_success;
    
    return 0;
}

int app_attr_request_remove_by_id(app_attr_module_t module, uint32_t request_id) {
    app_attr_request_t request = app_attr_request_find_by_id(module, request_id);
    if (request) {
        app_attr_request_free(request);
    }
    
    return 0;
}

int app_attr_request_remove_by_ctx(app_attr_module_t module, void * ctx) {
    app_attr_request_t request, next_request;

    for(request = TAILQ_FIRST(&module->m_requests); request; request = next_request) {
        next_request = TAILQ_NEXT(request, m_next_for_module);

        if (request->m_result_ctx == ctx) {
            app_attr_request_free(request);
        }
    }

    return 0;
}

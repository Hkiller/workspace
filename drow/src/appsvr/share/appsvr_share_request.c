#include <assert.h>
#include "cpe/utils/string_utils.h"
#include "gd/app/app_context.h"
#include "appsvr_share_request_i.h"
#include "appsvr_share_adapter_i.h"
#include "appsvr_share_request_block_i.h"

appsvr_share_request_t
appsvr_share_request_create(appsvr_share_adapter_t adapter) {
    appsvr_share_module_t module = adapter->m_module;
    appsvr_share_request_t request;

    request = TAILQ_FIRST(&module->m_free_requests);
    if (request) {
        TAILQ_REMOVE(&module->m_free_requests, request, m_next_for_module);
    }
    else {
        request = mem_calloc(module->m_alloc, sizeof(struct appsvr_share_request));
        if (request == NULL) {
            CPE_ERROR(module->m_em, "ad: create request: alloc fail!");
            return NULL;
        }
    }

    request->m_module = module;
    request->m_adapter = adapter;
    request->m_state = appsvr_share_request_init;
    request->m_id = module->m_request_max_id + 1;
    request->m_result_ctx = NULL;
    request->m_result_processor = NULL;
    request->m_result_arg = NULL;
    request->m_result_arg_free = NULL;
    
    request->m_is_success = 0;
    TAILQ_INIT(&request->m_blocks);
    
    module->m_request_count++;
    TAILQ_INSERT_TAIL(&module->m_requests, request, m_next_for_module);
    TAILQ_INSERT_TAIL(&module->m_requests_to_process, request, m_next_for_state);

    module->m_request_max_id++;

    return request;
}

void appsvr_share_request_free(appsvr_share_request_t request) {
    appsvr_share_module_t module;

    module = request->m_module;

    while(!TAILQ_EMPTY(&request->m_blocks)) {
        appsvr_share_request_block_free(TAILQ_FIRST(&request->m_blocks));
    }
    
    if (request->m_state != appsvr_share_request_working) {
        TAILQ_REMOVE(&module->m_requests_to_process, request, m_next_for_state);
    }

    if (request->m_result_arg && request->m_result_arg_free) {
        request->m_result_arg_free(request->m_result_arg);
    }
    
    module->m_request_count--;
    TAILQ_REMOVE(&module->m_requests, request, m_next_for_module);

    TAILQ_INSERT_TAIL(&module->m_free_requests, request, m_next_for_module);
}

void appsvr_share_request_real_free(appsvr_share_request_t request) {
    TAILQ_REMOVE(&request->m_module->m_free_requests, request, m_next_for_module);
    mem_free(request->m_module->m_alloc, request);
}

appsvr_share_request_t
appsvr_share_request_find_by_id(appsvr_share_module_t module, uint32_t id) {
    appsvr_share_request_t request;

    TAILQ_FOREACH(request, &module->m_requests, m_next_for_module) {
        if (request->m_id == id) return request;
    }
    
    return NULL;
}

uint32_t appsvr_share_request_id(appsvr_share_request_t request) {
    return request->m_id;
}

void appsvr_share_request_tick(appsvr_share_module_t module) {
    appsvr_share_request_t request, next_request;
    
    for(request = TAILQ_FIRST(&module->m_requests_to_process); request; request = next_request) {
        next_request = TAILQ_NEXT(request, m_next_for_state);

        if (request->m_state == appsvr_share_request_init) {
            if (request->m_adapter->m_commit_fun(request->m_adapter->m_ctx, request) != 0) {
                /*处理完成，设置为已经激活 */
                request->m_state = appsvr_share_request_done;
                request->m_is_success = 0;
            }
            else {
                if (request->m_state == appsvr_share_request_init) {
                    /*处理完成，设置为已经激活 */
                    request->m_state = appsvr_share_request_working;
                    TAILQ_REMOVE(&module->m_requests_to_process, request, m_next_for_state);
                    continue;
                }
                else {
                    assert(request->m_state == appsvr_share_request_done);
                }
            }
        }

        if (request->m_state == appsvr_share_request_done) {
            if (request->m_result_processor) {
                request->m_result_processor(request->m_result_ctx, request, request->m_result_arg);
            }
            appsvr_share_request_free(request);
        }
    }
}

int appsvr_share_request_set_result_processor(
    appsvr_share_request_t request,
    void * ctx, appsvr_share_request_result_process_fun_t on_result,
    void * arg, void (*arg_free)(void *))
{
    if (request->m_state != appsvr_share_request_init) {
        CPE_ERROR(request->m_module->m_em, "appsvr_share_request_set_result_processor: request is not in init!");
        return -1;
    }

    if (request->m_result_processor != NULL) {
        CPE_ERROR(request->m_module->m_em, "appsvr_share_request_set_result_processor: request result processor is already setted!");
        return -1;
    }
    
    request->m_result_ctx = ctx;
    request->m_result_processor = on_result;
    request->m_result_arg = arg;
    request->m_result_arg_free = arg_free;
    
    return 0;
}

uint8_t appsvr_share_request_is_success(appsvr_share_request_t request) {
    return request->m_is_success;
}
    
int appsvr_share_request_set_done(appsvr_share_request_t request, uint8_t is_success) {
    appsvr_share_module_t module = request->m_module;
    
    if (request->m_state == appsvr_share_request_done) {
        CPE_ERROR(module->m_em, "appsvr_share_request_set_done: request is already done!");
        return -1;
    }

    if (request->m_state != appsvr_share_request_init) {
        TAILQ_INSERT_TAIL(&module->m_requests_to_process, request, m_next_for_state);
    }

    request->m_state = appsvr_share_request_done;
    request->m_is_success = is_success;
    
    return 0;
}

int appsvr_share_request_append_str(appsvr_share_request_t request, appsvr_share_request_block_category_t category, const char * str) {
    return appsvr_share_request_block_create(request, category, str, strlen(str) + 1) ? 0 : -1;
}

int appsvr_share_request_append_data(appsvr_share_request_t request, appsvr_share_request_block_category_t category, void const * data, size_t data_size) {
    return appsvr_share_request_block_create(request, category, data, data_size) ? 0 : -1;
}

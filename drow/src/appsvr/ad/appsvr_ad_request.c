#include <assert.h>
#include "cpe/utils/string_utils.h"
#include "appsvr_ad_request_i.h"
#include "appsvr_ad_action_i.h"

appsvr_ad_request_t
appsvr_ad_request_create(
    appsvr_ad_module_t module, appsvr_ad_action_t action,
    void * ctx, appsvr_ad_resonse_fun_t response_fun,
    void * arg, void (*arg_free)(void * ctx))
{
    appsvr_ad_request_t request;

    request = TAILQ_FIRST(&module->m_free_requests);
    if (request) {
        TAILQ_REMOVE(&module->m_free_requests, request, m_next_for_module);
    }
    else {
        request = mem_calloc(module->m_alloc, sizeof(struct appsvr_ad_request));
        if (request == NULL) {
            CPE_ERROR(module->m_em, "ad: create request: alloc fail!");
            return NULL;
        }
    }

    request->m_module = module;
    request->m_action = action;
    request->m_id = module->m_request_max_id + 1;
    request->m_ctx = ctx;
    request->m_response_fun = response_fun;
    request->m_arg = arg;
    request->m_arg_free = arg_free;

    if (action == NULL || action->m_adapter == NULL) {
        request->m_in_process = 0;
        request->m_result = appsvr_ad_start_skip;
    }
    else {
        if (action->m_adapter->m_req_start_fun(action->m_adapter->m_ctx, request, action) != 0) {
            CPE_ERROR(module->m_em, "ad: create request: adapter %s start action %s fail!", action->m_adapter->m_name, action->m_name);
            request->m_in_process = 0;
            request->m_result = appsvr_ad_start_fail;
        }
        else {
            request->m_in_process = 1;
            request->m_result = appsvr_ad_start_fail;
        }
    }

    module->m_request_count++;
    TAILQ_INSERT_TAIL(&module->m_requests, request, m_next_for_module);

    if (action) {
        TAILQ_INSERT_TAIL(&action->m_requests, request, m_next_for_action);
    }

    module->m_request_max_id++;
    
    return request;
}

void appsvr_ad_request_free(appsvr_ad_request_t request) {
    appsvr_ad_module_t module;

    module = request->m_module;

    if (request->m_arg_free) {
        request->m_arg_free(request->m_arg);
    }
    
    if (request->m_action) {
        TAILQ_REMOVE(&request->m_action->m_requests, request, m_next_for_action);
    }
    
    module->m_request_count--;
    TAILQ_REMOVE(&module->m_requests, request, m_next_for_module);

    TAILQ_INSERT_TAIL(&module->m_free_requests, request, m_next_for_module);
}

void appsvr_ad_request_real_free(appsvr_ad_request_t request) {
    TAILQ_REMOVE(&request->m_module->m_free_requests, request, m_next_for_module);
    mem_free(request->m_module->m_alloc, request);
}

appsvr_ad_request_t
appsvr_ad_request_find_by_id(appsvr_ad_module_t module, uint32_t id) {
    appsvr_ad_request_t request;

    TAILQ_FOREACH(request, &module->m_requests, m_next_for_module) {
        if (request->m_id == id) return request;
    }
    
    return NULL;
}

uint32_t appsvr_ad_request_id(appsvr_ad_request_t request) {
    return request->m_id;
}

int appsvr_ad_request_set_result(appsvr_ad_request_t request, appsvr_ad_result_t result) {
    if (!request->m_in_process) {
        CPE_ERROR(request->m_module->m_em, "appsvr_ad_request_set_result: request %d is already done!", request->m_id);
        return -1;
    }

    request->m_result = result;
    request->m_in_process = 0;
    return 0;
}

